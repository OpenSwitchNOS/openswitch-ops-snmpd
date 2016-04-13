/*
 * (c) Copyright 2015 Hewlett Packard Enterprise Development LP
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */
/************************************************************************/

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
