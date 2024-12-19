# Copyright 2023 CARIAD SE.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.


add_executable(dump_component_description
    dump_component_description.cpp
)

gtest_discover_tests(dump_component_description
    TIMEOUT 10
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../"
)

target_include_directories(dump_component_description
PRIVATE 
    ${PROJECT_SOURCE_DIR}/include/
    ${CMAKE_CURRENT_BINARY_DIR}
)

set_target_properties(dump_component_description
PROPERTIES
    FOLDER "test/private/component_description"
)

target_link_libraries(dump_component_description 
PRIVATE
    GTest::gtest_main
    GTest::gmock
    fep3_sdk_component_plugin_analyzer
)

#plugins need also installing
add_dependencies(dump_component_description
    fep_components_plugin
    fep3_dds_service_bus_plugin
    fep3_connext_dds_plugin
)

set(COMP_INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/lib)
#we test after we install, so this should be ok, although a bit confusing
target_compile_definitions(dump_component_description 
PUBLIC 
    NATIVE_COMPONENTS_PATH="${COMP_INSTALL_PATH}/$<TARGET_FILE_NAME:fep_components_plugin>"
    DDS_SIM_BUS_PATH="${COMP_INSTALL_PATH}/rti/$<TARGET_FILE_NAME:fep3_connext_dds_plugin>"
    DDS_SERVICE_BUS_PATH="${COMP_INSTALL_PATH}/rti/$<TARGET_FILE_NAME:fep3_dds_service_bus_plugin>"
)
