# Copyright 2023 CARIAD SE.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.


find_package(dev_essential 1.3.0 REQUIRED COMPONENTS pkg_rpc ddl)
find_package(Boost REQUIRED COMPONENTS filesystem)

##################################################################
# Data Registry
##################################################################

add_executable(test_data_registry tester_data_registry.cpp
               data_registry_test_fixture.h
)

target_link_libraries(test_data_registry PRIVATE
    dev_essential::ddl
    dev_essential::pkg_rpc
    GTest::gtest_main
    GTest::gmock
    fep3_participant_private_lib
    participant_test_utils
    fep3_components_test
)

set_target_properties(test_data_registry PROPERTIES FOLDER "test/private/native_components/data_registry")
internal_fep3_participant_deploy(test_data_registry)

gtest_discover_tests(test_data_registry
    TIMEOUT 10
)

##################################################################
# Mapping
##################################################################

add_executable(test_mapping tester_mapping.cpp
               data_registry_test_fixture.h
)

target_link_libraries(test_mapping PRIVATE
    dev_essential::pkg_rpc
    GTest::gtest_main
    GTest::gmock
    dev_essential::ddl
    fep3_participant_private_lib
    fep3_components_test
    Boost::filesystem
)

target_compile_definitions(test_mapping PRIVATE
    TEST_FILE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../files/"
)

set_target_properties(test_mapping PROPERTIES FOLDER "test/private/native_components/data_registry")
internal_fep3_participant_deploy(test_mapping)

gtest_discover_tests(test_mapping
    TIMEOUT 10
)