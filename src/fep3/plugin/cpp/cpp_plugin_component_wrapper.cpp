/**
 * @file
 * @copyright
 * @verbatim
Copyright @ 2022 VW Group. All rights reserved.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
@endverbatim
 */

#include <fep3/plugin/cpp/cpp_plugin_component_wrapper.h>

#include <stdexcept>

namespace fep3 {
namespace plugin {
namespace cpp {
namespace arya {

ComponentWrapper::ComponentWrapper(const std::shared_ptr<fep3::arya::IComponent>& component,
                                   const std::shared_ptr<CallIdentifier>& last_call_identifier)
    : _component(component), _last_call_identifier(last_call_identifier)
{
    if (!_component) {
        throw std::runtime_error("Invalid component pointer.");
    }
    if (!_last_call_identifier) {
        throw std::runtime_error("Invalid last call identifier pointer.");
    }
}

fep3::Result ComponentWrapper::createComponent(
    const std::weak_ptr<const fep3::arya::IComponents>& components)
{
    return callFiltered(
        &fep3::arya::IComponent::createComponent, CallIdentifier::createComponent, components);
}

fep3::Result ComponentWrapper::destroyComponent()
{
    return callFiltered(&fep3::arya::IComponent::destroyComponent,
                        CallIdentifier::destroyComponent);
}

fep3::Result ComponentWrapper::initialize()
{
    return callFiltered(&fep3::arya::IComponent::initialize, CallIdentifier::initialize);
}

fep3::Result ComponentWrapper::tense()
{
    return callFiltered(&fep3::arya::IComponent::tense, CallIdentifier::tense);
}

fep3::Result ComponentWrapper::relax()
{
    return callFiltered(&fep3::arya::IComponent::relax, CallIdentifier::relax);
}

fep3::Result ComponentWrapper::start()
{
    return callFiltered(&fep3::arya::IComponent::start, CallIdentifier::start);
}

fep3::Result ComponentWrapper::stop()
{
    return callFiltered(&fep3::arya::IComponent::stop, CallIdentifier::stop);
}

fep3::Result ComponentWrapper::pause()
{
    return callFiltered(&fep3::arya::IComponent::pause, CallIdentifier::pause);
}

fep3::Result ComponentWrapper::deinitialize()
{
    return callFiltered(&fep3::arya::IComponent::deinitialize, CallIdentifier::deinitialize);
}

void* ComponentWrapper::getInterface(const std::string& iid)
{
    return _component->getInterface(iid);
}

template <typename method_type, typename... argument_types>
fep3::Result ComponentWrapper::callFiltered(method_type&& method,
                                            const CallIdentifier& call_identifier,
                                            argument_types&&... arguments)
{
    // if the call didn't occur yet, invoke it on the wrapped component instance
    if (*_last_call_identifier != call_identifier) {
        const auto result = (_component.get()->*method)(std::forward<argument_types>(arguments)...);
        // set the last call identifier for this component only if the invokation was successful
        // (because in case of error, a subsequent invokation to the same call might occur which
        // must not be filtered out)
        if (result) {
            *_last_call_identifier = call_identifier;
        }
        return result;
    }
    else {
        return {};
    }
}

} // namespace arya
} // namespace cpp
} // namespace plugin
} // namespace fep3