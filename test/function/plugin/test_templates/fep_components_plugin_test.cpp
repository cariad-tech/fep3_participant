/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "fep_components_plugin_test.h"

#include <fep3/fep3_filesystem.h>
#include <fep3/fep3_participant_version.h>

namespace test {
namespace helper {

void FEPComponentsPluginFixture::SetUp()
{
    fs::path plugin_file_path{GetParam()};
    const auto& plugin_file_path_string = plugin_file_path.string();
#ifdef WIN32
    // remember the cwd
    const auto& original_working_dir = fs::current_path();
    // on windows we need to switch to the directory where the library is located
    // to ensure loading of dependee dlls that reside in the same directory
    fs::current_path(plugin_file_path.parent_path());

    _library_handle = ::LoadLibrary(plugin_file_path_string.c_str());
    if (!_library_handle) {
        throw std::runtime_error("failed to load shared library '" + plugin_file_path.string() +
                                 "' with error code '" + std::to_string(GetLastError()) + "'");
    }

    // switch back to the original cwd
    auto ec = std::error_code{};
    if (fs::current_path(original_working_dir, ec); ec) {
        throw std::runtime_error("unable to switch back to original working directory; error: '" +
                                 std::to_string(ec.value()) + " - " + ec.message() +
                                 "'; current working directory might be wrong from now on.");
    }

#else
    _library_handle = ::dlopen(plugin_file_path_string.c_str(), RTLD_LAZY);
    ASSERT_NE(nullptr, _library_handle) << "failed to load shared library '" +
                                               plugin_file_path_string + "' with error '" +
                                               dlerror() + "'";
#endif
}

void FEPComponentsPluginFixture::TearDown()
{
    if (nullptr != _library_handle) {
#ifdef WIN32
        ::FreeLibrary(_library_handle);
#else
        ::dlclose(_library_handle);
#endif
    }
}

/**
 * Tests general exported functions of fep_components_plugin:
 * * fep3_plugin_base_ParticipantLibraryVersion
 * * fep3_plugin_getPluginVersion
 * * fep3_plugin_cpp_isDebugPlugin
 * * fep3_plugin_getVersionNamespace
 *
 * General note:
 * All function names and types appearing in the plugin interface, are hard coded here rather than
 * taken from the FEP Participant Library exports, because we test the interfaces that are
 * potentially already released. This way we ensure, that if a potentially released interface is
 * changed, the test fails. For example, the (forbidden) change of the name of an existing exported
 * symbol (e. g. "fep3_plugin_getParticipantLibraryVersion") will be detected by this test (while it
 * wouldn't be detected if the test would take the symbol name from the FEP Participant Library
 * exports).
 */
TEST_P(FEPComponentsPluginFixture, generalFunctions)
{
    // test "fep3_plugin_getParticipantLibraryVersion"
    typedef struct {
        /// The version identifier of the participant library
        const char* _id;
        /// The major version integer
        int32_t _major;
        /// The minor version integer
        int32_t _minor;
        /// The patch version integer
        int32_t _patch;
        /// The build version integer
        int32_t _build;
    } fep3_plugin_base_ParticipantLibraryVersion;
    auto get_participant_library_version_function =
        get<fep3_plugin_base_ParticipantLibraryVersion()>(
            "fep3_plugin_getParticipantLibraryVersion");
    ASSERT_NE(nullptr, get_participant_library_version_function);
    const auto participant_library_version = get_participant_library_version_function();
    EXPECT_STREQ(FEP3_PARTICIPANT_LIBRARY_VERSION_ID, participant_library_version._id);
    EXPECT_EQ(FEP3_PARTICIPANT_LIBRARY_VERSION_MAJOR, participant_library_version._major);
    EXPECT_EQ(FEP3_PARTICIPANT_LIBRARY_VERSION_MINOR, participant_library_version._minor);
    EXPECT_EQ(FEP3_PARTICIPANT_LIBRARY_VERSION_PATCH, participant_library_version._patch);
    EXPECT_EQ(FEP3_PARTICIPANT_LIBRARY_VERSION_BUILD +
                  0 // + 0 because macro value is not set in developer versions
              ,
              participant_library_version._build);

    // test "fep3_plugin_getPluginVersion"
    auto get_plugin_version_function =
        get<void(void(void*, const char*), void*)>("fep3_plugin_getPluginVersion");
    ASSERT_NE(nullptr, get_plugin_version_function);
    std::string plugin_version("foo");
    get_plugin_version_function(
        [](void* destination, const char* version) {
            *static_cast<decltype(plugin_version)*>(destination) = version;
        },
        static_cast<void*>(&plugin_version));
    EXPECT_EQ(FEP3_PARTICIPANT_LIBRARY_VERSION_STR, plugin_version);

#ifdef WIN32
    auto is_debug_plugin_function = get<bool()>("fep3_plugin_cpp_isDebugPlugin");
    ASSERT_NE(nullptr, is_debug_plugin_function);
    #ifdef _DEBUG
    EXPECT_TRUE(is_debug_plugin_function());
    #else
    EXPECT_FALSE(is_debug_plugin_function());
    #endif
#endif

    // test "fep3_plugin_getVersionNamespace"
    // Note: The FEP Components Plugin has to be backwards compatible to FEP Participants
    //       built against old versions of FEP Participant Library. The latter checks for
    //       fep3_plugin_getVersionNamespace returning exactly "arya".
    auto get_version_namespace_function = get<const char*()>("fep3_plugin_getVersionNamespace");
    ASSERT_NE(nullptr, get_version_namespace_function);
    std::string version_namespace = get_version_namespace_function();
    EXPECT_EQ("arya", version_namespace);
}

} // namespace helper
} // namespace test
