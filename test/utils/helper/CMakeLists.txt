
find_package(dev_essential 1.3.0 REQUIRED COMPONENTS pkg_rpc)
add_library(fep_test_helper INTERFACE)
target_include_directories(fep_test_helper INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(fep_test_helper INTERFACE dev_essential::pkg_rpc)
