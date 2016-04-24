#include <errno.h>
#include <getopt.h>
#include <stdlib.h>

#include <command-line.h>
#include <daemon.h>
#include <dirs.h>
#include <fatal-signal.h>
#include <unixctl.h>
#include <util.h>

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-features.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#ifdef OPS
#include "snmp_ovsdb_if.h"
#include "openvswitch/vlog.h"
#include <vswitch-idl.h>
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

bool exiting = false;
//static unixctl_cb_func ops_snmpd_exit;

RETSIGTYPE
stop_server(int a) {
    keep_running = 0;
}

static char *parse_options(int argc, char *argv[], char **unixctl_pathp){
    enum {
        OPT_UNIXCTL = UCHAR_MAX +1,
        VLOG_OPTION_ENUMS,
        DAEMON_OPTION_ENUMS,
    };
    static const struct option long_options[] = {
        {"help", no_argument, NULL, 'h'},
        {"unixctl", required_argument, NULL, OPT_UNIXCTL},
        DAEMON_LONG_OPTIONS,
        VLOG_LONG_OPTIONS,
        {NULL, 0, NULL, 0},
    };
    char *short_options = long_options_to_short_options(long_options);

    for(;;){
        int c;
        c = getopt_long(argc, argv, short_options, long_options, NULL);
        if(c == -1){
            break;
        }

        switch (c){
        case 'h':

        case OPT_UNIXCTL:
            *unixctl_pathp = optarg;
            break;

        VLOG_OPTION_HANDLERS
        DAEMON_OPTION_HANDLERS

        case '?':
            exit(EXIT_FAILURE);

        default:
            exit(EXIT_FAILURE);
        }
    }
    free(short_options);

    argc -= optind;
    argv += optind;

    switch(argc){
        case 0:
            return xasprintf("unix:%s/db.sock", ovs_rundir());

        case 1:
            return xstrdup(argv[0]);

        default:
            VLOG_FATAL("at most one non-options argument accepted;");
    }
}

void ops_snmpd_exit(struct unixctl_conn *conn, int argc OVS_UNUSED,
                    const char *argv[] OVS_UNUSED, void *exiting_)
{
    bool *exiting = exiting_;
    *exiting = true;
    unixctl_command_reply(conn, NULL);
}

int
main (int argc, char **argv) {
  char *appctl_path = NULL;
  struct unixctl_server *appctl;
  char *ovsdb_sock = NULL;
  int retval;
  //sigset_t sigset;
  //int signum;

  set_program_name(argv[0]);
  proctitle_init(argc, argv);
  fatal_ignore_sigpipe();

  ovsdb_sock = parse_options(argc, argv, &appctl_path);

  ovsrec_init();

  daemonize_start();

  retval = unixctl_server_create(appctl_path, &appctl);
  if(retval){
    exit(EXIT_FAILURE);
  }

  unixctl_command_register("exit", "", 0, 0, ops_snmpd_exit, &exiting);

  int agentx_subagent=1; /* change this if you want to be a SNMP master agent */
  int background = 0; /* change this if you want to run in the background */
  int syslog = 0; /* change this if you want to use syslog */

  int ret = 0;
  pthread_t snmp_ovsdb_if_thread;

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
                        (void *)appctl );
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

  free(ovsdb_sock);
  daemonize_complete();
  vlog_enable_async();

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
