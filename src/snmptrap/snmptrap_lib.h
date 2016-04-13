#ifndef SNMPTRAP_LIB_H
#define SNMPTRAP_LIB_H

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/net-snmp-features.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "vswitch-idl.h"

#define MAX_PORT_STR_LEN 10
#define MAX_UPTIME_STR_LEN 20
#define MAX_COMMUNITY_STR_LEN 64
#define MAX_PEERNAME_STR_LEN 128
#define MAX_SNMP_ENGINEID_STR_LEN 32

int ops_snmp_send_trap(struct ovsdb_idl* idl, netsnmp_pdu *pdu);

#endif /* SNMPTRAP_LIB_H */
