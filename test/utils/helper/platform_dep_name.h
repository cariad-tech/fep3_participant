/**
 * @copyright
 * @verbatim
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * @endverbatim
 */
#include <string>

/**
 * Create a platform (tester)-dependant name for stand-alone use.
 * @param [in] strOrigName  The original Module name
 * @return The modified name.
 */
const std::string makePlatformDepName(const std::string& original_name);
