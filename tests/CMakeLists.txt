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

if(NOT DISABLE_VALGRIND)
set (MEMORY_CHECK valgrind --leak-check=full --show-reachable=yes --track-origins=yes -v)
endif ()

link_directories ( ${LIBRARY_DIR} )
#-------------------------------------------------------------------------------
#   webcfgCli
#-------------------------------------------------------------------------------
set(SOURCES webcfgCli.c ../src/webcfg_helpers.c ../src/webcfg.c ../src/webcfg_param.c ../src/webcfg_pack.c ../src/webcfg_multipart.c ../src/webcfg_auth.c ../src/webcfg_notify.c ../src/webcfg_db.c ../src/webcfg_generic.c ../src/webcfg_blob.c ../src/webcfg_event.c ../src/webcfg_metadata.c ../src/webcfg_timer.c)

if (WEBCONFIG_BIN_SUPPORT)
add_definitions(-DWEBCONFIG_BIN_SUPPORT)
endif (WEBCONFIG_BIN_SUPPORT)

if (WEBCONFIG_BIN_SUPPORT)
set(SOURCES ${SOURCES} ../src/webcfg_generic_pc.c ../src/webcfg_rbus.c)
endif (WEBCONFIG_BIN_SUPPORT)

if (FEATURE_SUPPORT_AKER)
set(SOURCES ${SOURCES} ../src/webcfg_client.c ../src/webcfg_aker.c)
endif (FEATURE_SUPPORT_AKER)

add_executable(webcfgCli ${SOURCES})
target_link_libraries (webcfgCli -lmsgpackc -lcurl -lpthread  -lm -luuid -ltrower-base64 -lwdmp-c -lcimplog -lcjson -lwrp-c)

if (WEBCONFIG_BIN_SUPPORT)
target_link_libraries (webcfgCli -lrbus -lrbus-core)
endif (WEBCONFIG_BIN_SUPPORT)

if (FEATURE_SUPPORT_AKER)
target_link_libraries (webcfgCli -llibparodus -lnanomsg)
endif (FEATURE_SUPPORT_AKER)

#-------------------------------------------------------------------------------
#   test_webcfgparam
#-------------------------------------------------------------------------------
add_test(NAME test_webcfgparam COMMAND ${MEMORY_CHECK} ./test_webcfgparam)
add_executable(test_webcfgparam test_webcfgparam.c ../src/webcfg_param.c ../src/webcfg_helpers.c)
target_link_libraries (test_webcfgparam -lcunit -lmsgpackc -lcimplog)

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

set(SOURCES test_multipart.c ../src/webcfg_param.c ../src/webcfg_multipart.c ../src/webcfg_helpers.c ../src/webcfg.c ../src/webcfg_auth.c ../src/webcfg_notify.c ../src/webcfg_db.c ../src/webcfg_pack.c ../src/webcfg_blob.c ../src/webcfg_event.c ../src/webcfg_generic.c ../src/webcfg_metadata.c ../src/webcfg_timer.c)

if (WEBCONFIG_BIN_SUPPORT)
set(SOURCES ${SOURCES} ../src/webcfg_generic_pc.c ../src/webcfg_rbus.c)
endif (WEBCONFIG_BIN_SUPPORT)

if (FEATURE_SUPPORT_AKER)
set(SOURCES ${SOURCES} ../src/webcfg_client.c ../src/webcfg_aker.c)
endif (FEATURE_SUPPORT_AKER)

add_executable(test_multipart ${SOURCES})
target_link_libraries (test_multipart -lcunit -lmsgpackc -lcurl -lpthread  -lm -luuid -ltrower-base64 -lwdmp-c -lcimplog -lcjson -lwrp-c)

if (WEBCONFIG_BIN_SUPPORT)
target_link_libraries (test_multipart -lrbus -lrbus-core)
endif (WEBCONFIG_BIN_SUPPORT)

if (FEATURE_SUPPORT_AKER)
target_link_libraries (test_multipart -llibparodus -lnanomsg)
endif (FEATURE_SUPPORT_AKER)

target_link_libraries (test_multipart gcov -Wl,--no-as-needed )
#-------------------------------------------------------------------------------
#   test_multipart_supplementary
#-------------------------------------------------------------------------------
add_test(NAME test_mul_supp COMMAND ${MEMORY_CHECK} ./test_mul_supp)
set(SOURCES test_mul_supp.c ../src/webcfg_param.c ../src/webcfg_multipart.c ../src/webcfg_helpers.c ../src/webcfg.c ../src/webcfg_auth.c ../src/webcfg_notify.c ../src/webcfg_db.c ../src/webcfg_pack.c ../src/webcfg_blob.c ../src/webcfg_event.c ../src/webcfg_generic.c ../src/webcfg_metadata.c ../src/webcfg_timer.c)

if (WEBCONFIG_BIN_SUPPORT)
set(SOURCES ${SOURCES} ../src/webcfg_generic_pc.c ../src/webcfg_rbus.c)
endif (WEBCONFIG_BIN_SUPPORT)

if (FEATURE_SUPPORT_AKER)
set(SOURCES ${SOURCES} ../src/webcfg_client.c ../src/webcfg_aker.c)
endif (FEATURE_SUPPORT_AKER)

add_executable(test_mul_supp ${SOURCES})
target_link_libraries (test_mul_supp -lcunit -lmsgpackc -lcurl -lpthread  -lm -luuid -ltrower-base64 -lwdmp-c -lcimplog -lcjson -lwrp-c)

if (WEBCONFIG_BIN_SUPPORT)
target_link_libraries (test_mul_supp -lrbus -lrbus-core)
endif (WEBCONFIG_BIN_SUPPORT)

if (FEATURE_SUPPORT_AKER)
target_link_libraries (test_mul_supp -llibparodus -lnanomsg)
endif (FEATURE_SUPPORT_AKER)

target_link_libraries (test_mul_supp gcov -Wl,--no-as-needed )
#-------------------------------------------------------------------------------
#   test_events
#-------------------------------------------------------------------------------
add_test(NAME test_events COMMAND ${MEMORY_CHECK} ./test_events)
set(SOURCES test_events.c ../src/webcfg_param.c ../src/webcfg_multipart.c ../src/webcfg_helpers.c ../src/webcfg.c ../src/webcfg_auth.c ../src/webcfg_notify.c ../src/webcfg_db.c ../src/webcfg_pack.c ../src/webcfg_blob.c ../src/webcfg_event.c ../src/webcfg_generic.c ../src/webcfg_metadata.c ../src/webcfg_timer.c )

if (WEBCONFIG_BIN_SUPPORT)
set(SOURCES ${SOURCES} ../src/webcfg_generic_pc.c ../src/webcfg_rbus.c)
endif (WEBCONFIG_BIN_SUPPORT)

if (FEATURE_SUPPORT_AKER)
set(SOURCES ${SOURCES} ../src/webcfg_client.c ../src/webcfg_aker.c)
endif (FEATURE_SUPPORT_AKER)

add_executable(test_events ${SOURCES})

target_link_libraries (test_events -lcunit -lmsgpackc -lcurl -lpthread  -lm -luuid -ltrower-base64 -lwdmp-c -lcimplog -lcjson -lwrp-c)

if (WEBCONFIG_BIN_SUPPORT)
target_link_libraries (test_events -lrbus -lrbus-core)
endif (WEBCONFIG_BIN_SUPPORT)

if (FEATURE_SUPPORT_AKER)
target_link_libraries (test_events -llibparodus -lnanomsg)
endif (FEATURE_SUPPORT_AKER)

target_link_libraries (test_events gcov -Wl,--no-as-needed )

#-------------------------------------------------------------------------------
#   test_events_supplematary
#-------------------------------------------------------------------------------
add_test(NAME test_events_supp COMMAND ${MEMORY_CHECK} ./test_events_supp)

set(SOURCES test_events_supp.c ../src/webcfg_param.c ../src/webcfg_multipart.c ../src/webcfg_helpers.c ../src/webcfg.c ../src/webcfg_auth.c ../src/webcfg_notify.c ../src/webcfg_db.c ../src/webcfg_pack.c ../src/webcfg_blob.c ../src/webcfg_event.c ../src/webcfg_generic.c ../src/webcfg_metadata.c ../src/webcfg_timer.c)

if (WEBCONFIG_BIN_SUPPORT)
set(SOURCES ${SOURCES} ../src/webcfg_generic_pc.c ../src/webcfg_rbus.c)
endif (WEBCONFIG_BIN_SUPPORT)

if (FEATURE_SUPPORT_AKER)
set(SOURCES ${SOURCES} ../src/webcfg_client.c ../src/webcfg_aker.c)
endif (FEATURE_SUPPORT_AKER)

add_executable(test_events_supp  ${SOURCES})
target_link_libraries (test_events_supp -lcunit -lmsgpackc -lcurl -lpthread  -lm -luuid -ltrower-base64 -lwdmp-c -lcimplog -lcjson -lwrp-c)

if (WEBCONFIG_BIN_SUPPORT)
target_link_libraries (test_events_supp -lrbus -lrbus-core)
endif (WEBCONFIG_BIN_SUPPORT)

if (FEATURE_SUPPORT_AKER)
target_link_libraries (test_events_supp -llibparodus -lnanomsg)
endif (FEATURE_SUPPORT_AKER)

target_link_libraries (test_events_supp gcov -Wl,--no-as-needed )
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

set(SOURCES test_root.c ../src/webcfg_param.c ../src/webcfg_multipart.c ../src/webcfg_helpers.c ../src/webcfg.c ../src/webcfg_auth.c ../src/webcfg_notify.c ../src/webcfg_db.c ../src/webcfg_pack.c ../src/webcfg_blob.c ../src/webcfg_event.c ../src/webcfg_generic.c ../src/webcfg_metadata.c ../src/webcfg_timer.c)

if (WEBCONFIG_BIN_SUPPORT)
set(SOURCES ${SOURCES} ../src/webcfg_generic_pc.c ../src/webcfg_rbus.c)
endif (WEBCONFIG_BIN_SUPPORT)

if (FEATURE_SUPPORT_AKER)
set(SOURCES ${SOURCES} ../src/webcfg_client.c ../src/webcfg_aker.c)
endif (FEATURE_SUPPORT_AKER)

add_executable(test_root ${SOURCES})

target_link_libraries (test_root -lcunit -lmsgpackc -lcurl -lpthread  -lm -luuid -ltrower-base64 -lwdmp-c -lcimplog -lcjson -lwrp-c)

if (WEBCONFIG_BIN_SUPPORT)
target_link_libraries (test_root -lrbus -lrbus-core)
endif (WEBCONFIG_BIN_SUPPORT)

if (FEATURE_SUPPORT_AKER)
target_link_libraries (test_root -llibparodus -lnanomsg)
endif (FEATURE_SUPPORT_AKER)

target_link_libraries (test_root gcov -Wl,--no-as-needed )

#-------------------------------------------------------------------------------
#   test_webcfgdb
#-------------------------------------------------------------------------------
add_test(NAME test_db COMMAND ${MEMORY_CHECK} ./test_db)
add_executable(test_db test_db.c ../src/webcfg_db.c ../src/webcfg_pack.c ../src/webcfg_helpers.c ../src/webcfg_param.c ../src/webcfg_multipart.c ../src/webcfg_timer.c ../src/webcfg_notify.c )
target_link_libraries (test_db -lcunit -lmsgpackc -lcurl -lpthread  -lm -luuid -ltrower-base64 -lwdmp-c -lcimplog -lcjson -lwrp-c)

target_link_libraries (test_db gcov -Wl,--no-as-needed )

#-------------------------------------------------------------------------------
#   test_multipart_unittest
#-------------------------------------------------------------------------------
add_test(NAME test_multipart_unittest COMMAND ${MEMORY_CHECK} ./test_multipart_unittest)

set(SOURCES test_multipart_unittest.c ../src/webcfg_param.c ../src/webcfg_multipart.c ../src/webcfg_helpers.c ../src/webcfg.c ../src/webcfg_auth.c ../src/webcfg_notify.c ../src/webcfg_db.c ../src/webcfg_pack.c ../src/webcfg_blob.c ../src/webcfg_event.c ../src/webcfg_generic.c ../src/webcfg_metadata.c)

if (WEBCONFIG_BIN_SUPPORT)
set(SOURCES ${SOURCES} ../src/webcfg_generic_pc.c ../src/webcfg_rbus.c)
endif (WEBCONFIG_BIN_SUPPORT)

if (FEATURE_SUPPORT_AKER)
set(SOURCES ${SOURCES} ../src/webcfg_client.c ../src/webcfg_aker.c)
endif (FEATURE_SUPPORT_AKER)

add_executable(test_multipart_unittest ${SOURCES})

target_link_libraries (test_multipart_unittest -lcunit -lmsgpackc -lcurl -lpthread  -lm -luuid -ltrower-base64 -lwdmp-c -lcimplog -lcjson -lwrp-c)

if (WEBCONFIG_BIN_SUPPORT)
target_link_libraries (test_multipart_unittest -lrbus -lrbus-core)
endif (WEBCONFIG_BIN_SUPPORT)

if (FEATURE_SUPPORT_AKER)
target_link_libraries (test_multipart_unittest -llibparodus -lnanomsg)
endif (FEATURE_SUPPORT_AKER)

target_link_libraries (test_multipart_unittest gcov -Wl,--no-as-needed )



# Code coverage

add_custom_target(coverage
COMMAND lcov -q --capture --directory 
${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/test_webcfgpack.dir/__/src --output-file test_webcfgpack.info
COMMAND lcov -q --capture --directory 
${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/test_webcfgparam.dir/__/src --output-file test_webcfgparam.info

COMMAND lcov
-a test_webcfgparam.info
-a test_webcfgpack.info
--output-file coverage.info

COMMAND genhtml coverage.info
WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
