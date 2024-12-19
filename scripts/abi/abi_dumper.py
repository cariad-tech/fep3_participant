import os
from pathlib import Path

def create_dump(recipe):
    abi_dumper_path = os.path.join(
        recipe.deps_cpp_info["abi_dumper"].rootpath,
        recipe.deps_user_info["abi_dumper"].command_path)
    assert os.path.isfile(abi_dumper_path), "abi dumper executable not found"

    plugin_lib = 'libfep_components_plugin.so'
    plugin_lib_path = os.path.join(recipe.deps_cpp_info["fep_sdk_participant"].rootpath, "lib", plugin_lib)
    assert os.path.isfile(plugin_lib_path), "libfep_components_plugin.so not found in dependency"

    #write the header file
    header_file_path =  os.path.join(recipe.source_folder, recipe.headers_file_name)
    file = Path(header_file_path)
    output_header_file_name = "abi_check_headers.txt"
    output_header_file_path =  os.path.join(recipe.build_folder, output_header_file_name)
    output_header_file = open(output_header_file_path,"w+")
    part_package_path = recipe.deps_cpp_info["fep_sdk_participant"].rootpath
    output_header_file.write(file.read_text().replace('%PARTICIPANT_PACKAGE%', part_package_path))
    output_header_file.close()
    # setup the path where the abi will be dumped
    dump_output_folder= os.path.join(recipe.build_folder, recipe.dump_folder)
    if not os.path.exists(dump_output_folder):
        os.makedirs(dump_output_folder)

    plugin_dump_file_path = os.path.join(dump_output_folder, recipe.plugin_dump_file)
    plugin_dump_version = str(recipe.version).replace(".", "_")
    dump_command = f"{abi_dumper_path} {plugin_lib_path} -public-headers {output_header_file_path} -o {plugin_dump_file_path} -lver {plugin_dump_version}"

    dump_return_code = recipe.run(
        f"{dump_command}",
        ignore_errors=True)
    
    return dump_return_code, plugin_dump_file_path