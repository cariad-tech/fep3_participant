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

#include <fep3/base/component_registry/component_registry.h>

#include <algorithm>
#include <deque>
#include <functional>

namespace fep3 {
namespace arya {

ComponentRegistry::ComponentRegistry()
{
}

ComponentRegistry::~ComponentRegistry()
{
    clear();
}

fep3::Result ComponentRegistry::registerComponent(const std::string& fep_iid,
                                                  const std::shared_ptr<IComponent>& component,
                                                  const ComponentVersionInfo& version_info)
{
    auto supported_interface = component->getInterface(fep_iid);
    if (supported_interface == nullptr) {
        RETURN_ERROR_DESCRIPTION(fep3::ERR_INVALID_TYPE,
                                 "given component does not support the interface %s",
                                 fep_iid.c_str());
    }
    IComponent* component_found = findComponent(fep_iid);
    if (component_found == nullptr) {
        _components.emplace_back(fep_iid, component);
        _comp_version_info[fep_iid] = version_info;
        return fep3::Result();
    }
    RETURN_ERROR_DESCRIPTION(fep3::ERR_INVALID_ARG, "component %s already exists", fep_iid.c_str());
}

std::pair<fep3::Result, ComponentVersionInfo> ComponentRegistry::getComponentVersion(
    const std::string& fep_iid) const
{
    const auto it = _comp_version_info.find(fep_iid);

    if (it == _comp_version_info.end()) {
        using namespace std::literals;

        return {a_util::result::Result(
                    fep3::ERR_INVALID_ARG,
                    ("Error getting version for component "s + fep_iid + ", component not found"s)
                        .c_str(),
                    __LINE__,
                    __FILE__,
                    A_UTIL_CURRENT_FUNCTION),
                {}};
    }
    else {
        return {{}, it->second};
    }
}

std::vector<std::string> ComponentRegistry::getComponentIIDs() const
{
    using CompVersionPairType = decltype(_comp_version_info)::value_type;

    std::vector<std::string> components_iids{_comp_version_info.size()};
    std::transform(_comp_version_info.begin(),
                   _comp_version_info.end(),
                   components_iids.begin(),
                   [&](const CompVersionPairType& value) { return value.first; });
    return components_iids;
}

IComponent* ComponentRegistry::findComponent(const std::string& fep_iid) const
{
    for (const auto& comp: _components) {
        if (comp.first == fep_iid) {
            return comp.second.get();
        }
    }
    return nullptr;
}

std::shared_ptr<IComponent> ComponentRegistry::findComponentByPtr(IComponent* component) const
{
    for (const auto& comp: _components) {
        if (comp.second.get() == component) {
            return comp.second;
        }
    }
    return std::shared_ptr<IComponent>();
}

fep3::Result ComponentRegistry::unregisterComponent(const std::string& fep_iid)
{
    for (decltype(_components)::iterator comp_iterator = _components.begin();
         comp_iterator != _components.end();
         comp_iterator++) {
        if (std::get<0>(*comp_iterator) == fep_iid) {
            _components.erase(comp_iterator);
            _comp_version_info.erase(fep_iid);
            return fep3::Result();
        }
    }
    RETURN_ERROR_DESCRIPTION(fep3::ERR_INVALID_ARG, "component %s does not exist", fep_iid.c_str());
}

/**
 * "raises" the @p components by invoking the @p raise_func on each component.
 * If invoking the @p raise_func fails on one component, the @raise_func is not
 * invoked on remaining components, but the @p fallback_func is invoked on those
 * components on which the invokation of the @p raise_func already succeeded.
 * @note @components might contain entries pointing to the same component.
 *       @p raise_func and @p fallback_func are invoked at most once on each component.
 * @param components The list of components to "raise"
 * @param raise_func The function to be invoked for "raising"
 * @param fallback_func The function to be invoked for fallback
 * @param func_call_name The name of the @p raise_func as string
 * @return fep3::Result ERR_NOERROR if succeeded, error code otherwise:
 * @retval fep3::ERR_UNEXPECTED An exception as thrown from within the invokation
 *                              of the @p raise_func or @fallback_func was caught.
 */
fep3::Result raiseWithFallback(
    std::vector<std::pair<std::string, std::shared_ptr<IComponent>>>& components,
    std::function<fep3::Result(IComponent&)> raise_func,
    std::function<fep3::Result(IComponent&)> fallback_func,
    const std::string& func_call_name)
{
    std::deque<IComponent*> succeeded_components;
    std::deque<IComponent*> invoked_components;
    fep3::Result res;
    for (auto& component_iter: components) {
        const auto& current_component = component_iter.second;
        // In case of FEP Super Components, the entries point to the same component,
        // so invoke only if invokation didn't occur on the underlying component yet.
        if (std::find(invoked_components.cbegin(),
                      invoked_components.cend(),
                      current_component.get()) == invoked_components.cend()) {
            // we need to catch here because that might be user code in the plugins
            try {
                res = raise_func(*current_component.get());
            }
            catch (const std::exception& ex) {
                res = fep3::Result(
                    ERR_UNEXPECTED,
                    std::string("Exception occured while " + func_call_name + ": " + ex.what())
                        .c_str(),
                    __LINE__,
                    __FILE__,
                    std::string("ComponentRegistry::" + func_call_name).c_str());
            }

            // remember the components on which the function was invoked
            invoked_components.push_back(current_component.get());

            if (res) {
                // remember the components where raise_function succeeded
                succeeded_components.push_back(current_component.get());
            }
            else {
                std::deque<IComponent*> fell_back_components;
                // on error fallback to the previous "state" of the component (reverse remember
                // list)
                for (decltype(succeeded_components)::reverse_iterator comp_fallback =
                         succeeded_components.rbegin();
                     comp_fallback != succeeded_components.rend();
                     comp_fallback++) {
                    // In case of FEP Super Components, the entries point to the same component,
                    // so invoke fallback only if invokation of fallback didn't occur on the
                    // underlying component yet.
                    if (std::find(fell_back_components.cbegin(),
                                  fell_back_components.cend(),
                                  *comp_fallback) == fell_back_components.cend()) {
                        // we need to catch here because the implementation might be user code in
                        // the plugins
                        try {
                            fallback_func(**comp_fallback);
                        }
                        catch (const std::exception& ex) {
                            res |= fep3::Result(
                                ERR_UNEXPECTED,
                                std::string("Exception occured during invokation of fallback for " +
                                            func_call_name + ": " + ex.what())
                                    .c_str(),
                                __LINE__,
                                __FILE__,
                                std::string("ComponentRegistry::" + func_call_name).c_str());
                        }
                        // remember the components on which the fallback was invoked
                        fell_back_components.push_back(*comp_fallback);
                    }
                }
                return res;
            }
        }
    }
    return {};
}

/**
 * Invokes the @p function on each component.
 * @note @components might contain entries pointing to the same component.
 *       @p function is invoked exactly on each component.
 * @param components The list of components to invoke @p function on
 * @param function The function to be invoked
 * @param function_name The name of the @p function as string
 * @return fep3::Result ERR_NOERROR if succeeded, error code otherwise:
 * @retval fep3::ERR_UNEXPECTED An exception as thrown from within the invokation
 *                              of the @p function was caught.
 */
fep3::Result invokeReversely(
    std::vector<std::pair<std::string, std::shared_ptr<IComponent>>>& components,
    std::function<fep3::Result(IComponent&)> function,
    const std::string& function_name)
{
    std::deque<IComponent*> invoked_components;
    fep3::Result result;
    for (std::remove_reference<decltype(components)>::type::reverse_iterator component_iter =
             components.rbegin();
         component_iter != components.rend();
         component_iter++) {
        const auto& current_component = component_iter->second;
        // In case of FEP Super Components, the entries point to the same component,
        // so invoke only if invokation didn't occur on the underlying component yet.
        if (std::find(invoked_components.cbegin(),
                      invoked_components.cend(),
                      current_component.get()) == invoked_components.cend()) {
            try {
                result |= function(*current_component);
            }
            catch (const std::exception& ex) {
                result |= fep3::Result(
                    ERR_UNEXPECTED,
                    std::string("Exception occured while " + function_name + ": " + ex.what())
                        .c_str(),
                    __LINE__,
                    __FILE__,
                    std::string("ComponentRegistry::" + function_name).c_str());
            }
            // remember the components on which the function was invoked
            invoked_components.push_back(current_component.get());
        }
    }
    return result;
}

fep3::Result ComponentRegistry::create()
{
    // create or fallback if one of it failed
    return raiseWithFallback(
        _components,
        [components = shared_from_this()](IComponent& comp) -> fep3::Result {
            return comp.createComponent(static_cast<std::weak_ptr<const IComponents>>(
                std::static_pointer_cast<const IComponents>(components)));
        },
        [](IComponent& comp) -> fep3::Result { return comp.destroyComponent(); },
        "create");
}

fep3::Result ComponentRegistry::destroy()
{
    return invokeReversely(
        _components,
        [](IComponent& comp) -> fep3::Result { return comp.destroyComponent(); },
        "destroy");
}

fep3::Result ComponentRegistry::initialize()
{
    // initializing or fallback if one of it failed
    return raiseWithFallback(
        _components,
        [](IComponent& comp) -> fep3::Result { return comp.initialize(); },
        [](IComponent& comp) -> fep3::Result { return comp.deinitialize(); },
        "initialize");
}

fep3::Result ComponentRegistry::tense()
{
    // getready or fallback if one of it failed
    return raiseWithFallback(
        _components,
        [](IComponent& comp) -> fep3::Result { return comp.tense(); },
        [](IComponent& comp) -> fep3::Result { return comp.relax(); },
        "tense");
}

fep3::Result ComponentRegistry::relax()
{
    return invokeReversely(
        _components, [](IComponent& comp) -> fep3::Result { return comp.relax(); }, "relax");
}

fep3::Result ComponentRegistry::deinitialize()
{
    return invokeReversely(
        _components,
        [](IComponent& comp) -> fep3::Result { return comp.deinitialize(); },
        "deinitialize");
}

fep3::Result ComponentRegistry::start()
{
    // start or fallback if one of it failed
    return raiseWithFallback(
        _components,
        [](IComponent& comp) -> fep3::Result { return comp.start(); },
        [](IComponent& comp) -> fep3::Result { return comp.stop(); },
        "start");
}

fep3::Result ComponentRegistry::stop()
{
    return invokeReversely(
        _components, [](IComponent& comp) -> fep3::Result { return comp.stop(); }, "stop");
}

fep3::Result ComponentRegistry::pause()
{
    // return an error because pause is not implemented by any native component yet
    RETURN_ERROR_DESCRIPTION(ERR_NOT_IMPL, "The feature 'Pause' is not supported yet.");
}

void ComponentRegistry::clear()
{
    // destroy in reverse order
    decltype(_components)::iterator current_comp_it = _components.end();
    while (!_components.empty()) {
        _components.erase(--current_comp_it);
        current_comp_it = _components.end();
    }
    _comp_version_info.clear();
}
} // namespace arya
} // namespace fep3
