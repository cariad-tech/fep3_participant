# Copyright 2023 CARIAD SE.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

if(NOT TARGET dev_essential)
    find_package(dev_essential 1.3.0 REQUIRED COMPONENTS pkg_rpc result)
else()
    include(${dev_essential_SOURCE_DIR}/scripts/cmake/stub_generation.cmake)
endif()

set(PLUGIN_NAME fep3_http_service_bus)
add_library(${PLUGIN_NAME} SHARED
            fep_http_plugin.cpp)

set_target_properties(${PLUGIN_NAME} PROPERTIES FOLDER "plugins/cpp")

target_link_libraries(${PLUGIN_NAME} PRIVATE
                                        fep3_participant_cpp_plugin
                                        fep3_lssdp_service_discovery_lib
                                        dev_essential::result)

target_include_directories(${PLUGIN_NAME}
    PRIVATE
    ${PROJECT_SOURCE_DIR}/include)

if(MSVC)
    target_link_libraries(${PLUGIN_NAME} PRIVATE "Iphlpapi.lib")
    install(FILES $<TARGET_PDB_FILE:${PLUGIN_NAME}> DESTINATION lib/http OPTIONAL)
endif()

install(TARGETS ${PLUGIN_NAME}
        EXPORT ${PLUGIN_NAME}_targets
        LIBRARY NAMELINK_SKIP DESTINATION lib/http
        RUNTIME DESTINATION lib/http
)
install(EXPORT ${PLUGIN_NAME}_targets DESTINATION lib/cmake)
