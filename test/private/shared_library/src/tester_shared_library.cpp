/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <fep3/base/component_registry/include/plugin/base/shared_library.h>
#include <fep3/fep3_filesystem.h>

#include <gtest/gtest.h>

const std::string test_shared_library = SHARED_LIBRARY;
const std::string test_shared_library_dir = SHARED_LIBRARY_DIR;
const std::string test_shared_library_full_name =
    std::string(SHARED_LIBRARY_PREFIX) + "test_shared_library" + std::string(SHARED_LIBRARY_SUFFIX);

class SharedLibraryFixtureTests : public ::testing::TestWithParam<std::string> {
protected:
    std::unique_ptr<fep3::plugin::arya::SharedLibrary> shared_library;
};

/**
 * Test loading a shared library
 * @req_id FEPSDK-1907 FEPSDK-1915
 */
TEST_F(SharedLibraryFixtureTests, testLoading)
{
    ASSERT_NO_THROW(shared_library =
                        std::make_unique<fep3::plugin::arya::SharedLibrary>(test_shared_library););
    const auto& get_function = shared_library->get<int()>("get1");
    ASSERT_NE(get_function, nullptr);
    EXPECT_EQ(1, get_function());
}

/**
 * Test loading a shared library that has dependencies on another shared library
 * @req_id FEPSDK-1907 FEPSDK-1915
 */
TEST_F(SharedLibraryFixtureTests, testLoadingSharedLibraryWithDependency)
{
    ASSERT_NO_THROW(shared_library =
                        std::make_unique<fep3::plugin::arya::SharedLibrary>(test_shared_library););
    const auto& get_function = shared_library->get<int()>("get2FromDependeeLibrary");
    ASSERT_NE(get_function, nullptr);
    EXPECT_EQ(2, get_function());
}

/**
 * Test loading a shared library that has different naming possibilities, e.g. shared_library,
 * shared_library1.0.0, shared_library.dll, <lib>shared_library
 * @req_id
 */
TEST_P(SharedLibraryFixtureTests, testLoadingSharedLibraryWithDifferentNaming)
{
    fs::path test_full_library_path{test_shared_library_dir};
    test_full_library_path.append(GetParam());

    ASSERT_NO_THROW(shared_library = std::make_unique<fep3::plugin::arya::SharedLibrary>(
                        test_full_library_path.string()););
    const auto& get_function = shared_library->get<int()>("get2FromDependeeLibrary");
    ASSERT_NE(get_function, nullptr);
    EXPECT_EQ(2, get_function());
}

INSTANTIATE_TEST_SUITE_P(testLoadingSharedLibraryWithDifferentNaming,
                         SharedLibraryFixtureTests,
                         ::testing::Values("test_shared_library",
                                           "test_shared_library1.0.0",
                                           test_shared_library_full_name));
