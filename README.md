<!---
Copyright @ 2021 VW Group. All rights reserved.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
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

The FEP SDK provides googlemock classes mocking the FEP Component interfaces. These classes are useful for unit tests written with the googletest
framework if mocked FEP Components shall be used instead of real FEP Components. In order to add the include path containing the mock headers
just link your test target against the *fep3_components_test* target:

    target_link_libraries(my_test PRIVATE fep3_components_test)

## How to build using only cmake ###
### Prerequisites
- Download [CMake](https://cmake.org/) at least in version 3.17.0
- Using [git](https://git-scm.com/), clone the repository and checkout the desired branch (e.g. `master`)
- Build the dev_essential library as described in the [Readme](https://devstack.vwgroup.com/bitbucket/projects/OPENDEV/repos/dev_essential/browse/README.md) file.
- Boost in version 1.73, can be compiled from [sources](https://boostorg.jfrog.io/artifactory/main/release/1.73.0/source/) or the built binaries can be directly downloaded. For Windows [Boost 1.73](https://sourceforge.net/projects/boost/files/boost-binaries/1.73.0/) can be downloaded, for Linux/Debian distributions apt can be used.
- Using git, clone the [clipp](https://github.com/muellan/clipp/) repository, a command line interface for modern C++. Afterwards, build and install it using CMake.


### Optional
- <a id="howtodds"></a> Download [DDS](https://www.rti.com/free-trial) version 6.1.0.
- <a id="howtogtest"></a> [Gtest](https://github.com/google/googletest) version 1.10 or newer.
  - Gtest has to be compiled from sources. After checking out the gtest github repository, run the following commands inside the checked out folder (depending on your compiler or the configuration to be built, the cmake command should be adapted accordingly). After executing the commands, <gtest_install_dir> will contain the built libraries. *gtest_force_shared_crt* flag is needed for windows in order compile with the correct Windows Runtime Library and avoid linking errors later when building the *fep participant* library.
```shell
$ mkdir build
$ cd build
$ cmake -G "Visual Studio 16 2019" -A x64 -T v142 -DCMAKE_INSTALL_PREFIX=<gtest_install_dir> -Dgtest_force_shared_crt=ON  ../
$ cmake --build . --target install --config Release
```
- <a id="howtodoxygen"></a> [Doxygen](https://www.doxygen.nl/index.html) version 1.8.14. Doxygen executable should be located under <doxygen_dir>/bin
- <a id="howtosphinx"></a> [Sphinx](https://pypi.org/project/Sphinx/) version 3.3.0.

### Build with cmake
Run the following command, (adaptations may be needed in case a different Visual Studio version is used or different configuration should be built).
```shell
$ cmake -H<source_dir> -B<build_dir> -G "Visual Studio 16 2019" -A x64 -T v142 \
    -Dfep3_participant_cmake_enable_documentation=OFF \
    -DCMAKE_INSTALL_PREFIX=<install_dir> \
    -DCMAKE_PREFIX_PATH=<clipp_root> \
    -Ddev_essential_DIR=<dev_essential_root>/lib/cmake/dev_essential \
    -DBoost_DIR=<boost_root>/lib/cmake/Boost-<boost_version>/ \
    -DBUILD_SHARED_LIBS=ON \
    -DCMAKE_FIND_PACKAGE_PREFER_CONFIG=ON
$ cmake --build . --target install --config Release
```
- \<source_dir\>: The path where the *fep participant* library is cloned to and the main CMakeLists.txt is located.
- \<build_dir\>: The build directory
- \<install_dir\>: Path where the built artifacts will be installed.
- \<dev_essential_root\>: The path were the *dev_essential* library was installed. File *dev_essential-config.cmake* is located under \<dev_essential_root\>/lib/cmake/dev_essential.
- \<boost_root\>: The installation path of boost. File *BoostConfig.cmake* is located under \<boost_root\>/lib/cmake/Boost-\<boost_version\> (e.g. Boost-1.73.0).
- \<clipp_root\>: The installation path of clipp. File _clipp.h_ is located under \<clipp_root\>/include.

>  **Note**: The above cmake calls are exemplary for using Windows and Visual Studio as generator. For gcc the addition of -DCMAKE_POSITION_INDEPENDENT_CODE=True is needed. Also depending on the generator used, the *--config* in the build step could be ignored and the adaptation of CMAKE_CONFIGURATION_TYPES or CMAKE_BUILD_TYPE could be necessary for building in other configurations.

Depending on your CMake version and the configuration of third-party packages like Gtest, boost, etc. it might be necessary to use *-DCMAKE_FIND_PACKAGE_PREFER_CONFIG=ON* as well.
Depending on your boost installation, it might be nessecary to set the following variables as well:
- *fep3_participant_cmake_boost_diagnostic_definitions=ON|OFF*
- *fep3_participant_cmake_boost_disable_autolinking=ON|OFF*
- *fep3_participant_cmake_boost_dynamic_linking=ON|OFF*

See: https://cmake.org/cmake/help/latest/module/FindBoost.html#imported-targets

### Additional CMake options
#### Enable tests
CMake variables
- *fep3_participant_cmake_enable_tests=ON|OFF*
- *fep3_participant_cmake_enable_private_tests=ON|OFF* and
- *fep3_participant_cmake_enable_functional_tests=ON|OFF*

control the activation of the tests, private and functional tests. These flags are set by default to *OFF*. For activating either of these flags, [gtest](#howtogtest) is required.
Apart from the above flags, the *GTest_DIR* cmake variable should be set to the path where _GTestConfig.cmake_ is located. Assuming the [gtest](#howtogtest) was followed, this path is _\<gtest_install_dir\>/lib/cmake/GTest_.

A call to cmake with these flags could look like:
```shell
$ cmake.exe -H<source_dir> -B<build_dir> -G "Visual Studio 16 2019" -A x64 -T V142
    -Dfep3_participant_cmake_enable_documentation=OFF \
    -DCMAKE_INSTALL_PREFIX=<install_dir> \
    -DCMAKE_PREFIX_PATH=<clipp_root> \
    -Ddev_essential_DIR=<dev_essential_root>/lib/cmake/dev_essential \
    -DBoost_DIR=<boost_root>/lib/cmake/Boost-<boost_version>/ \
    -DGTest_DIR=<gtest_install_dir>/lib/cmake/GTest \
    -Dfep3_participant_cmake_enable_tests=ON \
    -Dfep3_participant_cmake_enable_private_tests=ON \
    -Dfep3_participant_cmake_enable_functional_tests=ON \
    -Dfep3_participant_export_test_targets=ON \
    -DBUILD_SHARED_LIBS=ON \
    -DCMAKE_FIND_PACKAGE_PREFER_CONFIG=ON
$ cmake --build . --target install --config Release
```

#### Enable the documentation
CMake variable
- *fep3_participant_cmake_enable_documentation=ON|OFF*

activates the build of the documentation. Default value is *ON*. For this flag [doxygen](#howtodoxygen) and [sphinx](#howtosphinx) are required.
The doxygen executable should be located in *\<doxygen_root\>/bin/doxygen.exe*.
Also, the dot tool from [Graphviz](https://www.graphviz.org/) is required. The dot executable should be located in *\<graphviz_root\>/bin/dot.exe*

A call to cmake with documentation activated could look like:
```shell
$ cmake.exe -H<source_dir> -B<build_dir> -G "Visual Studio 16 2019" -A x64 -T V142
    -Dfep3_participant_cmake_enable_documentation=ON \
    -DDoxygen_ROOT=<doxygen_root> \
    -DCMAKE_PREFIX_PATH="<clipp_root>;<graphviz_root>" \
    -DCMAKE_INSTALL_PREFIX=<install_dir> \
    -Ddev_essential_DIR=<dev_essential_root>/lib/cmake/dev_essential \
    -DBoost_DIR=<boost_root>/lib/cmake/Boost-<boost_version>/ \
    -DBUILD_SHARED_LIBS=ON \
    -DCMAKE_FIND_PACKAGE_PREFER_CONFIG=ON
$ cmake --build . --target install --config Release
```

#### Enable DDS
CMake variable
- *FEP3_USE_RTIDDS=ON|OFF*

activates the build of *fep participant* library with DDS support. For this flag the [dds](#howtodds) library is required. For activating this option the following cmake variables have to be set:
- *CONNEXTDDS_ARCH*: The hardware architecture for dds, for example *x64Win64VS2017*.
- *CONNEXTDDS_DIR*: Path where the library is located.
- *CMAKE_MODULE_PATH*: Path to directory where the \"FindRTIConnextDDS.cmake\" file lies.

A call to CMake with dds activated could look like:
```shell
$ cmake.exe -H<source_dir> -B<build_dir> -G "Visual Studio 16 2019" -A x64 -T V142 \
    -Dfep3_participant_cmake_enable_documentation=OFF \
    -DCMAKE_INSTALL_PREFIX=<install_dir> \
    -DCMAKE_PREFIX_PATH=<clipp_root> \
    -Ddev_essential_DIR=<dev_essential_root>/lib/cmake/dev_essential \
    -DBoost_DIR=<boost_root>/lib/cmake/Boost-<boost_version>/ \
    -DBUILD_SHARED_LIBS=ON \
    -DFEP3_USE_RTIDDS=ON \
    -DCONNEXTDDS_DIR=<dds_root> \
    -DCMAKE_MODULE_PATH=<dds_root> \
    -DCONNEXTDDS_ARCH=x64Win64VS2017 \
    -DCMAKE_FIND_PACKAGE_PREFER_CONFIG=ON
$ cmake --build . --target install --config Release
```
- \<dds_root\>: Path to root dir of RTI DDS installation

### Tested compilers
- Windows 10 x64 with Visual Studio C++ 2019 and v142 Toolset.
- Linux Ubuntu 18.04 LTS x64 with GCC 7.5 and libstdc++14 (C++14 ABI)

## Portation guide ###
### Classes

| Deprecated                                              | Actual                                  |
|---------------------------------------------------------| ------------------------------------    |
| `fep3::core::arya::ElementBase`                         | `fep3::core::ElementBase`               |
| `fep3::arya::ElementFactory`                            | `fep3::core::ElementFactory`            |
| `fep3::core::arya::ParticipantStateChanger`             | `fep3::core::ParticipantStateChanger`   |
| `fep3::cpp::arya::DataJob`                              | `fep3::cpp::DataJob`                    |
| `fep3::cpp::arya::DataJobElement`                       | `fep3::cpp::DataJobElement`             |
| `fep3::arya::IElementFactory`                           | `fep3::base::IElementFactory`           |
| `fep3::arya::ElementFactory`                            | `fep3::base::ElementFactory`            |
| `fep3::IElement`,`fep3::arya::IElement`                 | `fep3::base::IElement`                  |
| `fep3::arya::Participant`                               | `fep3::base::Participant`               |
| `fep3::arya::CommandLineParser`                         | `fep3::core::CommandLineParser`         |
| `fep3::arya::CommandLineParserFactory `                 | `fep3::core::CommandLineParserFactory`  |
| `fep3::arya::ParserDefaultValues `                      | `fep3::core::ParserDefaultValues`       |
| `fep3::core::arya::ParticipantExecutor `                | `fep3::core::ParticipantExecutor`       |

### Functions
| Deprecated                                                                                                                        | Actual                                                                                                |
|-----------------------------------------------------------------------------------------------------------------------------------|-------------------------------------------------------------------------------------------------------|
| `fep3::cpp::arya::createParticipant`                                                                                              | `fep3::cpp::createParticipant`                                                                        |
| `fep3::arya::createParticipant`, `fep3::createParticipant`                                                                        | `fep3::base::createParticipant`                                                                       |
| `fep3::core::arya::DataReader& operator>> (fep3::core::arya::DataReader&, T&)`                                                    | `fep3::core::arya::DataReader& operator>> (fep3::core::arya::DataReader&, fep3::arya::Optional<T>&)`  |
| `fep3::core::arya::DataReader& operator>>(fep3::core::arya::DataReader&, fep3::base::arya::StreamType&)`                          | `fep3::core::arya::DataReader::readType`                                                              |
| `fep3::core::arya::DataReader& operator>>(fep3::core::arya::DataReader&, fep3::data_read_ptr<const fep3::arya::IDataSample>&)`    | `fep3::core::arya::DataReader::popSampleOldest`                                                       |
| `fep3::core::arya::DataReader& operator>> (fep3::core::arya::DataReader&, fep3::data_read_ptr<const fep3::arya::IStreamType>&)`   | `fep3::core::arya::DataReader::readType`                                                              |
