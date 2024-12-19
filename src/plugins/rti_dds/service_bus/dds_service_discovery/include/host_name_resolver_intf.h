/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>

namespace fep3 {
namespace native {

class IHostNameResolver {
public:
    virtual ~IHostNameResolver() = default;
    virtual std::string findIp(const std::string& address, const std::string& port) = 0;

    IHostNameResolver() = default;
    IHostNameResolver(IHostNameResolver&&) = default;
    IHostNameResolver& operator=(IHostNameResolver&&) = default;
    IHostNameResolver& operator=(const IHostNameResolver&) = default;
};

} // namespace native
} // namespace fep3
