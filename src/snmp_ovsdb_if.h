#ifndef SNMP_OVSDB_IF_H
#define SNMP_OVSDB_IF_H

#define SNMP_ENGINEID_STR_LEN 32
/* function declarations */
void snmpd_ovsdb_init(void);
void *snmp_ovsdb_main_thread(void *arg);

#endif /* SNMP_OVSDB_IF_H */
