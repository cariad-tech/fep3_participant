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

#include <fep3/plugin/c/c_access/c_access_helper.h>
#include <fep3/components/base/c_intf/component_c_intf.h>
#include <fep3/components/base/components_intf.h>
#include <fep3/components/base/c_access_wrapper/component_getter.h>

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
 * @brief Access class for @ref fep3::arya::IComponents
 */
class Components
    : public fep3::arya::IComponents
    , private fep3::plugin::c::arya::DestructionManager
    , private fep3::plugin::c::access::arya::Helper
{
public:
    /// Type of access structure
    using Access = fep3_arya_SComponents;

    /**
     * CTOR
     * @param[in] access The C access structure for the remote object
     * @param[in] destructors List of destructors to be called upon destruction of this
     * @param[in] handle_to_component_getter_function_getters The handle to the component getter function getters to be set
     */
    inline Components
        (const Access& access
        , std::deque<std::unique_ptr<c::arya::IDestructor>> destructors
        , fep3_plugin_c_arya_HComponentGetterFunctionGetters handle_to_component_getter_function_getters
        );
    /// Deleted move CTOR
    inline Components(Components&&) = delete;
    /// Deleted copy CTOR
    inline Components(const Components&) = delete;
    /** Deleted move assignment
     * @return this Components
     */
    inline Components& operator=(Components&&) = delete;
    /** Deleted copy assignment
     * @return this Components
     */
    inline Components& operator=(const Components&) = delete;
    /// Default DTOR
    inline ~Components() override = default;

    /**
     * @brief Gets the component_getter of this
     *
     * @return Shared pointer to the component getter
     */
    inline std::shared_ptr<std::shared_ptr<fep3::plugin::c::arya::IComponentGetter>> getComponentGetter() const noexcept;

    // methods implementing ::fep3::arya::IComponents
    inline fep3::arya::IComponent* findComponent(const std::string& fep_iid) const override;

private:
    Access _access;
    // shared_ptr in shared_ptr to be able to return a reference to the content via const method getComponentGetter and modify the content
    std::shared_ptr<std::shared_ptr<fep3::plugin::c::arya::IComponentGetter>> _component_getter;
    fep3_plugin_c_arya_HComponentGetterFunctionGetters _handle_to_component_getter_function_getters;
};

Components::Components
    (const Access& access
    , std::deque<std::unique_ptr<c::arya::IDestructor>> destructors
    , fep3_plugin_c_arya_HComponentGetterFunctionGetters handle_to_component_getter_function_getters
    )
    : _access(access)
    , _component_getter(std::make_shared<std::shared_ptr<fep3::plugin::c::arya::IComponentGetter>>())
    , _handle_to_component_getter_function_getters(handle_to_component_getter_function_getters)
{
    addDestructors(std::move(destructors));
}

std::shared_ptr<std::shared_ptr<fep3::plugin::c::arya::IComponentGetter>> Components::getComponentGetter() const noexcept
{
    return _component_getter;
}

fep3::arya::IComponent* Components::findComponent(const std::string& fep_iid) const
{
    auto component_interface_access = arya::Helper::callWithResultParameter
        (_access._handle
        , _access.findComponent
        , _handle_to_component_getter_function_getters
        , fep_iid.c_str()
        );
    if(_component_getter)
    {
        const auto& inner_component_getter = *_component_getter.get();
        if(inner_component_getter)
        {
            return inner_component_getter->operator()
                (component_interface_access.getComponent
                    , fep_iid
                    , component_interface_access._handle
                );
        }
    }

    return nullptr;
}

} // namespace arya
} // namespace access
} // namespace c
} // namespace plugin
} // namespace fep3
