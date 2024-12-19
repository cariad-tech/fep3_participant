# Copyright 2023 CARIAD SE.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.


file(GLOB_RECURSE input_directories LIST_DIRECTORIES true "${PLANTUML_INPUT_DIR}/*")
file(MAKE_DIRECTORY ${PLANTUML_OUTPUT_DIR})

foreach(input_dir ${input_directories})
    if(IS_DIRECTORY ${input_dir})
        # get output directory name from input directory name
        string(REPLACE "${PLANTUML_INPUT_DIR}" "" new_dir "${input_dir}")
        set(output_dir ${PLANTUML_OUTPUT_DIR})
        string(APPEND output_dir "${new_dir}")
        # make output directory
        file(MAKE_DIRECTORY ${output_dir})
        
        execute_process(COMMAND java -jar ${PLANTUML_PACKAGE_DIR}/lib/plantuml.jar "${input_dir}/*.puml" -o "${output_dir}" -tsvg
                        COMMAND_ECHO STDOUT
        )
    else()
        continue()
    endif()
endforeach()