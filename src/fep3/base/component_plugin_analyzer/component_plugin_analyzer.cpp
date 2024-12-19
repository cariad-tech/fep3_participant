/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "component_factory.h"
#include "component_path_hash_function.h"
#include "component_registry_spy.h"
#include "property_tester.h"
#include "result_printer.h"
#include "signal_tester.h"

#include <fep3/base/component_plugin_analyzer/component_plugin_analyzer.h>
#include <fep3/components/service_bus/service_bus_intf.h>

#include <boost/algorithm/string/join.hpp>

#include <iostream>

namespace {
const std::string test_participant_name = "test_participant_name";

void prepareServiceBus(fep3::base::ComponentRegistrySpy& component_registry_spy)
{
    fep3::IServiceBus* service_bus = component_registry_spy.getComponent<fep3::IServiceBus>();
    assert(service_bus);
    uint32_t test_participant_port = 0;

    const std::string participant_host = "http://localhost:";
    const std::string system_name = "test_with_service_bus_default";

    auto res = service_bus->createSystemAccess(system_name, "use_default_url", true);
    if (!res) {
        return;
    }

    auto sysaccess = service_bus->getSystemAccess(system_name);
    if (!sysaccess) {
        return;
    }

    auto participant_url = participant_host + std::to_string(test_participant_port);
    res = sysaccess->createServer(test_participant_name, participant_url);

    if (!res) {
        return;
    }
}
} // namespace

namespace fep3::base {
ComponentPluginAnalyzer::ComponentPluginAnalyzer(
    const std::vector<ComponentToAnalyze>& components_to_analyze)
    : _spy(std::make_shared<ComponentRegistrySpy>())

{
    addComponentsToSpy(components_to_analyze);
    prepareServiceBus(*_spy);
    _prop_tester = std::make_unique<PropertyTester>(*_spy);
    _sig_tester = std::make_unique<SignalTester>(*_spy);
}

ComponentPluginAnalyzer::~ComponentPluginAnalyzer()
{
}

std::vector<ComponentMetaModelInfo> ComponentPluginAnalyzer::getData()
{
    auto component_dep_data = _spy->getData();
    std::vector<ComponentMetaModelInfo> _component_meta_info;
    for (const auto comp_dep: component_dep_data) {
        _component_meta_info.emplace_back(comp_dep,
                                          _prop_tester->getComponentProperties(comp_dep._iids),
                                          _sig_tester->getComponentSignals(comp_dep._iids));
    }

    return _component_meta_info;
}

fep3::Result ComponentPluginAnalyzer::analyzeStateTransition(
    std::function<fep3::Result(std::shared_ptr<fep3::IComponents>, fep3::IComponent&)> callable,
    const std::string& next_state)
{
    auto cc = [&](fep3::IComponent& comp) { return callable(_spy, comp); };
    return triggerComponentRegistryAndAnalyze(cc, next_state);
}

void ComponentPluginAnalyzer::addComponentsToSpy(
    const std::vector<ComponentToAnalyze>& components_to_analyze)
{
    std::unordered_set<ComponentPath> unique_component_paths =
        getAllComponentFilePaths(components_to_analyze);

    ComponentFactory factory(unique_component_paths);

    for (const auto& component_to_analyze: components_to_analyze) {
        auto component = factory.createComponent(component_to_analyze);
        _spy->add(component);
    }
}

std::unordered_set<ComponentPath> ComponentPluginAnalyzer::getAllComponentFilePaths(
    const std::vector<ComponentToAnalyze>& components_to_analyze)
{
    std::unordered_set<ComponentPath> component_paths;

    for (const auto& component_to_analyze: components_to_analyze) {
        component_paths.insert(component_to_analyze._component_path);
    }

    return component_paths;
}

fep3::Result ComponentPluginAnalyzer::triggerComponentRegistryAndAnalyze(
    std::function<fep3::Result(fep3::IComponent&)> callable, const std::string& next_state)
{
    _prop_tester->setNextState(next_state);
    auto call_before = [&]() {
        _prop_tester->checkPropertiesBefore();
        _sig_tester->checkSignalsBefore();
    };
    auto call_after = [&](const std::string& s) {
        _prop_tester->checkPropertiesAfter(s);
        _sig_tester->checkSignalsAfter(s);
    };

    auto res = _spy->invoke(callable, call_before, call_after);

    if (!res)
        std::cout << res.getDescription();
    return res;
}

void ComponentPluginAnalyzer::triggerComponentAndAnalyze(std::function<void()> callable,
                                                         const std::string& iid)
{
    _prop_tester->setNextState("ComponentFunction");
    auto call_before = [&]() {
        _prop_tester->checkPropertiesBefore();
        _sig_tester->checkSignalsBefore();
    };
    auto call_after = [&](const std::string& s) {
        _prop_tester->checkPropertiesAfter(s);
        _sig_tester->checkSignalsAfter(s);
    };

    call_before();
    _spy->setComponentToSpy(iid);
    callable();
    _spy->setComponentToSpy("");
    call_after(iid);
}

fep3::arya::IComponent* ComponentPluginAnalyzer::findComponent(const std::string& fep_iid)
{
    _spy->setComponentToSpy("");
    return _spy->findComponent(fep_iid);
}

void ComponentPluginAnalyzer::print(const std::vector<ComponentMetaModelInfo>& meta_info)
{
    ::fep3::base::print(meta_info);
}
} // namespace fep3::base
