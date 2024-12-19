# Copyright 2023 CARIAD SE.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

if(fep3_component_registry_FOUND)
    return()
endif()

if(NOT TARGET dev_essential)
    find_package(dev_essential 1.3.0 REQUIRED COMPONENTS result process xml)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/lib/cmake/fep3_component_registry_targets.cmake)
set(fep3_component_registry_FOUND true)
