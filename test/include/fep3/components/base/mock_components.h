/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <fep3/components/base/components_intf.h>

#include <gmock/gmock.h>

///@cond nodoc

namespace fep3 {
namespace mock {
namespace arya {

class Components : public fep3::arya::IComponents {
public:
    MOCK_METHOD(fep3::arya::IComponent*, findComponent, (const std::string&), (const, override));
};

} // namespace arya
using arya::Components;
} // namespace mock
} // namespace fep3

///@endcond nodoc