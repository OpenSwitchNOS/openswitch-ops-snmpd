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


TOPOLOGY = """
# +-------+
# |  sw1  |
# +-------+

# Nodes
[type=openswitch name="Switch 1"] sw1
"""


def config_snmp_agent_port_test(sw1):
    with sw1.libs.vtysh.Configure() as ctx:
        ctx.snmp_server_agent_port('7900')
    result = sw1.libs.vtysh.show_snmp_agent_port()
    assert result['agent_port'] == '7900'


def unconfig_snmp_agent_port_test(sw1):
    with sw1.libs.vtysh.Configure() as ctx:
        ctx.no_snmp_server_agent_port()
    result = sw1.libs.vtysh.show_snmp_agent_port()
    assert result is None


def config_snmp_community_test(sw1):
    with sw1.libs.vtysh.Configure() as ctx:
        ctx.snmp_server_community('private')
    result = sw1.libs.vtysh.show_snmp_community()
    for community in result:
        assert community == 'private'


def unconfig_snmp_community_test(sw1):
    with sw1.libs.vtysh.Configure() as ctx:
        ctx.no_snmp_server_community('private')
    result = sw1.libs.vtysh.show_snmp_community()
    for community in result:
        assert community is not 'private'


def config_snmp_system_test(sw1):
    with sw1.libs.vtysh.Configure() as ctx:
        ctx.snmp_server_system_description('OpenSwitch_system')
        ctx.snmp_server_system_location('Nothing_by_default')
        ctx.snmp_server_system_contact('xyz@abc.com')
    result = sw1.libs.vtysh.show_snmp_system()
    assert result['system_description'] == 'OpenSwitch_system'
    assert result['system_location'] == 'Nothing_by_default'
    assert result['system_contact'] == 'xyz@abc.com'


def unconfig_snmp_system_test(sw1):
    with sw1.libs.vtysh.Configure() as ctx:
        ctx.no_snmp_server_system_description()
        ctx.no_snmp_server_system_location()
        ctx.no_snmp_server_system_contact()
    result = sw1.libs.vtysh.show_snmp_system()
    assert bool(result) is False


def config_snmp_v1_trap_test(sw1):
    with sw1.libs.vtysh.Configure() as ctx:
        ctx.snmp_server_host_trap_version('10.1.1.1', 'v1')
    """
    sw1('vtysh')
    sw1('conf t')
    sw1('snmp-server host 10.1.1.1 trap version v1')
    sw1('end')
    """
    result = sw1.libs.vtysh.show_snmp_trap()
    assert result['10.1.1.1']['Type'] == 'trap'
    assert result['10.1.1.1']['Version'] == 'v1'


def unconfig_snmp_v1_trap_test(sw1):
    with sw1.libs.vtysh.Configure() as ctx:
        ctx.no_snmp_server_host_trap_version('10.1.1.1', 'v1')
    result = sw1.libs.vtysh.show_snmp_trap()
    assert result is None


def config_snmp_v2c_trap_test(sw1):
    with sw1.libs.vtysh.Configure() as ctx:
        ctx.snmp_server_host_trap_version('20.2.2.2', 'v2c',
                                          community='community',
                                          community_name='com2c')
    result = sw1.libs.vtysh.show_snmp_trap()
    assert result['20.2.2.2']['Type'] == 'trap'
    assert result['20.2.2.2']['Version'] == 'v2c'


def unconfig_snmp_v2c_trap_test(sw1):
    with sw1.libs.vtysh.Configure() as ctx:
        ctx.no_snmp_server_host_trap_version('20.2.2.2', 'v2c',
                                             community='community',
                                             community_name='com2c')
    result = sw1.libs.vtysh.show_snmp_trap()
    assert result is None


def config_snmp_v3_auth_user_test(sw1):
    with sw1.libs.vtysh.Configure() as ctx:
        ctx.snmpv3_user_auth_auth_pass('testv3user', auth_protocol='md5',
                                       auth_password='password')
    result = sw1.libs.vtysh.show_snmpv3_users()
    assert 'testv3user' in result
    assert result['testv3user']['AuthMode'] == 'md5'


def unconfig_snmp_v3_auth_user_test(sw1):
    with sw1.libs.vtysh.Configure() as ctx:
        ctx.no_snmpv3_user_auth_auth_pass('testv3user', auth_protocol='md5',
                                          auth_password='password')
    result = sw1.libs.vtysh.show_snmpv3_users()
    assert result is None


def config_snmp_v3_user_test(sw1):
    with sw1.libs.vtysh.Configure() as ctx:
        ctx.snmpv3_user('testv3user')
    result = sw1.libs.vtysh.show_snmpv3_users()
    assert 'testv3user' in result


def unconfig_snmp_v3_user_test(sw1):
    with sw1.libs.vtysh.Configure() as ctx:
        ctx.no_snmpv3_user('testv3user')
    result = sw1.libs.vtysh.show_snmpv3_users()
    assert result is None


def config_snmp_v3_trap_test(sw1):
    with sw1.libs.vtysh.Configure() as ctx:
        ctx.snmpv3_user('testv3user')
        ctx.snmp_server_host_trap_version('20.2.2.2', 'v3',
                                          community='user',
                                          community_name='testv3user')
    result = sw1.libs.vtysh.show_snmp_trap()
    assert result['20.2.2.2']['Type'] == 'trap'
    assert result['20.2.2.2']['Version'] == 'v3'
    assert result['20.2.2.2']['SecName'] == 'testv3user'


def unconfig_snmp_v3_trap_test(sw1):
    with sw1.libs.vtysh.Configure() as ctx:
        ctx.no_snmp_server_host_trap_version('20.2.2.2', 'v3',
                                             community='user',
                                             community_name='testv3user')
        ctx.no_snmpv3_user('testv3user')
    result = sw1.libs.vtysh.show_snmp_trap()
    assert result is None


def test_vtysh_ct_snmp(topology, step):
    sw1 = topology.get("sw1")
    config_snmp_agent_port_test(sw1)
    unconfig_snmp_agent_port_test(sw1)
    config_snmp_community_test(sw1)
    unconfig_snmp_community_test(sw1)
    config_snmp_system_test(sw1)
    unconfig_snmp_system_test(sw1)
    config_snmp_v1_trap_test(sw1)
    unconfig_snmp_v1_trap_test(sw1)
    config_snmp_v2c_trap_test(sw1)
    unconfig_snmp_v2c_trap_test(sw1)
    config_snmp_v3_auth_user_test(sw1)
    unconfig_snmp_v3_auth_user_test(sw1)
    config_snmp_v3_user_test(sw1)
    unconfig_snmp_v3_user_test(sw1)
    config_snmp_v3_trap_test(sw1)
    unconfig_snmp_v3_trap_test(sw1)
