<!---
Copyright @ 2021 VW Group. All rights reserved.
 
     This Source Code Form is subject to the terms of the Mozilla
     Public License, v. 2.0. A copy of the MPL licence can be found in doc/license/MPL2.0.txt
 
If it is not possible or desirable to put the notice in a particular file, then
You may include the notice in a location (such as a LICENSE file in a
relevant directory) where a recipient would be likely to look for such a notice.
 
You may add additional accurate notices of copyright ownership.
-->

FEP SDK - Participant library
=======================

## Description ##

This installed package contains the FEP Participant library.

* FEP Participant library
  * Middleware Abstraction for distributed simulation systems
  * dev_essential platform abstraction library
  * dev_essential::ddl library (data description language with codec API)
  * dev_essential::pkg_rpc library with JSON-RPC code generator
* FEP Participant library Examples
* FEP Participant library Documentation (see fep3_participant/doc/fep3-participant.html)

## How to use ###

The FEP SDK provides a CMake >= 3.18 configuration. Here's how to use it from your own CMake projects:

To build against the fep participant library only:

    find_package(fep3_participant REQUIRED)

After this instruction, you can create CMake executable targets linking against the FEP Participant library using the following command:

    add_executable(my_participant_target source_file1.cpp source_file2.cpp)

You need to append the *fep3_participant* target to your existing targets to add the dependency:

    target_link_libraries(my_participant_target PRIVATE fep3_participant)
    fep3_participant_install(my_participant_target)

The convenience macro *fep3_participant_install* will help to install all DLL/SO in the given install subdirectory.
The convenience macro *fep3_participant_deploy* will help to add DLL/SO dependencies to the build target directory of *my_participant_target*.

To build against the fep participant core library with convenience class to build your own elements as Tool integration, use the following:

    find_package(fep3_participant_core REQUIRED)
    add_executable(my_tool_element my_tool_element.cpp)
    target_link_libraries(my_tool_element PRIVATE fep3_participant_core)
    fep3_participant_install(my_tool_element destination_path)

To build against the fep participant cpp library with convenience class to build your own FEP CPP Interfaces use the following:

    find_package(fep3_participant_cpp REQUIRED)
    add_executable(my_cpp_element my_new_element.cpp)
    target_link_libraries(my_cpp_element PRIVATE fep3_participant_cpp)
    fep3_participant_install(my_cpp_element destination_path)

## How to build using only cmake ###
### Prerequisites
- Download [CMake](https://cmake.org/) at least in version 3.17.0
- Using [git](https://git-scm.com/), clone the repository and checkout the desired branch (e.g. `master`)
- Build the dev_essential library as described in the [Readme](https://www.github.com/cariad-tech/dev_essentials) file.
- Boost in version 1.73, can be compiled from [sources](https://boostorg.jfrog.io/artifactory/main/release/1.73.0/source/) or the built binaries can be directly downloaded. For Windows [Boost 1.73](https://sourceforge.net/projects/boost/files/boost-binaries/1.73.0/boost_1_73_0-msvc-14.1-64.exe/download) can be downloaded, for Linux/Debian distributions apt can be used.


### Optional
- <a id="howtodds"></a> Download [DDS](https://www.rti.com/free-trial) version 6.1.0.

-   <a id="howtogtest"></a> [Gtest](https://github.com/google/googletest) version 1.10 or newer.
    - Gtest has to be compiled from sources. After checking out the gtest github repository, run the following commands inside the checked out folder (depending on your compiler or the configuration to be built, the cmake command should be adapted accordingly). After executing the commands, <gtest_install_dir> will contain the built libraries. *gtest_force_shared_crt* flag is needed for windows in order compile with the correct Windows Runtime Library and avoid linking errors later when building the *fep participant* library.

    ```shell
     > mkdir build
     > cd build
     > cmake -G "Visual Studio 16 2019" -A x64 -T v142
     -DCMAKE_INSTALL_PREFIX=<gtest_install_dir> -Dgtest_force_shared_crt=ON  ../
     > cmake --build . --target install --config Release
    ```
- <a id="howtodoxygen"></a> [Doxygen](https://www.doxygen.nl/index.html) version 1.8.14. Doxygen executable should be located under <doxygen_dir>/bin
- <a id="howtosphinx"></a> [Sphinx](https://pypi.org/project/Sphinx/) version 3.3.0.
### Build with cmake
- Run the following command, (adaptations may be needed in case a different Visual Studio version is used or different configuration should be built).
    ```shell
    > cmake.exe -H<root_dir> -B<build_dir> -G "Visual Studio 16 2019" -A x64 -T v142 -DCMAKE_INSTALL_PREFIX=<install_dir> -Ddev_essential_DIR=<dev_essential_dir>/lib/cmake/dev_essential -DBoost_INCLUDE_DIR=<boost_install_dir> -Dfep3_participant_cmake_enable_documentation=OFF -DBUILD_SHARED_LIBS="ON"
    > cmake --build . --target install --config Release
    ```
    - <root_dir> The path where the  *fep participant* library is checked out and the main CMakeLists.txt is located.
    - <build_dir> The build directory
    - <install_dir> Path where the built artifacts will be installed.
    - <dev_essential_dir> The path were the *dev_essential* library was installed. File *dev_essential-config.cmake* is located under <dev_essential_dir>\lib\cmake\dev_essential.
    - <boost_install_dir> The installation path of boost, *version.hpp* is located under <boost_install_dir>/boost
    >  **Note**: The above cmake calls are exemplary for using Windows and Visual Studio as generator. For gcc the addition of -DCMAKE_POSITION_INDEPENDENT_CODE=True is needed. Also depending on the generator used, the *--config* in the build step could be ignored and the adaptation of CMAKE_CONFIGURATION_TYPES or CMAKE_BUILD_TYPE could be necessary for building in other configurations.
### Additional Cmake options

- Enable tests
    - **fep3_participant_cmake_enable_tests**, **fep3_participant_cmake_enable_private_tests** and **fep3_participant_cmake_enable_functional_tests** variables control the activation of the tests, private and functional tests. These flags are set by default to False. For activating either of these flags, [gtest](#howtogtest) is required.
    - Apart from the above flags, the **GTest_DIR** cmake variable should be set to the path where *GTestConfig.cmake* is located. Assuming the [gtest](#howtogtest) was followed, this path is *<gtest_install_dir>\lib\cmake\GTest*.
    - A call to cmake with these flags could look like:
         ```shell
        > cmake.exe -H<root_dir> -B<build_dir> -G "Visual Studio 16 2019" -A x64 -T v142 -DCMAKE_INSTALL_PREFIX=<install_dir> -Ddev_essential_DIR=<dev_essential_dir>/lib/cmake/dev_essential -DBoost_INCLUDE_DIR=<boost_install_dir> -Dfep3_participant_cmake_enable_documentation=OFF -DGTest_DIR=<gtest_install_dir>\lib\cmake\GTest -Dfep3_participant_cmake_enable_tests=True -Dfep3_participant_cmake_enable_functional_tests=True -DBUILD_SHARED_LIBS="ON"
        > cmake --build . --target install --config Release
        ```
- Enable the documentation
    - The **fep3_participant_cmake_enable_documentation** variable activates the build of the documentation. Default value is True. For this flag [doxygen](#howtodoxygen) and [sphinx](#howtosphinx) are required. The doxygen executable should be located in *${DOXYGEN_ROOT}/bin/doxygen.exe* and the cmake variable *DOXYGEN_ROOT* should be set accordingly.
    - A call to cmake with documentation activated could look like:
        ```shell
        > cmake.exe -H<root_dir> -B<build_dir> -G "Visual Studio 16 2019" -A x64 -T v142 -DCMAKE_INSTALL_PREFIX=<install_dir> -Ddev_essential_DIR=<dev_essential_dir>/lib/cmake/dev_essential -DBoost_INCLUDE_DIR=<boost_install_dir> -DDOXYGEN_ROOT=<doxyxen_dir> -Dfep3_participant_cmake_enable_documentation=True -DBUILD_SHARED_LIBS="ON"
        > cmake --build . --target install --config Release
        ```
- Enable DDS
    - The **FEP3_USE_RTIDDS** variable activates the building of *fep participant* library with DDS support. For this flag the [dds](#howtodds) library is required. For activating this option the following cmake variables have to be set:
     - **CONNEXTDDS_DIR** Path where the library is located.
     - **CMAKE_MODULE_PATH** Path where the "FindRTIConnextDDS.cmake" file lies.
     - **CONAN_BIN_DIRS_RTI_CONNEXT_DDS** Path where the dlls are located.
     - **CONNEXTDDS_ARCH** The hardware architecture for dds, for example *x64Win64VS2017*
     - A call to cmake with dds activated could look like:
        ```shell
        > cmake.exe -H<root_dir> -B<build_dir> -G "Visual Studio 16 2019" -A x64 -T v142 -DCMAKE_INSTALL_PREFIX=<install_dir> -Ddev_essential_DIR=<dev_essential_dir>/lib/cmake/dev_essential -DBoost_INCLUDE_DIR=<boost_install_dir> -Dfep3_participant_cmake_enable_documentation=OFF -DFEP3_USE_RTIDDS=True  -DCONNEXTDDS_DIR=<dds_path> -DCMAKE_MODULE_PATH=<dds_path>  -DCONNEXTDDS_ARCH="x64Win64VS2017" -DBUILD_SHARED_LIBS="ON" -DCONAN_BIN_DIRS_RTI_CONNEXT_DDS=<dds_path>\bin
        > cmake --build . --target install --config Release
        ```
### Tested compilers
- Windows 10 x64 with Visual Studio C++ 2019 and v142 Toolset.
- Linux Ubuntu 18.04 LTS x64 with GCC 7.5 and libstdc++14 (C++14 ABI)
