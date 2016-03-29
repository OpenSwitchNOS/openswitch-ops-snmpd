#ifndef SNMP_PLUGINS_H
#define SNMP_PLUGINS_H 1

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef __linux__
void plugins_snmp_init(const char *path);
void plugins_snmp_run(void);
void plugins_snmp_wait(void);
void plugins_snmp_destroy(void);
#endif

#ifdef  __cplusplus
}
#endif

#endif /* snmp_plugins.h */
