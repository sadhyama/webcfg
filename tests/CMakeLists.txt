#   Copyright 2020 Comcast Cable Communications Management, LLC
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -W -g -fprofile-arcs -ftest-coverage -O0")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DTEST ")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -W  -g -fprofile-arcs -ftest-coverage -O0")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage -O0")

if (WEBCONFIG_BIN_SUPPORT)
add_definitions(-DWEBCONFIG_BIN_SUPPORT)
endif (WEBCONFIG_BIN_SUPPORT)

if (WAN_FAILOVER_SUPPORTED)
add_definitions(-DWAN_FAILOVER_SUPPORTED)
endif (WAN_FAILOVER_SUPPORTED)

if(NOT DISABLE_VALGRIND)
set (MEMORY_CHECK valgrind --leak-check=full --show-reachable=yes --track-origins=yes -v)
endif ()

link_directories ( ${LIBRARY_DIR} )
#-------------------------------------------------------------------------------
#   webcfgCli
#-------------------------------------------------------------------------------
set(SOURCES webcfgCli.c ../src/webcfg_helpers.c ../src/webcfg.c ../src/webcfg_param.c ../src/webcfg_pack.c ../src/webcfg_multipart.c ../src/webcfg_auth.c ../src/webcfg_notify.c ../src/webcfg_db.c ../src/webcfg_generic_pc.c ../src/webcfg_blob.c ../src/webcfg_event.c ../src/webcfg_metadata.c ../src/webcfg_timer.c)

if (WEBCONFIG_BIN_SUPPORT)
set(SOURCES ${SOURCES} ../src/webcfg_rbus.c ../src/webcfg_method.c)
endif (WEBCONFIG_BIN_SUPPORT)

if (FEATURE_SUPPORT_AKER)
set(SOURCES ${SOURCES} ../src/webcfg_client.c ../src/webcfg_aker.c)
endif (FEATURE_SUPPORT_AKER)

add_executable(webcfgCli ${SOURCES})
target_link_libraries (webcfgCli -lmsgpackc -lcurl -lpthread  -lm -luuid -ltrower-base64 -lwdmp-c -lcimplog -lcjson -lwrp-c)

if (WEBCONFIG_BIN_SUPPORT)
target_link_libraries (webcfgCli -lrbus)
endif (WEBCONFIG_BIN_SUPPORT)

if (FEATURE_SUPPORT_AKER)
target_link_libraries (webcfgCli -llibparodus -lnanomsg)
endif (FEATURE_SUPPORT_AKER)

#-------------------------------------------------------------------------------
#   test_webcfgparam
#-------------------------------------------------------------------------------
add_test(NAME test_webcfgparam COMMAND ${MEMORY_CHECK} ./test_webcfgparam)
add_executable(test_webcfgparam test_webcfgparam.c ../src/webcfg_param.c ../src/webcfg_helpers.c)
target_link_libraries (test_webcfgparam -lcunit -lmsgpackc -lcimplog -lcjson)

target_link_libraries (test_webcfgparam gcov -Wl,--no-as-needed )

#-------------------------------------------------------------------------------
#   test_webcfgpack
#-------------------------------------------------------------------------------
add_test(NAME test_webcfgpack COMMAND ${MEMORY_CHECK} ./test_webcfgpack)
add_executable(test_webcfgpack test_webcfgpack.c ../src/webcfg_param.c ../src/webcfg_pack.c ../src/webcfg_helpers.c)
target_link_libraries (test_webcfgpack -lcunit -lmsgpackc -lcimplog -ltrower-base64 )

target_link_libraries (test_webcfgpack gcov -Wl,--no-as-needed )

#-------------------------------------------------------------------------------
#   test_multipart
#-------------------------------------------------------------------------------
add_test(NAME test_multipart COMMAND ${MEMORY_CHECK} ./test_multipart)

set(SOURCES test_multipart.c ../src/webcfg_param.c ../src/webcfg_multipart.c ../src/webcfg_helpers.c ../src/webcfg.c ../src/webcfg_auth.c ../src/webcfg_notify.c ../src/webcfg_db.c ../src/webcfg_pack.c ../src/webcfg_blob.c ../src/webcfg_event.c ../src/webcfg_metadata.c ../src/webcfg_timer.c ../src/webcfg_generic_pc.c )

if (WEBCONFIG_BIN_SUPPORT)
set(SOURCES ${SOURCES} ../src/webcfg_rbus.c ../src/webcfg_method.c)
endif (WEBCONFIG_BIN_SUPPORT)

if (FEATURE_SUPPORT_AKER)
set(SOURCES ${SOURCES} ../src/webcfg_client.c ../src/webcfg_aker.c)
endif (FEATURE_SUPPORT_AKER)

add_executable(test_multipart ${SOURCES})
target_link_libraries (test_multipart -lcunit -lmsgpackc -lcurl -lpthread  -lm -luuid -ltrower-base64 -lwdmp-c -lcimplog -lcjson -lwrp-c)

if (WEBCONFIG_BIN_SUPPORT)
target_link_libraries (test_multipart -lrbus)
endif (WEBCONFIG_BIN_SUPPORT)

if (FEATURE_SUPPORT_AKER)
target_link_libraries (test_multipart -llibparodus -lnanomsg)
endif (FEATURE_SUPPORT_AKER)

target_link_libraries (test_multipart gcov -Wl,--no-as-needed )
#-------------------------------------------------------------------------------
#   test_multipart_supplementary
#-------------------------------------------------------------------------------
add_test(NAME test_mul_supp COMMAND ${MEMORY_CHECK} ./test_mul_supp)
set(SOURCES test_mul_supp.c ../src/webcfg_param.c ../src/webcfg_multipart.c ../src/webcfg_helpers.c ../src/webcfg.c ../src/webcfg_auth.c ../src/webcfg_notify.c ../src/webcfg_db.c ../src/webcfg_pack.c ../src/webcfg_blob.c ../src/webcfg_event.c ../src/webcfg_generic_pc.c ../src/webcfg_metadata.c ../src/webcfg_timer.c)

if (WEBCONFIG_BIN_SUPPORT)
set(SOURCES ${SOURCES} ../src/webcfg_rbus.c ../src/webcfg_method.c)
endif (WEBCONFIG_BIN_SUPPORT)

if (FEATURE_SUPPORT_AKER)
set(SOURCES ${SOURCES} ../src/webcfg_client.c ../src/webcfg_aker.c)
endif (FEATURE_SUPPORT_AKER)

add_executable(test_mul_supp ${SOURCES})
target_link_libraries (test_mul_supp -lcunit -lmsgpackc -lcurl -lpthread  -lm -luuid -ltrower-base64 -lwdmp-c -lcimplog -lcjson -lwrp-c)

if (WEBCONFIG_BIN_SUPPORT)
target_link_libraries (test_mul_supp -lrbus)
endif (WEBCONFIG_BIN_SUPPORT)

if (FEATURE_SUPPORT_AKER)
target_link_libraries (test_mul_supp -llibparodus -lnanomsg)
endif (FEATURE_SUPPORT_AKER)

target_link_libraries (test_mul_supp gcov -Wl,--no-as-needed )
#-------------------------------------------------------------------------------
#   test_events
#-------------------------------------------------------------------------------
add_test(NAME test_events COMMAND ${MEMORY_CHECK} ./test_events)
set(SOURCES test_events.c ../src/webcfg_param.c ../src/webcfg_multipart.c ../src/webcfg_helpers.c ../src/webcfg.c ../src/webcfg_auth.c ../src/webcfg_notify.c ../src/webcfg_db.c ../src/webcfg_pack.c ../src/webcfg_blob.c ../src/webcfg_event.c ../src/webcfg_generic_pc.c ../src/webcfg_metadata.c ../src/webcfg_timer.c )

if (WEBCONFIG_BIN_SUPPORT)
set(SOURCES ${SOURCES} ../src/webcfg_rbus.c ../src/webcfg_method.c)
endif (WEBCONFIG_BIN_SUPPORT)

if (FEATURE_SUPPORT_AKER)
set(SOURCES ${SOURCES} ../src/webcfg_client.c ../src/webcfg_aker.c)
endif (FEATURE_SUPPORT_AKER)

add_executable(test_events ${SOURCES})

target_link_libraries (test_events -lcunit -lmsgpackc -lcurl -lpthread  -lm -luuid -ltrower-base64 -lwdmp-c -lcimplog -lcjson -lwrp-c)

if (WEBCONFIG_BIN_SUPPORT)
target_link_libraries (test_events -lrbus)
endif (WEBCONFIG_BIN_SUPPORT)

if (FEATURE_SUPPORT_AKER)
target_link_libraries (test_events -llibparodus -lnanomsg)
endif (FEATURE_SUPPORT_AKER)

target_link_libraries (test_events gcov -Wl,--no-as-needed )

#-------------------------------------------------------------------------------
#   test_events_supplematary
#-------------------------------------------------------------------------------
add_test(NAME test_events_supp COMMAND ${MEMORY_CHECK} ./test_events_supp)

set(SOURCES test_events_supp.c ../src/webcfg_param.c ../src/webcfg_multipart.c ../src/webcfg_helpers.c ../src/webcfg.c ../src/webcfg_auth.c ../src/webcfg_notify.c ../src/webcfg_db.c ../src/webcfg_pack.c ../src/webcfg_blob.c ../src/webcfg_event.c ../src/webcfg_generic_pc.c ../src/webcfg_metadata.c ../src/webcfg_timer.c)

if (WEBCONFIG_BIN_SUPPORT)
set(SOURCES ${SOURCES} ../src/webcfg_rbus.c ../src/webcfg_method.c)
endif (WEBCONFIG_BIN_SUPPORT)

if (FEATURE_SUPPORT_AKER)
set(SOURCES ${SOURCES} ../src/webcfg_client.c ../src/webcfg_aker.c)
endif (FEATURE_SUPPORT_AKER)

add_executable(test_events_supp  ${SOURCES})
target_link_libraries (test_events_supp -lcunit -lmsgpackc -lcurl -lpthread  -lm -luuid -ltrower-base64 -lwdmp-c -lcimplog -lcjson -lwrp-c)

if (WEBCONFIG_BIN_SUPPORT)
target_link_libraries (test_events_supp -lrbus)
endif (WEBCONFIG_BIN_SUPPORT)

if (FEATURE_SUPPORT_AKER)
target_link_libraries (test_events_supp -llibparodus -lnanomsg)
endif (FEATURE_SUPPORT_AKER)

target_link_libraries (test_events_supp gcov -Wl,--no-as-needed )


#-------------------------------------------------------------------------------
#   test_webcfgevents
#-------------------------------------------------------------------------------
add_test(NAME test_webcfgevents COMMAND ${MEMORY_CHECK} ./test_webcfgevents)
set(SOURCES test_webcfgevents.c ../src/webcfg_param.c ../src/webcfg_multipart.c ../src/webcfg_helpers.c ../src/webcfg.c ../src/webcfg_auth.c ../src/webcfg_notify.c ../src/webcfg_db.c ../src/webcfg_pack.c ../src/webcfg_blob.c ../src/webcfg_event.c ../src/webcfg_generic_pc.c ../src/webcfg_metadata.c ../src/webcfg_timer.c )

if (WEBCONFIG_BIN_SUPPORT)
set(SOURCES ${SOURCES} ../src/webcfg_rbus.c ../src/webcfg_method.c)
endif (WEBCONFIG_BIN_SUPPORT)

if (FEATURE_SUPPORT_AKER)
set(SOURCES ${SOURCES} ../src/webcfg_client.c ../src/webcfg_aker.c)
endif (FEATURE_SUPPORT_AKER)

add_executable(test_webcfgevents ${SOURCES})

target_link_libraries (test_webcfgevents -lcunit -lmsgpackc -lcurl -lpthread  -lm -luuid -ltrower-base64 -lwdmp-c -lcimplog -lcjson -lwrp-c)

if (WEBCONFIG_BIN_SUPPORT)
target_link_libraries (test_webcfgevents -lrbus)
endif (WEBCONFIG_BIN_SUPPORT)

if (FEATURE_SUPPORT_AKER)
target_link_libraries (test_webcfgevents -llibparodus -lnanomsg)
endif (FEATURE_SUPPORT_AKER)

target_link_libraries (test_webcfgevents gcov -Wl,--no-as-needed )

#-------------------------------------------------------------------------------
#   test_metadata
#-------------------------------------------------------------------------------
add_test(NAME test_metadata COMMAND ${MEMORY_CHECK} ./test_metadata)
add_executable(test_metadata test_metadata.c ../src/webcfg_metadata.c)
target_link_libraries (test_metadata -lcunit)

target_link_libraries (test_metadata gcov -Wl,--no-as-needed )

#-------------------------------------------------------------------------------
#   test_root
#-------------------------------------------------------------------------------
add_test(NAME test_root COMMAND ${MEMORY_CHECK} ./test_root)

set(SOURCES test_root.c ../src/webcfg_param.c ../src/webcfg_multipart.c ../src/webcfg_helpers.c ../src/webcfg.c ../src/webcfg_auth.c ../src/webcfg_notify.c ../src/webcfg_db.c ../src/webcfg_pack.c ../src/webcfg_blob.c ../src/webcfg_event.c ../src/webcfg_generic_pc.c ../src/webcfg_metadata.c ../src/webcfg_timer.c)

if (WEBCONFIG_BIN_SUPPORT)
set(SOURCES ${SOURCES} ../src/webcfg_rbus.c ../src/webcfg_method.c)
endif (WEBCONFIG_BIN_SUPPORT)

if (FEATURE_SUPPORT_AKER)
set(SOURCES ${SOURCES} ../src/webcfg_client.c ../src/webcfg_aker.c)
endif (FEATURE_SUPPORT_AKER)

add_executable(test_root ${SOURCES})

target_link_libraries (test_root -lcunit -lmsgpackc -lcurl -lpthread  -lm -luuid -ltrower-base64 -lwdmp-c -lcimplog -lcjson -lwrp-c)

if (WEBCONFIG_BIN_SUPPORT)
target_link_libraries (test_root -lrbus)
endif (WEBCONFIG_BIN_SUPPORT)

if (FEATURE_SUPPORT_AKER)
target_link_libraries (test_root -llibparodus -lnanomsg)
endif (FEATURE_SUPPORT_AKER)

target_link_libraries (test_root gcov -Wl,--no-as-needed )

#-------------------------------------------------------------------------------
#   test_db
#-------------------------------------------------------------------------------
add_test(NAME test_db COMMAND ${MEMORY_CHECK} ./test_db)

set(SOURCES test_db.c ../src/webcfg_param.c ../src/webcfg_multipart.c ../src/webcfg_helpers.c ../src/webcfg.c ../src/webcfg_auth.c ../src/webcfg_notify.c ../src/webcfg_db.c ../src/webcfg_pack.c ../src/webcfg_blob.c ../src/webcfg_metadata.c ../src/webcfg.c ../src/webcfg_generic_pc.c ../src/webcfg_timer.c)

if (WEBCONFIG_BIN_SUPPORT)
set(SOURCES ${SOURCES} ../src/webcfg_rbus.c ../src/webcfg_method.c)
endif (WEBCONFIG_BIN_SUPPORT)

add_executable(test_db ${SOURCES})
target_link_libraries (test_db -lcunit -lmsgpackc -lcurl -lpthread  -lm -luuid -ltrower-base64 -lwdmp-c -lcimplog -lcjson -lwrp-c)

if (WEBCONFIG_BIN_SUPPORT)
target_link_libraries (test_db -lrbus)
endif (WEBCONFIG_BIN_SUPPORT)

target_link_libraries (test_db gcov -Wl,--no-as-needed )
#-------------------------------------------------------------------------------
#   test_webcfgdb
#-------------------------------------------------------------------------------
add_test(NAME test_webcfgdb COMMAND ${MEMORY_CHECK} ./test_webcfgdb)
set(SOURCES test_webcfgdb.c ../src/webcfg_param.c ../src/webcfg_multipart.c ../src/webcfg_helpers.c ../src/webcfg.c ../src/webcfg_auth.c ../src/webcfg_notify.c ../src/webcfg_db.c ../src/webcfg_pack.c ../src/webcfg_blob.c ../src/webcfg_metadata.c ../src/webcfg.c ../src/webcfg_generic_pc.c ../src/webcfg_timer.c)

if (WEBCONFIG_BIN_SUPPORT)
set(SOURCES ${SOURCES} ../src/webcfg_rbus.c ../src/webcfg_method.c)
endif (WEBCONFIG_BIN_SUPPORT)

add_executable(test_webcfgdb ${SOURCES})
target_link_libraries (test_webcfgdb -lcunit -lmsgpackc -lcurl -lpthread  -lm -luuid -ltrower-base64 -lwdmp-c -lcimplog -lcjson -lwrp-c)

if (WEBCONFIG_BIN_SUPPORT)
target_link_libraries (test_webcfgdb -lrbus)
endif (WEBCONFIG_BIN_SUPPORT)

target_link_libraries (test_webcfgdb gcov -Wl,--no-as-needed )
#-------------------------------------------------------------------------------
#   test_multipart_unittest
#-------------------------------------------------------------------------------
add_test(NAME test_multipart_unittest COMMAND ${MEMORY_CHECK} ./test_multipart_unittest)

set(SOURCES test_multipart_unittest.c ../src/webcfg_param.c ../src/webcfg_multipart.c ../src/webcfg_helpers.c ../src/webcfg.c ../src/webcfg_auth.c ../src/webcfg_notify.c ../src/webcfg_db.c ../src/webcfg_pack.c ../src/webcfg_blob.c ../src/webcfg_event.c ../src/webcfg_generic_pc.c ../src/webcfg_metadata.c ../src/webcfg_timer.c)

if (WEBCONFIG_BIN_SUPPORT)
set(SOURCES ${SOURCES} ../src/webcfg_rbus.c ../src/webcfg_method.c)
endif (WEBCONFIG_BIN_SUPPORT)

if (FEATURE_SUPPORT_AKER)
set(SOURCES ${SOURCES} ../src/webcfg_client.c ../src/webcfg_aker.c)
endif (FEATURE_SUPPORT_AKER)

add_executable(test_multipart_unittest ${SOURCES})

target_link_libraries (test_multipart_unittest -lcunit -lmsgpackc -lcurl -lpthread  -lm -luuid -ltrower-base64 -lwdmp-c -lcimplog -lcjson -lwrp-c)

if (WEBCONFIG_BIN_SUPPORT)
target_link_libraries (test_multipart_unittest -lrbus)
endif (WEBCONFIG_BIN_SUPPORT)

if (FEATURE_SUPPORT_AKER)
target_link_libraries (test_multipart_unittest -llibparodus -lnanomsg)
endif (FEATURE_SUPPORT_AKER)

target_link_libraries (test_multipart_unittest gcov -Wl,--no-as-needed )

#-------------------------------------------------------------------------------
#   test_rbus_FR
#-------------------------------------------------------------------------------
if (WEBCONFIG_BIN_SUPPORT)
add_test(NAME test_rbus_fr COMMAND ${MEMORY_CHECK} ./test_rbus_fr)
add_executable(test_rbus_fr test_rbus_fr.c ../src/webcfg_timer.c ../src/webcfg_auth.c ../src/webcfg_event.c ../src/webcfg_blob.c ../src/webcfg_metadata.c ../src/webcfg_rbus.c ../src/webcfg_param.c ../src/webcfg_wanhandle.c ../src/webcfg_db.c ../src/webcfg_multipart.c ../src/webcfg_generic_pc.c ../src/webcfg_helpers.c ../src/webcfg_pack.c ../src/webcfg_notify.c ../src/webcfg_method.c)
target_link_libraries (test_rbus_fr -lcunit -lwrp-c -lcimplog -lmsgpackc -lcurl -lpthread  -lm -luuid -ltrower-base64 -lwdmp-c -lcjson  -lrbus)

target_link_libraries (test_rbus_fr gcov -Wl,--no-as-needed )
endif (WEBCONFIG_BIN_SUPPORT)

#-------------------------------------------------------------------------------
#   test_generic
#-------------------------------------------------------------------------------
add_test(NAME test_generic COMMAND ${MEMORY_CHECK} ./test_generic)
add_executable(test_generic test_generic.c ../src/webcfg_generic.c)
target_link_libraries (test_generic -lcunit -lmsgpackc -lcimplog -ltrower-base64 -lwdmp-c -lcjson)
 
if (WEBCONFIG_BIN_SUPPORT)
target_link_libraries (test_generic -lrbus)
endif (WEBCONFIG_BIN_SUPPORT)
 
target_link_libraries (test_generic gcov -Wl,--no-as-needed )

#-------------------------------------------------------------------------------
#   test_timer
#-------------------------------------------------------------------------------
add_test(NAME test_timer COMMAND ${MEMORY_CHECK} ./test_timer)
add_executable(test_timer test_timer.c ../src/webcfg_timer.c)
target_link_libraries (test_timer -lcunit -lmsgpackc -lcimplog -ltrower-base64 -lwdmp-c -lcjson)

if (WEBCONFIG_BIN_SUPPORT)
target_link_libraries (test_timer -lrbus)
endif (WEBCONFIG_BIN_SUPPORT)

target_link_libraries (test_timer gcov -Wl,--no-as-needed )

#-------------------------------------------------------------------------------
#   test_notify
#-------------------------------------------------------------------------------
add_test(NAME test_notify COMMAND ${MEMORY_CHECK} ./test_notify)
add_executable(test_notify test_notify.c ../src/webcfg_notify.c)
target_link_libraries (test_notify -lcunit -lmsgpackc -lcimplog -ltrower-base64 -lwdmp-c -lcjson -lpthread)
 
target_link_libraries (test_notify gcov -Wl,--no-as-needed )

#-------------------------------------------------------------------------------
#   test_webcfg
#-------------------------------------------------------------------------------
add_test(NAME test_webcfg COMMAND ${MEMORY_CHECK} ./test_webcfg)
set(SOURCES test_webcfg.c ../src/webcfg_param.c ../src/webcfg_multipart.c ../src/webcfg_helpers.c ../src/webcfg.c ../src/webcfg_auth.c ../src/webcfg_notify.c ../src/webcfg_db.c ../src/webcfg_pack.c ../src/webcfg_blob.c ../src/webcfg_metadata.c ../src/webcfg_generic_pc.c ../src/webcfg_timer.c ../src/webcfg_event.c)

if (WEBCONFIG_BIN_SUPPORT)
set(SOURCES ${SOURCES} ../src/webcfg_rbus.c ../src/webcfg_method.c)
endif (WEBCONFIG_BIN_SUPPORT)

add_executable(test_webcfg ${SOURCES})
target_link_libraries (test_webcfg -lcunit -lwrp-c -lwdmp-c -lcimplog -lmsgpackc -lcurl -lpthread  -lm -luuid -ltrower-base64 -lcjson -lcmocka)

if (WEBCONFIG_BIN_SUPPORT)
target_link_libraries (test_webcfg -lrbus)
endif (WEBCONFIG_BIN_SUPPORT)

target_link_libraries (test_webcfg gcov -Wl,--no-as-needed )

#-------------------------------------------------------------------------------
#   test_generic
#-------------------------------------------------------------------------------
add_test(NAME test_generic_pc COMMAND ${MEMORY_CHECK} ./test_generic_pc)
add_executable(test_generic_pc test_generic_pc.c ../src/webcfg_generic_pc.c)
target_link_libraries (test_generic_pc -lcunit -lmsgpackc -lcimplog -ltrower-base64 -lwdmp-c -lcjson)
 
if (WEBCONFIG_BIN_SUPPORT)
target_link_libraries (test_generic_pc -lrbus)
endif (WEBCONFIG_BIN_SUPPORT)
 
target_link_libraries (test_generic_pc gcov -Wl,--no-as-needed )

#-------------------------------------------------------------------------------
#   test_blob
#-------------------------------------------------------------------------------
add_test(NAME test_blob COMMAND ${MEMORY_CHECK} ./test_blob)
add_executable(test_blob test_blob.c ../src/webcfg_blob.c)
target_link_libraries (test_blob -lcunit -lmsgpackc -lcimplog -lcjson -ltrower-base64 )

target_link_libraries (test_blob gcov -Wl,--no-as-needed )

#-------------------------------------------------------------------------------
#   test_cmoka_multipart.c
#-------------------------------------------------------------------------------
add_test(NAME test_cmocka_multipart COMMAND ${MEMORY_CHECK} ./test_cmocka_multipart)

set(SOURCES test_cmocka_multipart.c ../src/webcfg_param.c ../src/webcfg_multipart.c ../src/webcfg_helpers.c ../src/webcfg.c ../src/webcfg_auth.c ../src/webcfg_notify.c ../src/webcfg_db.c ../src/webcfg_pack.c ../src/webcfg_blob.c ../src/webcfg_event.c ../src/webcfg_generic_pc.c ../src/webcfg_metadata.c ../src/webcfg_timer.c)

if (WEBCONFIG_BIN_SUPPORT)
set(SOURCES ${SOURCES} ../src/webcfg_rbus.c ../src/webcfg_method.c)
endif (WEBCONFIG_BIN_SUPPORT)

if (FEATURE_SUPPORT_AKER)
set(SOURCES ${SOURCES} ../src/webcfg_client.c ../src/webcfg_aker.c)
endif (FEATURE_SUPPORT_AKER)

add_executable(test_cmocka_multipart ${SOURCES})

target_link_libraries (test_cmocka_multipart -lcmocka -lcunit -lmsgpackc -lpthread  -lm -luuid -ltrower-base64 -lwdmp-c -lcimplog -lcjson -lwrp-c -lcurl )

if (WEBCONFIG_BIN_SUPPORT)
target_link_libraries (test_cmocka_multipart -lrbus)
endif (WEBCONFIG_BIN_SUPPORT)

if (FEATURE_SUPPORT_AKER)
target_link_libraries (test_cmoka_multipart -llibparodus -lnanomsg)
endif (FEATURE_SUPPORT_AKER)

target_link_libraries (test_cmocka_multipart gcov -Wl,--no-as-needed )

#-------------------------------------------------------------------------------
#   test_auth
#-------------------------------------------------------------------------------
add_test(NAME test_auth COMMAND ${MEMORY_CHECK} ./test_auth)
add_executable(test_auth test_auth.c ../src/webcfg_auth.c)
target_link_libraries (test_auth -lcunit -lmsgpackc -lcimplog -lcjson -ltrower-base64)
target_link_libraries (test_auth gcov -Wl,--no-as-needed )

# Code coverage

add_custom_target(coverage
COMMAND lcov -q --capture --directory 
${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/test_events_supp.dir/__/src --output-file test_events_supp.info
COMMAND lcov -q --capture --directory 
${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/test_multipart.dir/__/src --output-file test_multipart.info
COMMAND lcov -q --capture --directory 
${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/test_metadata.dir/__/src --output-file test_metadata.info
COMMAND lcov -q --capture --directory 
${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/test_mul_supp.dir/__/src --output-file test_mul_supp.info
COMMAND lcov -q --capture --directory 
${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/test_db.dir/__/src --output-file test_db.info
COMMAND lcov -q --capture --directory 
${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/test_webcfgparam.dir/__/src --output-file test_webcfgparam.info
COMMAND lcov -q --capture --directory 
${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/test_events.dir/__/src --output-file test_events.info
COMMAND lcov -q --capture --directory 
${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/test_root.dir/__/src --output-file test_root.info
COMMAND lcov -q --capture --directory 
${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/test_multipart_unittest.dir/__/src --output-file test_multipart_unittest.info
COMMAND lcov -q --capture --directory 
${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/test_webcfgpack.dir/__/src --output-file test_webcfgpack.info
COMMAND lcov -q --capture --directory 
${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/test_generic.dir/__/src --output-file test_generic.info
COMMAND lcov -q --capture --directory 
${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/test_rbus_fr.dir/__/src --output-file test_rbus_fr.info
COMMAND lcov -q --capture --directory
${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/test_timer.dir/__/src --output-file test_timer.info
COMMAND lcov -q --capture --directory 
${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/test_notify.dir/__/src --output-file test_notify.info
COMMAND lcov -q --capture --directory
${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/test_webcfg.dir/__/src --output-file test_webcfg.info
COMMAND lcov -q --capture --directory 
${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/test_generic_pc.dir/__/src --output-file test_generic_pc.info
COMMAND lcov -q --capture --directory 
${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/test_blob.dir/__/src --output-file test_blob.info
COMMAND lcov -q --capture --directory
${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/test_cmocka_multipart.dir/__/src --output-file test_cmocka_multipart.info
COMMAND lcov -q --capture --directory
${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/test_auth.dir/__/src --output-file test_auth.info


COMMAND lcov
-a test_events_supp.info
-a test_multipart.info
-a test_metadata.info
-a test_mul_supp.info
-a test_db.info
-a test_webcfgparam.info
-a test_events.info
-a test_root.info
-a test_multipart_unittest.info
-a test_webcfgpack.info
-a test_generic.info
-a test_rbus_fr.info
-a test_timer.info
-a test_notify.info
-a test_webcfg.info
-a test_generic_pc.info
-a test_blob.info
-a test_cmocka_multipart.info
-a test_auth.info
--output-file coverage.info

COMMAND genhtml coverage.info
WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
