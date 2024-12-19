/**
 * @file
 * @copyright
 * @verbatim
Copyright 2023 CARIAD SE.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
@endverbatim
 */

#pragma once

#include <fep3/components/base/component_intf.h>

namespace fep3 {
namespace plugin {
namespace cpp {
namespace arya {

/**
 * The ICPPPluginComponentFactory interface must be provided by a cpp-plugin.
 */
class ICPPPluginComponentFactory {
public:
    /// DTOR
    virtual ~ICPPPluginComponentFactory() = default;

    /**
     * @brief creates and returns one instance of the given @p component_iid (component interface
     * identifier).
     * @param[in] component_iid component interface identifier
     * @return std::unique_ptr<IComponent> valid component instance for the component_iid (component
     * interface identifier).
     * @retval std::unique_ptr<IComponent>() empty if not created
     */
    virtual std::unique_ptr<fep3::arya::IComponent> createComponent(
        const std::string& component_iid) const = 0;
};

} // namespace arya
namespace catelyn {

/**
 * The IComponentFactory interface must be provided by a cpp-plugin.
 */
class IComponentFactory {
public:
    /// DTOR
    virtual ~IComponentFactory() = default;

    /**
     * @brief creates and returns one instance of the given @p component_iid (component interface
     * identifier).
     * @param[in] component_iid component interface identifier
     * @return std::shared_ptr<IComponent> valid component instance for the component_iid (component
     * interface identifier).
     * @retval std::shared_ptr<IComponent>() empty if not created
     */
    virtual std::shared_ptr<fep3::arya::IComponent> createComponent(
        const std::string& component_iid) = 0;
};

} // namespace catelyn
} // namespace cpp
} // namespace plugin
} // namespace fep3
