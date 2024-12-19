/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <fep3/components/base/component_iid.h>
#include <fep3/components/base/component_intf.h>

#include <gmock/gmock.h>

///@cond nodoc

namespace fep3 {
namespace mock {
namespace arya {

class StubComponent : public fep3::arya::IComponent {
public:
    MOCK_METHOD(fep3::Result,
                createComponent,
                (const std::weak_ptr<const fep3::arya::IComponents>&),
                (override));
    MOCK_METHOD(fep3::Result, destroyComponent, (), (override));
    MOCK_METHOD(fep3::Result, initialize, (), (override));
    MOCK_METHOD(fep3::Result, deinitialize, (), (override));
    MOCK_METHOD(fep3::Result, tense, (), (override));
    MOCK_METHOD(fep3::Result, relax, (), (override));
    MOCK_METHOD(fep3::Result, start, (), (override));
    MOCK_METHOD(fep3::Result, stop, (), (override));
    MOCK_METHOD(fep3::Result, pause, (), (override));

    // note: we don't use MOCK_METHOD to mock the method getInterface
    // because there wouldn't be any meaningful return other than the this
    // pointer (or nullptr). As the instantiator of the class instance
    // does not necessarily have access to the this pointer, we implement
    // the method here just the same way as implemented in fep3::base::arya::Component.
    void* getInterface(const std::string&) override
    {
        return this;
    }
};

template <typename... component_interface_types>
class Component : public fep3::arya::IComponent, virtual public component_interface_types... {
public:
    using Interfaces = std::tuple<component_interface_types...>;

    MOCK_METHOD(fep3::Result,
                createComponent,
                (const std::weak_ptr<const fep3::arya::IComponents>&),
                (override));
    MOCK_METHOD(fep3::Result, destroyComponent, (), (override));
    MOCK_METHOD(fep3::Result, initialize, (), (override));
    MOCK_METHOD(fep3::Result, deinitialize, (), (override));
    MOCK_METHOD(fep3::Result, tense, (), (override));
    MOCK_METHOD(fep3::Result, relax, (), (override));
    MOCK_METHOD(fep3::Result, start, (), (override));
    MOCK_METHOD(fep3::Result, stop, (), (override));
    MOCK_METHOD(fep3::Result, pause, (), (override));

    // note: we don't use MOCK_METHOD to mock the method getInterface
    // because there wouldn't be any meaningful return other than the this
    // pointer (or nullptr). As the instantiator of the class instance
    // does not necessarily have access to the this pointer, we implement
    // the method here just the same way as implemented in fep3::base::arya::Component.
    void* getInterface(const std::string& iid) override
    {
        return InterfaceGetter<component_interface_types...>()(this, iid);
    }

private:
    /**
     * Functor getting a pointer to an interface by IID
     */
    template <typename... interface_types>
    struct InterfaceGetter {
        // end of compile time recursion
        void* operator()(Component*, const std::string&)
        {
            return {};
        }
    };
    /**
     * Specialization of above functor for more than zero interface types
     */
    template <typename interface_type, typename... remaining_interface_types>
    struct InterfaceGetter<interface_type, remaining_interface_types...> {
        void* operator()(Component* component, const std::string& iid)
        {
            if (interface_type::FEP3_COMP_IID == iid) {
                return static_cast<interface_type*>(component);
            }
            else {
                // compile time recursion: go on with remaining component access object types
                return InterfaceGetter<remaining_interface_types...>()(component, iid);
            }
        }
    };
};

} // namespace arya
using arya::Component;

/**
 * @brief gMock doesn't support mocking static methods from FEP_COMPONENT_IID, so we implement it
 * Usage:
 * struct IIDGetter
 * {
 *     constexpr const char* operator()() const
 *     {
 *         return "test_component.catelyn.mock.fep3.iid";
 *     }
 * };
 * getComponentIID<fep3::mock::ComponentInterface<IIDGetter>>()
 *
 * @tparam iid_getter_type The type of the functor to be used for getting the IID
 */
template <typename iid_getter_type>
class ComponentInterface {
public:
    // Updating gtest to 1.13 triggered the "Empty base optimization" (EBO)
    // because of a private inheritance in StrickMock class.
    // It will cause compilation error in Windows because StrickMock tries to verify
    // the size of the subclass.
    // To avoid this optimization we can a virtual DTOR into this class.
    virtual ~ComponentInterface() = default;
    FEP_COMPONENT_IID(iid_getter_type()())
};

} // namespace mock
} // namespace fep3

///@endcond nodoc
