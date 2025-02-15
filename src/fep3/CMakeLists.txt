#
# Copyright @ 2021 VW Group. All rights reserved.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.



###################################################
# Define the includes for building and target
#####################################################
set(BUILD_INTERFACE_INCLUDES ${PROJECT_SOURCE_DIR}/include
                             ${PROJECT_BINARY_DIR}/include
                             ${PROJECT_SOURCE_DIR}/src
                             ${PROJECT_BINARY_DIR}/include)

#this is how to use includes on the target
set(INSTALL_INTERFACE_INCLUDES include)


#######################################################
#version file generation (we want that in the project)
#######################################################

configure_file(${PROJECT_SOURCE_DIR}/include/fep3/fep3_participant_version.h.in
               ${PROJECT_BINARY_DIR}/include/fep3/fep3_participant_version.h @ONLY)
install(
    FILES ${PROJECT_BINARY_DIR}/include/fep3/fep3_participant_version.h
    DESTINATION include/fep3
)

if(NOT TARGET dev_essential)
    find_package(dev_essential 1.3.0 REQUIRED COMPONENTS pkg_rpc ddl)
else()
    include(${dev_essential_SOURCE_DIR}/scripts/cmake/stub_generation.cmake)
endif()

find_package(Boost 1.73.0 REQUIRED COMPONENTS date_time filesystem)
include(../../scripts/cmake/set_boost_interface_targets.cmake)

set (FEP3_COMMON_SOURCES ${PROJECT_SOURCE_DIR}/include/fep3/fep3_participant_export.h
                         ${PROJECT_SOURCE_DIR}/include/fep3/fep3_errors.h
                         ${PROJECT_SOURCE_DIR}/include/fep3/fep3_macros.h
                         ${PROJECT_SOURCE_DIR}/include/fep3/fep3_optional.h
                         ${PROJECT_SOURCE_DIR}/include/fep3/fep3_result_decl.h
                         ${PROJECT_SOURCE_DIR}/include/fep3/fep3_participant_types.h
                         ${PROJECT_SOURCE_DIR}/include/fep3/fep3_timestamp.h
                         ${PROJECT_SOURCE_DIR}/include/fep3/fep3_duration.h
                         ${PROJECT_BINARY_DIR}/include/fep3/fep3_participant_version.h)

set (FEP3_SOURCES ${FEP3_COMMON_SOURCES})
set (FEP3_PLUGIN_CPP_LIB_SOURCES ${FEP3_COMMON_SOURCES})
set (FEP_NATIVE_COMPONENTS_STUB_GENERATORS)
set (FEP_PARTICIPANT_STUB_GENERATORS)

source_group(common FILES ${FEP3_COMMON_SOURCES})
################################################
# now we include all sub directories' cmake.sources files
# (these append source files to the FEP3_SOURCES variable)
################################################

include(base/cmake.sources)
include(components/base/cmake.sources)
include(components/clock/cmake.sources)
include(components/configuration/cmake.sources)
include(components/job_registry/cmake.sources)
include(components/participant_info/cmake.sources)
include(components/scheduler/cmake.sources)
include(components/simulation_bus/cmake.sources)
include(components/service_bus/cmake.sources)
# native components
include(native_components/clock/cmake.sources)
include(native_components/clock_sync/cmake.sources)
# plugin
include(plugin/cpp/cmake.sources)
# participant
include(participant/cmake.sources)
# core api
include(core/cmake.sources)
# cpp api
include(cpp/cmake.sources)


###########################################
# create fep_participant and object_lib
###########################################

add_library(fep3_participant_object_lib OBJECT)
target_sources(fep3_participant_object_lib PRIVATE ${FEP3_SOURCES})
add_library(fep3_participant SHARED
    $<TARGET_OBJECTS:fep3_participant_object_lib>
)
add_dependencies(fep3_participant_object_lib ${FEP_NATIVE_COMPONENTS_STUB_GENERATORS}
                                             ${FEP_PARTICIPANT_STUB_GENERATORS})

###########################################
# configure object_library
# only neccessary for private testing
###########################################

set_target_properties(fep3_participant_object_lib PROPERTIES FOLDER "libraries")

target_include_directories(fep3_participant_object_lib PUBLIC
    "$<BUILD_INTERFACE:${BUILD_INTERFACE_INCLUDES}>"
)

target_link_libraries(fep3_participant_object_lib
    PUBLIC dev_essential::ddl
           dev_essential::pkg_rpc
    PRIVATE Boost::filesystem
            fep3_environment_variable
            fep3_default_severity_from_env_variable
)

set_target_properties(fep3_participant_object_lib PROPERTIES POSITION_INDEPENDENT_CODE ON)
if(MSVC)
    target_link_libraries(fep3_participant_object_lib PUBLIC "Iphlpapi.lib")
endif()

if (MSVC)
else ()
target_link_libraries(fep3_participant_object_lib PUBLIC
     pthread
     ${CMAKE_DL_LIBS})

endif()

target_compile_features(fep3_participant_object_lib PUBLIC cxx_std_${CMAKE_CXX_STANDARD})

target_compile_definitions(fep3_participant_object_lib PRIVATE _FEP3_PARTICIPANT_DO_EXPORT)

###########################################
# configure fep_participant
###########################################

target_link_libraries(fep3_participant PRIVATE fep3_participant_object_lib fep3_component_registry
                                       PUBLIC dev_essential::result
                                              dev_essential::memory)

if(UNIX)
    # we shall not export symbols from linked static libraries
    target_link_options(fep3_participant PRIVATE "-Wl,--exclude-libs,$<TARGET_FILE_NAME:fep3_component_registry>")
endif(UNIX)

target_compile_features(fep3_participant PUBLIC cxx_std_${CMAKE_CXX_STANDARD})
if(MSVC)
    # TODO /WD4100 should be removed when ODAUTIL-167 is fixed
    # 4251 is currently deactivated because some dll exported classes use std types within their interface (e.g. ComponentRegistry)
    target_compile_options(fep3_participant PUBLIC /wd4251 /wd4100)
endif()


set_target_properties(fep3_participant PROPERTIES FOLDER "libraries")

target_include_directories(fep3_participant PUBLIC
    "$<BUILD_INTERFACE:${BUILD_INTERFACE_INCLUDES}>"
    "$<INSTALL_INTERFACE:${INSTALL_INTERFACE_INCLUDES}>"
)


set_target_properties(fep3_participant PROPERTIES
    OUTPUT_NAME fep3_participant
    DEBUG_POSTFIX "d${FEP3_PARTICIPANT_LIBRARY_VERSION_MAJOR}.${FEP3_PARTICIPANT_LIBRARY_VERSION_MINOR}"
    RELEASE_POSTFIX "${FEP3_PARTICIPANT_LIBRARY_VERSION_MAJOR}.${FEP3_PARTICIPANT_LIBRARY_VERSION_MINOR}"
    RELWITHDEBINFO_POSTFIX "${FEP3_PARTICIPANT_LIBRARY_VERSION_MAJOR}.${FEP3_PARTICIPANT_LIBRARY_VERSION_MINOR}"
)

target_compile_definitions(fep3_participant PRIVATE _FEP3_PARTICIPANT_DO_EXPORT)

##################################################
# setup a static target for the cpp plugin creation
##################################################

# Add imported library target for the participant_cpp_plugin
add_library(fep3_participant_cpp_plugin STATIC
            ${FEP3_PLUGIN_CPP_LIB_SOURCES})

target_include_directories(fep3_participant_cpp_plugin PUBLIC
    "$<BUILD_INTERFACE:${BUILD_INTERFACE_INCLUDES}>"
    "$<INSTALL_INTERFACE:${INSTALL_INTERFACE_INCLUDES}>"
)
target_link_libraries(fep3_participant_cpp_plugin PUBLIC dev_essential::result)

set_target_properties(fep3_participant_cpp_plugin PROPERTIES
    OUTPUT_NAME fep3_participant_cpp_plugin
    DEBUG_POSTFIX "d${FEP3_PARTICIPANT_LIBRARY_VERSION_MAJOR}.${FEP3_PARTICIPANT_LIBRARY_VERSION_MINOR}"
    RELEASE_POSTFIX "${FEP3_PARTICIPANT_LIBRARY_VERSION_MAJOR}.${FEP3_PARTICIPANT_LIBRARY_VERSION_MINOR}"
    RELWITHDEBINFO_POSTFIX "${FEP3_PARTICIPANT_LIBRARY_VERSION_MAJOR}.${FEP3_PARTICIPANT_LIBRARY_VERSION_MINOR}"
)

set_target_properties(fep3_participant_cpp_plugin PROPERTIES FOLDER "libraries")

##################################################
# setup a static target for the core api library
##################################################

if(NOT TARGET clipp)
    find_package(clipp 1.2.3 REQUIRED COMPONENTS clipp)
endif()

# Add imported library target for the participant_core
add_library(fep3_participant_core STATIC
            ${FEP3_CORE_LIB_SOURCES})

target_include_directories(fep3_participant_core PUBLIC
    "$<BUILD_INTERFACE:${BUILD_INTERFACE_INCLUDES}>"
    "$<INSTALL_INTERFACE:${INSTALL_INTERFACE_INCLUDES}>"
)
target_link_libraries(fep3_participant_core PUBLIC fep3_participant clipp::clipp)
if (MSVC)
else ()
    target_link_libraries(fep3_participant_core PRIVATE pthread)
endif()
target_link_libraries(fep3_participant_core PRIVATE dev_essential::pkg_rpc)


set_target_properties(fep3_participant_core PROPERTIES
    OUTPUT_NAME fep3_participant_core
    DEBUG_POSTFIX "d${FEP3_PARTICIPANT_LIBRARY_VERSION_MAJOR}.${FEP3_PARTICIPANT_LIBRARY_VERSION_MINOR}"
    RELEASE_POSTFIX "${FEP3_PARTICIPANT_LIBRARY_VERSION_MAJOR}.${FEP3_PARTICIPANT_LIBRARY_VERSION_MINOR}"
    RELWITHDEBINFO_POSTFIX "${FEP3_PARTICIPANT_LIBRARY_VERSION_MAJOR}.${FEP3_PARTICIPANT_LIBRARY_VERSION_MINOR}"
)

set_target_properties(fep3_participant_core PROPERTIES FOLDER "libraries")

##################################################
# setup a static target for the cpp api library
##################################################

# Add imported library target for the participant_core
add_library(fep3_participant_cpp STATIC
            ${FEP3_CPP_LIB_SOURCES})

target_include_directories(fep3_participant_cpp PUBLIC
    "$<BUILD_INTERFACE:${BUILD_INTERFACE_INCLUDES}>"
    "$<INSTALL_INTERFACE:${INSTALL_INTERFACE_INCLUDES}>"
)
target_link_libraries(fep3_participant_cpp PUBLIC fep3_participant fep3_participant_core)

set_target_properties(fep3_participant_cpp PROPERTIES
    OUTPUT_NAME fep3_participant_cpp
    DEBUG_POSTFIX "d${FEP3_PARTICIPANT_LIBRARY_VERSION_MAJOR}.${FEP3_PARTICIPANT_LIBRARY_VERSION_MINOR}"
    RELEASE_POSTFIX "${FEP3_PARTICIPANT_LIBRARY_VERSION_MAJOR}.${FEP3_PARTICIPANT_LIBRARY_VERSION_MINOR}"
    RELWITHDEBINFO_POSTFIX "${FEP3_PARTICIPANT_LIBRARY_VERSION_MAJOR}.${FEP3_PARTICIPANT_LIBRARY_VERSION_MINOR}"
)

set_target_properties(fep3_participant_cpp PROPERTIES FOLDER "libraries")


###########################################
# configure the install package
###########################################

configure_file(fep3_participant-config.cmake.in ${CMAKE_BINARY_DIR}/fep3_participant-config.cmake @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/fep3_participant-config.cmake DESTINATION .)
# Create version config
include(CMakePackageConfigHelpers)
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/fep3_participant-config-version.cmake
                                 COMPATIBILITY AnyNewerVersion)
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/fep3_participant_core-config-version.cmake
                                 COMPATIBILITY AnyNewerVersion)
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/fep3_participant_cpp-config-version.cmake
                                 COMPATIBILITY AnyNewerVersion)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/fep3_participant-config-version.cmake DESTINATION .)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/fep3_participant_core-config-version.cmake DESTINATION .)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/fep3_participant_cpp-config-version.cmake DESTINATION .)
install(FILES fep3_participant-macros.cmake DESTINATION .)

install(FILES fep3_participant_core-config.cmake DESTINATION .)
install(FILES fep3_participant_cpp-config.cmake DESTINATION .)

if(fep3_participant_use_rtidds)
    # if we use rtidds plugin we have to add it in the components file
    SET(RTI_DDS_XML_TAG "<component>
        <source type=\"cpp-plugin\">
        rti/fep3_connext_dds_plugin
        </source>
        <iid>simulation_bus.arya.fep3.iid</iid>
    </component>")

endif()
# we patch the components file depending if rti dds plugin is used
configure_file(${CMAKE_CURRENT_LIST_DIR}/fep3_participant.fep_components.in fep3_participant.fep_components)

#we copy this to the bin dir because the macros files must work for the tests too
#always a components file is needed since the components are delivered as plugins
add_custom_command(TARGET fep3_participant POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/fep3_participant.fep_components "$<TARGET_FILE_DIR:fep3_participant>"
    )
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/fep3_participant.fep_components DESTINATION lib)

install(
    TARGETS fep3_participant
    EXPORT fep3_participant_targets
    LIBRARY NAMELINK_SKIP DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION lib
)
install(
    TARGETS fep3_participant_cpp_plugin
    EXPORT fep3_participant_cpp_plugin_targets
    LIBRARY NAMELINK_SKIP DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION lib
)
install(
    TARGETS fep3_participant_cpp
    EXPORT fep3_participant_cpp_targets
    LIBRARY NAMELINK_SKIP DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION lib
)
install(
    TARGETS fep3_participant_core
    EXPORT fep3_participant_core_targets
    LIBRARY NAMELINK_SKIP DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION lib
)
install(EXPORT fep3_participant_targets DESTINATION lib/cmake)
install(EXPORT fep3_participant_cpp_plugin_targets DESTINATION lib/cmake)
install(EXPORT fep3_participant_cpp_targets DESTINATION lib/cmake)
install(EXPORT fep3_participant_core_targets DESTINATION lib/cmake)
if(WIN32)
    install(FILES $<TARGET_PDB_FILE:fep3_participant> DESTINATION lib OPTIONAL)
    #install(FILES $<TARGET_PDB_FILE:fep3_participant_core> DESTINATION lib OPTIONAL)
    #install(FILES $<TARGET_PDB_FILE:fep3_participant_cpp> DESTINATION lib OPTIONAL)
endif(WIN32)

##################################################
# add subdirectories
##################################################
add_subdirectory(base/environment_variable)
add_subdirectory(base/component_registry)
add_subdirectory(base/logging)
add_subdirectory(base/thread_utilities)
