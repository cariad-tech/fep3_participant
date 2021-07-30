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
// @attention Changes in this file must be reflected in the corresponding C++ interface file component_base.h

#pragma once

#include <memory>

#include <fep3/c_access_wrapper/fep3_result_c_access_wrapper.h>
#include <fep3/components/base/component_intf.h>
#include <fep3/components/base/c_intf/component_c_intf.h>
#include <fep3/plugin/c/c_intf/c_intf_errors.h>
#include <fep3/plugin/c/c_access/c_access_helper.h>
#include <fep3/plugin/c/c_wrapper/destructor_c_wrapper.h>
#include <fep3/components/base/component.h>
#include "components_c_wrapper.h"
#include "transferable_component_base.h"
#include <fep3/components/base/c_access_wrapper/component_getter_function_getter_intf.h>

namespace fep3
{
namespace plugin
{
namespace c
{
namespace access
{
namespace arya
{

/**
 * @brief Access class for @ref fep3::arya::IComponent
 * Use this class to access a component that resides in another binary (e. g. a shared library).
 * @note IComponent is expected to be a base class for a "specific interface".
 *
 * @tparam interface_type The type of the specific component interface, that is derived from IComponent
 */
template<typename interface_type>
class ComponentBase
    : public ::fep3::plugin::c::arya::TransferableComponentBase<interface_type>
    , protected ::fep3::plugin::c::access::arya::Helper
{
private:
    using super = ::fep3::plugin::c::arya::TransferableComponentBase<interface_type>;

protected:
    /// Type of the access helper
    using Helper = fep3::plugin::c::access::arya::Helper;

public:
    /// Type of access structure
    using Access = fep3_arya_SIComponent;

    /**
     * CTOR
     * @param[in] access The C access structure for the remote object
     * @param[in] shared_binary Shared pointer to the shared binary this resides in
     */
    inline ComponentBase
        (const Access& access
        , const std::shared_ptr<c::arya::ISharedBinary>& shared_binary
        );
    /// DTOR
    virtual ~ComponentBase() override = default;

    /**
     * @brief Sets component getter function getters to this
     * @param[in] component_getter_function_getters The component getter function getters to be set
     */
    virtual inline void setComponentGetterFunctionGetter
        (const std::shared_ptr<fep3::plugin::c::arya::IComponentGetterFunctionGetter>& component_getter_function_getters);

    // methods implementing ::fep3::base::arya::Component
    /// @copydoc ::fep3::base::arya::ComponentImpl::initialize
    inline fep3::Result initialize() override;
    /// @copydoc ::fep3::base::Component::tense
    inline fep3::Result tense() override;
    /// @copydoc ::fep3::base::Component::relax
    inline fep3::Result relax() override;
    /// @copydoc ::fep3::base::Component::start
    inline fep3::Result start() override;
    /// @copydoc ::fep3::base::Component::stop
    inline fep3::Result stop() override;
    /// @copydoc ::fep3::base::Component::deinitialize
    inline fep3::Result deinitialize() override;
    /// @copydoc ::fep3::base::Component::getInterface
    using super::getInterface; // the getInterface method of ::fep3::base::arya::Component is just suitable
    /// @copydoc ::fep3::base::Component::create
    inline fep3::Result create() override;
    /// @copydoc ::fep3::base::Component::destroy
    inline fep3::Result destroy() override;

    // methods implementing ::fep3::arya::IComponent
    /**
     * @brief Calls createComponent on the remote object and stores the passed @p components
     *
     * @param[in] components Weak pointer to the components
     * @return fep3::Result
     */
    inline fep3::Result createComponent(const std::weak_ptr<const fep3::arya::IComponents>& components) override;

private:
    // helper method to simplify calls to access helper
    inline fep3::Result redirectCallToHandle
        (fep3_plugin_c_InterfaceError (*function)(fep3_arya_HIComponent, fep3_result_callback_type result_callback, void* result_destination)) const;

private:
    Access _access;
    std::deque
        <std::pair<std::weak_ptr<const ::fep3::arya::IComponents>
        , ::fep3::plugin::c::access::arya::Destructor<fep3_plugin_c_arya_SDestructionManager>>
        > _remote_component_destructors;
    std::shared_ptr<fep3::plugin::c::arya::IComponentGetterFunctionGetter> _component_getter_function_getters;
};

template<typename interface_type>
ComponentBase<interface_type>::ComponentBase
    (const Access& access
    , const std::shared_ptr<c::arya::ISharedBinary>& shared_binary
    )
    : ::fep3::plugin::c::arya::TransferableComponentBase<interface_type>()
    , _access(access)
{
    super::addDestructor(std::make_unique<arya::Destructor<fep3_plugin_c_arya_SDestructionManager>>(access._destruction_manager));
    super::setSharedBinary(shared_binary);
}

template<typename interface_type>
fep3::Result ComponentBase<interface_type>::create()
{
    return {};
}

template<typename interface_type>
fep3::Result ComponentBase<interface_type>::destroy()
{
    return redirectCallToHandle(_access.destroyComponent);
}

template<typename interface_type>
fep3::Result ComponentBase<interface_type>::initialize()
{
    return redirectCallToHandle(_access.initialize);
}

template<typename interface_type>
fep3::Result ComponentBase<interface_type>::tense()
{
    return redirectCallToHandle(_access.tense);
}

template<typename interface_type>
fep3::Result ComponentBase<interface_type>::relax()
{
    return redirectCallToHandle(_access.relax);
}

template<typename interface_type>
fep3::Result ComponentBase<interface_type>::start()
{
    return redirectCallToHandle(_access.start);
}

template<typename interface_type>
fep3::Result ComponentBase<interface_type>::stop()
{
    return redirectCallToHandle(_access.stop);
}

template<typename interface_type>
fep3::Result ComponentBase<interface_type>::deinitialize()
{
    return redirectCallToHandle(_access.deinitialize);
}

template<typename interface_type>
fep3::Result ComponentBase<interface_type>::createComponent(const std::weak_ptr<const fep3::arya::IComponents>& components)
{
    FEP3_RETURN_IF_FAILED(plugin::c::arya::TransferableComponentBase<interface_type>::createComponent(components))

    // create a new reference (shared pointer) to the component getter function getters to be accessed by the plugin
    const auto& shared_pointer_to_component_getter_function_getters
        = new decltype(_component_getter_function_getters)(_component_getter_function_getters);
    // ... this reference must be destroyed when the components object in the plugin is destroyed
    // , so we create a destruction mananger ...
    auto component_getter_function_getters_destruction_manager = new c::arya::DestructionManager;
    // ... and add a destructor
    component_getter_function_getters_destruction_manager->addDestructor
        (std::make_unique<c::arya::OtherDestructor<typename std::remove_pointer<typename std::remove_reference<decltype(shared_pointer_to_component_getter_function_getters)>::type>::type>>
        (shared_pointer_to_component_getter_function_getters));
    // ... the access to the destruction manager is passed to the plugin (see below)
    auto component_getter_function_getters_destruction_manager_access = fep3_plugin_c_arya_SDestructionManager
        {reinterpret_cast<fep3_plugin_c_arya_HDestructionManager>(static_cast<c::arya::DestructionManager*>(component_getter_function_getters_destruction_manager))
        , wrapper::arya::Destructor::destroy
        };

    return transferWeakPtrWithResultCallback<fep3::Result>
        (components
        , _remote_component_destructors
        , _access._handle
        , _access.createComponent
        , &getResult
        , [](const auto& pointer_to_components)
            {
                return plugin::c::wrapper::arya::Components::AccessCreator()
                    (pointer_to_components
                    );
            }
        , reinterpret_cast<fep3_plugin_c_arya_HComponentGetterFunctionGetters>(shared_pointer_to_component_getter_function_getters)
        , component_getter_function_getters_destruction_manager_access
        );
}

template<typename interface_type>
void ComponentBase<interface_type>::setComponentGetterFunctionGetter
    (const std::shared_ptr<fep3::plugin::c::arya::IComponentGetterFunctionGetter>& component_getter_function_getters)
{
    _component_getter_function_getters = component_getter_function_getters;
}

template<typename interface_type>
fep3::Result ComponentBase<interface_type>::redirectCallToHandle
    (fep3_plugin_c_InterfaceError (*function)(fep3_arya_HIComponent, fep3_result_callback_type result_callback, void* result_destination)) const
{
    return callWithResultCallback<fep3::Result>
        (_access._handle
        , function
        , &getResult
        );
}

} // namespace arya
using arya::ComponentBase;
} // namespace access
} // namespace c
} // namespace plugin
} // namespace fep3
