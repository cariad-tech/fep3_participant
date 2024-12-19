/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <fep3/components/base/component.h>
#include <fep3/components/base/component_iid.h>
#include <fep3/participant/component_registry_rpc/component_registry_rpc.h>

#include <a_util/strings/strings_functions.h>

#include <gmock/gmock.h>

#include <component_registry_rpc_service_helper.h>

using namespace ::testing;

class IMockInterface {
protected:
    // we dont want not be deleted thru the interface!!
    virtual ~IMockInterface() = default;

public:
    FEP_COMPONENT_IID("IMockInterface")

public:
    virtual int32_t get1() const = 0;
    virtual void set1(int32_t value) = 0;
};

class MockComponent1 : public fep3::base::Component<IMockInterface> {
public:
    MOCK_CONST_METHOD0(get1, int32_t());
    MOCK_METHOD1(set1, void(int32_t));

    // methods of base class base::ComponentImpl
    MOCK_METHOD0(create, fep3::Result());
    MOCK_METHOD0(destroy, fep3::Result());
    MOCK_METHOD0(initialize, fep3::Result());
    MOCK_METHOD0(tense, fep3::Result());
    MOCK_METHOD0(relax, fep3::Result());
    MOCK_METHOD0(deinitialize, fep3::Result());
    MOCK_METHOD0(start, fep3::Result());
    MOCK_METHOD0(stop, fep3::Result());
    MOCK_METHOD0(pause, fep3::Result());
};

class IMockInterface2 {
protected:
    // we dont want not be deleted thru the interface!!
    virtual ~IMockInterface2() = default;

public:
    FEP_COMPONENT_IID("IMockInterface2")

public:
    virtual int32_t get1() const = 0;
    virtual void set1(int32_t value) = 0;
};

class MockComponent2 : public fep3::base::Component<IMockInterface2> {
public:
    MOCK_CONST_METHOD0(get1, int32_t());
    MOCK_METHOD1(set1, void(int32_t));

    // methods of base class base::ComponentImpl
    MOCK_METHOD0(create, fep3::Result());
    MOCK_METHOD0(destroy, fep3::Result());
    MOCK_METHOD0(initialize, fep3::Result());
    MOCK_METHOD0(tense, fep3::Result());
    MOCK_METHOD0(relax, fep3::Result());
    MOCK_METHOD0(deinitialize, fep3::Result());
    MOCK_METHOD0(start, fep3::Result());
    MOCK_METHOD0(stop, fep3::Result());
    MOCK_METHOD0(pause, fep3::Result());
};

struct TestComponentRegistryRPC : public Test {
    TestComponentRegistryRPC()
        : _component_registry(std::make_shared<fep3::ComponentRegistry>()),
          _component_registry_rpc_service(_component_registry)
    {
    }

    void SetUp() override
    {
        _component_registry->registerComponent<IMockInterface>(
            std::make_unique<::testing::StrictMock<MockComponent1>>(),
            _dummy_component_version_info);
    }

    std::shared_ptr<fep3::ComponentRegistry> _component_registry{};
    // we do not use the macros here to be sure the give the correct string back
    const fep3::ComponentVersionInfo _dummy_component_version_info{"3.0.1", "dummyPath", "3.1.0"};
    fep3::native::ComponentRegistryRpcService _component_registry_rpc_service;
};

TEST_F(TestComponentRegistryRPC, getExististingComponentPluginVersion)
{
    auto json_value =
        _component_registry_rpc_service.getPluginVersion(IMockInterface::getComponentIID());

    ASSERT_NO_FATAL_FAILURE(fep3::test::helper::validateJsonKey(
        json_value, "version", _dummy_component_version_info.getVersion()))
        << "Json return value not as expected";
}

TEST_F(TestComponentRegistryRPC, getExististingComponentPluginPath)
{
    auto json_value =
        _component_registry_rpc_service.getFilePath(IMockInterface::getComponentIID());

    ASSERT_NO_FATAL_FAILURE(fep3::test::helper::validateJsonKey(
        json_value, "file_path", _dummy_component_version_info.getFilePath()))
        << "Json return value not as expected";
}

TEST_F(TestComponentRegistryRPC, getExististingComponentParticipantLibraryVersion)
{
    auto json_value = _component_registry_rpc_service.getParticipantLibraryVersion(
        IMockInterface::getComponentIID());

    ASSERT_NO_FATAL_FAILURE(fep3::test::helper::validateJsonKey(
        json_value,
        "participant_version",
        _dummy_component_version_info.getParticipantLibraryVersion()))
        << "Json return value not as expected";
}

TEST_F(TestComponentRegistryRPC, getNonExististingComponent)
{
    auto json_value = _component_registry_rpc_service.getPluginVersion("nonExistingComponent");
    ASSERT_NO_FATAL_FAILURE(fep3::test::helper::validateJsorError(json_value))
        << "Json return value not as expected";
}

TEST_F(TestComponentRegistryRPC, getComponentIIDs)
{
    _component_registry->registerComponent<MockComponent2>(
        std::make_unique<::testing::StrictMock<MockComponent2>>(), _dummy_component_version_info);

    auto json_value = _component_registry_rpc_service.getComponentIIDs();
    ASSERT_TRUE(json_value.isMember("component_iids"));

    const std::string component_iids_csv = json_value["component_iids"].asString();
    const std::vector<std::string> component_iids = a_util::strings::split(component_iids_csv, ",");

    ASSERT_EQ(component_iids.size(), 2);
    ASSERT_THAT(component_iids, ::testing::Contains("IMockInterface"));
    ASSERT_THAT(component_iids, ::testing::Contains("IMockInterface2"));
}
