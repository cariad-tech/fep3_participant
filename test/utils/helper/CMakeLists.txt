#
# Copyright 2023 CARIAD SE.
# 
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
# 
#

find_package(dev_essential 1.3.0 REQUIRED COMPONENTS pkg_rpc)
add_library(fep_test_helper INTERFACE)
target_include_directories(fep_test_helper INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(fep_test_helper INTERFACE dev_essential::pkg_rpc)
