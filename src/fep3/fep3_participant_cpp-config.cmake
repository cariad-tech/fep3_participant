# Copyright 2023 CARIAD SE.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

if(fep3_participant_cpp_FOUND)
    return()
endif()

# find participant core
find_package(fep3_participant_core REQUIRED)

# Add imported library target for the participant cpp interface
include(${CMAKE_CURRENT_LIST_DIR}/lib/cmake/fep3_participant_cpp_targets.cmake)

set(fep3_participant_cpp_FOUND true)
