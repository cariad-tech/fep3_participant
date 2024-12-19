/**
 * @copyright
 * @verbatim
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * @endverbatim
 */

#include "convenient_component_registry.h"

#include "stub_component_factory.h"

#include <fep3/plugin/cpp/cpp_plugin_component_factory_intf.h>
extern "C" {
fep3::plugin::cpp::catelyn::IComponentFactory* fep3_plugin_cpp_catelyn_getFactory();
}
namespace fep3::test {
ComponentRegistry::ComponentRegistry(const std::vector<std::string>& native_iids,
                                     const std::vector<std::string>& stub_iids)
{
    _native_factory = std::unique_ptr<fep3::plugin::cpp::catelyn::IComponentFactory>(
        fep3_plugin_cpp_catelyn_getFactory());

    _stub_factory = std::make_unique<StubComponentsFactory>();
    _comp_registry = std::make_shared<fep3::ComponentRegistry>();

    for (const auto& iid: native_iids) {
        addNativeComponentToRegistry(iid);
    }
    for (const auto& iid: stub_iids) {
        addStubComponentToRegistry(iid);
    }
}

ComponentRegistry::~ComponentRegistry() = default;

void ComponentRegistry::addNativeComponentToRegistry(const std::string& iid)
{
    auto component = _native_factory->createComponent(iid);
    _comp_registry->registerComponent(iid, component, ComponentVersionInfo{});
}

void ComponentRegistry::addStubComponentToRegistry(const std::string& iid)
{
    auto component = _stub_factory->createComponent(iid);
    _comp_registry->registerComponent(iid, component, ComponentVersionInfo{});
}

void ComponentRegistry::unregisterComponent(const std::string& iid)
{
    _comp_registry->unregisterComponent(iid);
}

std::shared_ptr<fep3::ComponentRegistry> ComponentRegistry::getRegistry()
{
    return _comp_registry;
}

} // namespace fep3::test
