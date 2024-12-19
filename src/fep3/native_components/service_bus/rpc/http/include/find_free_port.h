/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

namespace fep3 {
namespace helper {

int findFreeSocketPort(int begin_port, int count = 1000);

} // namespace helper
} // namespace fep3
