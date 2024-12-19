# Copyright 2023 CARIAD SE.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

add_library(fep3_default_severity_from_env_variable INTERFACE
${CMAKE_CURRENT_SOURCE_DIR}/default_severity_from_env_variable.h)

target_include_directories(fep3_default_severity_from_env_variable
                           INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})


target_link_libraries(fep3_default_severity_from_env_variable INTERFACE fep3_environment_variable)

set_target_properties(fep3_default_severity_from_env_variable PROPERTIES FOLDER "libraries/base")
