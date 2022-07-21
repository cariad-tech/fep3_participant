#
# Copyright @ 2021 VW Group. All rights reserved.
# 
#     This Source Code Form is subject to the terms of the Mozilla
#     Public License, v. 2.0. If a copy of the MPL was not distributed
#     with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
# 
# If it is not possible or desirable to put the notice in a particular file, then
# You may include the notice in a location (such as a LICENSE file in a
# relevant directory) where a recipient would be likely to look for such a notice.
# 
# You may add additional accurate notices of copyright ownership.
# 
#

if(fep3_participant_cpp_FOUND)
    return()
endif()

# find participant core
find_package(fep3_participant_core REQUIRED)

# Add imported library target for the participant cpp interface
include(${CMAKE_CURRENT_LIST_DIR}/lib/cmake/fep3_participant_cpp_targets.cmake)

set(fep3_participant_cpp_FOUND true)
