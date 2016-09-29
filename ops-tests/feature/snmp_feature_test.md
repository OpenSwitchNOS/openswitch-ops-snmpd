# Snmp Feature Test Cases

## Contents

- [Verify SNMP Framework Mibs](#verify-snmp-framework-mibs)
    - [Objective](#objective)
    - [Priority](#priority)
    - [Requirements](#requirements)
    - [Setup](#setup)
        - [Topology diagram](#topology-diagram)
        - [Test setup](#test-setup)

## Verify SNMP Framework Mibs
### Objective
This test is to verify snmp framework mibs support.

### Priority
2- Medium

### Requirements
The requirements for this test case are:
 - Docker version 1.7 or above.
 - Accton AS5712 switch docker instance.

### Setup
#### Topology diagram
```ditaa

              +------------------+
              |                  |
              |  AS5712 switch   |
              |                  |
              +------------------+
```
#### Test setup
AS5712 switch instance or a VSI.

## Test case 1 - Verify snmpInPkts: The total number of messages delivered to the SNMP entity from the transport service.
### Description
Verify the number of messages delivered to the SNMP entity from the transport service by executing `snmpget -v2c -c public localhost snmpInPkts.0` on bash.
### Test result criteria
Verify by executing `snmpget -v2c -c public localhost snmpInPkts.0` on bash and see if the counter is incremented after every SNMP message delivered to the SNMP entity.
#### Test pass criteria
This test passes if the snmpInPkts counter is incremented after every SNMP message received by the SNMP entity.
#### Test fail criteria
This test fails if the snmpInPkts counter is not incremented after every SNMP message received by the SNMP entity.

## Test case 2 - Verify snmpOutPkts: The total number of SNMP Messages which were passed from the SNMP protocol entity to the transport service.
### Description
Verify the number of messages delivered by the SNMP entity to the transport service by executing `snmpget -v2c -c public localhost snmpOutPkts.0` on bash.
### Test result criteria
Verify by executing `snmpget -v2c -c public localhost snmpOutPkts.0`on bash and see if the counter is incremented after every snmp message delivered by the SNMP entity.
#### Test pass criteria
This test passes if the snmpOutPkts counter is incremented after every message delivered by the SNMP entity.
#### Test fail criteria
This test fails if the snmpOutPkts counter is not incremented after every message delivered by the SNMP entity.

## Test case 3 - Verify snmpInBadCommunityNames: The total number of community-based SNMP messages delivered to the SNMP entity which used an SNMP community name not known to said entity.
### Description
Verify the number of community-based SNMP messages delivered to the SNMP entity with an unknown community name by executing `snmpget -v2c -c public localhost snmpInBadCommunityNames.0` on bash.
### Test result criteria
Deliver a SNMP Get request with unknown community name. Verify by executing `snmpget -v2c -c public localhost snmpInBadCommunityNames.0` on bash and see if the counter is incremented after recieving community-based message with unknown community name.
#### Test pass criteria
This test passes if the snmpInBadCommunityNames counter is incremented when a community-based SNMP message is delivered to the SNMP entity with an unknown community name.
#### Test fail criteria
This test fails if the snmpInBadCommunityNames counter is not incremented when a community-based SNMP message is delivered to the SNMP entity with an unknown community name.

## Test case 4 - Verify snmpInTotalReqVars: The total number of MIB objects which have been retrieved successfully by the SNMP protocol entity as the result of receiving valid SNMP Get-Request and Get-Next PDUs.
### Description
Verify the number of MIB objects retieved successfully by the SNMP entity by executing `snmpget -v2c -c public localhost snmpInTotalReqVars.0` on bash.
### Test result criteria
Deliver a valid SNMP Get/Get-Next to the SNMP entity and verify by executing `snmpget -v2c -c public localhost snmpInTotalReqVars.0` on bash to check if the counter is incremented by the total number of MIB objects retrieved successfully.
#### Test pass criteria
This test passes if the snmpInTotalReqVars counter is incremented by the total number of MIB objects retrieved successfully on recieving a valid SNMP Get and Get-Next request.
#### Test fail criteria
This test fails if the snmpInTotalReqVars counter is not incremented by the total number of MIB objects retrieved successfully on recieving a valid SNMP Get and Get-Next request.

## Test case 5 - Verify snmpInGetRequests: The total number of SNMP Get-Request PDUs which have been accepted and processed by the SNMP protocol entity.
### Description
Verify the number of SNMP Get-Request messages accepted and processed by the SNMP entity by executing `snmpget -v2c -c public localhost snmpInGetRequests.0` on bash.
### Test result criteria
Deliver a valid SNMP Get request to the SNMP entity and verify by executing `snmpget -v2c -c public localhost snmpInGetRequests.0` on bash to check if the counter is incremented after every successful SNMP Get-Request processed by the SNMP entity.
#### Test pass criteria
This test passes if the snmpInGetRequests counter is incremented by the total number of SNMP Get-Requests processed successfully by the SNMP entity.
#### Test fail criteria
This test fails if the snmpInGetRequests counter is not incremented by the total number of SNMP Get-Requests processed successfully by the SNMP entity.

## Test case 6 - Verify snmpInGetNexts: The total number of SNMP Get-Next PDUs which have been accepted and processed by the SNMP protocol entity.
### Description
Verify the number of SNMP Get-Next messages accepted and processed by the SNMP entity by executing `snmpget -v2c -c public localhost snmpInGetNexts.0` on bash.
### Test result criteria
Deliver a valid SNMP Get-Next request to the SNMP entity and verify by executing `snmpget -v2c -c public localhost snmpInGetNexts.0` on bash to check if the counter is incremented after every successful SNMP Get-Next-Request processed by the SNMP entity.
#### Test pass criteria
This test passes if the snmpInGetNexts counter is incremented after every SNMP Get-Next-Requests processed successfully by the SNMP entity.
#### Test fail criteria
This test fails if the snmpInGetNexts counter is not incremented after every SNMP Get-Next-Requests processed successfully by the SNMP entity.

## Test case 7 - Verify snmpOutGetResponses: The total number of SNMP Get-Response PDUs which have been generated by the SNMP protocol entity.
### Description
Verify whether the number of Get-Response messages generated by the SNMP entity are incremented after every SNMP Get and GetNext -Response message generated by the SNMP entity by executing `snmpget -v2c -c public localhost snmpOutGetResponses.0` on bash.
### Test result criteria
Deliver a valid SNMP Get request and GetNext request to the SNMP entity and verify by executing `snmpget -v2c -c public localhost snmpOutGetResponses.0` on bash to check if the counter is incremented after every successful SNMP Get-Response message generated by the SNMP entity.
#### Test pass criteria
This test passes if the snmpOutGetResponses counter is incremented by the total number of SNMP Get-Response messages generated by the SNMP entity.
#### Test fail criteria
This test fails if the snmpOutGetResponses counter is not incremented by the total number of SNMP Get-Response messages generated by the SNMP entity.

## Test case 8 - Verify vacmGroupName: The name of the group to which this entry(combination of securityModel and securityName) belongs.
### Description
Verify whether the vacm Group is created for every SNMP V3 user by executing `snmpwalk -v3 -u v3user -l authPriv -a MD5 -A password -x DES -X password localhost vacmGroupName` on bash.
### Test result criteria
Create SNMP V3 user from vtysh and verify if vacm Group has been created for the V3 user by executing `snmpwalk -v3 -u v3user -l authPriv -a MD5 -A password -x DES -X password localhost vacmGroupName` on bash.
#### Test pass criteria
This test passes if the vacm group is created for all the SNMP v3 users.
#### Test fail criteria
This test fails if the vacm group is not created for all the SNMP v3 users.

## Test case 9 - Verify the support for SYSTEM group MIB (1.3.6.1.2.1.1)
### Description
Verify whether the system group objects are retrieved without any errors by executing `snmpwalk -v2c -c public localhost 1.3.6.1.2.1.1` on bash.
### Test result criteria
Verify if the snmpwalk on system group MIB is retrieved without errors by executing `snmpwalk -v2c -c public localhost 1.3.6.1.2.1.1` on bash.
#### Test pass criteria
This test passes if the snmpwalk on system group MIB 1.3.6.1.2.1.1 is retrieved without any errors.
#### Test fail criteria
This test fails if snmpwalk on system group MIB 1.3.6.1.2.1.1 fails with Timeout/Error In Packet/No such object available errors.

## Test case 10 - Verify the support for SNMP group MIB (1.3.6.1.2.1.11)
### Description
Verify whether the snmp group objects are retrieved without any errors by executing `snmpwalk -v2c -c public localhost 1.3.6.1.2.1.11` on bash.
### Test result criteria
Verify if the snmpwalk on snmp group MIB is retrieved without errors by executing `snmpwalk -v2c -c public localhost 1.3.6.1.2.1.11` on bash.
#### Test pass criteria
This test passes if the snmpwalk on snmp group MIB 1.3.6.1.2.1.11 is retrieved without any errors.
#### Test fail criteria
This test fails if snmpwalk on snmp group MIB 1.3.6.1.2.1.11 fails with Timeout/Error In Packet/No such object available errors.

## Test case 11 - Verify the support for VACM MIB (1.3.6.1.6.3.16)
### Description
Verify whether the vacm objects are retrieved without any errors by executing `snmpwalk -v2c -c public localhost 1.3.6.1.6.3.16` on bash.
### Test result criteria
Verify if the snmpwalk on vacm MIB is retrieved without errors by executing `snmpwalk -v2c -c public localhost 1.3.6.1.6.3.16` on bash.
#### Test pass criteria
This test passes if the snmpwalk on vacm MIB 1.3.6.1.6.3.16 is retrieved without any errors.
#### Test fail criteria
This test fails if snmpwalk on vacm MIB 1.3.6.1.6.3.16 fails with Timeout/Error In Packet/No such object available errors.

## Test case 12 - Verify the support for Net SNMP extended VACM MIB (1.3.6.1.4.1.8072.1.9)
### Description
Verify whether the net snmp extended vacm objects are retrieved without any errors by executing `snmpwalk -v2c -c public localhost 1.3.6.1.4.1.8072.1.9` on bash.
### Test result criteria
Verify if the snmpwalk on net snmp extended vacm MIB is retrieved without errors by executing `snmpwalk -v2c -c public localhost 1.3.6.1.4.1.8072.1.9` on bash.
#### Test pass criteria
This test passes if the snmpwalk on net snmp extended vacm MIB 1.3.6.1.4.1.8072.1.9 is retrieved without any errors.
#### Test fail criteria
This test fails if snmpwalk on net snmp extended vacm MIB 1.3.6.1.4.1.8072.1.9 fails with Timeout/Error In Packet/No such object available errors.
