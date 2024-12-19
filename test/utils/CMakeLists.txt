# Copyright 2023 CARIAD SE.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

find_package(Boost 1.73.0 REQUIRED)
find_package(dev_essential 1.3.0 REQUIRED COMPONENTS process)

add_library(participant_test_utils STATIC)

target_sources(participant_test_utils PRIVATE
    helper/component_registry_rpc_service_helper.h
    helper/dds_test_service_discovery_helpers.h
    helper/gmock_async_helper.h
    helper/gmock_destruction_helper.h
    helper/platform_dep_name.cpp
    helper/platform_dep_name.h
    helper/service_bust_multicast_receiver.h
    helper/notification_latch.h
)

set_target_properties(participant_test_utils PROPERTIES FOLDER "test/function/scenarios")

target_include_directories(participant_test_utils
                           PUBLIC .
                                  $<TARGET_PROPERTY:fep3_participant,INTERFACE_INCLUDE_DIRECTORIES>
                                  ../private/utils/common)

target_link_libraries(participant_test_utils
    PUBLIC
        GTest::gmock
        fep3_components_test
    PRIVATE
        dev_essential::process
        ${CMAKE_DL_LIBS}
        Boost::headers
)

add_subdirectory(helper)
