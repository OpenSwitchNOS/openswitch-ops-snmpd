#include "dirs.h"
#include "daemon.h"
#include "ovsdb-idl.h"
#include "openvswitch/vlog.h"
#include "openswitch-idl.h"
#include <poll-loop.h>
#include <pthread.h>
#include "snmp_ovsdb_if.h"
#include "snmp_utils.h"
#include "snmp_plugins.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "vswitch-idl.h"
#include <unixctl.h>

struct ovsdb_idl *idl;
unsigned int snmp_idl_seqno;
char *ovsdb_socket;
bool snmp_exit = false;

VLOG_DEFINE_THIS_MODULE (snmp_ovsdb_if);

static bool system_configured = false;

static inline bool
ops_snmpd_system_is_configured(void)
{
    const struct ovsrec_system *sysrow = NULL;

    if (system_configured) {
        return true;
    }
    sysrow = ovsrec_system_first(idl);

    if (sysrow && sysrow->cur_cfg > INT64_C(0)) {
        VLOG_DBG("System now configured (cur_cfg=%" PRId64 ").",
                 sysrow->cur_cfg);
        system_configured = true;
        return system_configured;
    }
    return false;
}/* ops_system_is_configured */

static void
restart_snmpd()
{
    int ret = system("systemctl restart snmpd");

    if(ret < 0)
    {
        VLOG_ERR("failed to restart snmpd");
    }

    return;
}

static void
generate_snmpd_conf(const struct ovsrec_system* system_row,
                    const struct ovsrec_snmpv3_user *v3_user)
{
    char snmp_engine_id[SNMP_ENGINEID_STR_LEN] = {0};
    FILE* fp = fopen("/etc/snmp/snmpd.conf","w");

    if(fp == NULL)
    {
        VLOG_ERR("Couldn't open snmpd.conf");
        return;
    }

    fprintf(fp,"############################################################\n");
    fprintf(fp,"#\n");
    fprintf(fp,"#  AGENT BEHAVIOUR\n");
    fprintf(fp,"#\n\n\n");

    const char* agent_port = smap_get(&system_row->other_config, "snmp_agent_port");
    if(agent_port != NULL) {
        fprintf(fp, "agentAddress udp:%s\n\n", agent_port);
    }
    else {
        fprintf(fp, "agentAddress udp:161\n\n");
    }

    fprintf(fp,"############################################################\n");
    fprintf(fp,"#\n");
    fprintf(fp,"#  SYSTEM INFORMATION\n");
    fprintf(fp,"#\n\n\n");

    const char* sys_description = smap_get(&system_row->other_config, "system_description");
    if(sys_description != NULL) {
        fprintf(fp, "sysDescr  %s\n", sys_description);
    }
    else {
        fprintf(fp, "sysDescr  \"\" \n");
    }

    const char* sys_contact = smap_get(&system_row->other_config, "system_contact");
    if(sys_contact != NULL) {
        fprintf(fp, "sysContact  %s\n", sys_contact);
    }
    else {
        fprintf(fp, "sysContact  \"\"\n");
    }


    const char* sys_location = smap_get(&system_row->other_config, "system_location");
    if(sys_location != NULL) {
        fprintf(fp, "sysLocation  %s\n", sys_location);
    }
    else {
        fprintf(fp, "sysLocation \"\" \n");
    }

    fprintf(fp, "sysObjectID    1.3.6.1.4.1.47267\n" );

    fprintf(fp, "sysServices   74 \n");


    fprintf(fp,"##############################################################\n");
    fprintf(fp,"#\n");
    fprintf(fp,"#  ACCESS CONTROL\n");
    fprintf(fp,"#\n\n");

    size_t agent_len = system_row->n_snmp_communities;
    int i = 0;
    char** temp_snmp_comms = system_row->snmp_communities;
    if (temp_snmp_comms == NULL) {
        fprintf(fp, "rocommunity public\n");
    }
    else {
        for (i = 0; i < agent_len; i++) {
            fprintf(fp, "rocommunity %s\n", *temp_snmp_comms);
            temp_snmp_comms++;
        }
    }

    fprintf(fp,"##############################################################\n");
    fprintf(fp,"# SNMPv3 AUTHENTICATION and ACCESS CONTROL\n");
    fprintf(fp,"#\n\n");

    char *auth = NULL;
    char *priv = NULL;

    if (v3_user) {
         VLOG_DBG("v3 user exists\n");
         OVSREC_SNMPV3_USER_FOR_EACH(v3_user , idl) {
             if (strcmp (v3_user->auth_protocol, "none") != 0) {
                 if(strcmp(v3_user->auth_protocol,"md5") == 0){
                     auth = "MD5";
                 }
                 else {
                     auth = "SHA";
                 }
                 if (strcmp(v3_user->priv_protocol, "none") != 0){
                     if (strcmp(v3_user->priv_protocol,"aes") == 0){
                         priv = "AES";
                     }
                     else {
                         priv = "DES";
                     }
                     fprintf(fp, "createUser  %s  %s  %s  %s %s\n",v3_user->user_name,
                             auth, v3_user->auth_pass_phrase,
                             priv, v3_user->priv_pass_phrase );
                     fprintf(fp, "rouser %s  %s\n",v3_user->user_name, "priv");
                 }
                 else {
                     fprintf(fp, "createUser  %s  %s  %s \n",v3_user->user_name,
                             auth, v3_user->auth_pass_phrase);
                     fprintf(fp, "rouser %s %s\n",v3_user->user_name, "auth");
                 }
            }
            else {
                fprintf(fp, "createUser  %s \n",v3_user->user_name);
                fprintf(fp, "rouser %s\n",v3_user->user_name);
            }
        }
    }

    fprintf(fp,"##############################################################\n");
    fprintf(fp,"# SNMPv3 Configuration\n");
    fprintf(fp,"#\n\n");

    strncpy(snmp_engine_id, "0x0000b8a303", SNMP_ENGINEID_STR_LEN);
    const char *c = system_row->system_mac;
    int tmp_idx = 12;
    while (*c != '\0') {
        if (*c != ':') {
            *(snmp_engine_id + tmp_idx) = *c;
            tmp_idx++;
        }
        c++;
    }
    *(snmp_engine_id + tmp_idx) = '\0';

    fprintf(fp, "engineID   %s\n",snmp_engine_id);
    fprintf(fp,"##############################################################\n\n\n");

    fprintf(fp, "master agentx\n");
    fprintf(fp, "agentXSocket /var/agentx/master\n");

    fclose(fp);

    restart_snmpd();
    return;
}

static unsigned int system_seq_no = 0;
static unsigned int snmpv3_user_seq_no = 0;

static void
snmp_conf_gen(struct ovsdb_idl* idl){
    const struct ovsrec_system *system_row = NULL;
    const struct ovsrec_snmpv3_user *v3_user = NULL;

    system_row = ovsrec_system_first(idl);
    if(system_row == NULL){
        VLOG_ERR("Couldn't find system row");
        return;
    }
    v3_user = ovsrec_snmpv3_user_first(idl);

    unsigned int temp_sys_seq_no = ovsrec_system_get_seqno(idl);
    unsigned int temp_snmpv3_seq_no = ovsrec_snmpv3_user_get_seqno(idl);
    if((temp_sys_seq_no != system_seq_no) ||
       (temp_snmpv3_seq_no != snmpv3_user_seq_no)) {
        system_seq_no = temp_sys_seq_no;
        snmpv3_user_seq_no = temp_snmpv3_seq_no;
        generate_snmpd_conf(system_row, v3_user);
    }

    return;
}

static void
snmp_run(struct ovsdb_idl *idl)
{
    ovsdb_idl_run(idl);
    /* Nothing to do until system has been configured, i.e., cur_cfg > 0. */
    if(!ops_snmpd_system_is_configured()) {
        VLOG_DBG("checking for system configured");
        return;
    }

    snmp_conf_gen(idl);
}

static void
snmp_wait(struct ovsdb_idl *idl)
{
   ovsdb_idl_wait (idl);
}


void *
snmp_ovsdb_main_thread(void *arg)
{
    struct unixctl_server *appctl;

    /* Detach thread to avoid memory leak upon exit. */
    pthread_detach(pthread_self());

    appctl = (struct unixctl_server *)arg;

    snmp_exit = false;
    while (!snmp_exit) {
        SNMP_OVSDB_LOCK;

        /* This function updates the Cache by running
           ovsdb_idl_run. */
        snmp_run(idl);
        unixctl_server_run(appctl);

        /* This function adds the file descriptor for the
           DB to monitor using poll_fd_wait. */
        snmp_wait(idl);
        unixctl_server_wait(appctl);

        SNMP_OVSDB_UNLOCK;
        if (snmp_exit) {
            poll_immediate_wake();
        } else {
            poll_block();
            if (should_service_stop()) {
                snmp_exit = true;
            }
        }
    }
    VLOG_ERR("Exiting OVSDB main thread");
    unixctl_server_destroy(appctl);
    return NULL;
}

void
snmpd_ovsdb_init()
{
    long int pid;
    char *idl_lock;

    ovsdb_socket = xasprintf("unix:%s/db.sock", ovs_rundir());
    ovsrec_init();

    idl = ovsdb_idl_create(ovsdb_socket, &ovsrec_idl_class, false , true);
    pid = getpid();
    idl_lock = xasprintf("ops_snmp_%ld", pid);
    ovsdb_idl_set_lock(idl, idl_lock);
    free(idl_lock);

    snmp_idl_seqno = ovsdb_idl_get_seqno(idl);
    ovsdb_idl_enable_reconnect(idl);

    ovsdb_idl_add_table(idl, &ovsrec_table_system);
    ovsdb_idl_add_column(idl, &ovsrec_system_col_cur_cfg);
    ovsdb_idl_add_column(idl, &ovsrec_system_col_snmp_communities);
    ovsdb_idl_add_column(idl, &ovsrec_system_col_other_config);
    ovsdb_idl_add_column(idl, &ovsrec_system_col_system_mac);
    ovsdb_idl_add_table(idl, &ovsrec_table_snmpv3_user);
    ovsdb_idl_add_column(idl, &ovsrec_snmpv3_user_col_user_name);
    ovsdb_idl_add_column(idl, &ovsrec_snmpv3_user_col_auth_protocol);
    ovsdb_idl_add_column(idl, &ovsrec_snmpv3_user_col_auth_pass_phrase);
    ovsdb_idl_add_column(idl, &ovsrec_snmpv3_user_col_priv_protocol);
    ovsdb_idl_add_column(idl, &ovsrec_snmpv3_user_col_priv_pass_phrase);

    free(ovsdb_socket);
    VLOG_DBG("OPS snmp OVSDB Integration has been initialized");
    return;
}
