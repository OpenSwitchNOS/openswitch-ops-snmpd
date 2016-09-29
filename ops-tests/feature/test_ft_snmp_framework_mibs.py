# -*- coding: utf-8 -*-
# (C) Copyright 2015 Hewlett Packard Enterprise Development LP
# All Rights Reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License"); you may
#    not use this file except in compliance with the License. You may obtain
#    a copy of the License at
#
#         http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
#    WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
#    License for the specific language governing permissions and limitations
#    under the License.
#
##########################################################################

"""
Test to verify SNMP Framework Mibs.
"""
from pytest import mark

TOPOLOGY = """
# +-------+
# |  ops1  |
# +-------+

# Nodes
[type=openswitch name="Switch 1"] ops1
"""


def snmpget(ops1, version, accesscontrol, host, oid, extrav3conf=None):
    if version is "v1":
        retstruct = ops1("snmpget -v1 -c" + accesscontrol + " " + host + " "
                         + oid + " ", shell='bash')
        return retstruct
    elif version is "v2c":
        retstruct = ops1("snmpget -v2c -c" + accesscontrol + " " + host + " "
                         + oid + " ", shell='bash')
        return retstruct
    elif version is "v3":
        retstruct = ops1("snmpget -v3 " + extrav3conf + " " + host + " " +
                         oid + " ", shell='bash')
        return retstruct


def snmpgetnext(ops1, version, accesscontrol, host, oid, extrav3conf=None):
    if version is "v1":
        retstruct = ops1("snmpgetnext -v1 -c" + accesscontrol + " " + host +
                         " " + oid + " ", shell='bash')
        return retstruct
    elif version is "v2c":
        retstruct = ops1("snmpgetnext -v2c -c" + accesscontrol + " " + host +
                         " " + oid + " ", shell='bash')
        return retstruct
    elif version is "v3":
        retstruct = ops1("snmpgetnext -v3 " + extrav3conf + " " + host + " "
                         + oid + " ", shell='bash')
        return retstruct


def snmpwalk(ops1, version, accesscontrol, host, oid, extrav3conf=None):
    if version is "v1":
        retstruct = ops1("snmpwalk -v1 -c" + accesscontrol + " " + host + " "
                         + oid + " ", shell='bash')
        return retstruct
    elif version is "v2c":
        retstruct = ops1("snmpwalk -v2c -c" + accesscontrol + " " + host + " "
                         + oid + " ", shell='bash')
        return retstruct
    elif version is "v3":
        retstruct = ops1("snmpwalk -v3 " + extrav3conf + " " + host + " "
                         + oid + " ", shell='bash')
        return retstruct


def snmpget_v1_test_snmpinpkts(ops1):
    retstruct = snmpget(ops1, "v1", "public", "localhost", "snmpInPkts.0")
    offset = retstruct.find(': ')
    inpkts_start = int(retstruct[offset+2:])
    for iter in range(3):
        retstruct = snmpget(ops1, "v1", "public", "localhost", "snmpInPkts.0")
    offset = retstruct.find(': ')
    inpkts_end = int(retstruct[offset+2:])
    inpkts_diff = inpkts_end-inpkts_start
    assert inpkts_diff == 3


def snmpget_v2c_test_snmpinpkts(ops1):
    retstruct = snmpget(ops1, "v2c", "public", "localhost", "snmpInPkts.0")
    offset = retstruct.find(': ')
    inpkts_start = int(retstruct[offset+2:])
    for iter in range(3):
        retstruct = snmpget(ops1, "v2c", "public", "localhost", "snmpInPkts.0")
    offset = retstruct.find(': ')
    inpkts_end = int(retstruct[offset+2:])
    inpkts_diff = inpkts_end-inpkts_start
    assert inpkts_diff == 3


def snmpget_v3_test_snmpinpkts(ops1):
    retstruct = snmpget(ops1, "v3", "None", "localhost", "snmpInPkts.0",
                        "-u testv3user -l authNoPriv -a md5 -A password")
    offset = retstruct.find(': ')
    inpkts_start = int(retstruct[offset+2:])
    for iter in range(3):
        retstruct = snmpget(ops1, "v3", "None", "localhost", "snmpInPkts.0",
                            "-u testv3user -l authNoPriv -a md5 -A password")
    offset = retstruct.find(': ')
    inpkts_end = int(retstruct[offset+2:])
    inpkts_diff = inpkts_end-inpkts_start
    assert inpkts_diff == 6


def snmpget_v1_test_snmpoutpkts(ops1):
    retstruct = snmpget(ops1, "v1", "public", "localhost", "snmpOutPkts.0")
    offset = retstruct.find(': ')
    outpkts_start = int(retstruct[offset+2:])
    for iter in range(3):
        retstruct = snmpget(ops1, "v1", "public", "localhost", "snmpOutPkts.0")
    offset = retstruct.find(': ')
    outpkts_end = int(retstruct[offset+2:])
    outpkts_diff = outpkts_end-outpkts_start
    assert outpkts_diff == 3


def snmpget_v2c_test_snmpoutpkts(ops1):
    retstruct = snmpget(ops1, "v2c", "public", "localhost",
                        "snmpOutPkts.0")
    offset = retstruct.find(': ')
    outpkts_start = int(retstruct[offset+2:])
    for iter in range(3):
        retstruct = snmpget(ops1, "v2c", "public", "localhost",
                            "snmpOutPkts.0")
    offset = retstruct.find(': ')
    outpkts_end = int(retstruct[offset+2:])
    outpkts_diff = outpkts_end-outpkts_start
    assert outpkts_diff == 3


def snmpget_v3_test_snmpoutpkts(ops1):
    retstruct = snmpget(ops1, "v3", "None", "localhost", "snmpOutPkts.0",
                        "-u testv3user -l authNoPriv -a md5 -A password")
    offset = retstruct.find(': ')
    outpkts_start = int(retstruct[offset+2:])
    for iter in range(3):
        retstruct = snmpget(ops1, "v3", "None", "localhost", "snmpOutPkts.0",
                            "-u testv3user -l authNoPriv -a md5 -A password")
    offset = retstruct.find(': ')
    outpkts_end = int(retstruct[offset+2:])
    outpkts_diff = outpkts_end-outpkts_start
    assert outpkts_diff == 3


def snmpget_v2c_test_snmpinbadcommunitynames(ops1):
    retstruct = snmpget(ops1, "v2c", "unknown", "localhost",
                        "snmpInBadCommunityNames.0")
    retstruct = snmpget(ops1, "v2c", "public", "localhost",
                        "snmpInBadCommunityNames.0")
    offset = retstruct.find(': ')
    badcommunity_counter = int(retstruct[offset+2:])
    assert badcommunity_counter == 6


def snmpget_v1_test_snmpintotalreqvars(ops1):
    retstruct = snmpget(ops1, "v1", "public", "localhost",
                        "snmpInTotalReqVars.0")
    offset = retstruct.find(': ')
    intotalreq_start = int(retstruct[offset+2:])
    for iter in range(3):
        retstruct = snmpget(ops1, "v1", "public", "localhost",
                            "snmpInTotalReqVars.0")
        retstruct = snmpgetnext(ops1, "v1", "public", "localhost",
                                "sysORDescr.1")
    retstruct = snmpget(ops1, "v1", "public", "localhost",
                        "snmpInTotalReqVars.0")
    offset = retstruct.find(': ')
    intotalreq_end = int(retstruct[offset+2:])
    intotalreq_diff = intotalreq_end-intotalreq_start
    assert intotalreq_diff == 7


def snmpget_v2c_test_snmpintotalreqvars(ops1):
    retstruct = snmpget(ops1, "v2c", "public", "localhost",
                        "snmpInTotalReqVars.0")
    offset = retstruct.find(': ')
    intotalreq_start = int(retstruct[offset+2:])
    for iter in range(3):
        retstruct = snmpget(ops1, "v2c", "public", "localhost",
                            "snmpInTotalReqVars.0")
        retstruct = snmpgetnext(ops1, "v2c", "public", "localhost",
                                "sysORDescr.1")
    retstruct = snmpget(ops1, "v2c", "public", "localhost",
                        "snmpInTotalReqVars.0")
    offset = retstruct.find(': ')
    intotalreq_end = int(retstruct[offset+2:])
    intotalreq_diff = intotalreq_end-intotalreq_start
    assert intotalreq_diff == 7


def snmpget_v3_test_snmpintotalreqvars(ops1):
    retstruct = snmpget(ops1, "v3", "None", "localhost",
                        "snmpInTotalReqVars.0", "-u testv3user -l authNoPriv"
                        " -a md5 -A password")
    offset = retstruct.find(': ')
    intotalreq_start = int(retstruct[offset+2:])
    for iter in range(3):
        retstruct = snmpgetnext(ops1, "v3", "None", "localhost",
                                "sysORDescr.1", "-u testv3user"
                                " -l authNoPriv -a md5 -A password")
        retstruct = snmpget(ops1, "v3", "None", "localhost",
                            "snmpInTotalReqVars.0", "-u testv3user"
                            " -l authNoPriv -a md5 -A password")
    retstruct = snmpget(ops1, "v3", "None", "localhost",
                        "snmpInTotalReqVars.0", "-u testv3user -l authNoPriv"
                        " -a md5 -A password")
    offset = retstruct.find(': ')
    intotalreq_end = int(retstruct[offset+2:])
    intotalreq_diff = intotalreq_end-intotalreq_start
    assert intotalreq_diff == 7


def snmpget_v1_test_snmpingetrequests(ops1):
    retstruct = snmpget(ops1, "v1", "public", "localhost",
                        "snmpInGetRequests.0")
    offset = retstruct.find(': ')
    ingetreq_start = int(retstruct[offset+2:])
    for iter in range(3):
        retstruct = snmpget(ops1, "v1", "public", "localhost",
                            "snmpInGetRequests.0")
    offset = retstruct.find(': ')
    ingetreq_end = int(retstruct[offset+2:])
    ingetreq_diff = ingetreq_end-ingetreq_start
    assert ingetreq_diff == 3


def snmpget_v2c_test_snmpingetrequests(ops1):
    retstruct = snmpget(ops1, "v2c", "public", "localhost",
                        "snmpInGetRequests.0")
    offset = retstruct.find(': ')
    ingetreq_start = int(retstruct[offset+2:])
    for iter in range(3):
        retstruct = snmpget(ops1, "v2c", "public", "localhost",
                            "snmpInGetRequests.0")
    offset = retstruct.find(': ')
    ingetreq_end = int(retstruct[offset+2:])
    ingetreq_diff = ingetreq_end-ingetreq_start
    assert ingetreq_diff == 3


def snmpget_v3_test_snmpingetrequests(ops1):
    retstruct = snmpget(ops1, "v3", "None", "localhost", "snmpInGetRequests.0",
                        "-u testv3user -l authNoPriv -a md5 -A password")
    offset = retstruct.find(': ')
    ingetreq_start = int(retstruct[offset+2:])
    for iter in range(3):
        retstruct = snmpget(ops1, "v3", "None", "localhost",
                            "snmpInGetRequests.0", "-u testv3user"
                            " -l authNoPriv -a md5 -A password")
    offset = retstruct.find(': ')
    ingetreq_end = int(retstruct[offset+2:])
    ingetreq_diff = ingetreq_end-ingetreq_start
    assert ingetreq_diff == 3


def snmpget_v1_test_snmpingetnexts(ops1):
    retstruct = snmpget(ops1, "v1", "public", "localhost", "snmpInGetNexts.0")
    offset = retstruct.find(': ')
    ingetnexts_start = int(retstruct[offset+2:])
    retstruct = snmpgetnext(ops1, "v1", "public", "localhost",
                            "sysORDescr.1")
    retstruct = snmpget(ops1, "v1", "public", "localhost", "snmpInGetNexts.0")
    offset = retstruct.find(': ')
    ingetnexts_end = int(retstruct[offset+2:])
    ingetnexts_diff = ingetnexts_end-ingetnexts_start
    assert ingetnexts_diff == 1


def snmpget_v2c_test_snmpingetnexts(ops1):
    retstruct = snmpget(ops1, "v2c", "public", "localhost", "snmpInGetNexts.0")
    offset = retstruct.find(': ')
    ingetnexts_start = int(retstruct[offset+2:])
    retstruct = snmpgetnext(ops1, "v2c", "public", "localhost",
                            "sysORDescr.1")
    retstruct = snmpget(ops1, "v2c", "public", "localhost", "snmpInGetNexts.0")
    offset = retstruct.find(': ')
    ingetnexts_end = int(retstruct[offset+2:])
    ingetnexts_diff = ingetnexts_end-ingetnexts_start
    assert ingetnexts_diff == 1


def snmpget_v3_test_snmpingetnexts(ops1):
    retstruct = snmpget(ops1, "v3", "None", "localhost", "snmpInGetNexts.0",
                        "-u testv3user -l authNoPriv -a md5 -A password")
    offset = retstruct.find(': ')
    ingetnexts_start = int(retstruct[offset+2:])
    retstruct = snmpgetnext(ops1, "v3", "None", "localhost",
                            "sysORDescr.1", "-u testv3user"
                            " -l authNoPriv -a md5 -A password")
    retstruct = snmpget(ops1, "v3", "None", "localhost", "snmpInGetNexts.0",
                        "-u testv3user -l authNoPriv -a md5 -A password")
    offset = retstruct.find(': ')
    ingetnexts_end = int(retstruct[offset+2:])
    ingetnexts_diff = ingetnexts_end-ingetnexts_start
    assert ingetnexts_diff == 1


def snmpget_v1_test_snmpoutgetresponses(ops1):
    retstruct = snmpget(ops1, "v1", "public", "localhost",
                        "snmpOutGetResponses.0")
    offset = retstruct.find(': ')
    outgetresp_start = int(retstruct[offset+2:])
    for iter in range(3):
        retstruct = snmpget(ops1, "v1", "public", "localhost",
                            "snmpOutGetResponses.0")
        retstruct = snmpgetnext(ops1, "v1", "public", "localhost",
                                "sysORDescr.1")
    retstruct = snmpget(ops1, "v1", "public", "localhost",
                        "snmpOutGetResponses.0")
    offset = retstruct.find(': ')
    outgetresp_end = int(retstruct[offset+2:])
    outgetresp_diff = outgetresp_end-outgetresp_start
    assert outgetresp_diff == 7


def snmpget_v2c_test_snmpoutgetresponses(ops1):
    retstruct = snmpget(ops1, "v2c", "public", "localhost",
                        "snmpOutGetResponses.0")
    offset = retstruct.find(': ')
    outgetresp_start = int(retstruct[offset+2:])
    for iter in range(3):
        retstruct = snmpget(ops1, "v2c", "public", "localhost",
                            "snmpOutGetResponses.0")
        retstruct = snmpgetnext(ops1, "v2c", "public", "localhost",
                                "sysORDescr.1")
    retstruct = snmpget(ops1, "v2c", "public", "localhost",
                        "snmpOutGetResponses.0")
    offset = retstruct.find(': ')
    outgetresp_end = int(retstruct[offset+2:])
    outgetresp_diff = outgetresp_end-outgetresp_start
    assert outgetresp_diff == 7


def snmpget_v3_test_snmpoutgetresponses(ops1):
    retstruct = snmpget(ops1, "v3", "None", "localhost",
                        "snmpOutGetResponses.0", "-u testv3user"
                        " -l authNoPriv -a md5 -A password")
    offset = retstruct.find(': ')
    outgetresp_start = int(retstruct[offset+2:])
    for iter in range(3):
        retstruct = snmpget(ops1, "v3", "None", "localhost",
                            "snmpOutGetResponses.0", "-u testv3user"
                            " -l authNoPriv -a md5 -A password")
        retstruct = snmpgetnext(ops1, "v3", "None", "localhost",
                                "sysORDescr.1", "-u testv3user"
                                " -l authNoPriv -a md5 -A password")
    retstruct = snmpget(ops1, "v3", "None", "localhost",
                        "snmpOutGetResponses.0", "-u testv3user"
                        " -l authNoPriv -a md5 -A password")
    offset = retstruct.find(': ')
    outgetresp_end = int(retstruct[offset+2:])
    outgetresp_diff = outgetresp_end-outgetresp_start
    assert outgetresp_diff == 7


def snmpget_v3_test_vacmgroupname(ops1):
    ops1('configure terminal')
    ops1("snmpv3 user vacmuser auth md5 auth password"
         " priv des priv password")
    ops1('end')
    retstruct = snmpwalk(ops1, "v3", "None", "localhost",
                         "vacmGroupName", "-u vacmuser"
                         " -l authPriv -a md5 -A password"
                         " -x des -X password")
    retstruct_lines = retstruct.splitlines()
    for line in retstruct_lines:
        if "SNMP-VIEW-BASED-ACM-MIB::vacmGroupName.3.\"vacmuser\"" in line:
            assert "STRING: grpvacmuser" in line
    retstruct = ops1('configure terminal')
    retstruct = ops1("no snmpv3 user vacmuser")
    ops1('end')


def snmpwalk_v1_test_systemgroupmib(ops1):
    retstruct = snmpwalk(ops1, "v1", "public", "localhost",
                         "1.3.6.1.2.1.1")
    retstruct_lines = retstruct.splitlines()
    for line in retstruct_lines:
        assert "Error in packet" not in line, "General failure accured"
    for line in retstruct_lines:
        assert "Timeout: No Response" not in line, "snmpwalk timed out"


def snmpwalk_v2_test_systemgroupmib(ops1):
    retstruct = snmpwalk(ops1, "v2c", "public", "localhost",
                         "1.3.6.1.2.1.1")
    retstruct_lines = retstruct.splitlines()
    for line in retstruct_lines:
        assert "Error in packet" not in line, "General failure accured"
    for line in retstruct_lines:
        assert "Timeout: No Response" not in line, "snmpwalk timed out"


def snmpwalk_v3_test_systemgroupmib(ops1):
    retstruct = snmpwalk(ops1, "v3", "None", "localhost", "1.3.6.1.2.1.1",
                         "-u testv3user -l authNoPriv -a md5 -A password")
    retstruct_lines = retstruct.splitlines()
    for line in retstruct_lines:
        assert "Error in packet" not in line, "General failure accured"
    for line in retstruct_lines:
        assert "Timeout: No Response" not in line, "snmpwalk timed out"


def snmpwalk_v1_test_snmpgroupmib(ops1):
    retstruct = snmpwalk(ops1, "v1", "public", "localhost",
                         "1.3.6.1.2.1.11")
    retstruct_lines = retstruct.splitlines()
    for line in retstruct_lines:
        assert "Error in packet" not in line, "General failure accured"
    for line in retstruct_lines:
        assert "Timeout: No Response" not in line, "snmpwalk timed out"


def snmpwalk_v2_test_snmpgroupmib(ops1):
    retstruct = snmpwalk(ops1, "v2c", "public", "localhost",
                         "1.3.6.1.2.1.11")
    retstruct_lines = retstruct.splitlines()
    for line in retstruct_lines:
        assert "Error in packet" not in line, "General failure accured"
    for line in retstruct_lines:
        assert "Timeout: No Response" not in line, "snmpwalk timed out"


def snmpwalk_v3_test_snmpgroupmib(ops1):
    retstruct = snmpwalk(ops1, "v3", "None", "localhost", "1.3.6.1.2.1.11",
                         "-u testv3user -l authNoPriv -a md5 -A password")
    retstruct_lines = retstruct.splitlines()
    for line in retstruct_lines:
        assert "Error in packet" not in line, "General failure accured"
    for line in retstruct_lines:
        assert "Timeout: No Response" not in line, "snmpwalk timed out"


def snmpwalk_v1_test_vacmgroupmib(ops1):
    retstruct = snmpwalk(ops1, "v1", "public", "localhost",
                         "1.3.6.1.6.3.16")
    retstruct_lines = retstruct.splitlines()
    for line in retstruct_lines:
        assert "Error in packet" not in line, "General failure accured"
    for line in retstruct_lines:
        assert "Timeout: No Response" not in line, "snmpwalk timed out"


def snmpwalk_v2_test_vacmgroupmib(ops1):
    retstruct = snmpwalk(ops1, "v2c", "public", "localhost",
                         "1.3.6.1.6.3.16")
    retstruct_lines = retstruct.splitlines()
    for line in retstruct_lines:
        assert "Error in packet" not in line, "General failure accured"
    for line in retstruct_lines:
        assert "Timeout: No Response" not in line, "snmpwalk timed out"


def snmpwalk_v3_test_vacmgroupmib(ops1):
    retstruct = snmpwalk(ops1, "v3", "None", "localhost", "1.3.6.1.6.3.16",
                         "-u testv3user -l authNoPriv -a md5 -A password")
    retstruct_lines = retstruct.splitlines()
    for line in retstruct_lines:
        assert "Error in packet" not in line, "General failure accured"
    for line in retstruct_lines:
        assert "Timeout: No Response" not in line, "snmpwalk timed out"


def snmpwalk_v1_test_nsvacmgroupmib(ops1):
    retstruct = snmpwalk(ops1, "v1", "public", "localhost",
                         "1.3.6.1.4.1.8072.1.9")
    retstruct_lines = retstruct.splitlines()
    for line in retstruct_lines:
        assert "Error in packet" not in line, "General failure accured"
    for line in retstruct_lines:
        assert "Timeout: No Response" not in line, "snmpwalk timed out"


def snmpwalk_v2_test_nsvacmgroupmib(ops1):
    retstruct = snmpwalk(ops1, "v2c", "public", "localhost",
                         "1.3.6.1.4.1.8072.1.9")
    retstruct_lines = retstruct.splitlines()
    for line in retstruct_lines:
        assert "Error in packet" not in line, "General failure accured"
    for line in retstruct_lines:
        assert "Timeout: No Response" not in line, "snmpwalk timed out"


def snmpwalk_v3_test_nsvacmgroupmib(ops1):
    retstruct = snmpwalk(ops1, "v3", "None", "localhost",
                         "1.3.6.1.4.1.8072.1.9",
                         "-u testv3user -l authNoPriv -a md5 -A password")
    retstruct_lines = retstruct.splitlines()
    for line in retstruct_lines:
        assert "Error in packet" not in line, "General failure accured"
    for line in retstruct_lines:
        assert "Timeout: No Response" not in line, "snmpwalk timed out"


def config(ops1):
    with ops1.libs.vtysh.Configure() as ctx:
        ctx.snmpv3_user_auth_auth_pass('testv3user',
                                       auth_protocol='md5',
                                       auth_password='password')
    result = ops1.libs.vtysh.show_snmpv3_users()
    assert 'testv3user' in result
    assert result['testv3user']['AuthMode'] == 'md5'


def unconfig(ops1):
    with ops1.libs.vtysh.Configure() as ctx:
        ctx.no_snmpv3_user_auth_auth_pass('testv3user', auth_protocol='md5',
                                          auth_password='password')


@mark.timeout(600)
def test_ft_snmp_framework_mibs(topology, step):
    ops1 = topology.get("ops1")

    config(ops1)

    snmpget_v1_test_snmpinpkts(ops1)
    snmpget_v2c_test_snmpinpkts(ops1)
    snmpget_v3_test_snmpinpkts(ops1)
    snmpget_v1_test_snmpoutpkts(ops1)
    snmpget_v2c_test_snmpoutpkts(ops1)
    snmpget_v3_test_snmpoutpkts(ops1)
    snmpget_v2c_test_snmpinbadcommunitynames(ops1)
    snmpget_v1_test_snmpintotalreqvars(ops1)
    snmpget_v2c_test_snmpintotalreqvars(ops1)
    snmpget_v3_test_snmpintotalreqvars(ops1)
    snmpget_v1_test_snmpingetrequests(ops1)
    snmpget_v2c_test_snmpingetrequests(ops1)
    snmpget_v3_test_snmpingetrequests(ops1)
    snmpget_v1_test_snmpingetnexts(ops1)
    snmpget_v2c_test_snmpingetnexts(ops1)
    snmpget_v3_test_snmpingetnexts(ops1)
    snmpget_v1_test_snmpoutgetresponses(ops1)
    snmpget_v2c_test_snmpoutgetresponses(ops1)
    snmpget_v3_test_snmpoutgetresponses(ops1)
    snmpget_v3_test_vacmgroupname(ops1)
    snmpwalk_v1_test_systemgroupmib(ops1)
    snmpwalk_v2_test_systemgroupmib(ops1)
    snmpwalk_v3_test_systemgroupmib(ops1)
    snmpwalk_v1_test_snmpgroupmib(ops1)
    snmpwalk_v2_test_snmpgroupmib(ops1)
    snmpwalk_v3_test_snmpgroupmib(ops1)
    snmpwalk_v1_test_vacmgroupmib(ops1)
    snmpwalk_v2_test_vacmgroupmib(ops1)
    snmpwalk_v3_test_vacmgroupmib(ops1)
    snmpwalk_v1_test_nsvacmgroupmib(ops1)
    snmpwalk_v2_test_nsvacmgroupmib(ops1)
    snmpwalk_v3_test_nsvacmgroupmib(ops1)

    unconfig(ops1)
