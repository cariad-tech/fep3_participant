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

#include "data_registry_test_fixture.h"

#include <fep3/base/sample/data_sample.h>
#include <fep3/base/stream_type/default_stream_type.h>

#include <boost/filesystem.hpp>

using namespace testing;

// DDL Struct defined in the test description
struct TestStruct {
    uint8_t first;
    uint8_t second;
};

struct TestStructA {
    uint16_t first;
    int8_t second;
};

// alignas(8) needed for 32 bit, otherwise sizeof(TestStructB) is different
struct alignas(8) TestStructB {
    uint64_t first;
    uint32_t second;
};

struct TestStructC {
    uint64_t first;
    int64_t second;
    uint32_t third;
    uint16_t fourth;
    int8_t fifth;
};

static constexpr uint8_t first_test_value = 10;
static constexpr uint8_t second_test_value = 5;

template <typename T>
class MyDataReceiver : public fep3::IDataRegistry::IDataReceiver {
    void operator()(const fep3::data_read_ptr<const fep3::IStreamType>&) override{};
    void operator()(const fep3::data_read_ptr<const fep3::IDataSample>& sample) override
    {
        fep3::base::RawMemoryStandardType<T> memory{test_struct};
        ASSERT_EQ(sample->read(memory), memory.size());
    };

public:
    void verify(T expected_result)
    {
        ASSERT_EQ(expected_result.first, test_struct.first);
        ASSERT_EQ(expected_result.second, test_struct.second);
    }

private:
    T test_struct{};
};

struct MappingTester : public NativeDataRegistry {
    MappingTester() : NativeDataRegistry()
    {
    }

    void captureDataReceiver(
        const std::shared_ptr<fep3::ISimulationBus::IDataReceiver>& data_receiver,
        const std::string& signal_name)
    {
        _data_receivers.insert({signal_name, data_receiver});
    }

    void SetUpSimulationBusMock(std::vector<std::string> signal_names)
    {
        _source_readers.clear();
        _data_receivers.clear();

        for (auto signal_name: signal_names) {
            // Give the DataRegistry an actual DataReader object to work with
            auto source_reader = std::make_unique<fep3::mock::SimulationBus::DataReader>();
            _source_readers.insert({signal_name, source_reader.get()});
            EXPECT_CALL(*_simulation_bus, getReader(signal_name, _, _))
                .WillOnce(Return(ByMove(std::move(source_reader))));

            // Get a pointer to the data receiver that gets passed to the mocked simulation bus so
            // we can send samples to the data receiver without using the simulation bus
            EXPECT_CALL(*_source_readers.find(signal_name)->second, reset_(_))
                .WillOnce(WithArgs<0>(
                    Invoke([signal_name,
                            this](const std::shared_ptr<fep3::arya::ISimulationBus::IDataReceiver>
                                      data_receiver) {
                        this->captureDataReceiver(data_receiver, signal_name);
                    })));
        }
        EXPECT_CALL(*_simulation_bus, startBlockingReception(_)).WillOnce(InvokeArgument<0>());
    }

    template <typename SendType, typename ExpectedType>
    void sendAndReceiveSignal(SendType send_value,
                              ExpectedType expected_value,
                              const std::string data_receiver_name,
                              const std::string target_reader_name)
    {
        fep3::base::DataSampleType<SendType> input{send_value};
        auto data_sample = std::make_shared<fep3::base::DataSample>(input);
        (*_data_receivers.find(data_receiver_name)->second)(data_sample);

        MyDataReceiver<ExpectedType> receiver{};
        _target_readers.find(target_reader_name)->second->pop(receiver);
        receiver.verify(expected_value);
    }

    void sendAndReceiveSignal(TestStructA send_value_a,
                              const std::string& receiver_name_a,
                              TestStructB send_value_b,
                              const std::string& receiver_name_b,
                              TestStructC expected_value,
                              const std::string& target_reader_name)
    {
        fep3::base::DataSampleType<TestStructB> input_b{send_value_b};
        auto data_sample_b = std::make_shared<fep3::base::DataSample>(input_b);
        (*_data_receivers.find(receiver_name_b)->second)(data_sample_b);

        fep3::base::DataSampleType<TestStructA> input_a{send_value_a};
        auto data_sample_a = std::make_shared<fep3::base::DataSample>(input_a);
        (*_data_receivers.find(receiver_name_a)->second)(data_sample_a);

        MyDataReceiver<TestStructC> receiver{};
        _target_readers.find(target_reader_name)->second->pop(receiver);
        receiver.verify(expected_value);
    }

    void sendAndReceiveSignal(TestStructC send_value_c,
                              const std::string& receiver_name,
                              TestStructA expected_value_a,
                              const std::string& reader_name_a,
                              TestStructB expected_value_b,
                              const std::string& reader_name_b)
    {
        fep3::base::DataSampleType<TestStructC> input{send_value_c};
        auto data_sample = std::make_shared<fep3::base::DataSample>(input);
        (*_data_receivers.find(receiver_name)->second)(data_sample);

        MyDataReceiver<TestStructA> receiver_a{};
        _target_readers.find(reader_name_a)->second->pop(receiver_a);
        receiver_a.verify(expected_value_a);

        MyDataReceiver<TestStructB> receiver_b{};
        _target_readers.find(reader_name_b)->second->pop(receiver_b);
        receiver_b.verify(expected_value_b);
    }

    std::map<std::string, std::shared_ptr<fep3::ISimulationBus::IDataReceiver>> _data_receivers;
    std::map<std::string, fep3::mock::SimulationBus::DataReader*> _source_readers;
    std::map<std::string, std::unique_ptr<fep3::IDataRegistry::IDataReader>> _target_readers;
};

TEST_F(MappingTester, testSignalMappingIn)
{
    constexpr auto source_signal_name = "source_signal_in", target_signal_name = "target_signal_in";
    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(
        *_configuration_service,
        FEP3_DATA_REGISTRY_MAPPING_CONFIGURATION_FILE_PATH,
        TEST_FILE_DIR "test.map"));

    SetUpSimulationBusMock({source_signal_name});

    ASSERT_FEP3_NOERROR(_registry->registerDataIn(
        target_signal_name,
        fep3::base::StreamTypeDDLFileRef{"tTestStruct", TEST_FILE_DIR "test.description"}));
    _target_readers.insert(
        {target_signal_name, std::move(_registry->getReader(target_signal_name))});
    ASSERT_TRUE(_target_readers.find(target_signal_name) != _target_readers.end());

    ASSERT_FEP3_NOERROR(_component_registry->initialize());
    ASSERT_FEP3_NOERROR(_component_registry->tense());
    ASSERT_FEP3_NOERROR(_component_registry->start());

    ASSERT_EQ(_registry->getSignalInNames().size(), 1);
    ASSERT_STREQ(_registry->getSignalInNames()[0].c_str(), source_signal_name);

    TestStruct send_value{first_test_value, second_test_value},
        expected_value{first_test_value * 2, second_test_value};
    sendAndReceiveSignal(send_value, expected_value, source_signal_name, target_signal_name);

    ASSERT_FEP3_NOERROR(_component_registry->stop());
    ASSERT_FEP3_NOERROR(_component_registry->relax());
    ASSERT_FEP3_NOERROR(_component_registry->deinitialize());
}

// Test whether a Data Registry and the Signal Mapping functionality
// correctly unregister mapped target and source signals and therefore
// can be reinitialized successfully multiple times.
TEST_F(MappingTester, testSignalMappingInRestart)
{
    constexpr auto source_signal_name = "source_signal_in", target_signal_name = "target_signal_in";
    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(
        *_configuration_service,
        FEP3_DATA_REGISTRY_MAPPING_CONFIGURATION_FILE_PATH,
        TEST_FILE_DIR "test.map"));

    SetUpSimulationBusMock({source_signal_name});

    ASSERT_FEP3_NOERROR(_registry->registerDataIn(
        target_signal_name,
        fep3::base::StreamTypeDDLFileRef{"tTestStruct", TEST_FILE_DIR "test.description"}));
    _target_readers.insert(
        {target_signal_name, std::move(_registry->getReader(target_signal_name))});
    ASSERT_TRUE(_target_readers.find(target_signal_name) != _target_readers.end());

    ASSERT_FEP3_NOERROR(_component_registry->initialize());
    ASSERT_FEP3_NOERROR(_component_registry->tense());

    ASSERT_EQ(1, _registry->getSignalInNames().size());
    ASSERT_STREQ(_registry->getSignalInNames()[0].c_str(), source_signal_name);

    ASSERT_FEP3_NOERROR(_component_registry->start());

    TestStruct send_value{first_test_value, second_test_value},
        expected_value{first_test_value * 2, second_test_value};
    sendAndReceiveSignal(send_value, expected_value, source_signal_name, target_signal_name);

    ASSERT_FEP3_NOERROR(_component_registry->stop());
    ASSERT_FEP3_NOERROR(_component_registry->relax());
    ASSERT_FEP3_NOERROR(_component_registry->deinitialize());

    ASSERT_FEP3_NOERROR(_registry->unregisterDataIn(target_signal_name));
    ASSERT_EQ(0, _registry->getSignalInNames().size());

    _target_readers.clear();

    // Check whether a second initialization and start succeeds

    SetUpSimulationBusMock({source_signal_name});

    ASSERT_FEP3_NOERROR(_registry->registerDataIn(
        target_signal_name,
        fep3::base::StreamTypeDDLFileRef{"tTestStruct", TEST_FILE_DIR "test.description"}));
    _target_readers.insert(
        {target_signal_name, std::move(_registry->getReader(target_signal_name))});
    ASSERT_TRUE(_target_readers.find(target_signal_name) != _target_readers.end());

    ASSERT_FEP3_NOERROR(_component_registry->initialize());
    ASSERT_FEP3_NOERROR(_component_registry->tense());

    ASSERT_EQ(1, _registry->getSignalInNames().size());
    ASSERT_STREQ(_registry->getSignalInNames()[0].c_str(), source_signal_name);

    ASSERT_FEP3_NOERROR(_component_registry->start());

    send_value = {first_test_value, second_test_value};
    expected_value = {first_test_value * 2, second_test_value};
    sendAndReceiveSignal(send_value, expected_value, source_signal_name, target_signal_name);

    ASSERT_FEP3_NOERROR(_component_registry->stop());
    ASSERT_FEP3_NOERROR(_component_registry->relax());
    ASSERT_FEP3_NOERROR(_component_registry->deinitialize());

    ASSERT_FEP3_NOERROR(_registry->unregisterDataIn(target_signal_name));
    ASSERT_EQ(0, _registry->getSignalInNames().size());
}

TEST_F(MappingTester, testSignalMappingRegisterSource)
{
    constexpr auto source_signal_a = "source_signal_a", source_signal_b = "source_signal_b",
                   target_signal_name = "target_signal_c";

    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(
        *_configuration_service,
        FEP3_DATA_REGISTRY_MAPPING_CONFIGURATION_FILE_PATH,
        TEST_FILE_DIR "test_a_b_to_c.map"));

    SetUpSimulationBusMock({source_signal_a, source_signal_b});

    ASSERT_FEP3_NOERROR(
        _registry->registerDataIn(source_signal_a,
                                  fep3::base::StreamTypeDDLFileRef{
                                      "tTestStructA", TEST_FILE_DIR "test_signal_a.description"}));
    _target_readers.insert({source_signal_a, std::move(_registry->getReader(source_signal_a))});
    ASSERT_TRUE(_target_readers.find(source_signal_a) != _target_readers.end());

    ASSERT_FEP3_NOERROR(
        _registry->registerDataIn(source_signal_b,
                                  fep3::base::StreamTypeDDLFileRef{
                                      "tTestStructB", TEST_FILE_DIR "test_signal_b.description"}));
    _target_readers.insert({source_signal_b, std::move(_registry->getReader(source_signal_b))});
    ASSERT_TRUE(_target_readers.find(source_signal_b) != _target_readers.end());

    ASSERT_FEP3_NOERROR(
        _registry->registerDataIn(target_signal_name,
                                  fep3::base::StreamTypeDDLFileRef{
                                      "tTestStructC", TEST_FILE_DIR "test_signal_c.description"}));
    _target_readers.insert(
        {target_signal_name, std::move(_registry->getReader(target_signal_name))});
    ASSERT_TRUE(_target_readers.find(target_signal_name) != _target_readers.end());

    ASSERT_FEP3_NOERROR(_component_registry->initialize());
    ASSERT_FEP3_NOERROR(_component_registry->tense());
    ASSERT_FEP3_NOERROR(_component_registry->start());

    ASSERT_FEP3_NOERROR(_component_registry->stop());
    ASSERT_FEP3_NOERROR(_component_registry->relax());
    ASSERT_FEP3_NOERROR(_component_registry->deinitialize());
}

// Test whether a natively registered signal C of known type c can be mapped using two signals A and
// B of unknown type a and b which have not been natively registered. The unknown types a and b are
// provided via ddl description files which are set as property. Source signals are renamed to map
// onto required signals from mapping configuration. signal registration sequence:
// * register mapped 'target_signal_c'
// ** register source signals 'source_signal_a', 'source_signal_b' at data registry
// * apply signal renaming and register alias signals 'alias_signal_a', 'alias_signal_b' at
// simulation bus
TEST_F(MappingTester, testSignalMappingUnknownDDLFadeIn)
{
    constexpr auto source_signal_a_alias = "alias_signal_a",
                   source_signal_b_alias = "alias_signal_b", target_signal_name = "target_signal_c";
    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(
        *_configuration_service,
        FEP3_DATA_REGISTRY_MAPPING_CONFIGURATION_FILE_PATH,
        TEST_FILE_DIR "test_a_b_to_c.map"));
    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(
        *_configuration_service,
        FEP3_DATA_REGISTRY_SIGNAL_RENAMING_INPUT_CONFIGURATION,
        "source_signal_a:alias_signal_a,source_signal_b:alias_signal_b"));

    ASSERT_FEP3_NOERROR(
        _registry->registerDataIn(target_signal_name,
                                  fep3::base::StreamTypeDDLFileRef{
                                      "tTestStructC", TEST_FILE_DIR "test_signal_c.description"}));

    // At this point required DDL structs are missing and therefore registration of signal
    // 'target_signal_c' fails
    ASSERT_FEP3_RESULT(_component_registry->initialize(), fep3::ERR_INVALID_TYPE);
    ASSERT_FEP3_NOERROR(_component_registry->deinitialize());

    // Register missing DDL structs to make data reader registration succeed
    const auto relative_path_to_ddl = boost::filesystem::path(TEST_FILE_DIR)
                                          .lexically_relative(boost::filesystem::current_path());
    std::vector<std::string> ddl_files;
    ddl_files.emplace_back(TEST_FILE_DIR "test_signal_a.description");
#ifdef WIN32
    ddl_files.emplace_back("..\\" + relative_path_to_ddl.string() + "/test_signal_b.description");
#else // WIN32
    ddl_files.emplace_back(relative_path_to_ddl.string() + "/test_signal_b.description");
#endif
    fep3::base::setPropertyValue<std::vector<std::string>>(
        *_configuration_service, FEP3_DATA_REGISTRY_MAPPING_DDL_FILE_PATHS, ddl_files);

    SetUpSimulationBusMock({source_signal_a_alias, source_signal_b_alias});

    ASSERT_FEP3_NOERROR(
        _registry->registerDataIn(target_signal_name,
                                  fep3::base::StreamTypeDDLFileRef{
                                      "tTestStructC", TEST_FILE_DIR "test_signal_c.description"}));
    _target_readers.insert(
        {target_signal_name, std::move(_registry->getReader(target_signal_name))});
    ASSERT_TRUE(_target_readers.find(target_signal_name) != _target_readers.end());

    ASSERT_FEP3_NOERROR(_component_registry->initialize());
    ASSERT_FEP3_NOERROR(_component_registry->tense());
    ASSERT_FEP3_NOERROR(_component_registry->start());

    const auto signal_names = _registry->getSignalInNames();

    ASSERT_EQ(signal_names.size(), 2);
    ASSERT_TRUE(std::find(signal_names.begin(), signal_names.end(), source_signal_a_alias) !=
                signal_names.end());
    ASSERT_TRUE(std::find(signal_names.begin(), signal_names.end(), source_signal_b_alias) !=
                signal_names.end());

    TestStructA send_value_a{1, 2};
    TestStructB send_value_b{3, 4};
    TestStructC expected_value{0, 0, 4, 1, 2};
    sendAndReceiveSignal(send_value_a,
                         source_signal_a_alias,
                         send_value_b,
                         source_signal_a_alias,
                         expected_value,
                         target_signal_name);

    ASSERT_FEP3_NOERROR(_component_registry->stop());
    ASSERT_FEP3_NOERROR(_component_registry->relax());
    ASSERT_FEP3_NOERROR(_component_registry->deinitialize());
}

// Test whether natively registered signals A and B of known types a and b can be mapped using
// signal C of unknown type c which has not been natively registered. The unknown type c is provided
// via a ddl description file which is set as property. Signal registration sequence:
// * register mapped 'target_signal_a', 'target_signal_a'
// ** register source signal 'source_signal_c' at data registry
// * register signal 'source_signal_c' at simulation bus
TEST_F(MappingTester, testSignalMappingUnknownDDLFadeOut)
{
    constexpr auto source_signal_name = "source_signal_c", target_signal_a_name = "target_signal_a",
                   target_signal_b_name = "target_signal_b";

    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(
        *_configuration_service,
        FEP3_DATA_REGISTRY_MAPPING_CONFIGURATION_FILE_PATH,
        TEST_FILE_DIR "test_c_to_a_b.map"));

    std::vector<std::string> ddl_files;
    ddl_files.emplace_back(TEST_FILE_DIR "test_signal_a.description");
    ddl_files.emplace_back(TEST_FILE_DIR "test_signal_b.description");
    ddl_files.emplace_back(TEST_FILE_DIR "test_signal_c.description");
    fep3::base::setPropertyValue<std::vector<std::string>>(
        *_configuration_service, FEP3_DATA_REGISTRY_MAPPING_DDL_FILE_PATHS, ddl_files);

    SetUpSimulationBusMock({source_signal_name});

    ASSERT_FEP3_NOERROR(
        _registry->registerDataIn(target_signal_a_name,
                                  fep3::base::StreamTypeDDLFileRef{
                                      "tTestStructA", TEST_FILE_DIR "test_signal_a.description"}));
    ASSERT_FEP3_NOERROR(
        _registry->registerDataIn(target_signal_b_name,
                                  fep3::base::StreamTypeDDLFileRef{
                                      "tTestStructB", TEST_FILE_DIR "test_signal_b.description"}));

    _target_readers.insert(
        {target_signal_a_name, std::move(_registry->getReader(target_signal_a_name))});
    ASSERT_TRUE(_target_readers.find(target_signal_a_name) != _target_readers.end());
    _target_readers.insert(
        {target_signal_b_name, std::move(_registry->getReader(target_signal_b_name))});
    ASSERT_TRUE(_target_readers.find(target_signal_b_name) != _target_readers.end());

    ASSERT_FEP3_NOERROR(_component_registry->initialize());
    ASSERT_FEP3_NOERROR(_component_registry->tense());
    ASSERT_FEP3_NOERROR(_component_registry->start());

    ASSERT_EQ(_registry->getSignalInNames().size(), 1);
    ASSERT_STREQ(_registry->getSignalInNames()[0].c_str(), source_signal_name);

    TestStructC send_value{5, 4, 3, 2, 1};
    TestStructA expected_value_a{2, 1};
    TestStructB expected_value_b{5, 3};
    sendAndReceiveSignal(send_value,
                         source_signal_name,
                         expected_value_a,
                         target_signal_a_name,
                         expected_value_b,
                         target_signal_b_name);

    ASSERT_FEP3_NOERROR(_component_registry->stop());
    ASSERT_FEP3_NOERROR(_component_registry->relax());
    ASSERT_FEP3_NOERROR(_component_registry->deinitialize());
}

// Check whether source signals of different update rates result in target signal reception
// according to the mapping configuration.
TEST_F(MappingTester, testSignalMappingNonMatchingUpdateRates)
{
    constexpr auto source_signal_a_name = "source_signal_a",
                   source_signal_b_name = "source_signal_b", target_signal_name = "target_signal_c";

    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(
        *_configuration_service,
        FEP3_DATA_REGISTRY_MAPPING_CONFIGURATION_FILE_PATH,
        TEST_FILE_DIR "test_a_b_to_c.map"));

    // Register missing DDL structs to make data reader registration succeed
    const auto relative_path_to_ddl = boost::filesystem::path(TEST_FILE_DIR)
                                          .lexically_relative(boost::filesystem::current_path());
    std::vector<std::string> ddl_files;
    ddl_files.emplace_back(TEST_FILE_DIR "test_signal_a.description");
#ifdef WIN32
    ddl_files.emplace_back("..\\" + relative_path_to_ddl.string() + "/test_signal_b.description");
#else // WIN32
    ddl_files.emplace_back(relative_path_to_ddl.string() + "/test_signal_b.description");
#endif
    fep3::base::setPropertyValue<std::vector<std::string>>(
        *_configuration_service, FEP3_DATA_REGISTRY_MAPPING_DDL_FILE_PATHS, ddl_files);

    SetUpSimulationBusMock({source_signal_a_name, source_signal_b_name});

    ASSERT_FEP3_NOERROR(
        _registry->registerDataIn(target_signal_name,
                                  fep3::base::StreamTypeDDLFileRef{
                                      "tTestStructC", TEST_FILE_DIR "test_signal_c.description"}));
    _target_readers.insert(
        {target_signal_name, std::move(_registry->getReader(target_signal_name))});
    ASSERT_TRUE(_target_readers.find(target_signal_name) != _target_readers.end());

    ASSERT_FEP3_NOERROR(_component_registry->initialize());
    ASSERT_FEP3_NOERROR(_component_registry->tense());
    ASSERT_FEP3_NOERROR(_component_registry->start());

    const auto signal_names = _registry->getSignalInNames();

    ASSERT_EQ(signal_names.size(), 2);
    ASSERT_TRUE(std::find(signal_names.begin(), signal_names.end(), source_signal_a_name) !=
                signal_names.end());
    ASSERT_TRUE(std::find(signal_names.begin(), signal_names.end(), source_signal_b_name) !=
                signal_names.end());

    // as configured within the provided mapping configuration 'test_a_b_to_c.map', signal a is the
    // trigger to forward mapped target signals
    {
        // we only transmit signal b and expect not to receive a target signal c
        TestStructB send_value_b{3, 4};
        TestStructC expected_value{0, 0, 0, 0, 0};
        sendAndReceiveSignal(
            send_value_b, expected_value, source_signal_b_name, target_signal_name);

        // we only transmit signal a and expect to receive a target signal c
        // values of signal b will be considered
        TestStructA send_value_a{1, 2};
        expected_value = {0, 0, 4, 1, 2};
        sendAndReceiveSignal(
            send_value_a, expected_value, source_signal_a_name, target_signal_name);

        // we only transmit signal a and expect to receive a target signal c
        // values of signal b will be considered
        send_value_a = {5, 6};
        expected_value = {0, 0, 4, 5, 6};
        sendAndReceiveSignal(
            send_value_a, expected_value, source_signal_a_name, target_signal_name);

        // we only transmit signal b and expect not to receive a target signal c
        send_value_b = {7, 8};
        expected_value = {0, 0, 0, 0, 0};
        sendAndReceiveSignal(
            send_value_b, expected_value, source_signal_b_name, target_signal_name);

        // we transmit both signals and expect to receive a taget signal c
        // both source signals shall be considered
        send_value_a = {9, 10};
        send_value_b = {11, 12};
        expected_value = {0, 0, 12, 9, 10};
        sendAndReceiveSignal(send_value_a,
                             source_signal_a_name,
                             send_value_b,
                             source_signal_b_name,
                             expected_value,
                             target_signal_name);
    }

    ASSERT_FEP3_NOERROR(_component_registry->stop());
    ASSERT_FEP3_NOERROR(_component_registry->relax());
    ASSERT_FEP3_NOERROR(_component_registry->deinitialize());
}

// Test whether mapping configuration requiring 10 unknown source signal DDL descriptions can be
// used if all required DDL description files are provided via property.
TEST_F(MappingTester, testSignalMapping10DDLFiles)
{
    constexpr auto source_signal_name = "source_signal_d", target_signal_name = "target_signal_k";

    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(
        *_configuration_service,
        FEP3_DATA_REGISTRY_MAPPING_CONFIGURATION_FILE_PATH,
        TEST_FILE_DIR "test_10_sources_to_target.map"));

    // Register missing DDL structs to make data reader registration succeed
    std::vector<std::string> ddl_files;
    ddl_files.emplace_back(TEST_FILE_DIR "test_signal_a.description");
    ddl_files.emplace_back(TEST_FILE_DIR "test_signal_b.description");
    ddl_files.emplace_back(TEST_FILE_DIR "test_signal_c.description");
    ddl_files.emplace_back(TEST_FILE_DIR "test_signal_d.description");
    ddl_files.emplace_back(TEST_FILE_DIR "test_signal_e.description");
    ddl_files.emplace_back(TEST_FILE_DIR "test_signal_f.description");
    ddl_files.emplace_back(TEST_FILE_DIR "test_signal_g.description");
    ddl_files.emplace_back(TEST_FILE_DIR "test_signal_h.description");
    ddl_files.emplace_back(TEST_FILE_DIR "test_signal_i.description");
    ddl_files.emplace_back(TEST_FILE_DIR "test_signal_j.description");
    fep3::base::setPropertyValue<std::vector<std::string>>(
        *_configuration_service, FEP3_DATA_REGISTRY_MAPPING_DDL_FILE_PATHS, ddl_files);

    SetUpSimulationBusMock({
        source_signal_name,
    });

    ASSERT_FEP3_NOERROR(
        _registry->registerDataIn(target_signal_name,
                                  fep3::base::StreamTypeDDLFileRef{
                                      "tTestStructK", TEST_FILE_DIR "test_signal_k.description"}));
    _target_readers.insert(
        {target_signal_name, std::move(_registry->getReader(target_signal_name))});
    ASSERT_TRUE(_target_readers.find(target_signal_name) != _target_readers.end());

    ASSERT_FEP3_NOERROR(_component_registry->initialize());
    ASSERT_FEP3_NOERROR(_component_registry->tense());
    ASSERT_FEP3_NOERROR(_component_registry->start());

    const auto signal_names = _registry->getSignalInNames();

    ASSERT_EQ(signal_names.size(), 1);
    ASSERT_TRUE(std::find(signal_names.begin(), signal_names.end(), source_signal_name) !=
                signal_names.end());

    TestStruct send_value_d{1, 2};
    TestStruct expected_value{1, 2};
    sendAndReceiveSignal(send_value_d, expected_value, source_signal_name, target_signal_name);

    ASSERT_FEP3_NOERROR(_component_registry->stop());
    ASSERT_FEP3_NOERROR(_component_registry->relax());
    ASSERT_FEP3_NOERROR(_component_registry->deinitialize());
}

/**
 * @brief Check whether a mapped signal may be unregistered from the native DataRegistry.
 * A mapped signal is stored separated from the not mapped signals but shall be unregistered
 * using the same functionality.
 */
TEST_F(MappingTester, testUnregisterMappedSignal)
{
    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(
        *_configuration_service,
        FEP3_DATA_REGISTRY_MAPPING_CONFIGURATION_FILE_PATH,
        TEST_FILE_DIR "test.map"));

    SetUpSimulationBusMock({"source_signal_in"});

    ASSERT_FEP3_NOERROR(_registry->registerDataIn(
        "target_signal_in",
        fep3::base::StreamTypeDDLFileRef{"tTestStruct", TEST_FILE_DIR "test.description"}));

    ASSERT_FEP3_NOERROR(_component_registry->initialize());
    ASSERT_FEP3_NOERROR(_component_registry->tense());

    ASSERT_FEP3_NOERROR(_component_registry->relax());
    ASSERT_FEP3_NOERROR(_component_registry->deinitialize());

    ASSERT_FEP3_NOERROR(_registry->unregisterDataIn("target_signal_in"));
}

TEST_F(MappingTester, testMisconfiguredSignalMapping)
{
    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(
        *_configuration_service,
        FEP3_DATA_REGISTRY_MAPPING_CONFIGURATION_FILE_PATH,
        TEST_FILE_DIR "test_misconfigured.map"));
    ASSERT_FEP3_NOERROR(_registry->registerDataIn(
        "target_signal_in",
        fep3::base::StreamTypeDDLFileRef{"tTestStruct", TEST_FILE_DIR "test.description"}));
    ASSERT_FEP3_RESULT_WITH_MESSAGE(
        _component_registry->initialize(),
        fep3::ERR_INVALID_TYPE,
        ".*ui8SpellingError.*assignment.*ui8Second.*target.*target_signal_in.*");
    ASSERT_FEP3_NOERROR(_component_registry->deinitialize());
}

TEST_F(MappingTester, testMappingWithRenaming)
{
    constexpr auto source_signal_alias = "alias_signal_in", target_signal_name = "target_signal_in";
    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(
        *_configuration_service,
        FEP3_DATA_REGISTRY_MAPPING_CONFIGURATION_FILE_PATH,
        TEST_FILE_DIR "test.map"));
    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(
        *_configuration_service,
        FEP3_DATA_REGISTRY_SIGNAL_RENAMING_INPUT_CONFIGURATION,
        "source_signal_in:alias_signal_in"));

    SetUpSimulationBusMock({source_signal_alias});

    ASSERT_FEP3_NOERROR(_registry->registerDataIn(
        target_signal_name,
        fep3::base::StreamTypeDDLFileRef{"tTestStruct", TEST_FILE_DIR "test.description"}));
    _target_readers.insert(
        {target_signal_name, std::move(_registry->getReader(target_signal_name))});
    ASSERT_TRUE(_target_readers.find(target_signal_name) != _target_readers.end());

    ASSERT_FEP3_NOERROR(_component_registry->initialize());
    ASSERT_FEP3_NOERROR(_component_registry->tense());
    ASSERT_FEP3_NOERROR(_component_registry->start());

    ASSERT_EQ(_registry->getSignalInNames().size(), 1);
    ASSERT_STREQ(_registry->getSignalInNames()[0].c_str(), source_signal_alias);

    TestStruct send_value{first_test_value, second_test_value},
        expected_value{first_test_value * 2, second_test_value};
    sendAndReceiveSignal(send_value, expected_value, source_signal_alias, target_signal_name);

    ASSERT_FEP3_NOERROR(_component_registry->stop());
    ASSERT_FEP3_NOERROR(_component_registry->relax());
    ASSERT_FEP3_NOERROR(_component_registry->deinitialize());
}

TEST_F(MappingTester, testMappingWithInvalidRenaming)
{
    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(
        *_configuration_service,
        FEP3_DATA_REGISTRY_SIGNAL_RENAMING_INPUT_CONFIGURATION,
        "source_signal_in:alias_signal_in"));

    // No need to set up simulation bus because tense will not be reached

    // The source signal of the mapping target cannot be renamed if the alias already exists
    ASSERT_FEP3_NOERROR(
        _registry->registerDataIn("alias_signal_in", fep3::base::StreamTypePlain<int8_t>()));
    ASSERT_TRUE(_registry->getReader("alias_signal_in"));

    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(
        *_configuration_service,
        FEP3_DATA_REGISTRY_MAPPING_CONFIGURATION_FILE_PATH,
        TEST_FILE_DIR "test.map"));
    ASSERT_FEP3_NOERROR(_registry->registerDataIn(
        "target_signal_in",
        fep3::base::StreamTypeDDLFileRef{"tTestStruct", TEST_FILE_DIR "test.description"}));

    ASSERT_FEP3_RESULT_WITH_MESSAGE(
        _component_registry->initialize(),
        fep3::ERR_NOT_SUPPORTED,
        "The input signal name 'alias_signal_in' alias 'alias_signal_in' is already registered as "
        "signal with same alias name.");
}

// Test whether merging a description containing a redundant datatype results in an error logged and
// returned.
TEST_F(NativeDataRegistryWithMocksBase, testSignalMappingMergeRedundantDataType)
{
    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(
        *_data_registry_property_node->getChild(FEP3_MAPPING_CONFIGURATION_FILE_PATH_PROPERTY),
        TEST_FILE_DIR "test.map"));

    std::vector<std::string> ddl_files;
    ddl_files.emplace_back(TEST_FILE_DIR "test.description");
    ddl_files.emplace_back(TEST_FILE_DIR "test_redundant_datatype.description");
    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::vector<std::string>>(
        *_data_registry_property_node->getChild(FEP3_MAPPING_DDL_FILE_PATHS_PROPERTY), ddl_files));

    ASSERT_FEP3_NOERROR(_data_registry->registerDataIn(
        "target_signal_in",
        fep3::base::StreamTypeDDLFileRef{"tTestStruct", TEST_FILE_DIR "test.description"}));
    ASSERT_FEP3_RESULT_WITH_MESSAGE(_data_registry->initialize(),
                                    fep3::ERR_INVALID_ARG,
                                    "datamodel::DataDefinition::DataTypes::add\\(tFloat64\\): "
                                    "value with the given name already exists");
    ASSERT_FEP3_NOERROR(_data_registry->deinitialize());
}

// Test whether merging a description containing a redundant struct succeeds.
TEST_F(NativeDataRegistryWithMocksBase, testSignalMappingMergeRedundantStruct)
{
    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(
        *_data_registry_property_node->getChild(FEP3_MAPPING_CONFIGURATION_FILE_PATH_PROPERTY),
        TEST_FILE_DIR "test.map"));

    std::vector<std::string> ddl_files;
    ddl_files.emplace_back(TEST_FILE_DIR "test.description");
    ddl_files.emplace_back(TEST_FILE_DIR "test_redundant_struct.description");
    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::vector<std::string>>(
        *_data_registry_property_node->getChild(FEP3_MAPPING_DDL_FILE_PATHS_PROPERTY), ddl_files));

    ASSERT_FEP3_NOERROR(_data_registry->registerDataIn(
        "target_signal_in",
        fep3::base::StreamTypeDDLFileRef{"tTestStruct", TEST_FILE_DIR "test.description"}));
    ASSERT_FEP3_RESULT_WITH_MESSAGE(_data_registry->initialize(),
                                    fep3::ERR_INVALID_ARG,
                                    "datamodel::DataDefinition::StructTypes::add\\(tTestStruct\\): "
                                    "value with the given name already exists");
    ASSERT_FEP3_NOERROR(_data_registry->deinitialize());
}

// Test whether providing an invalid (here empty) ddl description file via absolute path results in
// an error logged and returned.
TEST_F(NativeDataRegistryWithMocksBase, testSignalMappingInvalidDDLFileAbsolutePath)
{
    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(
        *_data_registry_property_node->getChild(FEP3_MAPPING_CONFIGURATION_FILE_PATH_PROPERTY),
        TEST_FILE_DIR "test.map"));

    std::vector<std::string> ddl_files;
    ddl_files.emplace_back(TEST_FILE_DIR "non_existent_test.description");
    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::vector<std::string>>(
        *_data_registry_property_node->getChild(FEP3_MAPPING_DDL_FILE_PATHS_PROPERTY), ddl_files));

    EXPECT_CALL(*_logger_mock, logError(HasSubstr("Failed to read DDL")))
        .WillOnce(Return(fep3::ERR_NOERROR));

    ASSERT_FEP3_NOERROR(_data_registry->registerDataIn(
        "target_signal_in",
        fep3::base::StreamTypeDDLFileRef{"tTestStruct", TEST_FILE_DIR "test.description"}));
    ASSERT_FEP3_RESULT_WITH_MESSAGE(
        _data_registry->initialize(),
        fep3::ERR_INVALID_FILE,
        "Failed to read DDL description file .*non_existent_test.description");
    ASSERT_FEP3_NOERROR(_data_registry->deinitialize());
}

// Test whether providing an invalid (here empty) ddl description file via relative path results in
// an error logged and returned.
TEST_F(NativeDataRegistryWithMocksBase, testSignalMappingInvalidDDLFileRelativePath)
{
    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(
        *_data_registry_property_node->getChild(FEP3_MAPPING_CONFIGURATION_FILE_PATH_PROPERTY),
        TEST_FILE_DIR "test.map"));

    std::vector<std::string> ddl_files;
    ddl_files.emplace_back("../non_existent_test.description");
    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::vector<std::string>>(
        *_data_registry_property_node->getChild(FEP3_MAPPING_DDL_FILE_PATHS_PROPERTY), ddl_files));

    ASSERT_FEP3_NOERROR(_data_registry->registerDataIn(
        "target_signal_in",
        fep3::base::StreamTypeDDLFileRef{"tTestStruct", TEST_FILE_DIR "test.description"}));
    ASSERT_FEP3_RESULT_WITH_MESSAGE(
        _data_registry->initialize(),
        fep3::ERR_INVALID_FILE,
        "Failed to read DDL description file .*non_existent_test.description");
    ASSERT_FEP3_NOERROR(_data_registry->deinitialize());

    // Invalid ddl description files shall result in the same behaviour (log and error returned)
    // even if not adapting the properties
    ASSERT_FEP3_NOERROR(_data_registry->registerDataIn(
        "target_signal_in",
        fep3::base::StreamTypeDDLFileRef{"tTestStruct", TEST_FILE_DIR "test.description"}));
    ASSERT_FEP3_RESULT_WITH_MESSAGE(
        _data_registry->initialize(),
        fep3::ERR_INVALID_FILE,
        "Failed to read DDL description file .*non_existent_test.description");
    ASSERT_FEP3_NOERROR(_data_registry->deinitialize());
}

// Test whether providing an invalid (here empty) ddl description file via relative path results in
// an error logged and returned.
TEST_F(NativeDataRegistryWithMocksBase, testSignalMappingUnknownSourceDDLs)
{
    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(
        *_data_registry_property_node->getChild(FEP3_MAPPING_CONFIGURATION_FILE_PATH_PROPERTY),
        TEST_FILE_DIR "test_a_b_to_c.map"));

    EXPECT_CALL(*_logger_mock,
                logError(HasSubstr("Unknown type 'tTestStructA'\nUnknown type 'tTestStructB'")))
        .WillOnce(Return(fep3::ERR_NOERROR));

    ASSERT_FEP3_NOERROR(_data_registry->registerDataIn(
        "target_signal_c",
        fep3::base::StreamTypeDDLFileRef{"tTestStructC",
                                         TEST_FILE_DIR "test_signal_c.description"}));
    ASSERT_FEP3_RESULT(_data_registry->initialize(), fep3::ERR_INVALID_TYPE);
    ASSERT_FEP3_NOERROR(_data_registry->deinitialize());
}

// Test whether providing an invalid mapping file results in an error logged and returned.
TEST_F(NativeDataRegistryWithMocksBase, testSignalMappingInvalidMappingFile)
{
    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(
        *_data_registry_property_node->getChild(FEP3_MAPPING_CONFIGURATION_FILE_PATH_PROPERTY),
        TEST_FILE_DIR "test_invalid.map"));

    EXPECT_CALL(*_logger_mock, logError(HasSubstr("Failed to load mapping configuration file")))
        .WillOnce(Return(fep3::ERR_NOERROR));

    ASSERT_FEP3_NOERROR(_data_registry->registerDataIn(
        "target_signal_c",
        fep3::base::StreamTypeDDLFileRef{"tTestStructC",
                                         TEST_FILE_DIR "test_signal_c.description"}));
    ASSERT_FEP3_RESULT(_data_registry->initialize(), fep3::ERR_INVALID_FILE);
    ASSERT_FEP3_NOERROR(_data_registry->deinitialize());

    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(
        *_data_registry_property_node->getChild(FEP3_MAPPING_CONFIGURATION_FILE_PATH_PROPERTY),
        ""));
}
