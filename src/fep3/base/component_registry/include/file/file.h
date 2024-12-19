/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <fep3/fep3_filesystem.h>

#include <vector>

namespace fep3 {
// Namespace providing facilites to find a file
namespace file {

fs::path find(const fs::path& file_path, const std::vector<fs::path>& hints);

} // namespace file
} // namespace fep3
