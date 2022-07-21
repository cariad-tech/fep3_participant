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


#include "data_registry_test_fixture.h"
#include "fep3/base/stream_type/default_stream_type.h"
#include "fep3/base/sample/data_sample.h"

using namespace testing;

// DDL Struct defined in the test description
struct TestStruct
{
    uint8_t first;
    uint8_t second;
};

static constexpr uint8_t first_test_value = 10;
static constexpr uint8_t second_test_value = 5;

// Receiver of the mapped target signal
class MyDataReceiver : public fep3::IDataRegistry::IDataReceiver
{
    void operator()(const fep3::data_read_ptr<const fep3::IStreamType>&) override {};
    void operator()(const fep3::data_read_ptr<const fep3::IDataSample>& sample) override
    {
        TestStruct test_struct{ 0,0 };
        fep3::base::RawMemoryStandardType<TestStruct> memory{test_struct};
        sample->read(memory);
        ASSERT_EQ(test_struct.first, first_test_value * 2); // The test map has a transformation (multiplication with 2) for this signal
        ASSERT_EQ(test_struct.second, second_test_value);
    };
};

struct MappingTester : public NativeDataRegistry
{
    MappingTester() : NativeDataRegistry()
    {}

    void captureDataReceiver(const std::shared_ptr<fep3::ISimulationBus::IDataReceiver>& data_receiver)
    {
        _data_receiver = data_receiver;
    }

    void SetUpSimulationBusMock(const std::string& signal_name)
    {
        // Give the DataRegistry an actual DataReader object to work with
        _source_reader = new fep3::mock::DataReader();
        EXPECT_CALL(*_simulation_bus, getReader_(signal_name, _, _))
            .WillOnce(Return(_source_reader));

        // Get a pointer to the data receiver that gets passed to the mocked simulation bus so
        // we can send samples to the data receiver without using the simulation bus
        EXPECT_CALL(*_source_reader, reset_(_))
            .WillOnce(Invoke(this, &MappingTester::captureDataReceiver));

        EXPECT_CALL(*_simulation_bus, startBlockingReception(_))
            .WillOnce(InvokeArgument<0>());
    }

    void SendAndReceiveSignal()
    {
        // Send signal
        TestStruct test_struct{ first_test_value, second_test_value };
        fep3::base::DataSample input{ sizeof(TestStruct), true };
        input.set(&test_struct, sizeof(TestStruct));
        (*_data_receiver)(std::make_shared<fep3::base::DataSample>(input));

        // Receive Signal
        MyDataReceiver receiver{};
        _target_reader->pop(receiver);
    }

    std::shared_ptr<fep3::ISimulationBus::IDataReceiver> _data_receiver;
    fep3::mock::DataReader* _source_reader{ nullptr };
    std::unique_ptr<fep3::IDataRegistry::IDataReader> _target_reader;
};

TEST_F(MappingTester, testSignalMappingIn)
{
    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(*_configuration_service, FEP3_DATA_REGISTRY_MAPPING_CONFIGURATION_FILE_PATH, TEST_FILE_DIR "test.map"));

    SetUpSimulationBusMock("source_signal_in");

    ASSERT_FEP3_NOERROR(_registry->registerDataIn("target_signal_in", fep3::base::StreamTypeDDLFileRef{ "tTestStruct", TEST_FILE_DIR "test.description"}));
    _target_reader = _registry->getReader("target_signal_in");
    ASSERT_TRUE(_target_reader);

    ASSERT_FEP3_NOERROR(_component_registry->initialize());
    ASSERT_FEP3_NOERROR(_component_registry->tense());
    ASSERT_FEP3_NOERROR(_component_registry->start());

    ASSERT_EQ(_registry->getSignalInNames().size(), 1);
    ASSERT_STREQ(_registry->getSignalInNames()[0].c_str(), "source_signal_in");

    SendAndReceiveSignal();

    ASSERT_FEP3_NOERROR(_component_registry->stop());
    ASSERT_FEP3_NOERROR(_component_registry->relax());
    ASSERT_FEP3_NOERROR(_component_registry->deinitialize());
}

/**
 * @brief Check whether a mapped signal may be unregistered from the native DataRegistry.
 * A mapped signal is stored separated from the not mapped signals but shall be unregistered
 * using the same functionality.
 * 
 */
TEST_F(MappingTester, testUnregisterMappedSignal)
{
	ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(
		*_configuration_service,
		FEP3_DATA_REGISTRY_MAPPING_CONFIGURATION_FILE_PATH,
		TEST_FILE_DIR "test.map"));

	SetUpSimulationBusMock("source_signal_in");

	ASSERT_FEP3_NOERROR(_registry->registerDataIn(
		"target_signal_in",
		fep3::base::StreamTypeDDLFileRef{ "tTestStruct", TEST_FILE_DIR "test.description" }));

	ASSERT_FEP3_NOERROR(_component_registry->initialize());
	ASSERT_FEP3_NOERROR(_component_registry->tense());

	ASSERT_FEP3_NOERROR(_component_registry->relax());
	ASSERT_FEP3_NOERROR(_component_registry->deinitialize());

	ASSERT_FEP3_NOERROR(_registry->unregisterDataIn("target_signal_in"));
}

TEST_F(MappingTester, testInvalidSignalMapping)
{
    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(*_configuration_service, FEP3_DATA_REGISTRY_MAPPING_CONFIGURATION_FILE_PATH, TEST_FILE_DIR "test_invalid.map"));

    ASSERT_FEP3_RESULT_WITH_MESSAGE(_registry->registerDataIn("target_signal_in", fep3::base::StreamTypeDDLFileRef{ "tTestStruct", TEST_FILE_DIR "test.description" }), fep3::ERR_INVALID_TYPE,
        ".*ui8SpellingError.*assignment.*ui8Second.*target.*target_signal_in.*");
}

TEST_F(MappingTester, testMappingWithRenaming)
{
    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(*_configuration_service, FEP3_DATA_REGISTRY_MAPPING_CONFIGURATION_FILE_PATH, TEST_FILE_DIR "test.map"));
    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(*_configuration_service, FEP3_DATA_REGISTRY_SIGNAL_RENAMING_INPUT_CONFIGURATION, "source_signal_in:alias_signal_in"));

    SetUpSimulationBusMock("alias_signal_in");

    ASSERT_FEP3_NOERROR(_registry->registerDataIn("target_signal_in", fep3::base::StreamTypeDDLFileRef{ "tTestStruct", TEST_FILE_DIR "test.description" }));
    _target_reader = _registry->getReader("target_signal_in");
    ASSERT_TRUE(_target_reader);

    ASSERT_FEP3_NOERROR(_component_registry->initialize());
    ASSERT_FEP3_NOERROR(_component_registry->tense());
    ASSERT_FEP3_NOERROR(_component_registry->start());

    ASSERT_EQ(_registry->getSignalInNames().size(), 1);
    ASSERT_STREQ(_registry->getSignalInNames()[0].c_str(), "alias_signal_in");

    SendAndReceiveSignal();

    ASSERT_FEP3_NOERROR(_component_registry->stop());
    ASSERT_FEP3_NOERROR(_component_registry->relax());
    ASSERT_FEP3_NOERROR(_component_registry->deinitialize());
}

TEST_F(MappingTester, testMappingWithInvalidRenaming)
{
    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(*_configuration_service, FEP3_DATA_REGISTRY_SIGNAL_RENAMING_INPUT_CONFIGURATION, "source_signal_in:alias_signal_in"));

    // No need to set up simulation bus because tense will not be reached

    // The source signal of the mapping target cannot be renamed if the alias already exists
    ASSERT_FEP3_NOERROR(_registry->registerDataIn("alias_signal_in", fep3::base::StreamTypePlain<int8_t>()));
    ASSERT_TRUE(_registry->getReader("alias_signal_in"));

    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(*_configuration_service, FEP3_DATA_REGISTRY_MAPPING_CONFIGURATION_FILE_PATH, TEST_FILE_DIR "test.map"));
    ASSERT_FEP3_NOERROR(_registry->registerDataIn("target_signal_in", fep3::base::StreamTypeDDLFileRef{ "tTestStruct", TEST_FILE_DIR "test.description" }));
    _target_reader = _registry->getReader("target_signal_in");
    ASSERT_TRUE(_target_reader);

    ASSERT_FEP3_RESULT_WITH_MESSAGE(_component_registry->initialize(), fep3::ERR_NOT_SUPPORTED,
        "The input signal name 'alias_signal_in' alias 'alias_signal_in' is already registered as signal with same alias name.");
}