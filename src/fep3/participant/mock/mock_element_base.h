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


#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <fep3/fep3_result_decl.h>
#include <fep3/core/element_base.h>
#include <helper/gmock_destruction_helper.h>

namespace fep3
{
namespace mock
{

class MockElementBase
    : public fep3::core::ElementBase
    , public test::helper::Dieable
{
public:
    MockElementBase()
        : ElementBase("test_element", "0.0.1")
    {}

    // mocked non-final methods of ElementBase
    MOCK_METHOD0(initialize, Result());
    MOCK_METHOD0(deinitialize, void());
    MOCK_METHOD0(run, Result());
    MOCK_METHOD0(stop, void());
};

} // namespace mock
} // namespace fep3