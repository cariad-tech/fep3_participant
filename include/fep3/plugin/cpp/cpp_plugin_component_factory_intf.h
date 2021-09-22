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
#include <fep3/components/base/component_intf.h>

namespace fep3
{
namespace plugin
{
namespace cpp
{
namespace arya
{

/**
 * The ICPPPluginComponentFactory interface must be provided by a cpp-plugin.
 */
class ICPPPluginComponentFactory
{
public:
    /// DTOR
    virtual ~ICPPPluginComponentFactory() = default;

    /**
     * @brief creates and returns one instance of the given \p component_iid (component interface identifier).
     * @param[in] component_iid component interface identifier
     * @return std::unique_ptr<IComponent> valid component instance for the component_iid (component interface identifier).
     * @retval std::unique_ptr<IComponent>() empty if not created
     */
    virtual std::unique_ptr<fep3::arya::IComponent> createComponent(const std::string& component_iid) const = 0;
};

} // namespace arya
using arya::ICPPPluginComponentFactory;
} // namespace cpp
} // namespace plugin
} // namespace fep3





