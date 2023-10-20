#
# Copyright @ 2022 VW Group. All rights reserved.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

find_package(Boost 1.73.0 REQUIRED COMPONENTS date_time thread headers)

add_library(fep3_thread_utilities STATIC
${CMAKE_CURRENT_SOURCE_DIR}/include/notification_waiting.h
${CMAKE_CURRENT_SOURCE_DIR}/src/notification_waiting.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/threaded_executor.cpp
${CMAKE_CURRENT_SOURCE_DIR}/include/threaded_executor.h)

target_include_directories(fep3_thread_utilities
                           PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/
                           # for fep timestamp
                           ${PROJECT_SOURCE_DIR}/include/)

set_target_properties(fep3_thread_utilities PROPERTIES FOLDER "libraries/base")

target_link_libraries(fep3_thread_utilities
    PUBLIC
    Boost::date_time  # check if needed
    Boost::headers  # check if needed
    Boost::thread)

target_compile_definitions(fep3_thread_utilities PUBLIC BOOST_THREAD_VERSION=4 BOOST_THREAD_PROVIDES_EXECUTORS)
