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


#include <gmock/gmock.h>
#include <fep3/cpp/element_base.h>
#include <fep3/components/clock/mock/mock_clock_service.h>
#include <fep3/components/configuration/mock/mock_configuration_service.h>
#include <fep3/components/data_registry/mock/mock_data_registry.h>
#include <fep3/components/job_registry/mock/mock_job_registry.h>
#include <fep3/components/logging/mock/mock_logging_service.h>
#include <fep3/components/mock/mock_components.h>

// global getter for the cylce time
// note: we need a global instance because the DataJobElement class's default CTOR constructs an object 
// without the possibility to pass the parameters
std::function<fep3::arya::Duration()> g_cycle_time_getter;

// a test class for the data job
// note: we cannot use a mock class, because the base class of the job must be 
// fep3::cpp::arya::DataJob as required by the implementation of DataJobElement
class DataJobMock : public fep3::cpp::arya::DataJob
{
public:
    DataJobMock()
        : fep3::cpp::arya::DataJob("test_job", g_cycle_time_getter())
    {}
    
    MOCK_METHOD0(reset, fep3::Result());
};

// a test class for DataJobElement
// note: we can't access the getComponents() in DataJobElement for now to test the private member _components.
template<typename data_job_type>
class DataJobElementTestable : public fep3::cpp::arya::DataJobElement<data_job_type> 
{
public:
    // inherite all constructors
    using fep3::cpp::arya::DataJobElement<data_job_type>::DataJobElement;
    // expose getComponents to public
    using fep3::cpp::arya::DataJobElement<data_job_type>::getComponents;
};

class TestDataJobElement : public ::testing::TestWithParam<std::function<DataJobElementTestable<DataJobMock>()>>
{
public: 
    TestDataJobElement()
    {}
protected:
    void SetUp()
    {
        // setup components
        _clock_service_mock = std::make_shared<::testing::StrictMock<fep3::mock::ClockService<>>>();
        _configuration_service_mock = std::make_shared<::testing::StrictMock<fep3::mock::ConfigurationService<>>>();
        _data_registry_mock = std::make_shared<::testing::StrictMock<fep3::mock::DataRegistryComponent>>();
        _job_registry_mock = std::make_shared<::testing::StrictMock<fep3::mock::JobRegistryComponent<>>>();
        _logging_service_mock = std::make_shared<::testing::StrictMock<fep3::mock::MockLoggingService>>();
        EXPECT_CALL(_components_mock, findComponent("clock_service.arya.fep3.iid"))
            .WillRepeatedly(::testing::Return(_clock_service_mock.get()));
        EXPECT_CALL(_components_mock, findComponent("configuration_service.arya.fep3.iid"))
            .WillRepeatedly(::testing::Return(_configuration_service_mock.get()));
        EXPECT_CALL(_components_mock, findComponent("data_registry.arya.fep3.iid"))
            .WillRepeatedly(::testing::Return(_data_registry_mock.get()));
        EXPECT_CALL(_components_mock, findComponent("job_registry.arya.fep3.iid"))
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
        g_cycle_time_getter = decltype(g_cycle_time_getter)();
    }
protected:
    ::testing::StrictMock<fep3::mock::MockComponents> _components_mock;
    std::shared_ptr<fep3::mock::ClockService<>> _clock_service_mock;
    std::shared_ptr<fep3::mock::ConfigurationService<>> _configuration_service_mock;
    std::shared_ptr<fep3::mock::DataRegistryComponent> _data_registry_mock;
    std::shared_ptr<fep3::mock::JobRegistryComponent<>> _job_registry_mock;
    std::shared_ptr<fep3::mock::MockLoggingService> _logging_service_mock;
};

/**
* @brief Test CTORs of fep3::cpp::DataJobElement
*
*/
TEST_P(TestDataJobElement, methods)
{
    const auto& test_job_name = std::string("test_job");

    auto test_data_job_element = GetParam()();
    fep3::IElement& test_element = test_data_job_element;
    
    const auto& test_cycle_time = g_cycle_time_getter();
    
    // check initialization
    EXPECT_EQ("fep3::cpp::DataJobElement", test_element.getTypename());
    EXPECT_EQ(FEP3_PARTICIPANT_LIBRARY_VERSION_STR, test_element.getVersion());
    EXPECT_EQ(nullptr, test_data_job_element.getComponents());

    // check loadElement
    // calling loadElement is necessary to initialize base class fep3::core::ElementBase
    // that in turn provides access to components
    const auto& logger_mock = std::make_shared<::testing::StrictMock<fep3::mock::LoggerMock>>();
    
    // if loadElement fails while calling initLogger
    EXPECT_CALL(*_logging_service_mock, createLogger("element"))
        .WillOnce(::testing::Throw(std::runtime_error("runtime error")));
    EXPECT_NE(fep3::Result(), test_element.loadElement(_components_mock));
    EXPECT_EQ(nullptr, test_data_job_element.getComponents());

    // if loadElement succeeds
    EXPECT_CALL(*_logging_service_mock, createLogger("element"))
        .WillOnce(::testing::Return(logger_mock));
    EXPECT_CALL(*_logging_service_mock, createLogger("element.job." + test_job_name))
        .WillOnce(::testing::Return(logger_mock));
    EXPECT_CALL(*_configuration_service_mock, registerNode(::testing::_))
        .WillOnce(::testing::Return(fep3::Result()));
    // calling loadElement calls load and this inturn calls addJob
    EXPECT_CALL
        (*_job_registry_mock
        , addJob
            (test_job_name
            , ::testing::_
            , ::testing::Field(&fep3::arya::JobConfiguration::_cycle_sim_time, ::testing::Eq(test_cycle_time))
            )
        )
        .WillOnce(::testing::Return(fep3::Result()));
    
    EXPECT_EQ(fep3::Result(), test_element.loadElement(_components_mock));
    EXPECT_EQ(&_components_mock, (void *) test_data_job_element.getComponents());

    ::testing::Mock::VerifyAndClearExpectations(_logging_service_mock.get());
    ::testing::Mock::VerifyAndClearExpectations(_configuration_service_mock.get());
    ::testing::Mock::VerifyAndClearExpectations(_job_registry_mock.get());
    
    std::shared_ptr<fep3::arya::IJob> registered_job;
    
    // ------------------------------------
    // begin test methods of DataJobElement
    // ------------------------------------
    
    // method load
    EXPECT_CALL(*_logging_service_mock, createLogger("element.job." + test_job_name))
        .WillOnce(::testing::Return(logger_mock));
    EXPECT_CALL(*_configuration_service_mock, unregisterNode("job_" + test_job_name))
        .WillOnce(::testing::Return(fep3::Result()));
    EXPECT_CALL(*_configuration_service_mock, registerNode(::testing::_))
        .WillOnce(::testing::Return(fep3::Result()));
    // note: as the DataJobElement is a Job factory (at least when using the default CTOR)
    // the only way to access the job is to save the argument of addJob
    EXPECT_CALL
        (*_job_registry_mock
        , addJob
            (test_job_name
            , ::testing::_
            , ::testing::Field(&fep3::arya::JobConfiguration::_cycle_sim_time, ::testing::Eq(test_cycle_time))
            )
        )
        .WillOnce(::testing::Invoke
            ([&registered_job](const std::string&, 
                               const std::shared_ptr<fep3::arya::IJob>& job, 
                               const fep3::arya::JobConfiguration&)
                {
                    registered_job = job;
                    return fep3::Result{};
                }
            ));
    EXPECT_EQ(fep3::Result(), test_data_job_element.load());
    ::testing::Mock::VerifyAndClearExpectations(_logging_service_mock.get());
    ::testing::Mock::VerifyAndClearExpectations(_configuration_service_mock.get());
    ::testing::Mock::VerifyAndClearExpectations(_job_registry_mock.get());
    
    const auto& mock_data_job = std::dynamic_pointer_cast<DataJobMock>(registered_job);
    ASSERT_TRUE(mock_data_job);
    
    // method unload
    EXPECT_CALL(*_configuration_service_mock, unregisterNode("job_" + test_job_name))
        .WillOnce(::testing::Return(fep3::Result()));
    EXPECT_CALL(*_job_registry_mock, removeJob(test_job_name))
        .WillOnce(::testing::Return(fep3::Result()));
    test_data_job_element.unload();
    ::testing::Mock::VerifyAndClearExpectations(_configuration_service_mock.get());
    ::testing::Mock::VerifyAndClearExpectations(_job_registry_mock.get());
    
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
    EXPECT_CALL(*_job_registry_mock, removeJob(test_job_name))
        .WillOnce(::testing::Return(fep3::Result()));
    test_element.unloadElement();
    // component after unloadElement
    EXPECT_EQ(nullptr, test_data_job_element.getComponents());
    ::testing::Mock::VerifyAndClearExpectations(_job_registry_mock.get());
}

/**
* Test scenarios where resetting of the Job is necessary
* @req_id FEPSDK-1822
*/
TEST_P(TestDataJobElement, jobReset)
{
    const auto& test_job_name = std::string("test_job");

    auto test_data_job_element = GetParam()();
    fep3::IElement& test_element = test_data_job_element;
    
    const auto& test_cycle_time = g_cycle_time_getter();
    
    std::shared_ptr<fep3::arya::IJob> registered_job;
    
    // calling loadElement is necessary to initialize base class fep3::core::ElementBase
    // that in turn provides access to components
    const auto& logger_mock = std::make_shared<::testing::StrictMock<fep3::mock::LoggerMock>>();
    EXPECT_CALL(*_logging_service_mock, createLogger("element"))
        .WillOnce(::testing::Return(logger_mock));
    EXPECT_CALL(*_logging_service_mock, createLogger("element.job." + test_job_name))
        .WillOnce(::testing::Return(logger_mock));
    EXPECT_CALL(*_configuration_service_mock, registerNode(::testing::_))
        .WillOnce(::testing::Return(fep3::Result()));
    // note: as the DataJobElement is a Job factory (at least when using the default CTOR)
    // the only way to access the job is to save the argument of addJob
    EXPECT_CALL
        (*_job_registry_mock
        , addJob
            (test_job_name
            , ::testing::_
            , ::testing::Field(&fep3::arya::JobConfiguration::_cycle_sim_time, ::testing::Eq(test_cycle_time))
            )
        )
        .WillOnce(::testing::Invoke
            ([&registered_job](const std::string&, const std::shared_ptr<fep3::arya::IJob>& job, const fep3::arya::JobConfiguration&)
                {
                    registered_job = job;
                    return fep3::Result{};
                }
            ));
    EXPECT_EQ(fep3::Result(), test_element.loadElement(_components_mock));
    ::testing::Mock::VerifyAndClearExpectations(_logging_service_mock.get());
    ::testing::Mock::VerifyAndClearExpectations(_configuration_service_mock.get());
    ::testing::Mock::VerifyAndClearExpectations(_job_registry_mock.get());

    const auto& mock_data_job = std::dynamic_pointer_cast<DataJobMock>(registered_job);
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

INSTANTIATE_TEST_CASE_P
    (WaysOfConstruction
    , TestDataJobElement
    , ::testing::Values
        // default CTOR
        ([]()
            {
                const auto& test_cycle_time = fep3::arya::Duration(33);
                g_cycle_time_getter = [test_cycle_time]()
                    {
                        return test_cycle_time;
                    };
                return DataJobElementTestable<DataJobMock>();
            }
        // CTOR taking the job
        , []()
            {
                const auto& test_cycle_time = fep3::arya::Duration(44);
                g_cycle_time_getter = [test_cycle_time]()
                    {
                        return test_cycle_time;
                    };
                return DataJobElementTestable<DataJobMock>(std::make_shared<DataJobMock>());
            }
        )
    );
