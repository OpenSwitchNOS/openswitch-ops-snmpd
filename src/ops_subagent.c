#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-features.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#ifdef OPS
#include "snmp_ovsdb_if.h"
#include "openvswitch/vlog.h"
#include "snmp_utils.h"
#include "snmp_plugins.h"
#include <pthread.h>
#endif /* OPS */

#include <signal.h>

VLOG_DEFINE_THIS_MODULE(snmp_subagent);

#ifdef OPS
#define FEATURE_SNMP_PATH "/usr/lib/snmp/plugins"
#endif /* OPS */

static int keep_running;


RETSIGTYPE
stop_server(int a) {
    keep_running = 0;
}

int
main (int argc, char **argv) {
  int agentx_subagent=1; /* change this if you want to be a SNMP master agent */
  int background = 0; /* change this if you want to run in the background */
  int syslog = 0; /* change this if you want to use syslog */

  int ret = 0;
  pthread_t snmp_ovsdb_if_thread;
  vlog_set_verbosity("SYSLOG:DBG");

  /* print log errors to syslog or stderr */
  if (syslog)
    snmp_enable_calllog();
  else
    snmp_enable_stderrlog();

  /* we're an agentx subagent? */
  if (agentx_subagent) {
    /* make us a agentx client. */
    netsnmp_ds_set_boolean(NETSNMP_DS_APPLICATION_ID, NETSNMP_DS_AGENT_ROLE, 1);
  }

  /* run in background, if requested */
  if (background && netsnmp_daemonize(1, !syslog))
      exit(1);

  /* initialize tcpip, if necessary */
  SOCK_STARTUP;

  /* initialize the agent library */
  init_agent("ops-snmpd");

  /* initialize the OVSDB*/
  snmpd_ovsdb_init();

  /*  plugins_init is called after OVSDB socket creation
   *  before monitor thread creation.
   */
  plugins_snmp_init(FEATURE_SNMP_PATH);

  /* create a thread to poll OVSDB */
  ret = pthread_create( &snmp_ovsdb_if_thread,
                       (pthread_attr_t *)NULL,
                        snmp_ovsdb_main_thread,
                        NULL );
  if (ret)
  {
     VLOG_ERR("Failed to create the snmp poll thread %d",ret);
     exit(-ret);
  }

  /*initialize vacm/usm access control  */
  /*if (!agentx_subagent) {
      init_vacm_vars();
      init_usmUser();
  }*/

  /* example-demon will be used to read example-demon.conf files. */
  init_snmp("ops-snmpd");

  /* If we're going to be a snmp master agent, initial the ports */
  if (!agentx_subagent)
    init_master_agent();  /* open the port to listen on (defaults to udp:161) */

  /* In case we recevie a request to stop (kill -TERM or kill -INT) */
  keep_running = 1;
  signal(SIGTERM, stop_server);
  signal(SIGINT, stop_server);

  VLOG_DBG("subagent is up and running.");

  /* your main loop here... */
  while(keep_running) {
    /* if you use select(), see snmp_select_info() in snmp_api(3) */
    /*     --- OR ---  */
    agent_check_and_process(1); /* 0 == don't block */
  }

  VLOG_DBG("calling plugin destroy from subagent main");
  plugins_snmp_destroy();
  /* at shutdown time */
  snmp_shutdown("ops-snmpd");
  SOCK_CLEANUP;
  return 0;
}
