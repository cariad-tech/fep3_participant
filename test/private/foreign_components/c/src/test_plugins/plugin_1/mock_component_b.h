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

#include <memory>

#include <gmock/gmock.h>

#include "component_b_intf.h"
#include "component_b_c_intf.h"
#include <fep3/components/base/c_access_wrapper/transferable_component_base.h>
#include <helper/gmock_destruction_helper.h>

namespace test_plugin_1
{
namespace mock
{

class MockComponentB
    : public ::fep3::plugin::c::TransferableComponentBase<IComponentB>
    , public test::helper::Dieable
{
public:
    MockComponentB()
    {}

    MOCK_CONST_METHOD0(get, int32_t());
};

} // namespace mock
} // namespace test_plugin_1
