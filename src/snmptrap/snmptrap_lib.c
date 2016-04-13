#define _GNU_SOURCE
#include <fcntl.h>
#include <sched.h>
#include <sys/types.h>
#include <unistd.h>

#include "ovsdb-idl.h"
#include "snmptrap_lib.h"
#include "vswitch-idl.h"
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-features.h>
#include <net-snmp/net-snmp-includes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "openswitch-idl.h"
#include "openvswitch/vlog.h"
#include "smap.h"

VLOG_DEFINE_THIS_MODULE(ops_snmptrap_lib);

static char global_snmp_engine_id[MAX_SNMP_ENGINEID_STR_LEN] = {0};
static char global_trap_community[MAX_COMMUNITY_STR_LEN] = {0};

int create_snmp_engine_id(const struct ovsdb_idl *idl) {
    const struct ovsrec_system *sys_row = ovsrec_system_first(idl);
    if (sys_row == NULL) {
        VLOG_ERR("Failed to find a system row");
        return -1;
    }

    strncpy(global_snmp_engine_id, "0x0000b8a303", MAX_SNMP_ENGINEID_STR_LEN);
    const char *c = sys_row->system_mac;
    int tmp_idx = 12;
    while (*c != '\0') {
        if (*c != ':') {
            *(global_snmp_engine_id + tmp_idx) = *c;
            tmp_idx++;
        }
        c++;
    }
    if (tmp_idx >= MAX_SNMP_ENGINEID_STR_LEN) {
        VLOG_ERR("SNMP engine id corruption");
        return -1;
    }
    *(global_snmp_engine_id + tmp_idx) = '\0';
    return 0;
}

static const struct ovsrec_snmpv3_user *
find_snmpv3_user(const struct ovsdb_idl *idl, const char *username) {
    const struct ovsrec_snmpv3_user *user_row = NULL;

    OVSREC_SNMPV3_USER_FOR_EACH(user_row, idl) {
        if (strcmp(user_row->user_name, username) == 0) {
            break;
        }
    }
    return user_row;
}

static int add_auth_priv_info(netsnmp_session *session,
                              const struct ovsrec_snmpv3_user *user_row) {
    int auth_flag = 0, priv_flag = 0;

    const char *auth_protocol = user_row->auth_protocol;
    const char *priv_protocol = user_row->priv_protocol;
    const char *auth_pass_phrase = user_row->auth_pass_phrase;
    const char *priv_pass_phrase = user_row->priv_pass_phrase;

    if (strcmp(auth_protocol, OVSREC_SNMPV3_USER_AUTH_PROTOCOL_NONE) != 0) {
        auth_flag = 1;
    }
    if (strcmp(priv_protocol, OVSREC_SNMPV3_USER_PRIV_PROTOCOL_NONE) != 0) {
        priv_flag = 1;
    }

    if (auth_flag == 0 && priv_flag == 0) {
        session->securityLevel = SNMP_SEC_LEVEL_NOAUTH;
    } else if (auth_flag == 1 && priv_flag == 0) {
        session->securityLevel = SNMP_SEC_LEVEL_AUTHNOPRIV;
    } else if (auth_flag == 1 && priv_flag == 1) {
        session->securityLevel = SNMP_SEC_LEVEL_AUTHPRIV;
    } else {
        VLOG_ERR("Invalid option for snmpv3 security level");
        return -1;
    }

    if (auth_flag) {
        if (strcmp(auth_protocol, OVSREC_SNMPV3_USER_AUTH_PROTOCOL_MD5) == 0) {
            session->securityAuthProto = usmHMACMD5AuthProtocol;
            session->securityAuthProtoLen = USM_AUTH_PROTO_MD5_LEN;
        } else if (strcmp(auth_protocol,
                          OVSREC_SNMPV3_USER_AUTH_PROTOCOL_SHA) == 0) {
            session->securityAuthProto = usmHMACSHA1AuthProtocol;
            session->securityAuthProtoLen = USM_AUTH_PROTO_SHA_LEN;
        } else {
            VLOG_ERR("Invalid auth protocol for snmpv3 user: %s",
                     auth_protocol);
            return -1;
        }
    }

    if (priv_flag) {
        if (strcmp(priv_protocol, OVSREC_SNMPV3_USER_PRIV_PROTOCOL_DES) == 0) {
            session->securityPrivProto = usmDESPrivProtocol;
            session->securityPrivProtoLen = USM_PRIV_PROTO_DES_LEN;
        } else if (strcmp(priv_protocol,
                          OVSREC_SNMPV3_USER_PRIV_PROTOCOL_AES) == 0) {
            session->securityPrivProto = usmAESPrivProtocol;
            session->securityPrivProtoLen = USM_PRIV_PROTO_AES_LEN;
        } else {
            VLOG_ERR("Invalid priv protcol for snmpv3 user: %s", priv_protocol);
            return -1;
        }
    }

    if (auth_flag) {
        session->securityAuthKeyLen = USM_AUTH_KU_LEN;
        if (session->securityAuthProto == NULL) {
            const oid *def =
                get_default_authtype(&session->securityAuthProtoLen);
            session->securityAuthProto =
                snmp_duplicate_objid(def, session->securityAuthProtoLen);
        }
        if (session->securityAuthProto == NULL) {
            session->securityAuthProto = snmp_duplicate_objid(
                usmHMACMD5AuthProtocol, USM_AUTH_PROTO_MD5_LEN);
            session->securityAuthProtoLen = USM_AUTH_PROTO_MD5_LEN;
        }
        if (generate_Ku(session->securityAuthProto,
                        session->securityAuthProtoLen,
                        (u_char *)auth_pass_phrase, strlen(auth_pass_phrase),
                        session->securityAuthKey,
                        &session->securityAuthKeyLen) != SNMPERR_SUCCESS) {
            VLOG_ERR("Error generating a key (Ku) from the supplied "
                     "authentication pass phrase:%s",
                     auth_pass_phrase);
            return -1;
        }
    }

    if (priv_flag) {
        session->securityPrivKeyLen = USM_PRIV_KU_LEN;
        if (session->securityPrivProto == NULL) {
            const oid *def =
                get_default_privtype(&session->securityPrivProtoLen);
            session->securityPrivProto =
                snmp_duplicate_objid(def, session->securityPrivProtoLen);
        }
        if (session->securityPrivProto == NULL) {
            session->securityPrivProto = snmp_duplicate_objid(
                usmDESPrivProtocol, USM_PRIV_PROTO_DES_LEN);
            session->securityPrivProtoLen = USM_PRIV_PROTO_DES_LEN;
        }
        if (generate_Ku(session->securityAuthProto,
                        session->securityAuthProtoLen,
                        (u_char *)priv_pass_phrase, strlen(priv_pass_phrase),
                        session->securityPrivKey,
                        &session->securityPrivKeyLen) != SNMPERR_SUCCESS) {
            VLOG_ERR("Error generating a key (Ku) from the supplied privacy "
                     "pass phrase:%s",
                     priv_pass_phrase);
            return -1;
        }
    }

    return 0;
}

int ops_add_snmpv3_user(const struct ovsdb_idl *idl, netsnmp_session *session,
                        const struct ovsrec_snmp_trap *trap_row) {
    if (global_snmp_engine_id[0] == '\0') {
        int err = create_snmp_engine_id(idl);
        if (err < 0) {
            VLOG_ERR("Couldn't create snmp engine id");
            return -1;
        }
    }

    size_t ebuf_len = 32, eout_len = 0;
    u_char *ebuf = (u_char *)malloc(ebuf_len);

    if (ebuf == NULL) {
        VLOG_ERR("malloc failure processing engine id.");
        return -1;
    }
    if (!snmp_hex_to_binary(&ebuf, &ebuf_len, &eout_len, 1,
                            global_snmp_engine_id)) {
        VLOG_ERR("Bad engine ID value: %s", global_snmp_engine_id);
        free(ebuf);
        return -1;
    }
    if ((eout_len < 5) || (eout_len > 32)) {
        VLOG_ERR("Invalid engine ID value: %s", global_snmp_engine_id);
        free(ebuf);
        return -1;
    }
    session->securityEngineID = ebuf;
    session->securityEngineIDLen = eout_len;

    const struct ovsrec_snmpv3_user *user_row =
        find_snmpv3_user(idl, trap_row->community_name);
    if (user_row == NULL) {
        VLOG_ERR("Coudln't find snmpv3 user");
        return -1;
    }

    if (add_auth_priv_info(session, user_row) < 0) {
        VLOG_ERR("Failed in add_auth_priv_info");
        return -1;
    }

    session->securityName = strdup(trap_row->community_name);
    if (session->securityName == NULL) {
        VLOG_ERR("Failed to add securityName");
        return -1;
    }
    session->securityNameLen = strlen(session->securityName);

    return 0;
}

int ops_add_snmp_trap_community(netsnmp_session *sess,
                                const struct ovsrec_snmp_trap *trap_row) {
    strncpy(global_trap_community, trap_row->community_name,
            MAX_COMMUNITY_STR_LEN);
    if (global_trap_community[0] == '\0') {
        strncpy(global_trap_community, "public", MAX_COMMUNITY_STR_LEN);
    }

    sess->community = (u_char *)global_trap_community;
    sess->community_len = strlen(global_trap_community);
    return 0;
}

int ops_snmp_send_trap(const namespace_type nm_type, const struct ovsrec_snmp_trap *trap_row,
                       netsnmp_session *session, netsnmp_session *ss,
                       netsnmp_pdu *pdu, netsnmp_pdu *response, int inform) {
    VLOG_DBG("Entered ops_snmp_send_trap: %s", trap_row->community_name);
    int status = 0;
    int globalns_fd = -1, swns_fd = -1;
    if(nm_type == SWNS_NAMESPACE){
    globalns_fd = open("/proc/1/ns/net", O_RDONLY);
    if (globalns_fd < 0) {
        VLOG_ERR("Failed to open global namespace fd");
        return -1;
    }
    swns_fd = open("/var/run/netns/swns", O_RDONLY);
    if (swns_fd < 0) {
        VLOG_ERR("Failed to open swns namespace fd");
        return -1;
    }
    if (setns(globalns_fd, 0) < 0) {
        VLOG_ERR("Failed to switch to global namespace");
        return -1;
    }
}
    char temp_peername[MAX_PEERNAME_STR_LEN];
    char temp_port[MAX_PORT_STR_LEN];
    snprintf(temp_port, 10, "%d", (int)trap_row->receiver_udp_port);
    snprintf(temp_peername, MAX_PEERNAME_STR_LEN, "%s:%s",
             trap_row->receiver_address, temp_port);
    session->peername = temp_peername;
    ss = snmp_add(session,
                  netsnmp_transport_open_client("snmptrap", session->peername),
                  NULL, NULL);
    if (ss == NULL) {
        VLOG_ERR("Failed in snmp_add for %s", session->peername);
        goto namespace_cleanup;
    }

    if (inform) {
        status = snmp_synch_response(ss, pdu, &response);
    } else {
        status = snmp_send(ss, pdu) == 0;
    }

namespace_cleanup:
    if(nm_type == SWNS_NAMESPACE){
    if (setns(swns_fd, 0) == -1) {
        VLOG_ERR("Failed to switch back to swns");
        status = -1;
    }
}
    VLOG_DBG("Exiting ops_snmp_send_trap with status: %d", status);
    return status;
}
