/**
 * @file
 * @copyright
 * @verbatim
Copyright @ 2021 VW Group. All rights reserved.

    This Source Code Form is subject to the terms of the Mozilla
    Public License, v. 2.0. If a copy of the MPL was not distributed
    with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

If it is not possible or desirable to put the notice in a particular file, then
You may include the notice in a location (such as a LICENSE file in a
relevant directory) where a recipient would be likely to look for such a notice.

You may add additional accurate notices of copyright ownership.

@endverbatim
 */


#include <memory>

#include <gtest/gtest.h>

#include <fep3/plugin/base/shared_library.h>

const std::string test_shared_library = SHARED_LIBRARY;

/**
 * Test loading a shared library
 * @req_id FEPSDK-1907 FEPSDK-1915
*/
TEST(SharedLibraryTester, testLoading)
{
    std::unique_ptr<fep3::plugin::arya::SharedLibrary> shared_library;
    ASSERT_NO_THROW
    (
        shared_library = std::make_unique<fep3::plugin::arya::SharedLibrary>(test_shared_library);
    );
    const auto& get_function = shared_library->get<int()>("get1");
    ASSERT_NE(get_function, nullptr);
    EXPECT_EQ(1, get_function());
}

/**
 * Test loading a shared library that has dependencies on another shared library
 * @req_id FEPSDK-1907 FEPSDK-1915
*/
TEST(SharedLibraryTester, testLoadingSharedLibraryWithDependency)
{
    std::unique_ptr<fep3::plugin::arya::SharedLibrary> shared_library;
    ASSERT_NO_THROW
    (
        shared_library = std::make_unique<fep3::plugin::arya::SharedLibrary>(test_shared_library);
    );
    const auto& get_function = shared_library->get<int()>("get2FromDependeeLibrary");
    ASSERT_NE(get_function, nullptr);
    EXPECT_EQ(2, get_function());
}