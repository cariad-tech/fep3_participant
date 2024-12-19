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

#include <fep3/components/base/component.h>

namespace fep3 {
namespace plugin {
namespace cpp {
namespace arya {
/**
 * @brief Wrapper for a component
 * This class itself is a @ref fep3::arya::IComponent.
 * Use this class to return a FEP Super Component from @ref
 * fep3::plugin::cpp::arya::ICPPPluginComponentFactory::createComponent. This wrapper also
 * implements a call filter, calling methods of @ref fep3::arya::IComponent on the wrapped component
 * instance only once for all wrappers sharing same the last call identifier as passed to the
 * constructor.
 */
class ComponentWrapper : public fep3::arya::IComponent {
public:
    /**
     * @brief Enumeration identifying the call to @ref fep3::arya::IComponent
     */
    enum class CallIdentifier
    {
        unknown = 0,
        createComponent,
        destroyComponent,
        initialize,
        deinitialize,
        tense,
        relax,
        start,
        stop,
        pause
    };

public:
    /** CTOR taking the wrapped @p component
     *
     * @param[in] component Shared pointer to the component to be wrapped
     * @param[in] last_call_identifier The identifier of the last call to @ref
     * fep3::arya::IComponent that occurred on any wrapper sharing same the last call identifier.
     * @throw std::runtime_error if @p component is invalid
     * @throw std::runtime_error if @p last_call_identifier is invalid
     */
    ComponentWrapper(const std::shared_ptr<fep3::arya::IComponent>& component,
                     const std::shared_ptr<CallIdentifier>& last_call_identifier);

    /** Gets the identifier of the last state transition call (createComponent/initialize etc.) to
     * the component wrapper
     *
     * @return Call identifier identifying the last call that occurred on this wrapper
     */
    CallIdentifier getLastCallIdentifier() const;

    /// @copydoc fep3::arya::IComponent::createComponent
    fep3::Result createComponent(
        const std::weak_ptr<const fep3::arya::IComponents>& components) override;
    /// @copydoc fep3::arya::IComponent::destroyComponent
    fep3::Result destroyComponent() override;
    /// @copydoc fep3::arya::IComponent::initialize
    fep3::Result initialize() override;
    /// @copydoc fep3::arya::IComponent::tense
    fep3::Result tense() override;
    /// @copydoc fep3::arya::IComponent::relax
    fep3::Result relax() override;
    /// @copydoc fep3::arya::IComponent::start
    fep3::Result start() override;
    /// @copydoc fep3::arya::IComponent::stop
    fep3::Result stop() override;
    /// @copydoc fep3::arya::IComponent::pause
    fep3::Result pause() override;
    /// @copydoc fep3::arya::IComponent::deinitialize
    fep3::Result deinitialize() override;
    /// @copydoc fep3::arya::IComponent::getInterface
    void* getInterface(const std::string& iid) override;

private:
    template <typename method_type, typename... argument_types>
    fep3::Result callFiltered(method_type&& method,
                              const CallIdentifier& call_identifier,
                              argument_types&&... arguments);

private:
    const std::shared_ptr<fep3::arya::IComponent> _component;
    const std::shared_ptr<CallIdentifier> _last_call_identifier;
};

} // namespace arya

namespace catelyn {

/**
 * @brief Wrapper for a component
 * This class itself is a @ref fep3::arya::IComponent.
 * Use this class to return a FEP Super Component from @ref
 * fep3::plugin::cpp::catelyn::IComponentFactory::createComponent that underwent a disruptive
 * interface change, e. g.
 *
 * Usage:
 * Provided the component interface "IMyComponnent" underwent a disruptive change from
 * arya::IMyComponent to bronn::IMyComponent (i. e. bronn::IMyComponent does not derive from
 * arya::IMyComponent), e. g.
 * namespace arya
 * {
 * class IMyComponent
 * {
 * public:
 *     FEP_COMPONENT_IID("my_component.arya.fep3.iid")
 *     virtual void foo() const = 0;
 * };
 * } // namespace arya
 * // disruptive change: changed return type of foo
 * namespace bronn
 * class IMyComponent
 * {
 * public:
 *     FEP_COMPONENT_IID("my_component.bronn.fep3.iid")
 *     virtual int foo() const = 0;
 * };
 * } // namespace bronn
 * , the implementation bronn::MyComponent implements only bronn::IMyComponent.
 * Anyway, in order to make a Component Factory return a FEP Super Component that supports both
 * interfaces arya::IMyComponent and bronn::IMyComponent we can create a component wrapper like so:
 * class MySuperComponent
 *     : public fep3::plugin::cpp::catelyn::ComponentWrapper<bronn::MyComponent, arya::IMyComponent>
 * {
 * public:
 *    MySuperComponent()
 *        : fep3::plugin::cpp::catelyn::ComponentWrapper<bronn::MyComponent, arya::IMyComponent>()
 *     {}
 *     // forward methods of arya::IMyComponent to the wrapped component
 *     void foo() const override
 *     {
 *         _component->foo();
 *     }
 * };
 * The Component Wrappers can even be cascaded, i. e. a wrapper can be wrapped in another wrapper in
 * order to add more interfaces in case of multiple disruptive changes.
 *
 * @tparam component_impl_type The implementation type of the wrapped component
 * @tparam component_interface_types The types of the component interfaces that this wrapper shall
 * support (additionally to the component interfaces as supported by the wrapped component)
 */
template <typename component_impl_type, typename... component_interface_types>
class ComponentWrapper : public fep3::base::Component<component_interface_types...> {
private:
    using super = fep3::base::Component<component_interface_types...>;

    template <typename... interface_types>
    struct InterfaceConcatenator {
    };

    // specialization for std::tuple
    template <typename... tuple_interface_types, typename... interface_types>
    struct InterfaceConcatenator<std::tuple<tuple_interface_types...>, interface_types...> {
        using Interfaces = std::tuple<tuple_interface_types..., interface_types...>;
    };

public:
    /// Tuple whose element types are the interfaces this component wrapper supports
    using Interfaces = typename InterfaceConcatenator<typename component_impl_type::Interfaces,
                                                      component_interface_types...>::Interfaces;

    /**
     * CTOR taking any number of arguments. The arguments are forwarded to the constructor of @p
     * component_impl_type
     *
     * @tparam argument_types The types of the arguments
     * @param args The arguments to be forwarded to the constructor of @p component_impl_type
     */
    template <typename... argument_types>
    ComponentWrapper(argument_types&&... args) : _component(args...)
    {
    }

    /// @copydoc fep3::arya::IComponent::createComponent
    fep3::Result createComponent(
        const std::weak_ptr<const fep3::arya::IComponents>& components) override
    {
        return _component.createComponent(components);
    }

    /// @copydoc fep3::arya::IComponent::destroyComponent
    fep3::Result destroyComponent() override
    {
        return _component.destroyComponent();
    }

    /// @copydoc fep3::arya::IComponent::initialize
    fep3::Result initialize() override
    {
        return _component.initialize();
    }

    /// @copydoc fep3::arya::IComponent::tense
    fep3::Result tense() override
    {
        return _component.tense();
    }

    /// @copydoc fep3::arya::IComponent::relax
    fep3::Result relax() override
    {
        return _component.relax();
    }

    /// @copydoc fep3::arya::IComponent::start
    fep3::Result start() override
    {
        return _component.start();
    }

    /// @copydoc fep3::arya::IComponent::stop
    fep3::Result stop() override
    {
        return _component.stop();
    }

    /// @copydoc fep3::arya::IComponent::pause
    fep3::Result pause() override
    {
        return _component.pause();
    }

    /// @copydoc fep3::arya::IComponent::deinitialize
    fep3::Result deinitialize() override
    {
        return _component.deinitialize();
    }

    /// @copydoc fep3::arya::IComponent::getInterface
    void* getInterface(const std::string& iid) override
    {
        // first try the component IIDs of this wrapper ...
        const auto& interface = InterfaceGetter<component_interface_types...>()(this, iid);
        if (nullptr != interface) {
            return interface;
        }
        else {
            // ... then get the interface from the wrapped component
            return static_cast<fep3::arya::IComponent*>(&_component)->getInterface(iid);
        }
    }

private:
    template <typename... interface_types>
    struct InterfaceGetter {
        // end of compile time recursion
        void* operator()(super*, const std::string&)
        {
            return {};
        }
    };

    // Specialization of above functor for more than zero interface types
    template <typename interface_type, typename... remaining_interface_types>
    struct InterfaceGetter<interface_type, remaining_interface_types...> {
        void* operator()(super* component, const std::string& iid)
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

protected:
    /// The component as wrapped by this component wrapper
    component_impl_type _component;
};

} // namespace catelyn
} // namespace cpp
} // namespace plugin
} // namespace fep3
