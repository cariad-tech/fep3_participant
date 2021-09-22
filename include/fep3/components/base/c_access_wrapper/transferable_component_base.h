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

#include <fep3/components/base/component.h>
#include <fep3/components/base/c_intf/component_c_intf.h>
#include <fep3/plugin/c/destruction_manager.h>
#include <fep3/plugin/c/shared_binary_manager.h>
#include "component_getter.h"
#include <fep3/components/base/c_access_wrapper/components_c_access.h>

namespace fep3
{
namespace plugin
{
namespace c
{
namespace arya
{

/**
 * @brief Decorator class to make a component transferable over a C interface.
 *
 * @tparam interface_type The type of the interface to be transferred
 */
template<typename interface_type>
class TransferableComponentBase
    : public ::fep3::base::arya::Component<interface_type>
    , public arya::SharedBinaryManager // enable lifetime management of binary
    , public arya::DestructionManager // enable destruction of remote objects (i. e. objects on other side of binary boundaries)
{
public:
    /**
     * Default CTOR
     */
    inline TransferableComponentBase() = default;
    /**
     * CTOR taking deductors for the type deduction of the component access types
     *
     * @param[in] component_getter The component getter providing access to other components
     */
    inline TransferableComponentBase(const std::shared_ptr<::fep3::plugin::c::arya::IComponentGetter>& component_getter)
        : _component_getter(component_getter)
    {}
    /// Deleted move CTOR
    inline TransferableComponentBase(TransferableComponentBase&&) = delete;
    /// Deleted copy CTOR
    inline TransferableComponentBase(const TransferableComponentBase&) = delete;
    /**
     * Deleted move assignment
     * @return this TransferableComponentBase
     */
    inline TransferableComponentBase& operator=(TransferableComponentBase&&) = delete;
    /** Deleted copy assignment
     * @return this TransferableComponentBase
     */
    inline TransferableComponentBase& operator=(const TransferableComponentBase&) = delete;
    /**
     * Default DTOR
     */
    inline virtual ~TransferableComponentBase() = default;

    // methods overriding ::fep3::arya::ComponentBase
    /**
     * Creates a component and sets up access to remote components
     *
     * @param[in] components Weak pointer to all components to setup access for
     * @return FEP Result
     */
    inline ::fep3::Result createComponent(const std::weak_ptr<const fep3::arya::IComponents>& components) override
    {
        // note: non-lockable weak_ptr is not an error, there are just no components to access
        if(const auto& shared_components = components.lock())
        {
            // need downcast because the IComponents interface has no getComponentGetter
            // and this method shall not be added because it is specific to the C plugin system
            if(auto remote_components = dynamic_cast<const fep3::plugin::c::access::arya::Components*>
                (shared_components.get()))
            {
                if(const auto& component_getter = remote_components->getComponentGetter())
                {
                    if(const auto& inner_component_getter = component_getter.get())
                    {
                        *inner_component_getter = _component_getter;
                    }
                }
            }
        }
        return ::fep3::base::arya::ComponentImpl::createComponent(components);
    }

private:
    std::shared_ptr<::fep3::plugin::c::arya::IComponentGetter> _component_getter;
};

} // namespace arya
using arya::TransferableComponentBase;
} // namespace c
} // namespace plugin
} // namespace fep3
