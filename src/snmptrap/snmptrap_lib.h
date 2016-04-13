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

typedef enum{
    GLOBAL_NAMESPACE,
    SWNS_NAMESPACE
} namespace_type;

int ops_create_snmp_engine_id(const struct ovsdb_idl *idl);

int ops_snmp_send_trap(const namespace_type, const struct ovsrec_snmp_trap *trap_row,netsnmp_session *session, netsnmp_session *ss, netsnmp_pdu *pdu, netsnmp_pdu *response, int inform);

int ops_add_snmpv3_user(const struct ovsdb_idl *idl, netsnmp_session *session, const struct ovsrec_snmp_trap* trap_row);

int ops_add_snmp_trap_community(netsnmp_session *sess, const struct ovsrec_snmp_trap *trap_row);

#endif /* SNMPTRAP_LIB_H */
