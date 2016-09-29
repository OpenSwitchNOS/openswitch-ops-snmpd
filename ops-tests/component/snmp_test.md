# SNMP component Test Cases

## Contents

- [Verify SNMP configurations](#verify-snmp-configurations)
    - [Objective](#objective)
    - [Requirements](#requirements)
    - [Setup](#setup)
        - [Topology diagram](#topology-diagram)
        - [Test setup](#test-setup)

## Verify SNMP configurations
### Objective
To configure snmp agent port and verify the entry in OVSDB.
### Requirements
The requirements for this test case are:
 - Docker version 1.7 or above.
 - Accton AS5712 switch docker instance.

## Setup
### Topology diagram
```ditaa

              +------------------+
              |                  |
              |  AS5712 switch   |
              |                  |
              +------------------+
```
### Test setup
AS5712 switch instance or a VSI.

## Test case 1 - Verify config/ unconfig of SNMP agent port
### Description
Verify whether or not the snmp agent port is configured using `snmp-server agent-port <1-65535>` command.
### Test result criteria
Verify using `show snmp agent port` command.
#### Test pass criteria
This test passes if the snmp agent port key is set in system table.
#### Test fail criteria
This test fails if the snmp agent port key is not set in system table.

### Negative test :

### Description
Verify whether the invalid port number is rejected or not using `snmp-server agent <1-65535>`. Invalid inputs can be a number outside the range <1-65535>.
### Test result criteria
Verify using `show snmp agent port` command.
#### Test pass criteria
This test passes if the snmp agent port configuration is rejected.
#### Test fail criteria
This test fails if the invalid input is accepted and configured.


## Test case 2 : Verify config/ unconfig of SNMP communities.
### Description
Verify whether or not the snmp communities is configured using `snmp-server community WORD` command.
### Test result criteria
Verify using `show snmp community` command.
#### Test pass criteria
This test passes if the snmp communities is updated in system table.
#### Test fail criteria
This test fails if the snmp communities is not updated in system table.


## Test case 3 : Verify config/ unconfig of sytem description.
### Description
Verify whether or not the system description is configured using `snmp-server system-description .LINE` command.
### Test result criteria
Verify using `show snmp system` command.
#### Test pass criteria
This test passes if the system description key is set in system table.
#### Test fail criteria
This test fails if the system description key is not set in system table.

## Test case 4 : Verify config/ unconfig of sytem contact.
### Description
Verify whether or not the system contact is configured using `snmp-server system-contact .LINE` command.
### Test result criteria
Verify using `show snmp system` command.
#### Test pass criteria
This test passes if the system contact key is set in system table.
#### Test fail criteria
This test fails if the system contact key is not set in system table.

## Test case 5 : Verify config/ unconfig of sytem location.
### Description
Verify whether or not the system location is configured using `snmp-server system-location .LINE` command.
### Test result criteria
Verify using `show snmp system` command.
#### Test pass criteria
This test passes if the system location key is set in system table.
#### Test fail criteria
This test fails if the system location key is not set in system table.

## Test case 6 : Verify config/ unconfig of snmp version 1 traps.
### Description
Verify whether or not the snmp version 1 trap is configured using `snmp-server host <ip_address> trap version v1 [community WORD | port <1-65535>]` command.
### Test result criteria
Verify using `show snmp trap` command.
#### Test pass criteria
This test passes if an entry is created in snmp_trap table.
#### Test fail criteria
This test fails if if an entry is not created in snmp_trap table.

### Negative test :

### Description
Verify whether the invalid IP is rejected or not using `snmp-server host <ip_address> trap version v1 [community WORD | port <1-65535>]`. Invalid inputs can be a broadcast/global unicast IP address.
### Test result criteria
Verify using `show snmp trap` command.
#### Test pass criteria
This test passes if the configuration is rejected.
#### Test fail criteria
This test fails if the invalid input is accepted and configured.

## Test case 7 : Verify config/ unconfig of snmp version 2 traps/informs.
### Description
Verify whether or not the snmp version 2 trap is configured using `snmp-server host <ip_address> (trap|inform) version v2c [community WORD | port <1-65535>]` command.
### Test result criteria
Verify using `show snmp trap` command.
#### Test pass criteria
This test passes if an entry is created in snmp_trap table.
#### Test fail criteria
This test fails if if an entry is not created in snmp_trap table.

Verify whether the invalid IP is rejected or not using `snmp-server host <ip_address> trap version v2c [community WORD | port <1-65535>]`. Invalid inputs can be a broadcast/global unicast IP address.
### Test result criteria
Verify using `show snmp trap` command.
#### Test pass criteria
This test passes if the configuration is rejected.
#### Test fail criteria
This test fails if the invalid input is accepted and configured.

## Test case 8 : Verify config/ unconfig of snmp version 3 traps/informs.
### Description
Verify whether or not the snmp version 3 trap is configured using `snmp-server host <ip_address> (trap|inform) version v3 [user WORD | port <1-65535>]` command.
### Test result criteria
Verify using `show snmp trap` command.
#### Test pass criteria
This test passes if an entry is created in snmp_trap table.
#### Test fail criteria
This test fails if if an entry is not created in snmp_trap table.

Verify whether the invalid IP snd user is rejected or not using `snmp-server host <ip_address> trap version v3 user WORD [port <1-65535>]`. Invalid inputs can be a broadcast/global unicast IP address.
invalid user is an non-existing v3 user.
### Test result criteria
Verify using `show snmp trap` command.
#### Test pass criteria
This test passes if the configuration is rejected.
#### Test fail criteria
This test fails if the invalid input is accepted and configured.

## Test case 9 : Verify config/ unconfig of snmp version 3 users.
### Description
Verify whether or not the snmp version 3 user is configured using `snmpv3 user WORD [auth (md5|sha) auth-pass WORD [priv (aes | des) priv-pass WORD]]` command.
### Test result criteria
Verify using `show snmpv3 user` command.
#### Test pass criteria
This test passes if an entry is created snmpv3 user table.
#### Test fail criteria
This test fails if if an entry is not created in snmpv3 user table.

## Test case 10 : Verify running-configuration after config/unconfig of snmp system parameters.
### Description
Verify whether or not the running-configuration is updated after config/unconfig of snmp system parameters using `show running-configuration` command
### Test result criteria
Configure snmp server system-description, system-location, system-contact using `snmp-server system-description .LINE` , `snmp-server system-contact .LINE` and `snmp-server system-location .LINE` commands. Verify running-configuration using `show running-configuration` command. Unconfigure snmp system parameters using no form of the commands. Verify running-configuration.
#### Test pass criteria
This test passes if 'show running-configuration' output has the snmp system configuration commands after configuring the snmp system parameters and if `show running-configuration' output does not have the snmp system configuration commands after unconfiguring the snmp system parameters.
#### Test fail criteria
This test fails if 'show running-configuration' output does not have the snmp system configuration commands after configuring the snmp system parameters . This test also fails if `show running-configuration' output has the snmp system configuration commands after unconfiguring the snmp system parameters.