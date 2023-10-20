include(./cmake.sources)
set(SERVICE_BUS_DIR ${PROJECT_SOURCE_DIR}/src/fep3/native_components/service_bus/)

set(SERVICE_DISCOVERY_INTF_DIR ${SERVICE_BUS_DIR}/rpc/service_discovery/service_discovery_intf)
add_library(fep3_service_discovery_interface INTERFACE)

target_include_directories(fep3_service_discovery_interface INTERFACE ${SERVICE_DISCOVERY_INTF_DIR})

set(SERVICE_BUS_RPC_DIR ${SERVICE_BUS_DIR}/rpc/http)
add_library(fep3_service_bus_base_lib STATIC
${SERVICE_BUS_RPC_DIR}/src/http_systemaccess.cpp
${SERVICE_BUS_RPC_DIR}/src/find_free_port.cpp
${SERVICE_BUS_RPC_DIR}/src/http_client.cpp
${SERVICE_BUS_RPC_DIR}/src/http_server.cpp
${SERVICE_BUS_RPC_DIR}/src/http_url_getter.cpp
${SERVICE_BUS_RPC_DIR}/src/service_update_sink_registry.cpp
${SERVICE_BUS_RPC_DIR}/include/http_systemaccess.h
${SERVICE_BUS_RPC_DIR}/include/find_free_port.h
${SERVICE_BUS_RPC_DIR}/include/http_client.h
${SERVICE_BUS_RPC_DIR}/include/http_server.h
${SERVICE_BUS_RPC_DIR}/include/http_url_getter.h
${SERVICE_BUS_RPC_DIR}/include/service_update_sink_registry.h
${PROJECT_SOURCE_DIR}/include/fep3/components/service_bus/service_bus_intf.h
${PROJECT_SOURCE_DIR}/include/fep3/components/service_bus/service_registry_base.hpp
${PROJECT_SOURCE_DIR}/include/fep3/components/service_bus/system_access_base.hpp
${PROJECT_SOURCE_DIR}/include/fep3/components/service_bus/rpc/rpc_intf.h
${PROJECT_SOURCE_DIR}/include/fep3/components/service_bus/rpc/fep_json_rpc.h
${PROJECT_SOURCE_DIR}/include/fep3/components/service_bus/rpc/fep_rpc.h
${PROJECT_SOURCE_DIR}/include/fep3/components/service_bus/rpc/fep_rpc_stubs_client.h
${PROJECT_SOURCE_DIR}/include/fep3/components/service_bus/rpc/fep_rpc_stubs_service.h
${SERVICE_BUS_DIR}/rpc/logger/service_bus_logger.hpp
)

source_group("Header Files/rpc" FILES ${PROJECT_SOURCE_DIR}/include/fep3/components/service_bus/rpc/rpc_intf.h
${PROJECT_SOURCE_DIR}/include/fep3/components/service_bus/rpc/fep_json_rpc.h
${PROJECT_SOURCE_DIR}/include/fep3/components/service_bus/rpc/fep_rpc.h
${PROJECT_SOURCE_DIR}/include/fep3/components/service_bus/rpc/fep_rpc_stubs_client.h
${PROJECT_SOURCE_DIR}/include/fep3/components/service_bus/rpc/fep_rpc_stubs_service.h)

if(NOT TARGET dev_essential)
    find_package(dev_essential 1.3.0 REQUIRED COMPONENTS result pkg_rpc system process)
endif()

find_package(Boost REQUIRED COMPONENTS headers)

target_include_directories(fep3_service_bus_base_lib
    PUBLIC
    #service_bus.h
    ${SERVICE_BUS_RPC_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/fep3/components/service_bus
    # for fep results, and fep3/base
    ${PROJECT_SOURCE_DIR}/include/
    ${PROJECT_BINARY_DIR}/include/
    ${SERVICE_BUS_DIR}/rpc/logger/
    ${SERVICE_BUS_DIR}/rpc/http/include
    PRIVATE
    ${SERVICE_BUS_DIR})

set_target_properties(fep3_service_bus_base_lib PROPERTIES FOLDER "plugins/cpp/service_discovery")
set_target_properties(fep3_service_bus_base_lib PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(fep3_service_bus_base_lib
    PUBLIC fep3_service_discovery_interface
    dev_essential::result
    dev_essential::pkg_rpc
    dev_essential::process
    fep3_thread_utilities
    PRIVATE
    fep3_environment_variable
    fep3_lssdp
    Boost::headers)
add_dependencies(fep3_service_bus_base_lib service_bus_rpc_stub_generator)
## LSSDP Service Discovery

add_library(fep3_lssdp_service_discovery_lib STATIC
            ${PROJECT_SOURCE_DIR}/src/fep3/native_components/service_bus/include/service_bus.h
            ${PROJECT_SOURCE_DIR}/src/fep3/native_components/service_bus/include/service_bus_configuration.h
            ${PROJECT_SOURCE_DIR}/src/fep3/native_components/service_bus/service_bus.cpp
            ${PROJECT_SOURCE_DIR}/src/fep3/native_components/service_bus/service_bus_configuration.cpp
            ${PROJECT_SOURCE_DIR}/src/fep3/native_components/service_bus/rpc/service_discovery/lssdp_factory/include/service_discovery_factory_lssdp.h
            ${PROJECT_SOURCE_DIR}/src/fep3/native_components/service_bus/rpc/service_discovery/lssdp_factory/src/service_discovery_factory_lssdp.cpp)

set_target_properties(fep3_lssdp_service_discovery_lib PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(fep3_lssdp_service_discovery_lib PROPERTIES FOLDER "plugins/cpp/service_discovery")

target_include_directories(fep3_lssdp_service_discovery_lib
    PUBLIC
    ${PROJECT_SOURCE_DIR}/src/fep3/native_components/service_bus/include
    ${PROJECT_SOURCE_DIR}/src/fep3/native_components/service_bus/rpc/service_discovery/lssdp_factory/include/)

target_link_libraries(fep3_lssdp_service_discovery_lib
                                             PUBLIC
                                             fep3_service_bus_base_lib
                                             PRIVATE
                                             fep3_lssdp
                                             dev_essential::system
                                             dev_essential::concurrency)
target_compile_definitions(fep3_lssdp_service_discovery_lib PRIVATE LSSDP_SERVICE_DISCOVERY)
