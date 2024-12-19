/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <fep3/base/properties/propertynode.h>
#include <fep3/components/clock/mock_clock_service.h>
#include <fep3/components/configuration/configuration_service_intf.h>
#include <fep3/components/data_registry/mock_data_registry.h>
#include <fep3/components/job_registry/job_registry_intf.h>
#include <fep3/components/logging/logging_service_intf.h>
#include <fep3/components/simulation_bus/mock_simulation_bus.h>
#include <fep3/core.h>
#include <fep3/participant/mock/mock_custom_job_element.h>
#include <fep3/participant/mock/mock_default_job.h>
#include <fep3/participant/mock/stub_default_job.h>
#include <fep3/participant/state_machine/participant_state_machine.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <common/gtest_asserts.h>
#include <convenient_component_registry.h>
#include <helper/job_registry_helper.h>

using namespace testing;

class TestElement : public fep3::mock::CustomJobElement {
public:
    TestElement(std::shared_ptr<fep3::stub::DefaultJob> default_job,
                std::function<void(TestElement* leak_pointer)> constructor_call)
        : CustomJobElement("TestElement"), _default_job(default_job)
    {
        constructor_call(this);
        using namespace testing;
        ON_CALL(*this, createJobMock).WillByDefault(Return(fep3::Result{}));
        ON_CALL(*this, destroyJob).WillByDefault(Return(fep3::Result{}));
        ON_CALL(*this, load).WillByDefault(Return(fep3::Result{}));
        ON_CALL(*this, unload);
        ON_CALL(*this, initialize).WillByDefault(Return(fep3::Result{}));
        ON_CALL(*this, deinitialize);
        ON_CALL(*this, run).WillByDefault(Return(fep3::Result{}));
        ON_CALL(*this, stop);
    }

    std::string getTypename() const override
    {
        return "Demo Element Base Sender Type";
    }
    std::string getVersion() const override
    {
        return FEP3_PARTICIPANT_LIBRARY_VERSION_STR;
    }

    std::tuple<fep3::Result, JobPtr, JobConfigPtr> createJob(
        const fep3::arya::IComponents&) override
    {
        return {createJobMock(), _default_job, _job_builder.makeClockJobConfigPtr()};
    }
    MOCK_METHOD(fep3::Result, createJobMock, (), ());

private:
    fep3::test::helper::SimpleJobBuilder _job_builder;
    std::shared_ptr<fep3::stub::DefaultJob> _default_job;
};

struct DefaultJobElementUnwindingTest : public Test {
    struct Factory : public fep3::base::IElementFactory {
        Factory(std::shared_ptr<fep3::stub::DefaultJob> default_job,
                std::function<void(TestElement* leak_pointer)> constructor_call)
            : _default_job(default_job), _constructor_call(constructor_call)
        {
        }
        std::unique_ptr<fep3::base::IElement> createElement(const fep3::IComponents&) const override
        {
            return std::make_unique<
                fep3::core::DefaultJobElement<::testing::NiceMock<TestElement>>>(
                std::make_unique<::testing::NiceMock<TestElement>>(_default_job,
                                                                   _constructor_call));
        }

    private:
        std::shared_ptr<fep3::stub::DefaultJob> _default_job;
        std::function<void(TestElement* leak_pointer)> _constructor_call;
    };
    DefaultJobElementUnwindingTest()
    {
        _default_job = std::make_shared<::testing::NiceMock<fep3::stub::DefaultJob>>();
        initRegistry();
    }

    void TearDown() override
    {
        _default_job.reset();
        _registry.reset();
    }

    fep3::native::ParticipantStateMachine getStateMachine()
    {
        return fep3::native::ParticipantStateMachine(
            fep3::ElementManager(std::make_unique<Factory>(_default_job,
                                                           [&](TestElement* leak_pointer) {
                                                               _leak_pointer = leak_pointer;
                                                               _set_elem_load_expectations();
                                                           })),
            _registry->getRegistry(),
            nullptr);
    }

    void initRegistry()
    {
        _registry = std::make_unique<fep3::test::ComponentRegistry>(
            std::vector<std::string>{fep3::IConfigurationService::FEP3_COMP_IID,
                                     fep3::ILoggingService::FEP3_COMP_IID,
                                     fep3::IJobRegistry::FEP3_COMP_IID},
            std::vector<std::string>{fep3::IDataRegistry::FEP3_COMP_IID,
                                     fep3::ISimulationBus::FEP3_COMP_IID,
                                     fep3::arya::IClockService::FEP3_COMP_IID});
    }

    std::unique_ptr<fep3::test::ComponentRegistry> _registry;
    std::shared_ptr<fep3::stub::DefaultJob> _default_job;
    TestElement* _leak_pointer;
    std::function<void()> _set_elem_load_expectations = []() {};
};

TEST_F(DefaultJobElementUnwindingTest, load_initLoggerDefaultJobElement)
{
    auto sm = getStateMachine();
    _registry->unregisterComponent(fep3::ILoggingService::FEP3_COMP_IID);
    ASSERT_NE(sm.load(), fep3::Result(0));
    _registry->addNativeComponentToRegistry(fep3::ILoggingService::FEP3_COMP_IID);
    ASSERT_FEP3_NOERROR(sm.load());
}

TEST_F(DefaultJobElementUnwindingTest, load_initConfigurationCustomJobElement)
{
    auto sm = getStateMachine();
    auto config_service = _registry->getRegistry()->getComponent<fep3::IConfigurationService>();
    config_service->registerNode(std::make_shared<fep3::base::NativePropertyNode>("TestElement"));

    ASSERT_NE(sm.load(), fep3::Result(0));

    config_service->unregisterNode("TestElement");
    ASSERT_FEP3_NOERROR(sm.load());
}

TEST_F(DefaultJobElementUnwindingTest, load_initConfigurationCustomJob)
{
    auto sm = getStateMachine();
    auto config_service = _registry->getRegistry()->getComponent<fep3::IConfigurationService>();
    config_service->registerNode(
        std::make_shared<fep3::base::NativePropertyNode>(fep3::stub::stub_default_job_name));

    ASSERT_NE(sm.load(), fep3::Result(0));

    config_service->unregisterNode(fep3::stub::stub_default_job_name);
    ASSERT_FEP3_NOERROR(sm.load());
}

TEST_F(DefaultJobElementUnwindingTest, load_addJob)
{
    auto sm = getStateMachine();
    auto job_registry = _registry->getRegistry()->getComponent<fep3::IJobRegistry>();

    fep3::test::helper::SimpleJobBuilder job_builder;
    job_registry->addJob(fep3::stub::stub_default_job_name,
                         std::make_shared<fep3::stub::DefaultJob>(),
                         job_builder.makeClockJobConfig());

    ASSERT_NE(sm.load(), fep3::Result(0));

    job_registry->removeJob(fep3::stub::stub_default_job_name);
    ASSERT_FEP3_NOERROR(sm.load());
}

TEST_F(DefaultJobElementUnwindingTest, load_createJob)
{
    const fep3::Result job_creation_res = fep3::Result{-39};
    auto sm = getStateMachine();

    _set_elem_load_expectations = [&]() {
        EXPECT_CALL(*_leak_pointer, createJobMock).WillOnce(Return(job_creation_res));
    };

    ASSERT_FEP3_RESULT(sm.load(), job_creation_res);

    _set_elem_load_expectations = [&]() {
        EXPECT_CALL(*_leak_pointer, createJobMock).WillOnce(DoDefault());
    };
    ASSERT_FEP3_NOERROR(sm.load());
}

TEST_F(DefaultJobElementUnwindingTest, load_loadElement)
{
    const fep3::Result elem_transition_result = fep3::Result{-4};
    auto sm = getStateMachine();

    _set_elem_load_expectations = [&]() {
        EXPECT_CALL(*_leak_pointer, load).WillOnce(Return(elem_transition_result));
    };

    ASSERT_FEP3_RESULT(sm.load(), elem_transition_result);

    _set_elem_load_expectations = [&]() {
        EXPECT_CALL(*_leak_pointer, load).WillOnce(DoDefault());
    };
    ASSERT_FEP3_NOERROR(sm.load());
}

TEST_F(DefaultJobElementUnwindingTest, initialize_registerDataIO)
{
    auto sm = getStateMachine();
    sm.load();
    _registry->unregisterComponent(fep3::IJobRegistry::FEP3_COMP_IID);
    ASSERT_NE(sm.initialize(), fep3::Result{0});
    _registry->addNativeComponentToRegistry(fep3::IJobRegistry::FEP3_COMP_IID);
    ASSERT_FEP3_NOERROR(sm.initialize());
}

TEST_F(DefaultJobElementUnwindingTest, initialize_addJobIOsToDataRegistry)
{
    auto sm = getStateMachine();
    ASSERT_FEP3_RESULT_TEST_ARRANGE(sm.load());
    _registry->unregisterComponent(fep3::IDataRegistry::FEP3_COMP_IID);
    ASSERT_NE(sm.initialize(), fep3::Result{0});
    _registry->addStubComponentToRegistry(fep3::IDataRegistry::FEP3_COMP_IID);
    ASSERT_FEP3_NOERROR(sm.initialize());
}

TEST_F(DefaultJobElementUnwindingTest, initialize_jobInitialize)
{
    const fep3::Result job_init_res = fep3::Result{-51};

    auto sm = getStateMachine();
    ASSERT_FEP3_RESULT_TEST_ARRANGE(sm.load());
    {
        InSequence s;
        EXPECT_CALL(*_default_job, initialize).WillOnce(Return(job_init_res));
        EXPECT_CALL(*_default_job, initialize).WillOnce(DoDefault());

        ASSERT_EQ(sm.initialize(), job_init_res);
        ASSERT_FEP3_NOERROR(sm.initialize());
    }
}

TEST_F(DefaultJobElementUnwindingTest, initialize_elemInitialize)
{
    const fep3::Result elem_init_result = fep3::Result{-23};

    auto sm = getStateMachine();
    ASSERT_FEP3_RESULT_TEST_ARRANGE(sm.load());

    {
        InSequence s;
        EXPECT_CALL(*_default_job, initialize).WillOnce(DoDefault());
        EXPECT_CALL(*_leak_pointer, initialize).WillOnce(Return(elem_init_result));
        EXPECT_CALL(*_default_job, deinitialize);
        ASSERT_EQ(sm.initialize(), elem_init_result);
    }

    {
        InSequence s;
        EXPECT_CALL(*_default_job, initialize).WillOnce(DoDefault());
        EXPECT_CALL(*_leak_pointer, initialize).WillOnce(DoDefault());

        ASSERT_FEP3_NOERROR(sm.initialize());
    }
}

TEST_F(DefaultJobElementUnwindingTest, start_elemRun)
{
    const fep3::Result run_element_result = fep3::Result{-8};
    auto sm = getStateMachine();
    ASSERT_FEP3_RESULT_TEST_ARRANGE(sm.load());
    ASSERT_FEP3_RESULT_TEST_ARRANGE(sm.initialize());
    {
        InSequence s;
        EXPECT_CALL(*_default_job, start).WillOnce(DoDefault());
        EXPECT_CALL(*_leak_pointer, run).WillOnce(Return(run_element_result));
        EXPECT_CALL(*_default_job, stop).WillOnce(DoDefault());
        ASSERT_EQ(sm.start(), run_element_result);

        EXPECT_CALL(*_default_job, start).WillOnce(DoDefault());
        EXPECT_CALL(*_leak_pointer, run).WillOnce(DoDefault());
        ASSERT_FEP3_NOERROR(sm.start());
    }
}

TEST_F(DefaultJobElementUnwindingTest, start_jobRun)
{
    const fep3::Result start_job_result = fep3::Result{-6};
    auto sm = getStateMachine();
    ASSERT_FEP3_RESULT_TEST_ARRANGE(sm.load());
    ASSERT_FEP3_RESULT_TEST_ARRANGE(sm.initialize());
    {
        InSequence s;
        EXPECT_CALL(*_default_job, start).WillOnce(Return(start_job_result));
        ASSERT_EQ(sm.start(), start_job_result);

        EXPECT_CALL(*_default_job, start).WillOnce(DoDefault());
        EXPECT_CALL(*_leak_pointer, run).WillOnce(DoDefault());
        ASSERT_FEP3_NOERROR(sm.start());
    }
}
