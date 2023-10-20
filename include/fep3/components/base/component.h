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

#include <fep3/components/base/components_intf.h>

namespace fep3 {
namespace base {
namespace arya {

/**
 * @brief default helper implementation for component
 */
class ComponentImpl : public fep3::arya::IComponent {
protected:
    /// CTOR
    ComponentImpl() = default;
    /**
     * @brief Deleted Copy CTOR
     */
    ComponentImpl(const ComponentImpl&) = delete;

    /**
     * @brief Deleted Move CTOR
     */
    ComponentImpl(ComponentImpl&&) = delete;

    /**
     * @brief Deleted Copy assignment operator
     *
     * @return ComponentImpl&
     */
    ComponentImpl& operator=(const ComponentImpl&) = delete;

    /**
     * @brief Deleted Move assignment operator
     *
     * @return ComponentImpl&
     */
    ComponentImpl& operator=(ComponentImpl&&) = delete;

    /**
     * @brief Default DTOR
     */

public:
    ~ComponentImpl() override = default;

    /// @copydoc fep3::IComponent::createComponent
    fep3::Result createComponent(
        const std::weak_ptr<const fep3::arya::IComponents>& components) override
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
 */
template <typename... component_interface_types>
class Component : public arya::ComponentImpl, virtual public component_interface_types... {
protected:
    /// CTOR
    Component() = default;

    /**
     * @brief Deleted Copy CTOR
     */
    Component(const Component&) = delete;

    /**
     * @brief Deleted Move CTOR
     */
    Component(Component&&) = delete;

    /**
     * @brief Deleted Copy assignment operator
     *
     * @return Component&
     */
    Component& operator=(const Component&) = delete;

    /**
     * @brief Deleted Move assignment operator
     *
     * @return Component&
     */
    Component& operator=(Component&&) = delete;

public:
    /// DTOR
    ~Component() override = default;

    /// Tuple whose element types are the interfaces this component supports
    using Interfaces = std::tuple<component_interface_types...>;

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
 * @brief extracting @ref fep3::base::catelyn::Component from version namespace
 */
using arya::Component;
} // namespace base
} // namespace fep3
