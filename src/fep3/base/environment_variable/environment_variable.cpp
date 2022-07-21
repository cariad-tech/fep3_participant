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


#ifdef WIN32
#include <Windows.h>
#include <vector>
#else
#include <stdlib.h>
#endif

#include <locale>
#include <codecvt>

#include <fep3/fep3_errors.h>

#include "environment_variable.h"

namespace fep3
{
namespace environment_variable
{

Optional<std::string> get(const std::string& name)
{
#ifdef WIN32
    std::wstring wide_string_name(name.cbegin(), name.cend());
    const DWORD length = GetEnvironmentVariableW(wide_string_name.c_str(), nullptr, 0);
    if (0 < length)
    {
        std::wstring value_buffer(length, '\0');
        GetEnvironmentVariableW(wide_string_name.c_str(), &value_buffer[0], static_cast<DWORD>(value_buffer.size()));

        auto size_needed = WideCharToMultiByte(CP_UTF8, 0, &value_buffer[0], (int)value_buffer.size(), NULL, 0, NULL, NULL);
        std::string str_utf8(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, &value_buffer[0], (int)value_buffer.size(), &str_utf8[0], size_needed, NULL, NULL);
        return str_utf8;
    }
#else //WIN32
    char* const value = std::getenv(name.c_str());
    if(nullptr != value)
    {
        return value;
    }
#endif
    return {};
}

Result set(const std::string& name, const std::string& value)
{
#ifdef WIN32
    if (FALSE == ::SetEnvironmentVariable(name.c_str(), value.c_str()))
    {
        RETURN_ERROR_DESCRIPTION(ERR_FAILED, "failed to set environment variable %s", name.c_str());
    }
#else //WIN32
    if (setenv(name.c_str(), value.c_str(), 1) != 0)
    {
        RETURN_ERROR_DESCRIPTION(ERR_FAILED, "failed to set environment variable %s", name.c_str());
    }
#endif
    return {};
}

} // namespace environment_variable
} // namespace fep3