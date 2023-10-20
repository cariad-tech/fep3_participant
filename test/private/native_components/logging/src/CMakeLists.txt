#
# Copyright @ 2021 VW Group. All rights reserved.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

find_package(dev_essential REQUIRED COMPONENTS system process)
find_package(Boost REQUIRED COMPONENTS filesystem)

add_executable(tester_logging_service
    tester_rpc_log.cpp
    tester_console_log.cpp
    tester_file_log.cpp
    tester_logging_config.cpp
    tester_logging_sink_file_base.cpp
    tester_logging_sink_file_json.cpp
    tester_logging_sink_file_csv.cpp
    tester_logging_sink_file.h
)

get_filename_component(test_log_files_dir "./../files"
                       REALPATH  BASE_DIR ${CMAKE_CURRENT_BINARY_DIR})

target_compile_definitions(tester_logging_service
            PRIVATE TEST_LOGFILES_DIR="${test_log_files_dir}"
        )

add_test(NAME tester_logging_service
    COMMAND tester_logging_service
    TIMEOUT 10
    WORKING_DIRECTORY ".."
)

target_link_libraries(tester_logging_service PRIVATE
    GTest::gtest_main
    dev_essential::system
    dev_essential::process
    participant_private_test_utils
    fep3_participant_private_lib
    fep3_components_test
    Boost::filesystem
)

set_target_properties(tester_logging_service PROPERTIES FOLDER "test/private/native_components")
