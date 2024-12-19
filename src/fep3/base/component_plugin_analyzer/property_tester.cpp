/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "property_tester.h"

#include <algorithm>
#include <iterator>

namespace fep3::base {
PropertyTester::PropertyTester(fep3::arya::IComponents& components)
{
    _configuration_service = components.getComponent<fep3::IConfigurationService>();
    assert(_configuration_service);
}

void PropertyTester::checkPropertiesBefore()
{
    // auto prop1 = _config_service_rpc_client->getAllProperties("/");
    auto root_property_node = _configuration_service->getConstNode("");
    assert(root_property_node);
    auto prop2 = getProperties(*root_property_node, "");
    _properties_before.clear();
    std::transform(prop2.begin(),
                   prop2.end(),
                   std::back_inserter(_properties_before),
                   [](const PropertyInfo& prop_info) { return prop_info._path; });
}

void PropertyTester::checkPropertiesAfter(const std::string& comp_iid)
{
    auto root_property_node = _configuration_service->getConstNode("");
    assert(root_property_node);
    auto prop1 = getProperties(*root_property_node, "");
    auto props_added = propsAdded(prop1);
    _property_infos[comp_iid].insert(
        _property_infos[comp_iid].begin(), props_added.begin(), props_added.end());
}

std::vector<PropertyInfo> PropertyTester::getComponentProperties(
    const std::vector<std::string>& comp_iids) const
{
    std::vector<PropertyInfo> ret;
    for (const auto& comp_iid: comp_iids) {
        if (_property_infos.count(comp_iid) > 0)
            ret.insert(ret.end(),
                       _property_infos.at(comp_iid).begin(),
                       _property_infos.at(comp_iid).end());
    }

    return ret;
}

std::map<std::string, std::vector<PropertyInfo>> PropertyTester::getPropertyInfos() const
{
    return _property_infos;
}

void PropertyTester::setNextState(std::string state)
{
    _state = std::move(state);
}

std::vector<PropertyInfo> PropertyTester::getProperties(const fep3::IPropertyNode& prop,
                                                        const std::string& parent_node) const
{
    std::vector<PropertyInfo> ret;

    auto children = prop.getChildren();
    if (children.empty()) {
        if (prop.getTypeName() == "node") {
            return ret;
        }
        else {
            return {PropertyInfo{_state,
                                 prop.getName(),
                                 prop.getTypeName(),
                                 getNodeDefaultValue(prop),
                                 parent_node}};
        }
    }
    else {
        for (auto& node: prop.getChildren()) {
            if (node) {
                auto properties = getProperties(*node, parent_node + "/" + node->getName());
                ret.insert(ret.end(), properties.begin(), properties.end());
            }
        }
    }
    return ret;
}

std::vector<PropertyInfo> PropertyTester::propsAdded(
    const std::vector<PropertyInfo>& props_after) const
{
    std::vector<PropertyInfo> ret;
    for (auto prop_after: props_after) {
        if (!propExists(prop_after)) {
            prop_after._available_in_state = _state;
            ret.push_back(prop_after);
        }
    }
    return ret;
}

bool PropertyTester::propExists(const PropertyInfo& prop_info) const
{
    auto it = std::find_if(_properties_before.begin(),
                           _properties_before.end(),
                           [&prop_info](const std::string& prop_name_before) {
                               return prop_info._path == prop_name_before;
                           });
    return it != _properties_before.end();
}

std::string PropertyTester::getNodeDefaultValue(const fep3::IPropertyNode& prop) const
{
    // this should be only the case for string like types
    if (prop.getValue().empty()) {
        return "\"\"";
    }
    else {
        return prop.getValue();
    }
}
} // namespace fep3::base
