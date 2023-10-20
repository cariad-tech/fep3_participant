set(COMPONENT_PLUGIN_RPC_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include/fep3/rpc_services/)
set(COMPONENT_PLUGIN_RPC_BINARY_DIR ${PROJECT_BINARY_DIR}/include/fep3/rpc_services)
set(COMPONENT_PLUGIN_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include/fep3/components)

set(FEP_COMPONENT_PLUGIN_SOURCES)
set(FEP_COMPONENT_PLUGIN_STUB_GENERATORS)

add_compile_definitions(LSSDP_SERVICE_DISCOVERY)

if(NOT TARGET dev_essential)
    find_package(dev_essential 1.3.0 REQUIRED COMPONENTS pkg_rpc)
endif()
find_package(Boost 1.73.0 REQUIRED COMPONENTS date_time thread headers)

include(src/data_registry/cmake.sources)
include(src/scheduler/cmake.sources)
include(src/clock/cmake.sources)
include(src/clock_sync/cmake.sources)
include(src/simulation_bus/cmake.sources)
include(src/logging/cmake.sources)
include(src/job_registry/cmake.sources)
include(src/configuration/cmake.sources)
include(src/participant_info/cmake.sources)
include(src/health/cmake.sources)
include(src/plugin/cmake.sources)

add_library(fep_components_plugin_object_lib OBJECT)
target_sources(fep_components_plugin_object_lib PRIVATE ${FEP_COMPONENT_PLUGIN_SOURCES})
set_target_properties(fep_components_plugin_object_lib PROPERTIES POSITION_INDEPENDENT_CODE ON)
#lsddp GetAdaptersInfo referenced in function lssdp::updateNetworkInterfaces
if(WIN32)
  target_link_libraries(fep_components_plugin_object_lib PRIVATE Iphlpapi)
endif()

target_link_libraries(fep_components_plugin_object_lib
    PUBLIC
    dev_essential::pkg_rpc
    dev_essential::ddl
    fep3_participant
    fep3_lssdp_service_discovery_lib
    $<BUILD_INTERFACE:fep3_thread_utilities>
    PRIVATE
    dev_essential::filesystem
    dev_essential::datetime
    dev_essential::system
    Boost::thread
    Boost::headers
    fep3_participant_cpp_plugin
    Boost::date_time
    Boost::headers
    Boost::thread
    fep3_default_severity_from_env_variable
    $<INSTALL_INTERFACE:fep3_thread_utilities>)


target_include_directories(fep_components_plugin_object_lib
                           PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/src")

add_dependencies(fep_components_plugin_object_lib ${FEP_COMPONENT_PLUGIN_STUB_GENERATORS})

add_library(fep_components_plugin SHARED
    $<TARGET_OBJECTS:fep_components_plugin_object_lib>)

target_link_libraries(fep_components_plugin PRIVATE fep_components_plugin_object_lib)
install(
    TARGETS fep_components_plugin
    EXPORT fep_components_plugin_targets
    LIBRARY NAMELINK_SKIP DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION lib
)
install(EXPORT fep_components_plugin_targets DESTINATION lib/cmake)

if(WIN32)
    install(FILES $<TARGET_PDB_FILE:fep_components_plugin> DESTINATION lib OPTIONAL)
endif(WIN32)

#we always need a fep_components file
install(FILES $<TARGET_FILE_DIR:fep3_participant>/fep3_participant.fep_components DESTINATION lib)

set_target_properties(fep_components_plugin PROPERTIES FOLDER "plugins/cpp")
set_target_properties(fep_components_plugin_object_lib PROPERTIES FOLDER "plugins/cpp")

add_subdirectory(src)
