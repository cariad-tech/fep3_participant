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

#pragma once

#include <fep3/core/element_base.h>

#include <helper/gmock_destruction_helper.h>

namespace fep3 {
namespace mock {

class MockElementBase : public fep3::core::ElementBase, public test::helper::Dieable {
public:
    MockElementBase() : fep3::core::ElementBase("test_element", "0.0.1")
    {
    }

    // mocked non-final methods of ElementBase
    MOCK_METHOD0(load, Result());
    MOCK_METHOD0(unload, void());
    MOCK_METHOD0(initialize, Result());
    MOCK_METHOD0(deinitialize, void());
    MOCK_METHOD0(run, Result());
    MOCK_METHOD0(stop, void());
};

} // namespace mock
} // namespace fep3