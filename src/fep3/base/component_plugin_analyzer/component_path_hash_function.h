/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <fep3/base/component_plugin_analyzer/component_description_base.h>

// from https://en.cppreference.com/w/cpp/utility/hash
//   Custom specialization of std::hash can be injected in namespace std.
template <>
struct std::hash<fep3::base::ComponentPath> {
    std::size_t operator()(const fep3::base::ComponentPath& s) const noexcept
    {
        std::size_t h1 = std::hash<std::string>{}(s._absolute_path);
        std::size_t h2 = std::hash<std::string>{}(s._relative_path);
        return h1 ^ (h2 << 1);
    }
};
