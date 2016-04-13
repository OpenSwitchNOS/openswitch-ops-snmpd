# OPS-SNMPD Design

## Contents

- [High-level design of ops-snmpd](#high-level-design-of-ops-snmpd)
    - [Scope](#scope)
    - [Responsibilities](#responsibilities)
- [Design choices](#design-choices)
- [Relationships to external OpenSwitch entities](#relationships-to-external-openswitch-entities)
- [Internal structure](#internal-structure)
- [Trap Design](#trap-design)

## High-level design of ops-snmpd

Open-source framework`Net-SNMP 5.7.2` is used to support SNMP operations. The master agent is provided by this package. The subagent is registered to the master agent. The subagent registers the handlers for various MIB modules. When a SNMP query comes in, the agent receives it and forwards the request to the subagent, and the query is serviced. This package is used as is and no source code in this package is modified.

The primary goal of the `ops-snmpd` module is to facilitate network management on OpenSwitch. The core component of this module is the `subagent`.

### Scope

The following components are supported:
- Multiple versions of the protocol such as SNMPv1, SNMPv2 and SNMPv3.

- SNMP read operations like get, getNext, and getBulk

- SNMP traps that have the ability to configure trap collectors through the CLI and so on.

### Responsibilities
To perform SNMP read operations.

## Design choices
The following are the design choices made for the `ops-snmpd` modules:
- `Net-SNMP 5.7.2` package -- Used to support SNMP operations.

- Port selection -- By default, the master agent listens on port '161' port for the queries from the user. and on port '/var/agentx/master' for subagents.

- The subagent:

    - Registers itself to the master agent.

    - Initializes all the MIB modules from the shared object file.

    - Interacts with the OVSDB to fetch the data.

    - Feature MIB files are placed in the feature repository under the  `snmp` directory.

    - Python script given by `ops-snmpd` is executed on the feature MIB and a mapping file to get all of the necessary .c and .h files.

- Shared object file -- Automatically created and placed in a specified path (`usr/lib/snmp/plugins`) during the build time.

- The .so files -- Opened and executed during the subagent boot.

## Relationships to external OpenSwitch entities

The subagent is only connected with the OVSDB server. Like the CLI or REST, there is no direct communication between other OpenSwitch daemons from the subagent.

## Internal structure

The following steps indicate what happens inside the `ops-snmpd` internal structure:

1. The script, for example ops-snmpgen.py, is auto-generated.

A modified pysmi framework is employed and used by pysnmp for autogenerating the Net-SNMP compliant C files. This was done mainly to the ease of development in Python and implement significant improvements.

The `net-snmp` repository does not need to be modified and can be used as is.

An MIB represents a certain data format. The objects in the OVSDB represent a different data format.
To support read operations on a given MIB, it is necessary to map the MIB objects to specific schema objects. The SNMP infrastructure requires a "mapping file", which in its simplest form could be thought of as a two column table where the first column would be a MIB object and the second column is its corresponding OVSDB schema object. The feature owner or developer is required to provide this mapping.

Mapping from the OVSDB to the MIB object using a JSON file is described in the following format:

- Scalar MIB object:

        "lldpMessageTxInterval": {  <-------------------- MIB scalar
        "MIBType": "Scalar",        <-------------------- MIB type
        "OvsTable": "system",       <-------------------- corresponding table in OVSDB schema
        "OvsColumn": "other_config", <------------------- corresponding OVSDB column
        "Type": {
            "Key": "lldp_tx_interval" <------------------ key if its a key-value pair in the schema
        },
        "CustomFunction": null       <------------------- convert function to convert from OVSDB
                                                          type to MIB object type

- Tabular MIB object:

        "lldpRemManAddrTable": {  <-------------------- MIB table
              "MIBType": "Table", <-------------------- MIB type
              "RootOvsTable": "interface", <----------- corresponding table in OVSDB schema
              "SkipFunction": null,  <----------------- function indicating 'skip logic'
              "CacheTimeout": 30, <-------------------- cache timeout for this MIB table
              "Indexes": { <--------------------------- MIB indexes
                     "lldpRemTimeMark": {
                           "OvsTable": null,  --------+
                           "OvsColumn": null,         |--> no corresponding schema object
                           "Type": {                  |
                                  "Key": null --------+
                           }
                           "CustomFunction" : null
                     },
                     "lldpRemLocalPortNum": {
                           "OvsTable": "interface",
                           "OvsColumn": "ifname",
                           "Type": {
                                  "Key": null
                           }
                           "CustomFunction" : null
                     },
                     .
                     .
                     .
                     .
                     .
                     .
              },
              "Columns": {
                     "lldpRemManAddrIfSubtype": {
                           "OvsTable": null,
                           "OvsColumn": null,
                           "Type": {
                                  "Key": null
                           }
                           "CustomFunction": null <----- type convert function located
                                                         in ops-utils/local repo,
                     },
                     .
                     .
                     .
                     .
              },
          }

- If the type of schema object is not a key-value pair, and it is the value of a column, then the key is NULL. For example, in the above template of a table, under the columns refer to 'lldpPortConfigPortNum'. This MIB object corresponds to the ifname column in the interface table. As it is not a key-value pair, the key is NULL.

- For any MIB object, if there is no schema equivalent, then a default value of the MIB type is returned which is given by 'CustomConvertFunctionName'. This default value is provided by the feature developer while defining the custom function.

- If the MIB type and schema type are different, a custom convert function will have to be defined which returns the appropriate type. This custom convert function name will already be mentioned in json. Its up to the feature developer to define this custom convert function to return either the default or the type casted value. This function resides under `ops-utils` or the feature repository directory, based on the generality of the convert function.

For example, in the above template, 'lldpRemTimeMark' has no corresponding entry in the OVSDB schema. The default value of this will be returned by the custom convert function.

- The 'CustomSkipFunctionName' points to a function for a skip logic.

For example, in the above MIB table template sample, the 'lldpRemManAddrTable' table corresponds to the OVSDB 'interface' table. The interface table contains all the physical interfaces along with the 'bridge_normal' interface which is logical. If we are only concerned about physical interfaces then the logical interfaces are skipped. The skip logic is given by the  'CustomSkipFunctionName' function.

```ditaa

        +-----------------+          +-----------------+
        | mapping between |          |                 |
        | OVSDB schema and|          |    MIB file     |
        | MIB object      |          |                 |
        +--------------+--+          +--+--------------+
                       |                |
                       |                |
                       |                |
                  +----v----------------v-+
                  |                       |
                  |      ops-snmpgen.py   |
                  |                       |
                  +----------+------------+
                             |
                             |
                             |
                             |
                             v

                      .c and .h files

```

File residency:

- The mapping and MIB files reside in the feature repository under the `<feature>/src/snmp/` directory.

- The script that generates the .c and .h files given a mapping file and the MIB, ops-snmpgen.py, resides in the `ops-snmpd` repository.

2. The plugin mechanism autoloads .so files.

- The .bb file of a feature supporting a MIB should have a dependency on the `ops-snmpd` repository.

For example,  if `ops-lldpd` is providing the SNMP functinality then, `ops-lldpd.bb` has to be modified by adding `ops-snmpd` under 'DEPENDS'. By doing this, `ops-snmpd` is built prior to `ops-lldpd`.

- At this point, you must create a .so file for all feature specific SNMP files.

- The feature specific SNMP shared object,`liblldp_snmp.so`, is a compiled object of feature specific SNMP files. All such features supporting MIB have a .so file at `/usr/lib/snmp/plugins` on the image.

- When a subagent starts it looks for the .so file in the preset path on the image and registers all the MIB modules.

```ditaa



                             /usr/lib/snmp/plugins/*
                                                  +-----+
                              +----------+        +     |
                              |          +------->      |
        +---------------+     |.so files |              |
        |               |     |for SNMP  +------->      |
        |   subagent    +-----+modules   |              | Register all MIB
        |               |     |          +------->      | handlers defined
        +---------------+     |          |              |
                              |          +------->      |
                              +----------+        +     |
                                                  +-----+



```

For example, the following processes occur if `ops-lldpd` is providing SNMP functionality:

- During the `ops-lldpd` build time, `liblldp_snmp.so` is generated and is copied to the `/usr/lib/snmp/plugins/` directory.

- The 'ops-snmpd' is built, and when the subagent boots it looks in the `/usr/lib/snmp/plugins/` directory for SNMP shared objects. The subagent then executes each .so file.

3. Runtime flow

- First, the SNMP query is received by the master agent.

- Then the master agent reassigns the query to the subagent.

- The subagent will then service the query by retrieving the data from the OVSDB.

```ditaa



                       +                       +
                       |                       |
                       |                       |
        +--------+     |                       |
        |Net-SNMP|     |                       |
        |  5.7.2 |     |                       |
        |master  |     |                       |
        |  agent |     |                       |
        |        |     |     ops-snmpd         |
        |        |     |    +--------+         |
        |        |     |    |        |         |     +----------------+
        |        |     |    |subagent|         |     |                |
        |        +---------->        +--------------->     OVSDB      |
        |        |     |    |        |         |     |                |
        |        |     |    |        |         |     |                |
        |        |     |    |        |         |     |                |
        |        |     |    |        <---------------+                |
        |        |     |    |        |         |     |                |
        |        |     |    |        |         |     |                |
        +---^----+     |    +--------+         |     +----------------+
            |recieve the                       |
            |query     |                       |
            |          |                       |
            |          +                       +
            |
            |
            +
        user query

```

## Trap Design

When a daemon wants to send traps it calls the library routine that is generated from the script. The daemon code is compiled with the library routine and the functions need to be called at the right places with the varbind arguments required to be sent in the trap. The library then gets the necessary SNMP Trap state variables from the DB and populates the packets with the varbinds and sends the traps to all the trap sinks.

The library routines for sending the trap is located in ops-snmpd repo. Currently we switch from swns namespace to global namespace in the routine since SNMP traps need to be sent on the management interface. We temporarily switch the namespace using the setns syscall, and then switch back to swns in LLDP.


```ditaa
        App trying to send SNMP Traps
    +------------------------------------+
    |                                    |
    |  App Routine       library Routine |            OVSDB
    | +-----------+     +--------------+ |        +------------+
    | |           |     | +----------+ | |        |            |
    | |           |     | |Get SNMP  <------------> SNMP Trap  |
    | |  Call the |     | |Trap state| | |        | State Info |
    | |  library  |     | |from DB   | | |        |            |
    | |  function +-----> +----------+ | |        +------------+
    | |  to send  |     | +----------+ | |
    | |  trap     |     | |Create PDU| | |
    | |           |     | |and send  | | |
    | |           |     | +----------+ | |
    | +-----------+     +--------------+ |
    |                           |        |
    +------------------------------------+
                                |
                                |
                                |
                                |
                                |
                        +-----v-----+
                        |  Trap or  |
                        |  Inform   |
                        |  sent     |
                        +-----------+
```
