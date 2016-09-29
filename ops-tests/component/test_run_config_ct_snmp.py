# -*- coding: utf-8 -*-

# (c) Copyright 2015 Hewlett Packard Enterprise Development LP
#
# GNU Zebra is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2, or (at your option) any
# later version.
#
# GNU Zebra is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Zebra; see the file COPYING.  If not, write to the Free
# Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
# 02111-1307, USA.


from pytest import mark

TOPOLOGY = """
# +-------+
# |  sw1  |
# +-------+

# Nodes
[type=openswitch name="Switch 1"] sw1
"""


def check_run_config(sw1):
    with sw1.libs.vtysh.Configure() as ctx:
        ctx.snmp_server_system_description('OpenSwitch_system')
        ctx.snmp_server_system_location('Nothing_by_default')
        ctx.snmp_server_system_contact('xyz@abc.com')
    result = sw1.libs.vtysh.show_snmp_system()
    assert result['system_description'] == 'OpenSwitch_system'
    assert result['system_location'] == 'Nothing_by_default'
    assert result['system_contact'] == 'xyz@abc.com'

    result = sw1("show running-config")
    run_config_lines = result.splitlines()
    for line in run_config_lines:
        if "snmp-server system_description OpenSwitch_system" in line:
            assert "OpenSwitch_system" in line
        if "snmp-server system_location Nothing_by_default" in line:
            assert "Nothing_by_default" in line
        if "snmp-server system_contact xyz@abc.com" in line:
            assert "xyz@abc.com" in line
    with sw1.libs.vtysh.Configure() as ctx:
        ctx.no_snmp_server_system_description()
        ctx.no_snmp_server_system_location()
        ctx.no_snmp_server_system_contact()

    result = sw1("show running-config")
    run_config_lines = result.splitlines()
    for line in run_config_lines:
        assert "snmp-server system_description OpenSwitch_system" not in line
        assert "snmp-server system_location Nothing_by_default" not in line
        assert "snmp-server system_contact xyz@abc.com" not in line


@mark.gate
def test_ct_snmp_functionality(topology, step):
    sw1 = topology.get("sw1")
    check_run_config(sw1)
