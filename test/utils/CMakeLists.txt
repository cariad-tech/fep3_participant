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

find_package(Boost 1.73.0 REQUIRED COMPONENTS)
find_package(dev_essential REQUIRED COMPONENTS process)

add_library(participant_test_utils STATIC helper/platform_dep_name.cpp)
set_target_properties(participant_test_utils PROPERTIES FOLDER "test/function/scenarios")

target_include_directories(participant_test_utils PUBLIC
    .
    $<TARGET_PROPERTY:fep3_participant,INTERFACE_INCLUDE_DIRECTORIES
    ../private/utils/common
    PRIVATE
    ${Boost_INCLUDE_DIR}
)

target_link_libraries(participant_test_utils PUBLIC GTest::gmock PRIVATE dev_essential::process )
add_subdirectory(helper)
