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
snmp_run(struct ovsdb_idl *idl)
{
    ovsdb_idl_run(idl);
    /* Nothing to do until system has been configured, i.e., cur_cfg > 0. */
    if(!ops_snmpd_system_is_configured()) {
        VLOG_DBG("checking for system configured");
        return;
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

    free(ovsdb_socket);
    VLOG_DBG("OPS snmp OVSDB Integration has been initialized");
    return;
}
