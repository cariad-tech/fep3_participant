/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <fep3/components/base/mock_components.h>
#include <fep3/components/clock/mock_clock_service.h>
#include <fep3/components/configuration/mock_configuration_service.h>
#include <fep3/components/data_registry/mock_data_registry.h>
#include <fep3/components/job_registry/mock_job_registry.h>
#include <fep3/components/logging/mock_logger.h>
#include <fep3/components/logging/mock_logging_service.h>
#include <fep3/cpp/element_base.h>

// global getter for the cylce time
// note: we need a global instance because the DataJobElement class's default CTOR constructs an
// object without the possibility to pass the parameters
std::function<fep3::arya::Duration()> g_cycle_time_getter;
using namespace std::literals::chrono_literals;

// a test class for the data job
// note: we cannot use a mock class, because the base class of the job must be
// fep3::cpp::arya::DataJob as required by the implementation of DataJobElement
class DataJobMock : public fep3::cpp::DataJob {
public:
    DataJobMock() : fep3::cpp::DataJob("test_job", 10ms)
    {
    }

    MOCK_METHOD0(reset, fep3::Result());
};

class DataTriggeredJobMock : public fep3::cpp::DataJob {
public:
    DataTriggeredJobMock()
        : fep3::cpp::DataJob("test_job", fep3::DataTriggeredJobConfiguration({"signal_name"}))
    {
    }

    MOCK_METHOD0(reset, fep3::Result());
};

class ClockTriggeredJobMock : public fep3::cpp::DataJob {
public:
    ClockTriggeredJobMock()
        : fep3::cpp::DataJob("test_job", fep3::ClockTriggeredJobConfiguration(10ms))
    {
    }

    MOCK_METHOD0(reset, fep3::Result());
};

// a test class for DataJobElement
// note: we can't access the getComponents() in DataJobElement for now to test the private member
// _components.
template <typename data_job_type>
class DataJobElementTestable : public fep3::cpp::DataJobElement<data_job_type> {
public:
    // inherit all constructors
    using fep3::cpp::DataJobElement<data_job_type>::DataJobElement;
    // expose getComponents to public
    using fep3::cpp::DataJobElement<data_job_type>::getComponents;
};

class BaseTestDataJobElement {
public:
    BaseTestDataJobElement()
    {
    }

protected:
    void SetUp()
    {
        // setup components
        _clock_service_mock = std::make_shared<::testing::StrictMock<fep3::mock::ClockService>>();
        _configuration_service_mock =
            std::make_shared<::testing::StrictMock<fep3::mock::ConfigurationService>>();
        _data_registry_mock = std::make_shared<::testing::StrictMock<fep3::mock::DataRegistry>>();
        _job_registry_mock = std::make_shared<::testing::StrictMock<fep3::mock::JobRegistry>>();
        _logging_service_mock =
            std::make_shared<::testing::StrictMock<fep3::mock::LoggingService>>();
        EXPECT_CALL(_components_mock, findComponent("clock_service.arya.fep3.iid"))
            .WillRepeatedly(::testing::Return(_clock_service_mock.get()));
        EXPECT_CALL(_components_mock, findComponent("clock_service.experimental.fep3.iid"))
            .WillRepeatedly(::testing::Return(_clock_service_mock.get()));
        EXPECT_CALL(_components_mock, findComponent("configuration_service.arya.fep3.iid"))
            .WillRepeatedly(::testing::Return(_configuration_service_mock.get()));
        EXPECT_CALL(_components_mock, findComponent("data_registry.arya.fep3.iid"))
            .WillRepeatedly(::testing::Return(_data_registry_mock.get()));
        EXPECT_CALL(_components_mock, findComponent("job_registry.catelyn.fep3.iid"))
            .WillRepeatedly(::testing::Return(_job_registry_mock.get()));
        EXPECT_CALL(_components_mock, findComponent("logging_service.arya.fep3.iid"))
            .WillRepeatedly(::testing::Return(_logging_service_mock.get()));
    }
    void TearDown()
    {
        _clock_service_mock.reset();
        _configuration_service_mock.reset();
        _data_registry_mock.reset();
        _job_registry_mock.reset();
        _logging_service_mock.reset();

        // make sure the global cycle time getter is reset for the next test
        // g_cycle_time_getter = decltype(g_cycle_time_getter)();
    }

protected:
    ::testing::StrictMock<fep3::mock::Components> _components_mock;
    std::shared_ptr<fep3::mock::ClockService> _clock_service_mock;
    std::shared_ptr<fep3::mock::ConfigurationService> _configuration_service_mock;
    std::shared_ptr<fep3::mock::DataRegistry> _data_registry_mock;
    std::shared_ptr<fep3::mock::JobRegistry> _job_registry_mock;
    std::shared_ptr<fep3::mock::LoggingService> _logging_service_mock;
};

template <typename T>
class TestDataJobElement
    : public ::testing::TestWithParam<std::function<DataJobElementTestable<T>()>>,
      public BaseTestDataJobElement

{
public:
    TestDataJobElement() : BaseTestDataJobElement()
    {
    }

protected:
    void SetUp()
    {
        BaseTestDataJobElement::SetUp();
    }
    void TearDown()

    {
        BaseTestDataJobElement::TearDown();
    }
};

TYPED_TEST_SUITE_P(TestDataJobElement);
using Types = testing::Types<DataJobMock, DataTriggeredJobMock, ClockTriggeredJobMock>;

/**
 * @brief Test CTORs of fep3::cpp::DataJobElement
 */
TYPED_TEST_P(TestDataJobElement, methods)
{
    const auto& test_job_name = std::string("test_job");

    auto job_elements = std::vector<DataJobElementTestable<TypeParam>>{
        DataJobElementTestable<TypeParam>(),
        DataJobElementTestable<TypeParam>(std::make_shared<TypeParam>()),
    };

    for (auto& test_data_job_element: job_elements) {
        fep3::base::IElement& test_element = test_data_job_element;

        // const auto& test_cycle_time = g_cycle_time_getter();

        // check initialization
        EXPECT_EQ("fep3::cpp::DataJobElement", test_element.getTypename());
        EXPECT_EQ(FEP3_PARTICIPANT_LIBRARY_VERSION_STR, test_element.getVersion());
        EXPECT_EQ(nullptr, test_data_job_element.getComponents());

        // check loadElement
        // calling loadElement is necessary to initialize base class fep3::core::ElementBase
        // that in turn provides access to components
        const auto& logger_mock = std::make_shared<::testing::StrictMock<fep3::mock::Logger>>();

        // if loadElement fails while calling initLogger
        EXPECT_CALL(*this->_logging_service_mock, createLogger("element"))
            .WillOnce(::testing::Throw(std::runtime_error("runtime error")));
        EXPECT_NE(fep3::Result(), test_element.loadElement(this->_components_mock));
        EXPECT_EQ(nullptr, test_data_job_element.getComponents());

        // if loadElement succeeds
        EXPECT_CALL(*this->_logging_service_mock, createLogger("element"))
            .WillOnce(::testing::Return(logger_mock));
        EXPECT_CALL(*this->_logging_service_mock, createLogger("element.job." + test_job_name))
            .WillOnce(::testing::Return(logger_mock));
        EXPECT_CALL(*this->_configuration_service_mock, registerNode(::testing::_))
            .WillOnce(::testing::Return(fep3::Result()));
        // calling loadElement calls load and this in turn calls addJob
        EXPECT_CALL(*this->_job_registry_mock,
                    addJob(test_job_name,
                           ::testing::_,
                           ::testing::Matcher<const fep3::JobConfiguration&>(::testing::_)))
            .WillOnce(::testing::Return(fep3::Result()));

        EXPECT_EQ(fep3::Result(), test_element.loadElement(this->_components_mock));
        EXPECT_EQ(&this->_components_mock, (void*)test_data_job_element.getComponents());

        ::testing::Mock::VerifyAndClearExpectations(this->_logging_service_mock.get());
        ::testing::Mock::VerifyAndClearExpectations(this->_configuration_service_mock.get());
        ::testing::Mock::VerifyAndClearExpectations(this->_job_registry_mock.get());

        std::shared_ptr<fep3::arya::IJob> registered_job;

        // ------------------------------------
        // begin test methods of DataJobElement
        // ------------------------------------

        // method load
        EXPECT_CALL(*this->_logging_service_mock, createLogger("element.job." + test_job_name))
            .WillOnce(::testing::Return(logger_mock));
        EXPECT_CALL(*this->_configuration_service_mock, unregisterNode("job_" + test_job_name))
            .WillOnce(::testing::Return(fep3::Result()));
        EXPECT_CALL(*this->_configuration_service_mock, registerNode(::testing::_))
            .WillOnce(::testing::Return(fep3::Result()));
        // note: as the DataJobElement is a Job factory (at least when using the default CTOR)
        // the only way to access the job is to save the argument of addJob
        EXPECT_CALL(*this->_job_registry_mock,
                    addJob(test_job_name,
                           ::testing::_,
                           ::testing::Matcher<const fep3::JobConfiguration&>(::testing::_)))
            .WillOnce(
                ::testing::Invoke([&registered_job](const std::string&,
                                                    const std::shared_ptr<fep3::arya::IJob>& job,
                                                    const fep3::JobConfiguration&) {
                    registered_job = job;
                    return fep3::Result{};
                }));
        EXPECT_EQ(fep3::Result(), test_data_job_element.load());
        ::testing::Mock::VerifyAndClearExpectations(this->_logging_service_mock.get());
        ::testing::Mock::VerifyAndClearExpectations(this->_configuration_service_mock.get());
        ::testing::Mock::VerifyAndClearExpectations(this->_job_registry_mock.get());

        const auto& mock_data_job = std::dynamic_pointer_cast<TypeParam>(registered_job);
        ASSERT_TRUE(mock_data_job);

        // method unload
        EXPECT_CALL(*this->_configuration_service_mock, unregisterNode("job_" + test_job_name))
            .WillOnce(::testing::Return(fep3::Result()));
        EXPECT_CALL(*this->_job_registry_mock, removeJob(test_job_name))
            .WillOnce(::testing::Return(fep3::Result()));
        test_data_job_element.unload();
        ::testing::Mock::VerifyAndClearExpectations(this->_configuration_service_mock.get());
        ::testing::Mock::VerifyAndClearExpectations(this->_job_registry_mock.get());

        // method stop
        test_data_job_element.stop();

        // method run
        EXPECT_CALL(*mock_data_job, reset()).WillOnce(::testing::Return(fep3::Result()));
        EXPECT_EQ(fep3::Result(), test_data_job_element.run());
        ::testing::Mock::VerifyAndClearExpectations(mock_data_job.get());

        // ----------------------------------
        // end test methods of DataJobElement
        // ----------------------------------

        // check unloadElement
        //
        // component before unloadElement
        EXPECT_NE(nullptr, test_data_job_element.getComponents());
        EXPECT_CALL(*this->_job_registry_mock, removeJob(test_job_name))
            .WillOnce(::testing::Return(fep3::Result()));
        test_element.unloadElement();
        // component after unloadElement
        EXPECT_EQ(nullptr, test_data_job_element.getComponents());
        ::testing::Mock::VerifyAndClearExpectations(this->_job_registry_mock.get());
    }
}

/**
 * Test scenarios where resetting of the Job is necessary
 * @req_id FEPSDK-1822
 */
TYPED_TEST_P(TestDataJobElement, jobReset)
{
    const auto& test_job_name = std::string("test_job");

    auto job_elements = std::vector<DataJobElementTestable<TypeParam>>{
        DataJobElementTestable<TypeParam>(),
        DataJobElementTestable<TypeParam>(std::make_shared<TypeParam>()),
    };

    for (auto& test_data_job_element: job_elements) {
        fep3::base::IElement& test_element = test_data_job_element;

        // const auto& test_cycle_time = g_cycle_time_getter();

        std::shared_ptr<fep3::arya::IJob> registered_job;

        // calling loadElement is necessary to initialize base class fep3::core::ElementBase
        // that in turn provides access to components
        const auto& logger_mock = std::make_shared<::testing::StrictMock<fep3::mock::Logger>>();
        EXPECT_CALL(*this->_logging_service_mock, createLogger("element"))
            .WillOnce(::testing::Return(logger_mock));
        EXPECT_CALL(*this->_logging_service_mock, createLogger("element.job." + test_job_name))
            .WillOnce(::testing::Return(logger_mock));
        EXPECT_CALL(*this->_configuration_service_mock, registerNode(::testing::_))
            .WillOnce(::testing::Return(fep3::Result()));
        // note: as the DataJobElement is a Job factory (at least when using the default CTOR)
        // the only way to access the job is to save the argument of addJob
        EXPECT_CALL(*this->_job_registry_mock,
                    addJob(test_job_name,
                           ::testing::_,
                           ::testing::Matcher<const fep3::JobConfiguration&>(::testing::_)))
            .WillOnce(
                ::testing::Invoke([&registered_job](const std::string&,
                                                    const std::shared_ptr<fep3::arya::IJob>& job,
                                                    const fep3::JobConfiguration&) {
                    registered_job = job;
                    return fep3::Result{};
                }));
        EXPECT_EQ(fep3::Result(), test_element.loadElement(this->_components_mock));
        ::testing::Mock::VerifyAndClearExpectations(this->_logging_service_mock.get());
        ::testing::Mock::VerifyAndClearExpectations(this->_configuration_service_mock.get());
        ::testing::Mock::VerifyAndClearExpectations(this->_job_registry_mock.get());

        const auto& mock_data_job = std::dynamic_pointer_cast<TypeParam>(registered_job);
        ASSERT_TRUE(mock_data_job);

        // on initial call to run, reset of the job is required
        EXPECT_CALL(*mock_data_job, reset()).WillOnce(::testing::Return(fep3::Result()));
        EXPECT_EQ(fep3::Result(), test_data_job_element.run());
        ::testing::Mock::VerifyAndClearExpectations(mock_data_job.get());

        // on subsequential call to run, reset of the job is not required
        EXPECT_CALL(*mock_data_job, reset()).Times(0);
        EXPECT_EQ(fep3::Result(), test_data_job_element.run());
        ::testing::Mock::VerifyAndClearExpectations(mock_data_job.get());

        test_data_job_element.stop();

        // on call to run after stop, reset of the job is required
        EXPECT_CALL(*mock_data_job, reset()).WillOnce(::testing::Return(fep3::Result()));
        EXPECT_EQ(fep3::Result(), test_data_job_element.run());
        ::testing::Mock::VerifyAndClearExpectations(mock_data_job.get());
    }
}

REGISTER_TYPED_TEST_SUITE_P(TestDataJobElement, methods, jobReset);
INSTANTIATE_TYPED_TEST_SUITE_P(TestAllDataJob, TestDataJobElement, Types);
