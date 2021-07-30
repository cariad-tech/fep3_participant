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

#include <string>

namespace fep3
{
namespace arya
{

enum class ComponentSourceType
{
    built_in = 0,
    cpp_plugin,
    c_plugin,
    unknown
};

ComponentSourceType getComponentSourceType(const std::string& string);
std::string getString(ComponentSourceType component_origin);

}
using arya::ComponentSourceType;
using arya::getComponentSourceType;
using arya::getString;
}