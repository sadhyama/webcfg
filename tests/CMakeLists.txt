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
set (MEMORY_CHECK valgrind --leak-check=full --show-reachable=yes -v)
endif ()

link_directories ( ${LIBRARY_DIR} )

#-------------------------------------------------------------------------------
#   test_webcfgparam
#-------------------------------------------------------------------------------
add_test(NAME test_webcfgparam COMMAND ${MEMORY_CHECK} ./test_webcfgparam)
add_executable(test_webcfgparam test_webcfgparam.c ../src/webcfgparam.c ../src/helpers.c)
target_link_libraries (test_webcfgparam -lcunit -lmsgpackc)

target_link_libraries (test_webcfgparam gcov -Wl,--no-as-needed )

#-------------------------------------------------------------------------------
#   test_webcfgdoc
#-------------------------------------------------------------------------------
add_test(NAME test_webcfgdoc COMMAND ${MEMORY_CHECK} ./test_webcfgdoc)
add_executable(test_webcfgdoc test_webcfgdoc.c ../src/webcfgdoc.c ../src/webcfgparam.c ../src/helpers.c ../src/array_helpers.c)
target_link_libraries (test_webcfgdoc -lcunit -lmsgpackc)

target_link_libraries (test_webcfgdoc gcov -Wl,--no-as-needed )

#-------------------------------------------------------------------------------
#   test_webcfgpack
#-------------------------------------------------------------------------------
add_test(NAME test_webcfgpack COMMAND ${MEMORY_CHECK} ./test_webcfgpack)
add_executable(test_webcfgpack test_webcfgpack.c ../src/webcfgdoc.c ../src/webcfgparam.c ../src/webcfgpack.c ../src/helpers.c ../src/array_helpers.c )
target_link_libraries (test_webcfgpack -lcunit -lmsgpackc)

target_link_libraries (test_webcfgpack gcov -Wl,--no-as-needed )

#-------------------------------------------------------------------------------
#   test_multipart
#-------------------------------------------------------------------------------
add_test(NAME test_multipart COMMAND ${MEMORY_CHECK} ./test_multipart)
add_executable(test_multipart test_multipart.c ../src/webcfgdoc.c ../src/webcfgparam.c ../src/multipart.c ../src/helpers.c ../src/array_helpers.c )
target_link_libraries (test_multipart -lcunit -lmsgpackc -lcurl -luuid)

target_link_libraries (test_multipart gcov -Wl,--no-as-needed )

# Code coverage

add_custom_target(coverage
COMMAND lcov -q --capture --directory 
${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/test_webcfgpack.dir/__/src --output-file test_webcfgpack.info
COMMAND lcov -q --capture --directory 
${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/test_webcfgdoc.dir/__/src --output-file test_webcfgdoc.info
COMMAND lcov -q --capture --directory 
${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/test_webcfgparam.dir/__/src --output-file test_webcfgparam.info
COMMAND lcov -q --capture --directory

COMMAND lcov
-a test_webcfgdoc.info
-a test_webcfgparam.info
-a test_webcfgpack.info
--output-file coverage.info

COMMAND genhtml coverage.info
WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
