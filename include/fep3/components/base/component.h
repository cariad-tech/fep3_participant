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

#include "component_iid.h"
#include "components_intf.h"

#include <memory>

namespace fep3 {
namespace base {
namespace arya {

/**
 * @brief default helper implementation for component
 *
 */
class ComponentImpl : public fep3::arya::IComponent {
protected:
    /// CTOR
    ComponentImpl() = default;
    /// DTOR
    ~ComponentImpl() override = default;

public:
    /// @copydoc fep3::IComponent::createComponent
    fep3::Result createComponent(const std::weak_ptr<const fep3::arya::IComponents>& components) override
    {
        _components = components;
        return create();
    }
    /// @copydoc fep3::IComponent::destroyComponent
    fep3::Result destroyComponent() override
    {
        auto res = destroy();
        _components.reset();
        return res;
    }
    fep3::Result initialize() override
    {
        return Result();
    }
    fep3::Result tense() override
    {
        return Result();
    }
    fep3::Result relax() override
    {
        return Result();
    }
    Result deinitialize() override
    {
        return Result();
    }
    fep3::Result start() override
    {
        return Result();
    }
    fep3::Result stop() override
    {
        return Result();
    }
    fep3::Result pause() override
    {
        return Result();
    }

protected:
    /**
     * @brief create the base component.
     * if this create method is called the _components pointer is valid already
     *
     * @return Result
     */
    virtual fep3::Result create()
    {
        return Result();
    }
    /**
     * @brief destroy the base component.
     * if this destroy method is called the _components pointer is still valid
     *
     * @return Result
     */
    virtual fep3::Result destroy()
    {
        return Result();
    }

    /**
     * @brief Weak pointer to the components
     * @note The component (this) must not take permanent ownership of the components
     */
    std::weak_ptr<const fep3::arya::IComponents> _components;
};
/**
 * @brief default helper implementation for component
 *
 */
template <typename... component_interface_types>
class Component : public arya::ComponentImpl, public component_interface_types... {
protected:
    /// CTOR
    Component() : ComponentImpl()
    {
    }

    /// DTOR
    ~Component() override = default;

protected:
    /// @copydoc fep3::arya::IComponent::getInterface
    void* getInterface(const std::string& iid) override
    {
        return InterfaceGetter<component_interface_types...>()(this, iid);
    }

private:
    /**
     * Functor getting a pointer to an interface by IID
     */
    template <typename... interface_types>
    struct InterfaceGetter {
        // end of compile time recursion
        void* operator()(Component*, const std::string&)
        {
            return {};
        }
    };
    /**
     * Specialization of above functor for more than zero interface types
     */
    template <typename interface_type, typename... remaining_interface_types>
    struct InterfaceGetter<interface_type, remaining_interface_types...> {
        void* operator()(Component* component, const std::string& iid)
        {
            if (interface_type::FEP3_COMP_IID == iid) {
                return static_cast<interface_type*>(component);
            }
            else {
                // compile time recursion: go on with remaining component access object types
                return InterfaceGetter<remaining_interface_types...>()(component, iid);
            }
        }
    };
};
} // namespace arya

/**
 * @brief extracting \ref fep3::base::arya::Component from version namespace
 *
 */
using arya::Component;
} // namespace base
} // namespace fep3
