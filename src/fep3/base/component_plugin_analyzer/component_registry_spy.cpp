/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "component_registry_spy.h"

#include "component_path_hash_function.h"

namespace fep3::base {
fep3::arya::IComponent* ComponentRegistrySpy::findComponent(const std::string& fep_iid) const
{
    const_cast<ComponentRegistrySpy*>(this)->addToDependantComponents(fep_iid);
    auto it = std::find_if(_comp_data.begin(), _comp_data.end(), [&fep_iid](auto& comp) {
        return comp.hasIId(fep_iid);
    });

    if (it != _comp_data.end()) {
        return it->_super_component.get();
    }
    else {
        return nullptr;
    }
}

void ComponentRegistrySpy::add(ComponentData component_data)
{
    auto existing_comp_data = findComponentData(component_data._super_component.get());
    if (existing_comp_data) {
        existing_comp_data->_iids.push_back(component_data.getIID());
    }
    else {
        _comp_data.push_back(component_data);
    }
}

std::vector<ComponentData> ComponentRegistrySpy::getData()
{
    for (auto& comp: _comp_data) {
        // call component destructor
        comp._super_component = nullptr;
    }

    std::vector<ComponentData> copy = _comp_data;
    _comp_data.clear();
    return copy;
}

void ComponentRegistrySpy::setComponentToSpy(std::string comp_iid)
{
    _component_to_spy = std::move(comp_iid);
}

ComponentData* ComponentRegistrySpy::findComponentData(fep3::IComponent* comp)
{
    if (comp == nullptr) {
        return nullptr;
    }
    auto it = std::find_if(_comp_data.begin(), _comp_data.end(), [&comp](auto& comp_data) {
        return comp_data._super_component.get() == comp;
    });
    if (it == _comp_data.end()) {
        return nullptr;
    }
    else {
        return &*it;
    }
}

ComponentData* ComponentRegistrySpy::findComponentData(const std::string& component_iid)
{
    auto it = std::find_if(_comp_data.begin(), _comp_data.end(), [&component_iid](auto& comp_data) {
        return comp_data.hasIId(component_iid);
    });
    if (it == _comp_data.end()) {
        return nullptr;
    }
    else {
        return &*it;
    }
}

void ComponentRegistrySpy::addToDependantComponents(const std::string& component_dependency)
{
    if (component_dependency == _component_to_spy) {
        return;
    }
    ComponentData* p = findComponentData(_component_to_spy);
    if (p) {
        p->_required_components.insert(component_dependency);
    }
}
} // namespace fep3::base
