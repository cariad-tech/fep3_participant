/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "../../include/file/file.h"

namespace fep3 {
namespace file {

fs::path find(const fs::path& file_path, const std::vector<fs::path>& hints)
{
    // if path is given absolute, there is no need to evaluate the hints
    if (file_path.is_absolute()) {
        if (fs::exists(file_path)) {
            return file_path;
        }
    }
    else {
        for (const auto& hint: hints) {
            auto search_file_path = hint / file_path;
            if (fs::exists(search_file_path)) {
                return search_file_path;
            }
        }
    }
    return {};
}

} // namespace file
} // namespace fep3
