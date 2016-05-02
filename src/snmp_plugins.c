/*
 * Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2014 Nicira, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Hewlett-Packard Company Confidential (C) Copyright 2015 Hewlett-Packard Development Company, L.P.
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

void
plugins_snmp_run(void)
{
    PLUGINS_CALL(ops_snmp_run);
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
