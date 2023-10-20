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
#include "test_component_c_intf.h"

#include <fep3/components/base/component.h>
#include <fep3/plugin/cpp/cpp_plugin_impl_arya.hpp>

class TestComponentA : public fep3::base::Component<ITestComponentA> {
    int32_t _value;

public:
    TestComponentA() = default;
    ~TestComponentA() = default;

    std::string getIdentifier() const override
    {
        return "test_cpp_plugin_2:component_a";
    }

    int32_t get() override
    {
        return _value;
    }

    void set(int32_t value) override
    {
        _value = value;
    }
};

class TestComponentC : public fep3::base::Component<ITestComponentC> {
public:
    TestComponentC() = default;
    ~TestComponentC() = default;

    std::string getIdentifier() const override
    {
        return "test_cpp_plugin_2:component_c";
    }
};

void fep3_plugin_getPluginVersion(void (*callback)(void*, const char*), void* destination)
{
    callback(destination, "0.0.2");
}

class MyPluginComponentFactory : public fep3::plugin::cpp::catelyn::IComponentFactory {
private:
    std::shared_ptr<fep3::IComponent> createComponent(const std::string& component_iid)
    {
        if (component_iid == fep3::getComponentIID<ITestComponentA>()) {
            return std::make_shared<TestComponentA>();
        }
        else if (component_iid == fep3::getComponentIID<ITestComponentC>()) {
            return std::make_shared<TestComponentC>();
        }
        else {
            return {};
        }
    }
};

fep3::plugin::cpp::catelyn::IComponentFactory* fep3_plugin_cpp_catelyn_getFactory()
{
    return new MyPluginComponentFactory;
}
