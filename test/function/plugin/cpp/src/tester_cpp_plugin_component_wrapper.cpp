/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <fep3/components/base/mock_component.h>
#include <fep3/components/base/mock_components.h>
#include <fep3/plugin/cpp/cpp_plugin_component_wrapper.h>

struct IIDGetterA {
    constexpr const char* operator()() const
    {
        return "test_component_a.mock.fep3.iid";
    }
};

struct IIDGetterB {
    constexpr const char* operator()() const
    {
        return "test_component_b.mock.fep3.iid";
    }
};

struct IIDGetterC {
    constexpr const char* operator()() const
    {
        return "test_component_c.mock.fep3.iid";
    }
};

class arya_CPPPluginComponentWrapperTester : public ::testing::Test {
protected:
    class ComponentInterfaceA : public fep3::mock::ComponentInterface<IIDGetterA> {
    public:
        virtual void foo() const = 0;
    };
    // disruptive change from ComponentInterfaceA to ComponentInterfaceB: changed the method name
    // from "foo" to "bar" (in other words: removed "foo" and added "bar")
    class ComponentInterfaceB : public fep3::mock::ComponentInterface<IIDGetterB> {
    public:
        virtual void bar() const = 0;
    };

    class SuperComponentAB
        : public fep3::mock::arya::Component<ComponentInterfaceA, ComponentInterfaceB> {
    public:
        MOCK_METHOD(void, foo, (), (const, override));
        MOCK_METHOD(void, bar, (), (const, override));
    };
};

/**
 * Tests FEP Super Component using @ref fep3::plugin::cpp::arya::ComponentWrapper
 */
TEST_F(arya_CPPPluginComponentWrapperTester, test_SuperComponent)
{
    const auto& last_call_identifier_super_component_a_b =
        std::make_shared<fep3::plugin::cpp::arya::ComponentWrapper::CallIdentifier>();

    const auto& super_component_a_b_mock =
        std::make_shared<::testing::StrictMock<SuperComponentAB>>();
    fep3::plugin::cpp::arya::ComponentWrapper super_component_wrapper_a(
        super_component_a_b_mock, last_call_identifier_super_component_a_b);
    fep3::plugin::cpp::arya::ComponentWrapper super_component_wrapper_b(
        super_component_a_b_mock, last_call_identifier_super_component_a_b);

    const auto& components_mock = std::make_shared<::testing::NiceMock<fep3::mock::Components>>();

    // call to createComponent on wrapper a must result in state transition call
    EXPECT_CALL(*super_component_a_b_mock, createComponent(::testing::_))
        .WillOnce(::testing::Return(fep3::Result{}));
    EXPECT_EQ(fep3::ERR_NOERROR,
              super_component_wrapper_a.createComponent(components_mock).getErrorCode());

    // subsequent call to createComponent on wrapper b must NOT result in state transition call,
    // because this is filtered out
    EXPECT_EQ(fep3::ERR_NOERROR,
              super_component_wrapper_b.createComponent(components_mock).getErrorCode());

    // call to destroyComponent on wrapper b must result in state transition call
    EXPECT_CALL(*super_component_a_b_mock, destroyComponent())
        .WillOnce(::testing::Return(fep3::Result{}));
    EXPECT_EQ(fep3::ERR_NOERROR, super_component_wrapper_b.destroyComponent().getErrorCode());

    // subsequent call to destroyComponent on wrapper b must NOT result in state transition call,
    // because this is filtered out
    EXPECT_EQ(fep3::ERR_NOERROR, super_component_wrapper_b.destroyComponent().getErrorCode());

    // another call to createComponent on wrapper b must result in state transition call
    EXPECT_CALL(*super_component_a_b_mock, createComponent(::testing::_))
        .WillOnce(::testing::Return(fep3::Result{}));
    EXPECT_EQ(fep3::ERR_NOERROR,
              super_component_wrapper_b.createComponent(components_mock).getErrorCode());

    // subsequent call to createComponent on wrapper a must NOT result in state transition call,
    // because this is filtered out
    EXPECT_EQ(fep3::ERR_NOERROR,
              super_component_wrapper_a.createComponent(components_mock).getErrorCode());

    // call to initialize on wrapper a must result in state transition call
    // note: here the state transition call fails
    EXPECT_CALL(*super_component_a_b_mock, initialize())
        .WillOnce(::testing::Return(fep3::Result{fep3::ERR_FAILED}));
    EXPECT_EQ(fep3::ERR_FAILED, super_component_wrapper_a.initialize().getErrorCode());

    // subsequent call to initialize on wrapper b must result in state transition call
    EXPECT_CALL(*super_component_a_b_mock, initialize())
        .WillOnce(::testing::Return(fep3::Result{}));
    EXPECT_EQ(fep3::ERR_NOERROR, super_component_wrapper_b.initialize().getErrorCode());

    // subsequent call to initialize on wrapper a must NOT result in state transition call, because
    // this is filtered out
    EXPECT_EQ(fep3::ERR_NOERROR, super_component_wrapper_a.initialize().getErrorCode());

    // test getInterface and downcast to the known interfaces
    const auto& component_a =
        static_cast<ComponentInterfaceA*>(super_component_wrapper_a.getInterface(IIDGetterA{}()));
    const auto& component_b =
        static_cast<ComponentInterfaceB*>(super_component_wrapper_b.getInterface(IIDGetterB{}()));
    EXPECT_CALL(*super_component_a_b_mock, foo()).Times(1);
    component_a->foo();
    EXPECT_CALL(*super_component_a_b_mock, bar()).Times(1);
    component_b->bar();

    const auto& unsupported_iid = std::string("test_unsupported.fep3.iid");
    // getInterface must return nullptr for unsupported interfaces
    EXPECT_EQ(nullptr, super_component_wrapper_a.getInterface(unsupported_iid));
    EXPECT_EQ(nullptr, super_component_wrapper_b.getInterface(unsupported_iid));
}

class catelyn_CPPPluginComponentWrapperTester : public ::testing::Test {
protected:
    class ComponentInterfaceA : public fep3::mock::ComponentInterface<IIDGetterA> {
    public:
        virtual void foo() const = 0;
    };
    // disruptive change: changed return type of foo
    class ComponentInterfaceB : public fep3::mock::ComponentInterface<IIDGetterB> {
    public:
        virtual int foo() const = 0;
    };
    // another disruptive change: renamed method foo to bar (or in other words: removed foo and
    // introduced bar) (and added method getCTORArg)
    class ComponentInterfaceC : public fep3::mock::ComponentInterface<IIDGetterC> {
    public:
        virtual int bar() const = 0;
        virtual int getCTORArg() const = 0;
    };

    class ComponentC : public fep3::mock::arya::Component<ComponentInterfaceC> {
    public:
        // CTOR takes an argument
        ComponentC(int ctor_arg) : _ctor_arg(ctor_arg)
        {
        }
        int getCTORArg() const override
        {
            return _ctor_arg;
        }
        // mock methods of interface ComponentInterfaceC
        MOCK_METHOD(int, bar, (), (const, override));

    private:
        const int _ctor_arg{};
    };

    // wrap component C in order to add interface B
    class SuperComponentBC
        : public fep3::plugin::cpp::catelyn::ComponentWrapper<::testing::StrictMock<ComponentC>,
                                                              ComponentInterfaceB> {
    public:
        SuperComponentBC(int baz)
            : fep3::plugin::cpp::catelyn::ComponentWrapper<::testing::StrictMock<ComponentC>,
                                                           ComponentInterfaceB>(baz)
        {
        }
        // forward methods of ComponentInterfaceB to the wrapped component
        int foo() const override
        {
            return _component.bar();
        }
    };

    // cascade the wrappers in order to add interface A
    class SuperComponentABC
        : public fep3::plugin::cpp::catelyn::ComponentWrapper<SuperComponentBC,
                                                              ComponentInterfaceA> {
    public:
        SuperComponentABC(int baz)
            : fep3::plugin::cpp::catelyn::ComponentWrapper<SuperComponentBC, ComponentInterfaceA>(
                  baz)
        {
        }
        // forward methods of ComponentInterfaceA to the wrapped component
        void foo() const override
        {
            _component.foo();
        }
    };
};

/**
 * Tests FEP Super Component using @ref fep3::plugin::cpp::catelyn::ComponentWrapper
 */
TEST_F(catelyn_CPPPluginComponentWrapperTester, test_ComponentWrapper)
{
    std::unique_ptr<
        fep3::plugin::cpp::catelyn::ComponentWrapper<SuperComponentBC, ComponentInterfaceA>>
        super_component = std::make_unique<SuperComponentABC>(99);
    // we test through the fep3::arya::IComponent, so create a pointer to such
    fep3::arya::IComponent* pointer_to_super_component = super_component.get();
    // downcast to the mock class
    const auto& mock_component_c = dynamic_cast<ComponentC*>(static_cast<ComponentInterfaceC*>(
        pointer_to_super_component->getInterface(IIDGetterC{}())));
    EXPECT_CALL(*mock_component_c, bar())
        .WillOnce(::testing::Return(1))
        .WillOnce(::testing::Return(2))
        .WillOnce(::testing::Return(3));

    const auto& component_interface_a =
        static_cast<ComponentInterfaceA*>(pointer_to_super_component->getInterface(IIDGetterA{}()));
    ASSERT_NE(nullptr, component_interface_a);
    component_interface_a->foo();

    const auto& component_interface_b =
        static_cast<ComponentInterfaceB*>(pointer_to_super_component->getInterface(IIDGetterB{}()));
    ASSERT_NE(nullptr, component_interface_b);
    EXPECT_EQ(2, component_interface_b->foo());

    const auto& component_interface_c =
        static_cast<ComponentInterfaceC*>(pointer_to_super_component->getInterface(IIDGetterC{}()));
    ASSERT_NE(nullptr, component_interface_c);
    EXPECT_EQ(3, component_interface_c->bar());
    EXPECT_EQ(99, component_interface_c->getCTORArg());
}