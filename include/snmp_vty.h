/* SNMP CLI commands header file
 *
 * Copyright (C) 2015 Hewlett Packard Enterprise Development LP
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * File: snmp_vty.h
 *
 * Purpose:  To add declarations required for snmp_vty.c
 */

#ifndef SNMP_VTY_H
#define SNMP_VTY_H

/* Help Strings for SNMP */
#define SNMP_STR "Configure SNMP\n"
#define HOST_STR "Configure SNMP trap or inform\n"
#define AGENT_PORT_STR "The port on which the SNMP master agent listens for SNMP requests\n"
#define COMMUNITY_STR "The name of the community string. Default is public\n"
#define NOTIFICATION_TYPE_STR "The SNMP notification type\n"
#define VERSION_STR "The SNMP protocol version\n"


/* Default values */
#define DEFAULT_TRAP_TYPE "trap"
#define DEFAULT_VERSION_TYPE "v2c"
#define DEFAULT_COMMUNITY_TYPE "public"
#define DEFAULT_AGENT_PORT 161
#define DEFAULT_TRAP_RECEIVER_UDP_PORT 162


/* MAX values */
#define MAX_COMMUNITY_LENGTH 33
#define MAX_IP_STR_LENGTH 46
#define MAX_PORT_STR_LENGTH 6
#define MAX_VERSION_LENGTH 4
#define MAX_TYPE_LENGTH 7

void cli_pre_init();
void cli_post_init();

#endif /* SNMP_VTY_H */
