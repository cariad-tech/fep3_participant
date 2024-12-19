# Copyright 2023 CARIAD SE.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
if(NOT TARGET dev_essential)
    find_package(dev_essential 1.3.0 REQUIRED COMPONENTS result)
endif()

add_library(fep3_environment_variable STATIC
${CMAKE_CURRENT_SOURCE_DIR}/include/environment_variable.h
${CMAKE_CURRENT_SOURCE_DIR}/environment_variable.cpp)

target_include_directories(fep3_environment_variable
                           PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/
                           # for fep errors
                           PRIVATE
                           ${PROJECT_SOURCE_DIR}/include/)

set_target_properties(fep3_environment_variable PROPERTIES FOLDER "libraries/base")
target_link_libraries(fep3_environment_variable PUBLIC dev_essential::result)

set_source_files_properties(
        ${CMAKE_CURRENT_SOURCE_DIR}/environment_variable.cpp
        PROPERTIES COMPILE_DEFINITIONS $<$<CXX_COMPILER_ID:MSVC>:NOMINMAX>)
