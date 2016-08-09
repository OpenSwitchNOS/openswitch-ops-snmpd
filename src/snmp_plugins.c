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
#include "coverage.h"
#include "dynamic-string.h"
#include "openvswitch/vlog.h"

VLOG_DEFINE_THIS_MODULE(snmp_plugins);
#define FEATURE_SNMP_PATH_IF "/usr/lib/snmp/plugins/libintfd_snmp"

typedef void(*plugin_func)(void);

struct plugin_class {
    plugin_func ops_snmp_init;
    plugin_func ops_snmp_run;
    plugin_func ops_snmp_wait;
    plugin_func ops_snmp_destroy;
};

static lt_dlinterface_id interface_id;

static int
plugins_open_plugin(const char *filename, void *data)
{
    struct plugin_class *plcl;
    lt_dlhandle handle;

    if (!(handle = lt_dlopenadvise(filename, *(lt_dladvise *)data))) {
        VLOG_ERR("Failed loading %s: %s", filename, lt_dlerror());
        return 0;
    }

    if (!(plcl = (struct plugin_class *)malloc(sizeof(struct plugin_class)))) {
        VLOG_ERR("Couldn't allocate plugin class");
        goto err_plugin_class;
    }

    if (!(plcl->ops_snmp_init = lt_dlsym(handle, "ops_snmp_init")) ||
        !(plcl->ops_snmp_run = lt_dlsym(handle, "ops_snmp_run")) ||
        !(plcl->ops_snmp_destroy = lt_dlsym(handle, "ops_snmp_destroy"))) {
            VLOG_ERR("Couldn't initialize the interface for %s", filename);
            goto err_dlsym;
    }


    plcl->ops_snmp_init();

    VLOG_INFO("Loaded SNMP plugin library %s", filename);
    return 0;

err_dlsym:
    free(plcl);

err_plugin_class:
    if (lt_dlclose(handle)) {
        VLOG_ERR("Couldn't dlclose %s", filename);
    }

    return 0;
}

void
plugins_snmp_init(const char *path)
{
    lt_dladvise advise;

    if (path && !strcmp(path, "none")) {
        return;
    }

    if (lt_dlinit() ||
        lt_dlsetsearchpath(path) ||
        lt_dladvise_init(&advise)) {
        VLOG_ERR("ltdl initializations: %s", lt_dlerror());
    }

    if (!(interface_id = lt_dlinterface_register("ops-snmpd", NULL))) {
        VLOG_ERR("lt_dlinterface_register: %s", lt_dlerror());
        goto err_interface_register;
    }

    if (lt_dladvise_global(&advise) || lt_dladvise_ext (&advise) ||
        lt_dlforeachfile(lt_dlgetsearchpath(), &plugins_open_plugin, &advise)) {
        VLOG_ERR("ltdl setting advise: %s", lt_dlerror());
        goto err_set_advise;
    }

    VLOG_INFO("Successfully initialized all SNMP plugins");
    return;

err_set_advise:
    lt_dlinterface_free(interface_id);

err_interface_register:
    if (lt_dladvise_destroy(&advise)) {
        VLOG_ERR("destroying ltdl advise%s", lt_dlerror());
        return;
    }

}

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
/* Changed for to add a support for Context registration on runtime,
    Currently, only intfd plugin only was triggered  */
void
plugins_snmp_run(void)
{
   struct plugin_class *plcl;
    lt_dlhandle handle =0 ;
    lt_dladvise advise;
    const char *filename = FEATURE_SNMP_PATH_IF;

    if (!(lt_dladvise_init (&advise)) && !(lt_dladvise_ext (&advise)))
       {
       if (!(handle = lt_dlopenadvise(filename, advise))) {
        VLOG_ERR("Failed loading %s: %s",filename, lt_dlerror());
        }
        lt_dladvise_destroy (&advise);
      }

    if (!(plcl = (struct plugin_class *)malloc(sizeof(struct plugin_class)))) {
        VLOG_ERR("Couldn't allocate plugin class");
        goto err_plugin_class;
    }

    if (!(plcl->ops_snmp_run = lt_dlsym(handle, "ops_snmp_run"))) {
            VLOG_ERR("Couldn't initialize the interface for %s", filename);
            goto err_dlsym;
    }

    plcl->ops_snmp_run();
    VLOG_INFO("Loaded SNMP plugin library %s", filename);

err_dlsym:
    free(plcl);

err_plugin_class:
    if (lt_dlclose(handle)) {
        VLOG_ERR("Couldn't dlclose %s",filename);
    }
}

void
plugins_snmp_wait(void)
{
    PLUGINS_CALL(ops_snmp_wait);
}

void
plugins_snmp_destroy(void)
{
    PLUGINS_CALL(ops_snmp_destroy);
    lt_dlinterface_free(interface_id);
    lt_dlexit();
    VLOG_INFO("Destroyed all plugins");
}
