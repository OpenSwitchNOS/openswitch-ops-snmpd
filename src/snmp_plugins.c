/* Feature Specific CLI commands initialize via plugins source file.
 *
 * Copyright (C) 2015 Hewlett Packard Enterprise Development LP.
 *
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
 * File: snmp_plugins.c
 *
 * Purpose: To install the feature specific snmp MIB nodes & elements
 *          via plugins.
 */

#include <config.h>
#include <errno.h>
#include <ltdl.h>
#include <unistd.h>
#include "snmp_plugins.h"
#include "openvswitch/vlog.h"

#define SNMP_PLUGINS_ERR -1

VLOG_DEFINE_THIS_MODULE(snmp_plugins);

#define PLUGINS_CALL(FUNC) \
do { \
    lt_dlhandle iter_handle = 0; \
    struct plugin_class *plcl; \
    while ((iter_handle = lt_dlhandle_iterate(interface_id, iter_handle))) { \
        plcl = (struct plugin_class *)lt_dlcaller_get_data(interface_id, iter_handle); \
        if (plcl && plcl->FUNC) { \
            plcl->FUNC(); \
        } \
    } \
}while(0)

typedef void(*plugin_func)(void);

/* plugin_class structure used for calling feature specific
 * snmp init function by plugin_func.
 */
struct plugin_class {
    plugin_func    ops_snmp_init;
    plugin_func    ops_snmp_run;
    plugin_func    ops_snmp_wait;
    plugin_func    ops_snmp_destroy;
};

/*
 * Unique key to store and retrieve per-module data.
 */
static lt_dlinterface_id interface_id;

/*
 * Function : plugins_snmp_destroy.
 * Responsibility : unloading all feature specific snmp module.
 * Parameters : void.
 * Return : void.
 */
void
plugins_snmp_destroy(void)
{
    PLUGINS_CALL(ops_snmp_destroy);
    lt_dlinterface_free(interface_id);
    lt_dlexit();
    VLOG_INFO("Destroyed all plugins");
    exit(1);
}

void
plugins_snmp_run(void)
{
    struct plugin_class plcl = {NULL};
    plcl.ops_snmp_run();
}

void
plugins_snmp_wait(void)
{
    struct plugin_class plcl = {NULL};
    plcl.ops_snmp_wait();
}



/*
 * Function : plugins_open_plugin.
 * Responsibility : load and call the feature specific module and init function.
 * Parameters :
 *   const char *filename: module filename which is passed by libltdl.
 *   void *data : contains module loading modes which is initialized by libltdl.
 * Return : return 0 on suceess.
 */
static int
plugins_open_plugin(const char *filename, void *data)
{
    struct plugin_class plcl = {NULL};
    lt_dlhandle handle;

    if (!(handle = lt_dlopenadvise(filename, *(lt_dladvise *)data))) {
        VLOG_ERR("Failed loading %s: %s\n", filename, lt_dlerror());
        plugins_snmp_destroy();
        return SNMP_PLUGINS_ERR;
    }

    plcl.ops_snmp_init = lt_dlsym(handle, "ops_snmp_init");
    if (plcl.ops_snmp_init == NULL) {
        plugins_snmp_destroy();
        return SNMP_PLUGINS_ERR;
    }

    plcl.ops_snmp_run = lt_dlsym(handle, "ops_snmp_run");
    if (plcl.ops_snmp_run == NULL) {
        plugins_snmp_destroy();
        return SNMP_PLUGINS_ERR;
    }
    plcl.ops_snmp_wait = lt_dlsym(handle, "ops_snmp_wait");
    if (plcl.ops_snmp_wait == NULL) {
        plugins_snmp_destroy();
        return SNMP_PLUGINS_ERR;
    }
    plcl.ops_snmp_destroy = lt_dlsym(handle, "ops_snmp_destroy");
    if (plcl.ops_snmp_destroy == NULL) {
        plugins_snmp_destroy();
        return SNMP_PLUGINS_ERR;
    }

    if (lt_dlcaller_set_data(interface_id, handle, &plcl)) {
        VLOG_ERR("plugin %s initialized twice\n", filename);
        plugins_snmp_destroy();
        return SNMP_PLUGINS_ERR;
    }

    plcl.ops_snmp_init();

    VLOG_DBG("Loaded SNMP plugin library %s\n", filename);
    return 0;
}

/*
 * Function : plugins_snmp_init.
 * Responsibility : Initialize feature specific snmp.
 * Parameters : const char *path : libltdl search path.
 * Return : void.
 */
void
plugins_snmp_init(const char *path)
{
     lt_dladvise advise;

    /* Initialize libltdl, set the libltdl serach path,
     * initialize advise parameter,which is used pass hints
     * to module loader when using lt_dlopenadvise to perform the loading.
     plugins_snmp_init*/
    if (lt_dlinit() ||
        lt_dlsetsearchpath(path) ||
        lt_dladvise_init(&advise)) {
        VLOG_ERR("ltdl initializations: %s\n", lt_dlerror());
        return;
    }

    /* Register ops-snmpd interface validator with libltdl */
    /* TO-DO : repo name will change to ops-snmpd */
    if (!(interface_id = lt_dlinterface_register("ops-snmpd", NULL))) {
        VLOG_ERR("lt_dlinterface_register: %s\n", lt_dlerror());
        if (lt_dladvise_destroy(&advise)) {
            VLOG_ERR("destroying ltdl advise%s\n", lt_dlerror());
        }
        return;
    }

    /* set symglobal hint and call the feature specific snmp init function via
     * plugins_open_plugin function pointer.
     * 'lt_dlforeachfile' function will continue to make calls to
     * 'plugins_open_plugin()' for each file that it discovers in search_path
     * until one of these calls returns non-zero, or until the
     * files are exhausted.
     * `lt_dlforeachfile' returns value returned by the last call
     * made to 'plugins_open_plugin'.
     */
    if (lt_dladvise_global(&advise) || lt_dladvise_ext (&advise) ||
        lt_dlforeachfile(lt_dlgetsearchpath(), &plugins_open_plugin, &advise)) {
        VLOG_ERR("ltdl setting advise: %s\n", lt_dlerror());
        return;
    }

    VLOG_INFO("Successfully initialized all snmp plugins");
    return;
}
