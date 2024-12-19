/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifdef WIN32
    #include <Windows.h>
#endif // WIN32

#include <gmock/gmock.h>

#ifndef WIN32
    #include <dlfcn.h>
    #include <string.h>
#endif // !WIN32

namespace test {
namespace helper {

/**
 * The test fixture for testing the FEP Components Plugin
 */
class FEPComponentsPluginFixture : public ::testing::TestWithParam<std::string> {
public:
    void SetUp() override;
    void TearDown() override;

    template <typename T>
    T* get(const std::string& symbol_name) const
    {
#ifdef WIN32
        return reinterpret_cast<T*>(::GetProcAddress(_library_handle, symbol_name.c_str()));
#else
        return reinterpret_cast<T*>(::dlsym(_library_handle, symbol_name.c_str()));
#endif
    }

private:
#ifdef WIN32
    HMODULE _library_handle;
#else
    void* _library_handle;
#endif
};

} // namespace helper
} // namespace test
