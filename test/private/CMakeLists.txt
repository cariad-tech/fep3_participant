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



######################################################################################
# setup fep3_participant_testing library
# which is only a static library wrapper around the fep3_participant_object_lib
######################################################################################

# disable boost autolink, otherwise boost will look the library with the wrong name
add_compile_definitions(BOOST_ALL_NO_LIB)
find_package(Boost 1.73.0 REQUIRED COMPONENTS filesystem)

add_library(fep3_participant_private_lib STATIC
    $<TARGET_OBJECTS:fep3_participant_object_lib>
    $<TARGET_OBJECTS:fep_components_plugin_object_lib>
)
target_link_libraries(fep3_participant_private_lib PUBLIC
    fep3_participant_object_lib
    Boost::filesystem
    Boost::disable_autolinking
)

target_compile_definitions(fep3_participant_private_lib PUBLIC _FEP3_PARTICIPANT_INCLUDED_STATIC)
set_target_properties(fep3_participant_private_lib PROPERTIES FOLDER "test/libraries")

add_subdirectory(utils)
add_subdirectory(components_configuration/src)
add_subdirectory(plugin/c/src)
add_subdirectory(foreign_components/c/src)
add_subdirectory(foreign_components/cpp/src)
add_subdirectory(component_factory/cpp/src)
add_subdirectory(component_registry/src)
add_subdirectory(component_registry_factory/src)
add_subdirectory(component_intfs)
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