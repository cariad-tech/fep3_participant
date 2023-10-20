if(NOT TARGET dev_essential)
    find_package(dev_essential REQUIRED COMPONENTS strings)
endif()

add_library(fep3_lssdp STATIC
${CMAKE_CURRENT_SOURCE_DIR}/src/lssdpcpp/lssdpcpp.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/lssdpcpp/lssdpcpp.h
${CMAKE_CURRENT_SOURCE_DIR}/src/url/cxx_url.h
${CMAKE_CURRENT_SOURCE_DIR}/src/url/cxx_url.cpp
)
target_include_directories(fep3_lssdp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/lssdpcpp  ${CMAKE_CURRENT_SOURCE_DIR}/src/url)
target_link_libraries(fep3_lssdp PRIVATE dev_essential::strings)
set_target_properties(fep3_lssdp PROPERTIES FOLDER "libraries/3rdparty")
set_target_properties(fep3_lssdp PROPERTIES POSITION_INDEPENDENT_CODE ON)