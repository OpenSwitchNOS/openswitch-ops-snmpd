/* SNMP daemon client callback resigitration source files.
 *
 * Copyright (C) 2016 Hewlett Packard Enterprise Development LP.
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 * File: vtysh_ovsdb_snmp_context.c
 *
 * Purpose: Source for registering SNMP sub-context callback with
 *          global config context.
 */

#include "vtysh/vty.h"
#include "openswitch-idl.h"
#include "snmp_vty.h"
#include "vswitch-idl.h"
#include "vtysh/utils/system_vtysh_utils.h"
#include "vtysh/vector.h"
#include "vtysh/vtysh_ovsdb_config.h"
#include "vtysh/vtysh_ovsdb_if.h"
#include "vtysh_ovsdb_snmp_context.h"

extern struct ovsdb_idl *idl;
/***************************************************************************
 * @function      : vtysh_config_context_snmp_clientcallback
 * @detail    : client callback routine for SNMP configuration
 * @parame[in]
 *   p_private: Void pointer for holding address of vtysh_ovsdb_cbmsg_ptr
 *          structure object
 * @return : e_vtysh_ok on success
 ***************************************************************************/
vtysh_ret_val vtysh_config_context_snmp_clientcallback(void *p_private) {
    vtysh_ovsdb_cbmsg_ptr p_msg = (vtysh_ovsdb_cbmsg *)p_private;
    const struct ovsrec_system *psnmp_row = NULL;
    const struct ovsrec_snmp_trap *psnmp_trap_row = NULL;
    const struct ovsrec_snmpv3_user *v3user_row = NULL;
    size_t i = 0;
    const char *snmp_port = NULL;
    const char *snmp_sys_desc = NULL;
    const char *snmp_sys_loc = NULL;
    const char *snmp_sys_contact = NULL;
    const char *snmp_trap_community = NULL;
    bool trap_rcvr_port_not_default = false;
    bool community_not_default =  false;
    bool is_v3 = false;

    psnmp_row = ovsrec_system_first(idl);
    if (psnmp_row != NULL) {
        snmp_port = smap_get(&psnmp_row->other_config, "snmp_agent_port");
        snmp_sys_desc =
            smap_get(&psnmp_row->other_config, "system_description");
        snmp_sys_loc = smap_get(&psnmp_row->other_config, "system_location");
        snmp_sys_contact = smap_get(&psnmp_row->other_config, "system_contact");

        if (snmp_port != NULL) {
            vtysh_ovsdb_cli_print(p_msg, "%s %s", "snmp-server agent-port",
                                      snmp_port);
        }
        if (snmp_sys_desc != NULL) {
            vtysh_ovsdb_cli_print(p_msg, "%s %s",
                                  "snmp-server system-description",
                                  snmp_sys_desc);
        }
        if (snmp_sys_loc != NULL) {
                vtysh_ovsdb_cli_print(p_msg, "%s %s",
                                      "snmp-server system-location",
                                      snmp_sys_loc);
        }
        if (snmp_sys_contact != NULL) {
                vtysh_ovsdb_cli_print(p_msg, "%s %s",
                                      "snmp-server system-contact",
                                      snmp_sys_contact);
        }

        while (i < psnmp_row->n_snmp_communities) {
            vtysh_ovsdb_cli_print(p_msg, "%s %s", "snmp-server community",
                                  psnmp_row->snmp_communities[i]);
            i++;
        }
    }

    OVSREC_SNMP_TRAP_FOR_EACH(psnmp_trap_row, idl) {
        if (psnmp_trap_row != NULL) {

            snmp_trap_community = psnmp_trap_row->community_name;

            if (strcmp(snmp_trap_community, DEFAULT_COMMUNITY_TYPE) != 0)
                community_not_default = true;

            if ((int)psnmp_trap_row->receiver_udp_port != DEFAULT_TRAP_RECEIVER_UDP_PORT)
                trap_rcvr_port_not_default = true;

            if (strcmp(psnmp_trap_row->version, "v3") == 0)
                is_v3 = true;

            if(!is_v3) {
            if (community_not_default && trap_rcvr_port_not_default) {
                vtysh_ovsdb_cli_print(
                    p_msg, "%s %s %s %s %s %s %s %s %lld", "snmp-server host",
                    psnmp_trap_row->receiver_address, psnmp_trap_row->type,
                    "version", psnmp_trap_row->version, "community",
                    snmp_trap_community, "port", psnmp_trap_row->receiver_udp_port);
                continue;
            }
            else if (community_not_default) {
                vtysh_ovsdb_cli_print(
                    p_msg, "%s %s %s %s %s %s %s", "snmp-server host",
                    psnmp_trap_row->receiver_address, psnmp_trap_row->type,
                    "version", psnmp_trap_row->version, "community",
                    snmp_trap_community);
                continue;
            }
            else if (trap_rcvr_port_not_default) {
                vtysh_ovsdb_cli_print(
                    p_msg, "%s %s %s %s %s %s %lld", "snmp-server host",
                    psnmp_trap_row->receiver_address, psnmp_trap_row->type,
                    "version", psnmp_trap_row->version, "port",
                    psnmp_trap_row->receiver_udp_port);
                continue;
            }
            else if(!community_not_default && !trap_rcvr_port_not_default) {
                vtysh_ovsdb_cli_print(
                    p_msg, "%s %s %s %s %s", "snmp-server host",
                    psnmp_trap_row->receiver_address, psnmp_trap_row->type,
                    "version", psnmp_trap_row->version);
                continue;
            }
            }
            else {
                if (trap_rcvr_port_not_default) {
                    vtysh_ovsdb_cli_print(
                        p_msg, "%s %s %s %s %s %s %s %s %lld", "snmp-server host",
                        psnmp_trap_row->receiver_address, psnmp_trap_row->type,
                        "version", psnmp_trap_row->version,"user",snmp_trap_community,
                        "port", psnmp_trap_row->receiver_udp_port);
                    continue;
                }
                else {
                    vtysh_ovsdb_cli_print(
                        p_msg, "%s %s %s %s %s %s %s", "snmp-server host",
                        psnmp_trap_row->receiver_address, psnmp_trap_row->type,
                        "version", psnmp_trap_row->version,"user",snmp_trap_community);
                    continue;
                }
            }
        }
    }
    OVSREC_SNMPV3_USER_FOR_EACH(v3user_row, idl) {
        if(v3user_row != NULL) {
            bool auth_not_default = false;
            bool priv_not_default = false;

            if(v3user_row->auth_protocol != NULL){
                if(strncmp(v3user_row->auth_protocol, DEFAULT_AUTH, MAX_PROTOCOL_STR_LENGTH) != 0)
                    auth_not_default = true;
            }
            if (v3user_row->priv_protocol != NULL){
                if(strncmp(v3user_row->priv_protocol, DEFAULT_PRIVECY, MAX_PROTOCOL_STR_LENGTH) != 0)
                    priv_not_default = true;
            }
            if(auth_not_default && priv_not_default){
                vtysh_ovsdb_cli_print(
                    p_msg, "%s %s %s %s %s %s %s %s %s %s", "snmpv3 user",
                    v3user_row->user_name,"auth", v3user_row->auth_protocol, "auth-pass", v3user_row->auth_pass_phrase,
                    "priv", v3user_row->priv_protocol, "priv-pass", v3user_row->priv_pass_phrase);
                continue;
            }
            else if(auth_not_default) {
                vtysh_ovsdb_cli_print(
                    p_msg, "%s %s %s %s %s %s", "snmpv3 user",
                    v3user_row->user_name, "auth", v3user_row->auth_protocol, "auth-pass", v3user_row->auth_pass_phrase);
                continue;
            }
            else {
                vtysh_ovsdb_cli_print(
                    p_msg, "%s %s", "snmpv3 user", v3user_row->user_name);
                continue;
            }
        }
    }
    return e_vtysh_ok;
}
