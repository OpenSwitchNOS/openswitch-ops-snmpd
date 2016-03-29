#ifndef SNMP_UTILS_H
#define SNMP_UTILS_H 1

extern pthread_mutex_t snmp_ovsdb_mutex;

/* Macros to lock and unlock mutexes in a verbose manner. */
#define SNMP_OVSDB_LOCK { \
                VLOG_DBG("%s(%d): SNMP_OVSDB_LOCK: taking lock...", __FUNCTION__, __LINE__); \
                pthread_mutex_lock(&snmp_ovsdb_mutex); \
}

#define SNMP_OVSDB_UNLOCK { \
                VLOG_DBG("%s(%d): SNMP_OVSDB_UNLOCK: releasing lock...", __FUNCTION__, __LINE__); \
                pthread_mutex_unlock(&snmp_ovsdb_mutex); \
}

#endif /* SNMP_UTILS_H*/
