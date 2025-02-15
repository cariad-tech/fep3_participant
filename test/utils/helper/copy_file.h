/**
 * @file
 * @copyright
 * @verbatim
Copyright @ 2021 VW Group. All rights reserved.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
@endverbatim
 */

#include <a_util/filesystem.h>

#include <fstream>

namespace test {
namespace helper {

/**
 * @brief Copies a file from @p source to @p destination
 * @param[in] source The source file path
 * @param[in] destination The destination file path
 * @return @c true if successful, @c false otherwise
 */
bool copyFile(const a_util::filesystem::Path& source, const a_util::filesystem::Path& destination)
{
    if (a_util::filesystem::exists(source)) {
        std::ifstream in(source);
        std::ofstream out(destination);
        out << in.rdbuf();
        return true;
    }
    return false;
}

} // namespace helper
} // namespace test