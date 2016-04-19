#include "dirs.h"
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
generate_snmpd_conf(const struct ovsrec_system* system_row)
{
    char snmp_engine_id[SNMP_ENGINEID_STR_LEN] = {0};
    FILE* fp = fopen("/etc/snmp/snmpd.conf","w");
    const struct ovsrec_snmpv3_user *v3_user_row = NULL;

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

    OVSREC_SNMPV3_USER_FOR_EACH(v3_user_row , idl) {
        if (strcmp (v3_user_row->auth_protocol, "none") != 0) {
             if(strcmp(v3_user_row->auth_protocol,"md5") == 0){
                 auth = "MD5";
             }
             else {
                 auth = "SHA";
             }
             if (strcmp(v3_user_row->priv_protocol, "none") != 0){
                 if (strcmp(v3_user_row->priv_protocol,"aes") == 0){
                     priv = "AES";
                 }
                 else {
                     priv = "DES";
                 }
                 fprintf(fp, "createUser  %s  %s  %s  %s %s\n",v3_user_row->user_name,
                         auth, v3_user_row->auth_pass_phrase,
                         priv, v3_user_row->priv_pass_phrase );
                 fprintf(fp, "rouser %s  %s\n",v3_user_row->user_name, "priv");
             }
             else {
                 fprintf(fp, "createUser  %s  %s  %s \n",v3_user_row->user_name,
                         auth, v3_user_row->auth_pass_phrase);
                 fprintf(fp, "rouser %s %s\n",v3_user_row->user_name, "auth");
             }
        }
        else {
            fprintf(fp, "createUser  %s \n",v3_user_row->user_name);
            fprintf(fp, "rouser %s\n",v3_user_row->user_name);
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


static void
snmp_reconfigure(struct ovsdb_idl* idl){
    const struct ovsrec_system *system_row = NULL;
    const struct ovsrec_snmpv3_user *v3_user_row = NULL;
    bool modified = false;

    system_row = ovsrec_system_first(idl);
    if(system_row == NULL){
        VLOG_ERR("Couldn't find system row");
        return;
    }

    v3_user_row = ovsrec_snmpv3_user_first(idl);
    if(v3_user_row != NULL) {
        if((OVSREC_IDL_ANY_TABLE_ROWS_MODIFIED(v3_user_row, snmp_idl_seqno)) ||
           (OVSREC_IDL_ANY_TABLE_ROWS_DELETED(v3_user_row, snmp_idl_seqno))  ||
           (OVSREC_IDL_ANY_TABLE_ROWS_INSERTED(v3_user_row, snmp_idl_seqno)) ) {
            modified = true ;
        }
    }

    if(OVSREC_IDL_IS_COLUMN_MODIFIED(ovsrec_system_col_snmp_communities,
                                       snmp_idl_seqno)) {
        modified = true;
    }
    if(OVSREC_IDL_IS_COLUMN_MODIFIED(ovsrec_system_col_other_config,
                                       snmp_idl_seqno)) {
        modified = true;
    }

    if(modified) {
        generate_snmpd_conf(system_row);
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

    unsigned int temp_seq_no = ovsdb_idl_get_seqno(idl);

    if (temp_seq_no != snmp_idl_seqno) {
        snmp_reconfigure(idl);
        snmp_idl_seqno = ovsdb_idl_get_seqno(idl);
    }
}

static void
snmp_wait(struct ovsdb_idl *idl)
{
   ovsdb_idl_wait (idl);
}


void *
snmp_ovsdb_main_thread(void *arg)
{
    /* Detach thread to avoid memory leak upon exit. */
    pthread_detach(pthread_self());

    snmp_exit = false;
    while (!snmp_exit) {
        SNMP_OVSDB_LOCK;

        /* This function updates the Cache by running
           ovsdb_idl_run. */
        snmp_run(idl);

        /* This function adds the file descriptor for the
           DB to monitor using poll_fd_wait. */
        snmp_wait(idl);

        SNMP_OVSDB_UNLOCK;
        if (snmp_exit) {
            poll_immediate_wake();
        } else {
            poll_block();
        }
    }
    VLOG_ERR("Exiting OVSDB main thread");
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
