/**
 * @file
 * @copyright
 * @verbatim
Copyright @ 2021 VW Group. All rights reserved.

    This Source Code Form is subject to the terms of the Mozilla
    Public License, v. 2.0. If a copy of the MPL was not distributed
    with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

If it is not possible or desirable to put the notice in a particular file, then
You may include the notice in a location (such as a LICENSE file in a
relevant directory) where a recipient would be likely to look for such a notice.

You may add additional accurate notices of copyright ownership.

@endverbatim
 */


#include <fep3/plugin/cpp/cpp_plugin_impl_arya.hpp>
#include <fep3/plugin/cpp/cpp_plugin_component_factory.h>
#include <fep3/components/base/component.h>
#include "test_component_a_intf.h"
#include "test_component_b_intf.h"

class TestComponentA : public fep3::base::Component<ITestComponentA>
{
    int32_t _value;
    public:
        TestComponentA() = default;
        ~TestComponentA() = default;

        std::string getIdentifier() const override
        {
            return "test_cpp_plugin_1:component_a";
        }
        int32_t get() override
        {
            return _value;
        }
        void set(int32_t value)
        {
            _value = value;
        }
};

class TestComponentB : public fep3::base::Component<ITestComponentB>
{
    int32_t _value;
    public:
        TestComponentB() = default;
        ~TestComponentB() = default;

        std::string getIdentifier() const override
        {
            return "test_cpp_plugin_1:component_b";
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


class MyPluginComponentFactory : public fep3::plugin::cpp::ICPPPluginComponentFactory
{
private:
    std::unique_ptr<fep3::IComponent> createComponent(const std::string& component_iid) const
    {
        if (component_iid == fep3::getComponentIID<ITestComponentA>())
        {
            return std::unique_ptr<fep3::IComponent>(new TestComponentA());
        }
        else if (component_iid == fep3::getComponentIID<ITestComponentB>())
        {
            return std::unique_ptr<fep3::IComponent>(new TestComponentB());
        }
        else
        {
            return {};
        }
    }
};

void fep3_plugin_getPluginVersion(void(*callback)(void*, const char*), void* destination)
{
    callback(destination, "0.0.1");
}

fep3::plugin::cpp::arya::ICPPPluginComponentFactory* fep3_plugin_cpp_arya_getFactory()
{
    return new MyPluginComponentFactory;
}
