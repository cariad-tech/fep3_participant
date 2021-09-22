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

#include <stdint.h>

#include "class_a_intf.h"

namespace test_plugin_1
{

class ClassA : public IClassA
{
public:
    ClassA() = default;
    ~ClassA() = default;

    int32_t get() const override;
    void set(int32_t value) override;

private:
    int32_t _value;
};

} // namespace test_plugin_1
