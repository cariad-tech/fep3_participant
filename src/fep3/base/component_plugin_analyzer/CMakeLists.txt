# Copyright 2023 CARIAD SE.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

find_package(dev_essential 1.3.0 REQUIRED COMPONENTS pkg_rpc result)

set(GENERATED_STUB_DIR "${CMAKE_CURRENT_BINARY_DIR}")

add_custom_target(component_plugin_analyzer_rpc_stub_generator)
fep_generate_rpc_stubs_before_target(
    TARGET component_plugin_analyzer_rpc_stub_generator
    INPUT_FILE "${PROJECT_SOURCE_DIR}/include/fep3/rpc_services/data_registry/data_registry.json"
    OUTPUT_DIR "${GENERATED_STUB_DIR}"
    CLIENT_CLASS_NAME fep3::rpc_stubs::RPCDataRegistryClientStub
    CLIENT_FILE_NAME data_registry_client_stub.h
)

find_package(Boost 1.73.0 REQUIRED)

set(PUBLIC_HEADER
    "${PROJECT_SOURCE_DIR}/include/fep3/base/component_plugin_analyzer/component_description_base.h"
    "${PROJECT_SOURCE_DIR}/include/fep3/base/component_plugin_analyzer/component_plugin_analyzer.h"
)

add_library(fep3_sdk_component_plugin_analyzer
STATIC
    ${PUBLIC_HEADER}
    component_plugin_analyzer.cpp
    component_registry_spy.cpp
    component_registry_spy.h
    property_tester.cpp
    property_tester.h
    component_factory.h
    component_factory.cpp
    component_path_hash_function.h
    signal_tester.h
    signal_tester.cpp
    data_registry_rpc_client.h
    data_registry_rpc_client.cpp
    result_printer.cpp
    result_printer.h
    "${GENERATED_STUB_DIR}/data_registry_client_stub.h"
)

add_dependencies(fep3_sdk_component_plugin_analyzer
    component_plugin_analyzer_rpc_stub_generator)

target_include_directories(fep3_sdk_component_plugin_analyzer
PUBLIC
    "$<BUILD_INTERFACE:${GENERATED_STUB_DIR}>" 
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:include>"
PRIVATE
    ${Boost_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/src/
)

set_target_properties(fep3_sdk_component_plugin_analyzer
PROPERTIES 
    FOLDER "libraries/base"
    PUBLIC_HEADER "${PUBLIC_HEADER}"
    OUTPUT_NAME fep3_sdk_component_plugin_analyzer
    DEBUG_POSTFIX "d${FEP3_PARTICIPANT_LIBRARY_VERSION_MAJOR}.${FEP3_PARTICIPANT_LIBRARY_VERSION_MINOR}"
    RELEASE_POSTFIX "${FEP3_PARTICIPANT_LIBRARY_VERSION_MAJOR}.${FEP3_PARTICIPANT_LIBRARY_VERSION_MINOR}"
    RELWITHDEBINFO_POSTFIX "${FEP3_PARTICIPANT_LIBRARY_VERSION_MAJOR}.${FEP3_PARTICIPANT_LIBRARY_VERSION_MINOR}"
)

target_link_libraries(fep3_sdk_component_plugin_analyzer 
PUBLIC
    dev_essential::result
PRIVATE
    fep3_component_registry
    dev_essential::pkg_rpc
)
#dev_essential::pkg_rpc requires linking for linux
fep3_participant_link_pthread(fep3_sdk_component_plugin_analyzer)

#see commit message on installing public headers
install(
    TARGETS fep3_sdk_component_plugin_analyzer
    EXPORT fep3_sdk_component_plugin_analyzer_targets
    LIBRARY NAMELINK_SKIP DESTINATION lib
    ARCHIVE DESTINATION lib
    FILE_SET HEADERS
    RUNTIME DESTINATION lib
    PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/fep3/base/component_plugin_analyzer"
)

install(EXPORT fep3_sdk_component_plugin_analyzer_targets DESTINATION lib/cmake)
install(FILES fep3_sdk_component_plugin_analyzer-config.cmake DESTINATION .)
