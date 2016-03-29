#include <pwd.h>
#include <setjmp.h>
#include <sys/un.h>
#include <sys/wait.h>

#include <readline/history.h>
#include <readline/readline.h>

#include "getopt.h"
#include "openswitch-idl.h"
#include "openvswitch/vlog.h"
#include "ovsdb-idl.h"
#include "smap.h"
#include "snmp_vty.h"
#include "vswitch-idl.h"
#include "vtysh/command.h"
#include "vtysh/lib/version.h"
#include "vtysh/memory.h"
#include "vtysh/vtysh.h"
#include "vtysh/vtysh_ovsdb_config.h"
#include "vtysh/vtysh_ovsdb_if.h"
#include "vtysh/vtysh_user.h"
#include "vtysh/vtysh_utils.h"
#include "vtysh_ovsdb_snmp_context.h"

extern struct ovsdb_idl *idl;

VLOG_DEFINE_THIS_MODULE(snmp_cli);

#ifdef SNMPv3
static bool snmp_v3_user_exists(const char *user) {
    const struct ovsrec_snmpv3_user *v3user_row = NULL;
    bool user_found = false;

    OVSREC_SNMPV3_USER_FOR_EACH(v3user_row, idl) {
    if (v3user_row != NULL) {
        if (strncmp(v3user_row->user_name, user, 32) == 0)
            user_found = true;
            break;
        }
    }
    if (user_found){
        return true;
    }
    return false;
}
#endif

static int snmp_server_host_config(const char *ip, const char *community,
                                   unsigned int port, const char *version,
                                   const char *type) {
    const struct ovsrec_snmp_trap *host_row = NULL;
    enum ovsdb_idl_txn_status status;
    struct ovsdb_idl_txn *status_txn = NULL;
    bool trap_found = false;

    OVSREC_SNMP_TRAP_FOR_EACH(host_row, idl) {
        if (NULL != host_row) {
            if ((strncmp(type, "v3", 2) == 0)) {
#ifdef SNMPv3
                if (!snmp_v3_user_exists(*community)) {
                    vty_out(vty,
                            "User does not exist. Configure v3 user first.\n");
                    return CMD_SUCCESS;
                }
#endif /*SNMPv3 */
                vty_out(vty, "v3 trap is yet to be implemented");
                return CMD_SUCCESS;
            }
            if (strncmp(host_row->receiver_address, ip,
                        MAX_IP_STR_LENGTH) == 0 &&
                ((unsigned int)host_row->receiver_udp_port == port)) {
                trap_found = true;
                break;
            }
        }
    }
    if (trap_found) {
        vty_out(vty, "The SNMP trap receiver host already exists\n");
        return CMD_SUCCESS;
    }
    status_txn = cli_do_config_start();
    if (NULL == status_txn) {
        VLOG_ERR("[%s:%d]: Failed to create OVSDB transaction\n", __FUNCTION__,
                 __LINE__);
        cli_do_config_abort(NULL);
        return CMD_OVSDB_FAILURE;
    }

    host_row = ovsrec_snmp_trap_insert(status_txn);

    ovsrec_snmp_trap_set_receiver_address(host_row, ip);
    ovsrec_snmp_trap_set_receiver_udp_port(host_row, port);
    ovsrec_snmp_trap_set_community_name(host_row, community);
    ovsrec_snmp_trap_set_type(host_row, type);
    ovsrec_snmp_trap_set_version(host_row, version);

    status = cli_do_config_finish(status_txn);

    if (status == TXN_SUCCESS || status == TXN_UNCHANGED) {
        return CMD_SUCCESS;
    } else {
        return CMD_OVSDB_FAILURE;
    }
}

/* TO-DO make type and version to be given as optional
 * in the following DEFUNs
 */
DEFUN(snmp_server_host_inform,
      snmp_server_host_inform_cmd,
      "snmp-server host A.B.C.D  inform  version v2c  { community WORD | port <1-65535> }",
      SNMP_STR HOST_STR
      "IP address of SNMP notification host\n"
      "Configure inform\n"
      "Configure version\n"
      "Configure version v2c inform\n"
      "Configure community name\n"
      "Name of the community, max up to 32 characters (Default : public)\n"
      "Configure UDP port\n"
      "Port from 1-65535.(Default : 162)")
{
    const char *ip = NULL;
    const char *community = NULL;
    unsigned int port = 0;
    const char *type = "inform";
    const char *ver = NULL;

    ip = argv[0];
    community = (((argv[1])) ? argv[1] : DEFAULT_COMMUNITY_TYPE);
    port = ((argv[2]) ? atoi(argv[2]) : DEFAULT_TRAP_RECEIVER_UDP_PORT);
    ver = "v2c";

    snmp_server_host_config(ip, community, port, ver, type);
    return CMD_SUCCESS;
}

DEFUN(snmp_server_host_informv3,
      snmp_server_host_informv3_cmd,
      "snmp-server host A.B.C.D inform  version v3  user WORD { port <1-65535>}",
      SNMP_STR HOST_STR
      "IP address of SNMP notification host\n"
      "Configure inform\n"
      "Configure version\n"
      "Configure version 3 inform \n"
      "Configure v3 user\n"
      "Name of the user\n"
      "Configure UDP port\n"
      "Port from 1-65535 (Default : 162)")
{
    const char *ip = NULL;
    const char *user = NULL;
    unsigned int port = 0;
    const char *type = "inform";
    const char *ver = NULL;

    ip = argv[0];
    ver = "v3";
    user = argv[1];
    port = ((argv[2]) ? atoi(argv[2]) : DEFAULT_TRAP_RECEIVER_UDP_PORT);

    snmp_server_host_config(ip, user, port, ver, type);

    return CMD_SUCCESS;
}

DEFUN(snmp_server_host_trap,
      snmp_server_host_trap_cmd,
      "snmp-server host A.B.C.D trap version (v1 | v2c) {community  WORD | port <1-65535>}",
      SNMP_STR HOST_STR
      "IP address of SNMP notification host\n"
      "Configure trap notifications\n"
      "Configure version\n"
      "Configure version 1 trap\n"
      "Configure version 2c trap\n"
      "Configure community name\n"
      "Name of the community, max up to 32 characters (Default : public)\n"
      "Configure UDP port\n"
      "Port from 1-65535.(Default : 162)")
{
    const char *ip = NULL;
    const char *community = NULL;
    unsigned int port = 0;
    const char *type = "trap";
    const char *ver = NULL;

    ip = argv[0];
    ver = argv[1];
    community = ((argv[2]) ? argv[2] : DEFAULT_COMMUNITY_TYPE);
    port = ((argv[3]) ? atoi(argv[3]) : DEFAULT_TRAP_RECEIVER_UDP_PORT);

    snmp_server_host_config(ip, community, port, ver, type);

    return CMD_SUCCESS;
}

DEFUN(snmp_server_host_trapv3,
      snmp_server_host_trapv3_cmd,
      "snmp-server host A.B.C.D trap  version v3  user  WORD { port <1-65535> }",
      SNMP_STR
      HOST_STR
      "IP address of SNMP notification host\n"
      "Configure trap notifications\n"
      "Configure version\n"
      "Configure version 3 trap\n"
      "Configure user name\n"
      "Name of the user, upto 32 character bytes\n"
      "Configure UDP port\n"
      "Port from 1-65535.(Default : 162)")
{
    const char *ip = NULL;
    const char *user = NULL;
    unsigned int port = 0;
    const char *type = "trap";
    const char *ver = NULL;

    ip = argv[0];
    ver = "v3";
    user = argv[1];
    port = ((argv[2]) ? atoi(argv[2]) : DEFAULT_TRAP_RECEIVER_UDP_PORT);

    snmp_server_host_config(ip, user, port, ver, type);
    return CMD_SUCCESS;
}


static int snmp_server_host_unconfig(const char *ip, const char *community,
                                     unsigned int port, const char *ver,
                                     const char *type) {
    const struct ovsrec_snmp_trap *trap_row = NULL;
    struct ovsdb_idl_txn *status_txn = NULL;
    enum ovsdb_idl_txn_status status;

    status_txn = cli_do_config_start();
    if (NULL == status_txn) {
        VLOG_ERR("[%s:%d]: Failed to create OVSDB transaction\n", __FUNCTION__,
                 __LINE__);
        cli_do_config_abort(NULL);
        return CMD_OVSDB_FAILURE;
    }

    OVSREC_SNMP_TRAP_FOR_EACH(trap_row, idl) {
            if ((strcmp(ip, trap_row->receiver_address) == 0) &&
                (port == (unsigned int)trap_row->receiver_udp_port)) {
                ovsrec_snmp_trap_delete(trap_row);
            }

    }

    status = cli_do_config_finish(status_txn);

    if (status == TXN_SUCCESS || status == TXN_UNCHANGED) {
        return CMD_SUCCESS;
    } else {
        return CMD_OVSDB_FAILURE;
    }
}

DEFUN(no_snmp_server_host_inform,
      no_snmp_server_host_inform_cmd,
      "no snmp-server host A.B.C.D  inform  version v2c  { community WORD | port <1-65535> }",
      NO_STR
      SNMP_STR
      HOST_STR
      "IP address of SNMP notification host\n"
      "Configure inform\n"
      "Configure version\n"
      "Configure version v2c inform\n"
      "Configure community name\n"
      "Name of the community, max up to 32 characters (Default : public)\n"
      "Configure UDP port\n"
      "Port from 1-65535.(Default : 162)") {

    const char *ip = NULL;
    const char *community = NULL;
    unsigned int port = 0;
    const char *type = NULL;
    const char *ver = NULL;

    ip = argv[0];
    type = "inform";
    community = (((argv[1])) ? argv[1] : "public");
    port = ((argv[2]) ? atoi(argv[2]) : DEFAULT_TRAP_RECEIVER_UDP_PORT);
    ver = "v2c";

    snmp_server_host_unconfig(ip, community, port, ver, type);
    return CMD_SUCCESS;
}

DEFUN(no_snmp_server_host_informv3,
      no_snmp_server_host_informv3_cmd,
      "no snmp-server host A.B.C.D inform  version v3  user WORD { port <1-65535>}",
      NO_STR
      SNMP_STR
      HOST_STR
      "IP address of SNMP notification host\n"
      "Configure inform\n"
      "Configure version\n"
      "Configure version 3 inform \n"
      "Configure v3 user\n"
      "Name of the user\n"
      "Configure UDP port\n"
      "Port from 1-65535 (Default : 162)")
{
    const char *ip = NULL;
    const char *user = NULL;
    unsigned int port = 0;
    const char *type = NULL;
    const char *ver = NULL;

    ip = argv[0];
    type = "inform";
    ver = "v3";
    user = argv[1];
    port = ((argv[2]) ? atoi(argv[2]) : DEFAULT_TRAP_RECEIVER_UDP_PORT);

    snmp_server_host_unconfig(ip, user, port, ver, type);

    return CMD_SUCCESS;
}

DEFUN(no_snmp_server_host_trap,
      no_snmp_server_host_trap_cmd,
      "no snmp-server host A.B.C.D trap version (v1 | v2c) {community  WORD | port <1-65535>}",
      NO_STR
      SNMP_STR
      HOST_STR
      "IP address of SNMP notification host\n"
      "Configure trap notifications\n"
      "Configure version\n"
      "Configure version 1 trap\n"
      "Configure version 2c trap\n"
      "Configure community name\n"
      "me of the community, max up to 32 characters (Default : public)\n"
      "Configure UDP port\n"
      "Port from 1-65535.(Default : 162)")
{
    const char *ip = NULL;
    const char *community = NULL;
    unsigned int port = 0;
    const char *type = NULL;
    const char *ver = NULL;

    ip = argv[0];
    type = "trap";
    ver = argv[1];
    community = ((argv[2]) ? argv[2] : DEFAULT_COMMUNITY_TYPE);
    port = ((argv[3]) ? atoi(argv[3]) : DEFAULT_TRAP_RECEIVER_UDP_PORT);

    snmp_server_host_unconfig(ip, community, port, ver, type);

    return CMD_SUCCESS;
}

DEFUN(no_snmp_server_host_trapv3,
      no_snmp_server_host_trapv3_cmd,
      "no snmp-server host A.B.C.D trap  version v3  user  WORD { port <1-65535> }",
      NO_STR
      SNMP_STR
      HOST_STR
      "IP address of SNMP notification host\n"
      "Configure trap notifications\n"
      "Configure version\n"
      "Configure version 3 trap\n"
      "Configure user name\n"
      "Name of the user, upto 32 character bytes\n"
      "Configure UDP port\n"
      "Port from 1-65535.(Default : 162)")
{
    const char *ip = NULL;
    const char *user = NULL;
    unsigned int port = 0;
    const char *type = NULL;
    const char *ver = NULL;

    ip = argv[0];
    type = "trap";
    ver = "v3";
    user = argv[1];
    port = ((argv[2]) ? atoi(argv[2]) : DEFAULT_TRAP_RECEIVER_UDP_PORT);

    snmp_server_host_unconfig(ip, user, port, ver, type);
    return CMD_SUCCESS;
}


static int configure_snmp_agent_port(const char *agent_port, bool isconfig) {
    const struct ovsrec_system *row = NULL;
    enum ovsdb_idl_txn_status txn_status;
    struct smap smap_other_config;
    struct ovsdb_idl_txn *status_txn = cli_do_config_start();

    if (NULL == status_txn) {
        VLOG_ERR(OVSDB_TXN_CREATE_ERROR);
        cli_do_config_abort(status_txn);
        return CMD_OVSDB_FAILURE;
    }

    row = ovsrec_system_first(idl);

    if (!row) {
        VLOG_ERR(OVSDB_ROW_FETCH_ERROR);
        cli_do_config_abort(status_txn);
        return CMD_OVSDB_FAILURE;
    }

    smap_clone(&smap_other_config, &row->other_config);

    if (isconfig)
        smap_replace(&smap_other_config, "snmp_agent_port", agent_port);
    else
        smap_remove(&smap_other_config, "snmp_agent_port");

    ovsrec_system_set_other_config(row, &smap_other_config);

    txn_status = cli_do_config_finish(status_txn);
    smap_destroy(&smap_other_config);
    if (txn_status == TXN_SUCCESS || txn_status == TXN_UNCHANGED) {
        return CMD_SUCCESS;
    } else {
        VLOG_ERR(OVSDB_TXN_COMMIT_ERROR);
        return CMD_OVSDB_FAILURE;
    }
}
DEFUN(snmp_master_agent,
      snmp_master_agent_cmd,
      "snmp-server agent-port <1-65535>",
      SNMP_STR
      "Configure SNMP Master Agent\n"
      "Specify the port value in range (1-65535)\n")
{
    return configure_snmp_agent_port(argv[0], true);
}

DEFUN(no_snmp_master_agent,
      no_snmp_master_agent_cmd,
      "no snmp-server agent-port [<1-65535>]",
      NO_STR
      SNMP_STR
      "Configure SNMP Master Agent\n"
      "Specify the port value in range (1-65535)\n")
{
    if (argv[0])
        return configure_snmp_agent_port(argv[0], false);
    else {
        const struct ovsrec_system *row = NULL;
        enum ovsdb_idl_txn_status txn_status;
        struct smap smap_other_config;
        struct ovsdb_idl_txn *status_txn = cli_do_config_start();

        if (NULL == status_txn) {
            VLOG_ERR(OVSDB_TXN_CREATE_ERROR);
            cli_do_config_abort(status_txn);
            return CMD_OVSDB_FAILURE;
        }

        row = ovsrec_system_first(idl);

        if (!row) {
            VLOG_ERR(OVSDB_ROW_FETCH_ERROR);
            cli_do_config_abort(status_txn);
            return CMD_OVSDB_FAILURE;
        }

        smap_clone(&smap_other_config, &row->other_config);
        smap_remove(&smap_other_config, "snmp_agent_port");

        ovsrec_system_set_other_config(row, &smap_other_config);

        txn_status = cli_do_config_finish(status_txn);
        smap_destroy(&smap_other_config);
        if (txn_status == TXN_SUCCESS || txn_status == TXN_UNCHANGED) {
            return CMD_SUCCESS;
        } else {
            VLOG_ERR(OVSDB_TXN_COMMIT_ERROR);
            return CMD_OVSDB_FAILURE;
        }
    }
}

static int configure_snmp_system_description(const char *sys_desc,
                                             bool isconfig) {
    const struct ovsrec_system *row = NULL;
    enum ovsdb_idl_txn_status txn_status;
    struct smap smap_other_config;
    struct ovsdb_idl_txn *status_txn = cli_do_config_start();

    if (NULL == status_txn) {
        VLOG_ERR(OVSDB_TXN_CREATE_ERROR);
        cli_do_config_abort(status_txn);
        return CMD_OVSDB_FAILURE;
    }

    row = ovsrec_system_first(idl);

    if (!row) {
        VLOG_ERR(OVSDB_ROW_FETCH_ERROR);
        cli_do_config_abort(status_txn);
        return CMD_OVSDB_FAILURE;
    }

    smap_clone(&smap_other_config, &row->other_config);

    if (isconfig)
        smap_replace(&smap_other_config, "system_description", sys_desc);
    else
        smap_remove(&smap_other_config, "system_description");

    ovsrec_system_set_other_config(row, &smap_other_config);

    txn_status = cli_do_config_finish(status_txn);
    smap_destroy(&smap_other_config);
    if (txn_status == TXN_SUCCESS || txn_status == TXN_UNCHANGED) {
        return CMD_SUCCESS;
    } else {
        VLOG_ERR(OVSDB_TXN_COMMIT_ERROR);
        return CMD_OVSDB_FAILURE;
    }
}

DEFUN(snmp_system_description,
      snmp_system_description_cmd,
      "snmp-server system-description .LINE",
      SNMP_STR
      "Configure system description\n"
      "Specify the system description. maximum upto 64bytes\n")
{
    char *sys_desc = NULL;
    if (!argc) {
        vty_out(vty, "%% string argument required%s", VTY_NEWLINE);
        return CMD_WARNING;
    }

    sys_desc = argv_concat(argv, argc, 0);
    return configure_snmp_system_description(sys_desc, true);
}

DEFUN(no_snmp_system_description,
      no_snmp_system_description_cmd,
      "no snmp-server system-description [.LINE]",
      NO_STR SNMP_STR
      "Configure system description\n"
      "Specify the system description. maximum upto 64bytes\n")
{
    char *sys_desc;
    if (argc != 0) {
        sys_desc = argv_concat(argv, argc, 0);
        return configure_snmp_system_description(sys_desc, false);
    } else {
        const struct ovsrec_system *row = NULL;
        enum ovsdb_idl_txn_status txn_status;
        struct smap smap_other_config;
        struct ovsdb_idl_txn *status_txn = cli_do_config_start();

        if (NULL == status_txn) {
            VLOG_ERR(OVSDB_TXN_CREATE_ERROR);
            cli_do_config_abort(status_txn);
            return CMD_OVSDB_FAILURE;
        }

        row = ovsrec_system_first(idl);

        if (!row) {
            VLOG_ERR(OVSDB_ROW_FETCH_ERROR);
            cli_do_config_abort(status_txn);
            return CMD_OVSDB_FAILURE;
        }

        smap_clone(&smap_other_config, &row->other_config);
        smap_remove(&smap_other_config, "system_description");

        ovsrec_system_set_other_config(row, &smap_other_config);

        txn_status = cli_do_config_finish(status_txn);
        smap_destroy(&smap_other_config);
        if (txn_status == TXN_SUCCESS || txn_status == TXN_UNCHANGED) {
            return CMD_SUCCESS;
        } else {
            VLOG_ERR(OVSDB_TXN_COMMIT_ERROR);
            return CMD_OVSDB_FAILURE;
        }
    }
}

static int configure_system_contact(const char *sys_contact, bool isconfig)
{
    const struct ovsrec_system *row = NULL;
    enum ovsdb_idl_txn_status txn_status;
    struct smap smap_other_config;
    struct ovsdb_idl_txn *status_txn = cli_do_config_start();

    if (NULL == status_txn) {
        VLOG_ERR(OVSDB_TXN_CREATE_ERROR);
        cli_do_config_abort(status_txn);
        return CMD_OVSDB_FAILURE;
    }

    row = ovsrec_system_first(idl);

    if (!row) {
        VLOG_ERR(OVSDB_ROW_FETCH_ERROR);
        cli_do_config_abort(status_txn);
        return CMD_OVSDB_FAILURE;
    }

    smap_clone(&smap_other_config, &row->other_config);

    if (isconfig)
        smap_replace(&smap_other_config, "system_contact", sys_contact);
    else
        smap_remove(&smap_other_config, "system_contact");

    ovsrec_system_set_other_config(row, &smap_other_config);

    txn_status = cli_do_config_finish(status_txn);
    smap_destroy(&smap_other_config);
    if (txn_status == TXN_SUCCESS || txn_status == TXN_UNCHANGED) {
        return CMD_SUCCESS;
    } else {
        VLOG_ERR(OVSDB_TXN_COMMIT_ERROR);
        return CMD_OVSDB_FAILURE;
    }
}

DEFUN(snmp_system_contact,
      snmp_system_contact_cmd,
      "snmp-server system-contact .LINE",
      SNMP_STR
      "Configure system contact\n"
      "Specify the system contact in the format - name "
      "name@emailid.com. maximum upto 128bytes\n")
{
    char *sys_con = NULL;
    if (!argc) {
        vty_out(vty, "%% string argument required%s", VTY_NEWLINE);
        return CMD_WARNING;
    }

    sys_con = argv_concat(argv, argc, 0);
    return configure_system_contact(sys_con, true);
}

DEFUN(no_snmp_system_contact,
      no_snmp_system_contact_cmd,
      "no snmp-server system-contact [.LINE]",
      NO_STR
      SNMP_STR
      "Configure system contact\n"
      "Specify the system contact in the format - name "
      "name@emailid.com. maximum upto 128bytes\n")

{
    char *sys_con = NULL;
    if (argc != 0) {
        sys_con = argv_concat(argv, argc, 0);
        return configure_system_contact(sys_con, false);
    } else {
        const struct ovsrec_system *row = NULL;
        enum ovsdb_idl_txn_status txn_status;
        struct smap smap_other_config;
        struct ovsdb_idl_txn *status_txn = cli_do_config_start();

        if (NULL == status_txn) {
            VLOG_ERR(OVSDB_TXN_CREATE_ERROR);
            cli_do_config_abort(status_txn);
            return CMD_OVSDB_FAILURE;
        }

        row = ovsrec_system_first(idl);

        if (!row) {
            VLOG_ERR(OVSDB_ROW_FETCH_ERROR);
            cli_do_config_abort(status_txn);
            return CMD_OVSDB_FAILURE;
        }

        smap_clone(&smap_other_config, &row->other_config);
        smap_remove(&smap_other_config, "system_contact");

        ovsrec_system_set_other_config(row, &smap_other_config);

        txn_status = cli_do_config_finish(status_txn);
        smap_destroy(&smap_other_config);
        if (txn_status == TXN_SUCCESS || txn_status == TXN_UNCHANGED) {
            return CMD_SUCCESS;
        } else {
            VLOG_ERR(OVSDB_TXN_COMMIT_ERROR);
            return CMD_OVSDB_FAILURE;
        }
    }
}

static int configure_system_location(const char *sys_loc, bool isconfig)
{
    const struct ovsrec_system *row = NULL;
    enum ovsdb_idl_txn_status txn_status;
    struct smap smap_other_config;
    struct ovsdb_idl_txn *status_txn = cli_do_config_start();

    if (NULL == status_txn) {
        VLOG_ERR(OVSDB_TXN_CREATE_ERROR);
        cli_do_config_abort(status_txn);
        return CMD_OVSDB_FAILURE;
    }

    row = ovsrec_system_first(idl);

    if (!row) {
        VLOG_ERR(OVSDB_ROW_FETCH_ERROR);
        cli_do_config_abort(status_txn);
        return CMD_OVSDB_FAILURE;
    }

    smap_clone(&smap_other_config, &row->other_config);

    if (isconfig)
        smap_replace(&smap_other_config, "system_location", sys_loc);
    else
        smap_remove(&smap_other_config, "system_location");

    ovsrec_system_set_other_config(row, &smap_other_config);

    txn_status = cli_do_config_finish(status_txn);
    smap_destroy(&smap_other_config);
    if (txn_status == TXN_SUCCESS || txn_status == TXN_UNCHANGED) {
        return CMD_SUCCESS;
    } else {
        VLOG_ERR(OVSDB_TXN_COMMIT_ERROR);
        return CMD_OVSDB_FAILURE;
    }
}

DEFUN(snmp_system_location,
      snmp_system_location_cmd,
      "snmp-server system-location .LINE",
      SNMP_STR
      "Configure system location\n"
      "Specify the system location. maximum upto 128bytes\n")
{
    char *sys_loc = NULL;
    if (!argc) {
        vty_out(vty, "%% string argument required%s", VTY_NEWLINE);
        return CMD_WARNING;
    }

    sys_loc = argv_concat(argv, argc, 0);
    return configure_system_location(sys_loc, true);
}

DEFUN(no_snmp_system_location,
      no_snmp_system_location_cmd,
      "no snmp-server system-location [.LINE]",
      NO_STR
      SNMP_STR
      "Configure system location\n"
      "Specify the system location. maximum upto 128bytes\n")
{
    char *sys_loc = NULL;
    if (argc != 0) {
        sys_loc = argv_concat(argv, argc, 0);
        return configure_system_location(sys_loc, false);
    } else {
        const struct ovsrec_system *row = NULL;
        enum ovsdb_idl_txn_status txn_status;
        struct smap smap_other_config;
        struct ovsdb_idl_txn *status_txn = cli_do_config_start();

        if (NULL == status_txn) {
            VLOG_ERR(OVSDB_TXN_CREATE_ERROR);
            cli_do_config_abort(status_txn);
            return CMD_OVSDB_FAILURE;
        }

        row = ovsrec_system_first(idl);

        if (!row) {
            VLOG_ERR(OVSDB_ROW_FETCH_ERROR);
            cli_do_config_abort(status_txn);
            return CMD_OVSDB_FAILURE;
        }

        smap_clone(&smap_other_config, &row->other_config);
        smap_remove(&smap_other_config, "system_location");

        ovsrec_system_set_other_config(row, &smap_other_config);

        txn_status = cli_do_config_finish(status_txn);
        smap_destroy(&smap_other_config);
        if (txn_status == TXN_SUCCESS || txn_status == TXN_UNCHANGED) {
            return CMD_SUCCESS;
        } else {
            VLOG_ERR(OVSDB_TXN_COMMIT_ERROR);
            return CMD_OVSDB_FAILURE;
        }
    }
}

/*Set SNMPv1/v2c community strings*/
static int configure_community_name(const char *community_name_ptr) {
    const struct ovsrec_system *snmp_row = NULL;
    struct ovsdb_idl_txn *status_txn = NULL;
    size_t i = 0;
    char **community_names = NULL;

    status_txn = cli_do_config_start();

    if (NULL == status_txn) {
        VLOG_ERR("[%s:%d]: Failed to create OVSDB transaction\n", __FUNCTION__,
                 __LINE__);
        cli_do_config_abort(NULL);
        return CMD_OVSDB_FAILURE;
    }

    snmp_row = ovsrec_system_first(idl);

    community_names = xmalloc(sizeof *(snmp_row->snmp_communities) *
                              (snmp_row->n_snmp_communities + 1));

    for (i = 0; i < snmp_row->n_snmp_communities; i++) {
        community_names[i] = snmp_row->snmp_communities[i];
        if (strncmp(community_names[i], community_name_ptr,
                    MAX_COMMUNITY_LENGTH) == 0) {
            vty_out(vty, "This community is already configured\n");
            free(community_names);
            return CMD_SUCCESS;
        }
    }

    community_names[i] = (char *)community_name_ptr;
    ovsrec_system_set_snmp_communities(snmp_row, community_names,
                                       snmp_row->n_snmp_communities + 1);
    free(community_names);

    if (cli_do_config_finish(status_txn)) {
        return CMD_SUCCESS;
    } else {
        return CMD_OVSDB_FAILURE;
    }
}

DEFUN(snmp_community,
      snmp_community_cmd,
      "snmp-server community WORD",
      SNMP_STR
      COMMUNITY_STR
      "Specify the name of the community, max up to 32bytes")
{
    return configure_community_name(argv[0]);
}

static int remove_community_name(const char *community_name_ptr)
{
    const struct ovsrec_system *snmp_row = NULL;
    struct ovsdb_idl_txn *status_txn = NULL;
    size_t i = 0, community_index = 0;
    char **community_names = NULL;
    bool configured_community = false;

    snmp_row = ovsrec_system_first(idl);

    community_names = xmalloc(sizeof *(snmp_row->snmp_communities) *
                              (snmp_row->n_snmp_communities));
    community_index = -1;
    for (i = 0; i < snmp_row->n_snmp_communities; i++) {
        if (0 == strcmp(snmp_row->snmp_communities[i], community_name_ptr)) {
            configured_community = true;
            continue;
        }
        community_names[++community_index] = snmp_row->snmp_communities[i];
    }

    if (!configured_community) {
        vty_out(vty, "The community is not configured\n");
        free(community_names);
        return CMD_SUCCESS;
    }

    status_txn = cli_do_config_start();

    if (NULL == status_txn) {
        VLOG_ERR("[%s:%d]: Failed to create OVSDB transaction\n", __FUNCTION__,
                 __LINE__);
        cli_do_config_abort(NULL);
        return CMD_OVSDB_FAILURE;
    }

    ovsrec_system_set_snmp_communities(snmp_row, community_names,
                                       snmp_row->n_snmp_communities - 1);
    free(community_names);

    if (cli_do_config_finish(status_txn)) {
        return CMD_SUCCESS;
    } else {
        return CMD_OVSDB_FAILURE;
    }
}

DEFUN(no_snmp_community,
      no_snmp_community_cmd,
      "no snmp-server community WORD",
      NO_STR
      SNMP_STR
      COMMUNITY_STR
      "Specify the name of the community, upto 32bytes\n")
{
    vty_out(vty, "community to be unconfigured %s\n", argv[0]);
    return remove_community_name(argv[0]);
}

DEFUN(show_snmp_trap,
      show_snmp_trap_cmd,
      "show snmp trap",
      SHOW_STR
      "snmp configuration\n"
      "trap configuration\n") {
    const struct ovsrec_snmp_trap *trap_row = NULL;
    vty_out(vty,
        "------------------------------------------------------------------------\n");
    vty_out(vty,"%-25s%-12s%-15s%-15s%-32s\n","Host","Port","Type","Version","SecName");
    vty_out(vty,
        "------------------------------------------------------------------------\n");
    OVSREC_SNMP_TRAP_FOR_EACH(trap_row, idl) {
        if (trap_row != NULL) {
            vty_out(vty, "%-25s%-12u%-15s%-15s%-32s\n",
                    trap_row->receiver_address,
                    (unsigned int)trap_row->receiver_udp_port, trap_row->type,
                    trap_row->version, trap_row->community_name);
        }
    }
    return CMD_SUCCESS;
}

DEFUN(show_snmp_community,
      show_snmp_community_cmd,
      "show snmp community",
      SHOW_STR
      "snmp configuration\n"
      "Configured community names\n")
{
    const struct ovsrec_system *snmp_row = NULL;
    size_t i = 0;
    vty_out(vty, "---------------------\n");
    vty_out(vty, "SNMP communities\n");
    vty_out(vty, "---------------------\n");

    snmp_row = ovsrec_system_first(idl);
    if (snmp_row != NULL) {
        while (i < snmp_row->n_snmp_communities) {
            vty_out(vty, "%s\n", snmp_row->snmp_communities[i]);
            i++;
        }
    }
    return CMD_SUCCESS;
}

DEFUN(show_snmp_agent_port,
      show_snmp_agent_port_cmd,
      "show snmp agent-port",
      SHOW_STR
      "snmp configuration\n"
      "snmp agent-port\n")
{
    const struct ovsrec_system *snmp_row = NULL;
    const char *agent_port = NULL;
    snmp_row = ovsrec_system_first(idl);
    agent_port = smap_get(&snmp_row->other_config, "snmp_agent_port");
    if (agent_port)
        vty_out(vty, "SNMP agent port : %d\n", atoi(agent_port));
    return CMD_SUCCESS;
}

DEFUN(show_snmp_system,
      show_snmp_system_cmd,
      "show snmp system",
      SHOW_STR
      "snmp configuration\n"
      "SNMP system MIB configurations\n")
{
    const struct ovsrec_system *snmp_row = NULL;

    snmp_row = ovsrec_system_first(idl);

    if (snmp_row != NULL) {

        vty_out(vty, "SNMP system information\n");
        vty_out(vty, "----------------------------\n");
        if (smap_get(&snmp_row->other_config, "system_description"))
            vty_out(vty, "System description : %s\n",
                    smap_get(&snmp_row->other_config, "system_description"));
        if (smap_get(&snmp_row->other_config, "system_location"))
            vty_out(vty, "System location : %s\n",
                    smap_get(&snmp_row->other_config, "system_location"));
        if (smap_get(&snmp_row->other_config, "system_contact"))
            vty_out(vty, "System contact : %s\n",
                    smap_get(&snmp_row->other_config, "system_contact"));
    }
    return CMD_SUCCESS;
}

void snmp_ovsdb_init() {
    /* Add tables/columns needed for SNMP. */
    ovsdb_idl_add_table(idl, &ovsrec_table_system);
    ovsdb_idl_add_column(idl, &ovsrec_system_col_snmp_communities);
    ovsdb_idl_add_column(idl, &ovsrec_system_col_other_config);
    ovsdb_idl_add_table(idl, &ovsrec_table_snmp_trap);
    ovsdb_idl_add_column(idl, &ovsrec_snmp_trap_col_receiver_address);
    ovsdb_idl_add_column(idl, &ovsrec_snmp_trap_col_receiver_udp_port);
    ovsdb_idl_add_column(idl, &ovsrec_snmp_trap_col_type);
    ovsdb_idl_add_column(idl, &ovsrec_snmp_trap_col_community_name);
    ovsdb_idl_add_column(idl, &ovsrec_snmp_trap_col_version);
}

void cli_pre_init() { snmp_ovsdb_init(idl); }

void cli_post_init() {
    vtysh_ret_val retval = e_vtysh_error;

    install_element(CONFIG_NODE, &snmp_server_host_trap_cmd);
    install_element(CONFIG_NODE, &snmp_server_host_trapv3_cmd);
    install_element(CONFIG_NODE, &snmp_server_host_inform_cmd);
    install_element(CONFIG_NODE, &snmp_server_host_informv3_cmd);
    install_element(CONFIG_NODE, &no_snmp_server_host_trap_cmd);
    install_element(CONFIG_NODE, &no_snmp_server_host_trapv3_cmd);
    install_element(CONFIG_NODE, &no_snmp_server_host_inform_cmd);
    install_element(CONFIG_NODE, &no_snmp_server_host_informv3_cmd);
    install_element(CONFIG_NODE, &snmp_master_agent_cmd);
    install_element(CONFIG_NODE, &no_snmp_master_agent_cmd);
    install_element(CONFIG_NODE, &snmp_system_description_cmd);
    install_element(CONFIG_NODE, &no_snmp_system_description_cmd);
    install_element(CONFIG_NODE, &snmp_system_contact_cmd);
    install_element(CONFIG_NODE, &no_snmp_system_contact_cmd);
    install_element(CONFIG_NODE, &snmp_system_location_cmd);
    install_element(CONFIG_NODE, &no_snmp_system_location_cmd);
    install_element(CONFIG_NODE, &snmp_community_cmd);
    install_element(CONFIG_NODE, &no_snmp_community_cmd);
    install_element(ENABLE_NODE, &show_snmp_agent_port_cmd);
    install_element(ENABLE_NODE, &show_snmp_community_cmd);
    install_element(ENABLE_NODE, &show_snmp_trap_cmd);
    install_element(ENABLE_NODE, &show_snmp_system_cmd);

    retval = install_show_run_config_subcontext(e_vtysh_config_context,
                                      e_vtysh_config_context_snmp,
                                      &vtysh_config_context_snmp_clientcallback,
                                      NULL, NULL);
    if(e_vtysh_ok != retval)
    {
        vty_out(vty, "e_vtysh_ok is error\n");
        vtysh_ovsdb_config_logmsg(VTYSH_OVSDB_CONFIG_ERR,
                            "config context unable to add SNMP  cliet callback");
        assert(0);
    }


}
