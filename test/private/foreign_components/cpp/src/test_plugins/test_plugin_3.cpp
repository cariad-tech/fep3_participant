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

#include "test_component_a_intf.h"
#include "test_component_b_intf.h"

#include <fep3/components/base/component.h>
#include <fep3/plugin/cpp/cpp_plugin_impl_arya.hpp>

class TestSuperComponentAB : public fep3::base::Component<ITestComponentA, ITestComponentB> {
public:
    TestSuperComponentAB() = default;
    ~TestSuperComponentAB() = default;

    std::string getIdentifier() const override
    {
        return "test_cpp_plugin_3:super_component_a_b";
    }

    int32_t get() override
    {
        return _value;
    }

    void set(int32_t value) override
    {
        _value = value;
    }

private:
    int32_t _value{};
};

void fep3_plugin_getPluginVersion(void (*callback)(void*, const char*), void* destination)
{
    callback(destination, "0.0.3");
}

class MyPluginComponentFactory : public fep3::plugin::cpp::catelyn::IComponentFactory {
private:
    std::shared_ptr<fep3::IComponent> createComponent(const std::string& component_iid)
    {
        if (!_super_component_a_b) {
            _super_component_a_b = std::make_shared<TestSuperComponentAB>();
        }
        if (component_iid == fep3::getComponentIID<ITestComponentA>()) {
            return _super_component_a_b;
        }
        else if (component_iid == fep3::getComponentIID<ITestComponentB>()) {
            return _super_component_a_b;
        }
        else {
            return {};
        }
    }

private:
    std::shared_ptr<TestSuperComponentAB> _super_component_a_b;
};

fep3::plugin::cpp::catelyn::IComponentFactory* fep3_plugin_cpp_catelyn_getFactory()
{
    return new MyPluginComponentFactory;
}
