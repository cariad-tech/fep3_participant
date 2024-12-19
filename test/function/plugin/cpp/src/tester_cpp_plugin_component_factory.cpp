/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <fep3/components/base/mock_component.h>
#include <fep3/plugin/cpp/cpp_plugin_component_factory.h>

struct IIDGetter {
    constexpr const char* operator()() const
    {
        return "test_component.mock.fep3.iid";
    }
};

struct IIDGetterA {
    constexpr const char* operator()() const
    {
        return "test_component_a.mock.fep3.iid";
    }
};

struct IIDGetterB1 {
    constexpr const char* operator()() const
    {
        return "test_component_b1.mock.fep3.iid";
    }
};

struct IIDGetterB2 {
    constexpr const char* operator()() const
    {
        return "test_component_b2.mock.fep3.iid";
    }
};

struct IIDGetterC1 {
    constexpr const char* operator()() const
    {
        return "test_component_c1.mock.fep3.iid";
    }
};

struct IIDGetterC2 {
    constexpr const char* operator()() const
    {
        return "test_component_c2.mock.fep3.iid";
    }
};

struct IIDGetterD1 {
    constexpr const char* operator()() const
    {
        return "test_component_d1.mock.fep3.iid";
    }
};

struct IIDGetterD2 {
    constexpr const char* operator()() const
    {
        return "test_component_d2.mock.fep3.iid";
    }
};

struct IIDGetterE {
    constexpr const char* operator()() const
    {
        return "test_component_e.mock.fep3.iid";
    }
};

class ComponentInterfaceA {
public:
    FEP_COMPONENT_IID(IIDGetterA{}())
    virtual void foo() const = 0;
};

class ComponentInterfaceB1 {
public:
    FEP_COMPONENT_IID(IIDGetterB1{}())
    virtual int foo() const = 0;
};

// componentB2 is a FEP Super Component
// incremental change from B1 to B2: added method "bar"; B2 interits from B1
class ComponentInterfaceB2 : public virtual ComponentInterfaceB1 {
public:
    FEP_COMPONENT_IID(IIDGetterB2{}())
    virtual int bar() const = 0;
};

class ComponentInterfaceC1 {
public:
    FEP_COMPONENT_IID(IIDGetterC1{}())
    virtual int baz() const = 0;
};

// componentC2 is a FEP Super Component
// incremental change from C1 to C2: added methods "qux"; C2 interits from C1
class ComponentInterfaceC2 : public virtual ComponentInterfaceC1 {
public:
    FEP_COMPONENT_IID(IIDGetterC2{}())
    virtual int qux() const = 0;
};

class ComponentInterfaceD1 {
public:
    FEP_COMPONENT_IID(IIDGetterD1{}())
    virtual int fred() const = 0;
};

// componentD1/2 is a FEP Super Component
// disruptive change from D1 to D2: renamed method "fred" to "thud"; D2 does not interit from D1!
class ComponentInterfaceD2 {
public:
    FEP_COMPONENT_IID(IIDGetterD2{}())
    virtual int thud() const = 0;
};

class ComponentInterfaceE {
public:
    FEP_COMPONENT_IID(IIDGetterE{}())
    virtual int grunt() const = 0;
};

class ComponentA : public fep3::mock::arya::Component<ComponentInterfaceA> {
public:
    MOCK_METHOD(void, foo, (), (const, override));
};

class SuperComponentB
    : public fep3::mock::arya::Component<ComponentInterfaceB1, ComponentInterfaceB2> {
public:
    // mock methods of interface ComponentInterfaceB1
    MOCK_METHOD(int, foo, (), (const, override));
    // mock methods of interface ComponentInterfaceB2
    MOCK_METHOD(int, bar, (), (const, override));
};

class SuperComponentC
    : public fep3::mock::arya::Component<ComponentInterfaceC1, ComponentInterfaceC2> {
public:
    // mock methods of interface ComponentInterfaceC1
    MOCK_METHOD(int, baz, (), (const, override));
    // mock methods of interface ComponentInterfaceC2
    MOCK_METHOD(int, qux, (), (const, override));
};

class ComponentD2 : public fep3::mock::arya::Component<ComponentInterfaceD2> {
public:
    MOCK_METHOD(int, thud, (), (const, override));
};

// ComponentD2 is not a FEP Super Component of ComponentInterfaceD1/D2, but we can wrap it to make
// it appear as such
class SuperComponentD
    : public fep3::plugin::cpp::catelyn::ComponentWrapper<::testing::StrictMock<ComponentD2>,
                                                          ComponentInterfaceD1> {
public:
    // forward methods of ComponentInterfaceD1 to the wrapped component
    int fred() const override
    {
        return _component.thud();
    }
};

class ComponentE : public fep3::mock::arya::Component<ComponentInterfaceE> {
public:
    // CTOR takes an argument
    ComponentE(int /*ctor_arg*/)
    {
    }
    // mock methods of interface ComponentInterfaceE
    MOCK_METHOD(int, grunt, (), (const, override));
};

template <typename component_factory_type>
class CPPPluginComponentFactoryTester : public ::testing::Test {
};

template <template <typename...> typename component_factory_type>
struct ComponentFactoryTraitor {
    template <typename... component_descriptor_or_impl_types>
    using ComponentFactory = component_factory_type<component_descriptor_or_impl_types...>;

    // note: class template argument deduction of ComponentFactory does not work for the alias above
    //       , so it's done via a maker function
    template <typename... component_descriptor_or_impl_types>
    static auto makeComponentFactory(component_descriptor_or_impl_types&&... descriptors)
    {
        return component_factory_type<component_descriptor_or_impl_types...>(
            std::forward<component_descriptor_or_impl_types>(descriptors)...);
    }
};

using ComponentFactoryTypes =
    ::testing::Types<ComponentFactoryTraitor<fep3::plugin::cpp::arya::ComponentFactory>,
                     ComponentFactoryTraitor<fep3::plugin::cpp::catelyn::ComponentFactory>>;
TYPED_TEST_SUITE(CPPPluginComponentFactoryTester, ComponentFactoryTypes);

/**
 * Tests ComponentFactory for a single component implementation
 */
TYPED_TEST(CPPPluginComponentFactoryTester, test_createComponent)
{
    auto component_factory = typename TypeParam::template ComponentFactory<
        ::testing::StrictMock<fep3::mock::Component<fep3::mock::ComponentInterface<IIDGetter>>>>();
    const auto component = component_factory.createComponent(IIDGetter()());
    ASSERT_TRUE(component);
}

/**
 * Tests ComponentFactory for unsupported interface
 */
TYPED_TEST(CPPPluginComponentFactoryTester, test_createComponent_unsupported_iid)
{
    const auto& test_unsupported_iid = std::string("test_unsupported.arya.mock.fep3.iid");

    auto component_factory = typename TypeParam::template ComponentFactory<
        ::testing::StrictMock<fep3::mock::Component<fep3::mock::ComponentInterface<IIDGetter>>>>();
    // call to createComponent with unsupported iid must not return a component
    const auto component = component_factory.createComponent(test_unsupported_iid);
    ASSERT_FALSE(component);
}

/**
 * Tests the ComponentFactory with simple components
 */
TYPED_TEST(CPPPluginComponentFactoryTester, ComponentsFactory_Simple)
{
    // invoking the default CTOR of ComponentFactory
    auto component_factory = typename TypeParam::
        template ComponentFactory<ComponentA, SuperComponentB, SuperComponentC>();

    // create the components
    const auto& component_a = component_factory.createComponent(ComponentInterfaceA::FEP3_COMP_IID);
    const auto& component_b =
        component_factory.createComponent(ComponentInterfaceB1::FEP3_COMP_IID);
    const auto& component_c =
        component_factory.createComponent(ComponentInterfaceC1::FEP3_COMP_IID);

    // downcasts to the mock classes
    const auto& mock_component_a = dynamic_cast<ComponentA*>(static_cast<ComponentInterfaceA*>(
        component_a->getInterface(ComponentInterfaceA::FEP3_COMP_IID)));
    ASSERT_NE(nullptr, mock_component_a);
    const auto& mock_component_b =
        dynamic_cast<SuperComponentB*>(static_cast<ComponentInterfaceB1*>(
            component_b->getInterface(ComponentInterfaceB1::FEP3_COMP_IID)));
    ASSERT_NE(nullptr, mock_component_b);
    const auto& mock_component_c =
        dynamic_cast<SuperComponentC*>(static_cast<ComponentInterfaceC1*>(
            component_c->getInterface(ComponentInterfaceC1::FEP3_COMP_IID)));
    ASSERT_NE(nullptr, mock_component_c);

    EXPECT_CALL(*mock_component_a, foo()).Times(1);
    EXPECT_CALL(*mock_component_b, foo()).WillOnce(::testing::Return(1));
    EXPECT_CALL(*mock_component_c, baz()).WillOnce(::testing::Return(2));

    ASSERT_TRUE(component_a);
    const auto& component_interface_a = static_cast<ComponentInterfaceA*>(
        component_a->getInterface(ComponentInterfaceA::FEP3_COMP_IID));
    ASSERT_NE(nullptr, component_interface_a);
    component_interface_a->foo();

    ASSERT_TRUE(component_b);
    const auto& component_interface_b = static_cast<ComponentInterfaceB1*>(
        component_b->getInterface(ComponentInterfaceB1::FEP3_COMP_IID));
    ASSERT_NE(nullptr, component_interface_b);
    EXPECT_EQ(1, component_interface_b->foo());

    ASSERT_TRUE(component_c);
    const auto& component_interface_c = static_cast<ComponentInterfaceC1*>(
        component_c->getInterface(ComponentInterfaceC1::FEP3_COMP_IID));
    ASSERT_NE(nullptr, component_interface_c);
    EXPECT_EQ(2, component_interface_c->baz());
}

/**
 * Tests ComponentFactory with custom factory
 */
TYPED_TEST(CPPPluginComponentFactoryTester, ComponentFactory_CustomFactory)
{
    // invoking the non-default CTOR of fep3::plugin::cpp::arya::ComponentFactory
    // with ComponentDescriptors (making use of class template argument deduction)
    auto component_factory = TypeParam::makeComponentFactory
        // default factory, expose all supported interfaces of ComponentA
        (fep3::plugin::cpp::ComponentDescriptor<ComponentA>(),
         // default factory, expose only ComponentInterfaceC2
         fep3::plugin::cpp::ComponentDescriptor<SuperComponentC, ComponentInterfaceC2>(),
         // custom factory with CTOR arguments
         fep3::plugin::cpp::ComponentDescriptor<ComponentE>(
             [arg = 33]() { return std::make_shared<ComponentE>(arg); }));

    // create the components
    const auto& component_a = component_factory.createComponent(IIDGetterA{}());
    ASSERT_TRUE(component_a);
    // ComponentInterfaceC1 is not exposed by the component factory
    const auto& component_c1 = component_factory.createComponent(IIDGetterC1{}());
    ASSERT_FALSE(component_c1);
    const auto& component_c2 = component_factory.createComponent(IIDGetterC2{}());
    ASSERT_TRUE(component_c2);
    const auto& component_e = component_factory.createComponent(IIDGetterE{}());
    ASSERT_TRUE(component_e);

    // downcasts to the mock classes
    const auto& mock_component_a = dynamic_cast<ComponentA*>(
        static_cast<ComponentInterfaceA*>(component_a->getInterface(IIDGetterA{}())));
    ASSERT_NE(nullptr, mock_component_a);
    const auto& mock_component_c = dynamic_cast<SuperComponentC*>(
        static_cast<ComponentInterfaceC2*>(component_c2->getInterface(IIDGetterC2{}())));
    ASSERT_NE(nullptr, mock_component_c);
    const auto& mock_component_e = dynamic_cast<ComponentE*>(
        static_cast<ComponentInterfaceE*>(component_e->getInterface(IIDGetterE{}())));
    ASSERT_NE(nullptr, mock_component_e);

    EXPECT_CALL(*mock_component_a, foo()).Times(1);
    EXPECT_CALL(*mock_component_c, qux()).WillOnce(::testing::Return(6));
    EXPECT_CALL(*mock_component_e, grunt()).WillOnce(::testing::Return(7));

    const auto& component_interface_a =
        static_cast<ComponentInterfaceA*>(component_a->getInterface(IIDGetterA{}()));
    ASSERT_NE(nullptr, component_interface_a);
    component_interface_a->foo();

    const auto& component_interface_c2 =
        static_cast<ComponentInterfaceC2*>(component_c2->getInterface(IIDGetterC2{}()));
    ASSERT_NE(nullptr, component_interface_c2);
    EXPECT_EQ(6, component_interface_c2->qux());

    const auto& component_interface_e =
        static_cast<ComponentInterfaceE*>(component_e->getInterface(IIDGetterE{}()));
    ASSERT_NE(nullptr, component_interface_e);
    EXPECT_EQ(7, component_interface_e->grunt());
}

/**
 * Tests @ref fep3::plugin::cpp::arya::ComponentFactory with FEP Super Components
 */
TEST(arya_CPPPluginComponentFactoryTester, ComponentFactory_SuperComponents)
{
    // invoking the default CTOR of fep3::plugin::cpp::arya::ComponentFactory
    // with mixed template arguments (implementation type and ComponentDescriptor)
    auto component_factory = fep3::plugin::cpp::arya::ComponentFactory<
        ComponentA, // expose all supported interfaces of ComponentA
        // expose all supported interfaces of ComponentB
        fep3::plugin::cpp::ComponentDescriptor<SuperComponentB>,
        // expose only ComponentInterfaceC2
        fep3::plugin::cpp::ComponentDescriptor<SuperComponentC, ComponentInterfaceC2>,
        SuperComponentD // expose all supported interfaces of SuperComponentD
        >();

    // create the components
    const auto& component_a = component_factory.createComponent(IIDGetterA{}());
    ASSERT_TRUE(component_a);
    const auto& component_b1 = component_factory.createComponent(IIDGetterB1{}());
    ASSERT_TRUE(component_b1);
    const auto& component_b2 = component_factory.createComponent(IIDGetterB2{}());
    ASSERT_TRUE(component_b2);
    // SuperComponentB is a FEP Super Component, so the pointers of the underlying component must be
    // identical
    const auto& component_impl_b1 = dynamic_cast<SuperComponentB*>(
        static_cast<ComponentInterfaceB1*>(component_b1->getInterface(IIDGetterB1{}())));
    const auto& component_impl_b2 = dynamic_cast<SuperComponentB*>(
        static_cast<ComponentInterfaceB2*>(component_b2->getInterface(IIDGetterB2{}())));
    EXPECT_EQ(component_impl_b1, component_impl_b2);
    // ComponentInterfaceC1 is not exposed by the component factory
    const auto& component_c1 = component_factory.createComponent(IIDGetterC1{}());
    ASSERT_FALSE(component_c1);
    const auto& component_c2 = component_factory.createComponent(IIDGetterC2{}());
    ASSERT_TRUE(component_c2);
    const auto& component_d1 = component_factory.createComponent(IIDGetterD1{}());
    ASSERT_TRUE(component_d1);
    const auto& component_d2 = component_factory.createComponent(IIDGetterD2{}());
    ASSERT_TRUE(component_d2);
    // SuperComponentD is a FEP Super Component implemented by a
    // fep3::plugin::cpp::catelyn::ComponentWrapper ...
    const auto super_component_wrapper_d = dynamic_cast<SuperComponentD*>(
        static_cast<ComponentInterfaceD1*>(component_d1->getInterface(IIDGetterD1{}())));
    // ... so from the wrapper we can get pointer to the underlying component ...
    const auto component_impl_d1 = dynamic_cast<ComponentD2*>(static_cast<ComponentInterfaceD2*>(
        super_component_wrapper_d->getInterface(IIDGetterD2{}())));
    // ... which must be identical to the pointer of the underlying component
    const auto& component_impl_d2 = dynamic_cast<ComponentD2*>(
        static_cast<ComponentInterfaceD2*>(component_d2->getInterface(IIDGetterD2{}())));
    EXPECT_EQ(component_impl_d1, component_impl_d2);

    // downcasts to the mock classes
    const auto& mock_component_a = dynamic_cast<ComponentA*>(
        static_cast<ComponentInterfaceA*>(component_a->getInterface(IIDGetterA{}())));
    ASSERT_NE(nullptr, mock_component_a);
    const auto& mock_component_b = dynamic_cast<SuperComponentB*>(
        static_cast<ComponentInterfaceB1*>(component_b1->getInterface(IIDGetterB1{}())));
    ASSERT_NE(nullptr, mock_component_b);
    const auto& mock_component_c = dynamic_cast<SuperComponentC*>(
        static_cast<ComponentInterfaceC2*>(component_c2->getInterface(IIDGetterC2{}())));
    ASSERT_NE(nullptr, mock_component_c);
    const auto& mock_component_d = dynamic_cast<ComponentD2*>(
        static_cast<ComponentInterfaceD2*>(component_d2->getInterface(IIDGetterD2{}())));
    ASSERT_NE(nullptr, mock_component_d);

    EXPECT_CALL(*mock_component_a, foo()).Times(1);
    EXPECT_CALL(*mock_component_b, foo()).WillOnce(::testing::Return(1));
    EXPECT_CALL(*mock_component_b, bar()).WillOnce(::testing::Return(2));
    EXPECT_CALL(*mock_component_c, qux()).WillOnce(::testing::Return(3));
    EXPECT_CALL(*mock_component_d, thud())
        .WillOnce(::testing::Return(4))
        .WillOnce(::testing::Return(5));

    const auto& component_interface_a =
        static_cast<ComponentInterfaceA*>(component_a->getInterface(IIDGetterA{}()));
    ASSERT_NE(nullptr, component_interface_a);
    component_interface_a->foo();

    const auto& component_interface_b1 =
        static_cast<ComponentInterfaceB1*>(component_b1->getInterface(IIDGetterB1{}()));
    ASSERT_NE(nullptr, component_interface_b1);
    EXPECT_EQ(1, component_interface_b1->foo());

    const auto& component_interface_b2 =
        static_cast<ComponentInterfaceB2*>(component_b2->getInterface(IIDGetterB2{}()));
    ASSERT_NE(nullptr, component_interface_b2);
    EXPECT_EQ(2, component_interface_b2->bar());

    const auto& component_interface_c2 =
        static_cast<ComponentInterfaceC2*>(component_c2->getInterface(IIDGetterC2{}()));
    ASSERT_NE(nullptr, component_interface_c2);
    EXPECT_EQ(3, component_interface_c2->qux());

    const auto& component_interface_d1 =
        static_cast<ComponentInterfaceD1*>(component_d1->getInterface(IIDGetterD1{}()));
    ASSERT_NE(nullptr, component_interface_d1);
    EXPECT_EQ(4, component_interface_d1->fred());

    const auto& component_interface_d2 =
        static_cast<ComponentInterfaceD2*>(component_d2->getInterface(IIDGetterD2{}()));
    ASSERT_NE(nullptr, component_interface_d2);
    EXPECT_EQ(5, component_interface_d2->thud());
}

/**
 * Tests @ref fep3::plugin::cpp::catelyn::ComponentFactory with FEP Super Components
 */
TEST(catelyn_CPPPluginComponentFactoryTester, ComponentFactory_SuperComponents)
{
    // invoking the default CTOR of fep3::plugin::cpp::catelyn::ComponentFactory
    // with mixed template arguments (implementation type and ComponentDescriptor)
    auto component_factory = fep3::plugin::cpp::catelyn::ComponentFactory<
        ComponentA, // expose all supported interfaces of ComponentA
        // expose all supported interfaces of ComponentB
        fep3::plugin::cpp::ComponentDescriptor<SuperComponentB>,
        // expose only ComponentInterfaceC2
        fep3::plugin::cpp::ComponentDescriptor<SuperComponentC, ComponentInterfaceC2>,
        SuperComponentD // expose all supported interfaces of SuperComponentD
        >();

    // create the components
    const auto& component_a = component_factory.createComponent(IIDGetterA{}());
    ASSERT_TRUE(component_a);
    const auto& component_b1 = component_factory.createComponent(IIDGetterB1{}());
    ASSERT_TRUE(component_b1);
    const auto& component_b2 = component_factory.createComponent(IIDGetterB2{}());
    ASSERT_TRUE(component_b2);
    // component B is a FEP Super Component, so the pointers must be identical
    EXPECT_EQ(component_b1.get(), component_b2.get());
    // ComponentInterfaceC1 is not exposed by the component factory
    const auto& component_c1 = component_factory.createComponent(IIDGetterC1{}());
    ASSERT_FALSE(component_c1);
    const auto& component_c2 = component_factory.createComponent(IIDGetterC2{}());
    ASSERT_TRUE(component_c2);
    const auto& component_d1 = component_factory.createComponent(IIDGetterD1{}());
    ASSERT_TRUE(component_d1);
    const auto& component_d2 = component_factory.createComponent(IIDGetterD2{}());
    ASSERT_TRUE(component_d2);
    // component D is a FEP Super Component, so the pointers must be identical
    EXPECT_EQ(component_d1.get(), component_d2.get());

    // downcasts to the mock classes
    const auto& mock_component_a = dynamic_cast<ComponentA*>(
        static_cast<ComponentInterfaceA*>(component_a->getInterface(IIDGetterA{}())));
    ASSERT_NE(nullptr, mock_component_a);
    const auto& mock_component_b = dynamic_cast<SuperComponentB*>(
        static_cast<ComponentInterfaceB1*>(component_b1->getInterface(IIDGetterB1{}())));
    ASSERT_NE(nullptr, mock_component_b);
    const auto& mock_component_c = dynamic_cast<SuperComponentC*>(
        static_cast<ComponentInterfaceC2*>(component_c2->getInterface(IIDGetterC2{}())));
    ASSERT_NE(nullptr, mock_component_c);
    const auto& mock_component_d = dynamic_cast<ComponentD2*>(
        static_cast<ComponentInterfaceD2*>(component_d2->getInterface(IIDGetterD2{}())));
    ASSERT_NE(nullptr, mock_component_d);

    EXPECT_CALL(*mock_component_a, foo()).Times(1);
    EXPECT_CALL(*mock_component_b, foo()).WillOnce(::testing::Return(1));
    EXPECT_CALL(*mock_component_b, bar()).WillOnce(::testing::Return(2));
    EXPECT_CALL(*mock_component_c, qux()).WillOnce(::testing::Return(3));
    EXPECT_CALL(*mock_component_d, thud())
        .WillOnce(::testing::Return(4))
        .WillOnce(::testing::Return(5));

    const auto& component_interface_a =
        static_cast<ComponentInterfaceA*>(component_a->getInterface(IIDGetterA{}()));
    ASSERT_NE(nullptr, component_interface_a);
    component_interface_a->foo();

    const auto& component_interface_b1 =
        static_cast<ComponentInterfaceB1*>(component_b1->getInterface(IIDGetterB1{}()));
    ASSERT_NE(nullptr, component_interface_b1);
    EXPECT_EQ(1, component_interface_b1->foo());

    const auto& component_interface_b2 =
        static_cast<ComponentInterfaceB2*>(component_b2->getInterface(IIDGetterB2{}()));
    ASSERT_NE(nullptr, component_interface_b2);
    EXPECT_EQ(2, component_interface_b2->bar());

    const auto& component_interface_c2 =
        static_cast<ComponentInterfaceC2*>(component_c2->getInterface(IIDGetterC2{}()));
    ASSERT_NE(nullptr, component_interface_c2);
    EXPECT_EQ(3, component_interface_c2->qux());

    const auto& component_interface_d1 =
        static_cast<ComponentInterfaceD1*>(component_d1->getInterface(IIDGetterD1{}()));
    ASSERT_NE(nullptr, component_interface_d1);
    EXPECT_EQ(4, component_interface_d1->fred());

    const auto& component_interface_d2 =
        static_cast<ComponentInterfaceD2*>(component_d2->getInterface(IIDGetterD2{}()));
    ASSERT_NE(nullptr, component_interface_d2);
    EXPECT_EQ(5, component_interface_d2->thud());
}
