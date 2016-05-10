# LLDP Test Cases


## Contents



- [Supportability](#cli)
	- [VLOG Messages](#vlog-messages)
	- [Show Tech](#show-tech)

##CLI
### VLOG Messages
#### Objective
This test case validates if VLOG messages are being printed by ops-snmpd.
#### Requirements
- Physical Switch/Switch Test setup
- **FT File**: `ops-snmpd/ops-tests/feature/test_snmp_ft_supportability.py`(SNMP supportability)

#### Setup
##### Topology diagram
```ditaa
    +--------+
    |        |
    |   S1   |
    |        |
    +--------+
```

#### Description
1. Tabe a switch and set the logging level in snmpd to dbg using ovs-appctl.
2. Configure **snmp** by add v2 communities and v3 users.
3. Check if the Debug messages are printed in /var/log/messages for snmpd plugins loading and the configuration changes.

### Show Tech
#### Objective
This test case confirms that there is Show Tech support for snmp.
#### Requirements
- Physical Switch/Switch Test setup
- **FT File**: `ops-snmpd/ops-tests/feature/test_snmp_ft_supportability.py`(SNMP supportability)

#### Setup
##### Topology diagram
```ditaa
    +--------+
    |        |
    |   S1   |
    |        |
    |        |
    +--------+
```

#### Description
1. Take a switch and run show tech in cli and confirm there is support for SNMP.