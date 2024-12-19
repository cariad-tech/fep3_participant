/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <fep3/base/component_plugin_analyzer/component_description_base.h>
#include <fep3/components/base/components_intf.h>
#include <fep3/components/configuration/configuration_service_intf.h>

#include <map>
#include <string>
#include <vector>

namespace fep3::base {
class PropertyTester {
public:
    PropertyTester(fep3::arya::IComponents& components);

    void checkPropertiesBefore();

    void checkPropertiesAfter(const std::string& comp_iid);

    std::vector<PropertyInfo> getComponentProperties(
        const std::vector<std::string>& comp_iids) const;

    std::map<std::string, std::vector<PropertyInfo>> getPropertyInfos() const;

    void setNextState(std::string state);

private:
    std::vector<PropertyInfo> getProperties(const fep3::IPropertyNode& prop,
                                            const std::string& parent_node) const;

    std::vector<PropertyInfo> propsAdded(const std::vector<PropertyInfo>& props_after) const;
    std::string getNodeDefaultValue(const fep3::IPropertyNode& prop) const;

    bool propExists(const PropertyInfo& prop_info) const;

    fep3::IConfigurationService* _configuration_service;
    std::map<std::string, std::vector<PropertyInfo>> _property_infos;
    std::vector<std::string> _properties_before;
    std::string _state;
};
} // namespace fep3::base
