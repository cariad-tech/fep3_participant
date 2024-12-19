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

#include <fep3/plugin/cpp/cpp_plugin_component_factory_intf.h>
#include <fep3/plugin/cpp/cpp_plugin_component_wrapper.h>

#include <array>
#include <functional>
#include <stdexcept>

namespace fep3 {
namespace plugin {
namespace cpp {

/**
 * Class for creating an array of IIDs from @p interface_types
 *
 * @tparam interface_types The types of the interfaces to create the array from
 */
template <typename... interface_types>
struct IIDArray {
};
///@cond nodoc

/**
 * Specialization for std::tuple
 */
template <typename... interface_types>
struct IIDArray<std::tuple<interface_types...>> {
    /// The array of IIDs
    static constexpr const std::array<const char* const, sizeof...(interface_types)> _iids = {
        interface_types::FEP3_COMP_IID...};
};
///@endcond nodoc

/**
 * Class describing a FEP Component by types
 * If no @p component_interface_types are given the @ref Interfaces are set to all
 * supported interfaces of @p component_impl_type.
 *
 * @tparam component_impl_type The type of the FEP Component implementation
 * @tparam component_interface_types The types of the FEP Component interfaces
 *                                   to set @ref Interfaces to.
 */
template <typename component_impl_type, typename... component_interface_types>
class ComponentDescriptor final {
private:
    template <typename impl_type, typename... interface_types>
    struct InterfacesGenerator {
        using Interfaces = std::tuple<interface_types...>;
    };
    template <typename impl_type>
    struct InterfacesGenerator<impl_type> {
        using Interfaces = typename impl_type::Interfaces;
    };

public:
    /// default CTOR
    ComponentDescriptor() : _factory([]() { return std::make_shared<component_impl_type>(); })
    {
    }
    /**
     * CTOR taking a @p factory, which is a callable that can create a component
     *
     * @tparam factory_type The type of the factory
     * @param[in] factory The factory
     */
    template <typename factory_type>
    ComponentDescriptor(factory_type&& factory) : _factory(std::forward<factory_type>(factory))
    {
    }

    /// Tuple whose element types are the interfaces the component supports
    using Interfaces =
        typename InterfacesGenerator<component_impl_type, component_interface_types...>::Interfaces;
    /// The type of the factory function for instantiating the component
    using FactoryFunction = std::function<std::shared_ptr<fep3::arya::IComponent>()>;
    /**
     * @brief Gets the factory
     * @return The factory
     */
    FactoryFunction getFactory() const
    {
        return _factory;
    }

private:
    FactoryFunction _factory;
};

/**
 * Functor for getting a default factory for the component instantiation
 *
 * @tparam impl_type The type of the component implementation to instantiate
 */
template <typename impl_type>
struct DefaultFactoryGetter {
    /**
     * Gets the default factory for the component instantiation
     *
     * @return The factory
     */
    constexpr auto operator()() const
    {
        return []() { return std::make_shared<impl_type>(); };
    }
};

/**
 * Specialization for ComponentDescriptor
 */
template <typename impl_type, typename... interface_types>
struct DefaultFactoryGetter<ComponentDescriptor<impl_type, interface_types...>> {
    /**
     * Gets the default factory for the component instantiation
     *
     * @return The factory
     */
    constexpr auto operator()() const
    {
        return DefaultFactoryGetter<impl_type>()();
    }
};

namespace arya {

/**
 * Component factory
 * Each element of @p component_descriptor_or_impl_types leads to the instantiation of one component
 * when @ref createComponent is called with any IID of the exposed interfaces.
 * If the element of component_descriptor_or_impl_types is a @ref ComponentDescriptor,
 * the exposed interfaces are defined by the ComponentDescriptor.
 * In contrast, if the element of component_descriptor_or_impl_types is not a @ref
 * ComponentDescriptor the factory assumes it to be a component implementation type (i. e. has type
 * "Interfaces" defined as an std::tuple with one element for each interface to be exposed and
 * derives from
 * @ref fep3::arya::IComponent), all interfaces as supported by the component implementation type
 * are exposed.
 *
 * Usage:
 * 1. Providing a component implementation type as template argument:
 *    ComponentFactory<MyComponentImpl>
 *    This makes the component factory expose all interfaces as supported
 *    by MyComponentImpl.
 * 2. Providing a @ref ComponentDescriptor as template argument:
 *    ComponentFactory<ComponentDescriptor<MyComponentImpl, IMyComponent>>
 *    This makes the component factory expose interface IMyComponent only even if
 *    MyComponentImpl supports more interfaces.
 * 3. Providing a custom factory for the component instantiation:
 *    ComponentFactory(fep3::plugin::cpp::ComponentDescriptor<MyComponent>([my_variable]()
 *        {
 *            return std::make_shared<MyComponent>(my_variable);
 *        }));
 *
 * @tparam component_descriptor_or_impl_types The types defining the components to be factored.
 *                                            Each type can be a component implementation or a @ref
 *                                            ComponentDescriptor.
 */
template <typename... component_descriptor_or_impl_types>
class ComponentFactory : public fep3::plugin::cpp::arya::ICPPPluginComponentFactory {
public:
    /// default CTOR
    ComponentFactory()
        : _instantiators(std::make_unique<
                         std::tuple<Instantiator<component_descriptor_or_impl_types>...>>()){};

    /**
     * CTOR taking @p descriptors describing the components to be created by this factory
     *
     * @tparam component_descriptor_or_impl_types The types of the component descriptors
     * @param descriptors Descriptors describing the components to be created by this factory
     */
    ComponentFactory(component_descriptor_or_impl_types&&... descriptors)
        : _instantiators(
              std::make_unique<std::tuple<Instantiator<component_descriptor_or_impl_types>...>>(
                  std::forward<component_descriptor_or_impl_types>(descriptors)...))
    {
    }

    //! @copydoc fep3::plugin::cpp::catelyn::IComponentFactory::createComponent
    std::unique_ptr<fep3::arya::IComponent> createComponent(
        const std::string& component_iid) const override
    {
        std::unique_ptr<fep3::arya::IComponent> component;

        auto component_creator = [&](auto& instantiator) {
            instantiator(component, component_iid);
        };
        std::apply([&](auto&... instantiator) { (component_creator(instantiator), ...); },
                   *_instantiators);
        return component;
    }

private:
    // instantiator for component implementation type
    template <typename component_descriptor_or_impl_type>
    class Instantiator {
    public:
        Instantiator() : _factory(DefaultFactoryGetter<component_descriptor_or_impl_type>()())
        {
        }

        Instantiator(component_descriptor_or_impl_type&& descriptor)
            : _factory(descriptor.getFactory())
        {
        }

        void operator()(std::unique_ptr<fep3::arya::IComponent>& component, const std::string& iid)
        {
            constexpr auto exported_iids = IIDArray<Interfaces>::_iids;
            // check if iid is exported by this instantiator
            if (std::find(exported_iids.begin(), exported_iids.end(), iid) != exported_iids.end()) {
                auto current_component = _component.lock();
                // on first call with matching IID create the component
                if (!current_component) {
                    current_component = _factory();
                    _component = current_component;
                    _last_call_identifier = std::make_shared<
                        fep3::plugin::cpp::arya::ComponentWrapper::CallIdentifier>();
                }
                // expose the component wrapped in a ComponentWrapper
                component = std::make_unique<fep3::plugin::cpp::arya::ComponentWrapper>(
                    current_component, _last_call_identifier);
            }
        }

    private:
        using Interfaces = typename component_descriptor_or_impl_type::Interfaces;
        const std::function<std::shared_ptr<fep3::arya::IComponent>()> _factory;
        // Note: We hold a weak pointer here in order to prevent a static deinitialization fiasco:
        //       If the component factory instance (containing the instantiators) resides in the
        //       data segment of the shared library (e. g. because it is a static variable),
        //       the underlying components might access other variables in the data segment during
        //       destruction if the lifetime of the components is controlled by the component
        //       factory. Holding it in a weak pointer shifts the destruction of the components
        //       to the point when the user of the component factory releases the returned
        //       shared pointers which occurs before static deinitialization (provided
        //       that the user releases the returned shared pointers before static
        //       deinitialization).
        std::weak_ptr<fep3::arya::IComponent> _component;
        std::shared_ptr<fep3::plugin::cpp::arya::ComponentWrapper::CallIdentifier>
            _last_call_identifier;
    };

private:
    // @ref fep3::plugin::cpp:arya::ICPPPluginComponentFactory::createComponent is declared const,
    // so we have to wrap the instantiators in a pointer in order to be able to modify them
    // from within the member function.
    const std::unique_ptr<std::tuple<Instantiator<component_descriptor_or_impl_types>...>>
        _instantiators;
};

/**
 * Component factory wrapper
 * Use this wrapper in order to implement @ref fep3_plugin_cpp_arya_getFactory :
 *    fep3::plugin::cpp::arya::ICPPPluginComponentFactory* fep3_plugin_cpp_arya_getFactory()
 *    {
 *        static auto component_factory =
 * std::make_shared<fep3::plugin::cpp::arya::ComponentFactory<MyComponent>>(); return new
 * fep3::plugin::cpp::arya::ComponentFactoryWrapper(component_factory);
 *    }
 */
class ComponentFactoryWrapper : public fep3::plugin::cpp::arya::ICPPPluginComponentFactory {
public:
    /**
     * CTOR taking @p a shared pointer to the component factory to be wrapped
     *
     * @param component_factory The component factory to be wrapped
     */
    ComponentFactoryWrapper(
        const std::shared_ptr<fep3::plugin::cpp::arya::ICPPPluginComponentFactory>&
            component_factory)
        : _component_factory(component_factory)
    {
        if (!_component_factory) {
            throw std::runtime_error("Invalid component factory pointer.");
        }
    }

    //! @copydoc fep3::plugin::cpp::catelyn::IComponentFactory::createComponent
    std::unique_ptr<fep3::arya::IComponent> createComponent(
        const std::string& component_iid) const override
    {
        return _component_factory->createComponent(component_iid);
    }

private:
    const std::shared_ptr<fep3::plugin::cpp::arya::ICPPPluginComponentFactory> _component_factory;
};

} // namespace arya

namespace catelyn {

/**
 * Component factory
 * Each element of @p component_descriptor_or_impl_types leads to the instantiation of one component
 * when @ref createComponent is called with any IID of the exposed interfaces.
 * If the element of component_descriptor_or_impl_types is a @ref
 * fep3::plugin::cpp::ComponentDescriptor, the exposed interfaces are defined by the @ref
 * fep3::plugin::cpp::ComponentDescriptor. In contrast, if the element of
 * component_descriptor_or_impl_types is not a @ref fep3::plugin::cpp::ComponentDescriptor the
 * factory assumes it to be a component implementation type (i. e. has type "Interfaces" defined as
 * an std::tuple with one element for each interface to be exposed and derives from @ref
 * fep3::arya::IComponent), all interfaces as supported by the component implementation type are
 * exposed.
 *
 * Usage:
 * 1. Providing a component implementation type as template argument:
 *    ComponentFactory<MyComponentImpl>
 *    This makes the component factory expose all interfaces as supported
 *    by MyComponentImpl.
 * 2. Providing a @ref fep3::plugin::cpp::ComponentDescriptor as template argument:
 *    ComponentFactory<ComponentDescriptor<MyComponentImpl, IMyComponent>>
 *    This makes the component factory expose interface IMyComponent only even if
 *    MyComponentImpl supports more interfaces.
 * 3. Providing a custom factory for the component instantiation:
 *    ComponentFactory(ComponentDescriptor<MyComponent>([my_variable]()
 *        {
 *            return std::make_shared<MyComponent>(my_variable);
 *        }));
 *
 * @tparam component_descriptor_or_impl_types The types defining the components to be factored.
 *                                            Each type can be a component implementation or
 *                                            a @ref fep3::plugin::cpp::ComponentDescriptor.
 */
template <typename... component_descriptor_or_impl_types>
class ComponentFactory : public fep3::plugin::cpp::catelyn::IComponentFactory {
public:
    /// default CTOR
    ComponentFactory(){};

    /**
     * CTOR taking @p descriptors describing the components to be created by this factory
     *
     * @tparam component_descriptor_or_impl_types The types of the component descriptors
     * @param descriptors Descriptors describing the components to be created by this factory
     */
    ComponentFactory(component_descriptor_or_impl_types&&... descriptors)
        : _instantiators(std::forward<component_descriptor_or_impl_types>(descriptors)...)
    {
    }

    //! @copydoc fep3::plugin::cpp::catelyn::IComponentFactory::createComponent
    std::shared_ptr<fep3::arya::IComponent> createComponent(
        const std::string& component_iid) override
    {
        std::shared_ptr<fep3::arya::IComponent> component;

        auto component_creator = [&](auto& instantiator) {
            instantiator(component, component_iid);
        };
        std::apply([&](auto&... instantiator) { (component_creator(instantiator), ...); },
                   _instantiators);
        return component;
    }

private:
    // instantiator for component implementation type
    template <typename component_descriptor_or_impl_type>
    class Instantiator {
    public:
        Instantiator() : _factory(DefaultFactoryGetter<component_descriptor_or_impl_type>()())
        {
        }

        Instantiator(component_descriptor_or_impl_type&& descriptor)
            : _factory(descriptor.getFactory())
        {
        }

        void operator()(std::shared_ptr<fep3::arya::IComponent>& component, const std::string& iid)
        {
            constexpr auto exported_iids = IIDArray<Interfaces>::_iids;
            // check if iid is exported by this instantiator
            if (std::find(exported_iids.begin(), exported_iids.end(), iid) != exported_iids.end()) {
                // on first call with matching IID create the wrapper
                if (!_component) {
                    _component = _factory();
                }
                component = _component;
            }
        }

    private:
        using Interfaces = typename component_descriptor_or_impl_type::Interfaces;
        const std::function<std::shared_ptr<fep3::arya::IComponent>()> _factory;
        std::shared_ptr<fep3::arya::IComponent> _component;
    };

private:
    std::tuple<Instantiator<component_descriptor_or_impl_types>...> _instantiators;
};

} // namespace catelyn
using catelyn::ComponentFactory;
} // namespace cpp
} // namespace plugin
} // namespace fep3
