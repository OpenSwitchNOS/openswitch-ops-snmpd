#include <pthread.h>
#include "snmp_utils.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

pthread_mutex_t snmp_ovsdb_mutex = PTHREAD_MUTEX_INITIALIZER;
