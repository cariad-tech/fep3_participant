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
#else
#include <dlfcn.h>
#endif

#include <locale> 
#include <codecvt>
#include "binary_info.h"

namespace fep3
{
namespace binary_info
{

a_util::filesystem::Path getFilePath()
{
    a_util::filesystem::Path current_binary_file_path;
#ifdef WIN32
    HMODULE hModule = nullptr;
    if(GetModuleHandleEx
        (GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
        , (LPCTSTR)getFilePath
        , &hModule
        ))
    {
        std::wstring file_path_buffer;
        DWORD number_of_copied_characters = 0;
        // note: to support paths with length > MAX_PATH we have do trial-and-error
        // because GetModuleFileName does not indicate if the path was truncated
        while(number_of_copied_characters >= file_path_buffer.size())
        {
            file_path_buffer.resize(file_path_buffer.size() + MAX_PATH);
            number_of_copied_characters = GetModuleFileNameW(hModule, &file_path_buffer[0], static_cast<DWORD>(file_path_buffer.size()));
        }
        file_path_buffer.resize(number_of_copied_characters);

        auto size_needed = WideCharToMultiByte(CP_UTF8, 0, &file_path_buffer[0], (int)file_path_buffer.size(), NULL, 0, NULL, NULL);
        std::string str_utf8(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, &file_path_buffer[0], (int)file_path_buffer.size(), &str_utf8[0], size_needed, NULL, NULL);
        current_binary_file_path = str_utf8;
    }
#else   // WIN32
    Dl_info dl_info;
    dladdr(reinterpret_cast<void*>(getFilePath), &dl_info);
    current_binary_file_path = dl_info.dli_fname;
#endif
    return current_binary_file_path.getParent();
}

} // namespace binary_info
} // namespace fep3