# Copyright 2023 CARIAD SE.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.


add_library(participant_private_test_utils INTERFACE)

target_include_directories(participant_private_test_utils INTERFACE
    .
    $<TARGET_PROPERTY:fep3_participant,INTERFACE_INCLUDE_DIRECTORIES>
)

target_link_libraries(participant_private_test_utils INTERFACE GTest::gmock)
add_subdirectory(convenient_component_registry)