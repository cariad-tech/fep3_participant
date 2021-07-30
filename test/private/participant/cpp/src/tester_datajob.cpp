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


#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <common/gtest_asserts.h>

#include <fep3/cpp.h>
#include <fep3/core/job.h>
#include <fep3/components/base/component_registry.h>
#include <fep3/components/job_registry/mock/mock_job_registry.h>
#include <fep3/components/job_registry/job_configuration.h>
#include <fep3/components/clock/mock/mock_clock_service.h>
#include <fep3/components/data_registry/mock/mock_data_registry.h>
#include <fep3/components/logging/mock/mock_logging_service.h>
#include <fep3/components/logging/mock/mock_logger.h>
#include <fep3/base/sample/data_sample_intf.h>
#include <fep3/base/stream_type/stream_type.h>
#include <helper/gmock_destruction_helper.h>

using namespace ::testing;
using namespace fep3::cpp;
using namespace std::literals::chrono_literals;
using IJob = fep3::arya::IJob;
using Job = fep3::core::arya::Job;
using JobConfiguration = fep3::arya::JobConfiguration;

using ClockMockComponent = NiceMock<fep3::mock::ClockServiceComponentWithDefaultBehaviour>;
using JobRegistryComponent = NiceMock<fep3::mock::JobRegistryComponentBase>;
using DataRegistryComponent = NiceMock<fep3::mock::DataRegistryComponent>;
using LoggingServiceComponent = NiceMock<fep3::mock::LoggingService>;

using DataRegistryDataReader = NiceMock<fep3::mock::DataRegistryComponent::DataReader>;
using DataRegistryDataWriter = NiceMock<fep3::mock::DataRegistryComponent::DataWriter>;
using Logger = NiceMock<fep3::mock::LoggerWithDefaultBehaviour>;

using IDataSample = fep3::arya::IDataSample;
using DataSample = fep3::base::DataSample;

struct DataJobWithMocks : Test
{
    void SetUp() override
    {
        _component_registry = std::make_shared<fep3::ComponentRegistry>();
        ASSERT_FEP3_NOERROR(_component_registry->create());

        _clock_service_mock = std::make_unique<ClockMockComponent>();
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IClockService>(_clock_service_mock));

        _job_registry_mock = std::make_unique<JobRegistryComponent>();
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IJobRegistry>(_job_registry_mock));

        _data_registry_mock = std::make_unique<DataRegistryComponent>();
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IDataRegistry>(_data_registry_mock));
    }

    std::shared_ptr<fep3::ComponentRegistry> _component_registry;
    std::shared_ptr<ClockMockComponent> _clock_service_mock{};
    std::shared_ptr<JobRegistryComponent> _job_registry_mock{};
    std::shared_ptr<DataRegistryComponent> _data_registry_mock{};
};

struct DataJobWithLoggingService : DataJobWithMocks
{
    void SetUp() override
    {
        DataJobWithMocks::SetUp();

        _logger_mock = std::make_shared<Logger>();

        _logging_service_mock = std::make_shared<LoggingServiceComponent>(_logger_mock);
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::ILoggingService>(_logging_service_mock));
    }

    std::shared_ptr<LoggingServiceComponent> _logging_service_mock{};
    std::shared_ptr<Logger> _logger_mock{};
};

class MySimpleJob : public DataJob
{
public:
    static std::atomic<int32_t> _counter;

    MySimpleJob() : DataJob("myjob", 100ms)
    {
    }

    fep3::Result process(fep3::Timestamp /*time*/) override
    {
        ++_counter;
        return {};
    }

};

std::atomic<int32_t> MySimpleJob::_counter(0);

/**
* @brief Add job created by DataJob(name, cycle_time)
*
*/
TEST_F(DataJobWithMocks, addJobCtor1)
{
    EXPECT_CALL(*_job_registry_mock, addJob("myjob", _, AllOf(
        Field(&JobConfiguration::_cycle_sim_time, Eq(100ms)),
        Field(&JobConfiguration::_delay_sim_time, Eq(0ms)),
        Field(&JobConfiguration::_runtime_violation_strategy, Eq(JobConfiguration::TimeViolationStrategy::ignore_runtime_violation))
        ))).Times(1).WillOnce(Return(fep3::Result{}));
    std::vector<std::shared_ptr<Job>> jobs{ std::make_shared<MySimpleJob>() };
    ASSERT_FEP3_NOERROR(fep3::core::arya::addJobsToJobRegistry(jobs, *_job_registry_mock));

    IJob& job_interface = *jobs[0];
    MySimpleJob::_counter = 0;
    ASSERT_FEP3_NOERROR(job_interface.execute(0ms));
    EXPECT_EQ(MySimpleJob::_counter, 1);
}

/**
* @brief Add job created by DataJob(name, job_config)
*
*/
TEST_F(DataJobWithMocks, addJobCtor2)
{
    EXPECT_CALL(*_job_registry_mock, addJob("datajob", _, AllOf(
        Field(&JobConfiguration::_cycle_sim_time, Eq(55ms)),
        Field(&JobConfiguration::_delay_sim_time, Eq(20ms)),
        Field(&JobConfiguration::_runtime_violation_strategy, Eq(JobConfiguration::TimeViolationStrategy::skip_output_publish))
    ))).Times(1).WillOnce(Return(fep3::Result{}));

    JobConfiguration config(55ms, 20ms, {},
        JobConfiguration::TimeViolationStrategy::skip_output_publish);

    std::vector<std::shared_ptr<Job>> jobs{ std::make_shared<DataJob>("datajob", config) };
    ASSERT_FEP3_NOERROR(fep3::core::arya::addJobsToJobRegistry(jobs, *_job_registry_mock));
}

/**
* @brief Add job created by DataJob(name, cycle_time, fc)
*
*/
TEST_F(DataJobWithMocks, addJobCtor3)
{
    EXPECT_CALL(*_job_registry_mock, addJob("execjob", _, AllOf(
        Field(&JobConfiguration::_cycle_sim_time, Eq(33ms)),
        Field(&JobConfiguration::_delay_sim_time, Eq(0ms)),
        Field(&JobConfiguration::_runtime_violation_strategy, Eq(JobConfiguration::TimeViolationStrategy::ignore_runtime_violation))
    ))).Times(1).WillOnce(Return(fep3::Result{}));

    int val = 5;
    Job::ExecuteCallback fc = [&val](fep3::arya::Timestamp)
    {
        ++val;
        return fep3::Result{};
    };

    std::vector<std::shared_ptr<Job>> jobs{ std::make_shared<DataJob>("execjob", 33ms, fc) };
    ASSERT_FEP3_NOERROR(fep3::core::arya::addJobsToJobRegistry(jobs, *_job_registry_mock));

    IJob& job_interface = *jobs[0];
    ASSERT_FEP3_NOERROR(job_interface.execute(10ms));
    EXPECT_EQ(val, 6);
}

/**
* @brief Add job created by DataJob(name, job_config, fc)
*
*/
TEST_F(DataJobWithMocks, addJobCtor4)
{
    EXPECT_CALL(*_job_registry_mock, addJob("execjobconfig", _, AllOf(
        Field(&JobConfiguration::_cycle_sim_time, Eq(123ms)),
        Field(&JobConfiguration::_delay_sim_time, Eq(71ms)),
        Field(&JobConfiguration::_runtime_violation_strategy, Eq(JobConfiguration::TimeViolationStrategy::skip_output_publish))
    ))).Times(1).WillOnce(Return(fep3::Result{}));

    int val = 100;
    Job::ExecuteCallback fc = [&val](fep3::arya::Timestamp)
    {
        --val;
        return fep3::Result{};
    };

    JobConfiguration config(123ms, 71ms, {},
        JobConfiguration::TimeViolationStrategy::skip_output_publish);

    std::vector<std::shared_ptr<Job>> jobs{ std::make_shared<DataJob>("execjobconfig", config, fc) };
    ASSERT_FEP3_NOERROR(fep3::core::arya::addJobsToJobRegistry(jobs, *_job_registry_mock));

    IJob& job_interface = *jobs[0];
    ASSERT_FEP3_NOERROR(job_interface.execute(71ms));
    EXPECT_EQ(val, 99);
}

/**
* @brief test addDataIn
*
*/
TEST(DataJob, addDataIn)
{
    MySimpleJob job;
    auto data_in = job.addDataIn("reader", fep3::base::StreamTypeString());
    EXPECT_EQ(data_in->getName(), "reader");
    EXPECT_EQ(data_in->readType()->getMetaTypeName(), "ascii-string");
    EXPECT_EQ(data_in->getSampleQueueCapacity(), 2u);
    EXPECT_EQ(data_in->getSampleQueueSize(), 0u);
}

/**
* @brief test addDataIn with queue size and reconfigureDataIn
*
*/
TEST(DataJob, addDataInQueueSize)
{
    MySimpleJob job;
    auto data_in = job.addDataIn("reader", fep3::base::StreamTypeString(), 20u);
    EXPECT_EQ(data_in->getName(), "reader");
    EXPECT_EQ(data_in->readType()->getMetaTypeName(), "ascii-string");
    EXPECT_EQ(data_in->getSampleQueueCapacity(), 20u);
    EXPECT_EQ(data_in->getSampleQueueSize(), 0u);

    ASSERT_FEP3_NOERROR(job.reconfigureDataIn("reader", 10u));
    EXPECT_EQ(data_in->getSampleQueueCapacity(), 10u);
    EXPECT_EQ(data_in->getSampleQueueSize(), 0u);
}

/**
* @brief test addDataOut
*
*/
TEST(DataJob, addDataOut)
{
    MySimpleJob job;
    auto data_out = job.addDataOut("writer", fep3::base::StreamTypeString());
    EXPECT_EQ(data_out->getName(), "writer");
    EXPECT_EQ(data_out->getQueueSize(), fep3::core::arya::DATA_WRITER_QUEUE_SIZE_DEFAULT);
}

/**
* @brief test addDataOut with queue size
*
*/
TEST(DataJob, addDataOutQueueSize)
{
    MySimpleJob job;
    auto data_out = job.addDataOut("writer", fep3::base::StreamTypeString(), 100u);
    EXPECT_EQ(data_out->getName(), "writer");
    EXPECT_EQ(data_out->getQueueSize(), 100u);
}

/**
* @brief test failing addDataOut with queue size = 0
*
*/
TEST(DataJob, addDataOutQueueSizeFail)
{
    MySimpleJob job;
    EXPECT_THROW(job.addDataOut("writer", fep3::base::StreamTypeString(), 0u), std::runtime_error);
}

/**
* @brief test addDynamicDataOut
*
*/
TEST(DataJob, addDynamicDataOut)
{
    MySimpleJob job;
    auto data_out = job.addDynamicDataOut("writer", fep3::base::StreamTypeString());
    EXPECT_EQ(data_out->getName(), "writer");
    EXPECT_EQ(data_out->getQueueSize(), fep3::core::arya::DATA_WRITER_QUEUE_SIZE_DYNAMIC);
}

namespace fep3 {
namespace core {
namespace arya {
// if the parameter is IClockService pointer, writeByType will act like a _clock getter function
// nasty trick to get a private member variable through a template member function specialization
// temporary solution until _clock will become a shared pointer
template<>
fep3::Result DataWriter::writeByType<fep3::IClockService*>(fep3::IClockService*& clock)
{
    clock = _clock;
    return {};
}
}
}
}

/**
* @brief test addDataToComponents and removeDataFromComponents
*
*/
TEST_F(DataJobWithMocks, addAndRemoveDataFromAndToComponents)
{
    MySimpleJob job;
    auto data_in = job.addDataIn("reader", fep3::base::StreamTypeString());
    EXPECT_EQ(data_in->getName(), "reader");
    auto data_out = job.addDataOut("writer", fep3::base::StreamTypeString());
    EXPECT_EQ(data_out->getName(), "writer");

    auto dataregistry_reader = new DataRegistryDataReader();
    auto dataregistry_writer = new DataRegistryDataWriter();

    EXPECT_CALL(*_data_registry_mock, getReaderProxy("reader", 2u)).Times(1)
        .WillOnce(Return(dataregistry_reader));
    EXPECT_CALL(*_data_registry_mock, getWriterProxy("writer", 1u)).Times(1)
        .WillOnce(Return(dataregistry_writer));

    EXPECT_CALL(*_data_registry_mock, registerDataIn("reader", _, false)).Times(1)
        .WillOnce(Return(fep3::Result{}));
    EXPECT_CALL(*_data_registry_mock, registerDataOut("writer", _, false)).Times(1)
        .WillOnce(Return(fep3::Result{}));

    // checking if writer uses the clock service
    EXPECT_CALL(*_clock_service_mock, getTime()).Times(1)
        .WillOnce(Return(15ms));

    auto is_15ms = [](const IDataSample& sample) { return sample.getTime() == 15ms; };
    EXPECT_CALL(*dataregistry_writer, write(Matcher<const IDataSample&>(Truly(is_15ms)))).Times(1)
        .WillOnce(Return(fep3::Result{}));

    ASSERT_FEP3_NOERROR(job.addDataToComponents(*_component_registry));
    ASSERT_FEP3_NOERROR(data_out->write(DataSample()));

    fep3::IClockService* clock_service;
    ASSERT_FEP3_NOERROR(data_out->writeByType<fep3::IClockService*>(clock_service));
    EXPECT_EQ(clock_service, _clock_service_mock.get());

    const auto& dataregistry_reader_destruction_checker = dataregistry_reader->getDestructionChecker();
    EXPECT_DESTRUCTION(*dataregistry_reader);
    const auto& dataregistry_writer_destruction_checker = dataregistry_writer->getDestructionChecker();
    EXPECT_DESTRUCTION(*dataregistry_writer);

    EXPECT_CALL(*_data_registry_mock, unregisterDataIn("reader")).Times(1)
        .WillOnce(Return(fep3::Result{}));
    EXPECT_CALL(*_data_registry_mock, unregisterDataOut("writer")).Times(1)
        .WillOnce(Return(fep3::Result{}));

    ASSERT_FEP3_NOERROR(job.removeDataFromComponents(*_component_registry));
    ::testing::Mock::VerifyAndClearExpectations(dataregistry_reader_destruction_checker.get());
    ::testing::Mock::VerifyAndClearExpectations(dataregistry_writer_destruction_checker.get());

    ASSERT_FEP3_RESULT(data_out->write(DataSample()), fep3::ERR_NOT_CONNECTED);

    ASSERT_FEP3_NOERROR(data_out->writeByType<fep3::IClockService*>(clock_service));
    EXPECT_EQ(clock_service, nullptr);
}

/**
* @brief test addDataToComponents returns the correct result in case of an error happending during registration of a data writer
* and removes a previously registered data reader from the data registry during rollback
*/
TEST_F(DataJobWithLoggingService, addDataToComponentsFails)
{
    MySimpleJob job;
    auto data_in = job.addDataIn("reader_success", fep3::base::StreamTypeString());
    EXPECT_EQ(data_in->getName(), "reader_success");
    auto data_out = job.addDataOut("writer_error", fep3::base::StreamTypeString());
    EXPECT_EQ(data_out->getName(), "writer_error");

    auto dataregistry_reader = new DataRegistryDataReader();

    EXPECT_CALL(*_data_registry_mock, getReaderProxy("reader_success", 2u)).Times(1)
        .WillOnce(Return(dataregistry_reader));
    EXPECT_CALL(*_data_registry_mock, registerDataIn("reader_success", _, false)).Times(1)
        .WillOnce(Return(fep3::Result{}));

    EXPECT_CALL(*_data_registry_mock, registerDataOut("writer_error", _, false)).Times(1)
        .WillOnce(Return(fep3::Result{fep3::ERR_DEVICE_NOT_READY}));
    EXPECT_CALL(*_logger_mock, logError(fep3::mock::LogStringRegexMatcher
        (std::string() + "An error occurred during registration .* Trying to rollback.")))
        .WillOnce(::testing::Return(fep3::Result{}));

    EXPECT_CALL(*_data_registry_mock, unregisterDataIn("reader_success")).Times(1)
        .WillOnce(Return(fep3::Result{}));
	EXPECT_CALL(*_data_registry_mock, unregisterDataOut("writer_error")).Times(1)
		.WillOnce(Return(fep3::Result{}));

    ASSERT_FEP3_RESULT_WITH_MESSAGE(job.addDataToComponents(*_component_registry),
                                    fep3::Result{fep3::ERR_DEVICE_NOT_READY},
                                    "could not register data writer");
}

/**
* @brief test removeDataFromComponents returns the correct result in case of a single error happending during removal of data writers
* and data readers
*/
TEST_F(DataJobWithLoggingService, removeDataFromComponentsFailsSingleError)
{
    MySimpleJob job;
    auto data_in = job.addDataIn("reader", fep3::base::StreamTypeString());
    EXPECT_EQ(data_in->getName(), "reader");
    auto data_out = job.addDataOut("writer", fep3::base::StreamTypeString());
    EXPECT_EQ(data_out->getName(), "writer");

    auto dataregistry_reader = new DataRegistryDataReader();
    auto dataregistry_writer = new DataRegistryDataWriter();

    EXPECT_CALL(*_data_registry_mock, getReaderProxy("reader", 2u)).Times(1)
        .WillOnce(Return(dataregistry_reader));
    EXPECT_CALL(*_data_registry_mock, registerDataIn("reader", _, false)).Times(1)
        .WillOnce(Return(fep3::Result{}));
    EXPECT_CALL(*_data_registry_mock, getWriterProxy("writer", 1u)).Times(1)
        .WillOnce(Return(dataregistry_writer));
    EXPECT_CALL(*_data_registry_mock, registerDataOut("writer", _, false)).Times(1)
        .WillOnce(Return(fep3::Result{}));

    ASSERT_FEP3_NOERROR(job.addDataToComponents(*_component_registry));

    EXPECT_CALL(*_data_registry_mock, unregisterDataIn("reader")).Times(1)
        .WillOnce(Return(fep3::Result{}));
    EXPECT_CALL(*_data_registry_mock, unregisterDataOut("writer")).Times(1)
        .WillOnce(Return(fep3::Result{fep3::ERR_NOT_FOUND}));
    EXPECT_CALL(*_logger_mock, logError(fep3::mock::LogStringRegexMatcher
        (std::string() + "Removing 'writer' failed because of: '-20' - ''")))
        .WillOnce(::testing::Return(fep3::Result{}));

    ASSERT_FEP3_RESULT(job.removeDataFromComponents(*_component_registry),
                                    fep3::Result{fep3::ERR_NOT_FOUND});
}

/**
* @brief test removeDataFromComponents returns the correct result in case of multiple errors happending during removal of data writers
* and data readers
*/
TEST_F(DataJobWithLoggingService, removeDataFromComponentsFailsMultipleErrors)
{
    MySimpleJob job;
    auto data_in = job.addDataIn("reader", fep3::base::StreamTypeString());
    EXPECT_EQ(data_in->getName(), "reader");
    auto data_out = job.addDataOut("writer", fep3::base::StreamTypeString());
    EXPECT_EQ(data_out->getName(), "writer");

    auto dataregistry_reader = new DataRegistryDataReader();
    auto dataregistry_writer = new DataRegistryDataWriter();

    EXPECT_CALL(*_data_registry_mock, getReaderProxy("reader", 2u)).Times(1)
        .WillOnce(Return(dataregistry_reader));
    EXPECT_CALL(*_data_registry_mock, registerDataIn("reader", _, false)).Times(1)
        .WillOnce(Return(fep3::Result{}));
    EXPECT_CALL(*_data_registry_mock, getWriterProxy("writer", 1u)).Times(1)
        .WillOnce(Return(dataregistry_writer));
    EXPECT_CALL(*_data_registry_mock, registerDataOut("writer", _, false)).Times(1)
        .WillOnce(Return(fep3::Result{}));

    ASSERT_FEP3_NOERROR(job.addDataToComponents(*_component_registry));

    EXPECT_CALL(*_data_registry_mock, unregisterDataIn("reader")).Times(1)
        .WillOnce(Return(fep3::Result{fep3::ERR_NOT_FOUND}));
    EXPECT_CALL(*_data_registry_mock, unregisterDataOut("writer")).Times(1)
        .WillOnce(Return(fep3::Result{fep3::ERR_NOT_FOUND}));
    EXPECT_CALL(*_logger_mock, logError(fep3::mock::LogStringRegexMatcher
        (std::string() + "Removing 'reader' failed because of: '-20' - ''")))
        .WillOnce(::testing::Return(fep3::Result{}));
    EXPECT_CALL(*_logger_mock, logError(fep3::mock::LogStringRegexMatcher
        (std::string() + "Removing 'writer' failed because of: '-20' - ''")))
        .WillOnce(::testing::Return(fep3::Result{}));

    ASSERT_FEP3_RESULT_WITH_MESSAGE(job.removeDataFromComponents(*_component_registry),
                                    fep3::Result{fep3::ERR_FAILED},
                                    "Multiple errors occurred during removal of data writers and data readers from data job 'myjob'. Have a look at the logs for further information.");
}

/**
* @brief test executeDataIn
*
*/
TEST_F(DataJobWithMocks, executeDataIn)
{
    auto dataregistry_reader = new DataRegistryDataReader();

    EXPECT_CALL(*_data_registry_mock, registerDataIn("reader", _, false)).Times(1)
        .WillOnce(Return(fep3::Result{}));

    EXPECT_CALL(*_data_registry_mock, getReaderProxy("reader", 2u)).Times(1)
        .WillOnce(Return(dataregistry_reader));

    DataJob job("myJob", 50ms);
    auto data_in = job.addDataIn("reader", fep3::base::StreamTypeString());
    ASSERT_FEP3_NOERROR(data_in->addToDataRegistry(*_data_registry_mock));

    EXPECT_CALL(*dataregistry_reader, getFrontTime()).Times(7)
        .WillOnce(Return(fep3::Optional<fep3::Timestamp>(20ms)))
        .WillOnce(Return(fep3::Optional<fep3::Timestamp>(20ms)))
        .WillOnce(Return(fep3::Optional<fep3::Timestamp>(30ms)))
        .WillOnce(Return(fep3::Optional<fep3::Timestamp>(30ms)))
        .WillOnce(Return(fep3::Optional<fep3::Timestamp>(30ms)))
        .WillOnce(Return(fep3::Optional<fep3::Timestamp>(30ms)))
        .WillRepeatedly(Return(fep3::Optional<fep3::Timestamp>()));
    auto is_data_in = [&data_in](fep3::IDataRegistry::IDataReceiver& arg) { return &arg == data_in; };
    EXPECT_CALL(*dataregistry_reader, pop(Truly(is_data_in))).Times(2)
        .WillRepeatedly(Return(fep3::Result{}));
    EXPECT_DESTRUCTION(*dataregistry_reader);

    IJob& job_intf = job;

    // pops 20ms sample (2 calls) and skips 30ms sample (2 calls)
    ASSERT_FEP3_NOERROR(job_intf.executeDataIn(25ms));

    // pops 30ms sample (2 calls) and exits on "no value" case
    ASSERT_FEP3_NOERROR(job_intf.executeDataIn(35ms));
}

/**
* @brief test executeDataOut
*
*/
TEST_F(DataJobWithMocks, executeDataOut)
{
    auto dataregistry_writer = new DataRegistryDataWriter();

    EXPECT_CALL(*_data_registry_mock, registerDataOut("writer", _, false)).Times(1)
        .WillOnce(Return(fep3::Result{}));

    EXPECT_CALL(*_data_registry_mock, getWriterProxy("writer", 1u)).Times(1)
        .WillOnce(Return(dataregistry_writer));

    DataJob job("writerJob", 50ms);
    auto data_out = job.addDataOut("writer", fep3::base::StreamTypeString());
    ASSERT_FEP3_NOERROR(data_out->addToDataRegistry(*_data_registry_mock));

    EXPECT_CALL(*dataregistry_writer, flush()).Times(1)
        .WillRepeatedly(Return(fep3::Result{}));

    IJob& job_intf = job;
    ASSERT_FEP3_NOERROR(job_intf.executeDataOut(100ms));

    EXPECT_DESTRUCTION(*dataregistry_writer);

}

/**
* @brief test DataReader constructors
*
*/
TEST(DataReader, constructors)
{
    DataReader default_instance{};
    EXPECT_EQ(default_instance.getName(), "");
    EXPECT_EQ(default_instance.readType()->getMetaTypeName(), "anonymous");
    EXPECT_EQ(default_instance.getSampleQueueCapacity(), 2u);
    EXPECT_EQ(default_instance.getSampleQueueSize(), 0u);

    DataReader name_and_stream_type{ "reader0", fep3::base::StreamTypePlain<uint16_t>() };
    EXPECT_EQ(name_and_stream_type.getName(), "reader0");
    EXPECT_EQ(name_and_stream_type.readType()->getMetaTypeName(), "plain-ctype");
    EXPECT_EQ(name_and_stream_type.getSampleQueueCapacity(), 2u);
    EXPECT_EQ(name_and_stream_type.getSampleQueueSize(), 0u);

    DataReader name_stream_type_queuesize{ "reader1", fep3::base::StreamTypeString(), 15u };
    EXPECT_EQ(name_stream_type_queuesize.getName(), "reader1");
    EXPECT_EQ(name_stream_type_queuesize.readType()->getMetaTypeName(), "ascii-string");
    EXPECT_EQ(name_stream_type_queuesize.getSampleQueueCapacity(), 15u);
    EXPECT_EQ(name_stream_type_queuesize.getSampleQueueSize(), 0u);

    // cannot explicitely specify template argument for constructor, so
    // template<typename PLAIN_RAW_TYPE> DataReader(std::string name) and
    // template<typename PLAIN_RAW_TYPE> DataReader(std::string name, size_t queue_capacity)
    // cannot be tested

    DataReader copied = name_stream_type_queuesize;
    EXPECT_EQ(copied.getName(), "reader1");
    EXPECT_EQ(copied.readType()->getMetaTypeName(), "ascii-string");
    EXPECT_EQ(copied.getSampleQueueCapacity(), 15u);
    EXPECT_EQ(copied.getSampleQueueSize(), 0u);
}

/**
* @brief test DataReader::addToDataRegistry, removeFromDataRegistry and receiveNow
*
*/
TEST_F(DataJobWithMocks, dataReaderAddToDataRegistryRemoveFromDataRegistry)
{
    DataReader reader{ "reader", fep3::base::StreamTypeString(), 15u };

    auto dataregistry_reader = new DataRegistryDataReader();

    EXPECT_CALL(*_data_registry_mock, registerDataIn("reader", _, false)).Times(1)
        .WillOnce(Return(fep3::Result{}));

    EXPECT_CALL(*_data_registry_mock, getReaderProxy("reader", 15u)).Times(1)
        .WillOnce(Return(dataregistry_reader));

    ASSERT_FEP3_NOERROR(reader.addToDataRegistry(*_data_registry_mock));

    EXPECT_CALL(*dataregistry_reader, getFrontTime()).Times(5)
        .WillOnce(Return(fep3::Optional<fep3::Timestamp>(20ms)))
        .WillOnce(Return(fep3::Optional<fep3::Timestamp>(20ms)))
        .WillOnce(Return(fep3::Optional<fep3::Timestamp>(20ms)))
        .WillOnce(Return(fep3::Optional<fep3::Timestamp>(20ms)))
        .WillRepeatedly(Return(fep3::Optional<fep3::Timestamp>()));

    fep3::IDataRegistry::IDataReceiver* data_in = &reader;
    auto is_data_in = [&data_in](fep3::IDataRegistry::IDataReceiver& arg) { return &arg == data_in; };

    EXPECT_CALL(*dataregistry_reader, pop(Truly(is_data_in))).Times(1)
        .WillRepeatedly(Return(fep3::Result{}));

    reader.receiveNow(15ms);
    reader.receiveNow(25ms);

    EXPECT_CALL(*_data_registry_mock, unregisterDataIn("reader")).Times(1)
        .WillOnce(Return(fep3::Result{}));

    const auto& dataregistry_reader_destruction_checker = dataregistry_reader->getDestructionChecker();
    EXPECT_DESTRUCTION(*dataregistry_reader);

    ASSERT_FEP3_NOERROR(reader.removeFromDataRegistry(*_data_registry_mock));
    ::testing::Mock::VerifyAndClearExpectations(dataregistry_reader_destruction_checker.get());
}

/**
* @brief test DataReader streaming operator (>>) to values
*
*/
TEST(DataReader, rightShiftValue)
{
    DataReader reader{ "reader2", fep3::base::StreamTypePlain<int16_t>() };

    // read data to memory
    auto data_sample_1 = new DataSample();
    auto data_sample_2 = new DataSample();
    int a = 20, b = 30;
    data_sample_1->set(&a, sizeof(a));
    data_sample_2->set(&b, sizeof(b));
    fep3::arya::data_read_ptr<IDataSample> idata_sample_1(data_sample_1);
    fep3::arya::data_read_ptr<IDataSample> idata_sample_2(data_sample_2);

    reader(idata_sample_1);
    reader(idata_sample_2);
    int x,y;
    EXPECT_EQ(&(reader >> x >> y), &reader);
    EXPECT_EQ(x, a);
    EXPECT_EQ(y, b);

    reader(idata_sample_1);
    reader(idata_sample_2);

    // optional argument for output
    fep3::arya::Optional<int> xo, yo;
    // output exist
    EXPECT_EQ(&(reader >> xo >> yo), &reader);
    EXPECT_EQ(bool(xo), true);
    EXPECT_EQ(bool(yo), true);
    EXPECT_EQ(xo.value(), a);
    EXPECT_EQ(yo.value(), b);

    // output empty
    EXPECT_EQ(&(reader >> xo), &reader);
    EXPECT_EQ(bool(xo), false);
    EXPECT_NE(xo.value_or(0), a);

    // read and copy type
    fep3::arya::data_read_ptr<const fep3::arya::IStreamType> stream_type(new fep3::base::StreamTypeString());
    reader(stream_type);
    fep3::base::arya::StreamType read_type(fep3::base::arya::meta_type_raw);
    EXPECT_EQ(&(reader >> read_type), &reader);
    EXPECT_EQ(read_type.getMetaTypeName(), "ascii-string");
}

/**
* @brief test DataReader streaming operator (>>) to pointers
*
*/
TEST(DataReader, rightShiftPointer)
{
    DataReader reader{ "reader3", fep3::base::StreamTypePlain<int16_t>() };

    auto data_sample = new DataSample();
    int a = 20;
    data_sample->set(&a, sizeof(a));
    fep3::arya::data_read_ptr<IDataSample> idata_sample(data_sample);
    reader(idata_sample);

    fep3::arya::data_read_ptr<const IDataSample> read_idata_sample;
    EXPECT_EQ(&(reader >> read_idata_sample), &reader);
    EXPECT_EQ(read_idata_sample->getSize(), 4);
    int read_value;
    fep3::base::DataSampleType<int> sample_wrapup(read_value);
    ASSERT_EQ(read_idata_sample->read(sample_wrapup), 4u);
    EXPECT_EQ(a, read_value);

    fep3::arya::data_read_ptr<const fep3::arya::IStreamType> stream_type(new fep3::base::StreamTypeString());
    reader(stream_type);
    fep3::data_read_ptr<const fep3::arya::IStreamType> read_stream_type;
    EXPECT_EQ(&(reader >> read_stream_type), &reader);
    EXPECT_EQ(read_stream_type->getMetaTypeName(), "ascii-string");
}

/**
* @brief test DataWriter default constructor
*
*/
TEST(DataWriter, defaultConstructor)
{
    std::shared_ptr<DataRegistryComponent> data_registry_mock{ std::make_unique<DataRegistryComponent>() };

    DataWriter default_instance{};

    EXPECT_EQ(default_instance.getName(), "");
    EXPECT_EQ(default_instance.getQueueSize(), fep3::core::arya::DATA_WRITER_QUEUE_SIZE_DYNAMIC);

    auto is_anonymous_stream = [](const fep3::arya::IStreamType& stream)
    {
        return stream.getMetaTypeName() == "anonymous";
    };
    EXPECT_CALL(*data_registry_mock, registerDataOut("", Truly(is_anonymous_stream), false)).Times(1)
        .WillOnce(Return(fep3::Result{}));

    auto dataregistry_writer = new DataRegistryDataWriter();
    EXPECT_CALL(*data_registry_mock, getWriterProxy("", 0u)).Times(1)
        .WillOnce(Return(dataregistry_writer));

    ASSERT_FEP3_NOERROR(default_instance.addToDataRegistry(*data_registry_mock));

    EXPECT_DESTRUCTION(*dataregistry_writer);
}

/**
* @brief test DataWriter(std::string name, const StreamType& stream_type) constructor
*
*/
TEST(DataWriter, constructorNameStreamtype)
{
    std::shared_ptr<DataRegistryComponent> data_registry_mock{ std::make_unique<DataRegistryComponent>() };

    DataWriter writer{"writer1", fep3::base::StreamTypePlain<uint16_t>() };

    EXPECT_EQ(writer.getName(), "writer1");
    EXPECT_EQ(writer.getQueueSize(), fep3::core::arya::DATA_WRITER_QUEUE_SIZE_DYNAMIC);

    auto is_plainc_stream = [](const fep3::arya::IStreamType& stream)
    {
        return stream.getMetaTypeName() == "plain-ctype";
    };
    EXPECT_CALL(*data_registry_mock, registerDataOut("writer1", Truly(is_plainc_stream), false)).Times(1)
        .WillOnce(Return(fep3::Result{}));

    auto dataregistry_writer = new DataRegistryDataWriter();
    EXPECT_CALL(*data_registry_mock, getWriterProxy("writer1", 0u)).Times(1)
        .WillOnce(Return(dataregistry_writer));

    ASSERT_FEP3_NOERROR(writer.addToDataRegistry(*data_registry_mock));

    EXPECT_DESTRUCTION(*dataregistry_writer);
}

/**
* @brief test DataWriter::DataWriter(std::string name, const StreamType & stream_type, size_t queue_size) constructor
*
*/
TEST(DataWriter, constructorNameStreamtypeQueuesize)
{
    std::shared_ptr<DataRegistryComponent> data_registry_mock{ std::make_unique<DataRegistryComponent>() };

    DataWriter writer{ "writer2", fep3::base::StreamTypeString(), 5u };

    EXPECT_EQ(writer.getName(), "writer2");
    EXPECT_EQ(writer.getQueueSize(), 5u);

    auto is_string_stream = [](const fep3::arya::IStreamType& stream)
    {
        return stream.getMetaTypeName() == "ascii-string";
    };
    EXPECT_CALL(*data_registry_mock, registerDataOut("writer2", Truly(is_string_stream), false)).Times(1)
        .WillOnce(Return(fep3::Result{}));

    auto dataregistry_writer = new DataRegistryDataWriter();
    EXPECT_CALL(*data_registry_mock, getWriterProxy("writer2", 5u)).Times(1)
        .WillOnce(Return(dataregistry_writer));

    ASSERT_FEP3_NOERROR(writer.addToDataRegistry(*data_registry_mock));

    EXPECT_DESTRUCTION(*dataregistry_writer);
}

/**
* @brief test DataWriter copy constructor
*
*/
TEST(DataWriter, copyConstructor)
{
    std::shared_ptr<DataRegistryComponent> data_registry_mock{ std::make_unique<DataRegistryComponent>() };

    DataWriter writer_original{ "writer3", fep3::base::StreamTypeString(), 5u };
    DataWriter writer(writer_original);

    EXPECT_EQ(writer.getName(), "writer3");
    EXPECT_EQ(writer.getQueueSize(), 5u);

    auto is_string_stream = [](const fep3::arya::IStreamType& stream)
    {
        return stream.getMetaTypeName() == "ascii-string";
    };
    EXPECT_CALL(*data_registry_mock, registerDataOut("writer3", Truly(is_string_stream), false)).Times(1)
        .WillOnce(Return(fep3::Result{}));

    auto dataregistry_writer = new DataRegistryDataWriter();
    EXPECT_CALL(*data_registry_mock, getWriterProxy("writer3", 5u)).Times(1)
        .WillOnce(Return(dataregistry_writer));

    ASSERT_FEP3_NOERROR(writer.addToDataRegistry(*data_registry_mock));

    EXPECT_DESTRUCTION(*dataregistry_writer);
}

/**
* @brief test DataWriter assignment operator
*
*/
TEST(DataWriter, assignmentOperator)
{
    std::shared_ptr<DataRegistryComponent> data_registry_mock{ std::make_unique<DataRegistryComponent>() };

    DataWriter writer_original{ "writer4", fep3::base::StreamTypeString(), 5u };
    DataWriter writer;
    writer = writer_original;

    EXPECT_EQ(writer.getName(), "writer4");
    EXPECT_EQ(writer.getQueueSize(), 5u);

    auto is_string_stream = [](const fep3::arya::IStreamType& stream)
    {
        return stream.getMetaTypeName() == "ascii-string";
    };
    EXPECT_CALL(*data_registry_mock, registerDataOut("writer4", Truly(is_string_stream), false)).Times(1)
        .WillOnce(Return(fep3::Result{}));

    auto dataregistry_writer = new DataRegistryDataWriter();
    EXPECT_CALL(*data_registry_mock, getWriterProxy("writer4", 5u)).Times(1)
        .WillOnce(Return(dataregistry_writer));

    ASSERT_FEP3_NOERROR(writer.addToDataRegistry(*data_registry_mock));

    EXPECT_DESTRUCTION(*dataregistry_writer);
}

/**
* @brief test DataWriter move constructor
*
*/
TEST(DataWriter, moveConstructor)
{
    std::shared_ptr<DataRegistryComponent> data_registry_mock{ std::make_unique<DataRegistryComponent>() };

    DataWriter writer_original{ "writer5", fep3::base::StreamTypeString(), 5u };
    DataWriter writer(std::move(writer_original));

    EXPECT_EQ(writer.getName(), "writer5");
    EXPECT_EQ(writer.getQueueSize(), 5u);

    auto is_string_stream = [](const fep3::arya::IStreamType& stream)
    {
        return stream.getMetaTypeName() == "ascii-string";
    };
    EXPECT_CALL(*data_registry_mock, registerDataOut("writer5", Truly(is_string_stream), false)).Times(1)
        .WillOnce(Return(fep3::Result{}));

    auto dataregistry_writer = new DataRegistryDataWriter();
    EXPECT_CALL(*data_registry_mock, getWriterProxy("writer5", 5u)).Times(1)
        .WillOnce(Return(dataregistry_writer));

    ASSERT_FEP3_NOERROR(writer.addToDataRegistry(*data_registry_mock));

    EXPECT_DESTRUCTION(*dataregistry_writer);
}

/**
* @brief test DataWriter move assignment
*
*/
TEST(DataWriter, moveAssignment)
{
    std::shared_ptr<DataRegistryComponent> data_registry_mock{ std::make_unique<DataRegistryComponent>() };

    DataWriter writer_original{ "writer6", fep3::base::StreamTypeString(), 5u };
    DataWriter writer;
    writer = std::move(writer_original);

    EXPECT_EQ(writer.getName(), "writer6");
    EXPECT_EQ(writer.getQueueSize(), 5u);

    auto is_string_stream = [](const fep3::arya::IStreamType& stream)
    {
        return stream.getMetaTypeName() == "ascii-string";
    };
    EXPECT_CALL(*data_registry_mock, registerDataOut("writer6", Truly(is_string_stream), false)).Times(1)
        .WillOnce(Return(fep3::Result{}));

    auto dataregistry_writer = new DataRegistryDataWriter();
    EXPECT_CALL(*data_registry_mock, getWriterProxy("writer6", 5u)).Times(1)
        .WillOnce(Return(dataregistry_writer));

    ASSERT_FEP3_NOERROR(writer.addToDataRegistry(*data_registry_mock));

    EXPECT_DESTRUCTION(*dataregistry_writer);
}

/**
* @brief test DataWriter addToDataRegistry and removeFromDataRegistry
*
*/
TEST(DataWriter, addToDataRegistryRemoveFromDataRegistry)
{
    std::shared_ptr<DataRegistryComponent> data_registry_mock{ std::make_unique<DataRegistryComponent>() };

    DataWriter writer{};

    EXPECT_CALL(*data_registry_mock, registerDataOut("", _, false)).Times(1)
        .WillOnce(Return(fep3::Result{}));

    auto dataregistry_writer = new DataRegistryDataWriter();
    EXPECT_CALL(*data_registry_mock, getWriterProxy("", 0u)).Times(1)
        .WillOnce(Return(dataregistry_writer));

    ASSERT_FEP3_NOERROR(writer.addToDataRegistry(*data_registry_mock));

    EXPECT_CALL(*data_registry_mock, unregisterDataOut("")).Times(1)
        .WillOnce(Return(fep3::Result{}));

    const auto& dataregistry_writer_destruction_checker = dataregistry_writer->getDestructionChecker();
    EXPECT_DESTRUCTION(*dataregistry_writer);

    ASSERT_FEP3_NOERROR(writer.removeFromDataRegistry(*data_registry_mock));
    ::testing::Mock::VerifyAndClearExpectations(dataregistry_writer_destruction_checker.get());
}

/**
* @brief test DataWriter addClock and removeClock
*
*/
TEST(DataWriter, addClockRemoveClock)
{
    DataWriter writer{};
    std::shared_ptr<ClockMockComponent> clock_service_mock(std::make_unique<ClockMockComponent>());
    ASSERT_FEP3_NOERROR(writer.addClock(*clock_service_mock));

    fep3::IClockService& clock_service_intf = *clock_service_mock;
    fep3::IClockService* stored_clock_service;
    // this is actually a getter for _clock
    ASSERT_FEP3_NOERROR(writer.writeByType<fep3::IClockService*>(stored_clock_service));
    EXPECT_EQ(stored_clock_service, &clock_service_intf);

    ASSERT_FEP3_NOERROR(writer.removeClock());
    ASSERT_FEP3_NOERROR(writer.writeByType<fep3::IClockService*>(stored_clock_service));
    EXPECT_EQ(stored_clock_service, nullptr);
}

/**
* @brief test DataWriter write functions (with flushNow)
*
*/
TEST(DataWriter, write)
{
    DataWriter writer{ "writer7", fep3::base::StreamTypeString() };
    ASSERT_FEP3_RESULT(writer.write(DataSample()), fep3::ERR_NOT_CONNECTED);

    std::shared_ptr<DataRegistryComponent> data_registry_mock{ std::make_unique<DataRegistryComponent>() };
    std::shared_ptr<ClockMockComponent> clock_service_mock(std::make_unique<ClockMockComponent>());

    auto dataregistry_writer = new DataRegistryDataWriter();

    EXPECT_CALL(*data_registry_mock, registerDataOut("writer7", _, false)).Times(1)
        .WillOnce(Return(fep3::Result{}));

    EXPECT_CALL(*data_registry_mock, getWriterProxy("writer7", 0u)).Times(1)
        .WillOnce(Return(dataregistry_writer));

    ASSERT_FEP3_NOERROR(writer.addToDataRegistry(*data_registry_mock));
    ASSERT_FEP3_NOERROR(writer.addClock(*clock_service_mock));

    EXPECT_CALL(*clock_service_mock, getTime()).Times(1)
        .WillOnce(Return(15ms));

    auto is_15ms = [](const IDataSample& sample) { return sample.getTime() == 15ms; };
    EXPECT_CALL(*dataregistry_writer, write(Matcher<const IDataSample&>(Truly(is_15ms))))
        .Times(1)
        .WillOnce(Return(fep3::Result{}));

    EXPECT_CALL(*dataregistry_writer, write(An<const fep3::arya::IStreamType&>()))
        .Times(1)
        .WillOnce(Return(fep3::Result{}));

    auto is_27ms = [](const IDataSample& sample) { return sample.getTime() == 27ms; };
    EXPECT_CALL(*dataregistry_writer, write(Matcher<const IDataSample&>(Truly(is_27ms))))
        .Times(1)
        .WillOnce(Return(fep3::Result{}));

    DataSample sample;
    ASSERT_FEP3_NOERROR(writer.write(sample));
    ASSERT_FEP3_NOERROR(writer.write(fep3::base::StreamTypePlain<int16_t>()));

    ASSERT_FEP3_NOERROR(writer.removeClock());
    ASSERT_FEP3_NOERROR(writer.write(27ms, nullptr, 0u));

    EXPECT_CALL(*dataregistry_writer, flush()).Times(1)
        .WillOnce(Return(fep3::Result{}));
    ASSERT_FEP3_NOERROR(writer.flushNow(30ms));

    EXPECT_DESTRUCTION(*dataregistry_writer);
}

/**
* @brief test addToComponents and removeFromComponents
*
*/
TEST_F(DataJobWithMocks, addToComponentsRemoveFromComponents)
{
    DataWriter writer{ "writer8", fep3::base::StreamTypeString(), 5u };
    auto dataregistry_writer = new DataRegistryDataWriter();

    EXPECT_CALL(*_data_registry_mock, getWriterProxy("writer8", 5u)).Times(1)
        .WillOnce(Return(dataregistry_writer));

    EXPECT_CALL(*_data_registry_mock, registerDataOut("writer8", _, false)).Times(1)
        .WillOnce(Return(fep3::Result{}));

    ASSERT_FEP3_NOERROR(addToComponents(writer, *_component_registry));

    fep3::IClockService* clock_service;
    ASSERT_FEP3_NOERROR(writer.writeByType<fep3::IClockService*>(clock_service));
    EXPECT_EQ(clock_service, _clock_service_mock.get());

    const auto& dataregistry_writer_destruction_checker = dataregistry_writer->getDestructionChecker();
    EXPECT_DESTRUCTION(*dataregistry_writer);
	EXPECT_CALL(*_data_registry_mock, unregisterDataOut("writer8")).Times(1)
		.WillOnce(Return(fep3::Result{}));

    ASSERT_FEP3_NOERROR(removeFromComponents(writer, *_component_registry));
    ::testing::Mock::VerifyAndClearExpectations(dataregistry_writer_destruction_checker.get());

    // _clock is not reset, so nothing to be checked
    //ASSERT_FEP3_NOERROR(writer.writeByType<fep3::IClockService*>(clock_service));
    //EXPECT_EQ(clock_service, nullptr);
}
