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

#include <fep3/components/base/component_intf.h>

// Note: fep3::plugin::c::wrapper::arya::Components has to be declared friend of IComponents
// because the component wrapper has to call access to IComponents::findComponent.
namespace fep3 {
namespace arya {

/**
 * @brief the components composition will manage and hold the instances of all components.
 * The registered components can be obtained by its interface identifier (see @ref
 * FEP3_COMPONENT_IID)
 */
class IComponents {
public:
    /**
     * @brief Get the component pointer of the registered component that is registered
     * with the component interface id of @p interface_type (see @ref FEP3_COMPONENT_IID)
     *
     * @tparam interface_type the component interface to retrieve.
     * @retval valid pointer to an instance of the given type @p interface_type
     * @retval nullptr if not found
     */
    template <class interface_type>
    interface_type* getComponent() const
    {
        const std::string fep_iid = getComponentIID<interface_type>();
        arya::IComponent* component = findComponent(fep_iid);
        if (component) {
            return static_cast<interface_type*>(component->getInterface(fep_iid.c_str()));
        }
        else {
            return nullptr;
        }
    }

protected:
    /// DTOR
    ~IComponents() = default;

private:
    /// @endcond nodoc
    /**
     * @brief Get the component pointer of the registered component that is registered
     * with the component interface id of @p fep_iid
     *
     * @param[in] fep_iid the component interface identifier to retrieve.
     * @retval IComponent* valid pointer to a component which is registered for the given @p fep_iid
     * @retval nullptr if not found
     */
    virtual arya::IComponent* findComponent(const std::string& fep_iid) const = 0;
};

/**
 * @brief helper function to retrieve a component from the module's components registry
 *
 * @tparam INTERFACE the interface of the component
 * @param[in] components the components container
 * @return INTERFACE*
 * @retval nullptr the interface was not found
 */
template <class interface_type>
interface_type* getComponent(const arya::IComponents& components)
{
    return components.getComponent<interface_type>();
}

} // namespace arya

using arya::IComponents;

/**
 * @brief extracting @ref fep3::arya::getComponent() from version namespace
 * @tparam interface_type the component interface looking for
 * @param[in] components the reference to the components
 * @return interface_type* the valid interface pointer
 * @retval nullptr the interface was not found
 */
template <class interface_type>
interface_type* getComponent(const arya::IComponents& components)
{
    return arya::getComponent<interface_type>(components);
}

} // namespace fep3
