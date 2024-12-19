add_library(convenient_component_registry STATIC 
    convenient_component_registry.cpp
    convenient_component_registry.h
    stub_component_factory.h)

target_include_directories(convenient_component_registry PUBLIC .)
target_link_libraries(convenient_component_registry 
    PUBLIC
    fep3_participant_private_lib
    fep3_component_registry
    PRIVATE
    fep3_components_test)
set_target_properties(convenient_component_registry PROPERTIES FOLDER "test/libraries")
