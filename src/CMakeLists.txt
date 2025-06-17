#   Copyright 2016 Comcast Cable Communications Management, LLC
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

set(PROJ_WEBCFG webcfg)

if (WEBCONFIG_BIN_SUPPORT)
add_definitions(-DWEBCONFIG_BIN_SUPPORT)

if (FEATURE_SUPPORT_MQTTCM)
add_definitions(-DFEATURE_SUPPORT_MQTTCM)
endif (FEATURE_SUPPORT_MQTTCM)

endif (WEBCONFIG_BIN_SUPPORT)

if (WAN_FAILOVER_SUPPORTED)
add_definitions(-DWAN_FAILOVER_SUPPORTED)
endif (WAN_FAILOVER_SUPPORTED)

if (FEATURE_SUPPORT_AKER)
add_definitions(-DFEATURE_SUPPORT_AKER)
endif (FEATURE_SUPPORT_AKER)

if (WEBCONFIG_BIN_SUPPORT)
message(STATUS "WEBCONFIG_BIN_SUPPORT is supported")
else()
message(STATUS "WEBCONFIG_BIN_SUPPORT is not supported")
endif (WEBCONFIG_BIN_SUPPORT)

if (WAN_FAILOVER_SUPPORTED)
message(STATUS "WAN_FAILOVER_SUPPORTED is supported")
else()
message(STATUS "WAN_FAILOVER_SUPPORTED is not supported")
endif (WAN_FAILOVER_SUPPORTED)

set(HEADERS webcfg.h webcfg_param.h webcfg_pack.h webcfg_multipart.h webcfg_auth.h webcfg_notify.h webcfg_generic.h webcfg_db.h webcfg_log.h webcfg_blob.h webcfg_event.h webcfg_metadata.h webcfg_timer.h webcfg_privilege.h webcfg_wanhandle.h)
set(SOURCES webcfg_helpers.c webcfg.c webcfg_param.c webcfg_pack.c webcfg_multipart.c webcfg_auth.c webcfg_notify.c webcfg_db.c webcfg_blob.c webcfg_event.c webcfg_metadata.c webcfg_timer.c webcfg_privilege.c webcfg_wanhandle.c)

if (FEATURE_SUPPORT_AKER)
set(HEADERS ${HEADERS} webcfg_aker.h)
set(SOURCES ${SOURCES} webcfg_aker.c webcfg_client.c)
endif (FEATURE_SUPPORT_AKER)

if (NOT BUILD_YOCTO)
set(SOURCES ${SOURCES} webcfg_generic_pc.c)
else()
set(SOURCES ${SOURCES} webcfg_generic.c)
endif()

#Support webcfg lib for backward compatibility
if (NOT WEBCONFIG_BIN_SUPPORT)
add_library(${PROJ_WEBCFG} STATIC ${HEADERS} ${SOURCES})
add_library(${PROJ_WEBCFG}.shared SHARED ${HEADERS} ${SOURCES})
set_target_properties(${PROJ_WEBCFG}.shared PROPERTIES OUTPUT_NAME ${PROJ_WEBCFG})
set_property(TARGET ${PROJ_WEBCFG} PROPERTY C_STANDARD 99)
set_property(TARGET ${PROJ_WEBCFG}.shared PROPERTY C_STANDARD 99)

install (TARGETS ${PROJ_WEBCFG} DESTINATION lib${LIB_SUFFIX})
install (TARGETS ${PROJ_WEBCFG}.shared DESTINATION lib${LIB_SUFFIX})
install (FILES ${HEADERS} DESTINATION include/${PROJ_WEBCFG})
endif()

#Support webconfig independent bin for new design
if (WEBCONFIG_BIN_SUPPORT)
set(HEADERS ${HEADERS} webcfg_rbus.h webcfg_method.h)
set(SOURCES ${SOURCES} main.c webcfg_rbus.c webcfg_method.c)

if (FEATURE_SUPPORT_MQTTCM)
set(HEADERS ${HEADERS} webcfg_mqtt.h)
set(SOURCES ${SOURCES} webcfg_mqtt.c)
endif (FEATURE_SUPPORT_MQTTCM)

add_executable(webconfig ${SOURCES} ${HEADERS})
set_property(TARGET webconfig PROPERTY C_STANDARD 99)
set(COMMON_LIBS -lwrp-c -lpthread -lwdmp-c -lmsgpackc -ltrower-base64 -lm -lcjson -lrt -luuid -lrbus)

if (NOT FEATURE_SUPPORT_MQTTCM)
set(COMMON_LIBS ${COMMON_LIBS} -lcurl)
endif()

if (NOT DEVICE_EXTENDER)
set(COMMON_LIBS ${COMMON_LIBS} -lcimplog)
else ()
add_definitions(-DDEVICE_EXTENDER)
endif()

if (FEATURE_SUPPORT_AKER)
set(COMMON_LIBS ${COMMON_LIBS} -llibparodus)
endif(FEATURE_SUPPORT_AKER)

target_link_libraries (webconfig ${CMAKE_THREAD_LIBS_INIT} ${COMMON_LIBS})
install (TARGETS webconfig DESTINATION bin)
endif()
