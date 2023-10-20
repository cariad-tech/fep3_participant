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

#include <fep3/components/base/mock_component.h>
#include <fep3/components/logging/mock/mock_logger_addons.h>
#include <fep3/components/logging/mock_logging_service.h>
#include <fep3/fep3_participant_version.h>
#include <fep3/participant/mock/mock_element_base.h>
#include <fep3/participant/state_machine/participant_state_machine.h>

#include <gtest_asserts.h>

using namespace ::testing;

using LoggingServiceComponent = NiceMock<fep3::mock::LoggingService>;
using Logger = NiceMock<fep3::mock::LoggerWithDefaultBehaviour>;

/**
 * Test the state machine, when it has no element manager set
 *         -> must be unable to load
 * @req_id // TODO reference the REQUIREMENT as soon it has been created
 */
TEST(BaseParticipantStateMachineTester, testNoElementManager)
{
    // dont't set the element manager / component registry in this test
    ::fep3::native::ParticipantStateMachine state_machine({}, {}, {});
    // the state machine must immediately enter the state "Unloaded"
    EXPECT_FALSE(state_machine.isFinalized());
    EXPECT_EQ("Unloaded", state_machine.getCurrentStateName());

    // loading must fail because no element manager was set
    EXPECT_FALSE(state_machine.load());
    // after failed loading, the state must still be "Unloaded"
    EXPECT_EQ("Unloaded", state_machine.getCurrentStateName());

    // all other events (except exit) must fail
    EXPECT_FALSE(state_machine.unload());
    EXPECT_FALSE(state_machine.initialize());
    EXPECT_FALSE(state_machine.deinitialize());
    EXPECT_FALSE(state_machine.start());
    EXPECT_FALSE(state_machine.stop());
    EXPECT_FALSE(state_machine.pause());

    // exit must succeed
    EXPECT_TRUE(state_machine.exit());
    ASSERT_TRUE(state_machine.isFinalized());
    // state machine has finalized, so the current state name is unknown
    EXPECT_EQ("Finalized", state_machine.getCurrentStateName());
}

class TestElementFactory : public ::fep3::base::IElementFactory {
public:
    TestElementFactory(const std::shared_ptr<
                       std::unique_ptr<::testing::StrictMock<::fep3::mock::MockElementBase>>>&
                           test_element_wrapper)
        : _test_element_wrapper(test_element_wrapper)
    {
    }
    std::unique_ptr<::fep3::base::IElement> createElement(const ::fep3::IComponents&) const override
    {
        if (_test_element_wrapper) {
            auto& unique_ptr_to_element = *(_test_element_wrapper.get());
            return std::move(unique_ptr_to_element);
        }
        else {
            return {};
        }
    }

private:
    std::shared_ptr<std::unique_ptr<::testing::StrictMock<::fep3::mock::MockElementBase>>>
        _test_element_wrapper;
};

/**
 * Test the state machine in normal operation (i. e. no error)
 *  * step through all states
 *  * in each state check that
 *  * all events, that don't trigger a transition (uncommented below), are not handled
 * @req_id // TODO reference the REQUIREMENT as soon it has been created
 */
TEST(BaseParticipantStateMachineTester, testNormalOperation)
{
    // wrapping unique_ptr into shared_ptr to be able to capture it in the lambda for the element
    // factory (see below)
    auto test_element_wrapper =
        std::make_shared<std::unique_ptr<::testing::StrictMock<::fep3::mock::MockElementBase>>>(
            std::make_unique<::testing::StrictMock<::fep3::mock::MockElementBase>>());
    {
        auto& test_element = *(test_element_wrapper.get()->get());
        ::testing::InSequence call_sequence;
        EXPECT_CALL(test_element, load()).WillOnce(::testing::Return(::fep3::Result{}));
        EXPECT_CALL(test_element, initialize()).WillOnce(::testing::Return(::fep3::Result{}));
        EXPECT_CALL(test_element, run()).WillOnce(::testing::Return(::fep3::Result{}));
        EXPECT_CALL(test_element, stop()).WillOnce(::testing::Return());
        EXPECT_CALL(test_element, deinitialize()).WillOnce(::testing::Return());
        EXPECT_CALL(test_element, unload()).Times(1);
        EXPECT_DESTRUCTION(test_element);
    }

    const auto& component_registry = std::make_shared<::fep3::ComponentRegistry>();
    auto logger = std::make_shared<Logger>();
    auto logging_service_mock = std::make_shared<LoggingServiceComponent>();
    EXPECT_CALL(*logging_service_mock, createLogger(_)).WillRepeatedly(Return(logger));
    ASSERT_FEP3_NOERROR(component_registry->registerComponent<fep3::ILoggingService>(
        logging_service_mock,
        fep3::ComponentVersionInfo{FEP3_PARTICIPANT_LIBRARY_VERSION_STR,
                                   "dummyPath",
                                   FEP3_PARTICIPANT_LIBRARY_VERSION_STR}));

    ::fep3::native::ParticipantStateMachine state_machine(
        ::fep3::arya::ElementManager(std::make_shared<TestElementFactory>(test_element_wrapper)),
        component_registry,
        logger);
    // the state machine must immediately enter the state "Unloaded"
    EXPECT_EQ("Unloaded", state_machine.getCurrentStateName());
    // EXPECT_FALSE(state_machine.load());
    EXPECT_FALSE(state_machine.unload());
    EXPECT_FALSE(state_machine.initialize());
    EXPECT_FALSE(state_machine.deinitialize());
    EXPECT_FALSE(state_machine.start());
    EXPECT_FALSE(state_machine.pause());
    EXPECT_FALSE(state_machine.stop());
    // EXPECT_FALSE(state_machine.exit());

    // switch to Loaded
    EXPECT_TRUE(state_machine.load());
    EXPECT_EQ("Loaded", state_machine.getCurrentStateName());
    EXPECT_FALSE(state_machine.load());
    // EXPECT_FALSE(state_machine.unload());
    // EXPECT_FALSE(state_machine.initialize());
    EXPECT_FALSE(state_machine.deinitialize());
    EXPECT_FALSE(state_machine.start());
    EXPECT_FALSE(state_machine.pause());
    EXPECT_FALSE(state_machine.stop());
    EXPECT_FALSE(state_machine.exit());

    // switch to Initialized
    EXPECT_TRUE(state_machine.initialize());
    EXPECT_EQ("Initialized", state_machine.getCurrentStateName());
    EXPECT_FALSE(state_machine.load());
    EXPECT_FALSE(state_machine.unload());
    EXPECT_FALSE(state_machine.initialize());
    // EXPECT_FALSE(state_machine.deinitialize());
    // EXPECT_FALSE(state_machine.start());
    // EXPECT_FALSE(state_machine.pause());
    EXPECT_FALSE(state_machine.stop());
    EXPECT_FALSE(state_machine.exit());

    // switch to Running
    EXPECT_TRUE(state_machine.start());
    EXPECT_EQ("Running", state_machine.getCurrentStateName());
    EXPECT_FALSE(state_machine.load());
    EXPECT_FALSE(state_machine.unload());
    EXPECT_FALSE(state_machine.initialize());
    EXPECT_FALSE(state_machine.deinitialize());
    EXPECT_FALSE(state_machine.start());
    // EXPECT_FALSE(state_machine.pause());
    // EXPECT_FALSE(state_machine.stop());
    EXPECT_FALSE(state_machine.exit());

    // switch to Paused
    // component registry does not implement 'pause' functionality yet (see FEPSDK-2766)
    EXPECT_FALSE(state_machine.pause());
    EXPECT_EQ("Running", state_machine.getCurrentStateName());
    EXPECT_FALSE(state_machine.load());
    EXPECT_FALSE(state_machine.unload());
    EXPECT_FALSE(state_machine.initialize());
    EXPECT_FALSE(state_machine.deinitialize());
    // EXPECT_FALSE(state_machine.start());
    EXPECT_FALSE(state_machine.pause());
    // EXPECT_FALSE(state_machine.stop());
    EXPECT_FALSE(state_machine.exit());

    // switch back to Initialized
    EXPECT_TRUE(state_machine.stop());
    EXPECT_EQ("Initialized", state_machine.getCurrentStateName());
    EXPECT_FALSE(state_machine.load());
    EXPECT_FALSE(state_machine.unload());
    EXPECT_FALSE(state_machine.initialize());
    // EXPECT_FALSE(state_machine.deinitialize());
    // EXPECT_FALSE(state_machine.start());
    // EXPECT_FALSE(state_machine.pause());
    EXPECT_FALSE(state_machine.stop());
    EXPECT_FALSE(state_machine.exit());

    // switch back to Loaded
    EXPECT_TRUE(state_machine.deinitialize());
    EXPECT_EQ("Loaded", state_machine.getCurrentStateName());
    EXPECT_FALSE(state_machine.load());
    // EXPECT_FALSE(state_machine.unload());
    // EXPECT_FALSE(state_machine.initialize());
    EXPECT_FALSE(state_machine.deinitialize());
    EXPECT_FALSE(state_machine.start());
    EXPECT_FALSE(state_machine.pause());
    EXPECT_FALSE(state_machine.stop());
    EXPECT_FALSE(state_machine.exit());

    // switch back to Unloaded
    EXPECT_TRUE(state_machine.unload());
    EXPECT_EQ("Unloaded", state_machine.getCurrentStateName());
    // EXPECT_FALSE(state_machine.load());
    EXPECT_FALSE(state_machine.unload());
    EXPECT_FALSE(state_machine.initialize());
    EXPECT_FALSE(state_machine.deinitialize());
    EXPECT_FALSE(state_machine.start());
    EXPECT_FALSE(state_machine.pause());
    EXPECT_FALSE(state_machine.stop());
    // EXPECT_FALSE(state_machine.exit());

    // exit
    EXPECT_TRUE(state_machine.exit());
    ASSERT_TRUE(state_machine.isFinalized());
    // state machine has finalized, so the current state name is unknown
    EXPECT_EQ("Finalized", state_machine.getCurrentStateName());
}

struct StateMachineTest : public ::testing::Test {
    void SetUp() override
    {
        // wrapping unique_ptr into shared_ptr to be able to capture it in the lambda for the
        // element
        // factory (see below)
        test_element_wrapper =
            std::make_shared<std::unique_ptr<::testing::StrictMock<::fep3::mock::MockElementBase>>>(
                std::make_unique<::testing::StrictMock<::fep3::mock::MockElementBase>>());

        test_element = (test_element_wrapper.get()->get());

        state_machine = std::make_unique<::fep3::native::ParticipantStateMachine>(
            ::fep3::arya::ElementManager(
                std::make_shared<TestElementFactory>(test_element_wrapper)),
            component_registry,
            nullptr);

        auto logging_service_mock = std::make_shared<LoggingServiceComponent>();
        EXPECT_CALL(*logging_service_mock, createLogger(_)).WillRepeatedly(Return(logger));

        ASSERT_FEP3_NOERROR(component_registry->registerComponent<fep3::ILoggingService>(
            logging_service_mock,
            fep3::ComponentVersionInfo{FEP3_PARTICIPANT_LIBRARY_VERSION_STR,
                                       "dummyPath",
                                       FEP3_PARTICIPANT_LIBRARY_VERSION_STR}));

        mock_comp = std::make_shared<NiceMock<fep3::mock::arya::StubComponent>>();
        ASSERT_FEP3_NOERROR(component_registry->registerComponent(
            "mockComponent",
            mock_comp,
            fep3::ComponentVersionInfo{FEP3_PARTICIPANT_LIBRARY_VERSION_STR,
                                       "dummyPath",
                                       FEP3_PARTICIPANT_LIBRARY_VERSION_STR}));
    }

protected:
    std::shared_ptr<::fep3::ComponentRegistry> component_registry =
        std::make_shared<::fep3::ComponentRegistry>();

    std::shared_ptr<std::unique_ptr<::testing::StrictMock<::fep3::mock::MockElementBase>>>
        test_element_wrapper;
    ::testing::StrictMock<::fep3::mock::MockElementBase>* test_element;
    std::unique_ptr<::fep3::native::ParticipantStateMachine> state_machine;
    std::shared_ptr<Logger> logger = std::make_shared<Logger>();
    std::shared_ptr<NiceMock<fep3::mock::arya::StubComponent>> mock_comp;
};

TEST_F(StateMachineTest, exit)
{
    EXPECT_FALSE(state_machine->isFinalized());
    EXPECT_TRUE(state_machine->exit());
    EXPECT_TRUE(state_machine->isFinalized());
    EXPECT_EQ(state_machine->getCurrentStateName(), "Finalized");
}

TEST_F(StateMachineTest, load)
{
    EXPECT_CALL(*test_element, load()).WillOnce(::testing::Return(::fep3::Result{}));

    EXPECT_TRUE(state_machine->load());
    EXPECT_EQ(state_machine->getCurrentStateName(), "Loaded");
}

TEST_F(StateMachineTest, load_elementLoadError)
{
    EXPECT_CALL(*test_element, load()).WillOnce(::testing::Return(::fep3::Result{-32}));

    EXPECT_FALSE(state_machine->load());
    EXPECT_EQ(state_machine->getCurrentStateName(), "Unloaded");
}

TEST_F(StateMachineTest, unload)
{
    EXPECT_CALL(*test_element, load()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*test_element, unload()).Times(1);

    EXPECT_TRUE(state_machine->load());
    EXPECT_TRUE(state_machine->unload());
    EXPECT_EQ(state_machine->getCurrentStateName(), "Unloaded");
}

TEST_F(StateMachineTest, unload_elementThrows)
{
    EXPECT_CALL(*test_element, load()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*test_element, unload()).WillOnce(::testing::Invoke([]() {
        throw std::exception();
    }));

    EXPECT_TRUE(state_machine->load());
    EXPECT_TRUE(state_machine->unload());
    EXPECT_EQ(state_machine->getCurrentStateName(), "Unloaded");
}

TEST_F(StateMachineTest, initialize)
{
    ::testing::InSequence call_sequence;
    EXPECT_CALL(*test_element, load()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*test_element, initialize()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*mock_comp, initialize()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*mock_comp, tense()).WillOnce(::testing::Return(::fep3::Result{}));

    EXPECT_TRUE(state_machine->load());
    EXPECT_TRUE(state_machine->initialize());
    EXPECT_EQ(state_machine->getCurrentStateName(), "Initialized");
}

TEST_F(StateMachineTest, initialize_elementInitializeError)
{
    ::testing::InSequence call_sequence;
    EXPECT_CALL(*test_element, load()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*test_element, initialize()).WillOnce(::testing::Return(::fep3::Result{-32}));

    EXPECT_TRUE(state_machine->load());
    EXPECT_FALSE(state_machine->initialize());
    EXPECT_EQ(state_machine->getCurrentStateName(), "Loaded");
}

TEST_F(StateMachineTest, initialize_componentRegistryInitializeError_deinitializeElement)
{
    ::testing::InSequence call_sequence;
    EXPECT_CALL(*test_element, load()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*test_element, initialize()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*mock_comp, initialize()).WillOnce(::testing::Return(::fep3::Result{-32}));
    EXPECT_CALL(*test_element, deinitialize()).Times(1);

    EXPECT_TRUE(state_machine->load());
    EXPECT_FALSE(state_machine->initialize());
    EXPECT_EQ(state_machine->getCurrentStateName(), "Loaded");
}

TEST_F(StateMachineTest, initialize_componentRegistryTenseError_deinitializeRegistryAndElement)
{
    ::testing::InSequence call_sequence;
    EXPECT_CALL(*test_element, load()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*test_element, initialize()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*mock_comp, initialize()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*mock_comp, tense()).WillOnce(::testing::Return(::fep3::Result{-32}));
    EXPECT_CALL(*mock_comp, deinitialize()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*test_element, deinitialize()).Times(1);

    EXPECT_TRUE(state_machine->load());
    EXPECT_FALSE(state_machine->initialize());
    EXPECT_EQ(state_machine->getCurrentStateName(), "Loaded");
}

TEST_F(StateMachineTest, deinitialize)
{
    ::testing::InSequence call_sequence;
    EXPECT_CALL(*test_element, load()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*test_element, initialize()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*mock_comp, initialize()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*mock_comp, tense()).WillOnce(::testing::Return(::fep3::Result{}));

    EXPECT_TRUE(state_machine->load());
    EXPECT_TRUE(state_machine->initialize());
    EXPECT_EQ(state_machine->getCurrentStateName(), "Initialized");
}

TEST_F(StateMachineTest, deinitialize_componentRegistryRelaxError)
{
    ::testing::InSequence call_sequence;
    EXPECT_CALL(*test_element, load()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*test_element, initialize()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*mock_comp, initialize()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*mock_comp, tense()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*mock_comp, relax()).WillOnce(::testing::Return(::fep3::Result{-32}));
    EXPECT_CALL(*mock_comp, deinitialize()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*test_element, deinitialize()).Times(1);

    EXPECT_TRUE(state_machine->load());
    EXPECT_TRUE(state_machine->initialize());
    EXPECT_TRUE(state_machine->deinitialize());
    EXPECT_EQ(state_machine->getCurrentStateName(), "Loaded");
}

TEST_F(StateMachineTest, deinitialize_componentRegistryDeinitializeError)
{
    ::testing::InSequence call_sequence;
    EXPECT_CALL(*test_element, load()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*test_element, initialize()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*mock_comp, initialize()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*mock_comp, tense()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*mock_comp, relax()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*mock_comp, deinitialize()).WillOnce(::testing::Return(::fep3::Result{-32}));
    EXPECT_CALL(*test_element, deinitialize()).Times(1);

    EXPECT_TRUE(state_machine->load());
    EXPECT_TRUE(state_machine->initialize());
    EXPECT_FALSE(state_machine->deinitialize());
    EXPECT_EQ(state_machine->getCurrentStateName(), "Loaded");
}

TEST_F(StateMachineTest, deinitialize_elementDeinitializeThrows)
{
    ::testing::InSequence call_sequence;
    EXPECT_CALL(*test_element, load()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*test_element, initialize()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*mock_comp, initialize()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*mock_comp, tense()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*mock_comp, relax()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*mock_comp, deinitialize()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*test_element, deinitialize()).WillOnce(::testing::Invoke([]() {
        throw std::exception();
    }));

    EXPECT_TRUE(state_machine->load());
    EXPECT_TRUE(state_machine->initialize());
    EXPECT_TRUE(state_machine->deinitialize());
    EXPECT_EQ(state_machine->getCurrentStateName(), "Loaded");
}

TEST_F(StateMachineTest, start)
{
    ::testing::InSequence call_sequence;
    EXPECT_CALL(*test_element, load()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*test_element, initialize()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*mock_comp, initialize()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*mock_comp, tense()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*test_element, run()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*mock_comp, start()).WillOnce(::testing::Return(::fep3::Result{}));

    EXPECT_TRUE(state_machine->load());
    EXPECT_TRUE(state_machine->initialize());
    EXPECT_TRUE(state_machine->start());
    EXPECT_EQ(state_machine->getCurrentStateName(), "Running");
}

TEST_F(StateMachineTest, start_elementRunError)
{
    ::testing::InSequence call_sequence;
    EXPECT_CALL(*test_element, load()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*test_element, initialize()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*mock_comp, initialize()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*mock_comp, tense()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*test_element, run()).WillOnce(::testing::Return(::fep3::Result{-32}));

    EXPECT_TRUE(state_machine->load());
    EXPECT_TRUE(state_machine->initialize());
    EXPECT_FALSE(state_machine->start());
    EXPECT_EQ(state_machine->getCurrentStateName(), "Initialized");
}

TEST_F(StateMachineTest, start_componentRegistryStartError_stopElement)
{
    ::testing::InSequence call_sequence;
    EXPECT_CALL(*test_element, load()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*test_element, initialize()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*mock_comp, initialize()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*mock_comp, tense()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*test_element, run()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*mock_comp, start()).WillOnce(::testing::Return(::fep3::Result{-32}));
    EXPECT_CALL(*test_element, stop()).Times(1);

    EXPECT_TRUE(state_machine->load());
    EXPECT_TRUE(state_machine->initialize());
    EXPECT_FALSE(state_machine->start());
    EXPECT_EQ(state_machine->getCurrentStateName(), "Initialized");
}

TEST_F(StateMachineTest, stop)
{
    ::testing::InSequence call_sequence;
    EXPECT_CALL(*test_element, load()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*test_element, initialize()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*mock_comp, initialize()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*mock_comp, tense()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*test_element, run()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*mock_comp, start()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*mock_comp, stop()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*test_element, stop()).Times(1);

    EXPECT_TRUE(state_machine->load());
    EXPECT_TRUE(state_machine->initialize());
    EXPECT_TRUE(state_machine->start());
    EXPECT_TRUE(state_machine->stop());
    EXPECT_EQ(state_machine->getCurrentStateName(), "Initialized");
}

TEST_F(StateMachineTest, stop_elementThrows)
{
    ::testing::InSequence call_sequence;
    EXPECT_CALL(*test_element, load()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*test_element, initialize()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*mock_comp, initialize()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*mock_comp, tense()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*test_element, run()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*mock_comp, start()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*mock_comp, stop()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*test_element, stop()).WillOnce(::testing::Invoke([]() {
        throw std::exception();
    }));

    EXPECT_TRUE(state_machine->load());
    EXPECT_TRUE(state_machine->initialize());
    EXPECT_TRUE(state_machine->start());
    EXPECT_TRUE(state_machine->stop());
    EXPECT_EQ(state_machine->getCurrentStateName(), "Initialized");
}

TEST_F(StateMachineTest, stop_componentRegistryStopError)
{
    ::testing::InSequence call_sequence;
    EXPECT_CALL(*test_element, load()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*test_element, initialize()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*mock_comp, initialize()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*mock_comp, tense()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*test_element, run()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*mock_comp, start()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*mock_comp, stop()).WillOnce(::testing::Return(::fep3::Result{-32}));
    EXPECT_CALL(*test_element, stop()).Times(1);

    EXPECT_TRUE(state_machine->load());
    EXPECT_TRUE(state_machine->initialize());
    EXPECT_TRUE(state_machine->start());
    EXPECT_FALSE(state_machine->stop());
    EXPECT_EQ(state_machine->getCurrentStateName(), "Initialized");
}

TEST_F(StateMachineTest, pause_fromRunning)
{
    ::testing::InSequence call_sequence;
    EXPECT_CALL(*test_element, load()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*test_element, initialize()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*mock_comp, initialize()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*mock_comp, tense()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*test_element, run()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*mock_comp, start()).WillOnce(::testing::Return(::fep3::Result{}));
    // Pause is not implemented yet and therefore is not called
    // EXPECT_CALL(*mock_comp, pause()).WillOnce(::testing::Return(::fep3::Result{}));

    EXPECT_TRUE(state_machine->load());
    EXPECT_TRUE(state_machine->initialize());
    EXPECT_TRUE(state_machine->start());

    // Pause is not implemented yet and therefore returns an error
    EXPECT_FALSE(state_machine->pause());
    // EXPECT_TRUE(state_machine->pause());
    EXPECT_EQ(state_machine->getCurrentStateName(), "Running");
    // EXPECT_EQ(state_machine->getCurrentStateName(), "Paused");
}

TEST_F(StateMachineTest, pause_fromRunning_ComponentRegistryPauseError)
{
    ::testing::InSequence call_sequence;
    EXPECT_CALL(*test_element, load()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*test_element, initialize()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*mock_comp, initialize()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*mock_comp, tense()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*test_element, run()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*mock_comp, start()).WillOnce(::testing::Return(::fep3::Result{}));
    // Pause is not implemented yet and therefore is not called
    // EXPECT_CALL(*mock_comp, pause()).WillOnce(::testing::Return(::fep3::Result{-32}));

    EXPECT_TRUE(state_machine->load());
    EXPECT_TRUE(state_machine->initialize());
    EXPECT_TRUE(state_machine->start());
    EXPECT_FALSE(state_machine->pause());
    EXPECT_EQ(state_machine->getCurrentStateName(), "Running");
}

TEST_F(StateMachineTest, pause_fromInitialized)
{
    ::testing::InSequence call_sequence;
    EXPECT_CALL(*test_element, load()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*test_element, initialize()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*mock_comp, initialize()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*mock_comp, tense()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*test_element, run()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*test_element, stop()).Times(1);
    // Pause is not implemented yet and therefore is not called
    // EXPECT_CALL(*mock_comp, pause()).WillOnce(::testing::Return(::fep3::Result{}));

    EXPECT_TRUE(state_machine->load());
    EXPECT_TRUE(state_machine->initialize());
    // Pause is not implemented yet and therefore returns an error
    EXPECT_FALSE(state_machine->pause());
    // EXPECT_TRUE(state_machine->pause());
    EXPECT_EQ(state_machine->getCurrentStateName(), "Initialized");
    // EXPECT_EQ(state_machine->getCurrentStateName(), "Paused");
}

TEST_F(StateMachineTest, pause_fromInitialized_ElementRunError)
{
    ::testing::InSequence call_sequence;
    EXPECT_CALL(*test_element, load()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*test_element, initialize()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*mock_comp, initialize()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*mock_comp, tense()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*test_element, run()).WillOnce(::testing::Return(::fep3::Result{-32}));

    EXPECT_TRUE(state_machine->load());
    EXPECT_TRUE(state_machine->initialize());
    EXPECT_FALSE(state_machine->pause());
    EXPECT_EQ(state_machine->getCurrentStateName(), "Initialized");
}