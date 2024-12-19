# Copyright 2023 CARIAD SE.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.



######################################################################################
# setup fep3_participant_testing library
# which is only a static library wrapper around the fep3_participant_object_lib
######################################################################################

find_package(Boost 1.73.0 REQUIRED COMPONENTS thread)
include(../../scripts/cmake/set_boost_interface_targets.cmake)

add_library(fep3_participant_private_lib STATIC
    $<TARGET_OBJECTS:fep3_participant_object_lib>
    $<TARGET_OBJECTS:fep_components_plugin_object_lib>
)
target_link_libraries(fep3_participant_private_lib PUBLIC
    fep3_participant_object_lib
    fep_components_plugin_object_lib
    fep3_component_registry
    Boost::headers
    Boost::thread
)

target_include_directories(fep3_participant_private_lib PUBLIC
                           ${PROJECT_SOURCE_DIR}/src/fep3/base/component_registry/include)

target_compile_definitions(fep3_participant_private_lib PUBLIC _FEP3_PARTICIPANT_INCLUDED_STATIC)
set_target_properties(fep3_participant_private_lib PROPERTIES FOLDER "test/libraries")

add_subdirectory(base)
add_subdirectory(utils)
add_subdirectory(components_configuration/src)
add_subdirectory(foreign_components/cpp/src)
add_subdirectory(component_description)
add_subdirectory(component_factory/cpp/src)
add_subdirectory(component_registry/src)
add_subdirectory(component_registry_factory/src)
add_subdirectory(participant/core/src)
add_subdirectory(participant/cpp/src)
add_subdirectory(shared_library/src)
add_subdirectory(console_logger/src)

#native components are built in classes which are not exported
add_subdirectory(native_components/clock/src)
add_subdirectory(native_components/clock_sync/src)
add_subdirectory(native_components/simulation_bus/src)
add_subdirectory(native_components/service_bus/src)
add_subdirectory(native_components/scheduler/src)
add_subdirectory(native_components/data_registry/src)
add_subdirectory(native_components/health/src)
add_subdirectory(native_components/job_registry/src)
add_subdirectory(native_components/configuration/src)
add_subdirectory(native_components/logging/src)
add_subdirectory(native_components/participant_info/src)

add_subdirectory(native_components/integration/scheduling/src)

#participant interface
add_subdirectory(participant/interface/state_machine/src)
add_subdirectory(participant/interface/rpc_component_registry/src)
add_subdirectory(participant/cli/src)

#rti_dds
if(fep3_participant_use_rtidds)
    add_subdirectory(plugin/rti_dds/src)
endif()
