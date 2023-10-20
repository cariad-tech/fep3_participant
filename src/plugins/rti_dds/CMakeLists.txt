#
# Copyright @ 2021 VW Group. All rights reserved.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
#

# These two variables are needed so that the ndds find script can find the libraries we want to use.
# The ndds recipe will set environment variables that will be written to the cache when CMake is
# initially instrumented.
set(CONNEXTDDS_DIR "$ENV{NDDSHOME}" CACHE PATH "Path to RTI Connext DDS installation")
set(CONNEXTDDS_ARCH $ENV{CONNEXTDDS_ARCH} CACHE STRING "RTI platform string RTI Connext shall be using")

if (NOT CONNEXTDDS_DIR)
    message(FATAL_ERROR "CONNEXTDDS_DIR not set.")
endif()

include(rti-macros.cmake)

# Hack (*shiver*) to convince RTI Connext DDS find script to support native armv8 compiling
changeHostSystemIfArm()
# Execute (now hacked) find package script
find_package(RTIConnextDDS 6.1.0 EXACT REQUIRED)

if(NOT TARGET dev_essential)
    find_package(dev_essential 1.3.0 REQUIRED COMPONENTS pkg_rpc system filesystem process ddl)
endif()

# Undo hack
revertHostSystemIfArm()

set(SIMULATION_BUS_SOURCES types/stream_types.cxx
            fep_connext_simulation_bus_dds_plugin.cpp
            types/stream_typesPlugin.cxx

            simulation_bus/topic_intf.h
            simulation_bus/converter.h
            simulation_bus/converter.cpp
            simulation_bus/reader_item_queue.h
            simulation_bus/reader_item_queue.cpp
            simulation_bus/vector_raw_memory.hpp

            simulation_bus/stream_item_topic/stream_item_reader.h
            simulation_bus/stream_item_topic/stream_item_reader.cpp
            simulation_bus/stream_item_topic/stream_item_topic.h
            simulation_bus/stream_item_topic/stream_item_topic.cpp
            simulation_bus/stream_item_topic/stream_item_writer.h
            simulation_bus/stream_item_topic/stream_item_writer.cpp

            simulation_bus/internal_topic/internal_topic.h
            simulation_bus/internal_topic/internal_topic.cpp

            simulation_bus/fep_connext_dds_simulation_bus.cpp
            simulation_bus/fep_connext_dds_simulation_bus.h

            simulation_bus/bus_info/bus_info.h
            simulation_bus/bus_info/bus_info.cpp

            USER_QOS_PROFILES.xml)

###################################################
# create fep3_connext_dds_simulation_bus_object_lib
###################################################

add_library(fep3_connext_dds_simulation_bus_object_lib OBJECT)
target_sources(fep3_connext_dds_simulation_bus_object_lib PRIVATE ${SIMULATION_BUS_SOURCES})

set_target_properties(fep3_connext_dds_simulation_bus_object_lib PROPERTIES FOLDER "libraries")

target_include_directories(fep3_connext_dds_simulation_bus_object_lib PUBLIC
    "$<BUILD_INTERFACE:${BUILD_INTERFACE_INCLUDES}>"
)

target_link_libraries(fep3_connext_dds_simulation_bus_object_lib
    PUBLIC dev_essential::ddl
           dev_essential::pkg_rpc
           dev_essential::filesystem
           dev_essential::system
           RTIConnextDDS::cpp2_api
    PRIVATE fep3_participant_cpp_plugin
            fep3_environment_variable
)

if(MSVC)
    # warning coming from dds headers
    target_compile_options(fep3_connext_dds_simulation_bus_object_lib PRIVATE /wd4245 /wd4244 /wd4996)
    target_link_libraries(fep3_connext_dds_simulation_bus_object_lib PRIVATE "Iphlpapi.lib")
endif()

###################################################
# create fep3_connext_dds_plugin
###################################################

set(PLUGIN_NAME fep3_connext_dds_plugin)
add_library(${PLUGIN_NAME} SHARED
    $<TARGET_OBJECTS:fep3_connext_dds_simulation_bus_object_lib>)

set_target_properties(${PLUGIN_NAME} PROPERTIES FOLDER "plugins/cpp/connext_dds_plugin")
source_group("service_bus" FILES ${SERVICE_BUS_SOURCES})

target_include_directories(${PLUGIN_NAME}
    PRIVATE
    ${PROJECT_SOURCE_DIR}/src/fep3/native_components/service_bus/rpc/http/service_discovery
    ${PROJECT_SOURCE_DIR}/src/fep3/native_components/service_bus/rpc//http/service_discovery/dds_service_discovery
    ${PROJECT_SOURCE_DIR}/src/fep3/native_components/service_bus/rpc/http/
)

target_link_libraries(${PLUGIN_NAME} PRIVATE
    fep3_connext_dds_simulation_bus_object_lib)

if(MSVC)
    install(FILES $<TARGET_PDB_FILE:${PLUGIN_NAME}> DESTINATION lib/rti OPTIONAL)
endif()

rti_deploy(${PLUGIN_NAME})
rti_install(lib/rti)

install(TARGETS ${PLUGIN_NAME}
        EXPORT ${PLUGIN_NAME}_targets
        LIBRARY NAMELINK_SKIP DESTINATION lib/rti
        RUNTIME DESTINATION lib/rti)

install(EXPORT ${PLUGIN_NAME}_targets DESTINATION lib/cmake)

## DDS SERVICE DISCOVERY LIBRARY
find_package(Boost 1.73.0 REQUIRED COMPONENTS headers)

set(DDS_SERVICE_DISCOVERY_SOURCES
    ${PROJECT_SOURCE_DIR}/src/fep3/native_components/service_bus/service_bus.cpp
    ${PROJECT_SOURCE_DIR}/src/fep3/native_components/service_bus/service_bus_configuration.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/service_bus/dds_service_discovery/src/host_name_resolver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/service_bus/dds_service_discovery/src/host_name_resolver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/service_bus/dds_factory/service_discovery_factory_dds.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/service_bus/dds_service_discovery/src/dds_service_discovery.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/service_bus/dds_service_discovery/src/DdsServiceDiscoveryTopic.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/service_bus/dds_service_discovery/src/DdsServiceDiscoveryTopicPlugin.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/service_bus/dds_service_discovery/src/service_discovery_dds_topic_listener.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/service_bus/dds_service_discovery/src/dds_service_update_handler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/service_bus/dds_service_discovery/src/dds_service_finder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/service_bus/dds_service_discovery/src/service_discovery_topic_writer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/service_bus/dds_service_discovery/src/dds_service_discovery_participant.cpp)

set(DDS_SERVICE_DISCOVERY_HEADERS
   ${PROJECT_SOURCE_DIR}/src/fep3/native_components/service_bus/include/service_bus.h
   ${PROJECT_SOURCE_DIR}/src/fep3/native_components/service_bus/include/service_bus_configuration.h
   ${CMAKE_CURRENT_SOURCE_DIR}/service_bus/dds_factory/include/service_discovery_factory_dds.h
   ${CMAKE_CURRENT_SOURCE_DIR}/service_bus/dds_factory/include/conversion_function.h
   ${CMAKE_CURRENT_SOURCE_DIR}/service_bus/dds_service_discovery/include/host_name_resolver_intf.h
   ${CMAKE_CURRENT_SOURCE_DIR}/service_bus/dds_service_discovery/include/dds_service_discovery.h
   ${CMAKE_CURRENT_SOURCE_DIR}/service_bus/dds_service_discovery/include/DdsServiceDiscoveryTopic.hpp
   ${CMAKE_CURRENT_SOURCE_DIR}/service_bus/dds_service_discovery/include/DdsServiceDiscoveryTopicPlugin.hpp
   ${CMAKE_CURRENT_SOURCE_DIR}/service_bus/dds_service_discovery/include/service_discovery_dds_topic_listener.h
   ${CMAKE_CURRENT_SOURCE_DIR}/service_bus/dds_service_discovery/include/dds_service_discovery_topic_handler.h
   ${CMAKE_CURRENT_SOURCE_DIR}/service_bus/dds_service_discovery/include/dds_service_update_handler.h
   ${CMAKE_CURRENT_SOURCE_DIR}/service_bus/dds_service_discovery/include/dds_service_finder.h
   ${CMAKE_CURRENT_SOURCE_DIR}/service_bus/dds_service_discovery/include/service_discovery_topic_writer.h
   ${CMAKE_CURRENT_SOURCE_DIR}/service_bus/dds_service_discovery/include/dds_service_discovery_participant.h)

add_library(fep3_dds_service_discovery_lib STATIC
            ${DDS_SERVICE_DISCOVERY_SOURCES}
            ${DDS_SERVICE_DISCOVERY_HEADERS}
            ${SERVICE_DISCOVERY_HEADERS})

set_target_properties(fep3_dds_service_discovery_lib PROPERTIES POSITION_INDEPENDENT_CODE ON)

if(MSVC)
    set_source_files_properties(
        ${CMAKE_CURRENT_SOURCE_DIR}/service_bus/dds_service_discovery/src/DdsServiceDiscoveryTopic.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/service_bus/dds_service_discovery/include/DdsServiceDiscoveryTopicPlugin.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/service_bus/dds_service_discovery/include/instancePlugin.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/service_bus/dds_service_discovery/src/DdsServiceDiscoveryTopicPlugin.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/service_bus/dds_service_discovery/src/dds_service_discovery.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/service_bus/dds_service_discovery/src/service_discovery_dds_topic_listener.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/service_bus/dds_service_discovery/src/dds_service_finder.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/service_bus/dds_service_discovery/src/service_discovery_topic_writer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/service_bus/dds_service_discovery/src/dds_service_update_handler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/service_bus/dds_service_discovery/include/dds_participant_singleton.h
        ${CMAKE_CURRENT_SOURCE_DIR}/service_bus/dds_service_discovery/src/dds_participant_singleton.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/service_bus/dds_factory/service_discovery_factory_dds.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/service_bus/dds_factory/include/service_discovery_factory_dds.h
        ${PROJECT_SOURCE_DIR}/src/fep3/native_components/service_bus/include/service_bus.h
        ${PROJECT_SOURCE_DIR}/src/fep3/native_components/service_bus/include/service_bus_configuration.h
        ${PROJECT_SOURCE_DIR}/src/fep3/native_components/service_bus/service_bus.cpp
        ${PROJECT_SOURCE_DIR}/src/fep3/native_components/service_bus/service_bus_configuration.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/service_bus/dds_service_discovery/include/dds_service_discovery_participant.h
        ${CMAKE_CURRENT_SOURCE_DIR}/service_bus/dds_service_discovery/src/dds_service_discovery_participant.cpp
    PROPERTIES COMPILE_FLAGS /wd4245)
endif(MSVC)

set_source_files_properties(
        simulation_bus/internal_topic/internal_topic.cpp
        simulation_bus/stream_item_topic/stream_item_topic.cpp
        PROPERTIES COMPILE_DEFINITIONS $<$<CXX_COMPILER_ID:MSVC>:NOMINMAX>)

target_include_directories(fep3_dds_service_discovery_lib
PUBLIC
 ${CMAKE_CURRENT_SOURCE_DIR}/service_bus/dds_factory/include/
 ${CMAKE_CURRENT_SOURCE_DIR}/service_bus/dds_service_discovery/include/
 ${PROJECT_SOURCE_DIR}/src/fep3/native_components/service_bus/include/)

 target_link_libraries(fep3_dds_service_discovery_lib PUBLIC
            RTIConnextDDS::cpp2_api
            Boost::headers
            dev_essential::result
            fep3_service_bus_base_lib
            fep3_thread_utilities
            PRIVATE
            dev_essential::system
            dev_essential::process
            fep3_lssdp)

set_target_properties(fep3_dds_service_discovery_lib PROPERTIES FOLDER "plugins/cpp/connext_dds_plugin")
target_compile_definitions(fep3_dds_service_discovery_lib PRIVATE _FEP3_PARTICIPANT_DO_EXPORT DDS_SERVICE_DISCOVERY)

# SERVICE BUS PLUGIN
add_library(fep3_dds_service_bus_plugin SHARED
            fep_connext_service_bus_dds_plugin.cpp
            )

if(MSVC)
    # warning coming from dds headers
    target_compile_options(fep3_dds_service_bus_plugin PRIVATE /wd4245 /wd4244 /wd4996)
    target_link_libraries(fep3_dds_service_bus_plugin PRIVATE "Iphlpapi.lib")
    install(FILES $<TARGET_PDB_FILE:fep3_dds_service_bus_plugin> DESTINATION lib/rti OPTIONAL)
endif()

rti_deploy(fep3_dds_service_bus_plugin)
rti_install(lib/rti)

install(TARGETS fep3_dds_service_bus_plugin
        EXPORT fep3_dds_service_bus_plugin_targets
        LIBRARY NAMELINK_SKIP DESTINATION lib/rti
        RUNTIME DESTINATION lib/rti)

install(EXPORT fep3_dds_service_bus_plugin_targets DESTINATION lib/cmake)

target_include_directories(fep3_dds_service_bus_plugin
    PRIVATE
    ${PROJECT_SOURCE_DIR}/src/fep3/native_components/service_bus/include
    ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(fep3_dds_service_bus_plugin
            PUBLIC
            PRIVATE
            fep3_participant_cpp_plugin
            fep3_dds_service_discovery_lib
            dev_essential::system)

set_target_properties(fep3_dds_service_bus_plugin PROPERTIES FOLDER "plugins/cpp/connext_dds_plugin")
