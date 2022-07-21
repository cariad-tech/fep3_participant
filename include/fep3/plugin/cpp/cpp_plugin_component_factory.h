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

#include <fep3/components/base/component_iid.h>
#include <fep3/components/base/component_intf.h>

#include "cpp_plugin_component_factory_intf.h"

namespace fep3
{
namespace plugin
{
namespace cpp
{
namespace arya
{

/**
 * @brief CPP plugin Component factory helper template to create one instance of the implementation \p component_impl_type.
 *
 * @tparam component_impl_type the implementation type
 */
template<class component_impl_type>
class CPPPluginComponentFactory : public arya::ICPPPluginComponentFactory
{
private:
    std::unique_ptr<fep3::arya::IComponent> createComponent(const std::string& component_iid) const
    {
        if (component_iid == getComponentIID<component_impl_type>())
        {
            return std::unique_ptr<fep3::arya::IComponent>(new component_impl_type());
        }
        else
        {
            return {};
        }
    }
};

} // namespace arya
using arya::CPPPluginComponentFactory;
} // namespace cpp
} // namespace plugin
} // namespace fep3

