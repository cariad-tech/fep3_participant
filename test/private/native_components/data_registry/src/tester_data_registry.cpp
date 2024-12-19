/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "data_registry_test_fixture.h"

#include <fep3/base/sample/data_sample.h>
#include <fep3/base/stream_type/default_stream_type.h>
#include <fep3/base/stream_type/mock/mock_stream_type.h>
#include <fep3/components/service_bus/rpc/fep_rpc_stubs_client.h>
#include <fep3/native_components/simulation_bus/simulation_bus.h>
#include <fep3/rpc_services/data_registry/data_registry_client_stub.h>

#include <helper/gmock_destruction_helper.h>

bool containsVector(const std::vector<std::string>& source_vec,
                    const std::vector<std::string>& contain_vec)
{
    for (const auto& item: contain_vec) {
        bool found = false;
        for (const auto& item_source: source_vec) {
            if (item_source == item) {
                found = true;
                break;
            }
        }
        if (!found) {
            return false;
        }
    }
    return true;
}

TEST(DataRegistry, testReturnDDLMergeErrorInfo)
{
    fep3::native::DataRegistry data_registry;

    const auto struct_name = "tTestStruct";
    const auto ddl_description = R"(<?xml version="1.0" encoding="utf-8" standalone="no"?>
<adtf:ddl xmlns:adtf="adtf">
 <header>
  <language_version>0.00</language_version>
  <author>fep_team</author>
  <date_creation></date_creation>
  <date_change></date_change>
  <description>Simplistic DDL for testing purposes</description>
 </header>
 <units />
 <datatypes/>
 <enums>
 </enums>
 <structs/>
 <streams />
</adtf:ddl>)";

    ASSERT_FEP3_RESULT_WITH_MESSAGE(
        data_registry.registerDataOut("signal_out",
                                      fep3::base::StreamTypeDDL(struct_name, ddl_description)),
        fep3::ERR_INVALID_ARG,
        ".*is not a valid DDL string. See problem list!.*xml - header.*Major version 0 invalid");

    ASSERT_FEP3_RESULT_WITH_MESSAGE(
        data_registry.registerDataIn("signal_in",
                                     fep3::base::StreamTypeDDL(struct_name, ddl_description)),
        fep3::ERR_INVALID_ARG,
        ".*is not a valid DDL string. See problem list!.*xml - header.*Major version 0 invalid");
}

TEST_F(NativeDataRegistryWithMocks, testGetStreamTypeNotFound)
{
    ASSERT_FEP3_NOERROR(_data_registry->initialize());

    EXPECT_CALL(*_component_registry, findComponent(_simulation_bus->getComponentIID()))
        .Times(1)
        .WillOnce(::testing::Return(_simulation_bus.get()));

    ASSERT_FEP3_NOERROR(_data_registry->tense());

    EXPECT_EQ(_data_registry->getStreamType("signal_name").getMetaTypeName(), "hook");

    EXPECT_CALL(*_component_registry, findComponent(_simulation_bus->getComponentIID()))
        .Times(1)
        .WillOnce(::testing::Return(_simulation_bus.get()));

    ASSERT_FEP3_NOERROR(_data_registry->relax());
    ASSERT_FEP3_NOERROR(_data_registry->deinitialize());
}

TEST_F(NativeDataRegistryWithMocks, testGetStreamType)
{
    const std::string signal_in_name = "signal_in";
    const std::string signal_out_name = "signal_out";
    auto mock_data_writer =
        std::make_unique<::testing::NiceMock<::fep3::mock::SimulationBus::DataWriter>>();
    auto mock_data_reader =
        std::make_unique<::testing::NiceMock<::fep3::mock::SimulationBus::DataReader>>();

    const fep3::base::StreamTypeRaw stream_type_raw;

    ASSERT_FEP3_NOERROR(_data_registry->registerDataIn(signal_in_name, stream_type_raw));
    ASSERT_FEP3_NOERROR(_data_registry->registerDataOut(signal_out_name, stream_type_raw));

    ASSERT_FEP3_NOERROR(_data_registry->initialize());

    EXPECT_CALL(*_simulation_bus, getReader(_, _, _))
        .Times(1)
        .WillOnce(Return(::testing::ByMove(std::move(mock_data_reader))));
    EXPECT_CALL(*_simulation_bus,
                getWriter(_,
                          ::testing::Matcher<const ::fep3::IStreamType&>(
                              fep3::mock::StreamTypeMatcher(stream_type_raw))))
        .Times(1)
        .WillOnce(Return(::testing::ByMove(std::move(mock_data_writer))));
    EXPECT_CALL(*_component_registry, findComponent(_simulation_bus->getComponentIID()))
        .Times(1)
        .WillOnce(::testing::Return(_simulation_bus.get()));

    ASSERT_FEP3_NOERROR(_data_registry->tense());

    EXPECT_EQ(_data_registry->getStreamType(signal_in_name).getMetaTypeName(),
              stream_type_raw.getMetaTypeName());
    EXPECT_EQ(_data_registry->getStreamType(signal_out_name).getMetaTypeName(),
              stream_type_raw.getMetaTypeName());

    EXPECT_CALL(*_component_registry, findComponent(_simulation_bus->getComponentIID()))
        .Times(1)
        .WillOnce(::testing::Return(_simulation_bus.get()));

    ASSERT_FEP3_NOERROR(_data_registry->relax());
    ASSERT_FEP3_NOERROR(_data_registry->deinitialize());
}

TEST_F(NativeDataRegistryWithMocks, testGetStreamTypeOfAlias)
{
    const std::string signal_in_name = "signal_in";
    const std::string signal_in_name_renamed = "signal_in_renamed";
    const std::string signal_out_name = "signal_out";
    const std::string signal_out_name_renamed = "signal_out_renamed";
    auto mock_data_writer =
        std::make_unique<::testing::NiceMock<::fep3::mock::SimulationBus::DataWriter>>();
    auto mock_data_reader =
        std::make_unique<::testing::NiceMock<::fep3::mock::SimulationBus::DataReader>>();
    const fep3::base::StreamTypeRaw stream_type_raw;

    ASSERT_FEP3_NOERROR(_data_registry->registerDataIn(signal_in_name, stream_type_raw));
    ASSERT_FEP3_NOERROR(_data_registry->registerDataOut(signal_out_name, stream_type_raw));

    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(
        *_data_registry_property_node->getChild(FEP3_SIGNAL_RENAMING_INPUT_CONFIGURATION_PROPERTY),
        signal_in_name + ":" + signal_in_name_renamed));
    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(
        *_data_registry_property_node->getChild(FEP3_SIGNAL_RENAMING_OUTPUT_CONFIGURATION_PROPERTY),
        signal_out_name + ":" + signal_out_name_renamed));

    ASSERT_FEP3_NOERROR(_data_registry->initialize());

    EXPECT_CALL(*_simulation_bus, getReader(_, _, _))
        .Times(1)
        .WillOnce(Return(::testing::ByMove(std::move(mock_data_reader))));
    EXPECT_CALL(*_simulation_bus,
                getWriter(_,
                          ::testing::Matcher<const ::fep3::IStreamType&>(
                              fep3::mock::StreamTypeMatcher(stream_type_raw))))
        .Times(1)
        .WillOnce(Return(::testing::ByMove(std::move(mock_data_writer))));
    EXPECT_CALL(*_component_registry, findComponent(_simulation_bus->getComponentIID()))
        .Times(1)
        .WillOnce(::testing::Return(_simulation_bus.get()));

    ASSERT_FEP3_NOERROR(_data_registry->tense());

    EXPECT_EQ(_data_registry->getStreamType(signal_in_name_renamed).getMetaTypeName(),
              stream_type_raw.getMetaTypeName());
    EXPECT_EQ(_data_registry->getStreamType(signal_in_name).getMetaTypeName(), "hook");
    EXPECT_EQ(_data_registry->getStreamType(signal_out_name_renamed).getMetaTypeName(),
              stream_type_raw.getMetaTypeName());
    EXPECT_EQ(_data_registry->getStreamType(signal_out_name).getMetaTypeName(), "hook");

    EXPECT_CALL(*_component_registry, findComponent(_simulation_bus->getComponentIID()))
        .Times(1)
        .WillOnce(::testing::Return(_simulation_bus.get()));

    ASSERT_FEP3_NOERROR(_data_registry->relax());
    ASSERT_FEP3_NOERROR(_data_registry->deinitialize());
}

TEST_F(NativeDataRegistryWithMocks, testResetRenamedSignals)
{
    const std::string signal_in_name = "signal_in";
    const std::string signal_in_name_renamed = "signal_in_renamed";
    const std::string signal_out_name = "signal_out";
    const std::string signal_out_name_renamed = "signal_out_renamed";
    auto mock_data_writer =
        std::make_unique<::testing::NiceMock<::fep3::mock::SimulationBus::DataWriter>>();
    auto mock_data_reader =
        std::make_unique<::testing::NiceMock<::fep3::mock::SimulationBus::DataReader>>();
    const fep3::base::StreamTypeRaw stream_type_raw;

    ASSERT_FEP3_NOERROR(_data_registry->registerDataIn(signal_in_name, stream_type_raw));
    ASSERT_FEP3_NOERROR(_data_registry->registerDataOut(signal_out_name, stream_type_raw));

    // Rename signals via properties
    {
        ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(
            *_data_registry_property_node->getChild(
                FEP3_SIGNAL_RENAMING_INPUT_CONFIGURATION_PROPERTY),
            signal_in_name + ":" + signal_in_name_renamed));
        ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(
            *_data_registry_property_node->getChild(
                FEP3_SIGNAL_RENAMING_OUTPUT_CONFIGURATION_PROPERTY),
            signal_out_name + ":" + signal_out_name_renamed));

        ASSERT_FEP3_NOERROR(_data_registry->initialize());

        EXPECT_CALL(*_simulation_bus, getReader(signal_in_name_renamed, _, _))
            .Times(1)
            .WillOnce(Return(::testing::ByMove(std::move(mock_data_reader))));
        EXPECT_CALL(*_simulation_bus,
                    getWriter(signal_out_name_renamed,
                              ::testing::Matcher<const ::fep3::IStreamType&>(
                                  fep3::mock::StreamTypeMatcher(stream_type_raw))))
            .Times(1)
            .WillOnce(Return(::testing::ByMove(std::move(mock_data_writer))));
        EXPECT_CALL(*_component_registry, findComponent(_simulation_bus->getComponentIID()))
            .Times(1)
            .WillOnce(::testing::Return(_simulation_bus.get()));

        ASSERT_FEP3_NOERROR(_data_registry->tense());

        EXPECT_EQ(signal_in_name_renamed, _data_registry->getSignalInNames().back());
        EXPECT_EQ(signal_out_name_renamed, _data_registry->getSignalOutNames().back());

        EXPECT_CALL(*_component_registry, findComponent(_simulation_bus->getComponentIID()))
            .Times(1)
            .WillOnce(::testing::Return(_simulation_bus.get()));

        ASSERT_FEP3_NOERROR(_data_registry->relax());
        ASSERT_FEP3_NOERROR(_data_registry->deinitialize());
    }

    auto mock_data_writer2 =
        std::make_unique<::testing::NiceMock<::fep3::mock::SimulationBus::DataWriter>>();
    auto mock_data_reader2 =
        std::make_unique<::testing::NiceMock<::fep3::mock::SimulationBus::DataReader>>();

    // Reset renaming property configuration
    {
        ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(
            *_data_registry_property_node->getChild(
                FEP3_SIGNAL_RENAMING_INPUT_CONFIGURATION_PROPERTY),
            ""));
        ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(
            *_data_registry_property_node->getChild(
                FEP3_SIGNAL_RENAMING_OUTPUT_CONFIGURATION_PROPERTY),
            ""));

        ASSERT_FEP3_NOERROR(_data_registry->initialize());

        EXPECT_CALL(*_simulation_bus, getReader(signal_in_name, _, _))
            .Times(1)
            .WillOnce(Return(::testing::ByMove(std::move(mock_data_reader2))));
        EXPECT_CALL(*_simulation_bus,
                    getWriter(signal_out_name,
                              ::testing::Matcher<const ::fep3::IStreamType&>(
                                  fep3::mock::StreamTypeMatcher(stream_type_raw))))
            .Times(1)
            .WillOnce(Return(::testing::ByMove(std::move(mock_data_writer2))));
        EXPECT_CALL(*_component_registry, findComponent(_simulation_bus->getComponentIID()))
            .Times(1)
            .WillOnce(::testing::Return(_simulation_bus.get()));
        EXPECT_CALL(*_simulation_bus, startBlockingReception(_)).WillOnce(InvokeArgument<0>());

        ASSERT_FEP3_NOERROR(_data_registry->tense());

        EXPECT_EQ(signal_in_name, _data_registry->getSignalInNames().back());
        EXPECT_EQ(signal_out_name, _data_registry->getSignalOutNames().back());

        EXPECT_CALL(*_component_registry, findComponent(_simulation_bus->getComponentIID()))
            .Times(1)
            .WillOnce(::testing::Return(_simulation_bus.get()));

        ASSERT_FEP3_NOERROR(_data_registry->relax());
        ASSERT_FEP3_NOERROR(_data_registry->deinitialize());
    }
}

class TestClient : public fep3::rpc::RPCServiceClient<fep3::rpc_stubs::RPCDataRegistryClientStub,
                                                      fep3::rpc::IRPCDataRegistryDef> {
private:
    typedef fep3::rpc::RPCServiceClient<fep3::rpc_stubs::RPCDataRegistryClientStub,
                                        fep3::rpc::IRPCDataRegistryDef>
        base_type;

public:
    using base_type::GetStub;

    TestClient(const char* server_object_name, std::shared_ptr<fep3::IRPCRequester> rpc)
        : base_type(server_object_name, rpc)
    {
    }

    std::vector<std::string> getSignalInNames() const
    {
        try {
            return a_util::strings::split(GetStub().getSignalInNames(), ",");
        }
        catch (jsonrpc::JsonRpcException&) {
            ADD_FAILURE();
        }
        return std::vector<std::string>{};
    }

    std::vector<std::string> getSignalOutNames() const
    {
        try {
            return a_util::strings::split(GetStub().getSignalOutNames(), ",");
        }
        catch (jsonrpc::JsonRpcException&) {
            ADD_FAILURE();
        }
        return std::vector<std::string>{};
    }

    fep3::base::StreamType getStreamType(const std::string& signal_name) const
    {
        try {
            const auto json_value = GetStub().getStreamType(signal_name);
            fep3::base::StreamType stream_type(
                fep3::base::StreamMetaType(json_value["meta_type"].asString()));

            const auto properties = json_value["properties"];
            auto current_property = properties.begin();

            while (current_property != properties.end()) {
                stream_type.setProperty((*current_property)["name"].asString(),
                                        (*current_property)["value"].asString(),
                                        (*current_property)["type"].asString());

                current_property++;
            }

            return stream_type;
        }
        catch (jsonrpc::JsonRpcException&) {
            ADD_FAILURE();
            return fep3::base::StreamType{fep3::base::StreamMetaType{"void"}};
        }
    }
};

// Dummy class
struct TestDataReceiver : public fep3::ISimulationBus::IDataReceiver {
    fep3::data_read_ptr<const fep3::IStreamType> _last_type;
    fep3::data_read_ptr<const fep3::IDataSample> _last_sample;

    void operator()(const fep3::data_read_ptr<const fep3::IStreamType>& rec_type) override
    {
        _last_type = rec_type;
    };
    void operator()(const fep3::data_read_ptr<const fep3::IDataSample>& rec_sample) override
    {
        _last_sample = rec_sample;
    };

    void reset()
    {
        _last_type.reset();
        _last_sample.reset();
    }
    bool waitForSampleUpdate(int trycount)
    {
        while (trycount > 0) {
            if (_last_sample) {
                break;
            }
            else {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                trycount--;
            }
        }
        return static_cast<bool>(_last_sample);
    }
};

struct NativeDataCommunication : public ::testing::Test

{
    NativeDataCommunication()
    {
    }

    void SetUp() override
    {
        _sender.SetUp();
        _receiver.SetUp();
    }

    void TearDown() override
    {
        stop_deinit();
        _sender.TearDown();
        _receiver.TearDown();
    }

    void init_run()
    {
        ASSERT_TRUE(_sender._component_registry->initialize());
        ASSERT_TRUE(_receiver._component_registry->initialize());
        ASSERT_TRUE(_sender._component_registry->tense());
        ASSERT_TRUE(_receiver._component_registry->tense());
        ASSERT_TRUE(_sender._component_registry->start());
        ASSERT_TRUE(_receiver._component_registry->start());
        _is_running = true;
    }

    void stop_deinit()
    {
        if (_is_running) {
            EXPECT_TRUE(_receiver._component_registry->stop());
            EXPECT_TRUE(_sender._component_registry->stop());
            EXPECT_TRUE(_receiver._component_registry->relax());
            EXPECT_TRUE(_sender._component_registry->relax());
            EXPECT_TRUE(_receiver._component_registry->deinitialize());
            EXPECT_TRUE(_sender._component_registry->deinitialize());
            _is_running = false;
        }
    }

    ParticipantUnderTest<fep3::native::SimulationBus> _sender{"test_sender"};
    ParticipantUnderTest<fep3::native::SimulationBus> _receiver{"test_receiver"};

    bool _is_running{false};
};

TEST_F(NativeDataRegistry, testRegisterSignals)
{
    TestClient client(fep3::rpc::IRPCDataRegistryDef::getRPCDefaultName(),
                      _service_bus->getRequester(fep3::native::testing::participant_name_default));

    ASSERT_FEP3_NOERROR(_registry->registerDataIn(
        "a_in", fep3::base::StreamType{fep3::base::StreamMetaType{"meta_type_raw"}}));
    // we do not care which meta type is use ... we support everything in data registry (all kind)
    // we do not check any special support for types becuse we can deal with every thing and
    //  we will deal with special types like DDL for mapping and something like that
    ASSERT_FEP3_NOERROR(_registry->registerDataIn(
        "b_in", fep3::base::StreamType{fep3::base::StreamMetaType{"unknown_type"}}));
    // we can not register it a second time with a different type
    ASSERT_EQ(_registry->registerDataIn(
                  "a_in", fep3::base::StreamType{fep3::base::StreamMetaType{"meta_type_ddl"}}),
              fep3::ERR_INVALID_TYPE);

    ASSERT_FEP3_NOERROR(_registry->registerDataOut(
        "a_out", fep3::base::StreamType{fep3::base::StreamMetaType{"meta_type_raw"}}));
    // we also support unknown types
    ASSERT_FEP3_NOERROR(_registry->registerDataOut(
        "b_out", fep3::base::StreamType{fep3::base::StreamMetaType{"unknown_type"}}));
    // we can not register it a second time with a different type
    ASSERT_EQ(_registry->registerDataOut(
                  "a_out", fep3::base::StreamType{fep3::base::StreamMetaType{"meta_type_ddl"}}),
              fep3::ERR_INVALID_TYPE);

    ASSERT_EQ(client.getSignalInNames().size(), 2);
    ASSERT_TRUE(containsVector(client.getSignalInNames(), {"a_in", "b_in"}));
    ASSERT_EQ(client.getSignalOutNames().size(), 2);
    ASSERT_TRUE(containsVector(client.getSignalOutNames(), {"a_out", "b_out"}));

    ASSERT_FEP3_NOERROR(_registry->unregisterDataIn("a_in"));
    ASSERT_FEP3_NOERROR(_registry->unregisterDataIn("b_in"));
    ASSERT_EQ(_registry->unregisterDataIn("signal_in_3"), fep3::ERR_NOT_FOUND);

    ASSERT_FEP3_NOERROR(_registry->unregisterDataOut("a_out"));
    ASSERT_FEP3_NOERROR(_registry->unregisterDataOut("b_out"));
    ASSERT_EQ(_registry->unregisterDataOut("signal_out_3"), fep3::ERR_NOT_FOUND);

    ASSERT_EQ(client.getSignalInNames().size(), 0);
    ASSERT_EQ(client.getSignalOutNames().size(), 0);
}

TEST_F(NativeDataRegistry, testWriter)
{
    ASSERT_FEP3_NOERROR(_registry->registerDataOut(
        "signal_out", fep3::base::StreamType{fep3::base::StreamMetaType{"anonymous"}}));
    auto writer = _registry->getWriter("signal_out");
    ASSERT_TRUE(writer);
    ASSERT_FALSE(_registry->getWriter("unknown_signal"));
}

TEST_F(NativeDataRegistry, testReader)
{
    ASSERT_FEP3_NOERROR(_registry->registerDataOut(
        "signal_in", fep3::base::StreamType{fep3::base::StreamMetaType{"meta_type_raw"}}));

    ASSERT_FEP3_NOERROR(_registry->registerDataIn(
        "signal_in", fep3::base::StreamType{fep3::base::StreamMetaType{"meta_type_raw"}}));
    auto reader1 = _registry->getReader("signal_in");
    ASSERT_TRUE(reader1);
    auto reader2 = _registry->getReader("signal_in");
    ASSERT_TRUE(reader2);
    ASSERT_FALSE(_registry->getReader("unknown_signal"));
}

TEST_F(NativeDataRegistry, testListenerRegistration)
{
    ASSERT_FEP3_NOERROR(_registry->registerDataIn(
        "signal_in", fep3::base::StreamType{fep3::base::StreamMetaType{"anonymous"}}));
    auto listener = std::make_shared<TestDataReceiver>();
    ASSERT_FEP3_NOERROR(_registry->registerDataReceiveListener("signal_in", listener));
    ASSERT_EQ(_registry->registerDataReceiveListener("unknown_signal", listener),
              fep3::ERR_NOT_FOUND);

    ASSERT_FEP3_NOERROR(_registry->unregisterDataReceiveListener("signal_in", listener));
    ASSERT_EQ(_registry->unregisterDataReceiveListener("unknown_signal", listener),
              fep3::ERR_NOT_FOUND);
}

TEST_F(NativeDataCommunication, sendAndReceiveData)
{
    auto& data_reg_sender = _sender._registry;
    auto& data_reg_receiver = _receiver._registry;

    ASSERT_TRUE(data_reg_sender->registerDataOut("string_data", fep3::base::StreamTypeString(0)));
    ASSERT_TRUE(data_reg_receiver->registerDataIn("string_data", fep3::base::StreamTypeString(0)));

    auto listener = std::make_shared<TestDataReceiver>();
    ASSERT_FEP3_NOERROR(data_reg_receiver->registerDataReceiveListener("string_data", listener));

    ASSERT_NO_THROW(auto readerqueuetest = data_reg_receiver->getReader("string_data");
                    auto writerqueuetest = data_reg_sender->getWriter("string_data"););

    auto readerreceiver_dynamic_size = std::make_shared<TestDataReceiver>();
    auto readerqueue_dynamic_size = data_reg_receiver->getReader("string_data");
    auto readerreceiver_1 = std::make_shared<TestDataReceiver>();
    auto readerqueue_1 = data_reg_receiver->getReader("string_data", 1);
    auto writerqueue = data_reg_sender->getWriter("string_data");

    init_run();

    // just write one now!
    std::string value_written = "string_written";
    // this is the time where the serialization is set at the moment
    //  ... this class will serialize while writing with copy only.
    fep3::base::DataSampleType<std::string> value_to_write(value_written);
    ASSERT_TRUE(writerqueue->write(value_to_write));

    listener->reset();
    ASSERT_TRUE(writerqueue->flush());

    // check if it is received in an asynchronous time ;-)
    listener->waitForSampleUpdate(20);

    // data triggered
    ASSERT_TRUE(listener->_last_sample);

    // assync dynamic queue
    readerreceiver_dynamic_size->reset();
    EXPECT_TRUE(readerqueue_dynamic_size->pop(*readerreceiver_dynamic_size));

    // check if it is received now ;-)
    ASSERT_TRUE(readerreceiver_dynamic_size->_last_sample);

    // assync queue 1
    readerreceiver_1->reset();
    EXPECT_TRUE(readerqueue_1->pop(*readerreceiver_1));

    // check if it is received now ;-)
    ASSERT_TRUE(readerreceiver_1->_last_sample);

    // check content
    std::string value_read_from_listener;
    {
        fep3::base::RawMemoryClassType<std::string> string_ref(value_read_from_listener);
        // expect the string length + 1 because i know the serialization
        EXPECT_EQ(listener->_last_sample->read(string_ref), value_written.length() + 1);
    }

    std::string value_read_from_reader_dynamic_size;
    {
        fep3::base::RawMemoryClassType<std::string> string_ref(value_read_from_reader_dynamic_size);
        // expect the string length + 1 because i know the serialization
        EXPECT_EQ(readerreceiver_dynamic_size->_last_sample->read(string_ref),
                  value_written.length() + 1);
    }

    std::string value_read_from_reader_1;
    {
        fep3::base::RawMemoryClassType<std::string> string_ref(value_read_from_reader_1);
        // expect the string length + 1 because i know the serialization
        EXPECT_EQ(readerreceiver_1->_last_sample->read(string_ref), value_written.length() + 1);
    }

    EXPECT_EQ(value_read_from_listener, value_read_from_reader_dynamic_size);
    EXPECT_EQ(value_read_from_listener, value_read_from_reader_1);
    EXPECT_EQ(value_read_from_listener, value_written);
}

MATCHER_P(ArrayEqual, arrayToCompare, "")
{
    for (const std::string& elem: arrayToCompare) {
        if (std::string(arg) == elem)
            return true;
    }
    return false;
}

/*
 * @detail Test the renaming of input and output signals by setting the property
 * FEP3_SIGNAL_RENAMING_INPUT_CONFIGURATION_PROPERTY and
 * FEP3_SIGNAL_RENAMING_OUTPUT_CONFIGURATION_PROPERTY from the data registry.
 * @req_id FEPDEV-5119
 */
TEST_F(NativeDataRegistry, testRenameSignals)
{
    TestClient client(fep3::rpc::IRPCDataRegistryDef::getRPCDefaultName(),
                      _service_bus->getRequester(fep3::native::testing::participant_name_default));

    ASSERT_TRUE(_configuration_service->getNode("data_registry/renaming_input"));
    ASSERT_TRUE(_configuration_service->getNode("data_registry/renaming_output"));

    // Set Properties

    if (auto property = std::dynamic_pointer_cast<fep3::base::arya::IPropertyWithExtendedAccess>(
            _configuration_service->getNode("data_registry")
                ->getChild(FEP3_SIGNAL_RENAMING_INPUT_CONFIGURATION_PROPERTY))) {
        property->setValue("a_in:a_in_alias,b_in:b_in_alias");
        property->updateObservers();
    }

    if (auto property = std::dynamic_pointer_cast<fep3::base::arya::IPropertyWithExtendedAccess>(
            _configuration_service->getNode("data_registry")
                ->getChild(FEP3_SIGNAL_RENAMING_OUTPUT_CONFIGURATION_PROPERTY))) {
        property->setValue("a_out:a_out_alias,b_out:b_out_alias");
        property->updateObservers();
    }

    // Add Signals
    fep3::base::StreamType some_stream_type{fep3::base::StreamMetaType{"meta_type_raw"}};

    ASSERT_FEP3_NOERROR(_registry->registerDataIn("a_in", some_stream_type));
    ASSERT_FEP3_NOERROR(_registry->registerDataIn("b_in", some_stream_type));
    ASSERT_FEP3_NOERROR(_registry->registerDataIn("c_in_no_alias", some_stream_type));
    ASSERT_FEP3_NOERROR(_registry->registerDataOut("a_out", some_stream_type));
    ASSERT_FEP3_NOERROR(_registry->registerDataOut("b_out", some_stream_type));
    ASSERT_FEP3_NOERROR(_registry->registerDataOut("c_out_no_alias", some_stream_type));

    // Set expectations to simulation bus
    std::vector<std::string> alias_signals_in{"b_in_alias", "a_in_alias", "c_in_no_alias"};
    std::vector<std::string> alias_signals_out{"b_out_alias", "a_out_alias", "c_out_no_alias"};
    EXPECT_CALL(*_simulation_bus.get(),
                getReader(ArrayEqual(alias_signals_in),
                          ::testing::Matcher<const fep3::IStreamType&>(::testing::_),
                          1))
        .WillRepeatedly(::testing::Invoke([](const std::string&, const fep3::IStreamType&, size_t)
                                              -> std::unique_ptr<
                                                  fep3::ISimulationBus::IDataReader> {
            return std::make_unique<::testing::NiceMock<fep3::mock::SimulationBus::DataReader>>();
        }));
    EXPECT_CALL(*_simulation_bus.get(),
                getWriter(ArrayEqual(alias_signals_out),
                          ::testing::Matcher<const fep3::IStreamType&>(::testing::_)))
        .WillRepeatedly(::testing::Invoke(
            [](const std::string&,
               const fep3::IStreamType&) -> std::unique_ptr<fep3::ISimulationBus::IDataWriter> {
                return std::make_unique<
                    ::testing::NiceMock<fep3::mock::SimulationBus::DataWriter>>();
            }));
    EXPECT_CALL(*_simulation_bus.get(), startBlockingReception(::testing::_))
        .WillRepeatedly(::testing::Invoke(
            [](const std::function<void(void)>& callback) -> void { callback(); }));
    EXPECT_CALL(*_simulation_bus.get(), stopBlockingReception())
        .WillRepeatedly(::testing::Return());

    // Start test

    // Test names with ","
    auto result = _registry->registerDataIn("a,in", some_stream_type);
    ASSERT_FALSE(result);
    EXPECT_EQ(
        std::string(result.getDescription()),
        "Signal name 'a,in' is not supported. Use alphanumeric characters and underscore only!");

    result = _registry->registerDataOut("a,out", some_stream_type);
    ASSERT_FALSE(result);
    EXPECT_EQ(
        std::string(result.getDescription()),
        "Signal name 'a,out' is not supported. Use alphanumeric characters and underscore only!");

    // Check that alias names doesn't collide with already registered signals
    // Renaming a:b and register a than b
    ASSERT_FEP3_NOERROR(_registry->registerDataOut("b_out_alias", some_stream_type));

    result = _component_registry->initialize();
    ASSERT_FALSE(result);
    EXPECT_EQ(std::string(result.getDescription()),
              "The output signal name 'b_out_alias' alias 'b_out_alias' is already registered as "
              "signal with same alias name.");

    ASSERT_FEP3_NOERROR(_registry->unregisterDataOut("b_out_alias"));

    // Now check signal renaming
    ASSERT_TRUE(_component_registry->initialize());
    ASSERT_TRUE(_registry->tense());

    ASSERT_EQ(client.getSignalInNames().size(), 3);
    ASSERT_TRUE(
        containsVector(client.getSignalInNames(), {"a_in_alias", "b_in_alias", "c_in_no_alias"}));

    ASSERT_EQ(client.getSignalOutNames().size(), 3);
    ASSERT_TRUE(containsVector(client.getSignalOutNames(),
                               {"a_out_alias", "b_out_alias", "c_out_no_alias"}));

    // Try to register new signals with already registered alias names
    // Renaming a:b and register a than b
    result = _registry->registerDataIn("a_in_alias", some_stream_type);
    ASSERT_FALSE(result);
    EXPECT_EQ(
        std::string(result.getDescription()),
        "The input signal name 'a_in_alias' is already registered as signal with same alias name.");

    result = _registry->registerDataOut("a_out_alias", some_stream_type);
    ASSERT_FALSE(result);
    EXPECT_EQ(std::string(result.getDescription()),
              "The output signal name 'a_out_alias' is already registered as signal with same "
              "alias name.");

    ASSERT_TRUE(_registry->relax());
    ASSERT_TRUE(_component_registry->deinitialize());
}

/*
 * @detail Test the renaming of input and output signals by setting the property
 * FEP3_SIGNAL_RENAMING_INPUT_CONFIGURATION_PROPERTY and
 * FEP3_SIGNAL_RENAMING_OUTPUT_CONFIGURATION_PROPERTY from the data registry after loading.
 * @req_id FEPSDK-2995
 */
TEST_F(NativeDataRegistry, testRenameSignalsAtInit)
{
    TestClient client(fep3::rpc::IRPCDataRegistryDef::getRPCDefaultName(),
                      _service_bus->getRequester(fep3::native::testing::participant_name_default));

    ASSERT_TRUE(_configuration_service->getNode("data_registry/renaming_input"));
    ASSERT_TRUE(_configuration_service->getNode("data_registry/renaming_output"));

    // Add Signals
    fep3::base::StreamType some_stream_type{fep3::base::StreamMetaType{"meta_type_raw"}};
    ASSERT_FEP3_NOERROR(_registry->registerDataIn("a_in", some_stream_type));
    ASSERT_FEP3_NOERROR(_registry->registerDataIn("b_in", some_stream_type));
    ASSERT_FEP3_NOERROR(_registry->registerDataIn("c_in_no_alias", some_stream_type));
    ASSERT_FEP3_NOERROR(_registry->registerDataOut("a_out", some_stream_type));
    ASSERT_FEP3_NOERROR(_registry->registerDataOut("b_out", some_stream_type));
    ASSERT_FEP3_NOERROR(_registry->registerDataOut("c_out_no_alias", some_stream_type));

    // Set Properties (after load)
    if (auto property = std::dynamic_pointer_cast<fep3::base::arya::IPropertyWithExtendedAccess>(
            _configuration_service->getNode("data_registry")
                ->getChild(FEP3_SIGNAL_RENAMING_INPUT_CONFIGURATION_PROPERTY))) {
        property->setValue("a_in:a_in_alias,b_in:b_in_alias");
    }

    if (auto property = std::dynamic_pointer_cast<fep3::base::arya::IPropertyWithExtendedAccess>(
            _configuration_service->getNode("data_registry")
                ->getChild(FEP3_SIGNAL_RENAMING_OUTPUT_CONFIGURATION_PROPERTY))) {
        property->setValue("a_out:a_out_alias,b_out:b_out_alias");
    }

    // Set expectations to simulation bus (called during tense())
    std::vector<std::string> alias_signals_in{"b_in_alias", "a_in_alias", "c_in_no_alias"};
    std::vector<std::string> alias_signals_out{"b_out_alias", "a_out_alias", "c_out_no_alias"};
    EXPECT_CALL(*_simulation_bus.get(),
                getReader(ArrayEqual(alias_signals_in),
                          ::testing::Matcher<const fep3::IStreamType&>(::testing::_),
                          1))
        .WillRepeatedly(::testing::Invoke([](const std::string&, const fep3::IStreamType&, size_t)
                                              -> std::unique_ptr<
                                                  fep3::ISimulationBus::IDataReader> {
            return std::make_unique<::testing::NiceMock<fep3::mock::SimulationBus::DataReader>>();
        }));
    EXPECT_CALL(*_simulation_bus.get(),
                getWriter(ArrayEqual(alias_signals_out),
                          ::testing::Matcher<const fep3::IStreamType&>(::testing::_)))
        .WillRepeatedly(::testing::Invoke(
            [](const std::string&,
               const fep3::IStreamType&) -> std::unique_ptr<fep3::ISimulationBus::IDataWriter> {
                return std::make_unique<
                    ::testing::NiceMock<fep3::mock::SimulationBus::DataWriter>>();
            }));
    EXPECT_CALL(*_simulation_bus.get(), startBlockingReception(::testing::_))
        .WillRepeatedly(::testing::Invoke(
            [](const std::function<void(void)>& callback) -> void { callback(); }));
    EXPECT_CALL(*_simulation_bus.get(), stopBlockingReception())
        .WillRepeatedly(::testing::Return());

    // check signal names
    ASSERT_EQ(client.getSignalInNames().size(), 3);
    ASSERT_TRUE(containsVector(client.getSignalInNames(), {"a_in", "b_in", "c_in_no_alias"}));

    ASSERT_EQ(client.getSignalOutNames().size(), 3);
    ASSERT_TRUE(containsVector(client.getSignalOutNames(), {"a_out", "b_out", "c_out_no_alias"}));

    // check signal renaming
    ASSERT_TRUE(_component_registry->initialize());
    ASSERT_TRUE(_registry->tense());

    ASSERT_EQ(client.getSignalInNames().size(), 3);
    ASSERT_TRUE(
        containsVector(client.getSignalInNames(), {"a_in_alias", "b_in_alias", "c_in_no_alias"}));

    ASSERT_EQ(client.getSignalOutNames().size(), 3);
    ASSERT_TRUE(containsVector(client.getSignalOutNames(),
                               {"a_out_alias", "b_out_alias", "c_out_no_alias"}));

    ASSERT_TRUE(_registry->relax());
    ASSERT_TRUE(_component_registry->deinitialize());
}

/*
 * @detail Test renaming with aliases a:c and b:c and register both
 * @req_id FEPDEV-5119
 */
TEST_F(NativeDataRegistry, testRenameSignalsSameAlias)
{
    // Test a:b and c:b is set and then a and c are registered
    if (auto property = std::dynamic_pointer_cast<fep3::base::arya::IPropertyWithExtendedAccess>(
            _configuration_service->getNode("data_registry")
                ->getChild(FEP3_SIGNAL_RENAMING_INPUT_CONFIGURATION_PROPERTY))) {
        property->setValue("a:c,b:c");
        property->updateObservers();
    }

    fep3::base::StreamType some_stream_type{fep3::base::StreamMetaType{"meta_type_raw"}};
    ASSERT_FEP3_NOERROR(_registry->registerDataIn("a", some_stream_type));
    ASSERT_FEP3_NOERROR(_registry->registerDataIn("b", some_stream_type));

    // Start test
    const auto result = _component_registry->initialize();
    ASSERT_FALSE(result);
    if (std::string(result.getDescription()) != "The input signal name 'a' alias 'c' is already "
                                                "registered as signal with same alias name." &&
        std::string(result.getDescription()) != "The input signal name 'b' alias 'c' is already "
                                                "registered as signal with same alias name.") {
        FAIL() << "Expected: The input signal name '*' alias 'c' is already registered as signal "
                  "with same alias name.";
    }
}

/*
 * @detail Test wrong value of property FEP3_SIGNAL_RENAMING_INPUT_CONFIGURATION_PROPERTY from the
 * data registry.
 * @req_id FEPDEV-5119
 */
TEST_F(NativeDataRegistry, testRenamingSignalsWrongProperty)
{
    TestClient client(fep3::rpc::IRPCDataRegistryDef::getRPCDefaultName(),
                      _service_bus->getRequester(fep3::native::testing::participant_name_default));

    ASSERT_TRUE(_configuration_service->getNode("data_registry/renaming_input"));

    // Set Properties
    if (auto property = std::dynamic_pointer_cast<fep3::base::arya::IPropertyWithExtendedAccess>(
            _configuration_service->getNode("data_registry")
                ->getChild(FEP3_SIGNAL_RENAMING_INPUT_CONFIGURATION_PROPERTY))) {
        property->setValue("a_in:a_in_alias#b_in:b_in_alias");
        property->updateObservers();
    }

    // Start test
    auto result = _component_registry->initialize();
    ASSERT_FALSE(result);
    EXPECT_EQ(std::string(result.getDescription()),
              "Line '0' ('a_in:a_in_alias#b_in:b_in_alias') "
              "doesn't contain a ':' separated key value pair 'original_name:renamed_name': "
              "'a_in:a_in_alias#b_in:b_in_alias'");

    // Set Properties
    if (auto property = std::dynamic_pointer_cast<fep3::base::arya::IPropertyWithExtendedAccess>(
            _configuration_service->getNode("data_registry")
                ->getChild(FEP3_SIGNAL_RENAMING_INPUT_CONFIGURATION_PROPERTY))) {
        property->setValue("a_in:a_in_alias,a_in:b_in_alias");
        property->updateObservers();
    }

    // Start test
    result = _component_registry->initialize();
    ASSERT_FALSE(result);
    EXPECT_EQ(std::string(result.getDescription()),
              "The key 'a_in' is not unique. It's not possible to map 'a_in' "
              "to 'a_in_alias' and 'b_in_alias'");
}

class DataReaderDestruct : public ::testing::NiceMock<fep3::mock::SimulationBus::DataReader>,
                           public test::helper::Dieable {
};

class DataWriterDestruct : public ::testing::NiceMock<fep3::mock::SimulationBus::DataWriter>,
                           public test::helper::Dieable {
};

/*
 * @detail Test the registration of register data in and out after tense
 * @req_id FEPSDK-2931
 */
TEST_F(NativeDataRegistry, testRegisterSignalAfterTense)
{
    TestClient client(fep3::rpc::IRPCDataRegistryDef::getRPCDefaultName(),
                      _service_bus->getRequester(fep3::native::testing::participant_name_default));

    // Prepare
    EXPECT_CALL(*_simulation_bus.get(), startBlockingReception(::testing::_))
        .WillRepeatedly(::testing::Invoke(
            [](const std::function<void(void)>& callback) -> void { callback(); }));
    EXPECT_CALL(*_simulation_bus.get(), stopBlockingReception())
        .WillRepeatedly(::testing::Return());

    ASSERT_TRUE(_component_registry->initialize());
    ASSERT_TRUE(_component_registry->tense());

    auto reader = std::make_unique<DataReaderDestruct>();
    auto pointer_to_reader = reader.get();
    auto writer = std::make_unique<DataWriterDestruct>();
    auto pointer_to_writer = writer.get();

    std::vector<std::string> alias_signals_in{"a"};
    std::vector<std::string> alias_signals_out{"b"};
    EXPECT_CALL(*_simulation_bus.get(),
                getReader(ArrayEqual(alias_signals_in),
                          ::testing::Matcher<const fep3::IStreamType&>(::testing::_),
                          1))
        .WillOnce(::testing::Invoke([&reader](const std::string&, const fep3::IStreamType&, size_t)
                                        -> std::unique_ptr<fep3::ISimulationBus::IDataReader> {
            return std::move(reader);
        }));
    EXPECT_CALL(*_simulation_bus.get(),
                getWriter(ArrayEqual(alias_signals_out),
                          ::testing::Matcher<const fep3::IStreamType&>(::testing::_)))
        .WillOnce(::testing::Invoke([&writer](const std::string&, const fep3::IStreamType&)
                                        -> std::unique_ptr<fep3::ISimulationBus::IDataWriter> {
            return std::move(writer);
        }));

    // Run test
    fep3::base::StreamType some_stream_type{fep3::base::StreamMetaType{"meta_type_raw"}};

    ASSERT_FEP3_NOERROR(_registry->registerDataIn("a", some_stream_type));
    ASSERT_FEP3_NOERROR(_registry->registerDataOut("b", some_stream_type));

    ASSERT_EQ(client.getSignalInNames().size(), 1);
    ASSERT_TRUE(containsVector(client.getSignalInNames(), {"a"}));
    ASSERT_EQ(client.getSignalOutNames().size(), 1);
    ASSERT_TRUE(containsVector(client.getSignalOutNames(), {"b"}));

    EXPECT_DESTRUCTION(*pointer_to_reader);
    EXPECT_DESTRUCTION(*pointer_to_writer);

    _registry->unregisterDataIn("a");
    _registry->unregisterDataOut("b");
}

/*
 * @detail Test that new stream_types are reflected by the DataRegistry and can be requested
 * @req_id FEPSDK-2929
 */
TEST_F(NativeDataCommunication, testCurrentStreamType)
{
    auto& data_reg_sender = _sender._registry;
    auto& data_reg_receiver = _receiver._registry;

    ASSERT_TRUE(data_reg_sender->registerDataOut("data", fep3::base::StreamTypeString(0)));
    ASSERT_TRUE(data_reg_receiver->registerDataIn("data", fep3::base::StreamTypeString(0)));

    auto readerqueue_1 = data_reg_receiver->getReader("data", 1);
    auto writerqueue = data_reg_sender->getWriter("data");

    auto listener = std::make_shared<TestDataReceiver>();
    ASSERT_FEP3_NOERROR(data_reg_receiver->registerDataReceiveListener("data", listener));

    init_run();

    // Check current StreamType
    EXPECT_EQ(_receiver._registry->getStreamType("data").getMetaTypeName(),
              fep3::base::arya::meta_type_string.getName());
    EXPECT_EQ(_sender._registry->getStreamType("data").getMetaTypeName(),
              fep3::base::arya::meta_type_string.getName());

    writerqueue->write(fep3::base::StreamTypePlain<uint32_t>());
    writerqueue->flush();
    listener->waitForSampleUpdate(20);

    // Check new StreamType
    EXPECT_EQ(_receiver._registry->getStreamType("data").getMetaTypeName(),
              fep3::base::arya::meta_type_plain.getName());
    EXPECT_EQ(_sender._registry->getStreamType("data").getMetaTypeName(),
              fep3::base::arya::meta_type_plain.getName());
}

/*
 * @detail Test whether a stream type containing special characters like ',' can be retrieved via
 * RPC.
 */
TEST_F(NativeDataRegistry, testCurrentStreamTypeViaRPC)
{
    TestClient client(fep3::rpc::IRPCDataRegistryDef::getRPCDefaultName(),
                      _service_bus->getRequester(fep3::native::testing::participant_name_default));

    const auto struct_name = "tTestStruct";
    const auto ddl_description = R"(<?xml version="1.0" encoding="utf-8" standalone="no"?>
<!--
Copyright 2023 CARIAD SE.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
-->
<adtf:ddl xmlns:adtf="adtf">
 <header>
  <language_version>3.00</language_version>
  <author>fep_team</author>
  <date_creation>20.02.2020</date_creation>
  <date_change>20.02.2020</date_change>
  <description>Simplistic DDL for testing purposes</description>
 </header>
 <units />
 <datatypes>
  <datatype description="predefined ADTF tBool datatype" name="tBool" size="8" />
  <datatype description="predefined ADTF tChar datatype" name="tChar" size="8" />
  <datatype description="predefined ADTF tUInt8 datatype" name="tUInt8" size="8" />
  <datatype description="predefined ADTF tInt8 datatype" name="tInt8" size="8" />
  <datatype description="predefined ADTF tUInt16 datatype" name="tUInt16" size="16" />
  <datatype description="predefined ADTF tInt16 datatype" name="tInt16" size="16" />
  <datatype description="predefined ADTF tUInt32 datatype" name="tUInt32" size="32" />
  <datatype description="predefined ADTF tInt32 datatype" name="tInt32" size="32" />
  <datatype description="predefined ADTF tUInt64 datatype" name="tUInt64" size="64" />
  <datatype description="predefined ADTF tInt64 datatype" name="tInt64" size="64" />
  <datatype description="predefined ADTF tFloat32 datatype" name="tFloat32" size="32" />
  <datatype description="predefined, ADTF tFloat64 datatype" name="tFloat64" size="64" />
 </datatypes>
 <enums>
 </enums>
 <structs>
  <struct alignment="1" name="tTestStruct" version="1">
   <element alignment="1" arraysize="1" byteorder="LE" bytepos="0" name="ui8First" type="tUInt8" default="0"/>
   <element alignment="1" arraysize="1" byteorder="LE" bytepos="1" name="ui8Second" type="tUInt8" default="0"/>
  </struct>
 </structs>
 <streams />
</adtf:ddl>)";

    ASSERT_TRUE(std::string(ddl_description).find(",") != std::string::npos)
        << "We explicitly test for special characters (',') within this test but none are part of "
           "the description provided";

    const auto stream_type_expected = fep3::base::StreamTypeDDL(struct_name, ddl_description);
    ASSERT_FEP3_NOERROR(_registry->registerDataIn("a_in", stream_type_expected));

    const auto stream_type_retrieved = client.getStreamType("a_in");
    ASSERT_EQ(stream_type_expected.getMetaTypeName(), stream_type_retrieved.getMetaTypeName());
    ASSERT_EQ(stream_type_expected.getPropertyValues(), stream_type_retrieved.getPropertyValues());
    ASSERT_EQ(stream_type_expected.getPropertyNames(), stream_type_retrieved.getPropertyNames());
    ASSERT_EQ(stream_type_expected.getPropertyTypes(), stream_type_retrieved.getPropertyTypes());
}

TEST_F(NativeDataRegistry, checkRegistrationOnServiceBus)
{
    const fep3::ComponentVersionInfo dummy_component_version_info{"3.0.1", "dummyPath", "3.1.0"};
    auto service_bus = std::make_shared<ServiceBusComponentMock>();
    auto server = std::make_shared<RPCServerMock>();
    auto logging_service = std::make_shared<LoggingServiceComponentMock>();
    auto logger = std::make_shared<Logger>();

    EXPECT_CALL(*dynamic_cast<::testing::NiceMock<fep3::mock::ServiceBus>*>(service_bus.get()),
                getServer())
        .WillRepeatedly(::testing::Return(server));
    EXPECT_CALL(*server.get(),
                registerService(::fep3::rpc::IRPCDataRegistryDef::getRPCDefaultName(),
                                ::testing::NotNull()))
        .Times(1)
        .WillRepeatedly(::testing::Return(fep3::Result()));
    EXPECT_CALL(*server.get(),
                unregisterService(::fep3::rpc::IRPCDataRegistryDef::getRPCDefaultName()))
        .Times(1)
        .WillRepeatedly(::testing::Return(fep3::Result()));

    EXPECT_CALL(*logging_service, createLogger(_)).WillRepeatedly(Return(logger));

    auto components = std::make_shared<fep3::ComponentRegistry>();
    components->registerComponent<fep3::IServiceBus>(service_bus, dummy_component_version_info);
    components->registerComponent<fep3::ILoggingService>(logging_service,
                                                         dummy_component_version_info);
    auto data_registry = std::make_shared<fep3::native::DataRegistry>();
    data_registry->createComponent(components);
    data_registry->initialize();
    data_registry->deinitialize();
    data_registry->destroyComponent();
}

/*
 * @detail Test the correct order of getReader and getWriter calls in DataRegistry's tense() method
 * getReaders must be called after getWriters
 *
 * @req_id FEPSDK-3292
 */
TEST_F(NativeDataRegistryWithMocks, testGetReaderWriterOrder)
{
    const std::string signal_in_name = "signal_in";
    const std::string signal_out_1_name = "signal_out_1";
    const std::string signal_out_2_name = "signal_out_2";
    auto mock_data_writer_1 =
        std::make_unique<::testing::NiceMock<::fep3::mock::SimulationBus::DataWriter>>();
    auto mock_data_writer_2 =
        std::make_unique<::testing::NiceMock<::fep3::mock::SimulationBus::DataWriter>>();
    auto mock_data_reader =
        std::make_unique<::testing::NiceMock<::fep3::mock::SimulationBus::DataReader>>();

    const fep3::base::StreamTypeRaw stream_type_raw;

    ASSERT_FEP3_NOERROR(_data_registry->registerDataOut(signal_out_1_name, stream_type_raw));
    ASSERT_FEP3_NOERROR(_data_registry->registerDataIn(signal_in_name, stream_type_raw));
    ASSERT_FEP3_NOERROR(_data_registry->registerDataOut(signal_out_2_name, stream_type_raw));

    ASSERT_FEP3_NOERROR(_data_registry->initialize());

    Expectation get_writer =
        EXPECT_CALL(*_simulation_bus,
                    getWriter(_,
                              ::testing::Matcher<const ::fep3::IStreamType&>(
                                  fep3::mock::StreamTypeMatcher(stream_type_raw))))
            .Times(2)
            .WillOnce(Return(ByMove(std::move(mock_data_writer_1))))
            .WillOnce(Return(ByMove(std::move(mock_data_writer_2))));

    EXPECT_CALL(*_simulation_bus, getReader(_, _, _))
        .Times(1)
        .After(get_writer)
        .WillOnce(Return(ByMove(std::move(mock_data_reader))));

    EXPECT_CALL(*_component_registry, findComponent(_simulation_bus->getComponentIID()))
        .Times(1)
        .WillOnce(::testing::Return(_simulation_bus.get()));

    ASSERT_FEP3_NOERROR(_data_registry->tense());

    EXPECT_CALL(*_component_registry, findComponent(_simulation_bus->getComponentIID()))
        .Times(1)
        .WillOnce(::testing::Return(_simulation_bus.get()));

    ASSERT_FEP3_NOERROR(_data_registry->relax());
    ASSERT_FEP3_NOERROR(_data_registry->deinitialize());
}
