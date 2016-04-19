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
generate_snmpd_conf(struct ovsrec_system* system_row)
{
    FILE* fp = fopen("/etc/snmp/snmpd.conf","w");

    const struct ovsrec_snmpv3_user *v3_user = NULL;

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

    const char* sys_description = smap_get(&system_row->other_config, "system-description");
    if(sys_description != NULL) {
        fprintf(fp, "sysDescr  %s\n", sys_description);
    }
    else {
        fprintf(fp, "sysDescr  \n\n");
    }

    const char* sys_contact = smap_get(&system_row->other_config, "system-contact");
    if(sys_description != NULL) {
        fprintf(fp, "sysContact  %s\n", sys_contact);
    }
    else {
        fprintf(fp, "sysContact  \n\n");
    }


    const char* sys_location = smap_get(&system_row->other_config, "system-location");
    if(sys_description != NULL) {
        fprintf(fp, "sysLocation  %s\n", sys_location);
    }
    else {
        fprintf(fp, "sysDescr  \n\n");
    }

    fprintf(fp, "sysServices   74 \n\n");


    fprintf(fp,"##############################################################\n");
    fprintf(fp,"#\n");
    fprintf(fp,"#  ACCESS CONTROL\n");
    fprintf(fp,"#\n\n\n");

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

    OVSREC_SNMPV3_USER_FOR_EACH(v3_user , idl) {
        if (strcmp (v3_user->auth_protocol, "None")) {
              fprintf(fp,"##############################################################\n");
              fprintf(fp,"# SNMPv3 AUTHENTICATION and ACCESS CONTROL\n");
              fprintf(fp,"#\n\n");
              if (strcmp(v3_user->priv_protocol, "None")){
                   fprintf(fp, "createUser  %s  %s  %s  %s\n",v3_user->user_name,
                           v3_user->auth_protocol, v3_user->auth_pass_phrase,
                           v3_user->priv_protocol );
                   fprintf(fp, "rouser %s  %s\n\n",v3_user->user_name, v3_user->priv_protocol);
              }
              else {
                   fprintf(fp, "createUser  %s  %s  %s \n\n",v3_user->user_name,
                           v3_user->auth_protocol, v3_user->auth_pass_phrase);
                   fprintf(fp, "rouser %s\n\n",v3_user->user_name);
              }
         fprintf(fp,"##############################################################\n\n\n");
         }
    }

    fprintf(fp, "master agentx\n");
    fprintf(fp, "agentXSocket /var/agentx/master\n");

    fclose(fp);

    restart_snmpd();
    return;
}

static unsigned int system_seq_no = 0;

static void
snmp_conf_gen(struct ovsdb_idl* idl){
    const struct ovsrec_system *system_row = NULL;
    system_row = ovsrec_system_first(idl);
    if(system_row == NULL){
        VLOG_ERR("Couldn't find system row");
        return;
    }

    unsigned int temp_seq_no = ovsrec_system_get_seqno(idl);
    if(temp_seq_no != system_seq_no){
        system_seq_no = temp_seq_no;
        generate_snmpd_conf((struct ovsrec_system*)system_row);
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
