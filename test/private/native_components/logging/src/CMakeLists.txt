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

find_package(Boost 1.73.0 REQUIRED COMPONENTS filesystem regex)
find_package(dev_essential REQUIRED COMPONENTS system)

# disable boost autolink, otherwise boost will look the library with the wrong name
add_compile_definitions(BOOST_ALL_NO_LIB)

add_executable(tester_logging_service
    tester_rpc_log.cpp
    tester_console_log.cpp
    tester_file_log.cpp
    tester_logging_config.cpp
    tester_logging_sink_base.cpp
    tester_logging_sink_file_json.cpp
    tester_logging_sink_folder_setup
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
    Boost::filesystem
    Boost::regex
    participant_private_test_utils
    fep3_participant_private_lib
)

set_target_properties(tester_logging_service PROPERTIES FOLDER "test/private/native_components")
