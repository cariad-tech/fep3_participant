/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <fep3/base/component_plugin_analyzer/component_description_base.h>
#include <fep3/components/base/components_intf.h>

#include <functional>

namespace fep3::base {
class ComponentRegistrySpy : public fep3::arya::IComponents {
public:
    template <typename T>
    T* getComponent()
    {
        return static_cast<T*>(findComponent(T::FEP3_COMP_IID)->getInterface(T::FEP3_COMP_IID));
    }

    fep3::arya::IComponent* findComponent(const std::string& fep_iid) const override;
    void add(ComponentData component_data);

    template <typename T>
    fep3::Result invoke(T t,
                        std::function<void()> invoke_before,
                        std::function<void(const std::string&)> invoke_after)
    {
        for (auto& comp: _comp_data) {
            if (invoke_before)
                invoke_before();
            setComponentToSpy(comp.getIID());
            auto res = std::invoke(t, *comp._super_component);
            const std::string component_invoked = _component_to_spy;
            setComponentToSpy("");
            if (!res)
                return res;
            else {
                if (invoke_after)
                    invoke_after(component_invoked);
            }
        }
        return {};
    }

    std::vector<ComponentData> getData();
    void setComponentToSpy(std::string comp_iid);

private:
    ComponentData* findComponentData(fep3::IComponent* comp);

    ComponentData* findComponentData(const std::string& component_iid);

    void addToDependantComponents(const std::string& component_dependency);

    std::vector<ComponentData> _comp_data;
    std::string _component_to_spy;
};
} // namespace fep3::base
