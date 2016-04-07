#!/usr/bin/env pythonll Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may
# not use this file except in compliance with the License. You may obtain
# a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations
# under the License
# Copyright (C) 2015 Hewlett Packard Enterprise Development LP
# All Rights Reserved.

from opstestfw import *
from opstestfw.switch.CLI import *
from opstestfw.switch import *
from opstestfw import testEnviron
from opstestfw import LogOutput
from opstestfw import Sleep

# Topology definition
topoDict = {"topoExecution": 1000,
            "topoTarget": "dut01",
            "topoDevices": "dut01",
            "topoFilters": "dut01:system-category:switch"}


def snmp_vtysh_test(**kwargs):
    device1 = kwargs.get('device1', None)

    # 1.1 Configuring snmp agent-port on SW1
    LogOutput('info', "\n\n\nConfiguring snmp agent-port on SW1")
    configured = False
    devIntRetStruct =\
        device1.DeviceInteract(command="conf t")
    retCode = devIntRetStruct.get('returnCode')
    assert retCode == 0, "Failed to enter global config context"

    devIntRetStruct =\
        device1.DeviceInteract(command="snmp-server agent-port 700")
    retCode = devIntRetStruct.get('returnCode')
    assert retCode == 0, "Failed to configure agent-port"

    devIntRetStruct =\
        device1.DeviceInteract(command="do show snmp agent-port")
    retCode = devIntRetStruct.get('returnCode')
    assert retCode == 0, "Failed to execute show snmp agent-port"
    buff = devIntRetStruct.get('buffer')
    if '700' in buff:
        configured = True
    assert configured, "SNMP agent port could not be configured"


    # 1.2 Unonfiguring snmp agent-port on SW1
    LogOutput('info', "\n\n\nUnonfiguring snmp agent-port on SW1")
    unconfigured = False

    devIntRetStruct =\
        device1.DeviceInteract(command="no snmp-server agent-port")
    retCode = devIntRetStruct.get('returnCode')
    assert retCode == 0, "Failed to configure agent-port"

    devIntRetStruct =\
        device1.DeviceInteract(command="do show snmp agent-port")
    retCode = devIntRetStruct.get('returnCode')
    assert retCode == 0, "Failed to execute show snmp agent-port"
    buff = devIntRetStruct.get('buffer')
    if '700' not in buff:
        unconfigured = True
    assert unconfigured , "SNMP agent port could not be configured"

    # 2.1 Configuring snmp-server community on SW1
    LogOutput('info', "\n\n\nConfiguring snmp-server community on SW1")
    configured = False

    devIntRetStruct =\
        device1.DeviceInteract(command="snmp-server community testcom")
    retCode = devIntRetStruct.get('returnCode')
    assert retCode == 0, "Failed to configure snmp community"

    devIntRetStruct =\
        device1.DeviceInteract(command="do show snmp community")
    retCode = devIntRetStruct.get('returnCode')
    assert retCode == 0, "Failed to execute show snmp community"
    buff = devIntRetStruct.get('buffer')
    if 'testcom' in buff:
        configured = True
    assert configured , "SNMP community could not be configured"

    # 2.2 Unconfiguring snmp-server community on SW1
    LogOutput('info', "\n\n\nUnconfiguring snmp-server community on SW1")
    unconfigured = False

    devIntRetStruct =\
        device1.DeviceInteract(command="no snmp-server community testcom")
    retCode = devIntRetStruct.get('returnCode')
    assert retCode == 0, "Failed to configure snmp community"

    devIntRetStruct =\
        device1.DeviceInteract(command="do show snmp community")
    retCode = devIntRetStruct.get('returnCode')
    assert retCode == 0, "Failed to execute show snmp community"
    buff = devIntRetStruct.get('buffer')
    if 'testcom' not in buff:
        unconfigured = True
    assert unconfigured , "SNMP community could not be configured"

    # 3.1 Configuring snmp-server host community on SW1
    LogOutput('info', "\n\n\nConfiguring snmp-server host community on SW1")
    configured = False

    devIntRetStruct =\
        device1.DeviceInteract(command="snmp-server host 10.1.1.1 trap version v1")
    retCode = devIntRetStruct.get('returnCode')
    assert retCode == 0, "Failed to configure snmp-server host"

    devIntRetStruct =\
        device1.DeviceInteract(command="do show snmp trap")
    retCode = devIntRetStruct.get('returnCode')
    assert retCode == 0, "Failed to execute show snmp trap"
    buff = devIntRetStruct.get('buffer')
    buffLines = buff.splitlines()
    for line in buffLines:
        if '10.1.1.1' and 'v2c' and 'trap' and 'public' and '162' in line:
            configured = True
    assert configured , "SNMP trap could not be configured"

    # 3.2 Unconfiguring snmp-server host community on SW1
    LogOutput('info', "\n\n\nUnconfiguring snmp-server host community on SW1")
    unconfigured = False

    devIntRetStruct =\
        device1.DeviceInteract(command="no snmp-server host 10.1.1.1 trap version v1")
    retCode = devIntRetStruct.get('returnCode')
    assert retCode == 0, "Failed to configure snmp-server host"

    devIntRetStruct =\
        device1.DeviceInteract(command="do show snmp trap")
    retCode = devIntRetStruct.get('returnCode')
    assert retCode == 0, "Failed to execute show snmp trap"
    buff = devIntRetStruct.get('buffer')
    buffLines = buff.splitlines()
    for line in buffLines:
        if '10.1.1.1' and 'v2c' and 'trap' and 'public' and '162' not in line:
            unconfigured = True
    assert unconfigured , "SNMP trap could not be unconfigured"


    # 4.1 Configuring snmp-server host community on SW1
    LogOutput('info', "\n\n\nConfiguring snmp-server host community on SW1")
    configured = False

    devIntRetStruct =\
        device1.DeviceInteract(command="snmp-server host 20.1.1.1 trap version v2c community testcom port 907")
    retCode = devIntRetStruct.get('returnCode')
    assert retCode == 0, "Failed to configure snmp-server host"

    devIntRetStruct =\
        device1.DeviceInteract(command="do show snmp trap")
    retCode = devIntRetStruct.get('returnCode')
    assert retCode == 0, "Failed to execute show snmp trap"
    buff = devIntRetStruct.get('buffer')
    buffLines = buff.splitlines()
    for line in buffLines:
        if '10.1.1.1' and 'v2c' and 'trap' and 'testcom' and '907' in line:
            configured = True
    assert configured , "SNMP trap could not be configured"

    # 4.2 Unconfiguring snmp-server host community on SW1
    LogOutput('info', "\n\n\n Unconfiguring snmp-server host community on SW1")
    unconfigured = False

    devIntRetStruct =\
        device1.DeviceInteract(command="no snmp-server host 20.1.1.1 trap version v2c community testcom port 907")
    retCode = devIntRetStruct.get('returnCode')
    assert retCode == 0, "Failed to Unconfiguring snmp-server host community on SW1"

    devIntRetStruct =\
        device1.DeviceInteract(command="do show snmp trap")
    retCode = devIntRetStruct.get('returnCode')
    assert retCode == 0, "Failed to execute show snmp trap"
    buff = devIntRetStruct.get('buffer')
    buffLines = buff.splitlines()
    for line in buffLines:
        if '10.1.1.1' and 'v2c' and 'trap' and 'testcom' and '907' not in line:
            unconfigured = True
    assert unconfigured , "SNMP trap could not be configured"


    #5 1 Configuring snmp-server host community on SW1
    LogOutput('info', "\n\n\nConfiguring snmp-server host community on SW1")
    configured = False

    devIntRetStruct =\
        device1.DeviceInteract(command="snmp-server host 10.10.10.1 inform version v2c community stpcommunity port 655")
    retCode = devIntRetStruct.get('returnCode')
    assert retCode == 0, "Failed to configure snmp-server host"

    devIntRetStruct =\
        device1.DeviceInteract(command="do show snmp trap")
    retCode = devIntRetStruct.get('returnCode')
    assert retCode == 0, "Failed to execute show snmp trap"
    buff = devIntRetStruct.get('buffer')
    buffLines = buff.splitlines()
    for line in buffLines:
        if '10.10.1.1' and 'v1' and 'inform' and 'stpcommunity' and '655' in line:
            configured = True
    assert configured , "SNMP v2c inform could not be configured"

    #5 2 Unconfiguring snmp-server host community on SW1
    LogOutput('info', "\n\n\nUnconfiguring snmp-server host community on SW1")
    unconfigured = False

    devIntRetStruct =\
        device1.DeviceInteract(command="no snmp-server host 10.10.10.1 inform version v2c community stpcommunity port 655")
    retCode = devIntRetStruct.get('returnCode')
    assert retCode == 0, "Failed to configure snmp-server host"

    devIntRetStruct =\
        device1.DeviceInteract(command="do show snmp trap")
    retCode = devIntRetStruct.get('returnCode')
    assert retCode == 0, "Failed to execute show snmp trap"
    buff = devIntRetStruct.get('buffer')
    buffLines = buff.splitlines()
    for line in buffLines:
        if '10.10.1.1' and 'v1' and 'inform' and 'stpcommunity' and '655' not in line:
            unconfigured = True
    assert unconfigured , "SNMP v2c inform could not be configured"


    # 6.1 Configuring snmp system description on SW1
    LogOutput('info', "\n\n\nConfiguring snmp system description on SW1")
    configured = False

    devIntRetStruct =\
        device1.DeviceInteract(command="snmp-server system-description this has nothing by default")
    retCode = devIntRetStruct.get('returnCode')
    assert retCode == 0, "Failed to configure agent-port"

    devIntRetStruct =\
        device1.DeviceInteract(command="do show snmp system")
    retCode = devIntRetStruct.get('returnCode')
    assert retCode == 0, "Failed to execute show snmp system"
    buff = devIntRetStruct.get('buffer')
    if 'this has nothing by default' in buff:
        configured = True
    assert configured , "SNMP system description can not be configured"

    # 6.2 Unconfiguring snmp system description on SW1
    LogOutput('info', "\n\n\nUnconfiguring snmp system description on SW1")
    unconfigured = False

    devIntRetStruct =\
        device1.DeviceInteract(command="no snmp-server system-description")
    retCode = devIntRetStruct.get('returnCode')
    assert retCode == 0, "Failed to unconfigure system description"

    devIntRetStruct =\
        device1.DeviceInteract(command="do show snmp system")
    retCode = devIntRetStruct.get('returnCode')
    assert retCode == 0, "Failed to execute show snmp system"
    buff = devIntRetStruct.get('buffer')
    if 'this has nothing by default' not in buff:
        unconfigured = True
    assert unconfigured , "SNMP system description can not be configured"

    # 7.1 Configuring snmp system contact on SW1
    LogOutput('info', "\n\n\nConfiguring snmp system contact on SW1")
    configured = False

    devIntRetStruct =\
        device1.DeviceInteract(command="snmp-server system-contact abc adc@def.com")
    retCode = devIntRetStruct.get('returnCode')
    assert retCode == 0, "Failed to Configuring snmp system contact on SW1"

    devIntRetStruct =\
        device1.DeviceInteract(command="do show snmp system")
    retCode = devIntRetStruct.get('returnCode')
    assert retCode == 0, "Failed to execute show snmp system"
    buff = devIntRetStruct.get('buffer')
    if 'abc adc@def.com' in buff:
        configured = True
    assert configured , "SNMP system contact can not be configured"

    # 7.2 Unconfiguring snmp system contact on SW1
    LogOutput('info', "\n\n\nUnconfiguring snmp system contact on SW1")
    configured = False

    devIntRetStruct =\
        device1.DeviceInteract(command="no snmp-server system-contact")
    retCode = devIntRetStruct.get('returnCode')
    assert retCode == 0, "Failed to Unconfiguring snmp system contact on SW1"

    devIntRetStruct =\
        device1.DeviceInteract(command="do show snmp system")
    retCode = devIntRetStruct.get('returnCode')
    assert retCode == 0, "Failed to execute show snmp system"
    buff = devIntRetStruct.get('buffer')
    if 'abc adc@def.com' not in buff:
        configured = True
    assert configured , "SNMP system contact can not be unconfigured"

    # 8.1 Configuring snmp system location on SW1
    LogOutput('info', "\n\n\nConfiguring snmp system location on SW1")
    configured = False

    devIntRetStruct =\
        device1.DeviceInteract(command="snmp-server system-location on the dock of the bay")
    retCode = devIntRetStruct.get('returnCode')
    assert retCode == 0, "Failed to Configuring snmp system location on SW1"

    devIntRetStruct =\
        device1.DeviceInteract(command="do show snmp system")
    retCode = devIntRetStruct.get('returnCode')
    assert retCode == 0, "Failed to execute show snmp system"
    buff = devIntRetStruct.get('buffer')
    if 'on the dock of the bay' in buff:
        configured = True
    assert configured , "SNMP system location can not be configured"

    # 8.2 Unconfiguring snmp system location on SW1
    LogOutput('info', "\n\n\nUnconfiguring snmp system location on SW1")
    unconfigured = False

    devIntRetStruct =\
        device1.DeviceInteract(command="no snmp-server system-location")
    retCode = devIntRetStruct.get('returnCode')
    assert retCode == 0, "Failed to Unconfiguring snmp system location on SW1"

    devIntRetStruct =\
        device1.DeviceInteract(command="do show snmp system")
    retCode = devIntRetStruct.get('returnCode')
    assert retCode == 0, "Failed to execute show snmp system"
    buff = devIntRetStruct.get('buffer')
    if 'on the dock of the bay' not in buff:
        unconfigured = True
    assert unconfigured, "SNMP system location can not be configured"



class Test_snmp_configuration:
    def setup_class(cls):
        # Test object will parse command line and formulate the env
        Test_snmp_configuration.testObj =\
            testEnviron(topoDict=topoDict, defSwitchContext="vtyShell")
        #    Get topology object
        Test_snmp_configuration.topoObj = \
            Test_snmp_configuration.testObj.topoObjGet()

    def teardown_class(cls):
        Test_snmp_configuration.topoObj.terminate_nodes()

    def test_lldp_enable_disable(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        snmp_vtysh_test(device1=dut01Obj)
