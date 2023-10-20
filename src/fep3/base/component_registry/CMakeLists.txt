#
# Copyright @ 2022 VW Group. All rights reserved.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
if(NOT TARGET dev_essential)
    find_package(dev_essential 1.3.0 REQUIRED COMPONENTS result process xml)
endif()

find_package(Boost 1.73.0 REQUIRED COMPONENTS filesystem)

add_library(fep3_component_registry STATIC

            ${CMAKE_CURRENT_SOURCE_DIR}/include/component_factory/component_factory_cpp_plugins.h
            ${CMAKE_CURRENT_SOURCE_DIR}/src/component_factory/component_factory_cpp_plugins.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/include/component_factory/component_factory_base.h
            ${CMAKE_CURRENT_SOURCE_DIR}/include/component_factory/component_source_type.h
            ${CMAKE_CURRENT_SOURCE_DIR}/src/component_factory/component_source_type.cpp

            ${CMAKE_CURRENT_SOURCE_DIR}/src/component_registry.cpp

            ${CMAKE_CURRENT_SOURCE_DIR}/src/component_registry_factory/component_registry_factory.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/component_registry_factory/components_configuration.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/include/component_registry_factory/components_configuration.h

            ${CMAKE_CURRENT_SOURCE_DIR}/src/file/file.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/include/file/file.h

            ${PROJECT_SOURCE_DIR}/include/fep3/base/component_registry/component_registry.h
            ${PROJECT_SOURCE_DIR}/include/fep3/base/component_registry/component_registry_factory.h
            ${PROJECT_SOURCE_DIR}/include/fep3/base/component_registry/component_version_info.h

            ${CMAKE_CURRENT_SOURCE_DIR}/include/plugin/base/shared_library.h
            ${CMAKE_CURRENT_SOURCE_DIR}/include/plugin/base/host_plugin_base.h
            ${CMAKE_CURRENT_SOURCE_DIR}/include/plugin/cpp/cpp_host_plugin.h

            ${CMAKE_CURRENT_SOURCE_DIR}/src/plugin/cpp/cpp_host_plugin.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/plugin/base/shared_library.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/plugin/base/host_plugin_base.cpp

            )

target_include_directories(fep3_component_registry
                           PUBLIC
                              $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/component_registry>
                              $<INSTALL_INTERFACE:include/fep3/base/component_registry>
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
                                   ${PROJECT_SOURCE_DIR}/include/
                                   ${PROJECT_SOURCE_DIR}/src
                                   ${PROJECT_BINARY_DIR}/include
                           )

set_target_properties(fep3_component_registry PROPERTIES FOLDER "libraries/base")
target_link_libraries(fep3_component_registry PUBLIC
                                              dev_essential::result
                                              PRIVATE
                                              Boost::filesystem
                                              dev_essential::process
                                              dev_essential::xml)

#set_source_files_properties(
#        ${CMAKE_CURRENT_SOURCE_DIR}/environment_variable.cpp
#        PROPERTIES COMPILE_DEFINITIONS $<$<CXX_COMPILER_ID:MSVC>:NOMINMAX>)

set_target_properties(fep3_component_registry PROPERTIES
    OUTPUT_NAME fep3_component_registry
    DEBUG_POSTFIX "d${FEP3_PARTICIPANT_LIBRARY_VERSION_MAJOR}.${FEP3_PARTICIPANT_LIBRARY_VERSION_MINOR}"
    RELEASE_POSTFIX "${FEP3_PARTICIPANT_LIBRARY_VERSION_MAJOR}.${FEP3_PARTICIPANT_LIBRARY_VERSION_MINOR}"
    RELWITHDEBINFO_POSTFIX "${FEP3_PARTICIPANT_LIBRARY_VERSION_MAJOR}.${FEP3_PARTICIPANT_LIBRARY_VERSION_MINOR}"
)

install(
    TARGETS fep3_component_registry
    EXPORT fep3_component_registry_targets
    LIBRARY NAMELINK_SKIP DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION lib
)

install(EXPORT fep3_component_registry_targets DESTINATION lib/cmake)
install(FILES fep3_component_registry-config.cmake DESTINATION .)

