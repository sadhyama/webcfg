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

if (WEBCONFIG_BIN_SUPPORT)
add_definitions(-DWEBCONFIG_BIN_SUPPORT)
endif (WEBCONFIG_BIN_SUPPORT)

if (WEBCONFIG_BIN_SUPPORT)
message(STATUS "WEBCONFIG_BIN_SUPPORT is supported")
else()
message(STATUS "WEBCONFIG_BIN_SUPPORT is not supported")
endif (WEBCONFIG_BIN_SUPPORT)

set(PROJ_WEBCFG webcfg)
set(HEADERS webcfg.h webcfg_param.h webcfg_pack.h webcfg_multipart.h webcfg_auth.h webcfg_notify.h webcfg_generic.h webcfg_db.h webcfg_log.h webcfg_blob.h webcfg_event.h webcfg_aker.h webcfg_metadata.h webcfg_timer.h webcfg_rbus.h)
set(SOURCES main.c webcfg_helpers.c webcfg.c webcfg_param.c webcfg_pack.c webcfg_multipart.c webcfg_auth.c webcfg_notify.c webcfg_db.c webcfg_generic.c webcfg_blob.c webcfg_event.c webcfg_client.c webcfg_aker.c webcfg_metadata.c webcfg_timer.c webcfg_rbus.c)

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
add_executable(webconfig ${SOURCES} ${HEADERS})
set_property(TARGET webconfig PROPERTY C_STANDARD 99)
set(COMMON_LIBS -lwrp-c -lpthread -lwdmp-c -lmsgpackc -ltrower-base64 -lm -lcjson -lrt -luuid -llibparodus -lnanomsg -lcimplog -lcurl -lrbus -lrbus-core)
target_link_libraries (webconfig ${CMAKE_THREAD_LIBS_INIT} ${COMMON_LIBS})
install (TARGETS webconfig DESTINATION bin)
endif()
