# (C) Copyright 2016 Hewlett Packard Enterprise Development LP
#
#  Licensed under the Apache License, Version 2.0 (the "License"); you may
#  not use this file except in compliance with the License. You may obtain
#  a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
#  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
#  License for the specific language governing permissions and limitations
#  under the License.

cmake_minimum_required (VERSION 2.8)

project ("ops_snmptrap")

set (INCL_DIR ${CMAKE_SOURCE_DIR}/include)
set (LIBSNMPTRAP ops_snmptrap)

# Define compile flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -Wall -Werror")

add_definitions(-DHAVE_CONFIG_H -DHAVE_SOCKLEN_T)

# Rules to locate needed libraries
include(FindPkgConfig)
pkg_check_modules(OVSCOMMON REQUIRED libovscommon)
pkg_check_modules(OVSDB REQUIRED libovsdb)

include_directories (${INCL_DIR}
                     ${PROJECT_SOURCE_DIR}
                     ${OVSCOMMON_INCLUDE_DIRS}
                     ${OPSCLI_INCLUDE_DIRS}
                    )

# CLI libraries source files
set (SOURCES_SNMPTRAP ${PROJECT_SOURCE_DIR}/snmptrap_lib.c
     )

add_library (${LIBSNMPTRAP} SHARED ${SOURCES_SNMPTRAP})

#target_link_libraries (${LIBSNMPTRAP} ${OVSCOMMON_LIBRARIES} ${OVSDB_LIBRARIES}
#                        -lpthread -lrt -lnetsnmpmibs -lnetsnmpagent -lnetsnmp -lpci -ldl)

set(OPS_U_VER_MAJOR "1")
set(OPS_U_VER_MINOR "0")
set(OPS_U_VER_PATCH "0")
set(OPSUTILS_VERSION "${OPS_U_VER_MAJOR}.${OPS_U_VER_MINOR}.${OPS_U_VER_PATCH}")
set_target_properties(${LIBSNMPTRAP} PROPERTIES VERSION ${OPSUTILS_VERSION})


# Installation
install(TARGETS ${LIBSNMPTRAP}
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
       )
install(FILES ${PROJECT_SOURCE_DIR}/snmptrap_lib.h
        DESTINATION include)
