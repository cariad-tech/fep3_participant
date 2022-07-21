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
# TODO: Get rid of this hack as soon as possible   ##
set(CMAKE_BUILD_TYPE ${CONAN_SETTINGS_BUILD_TYPE})  #
#####################################################

fep3_participant_create_copy_target(rti_dds)
set_target_properties(fep_participant_file_copy_rti_dds PROPERTIES FOLDER "test/private/plugins")

include(../../../../../src/plugins/rti_dds/rti-macros.cmake)

changeHostSystemIfArm()
find_package(RTIConnextDDS 6.1.0 EXACT REQUIRED)
revertHostSystemIfArm()
find_package(dev_essential REQUIRED COMPONENTS memory)

add_executable(test_connext_dds_plugin tester_rti_common.cpp)
add_test(NAME test_connext_dds_plugin
    COMMAND test_connext_dds_plugin
    TIMEOUT 30
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"
)
target_link_libraries(test_connext_dds_plugin PRIVATE
    GTest::gtest_main
    GTest::gmock
    fep3_participant
    participant_test_utils
)
set_target_properties(test_connext_dds_plugin PROPERTIES FOLDER "test/private/plugins")

target_link_libraries(test_connext_dds_plugin PRIVATE fep3_participant participant_test_utils)
add_dependencies(test_connext_dds_plugin fep_participant_file_copy_rti_dds)
set_target_properties(test_connext_dds_plugin PROPERTIES INSTALL_RPATH "$ORIGIN")


add_executable(test_connext_dds_simulation_bus
    tester_dds_simbus.cpp
    USER_QOS_PROFILES.xml)

target_include_directories(test_connext_dds_simulation_bus PRIVATE
    "${CMAKE_SOURCE_DIR}/src/plugins/rti_dds")

add_test(NAME test_connext_dds_simulation_bus
    COMMAND test_connext_dds_simulation_bus
    TIMEOUT 80
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"
)
target_link_libraries(test_connext_dds_simulation_bus PRIVATE
    GTest::gtest_main
    GTest::gmock_main
    dev_essential::memory
    fep3_participant_private_lib
    participant_test_utils
)
set_target_properties(test_connext_dds_simulation_bus PROPERTIES FOLDER "test/private/plugins")
target_compile_definitions(test_connext_dds_simulation_bus PRIVATE FEP3_RTI_DDS_HTTP_SERVICE_BUS_SHARED_LIB="$<TARGET_FILE:fep3_connext_dds_plugin>")

add_dependencies(test_connext_dds_simulation_bus fep_participant_file_copy_rti_dds)
set_target_properties(test_connext_dds_simulation_bus PROPERTIES INSTALL_RPATH "$ORIGIN")




add_executable(test_connext_dds_simulation_bus_domain_id
    tester_dds_simbus_domain_id.cpp
    USER_QOS_PROFILES.xml)

target_include_directories(test_connext_dds_simulation_bus_domain_id PRIVATE
    "${CMAKE_SOURCE_DIR}/src/plugins/rti_dds")

add_test(NAME test_connext_dds_simulation_bus_domain_id
    COMMAND test_connext_dds_simulation_bus_domain_id
    TIMEOUT 40
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"
)
target_link_libraries(test_connext_dds_simulation_bus_domain_id PRIVATE
    GTest::gtest_main
    GTest::gmock_main
    dev_essential::memory
    fep3_participant_private_lib
    participant_test_utils
)
set_target_properties(test_connext_dds_simulation_bus_domain_id PROPERTIES FOLDER "test/private/plugins")
target_compile_definitions(test_connext_dds_simulation_bus_domain_id PRIVATE FEP3_RTI_DDS_HTTP_SERVICE_BUS_SHARED_LIB="$<TARGET_FILE:fep3_connext_dds_plugin>")

add_dependencies(test_connext_dds_simulation_bus_domain_id fep_participant_file_copy_rti_dds)
set_target_properties(test_connext_dds_simulation_bus_domain_id PROPERTIES INSTALL_RPATH "$ORIGIN")



add_executable(test_connext_dds_simulation_bus_late_joiner
    tester_dds_simbus_late_joiner.cpp
    USER_QOS_PROFILES.xml)

target_include_directories(test_connext_dds_simulation_bus_late_joiner PRIVATE
    "${CMAKE_SOURCE_DIR}/src/plugins/rti_dds")

add_test(NAME test_connext_dds_simulation_bus_late_joiner
    COMMAND test_connext_dds_simulation_bus_late_joiner
    TIMEOUT 40
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"
)
target_link_libraries(test_connext_dds_simulation_bus_late_joiner PRIVATE
    GTest::gtest_main
    dev_essential::memory
    fep3_participant_private_lib
    participant_test_utils
)
set_target_properties(test_connext_dds_simulation_bus_late_joiner PROPERTIES FOLDER "test/private/plugins")
target_compile_definitions(test_connext_dds_simulation_bus_late_joiner PRIVATE FEP3_RTI_DDS_HTTP_SERVICE_BUS_SHARED_LIB="$<TARGET_FILE:fep3_connext_dds_plugin>")

add_dependencies(test_connext_dds_simulation_bus_late_joiner fep_participant_file_copy_rti_dds)
set_target_properties(test_connext_dds_simulation_bus_late_joiner PROPERTIES INSTALL_RPATH "$ORIGIN")



add_executable(test_connext_dds_simulation_bus_version
    tester_dds_simbus_version.cpp
    USER_QOS_PROFILES.xml)

target_include_directories(test_connext_dds_simulation_bus_version PRIVATE
    "${CMAKE_SOURCE_DIR}/src/plugins/rti_dds")

add_test(NAME test_connext_dds_simulation_bus_version
    COMMAND test_connext_dds_simulation_bus_version
    TIMEOUT 30
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"
)
target_link_libraries(test_connext_dds_simulation_bus_version PRIVATE GTest::gtest_main GTest::gmock fep3_participant dev_essential::memory fep3_participant_private_lib participant_test_utils)
set_target_properties(test_connext_dds_simulation_bus_version PROPERTIES FOLDER "test/private/plugins")
target_compile_definitions(test_connext_dds_simulation_bus_version PRIVATE FEP3_RTI_DDS_HTTP_SERVICE_BUS_SHARED_LIB="$<TARGET_FILE:fep3_connext_dds_plugin>")

add_dependencies(test_connext_dds_simulation_bus_version fep_participant_file_copy_rti_dds)
set_target_properties(test_connext_dds_simulation_bus_version PROPERTIES INSTALL_RPATH "$ORIGIN")



add_executable(test_connext_dds_simulation_bus_performance
    tester_dds_simbus_performance.cpp
    USER_QOS_PROFILES.xml)
target_compile_definitions(test_connext_dds_simulation_bus_performance PRIVATE FEP3_RTI_DDS_HTTP_SERVICE_BUS_SHARED_LIB="$<TARGET_FILE:fep3_connext_dds_plugin>")

target_include_directories(test_connext_dds_simulation_bus_performance PRIVATE
    "${CMAKE_SOURCE_DIR}/src/plugins/rti_dds")

# TODO: add this test again  ################################
#add_test(NAME test_connext_dds_simulation_bus_performance  #
    #COMMAND test_connext_dds_simulation_bus_performance    #
    #TIMEOUT 30                                             #
    #WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"       #
#)                                                          #
#############################################################
target_link_libraries(test_connext_dds_simulation_bus_performance PRIVATE
    GTest::gtest_main
    fep3_participant
    dev_essential::memory
    fep3_participant_private_lib
    participant_test_utils
)
set_target_properties(test_connext_dds_simulation_bus_performance PROPERTIES FOLDER "test/private/plugins")
target_compile_definitions(test_connext_dds_simulation_bus_performance PRIVATE FEP3_RTI_DDS_HTTP_SERVICE_BUS_SHARED_LIB="$<TARGET_FILE:fep3_connext_dds_plugin>")

add_dependencies(test_connext_dds_simulation_bus_performance fep_participant_file_copy_rti_dds)
set_target_properties(test_connext_dds_simulation_bus_performance PROPERTIES INSTALL_RPATH "$ORIGIN")
