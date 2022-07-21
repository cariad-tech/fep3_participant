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

#include "data_registry_test_fixture.h"
#include "fep3/rpc_services/data_registry/data_registry_client_stub.h"

#include "fep3/components/service_bus/rpc/fep_rpc.h"
#include "fep3/components/simulation_bus/simulation_bus_intf.h" // Used only to override IDataReceiver
#include "fep3/native_components/simulation_bus/simulation_bus.h"
#include "fep3/rpc_services/base/fep_rpc_client.h"
#include "fep3/rpc_services/data_registry/data_registry_rpc_intf_def.h"
#include "fep3/native_components/configuration/configuration_service.h"
#include "fep3/components/logging/mock/mock_logger.h"
#include "fep3/components/service_bus/mock/mock_service_bus.h"
#include "fep3/components/simulation_bus/mock/mock_simulation_bus.h"
#include "fep3/base/stream_type/mock/mock_stream_type.h"
#include "fep3/base/stream_type/default_stream_type.h"
#include "fep3/base/sample/data_sample.h"
#include "fep3/base/properties/propertynode.h"
#include <helper/gmock_destruction_helper.h>

bool containsVector(const std::vector<std::string>& source_vec,
                    const std::vector<std::string>& contain_vec)
{
    for (const auto& item : contain_vec)
    {
        bool found = false;
        for (const auto& item_source : source_vec)
        {
            if (item_source == item)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            return false;
        }
    }
    return true;
}

TEST_F(NativeDataRegistryWithMocks, testGetStreamTypeNotFound)
{
    ASSERT_FEP3_NOERROR(_data_registry->initialize());

    EXPECT_CALL(*_component_registry, findComponent(_simulation_bus->getComponentIID()))
            .Times(1).WillOnce(::testing::Return(_simulation_bus.get()));

    ASSERT_FEP3_NOERROR(_data_registry->tense());

    EXPECT_EQ(_data_registry->getStreamType("signal_name").getMetaTypeName(), "hook");

    EXPECT_CALL(*_component_registry, findComponent(_simulation_bus->getComponentIID()))
            .Times(1).WillOnce(::testing::Return(_simulation_bus.get()));

    ASSERT_FEP3_NOERROR(_data_registry->relax());
    ASSERT_FEP3_NOERROR(_data_registry->deinitialize());
}

TEST_F(NativeDataRegistryWithMocks, testGetStreamType)
{
    const std::string signal_in_name = "signal_in";
    const std::string signal_out_name = "signal_out";
    auto mock_data_writer = std::make_unique<::testing::NiceMock<::fep3::mock::DataWriter>>();
    auto mock_data_reader = std::make_unique<::testing::NiceMock<::fep3::mock::DataReader>>();

    const fep3::base::StreamTypeRaw stream_type_raw;

    ASSERT_FEP3_NOERROR(_data_registry->registerDataIn(signal_in_name, stream_type_raw));
    ASSERT_FEP3_NOERROR(_data_registry->registerDataOut(signal_out_name, stream_type_raw));

    ASSERT_FEP3_NOERROR(_data_registry->initialize());

    EXPECT_CALL(*_simulation_bus, getReader_(_, _ , _)).Times(1).WillOnce(
                Return(mock_data_reader.release()));
    EXPECT_CALL(*_simulation_bus, getWriter_(_, ::testing::Matcher<const ::fep3::IStreamType&>(fep3::mock::StreamTypeMatcher(stream_type_raw))))
            .Times(1).WillOnce(Return(mock_data_writer.release()));
    EXPECT_CALL(*_component_registry, findComponent(_simulation_bus->getComponentIID()))
            .Times(1).WillOnce(::testing::Return(_simulation_bus.get()));

    ASSERT_FEP3_NOERROR(_data_registry->tense());

    EXPECT_EQ(_data_registry->getStreamType(signal_in_name).getMetaTypeName(), stream_type_raw.getMetaTypeName());
    EXPECT_EQ(_data_registry->getStreamType(signal_out_name).getMetaTypeName(), stream_type_raw.getMetaTypeName());

    EXPECT_CALL(*_component_registry, findComponent(_simulation_bus->getComponentIID()))
            .Times(1).WillOnce(::testing::Return(_simulation_bus.get()));

    ASSERT_FEP3_NOERROR(_data_registry->relax());
    ASSERT_FEP3_NOERROR(_data_registry->deinitialize());
}

TEST_F(NativeDataRegistryWithMocks, testGetStreamTypeOfAlias)
{
    const std::string signal_in_name = "signal_in";
    const std::string signal_in_name_renamed = "signal_in_renamed";
    const std::string signal_out_name = "signal_out";
    const std::string signal_out_name_renamed = "signal_out_renamed";
    auto mock_data_writer = std::make_unique<::testing::NiceMock<::fep3::mock::DataWriter>>();
    auto mock_data_reader = std::make_unique<::testing::NiceMock<::fep3::mock::DataReader>>();
    const fep3::base::StreamTypeRaw stream_type_raw;

    ASSERT_FEP3_NOERROR(_data_registry->registerDataIn(signal_in_name, stream_type_raw));
    ASSERT_FEP3_NOERROR(_data_registry->registerDataOut(signal_out_name, stream_type_raw));

    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(*_data_registry_property_node->getChild(FEP3_SIGNAL_RENAMING_INPUT_CONFIGURATION_PROPERTY), signal_in_name + ":" + signal_in_name_renamed));
    ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(*_data_registry_property_node->getChild(FEP3_SIGNAL_RENAMING_OUTPUT_CONFIGURATION_PROPERTY), signal_out_name + ":" + signal_out_name_renamed));

    ASSERT_FEP3_NOERROR(_data_registry->initialize());

    EXPECT_CALL(*_simulation_bus, getReader_(_, _ , _)).Times(1).WillOnce(
                Return(mock_data_reader.release()));
    EXPECT_CALL(*_simulation_bus, getWriter_(_, ::testing::Matcher<const ::fep3::IStreamType&>(fep3::mock::StreamTypeMatcher(stream_type_raw))))
            .Times(1).WillOnce(Return(mock_data_writer.release()));
    EXPECT_CALL(*_component_registry, findComponent(_simulation_bus->getComponentIID()))
            .Times(1).WillOnce(::testing::Return(_simulation_bus.get()));

    ASSERT_FEP3_NOERROR(_data_registry->tense());

    EXPECT_EQ(_data_registry->getStreamType(signal_in_name_renamed).getMetaTypeName(), stream_type_raw.getMetaTypeName());
    EXPECT_EQ(_data_registry->getStreamType(signal_in_name).getMetaTypeName(), "hook");
    EXPECT_EQ(_data_registry->getStreamType(signal_out_name_renamed).getMetaTypeName(), stream_type_raw.getMetaTypeName());
    EXPECT_EQ(_data_registry->getStreamType(signal_out_name).getMetaTypeName(), "hook");

    EXPECT_CALL(*_component_registry, findComponent(_simulation_bus->getComponentIID()))
            .Times(1).WillOnce(::testing::Return(_simulation_bus.get()));

    ASSERT_FEP3_NOERROR(_data_registry->relax());
    ASSERT_FEP3_NOERROR(_data_registry->deinitialize());
}

TEST_F(NativeDataRegistryWithMocks, testResetRenamedSignals)
{
    const std::string signal_in_name = "signal_in";
    const std::string signal_in_name_renamed = "signal_in_renamed";
    const std::string signal_out_name = "signal_out";
    const std::string signal_out_name_renamed = "signal_out_renamed";
    auto mock_data_writer = std::make_unique<::testing::NiceMock<::fep3::mock::DataWriter>>();
    auto mock_data_reader = std::make_unique<::testing::NiceMock<::fep3::mock::DataReader>>();
    const fep3::base::StreamTypeRaw stream_type_raw;

    ASSERT_FEP3_NOERROR(_data_registry->registerDataIn(signal_in_name, stream_type_raw));
    ASSERT_FEP3_NOERROR(_data_registry->registerDataOut(signal_out_name, stream_type_raw));

    // Rename signals via properties
    {
        ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(*_data_registry_property_node->getChild(FEP3_SIGNAL_RENAMING_INPUT_CONFIGURATION_PROPERTY), signal_in_name + ":" + signal_in_name_renamed));
        ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(*_data_registry_property_node->getChild(FEP3_SIGNAL_RENAMING_OUTPUT_CONFIGURATION_PROPERTY), signal_out_name + ":" + signal_out_name_renamed));

        ASSERT_FEP3_NOERROR(_data_registry->initialize());

        EXPECT_CALL(*_simulation_bus, getReader_(signal_in_name_renamed, _ , _)).Times(1).WillOnce(
                    Return(mock_data_reader.release()));
        EXPECT_CALL(*_simulation_bus, getWriter_(signal_out_name_renamed, ::testing::Matcher<const ::fep3::IStreamType&>(fep3::mock::StreamTypeMatcher(stream_type_raw))))
                .Times(1).WillOnce(Return(mock_data_writer.release()));
        EXPECT_CALL(*_component_registry, findComponent(_simulation_bus->getComponentIID()))
                .Times(1).WillOnce(::testing::Return(_simulation_bus.get()));

        ASSERT_FEP3_NOERROR(_data_registry->tense());

        EXPECT_EQ(signal_in_name_renamed, _data_registry->getSignalInNames().back());
        EXPECT_EQ(signal_out_name_renamed, _data_registry->getSignalOutNames().back());

        EXPECT_CALL(*_component_registry, findComponent(_simulation_bus->getComponentIID()))
                .Times(1).WillOnce(::testing::Return(_simulation_bus.get()));

        ASSERT_FEP3_NOERROR(_data_registry->relax());
        ASSERT_FEP3_NOERROR(_data_registry->deinitialize());
    }

    auto mock_data_writer2 = std::make_unique<::testing::NiceMock<::fep3::mock::DataWriter>>();
    auto mock_data_reader2 = std::make_unique<::testing::NiceMock<::fep3::mock::DataReader>>();

    // Reset renaming property configuration
    {
        ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(*_data_registry_property_node->getChild(FEP3_SIGNAL_RENAMING_INPUT_CONFIGURATION_PROPERTY), ""));
        ASSERT_FEP3_NOERROR(fep3::base::setPropertyValue<std::string>(*_data_registry_property_node->getChild(FEP3_SIGNAL_RENAMING_OUTPUT_CONFIGURATION_PROPERTY), ""));

        ASSERT_FEP3_NOERROR(_data_registry->initialize());

        EXPECT_CALL(*_simulation_bus, getReader_(signal_in_name, _ , _)).Times(1).WillOnce(
                    Return(mock_data_reader2.release()));
        EXPECT_CALL(*_simulation_bus, getWriter_(signal_out_name, ::testing::Matcher<const ::fep3::IStreamType&>(fep3::mock::StreamTypeMatcher(stream_type_raw))))
                .Times(1).WillOnce(Return(mock_data_writer2.release()));
        EXPECT_CALL(*_component_registry, findComponent(_simulation_bus->getComponentIID()))
                .Times(1).WillOnce(::testing::Return(_simulation_bus.get()));
        EXPECT_CALL(*_simulation_bus, startBlockingReception(_))
            .WillOnce(InvokeArgument<0>());

        ASSERT_FEP3_NOERROR(_data_registry->tense());

        EXPECT_EQ(signal_in_name, _data_registry->getSignalInNames().back());
        EXPECT_EQ(signal_out_name, _data_registry->getSignalOutNames().back());

        EXPECT_CALL(*_component_registry, findComponent(_simulation_bus->getComponentIID()))
                .Times(1).WillOnce(::testing::Return(_simulation_bus.get()));

        ASSERT_FEP3_NOERROR(_data_registry->relax());
        ASSERT_FEP3_NOERROR(_data_registry->deinitialize());
    }
}

class TestClient : public fep3::rpc::RPCServiceClient<fep3::rpc_stubs::RPCDataRegistryClientStub, fep3::rpc::IRPCDataRegistryDef>
{
private:
    typedef fep3::rpc::RPCServiceClient<fep3::rpc_stubs::RPCDataRegistryClientStub, fep3::rpc::IRPCDataRegistryDef> base_type;

public:
    using base_type::GetStub;

    TestClient(const char* server_object_name,
        std::shared_ptr<fep3::IRPCRequester> rpc) : base_type(server_object_name, rpc)
    {
    }

    std::vector<std::string> getSignalInNames() const
    {
        try
        {
            return a_util::strings::split(GetStub().getSignalInNames(), ",");
        }
        catch (jsonrpc::JsonRpcException&)
        {
            ADD_FAILURE();
        }
        return std::vector<std::string>{};
    }

    std::vector<std::string> getSignalOutNames() const
    {
        try
        {
            return a_util::strings::split(GetStub().getSignalOutNames(), ",");
        }
        catch (jsonrpc::JsonRpcException&)
        {
            ADD_FAILURE();
        }
        return std::vector<std::string>{};
    }

    fep3::base::StreamType getStreamType(const std::string& signal_name) const
    {
        try
        {
            Json::Value json_value = GetStub().getStreamType(signal_name);
            fep3::base::StreamType stream_type(fep3::base::StreamMetaType(json_value["meta_type"].asString()));
            std::vector<std::string> property_names = a_util::strings::split(json_value["properties"]["names"].asString(), ",");
            std::vector<std::string> property_values = a_util::strings::split(json_value["properties"]["values"].asString(), ",", true);
            std::vector<std::string> property_types = a_util::strings::split(json_value["properties"]["types"].asString(), ",");
            for (decltype(property_names)::size_type i = 0; i < property_names.size(); ++i)
            {
                stream_type.setProperty(property_names.at(i), property_values.at(i), property_types.at(i));
            }
            return stream_type;
        }
        catch (jsonrpc::JsonRpcException&)
        {
            ADD_FAILURE();
            return fep3::base::StreamType{ fep3::base::StreamMetaType{"void"} };
        }
    }
};

// Dummy class
struct TestDataReceiver : public fep3::ISimulationBus::IDataReceiver
{
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
        while (trycount > 0)
        {
            if (_last_sample)
            {
                break;
            }
            else
            {
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
    {}

    void SetUp() override
    {
        _sender.SetUp();
        _receiver.SetUp();
    }

    void TearDown() override
    {
        stop_deinit();
    }

    void init_run()
    {
        ASSERT_TRUE(fep3::isOk(_sender._component_registry->initialize()));
        ASSERT_TRUE(fep3::isOk(_receiver._component_registry->initialize()));
        ASSERT_TRUE(fep3::isOk(_sender._component_registry->tense()));
        ASSERT_TRUE(fep3::isOk(_receiver._component_registry->tense()));
        ASSERT_TRUE(fep3::isOk(_sender._component_registry->start()));
        ASSERT_TRUE(fep3::isOk(_receiver._component_registry->start()));
        _is_running = true;
    }

    void stop_deinit()
    {
        if (_is_running)
        {
            EXPECT_TRUE(fep3::isOk(_receiver._component_registry->stop()));
            EXPECT_TRUE(fep3::isOk(_sender._component_registry->stop()));
            EXPECT_TRUE(fep3::isOk(_receiver._component_registry->relax()));
            EXPECT_TRUE(fep3::isOk(_sender._component_registry->relax()));
            EXPECT_TRUE(fep3::isOk(_receiver._component_registry->deinitialize()));
            EXPECT_TRUE(fep3::isOk(_sender._component_registry->deinitialize()));
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

    ASSERT_FEP3_NOERROR(_registry->registerDataIn("a_in", fep3::base::StreamType{ fep3::base::StreamMetaType{"meta_type_raw"} }));
    //we do not care which meta type is use ... we support everything in data registry (all kind)
    //we do not check any special support for types becuse we can deal with every thing and
    // we will deal with special types like DDL for mapping and something like that
    ASSERT_FEP3_NOERROR(_registry->registerDataIn("b_in", fep3::base::StreamType{ fep3::base::StreamMetaType{"unknown_type"} }));
    //we can not register it a second time with a different type
    ASSERT_EQ(_registry->registerDataIn("a_in", fep3::base::StreamType{ fep3::base::StreamMetaType{"meta_type_ddl"} }), fep3::ERR_INVALID_TYPE);

    ASSERT_FEP3_NOERROR(_registry->registerDataOut("a_out", fep3::base::StreamType{ fep3::base::StreamMetaType{"meta_type_raw"} }));
    //we also support unknown types
    ASSERT_FEP3_NOERROR(_registry->registerDataOut("b_out", fep3::base::StreamType{ fep3::base::StreamMetaType{"unknown_type"} }));
    //we can not register it a second time with a different type
    ASSERT_EQ(_registry->registerDataOut("a_out", fep3::base::StreamType{ fep3::base::StreamMetaType{"meta_type_ddl"} }), fep3::ERR_INVALID_TYPE);

    ASSERT_EQ(client.getSignalInNames().size(), 2);
    ASSERT_TRUE(containsVector(client.getSignalInNames(),
                               { "a_in" , "b_in"}));
    ASSERT_EQ(client.getSignalOutNames().size(), 2);
    ASSERT_TRUE(containsVector(client.getSignalOutNames(),
                               { "a_out" , "b_out"}));

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
    ASSERT_FEP3_NOERROR(_registry->registerDataOut("signal_out", fep3::base::StreamType{ fep3::base::StreamMetaType{"anonymous"} }));
    auto writer = _registry->getWriter("signal_out");
    ASSERT_TRUE(writer);
    ASSERT_FALSE(_registry->getWriter("unknown_signal"));
}

TEST_F(NativeDataRegistry, testReader)
{
    ASSERT_FEP3_NOERROR(_registry->registerDataOut("signal_in", fep3::base::StreamType{ fep3::base::StreamMetaType{"meta_type_raw"} }));

    ASSERT_FEP3_NOERROR(_registry->registerDataIn("signal_in", fep3::base::StreamType{ fep3::base::StreamMetaType{"meta_type_raw"} }));
    auto reader1 = _registry->getReader("signal_in");
    ASSERT_TRUE(reader1);
    auto reader2 = _registry->getReader("signal_in");
    ASSERT_TRUE(reader2);
    ASSERT_FALSE(_registry->getReader("unknown_signal"));
}

TEST_F(NativeDataRegistry, testListenerRegistration)
{
    ASSERT_FEP3_NOERROR(_registry->registerDataIn("signal_in", fep3::base::StreamType{ fep3::base::StreamMetaType{ "anonymous" } }));
    auto listener = std::make_shared<TestDataReceiver>();
    ASSERT_FEP3_NOERROR(_registry->registerDataReceiveListener("signal_in", listener));
    ASSERT_EQ(_registry->registerDataReceiveListener("unknown_signal", listener), fep3::ERR_NOT_FOUND);

    ASSERT_FEP3_NOERROR(_registry->unregisterDataReceiveListener("signal_in", listener));
    ASSERT_EQ(_registry->unregisterDataReceiveListener("unknown_signal", listener), fep3::ERR_NOT_FOUND);
}



TEST_F(NativeDataCommunication, sendAndReceiveData)
{
    auto& data_reg_sender = _sender._registry;
    auto& data_reg_receiver = _receiver._registry;

    ASSERT_TRUE(fep3::isOk(data_reg_sender->registerDataOut("string_data", fep3::base::StreamTypeString(0))));
    ASSERT_TRUE(fep3::isOk(data_reg_receiver->registerDataIn("string_data", fep3::base::StreamTypeString(0))));

    auto listener = std::make_shared<TestDataReceiver>();
    ASSERT_FEP3_NOERROR(data_reg_receiver->registerDataReceiveListener("string_data", listener));

    ASSERT_NO_THROW(
        auto readerqueuetest = data_reg_receiver->getReader("string_data");
        auto writerqueuetest = data_reg_sender->getWriter("string_data");
    );

    auto readerreceiver_dynamic_size = std::make_shared<TestDataReceiver>();
    auto readerqueue_dynamic_size = data_reg_receiver->getReader("string_data");
    auto readerreceiver_1 = std::make_shared<TestDataReceiver>();
    auto readerqueue_1 = data_reg_receiver->getReader("string_data", 1);
    auto writerqueue = data_reg_sender->getWriter("string_data");

    init_run();

    //just write one now!
    std::string value_written = "string_written";
    //this is the time where the serialization is set at the moment
    // ... this class will serialize while writing with copy only.
    fep3::base::DataSampleType<std::string> value_to_write(value_written);
    ASSERT_TRUE(fep3::isOk(writerqueue->write(value_to_write)));

    listener->reset();
    ASSERT_TRUE(fep3::isOk(writerqueue->flush()));

    //check if it is received in an asynchronous time ;-)
    listener->waitForSampleUpdate(20);

    //data triggered
    ASSERT_TRUE(listener->_last_sample);

    //assync dynamic queue
    readerreceiver_dynamic_size->reset();
    EXPECT_TRUE(fep3::isOk(readerqueue_dynamic_size->pop(*readerreceiver_dynamic_size)));

    //check if it is received now ;-)
    ASSERT_TRUE(readerreceiver_dynamic_size->_last_sample);

    //assync queue 1
    readerreceiver_1->reset();
    EXPECT_TRUE(fep3::isOk(readerqueue_1->pop(*readerreceiver_1)));

    //check if it is received now ;-)
    ASSERT_TRUE(readerreceiver_1->_last_sample);

    //check content
    std::string value_read_from_listener;
    {
        fep3::base::RawMemoryClassType<std::string> string_ref(value_read_from_listener);
        //expect the string length + 1 because i know the serialization
        EXPECT_EQ(listener->_last_sample->read(string_ref), value_written.length() + 1);
    }

    std::string value_read_from_reader_dynamic_size;
    {
        fep3::base::RawMemoryClassType<std::string> string_ref(value_read_from_reader_dynamic_size);
        //expect the string length + 1 because i know the serialization
        EXPECT_EQ(readerreceiver_dynamic_size->_last_sample->read(string_ref), value_written.length() + 1);
    }

    std::string value_read_from_reader_1;
    {
        fep3::base::RawMemoryClassType<std::string> string_ref(value_read_from_reader_1);
        //expect the string length + 1 because i know the serialization
        EXPECT_EQ(readerreceiver_1->_last_sample->read(string_ref), value_written.length() + 1);
    }

    EXPECT_EQ(value_read_from_listener, value_read_from_reader_dynamic_size);
    EXPECT_EQ(value_read_from_listener, value_read_from_reader_1);
    EXPECT_EQ(value_read_from_listener, value_written);
}

MATCHER_P(ArrayEqual, arrayToCompare, "")
{
    for (const std::string& elem : arrayToCompare)
    {
        if (std::string(arg) == elem)
            return true;
    }
    return false;
}

/*
 * @detail Test the renaming of input and output signals by setting the property FEP3_SIGNAL_RENAMING_INPUT_CONFIGURATION_PROPERTY
 * and FEP3_SIGNAL_RENAMING_OUTPUT_CONFIGURATION_PROPERTY
 * from the data registry.
 * @req_id FEPDEV-5119
 */
TEST_F(NativeDataRegistry, testRenameSignals)
{
    TestClient client(fep3::rpc::IRPCDataRegistryDef::getRPCDefaultName(),
        _service_bus->getRequester(fep3::native::testing::participant_name_default));

    ASSERT_TRUE(_configuration_service->getNode("data_registry/renaming_input"));
    ASSERT_TRUE(_configuration_service->getNode("data_registry/renaming_output"));

    // Set Properties

    if (auto property = std::dynamic_pointer_cast<fep3::base::arya::IPropertyWithExtendedAccess>
        (_configuration_service->getNode("data_registry")->getChild(FEP3_SIGNAL_RENAMING_INPUT_CONFIGURATION_PROPERTY)))
    {
        property->setValue("a_in:a_in_alias,b_in:b_in_alias");
        property->updateObservers();
    }

    if (auto property = std::dynamic_pointer_cast<fep3::base::arya::IPropertyWithExtendedAccess>
        (_configuration_service->getNode("data_registry")->getChild(FEP3_SIGNAL_RENAMING_OUTPUT_CONFIGURATION_PROPERTY)))
    {
        property->setValue("a_out:a_out_alias,b_out:b_out_alias");
        property->updateObservers();
    }

    // Add Signals
    fep3::base::StreamType some_stream_type{ fep3::base::StreamMetaType{"meta_type_raw"} };

    ASSERT_FEP3_NOERROR(_registry->registerDataIn("a_in", some_stream_type));
    ASSERT_FEP3_NOERROR(_registry->registerDataIn("b_in", some_stream_type));
    ASSERT_FEP3_NOERROR(_registry->registerDataIn("c_in_no_alias", some_stream_type));
    ASSERT_FEP3_NOERROR(_registry->registerDataOut("a_out", some_stream_type));
    ASSERT_FEP3_NOERROR(_registry->registerDataOut("b_out", some_stream_type));
    ASSERT_FEP3_NOERROR(_registry->registerDataOut("c_out_no_alias", some_stream_type));

    // Set expectations to simulation bus
    std::vector<std::string> alias_signals_in{ "b_in_alias", "a_in_alias", "c_in_no_alias" }; 
    std::vector<std::string> alias_signals_out{ "b_out_alias", "a_out_alias", "c_out_no_alias" };
    EXPECT_CALL(*_simulation_bus.get(), getReader_(ArrayEqual(alias_signals_in), ::testing::Matcher<const fep3::IStreamType&>(::testing::_), 1)).WillRepeatedly(::testing::Invoke(
        [](const std::string&, const fep3::IStreamType&, size_t) -> fep3::ISimulationBus::IDataReader*
    {
        return new ::testing::NiceMock<fep3::mock::DataReader>();
    }));
    EXPECT_CALL(*_simulation_bus.get(), getWriter_(ArrayEqual(alias_signals_out), ::testing::Matcher<const fep3::IStreamType&>(::testing::_))).WillRepeatedly(::testing::Invoke(
        [](const std::string&, const fep3::IStreamType&) -> fep3::ISimulationBus::IDataWriter*
    {
        return new ::testing::NiceMock<fep3::mock::DataWriter>();
    }));
    EXPECT_CALL(*_simulation_bus.get(), startBlockingReception(::testing::_)).WillRepeatedly(::testing::Invoke(
        [](const std::function<void(void)> & callback) -> void
        {
            callback();
        }));
    EXPECT_CALL(*_simulation_bus.get(), stopBlockingReception()).WillRepeatedly(::testing::Return());

    // Start test

    // Test names with "," 
    auto result = _registry->registerDataIn("a,in", some_stream_type);
    ASSERT_TRUE(fep3::isFailed(result));
    EXPECT_EQ(std::string(result.getDescription()), "Signal name 'a,in' is not supported. Use alphanumeric characters and underscore only!");

    result = _registry->registerDataOut("a,out", some_stream_type);
    ASSERT_TRUE(fep3::isFailed(result));
    EXPECT_EQ(std::string(result.getDescription()), "Signal name 'a,out' is not supported. Use alphanumeric characters and underscore only!");

    // Check that alias names doesn't collide with already registered signals
    // Renaming a:b and register a than b
    ASSERT_FEP3_NOERROR(_registry->registerDataOut("b_out_alias", some_stream_type));
    
    result = _component_registry->initialize();
    ASSERT_TRUE(fep3::isFailed(result));
    EXPECT_EQ(std::string(result.getDescription()), "The output signal name 'b_out_alias' alias 'b_out_alias' is already registered as signal with same alias name.");

    ASSERT_FEP3_NOERROR(_registry->unregisterDataOut("b_out_alias"));

    // Now check signal renaming
    ASSERT_TRUE(fep3::isOk(_component_registry->initialize()));
    ASSERT_TRUE(fep3::isOk(_registry->tense()));

    ASSERT_EQ(client.getSignalInNames().size(), 3);
    ASSERT_TRUE(containsVector(client.getSignalInNames(),
        { "a_in_alias" , "b_in_alias", "c_in_no_alias" }));

    ASSERT_EQ(client.getSignalOutNames().size(), 3);
    ASSERT_TRUE(containsVector(client.getSignalOutNames(),
        { "a_out_alias" , "b_out_alias", "c_out_no_alias" }));


    // Try to register new signals with already registered alias names
    // Renaming a:b and register a than b
    result = _registry->registerDataIn("a_in_alias", some_stream_type);
    ASSERT_TRUE(fep3::isFailed(result));
    EXPECT_EQ(std::string(result.getDescription()), "The input signal name 'a_in_alias' is already registered as signal with same alias name.");

    result = _registry->registerDataOut("a_out_alias", some_stream_type);
    ASSERT_TRUE(fep3::isFailed(result));
    EXPECT_EQ(std::string(result.getDescription()), "The output signal name 'a_out_alias' is already registered as signal with same alias name.");
    
    ASSERT_TRUE(fep3::isOk(_registry->relax()));
    ASSERT_TRUE(fep3::isOk(_component_registry->deinitialize()));
}

/*
 * @detail Test the renaming of input and output signals by setting the property FEP3_SIGNAL_RENAMING_INPUT_CONFIGURATION_PROPERTY
 * and FEP3_SIGNAL_RENAMING_OUTPUT_CONFIGURATION_PROPERTY
 * from the data registry after loading.
 * @req_id FEPSDK-2995
 */
TEST_F(NativeDataRegistry, testRenameSignalsAtInit)
{
    TestClient client(fep3::rpc::IRPCDataRegistryDef::getRPCDefaultName(),
        _service_bus->getRequester(fep3::native::testing::participant_name_default));

    ASSERT_TRUE(_configuration_service->getNode("data_registry/renaming_input"));
    ASSERT_TRUE(_configuration_service->getNode("data_registry/renaming_output"));

    // Add Signals
    fep3::base::StreamType some_stream_type{ fep3::base::StreamMetaType{"meta_type_raw"} };
    ASSERT_FEP3_NOERROR(_registry->registerDataIn("a_in", some_stream_type));
    ASSERT_FEP3_NOERROR(_registry->registerDataIn("b_in", some_stream_type));
    ASSERT_FEP3_NOERROR(_registry->registerDataIn("c_in_no_alias", some_stream_type));
    ASSERT_FEP3_NOERROR(_registry->registerDataOut("a_out", some_stream_type));
    ASSERT_FEP3_NOERROR(_registry->registerDataOut("b_out", some_stream_type));
    ASSERT_FEP3_NOERROR(_registry->registerDataOut("c_out_no_alias", some_stream_type));

    // Set Properties (after load)
    if (auto property = std::dynamic_pointer_cast<fep3::base::arya::IPropertyWithExtendedAccess>
        (_configuration_service->getNode("data_registry")->getChild(FEP3_SIGNAL_RENAMING_INPUT_CONFIGURATION_PROPERTY)))
    {
        property->setValue("a_in:a_in_alias,b_in:b_in_alias");
    }

    if (auto property = std::dynamic_pointer_cast<fep3::base::arya::IPropertyWithExtendedAccess>
        (_configuration_service->getNode("data_registry")->getChild(FEP3_SIGNAL_RENAMING_OUTPUT_CONFIGURATION_PROPERTY)))
    {
        property->setValue("a_out:a_out_alias,b_out:b_out_alias");
    }

    // Set expectations to simulation bus (called during tense())
    std::vector<std::string> alias_signals_in{ "b_in_alias", "a_in_alias", "c_in_no_alias" };
    std::vector<std::string> alias_signals_out{ "b_out_alias", "a_out_alias", "c_out_no_alias" };
    EXPECT_CALL(*_simulation_bus.get(), getReader_(ArrayEqual(alias_signals_in), ::testing::Matcher<const fep3::IStreamType&>(::testing::_), 1)).WillRepeatedly(::testing::Invoke(
        [](const std::string&, const fep3::IStreamType&, size_t) -> fep3::ISimulationBus::IDataReader*
    {
        return new ::testing::NiceMock<fep3::mock::DataReader>();
    }));
    EXPECT_CALL(*_simulation_bus.get(), getWriter_(ArrayEqual(alias_signals_out), ::testing::Matcher<const fep3::IStreamType&>(::testing::_))).WillRepeatedly(::testing::Invoke(
        [](const std::string&, const fep3::IStreamType&) -> fep3::ISimulationBus::IDataWriter*
    {
        return new ::testing::NiceMock<fep3::mock::DataWriter>();
    }));
    EXPECT_CALL(*_simulation_bus.get(), startBlockingReception(::testing::_)).WillRepeatedly(::testing::Invoke(
        [](const std::function<void(void)> & callback) -> void
    {
        callback();
    }));
    EXPECT_CALL(*_simulation_bus.get(), stopBlockingReception()).WillRepeatedly(::testing::Return());

    // check signal names
    ASSERT_EQ(client.getSignalInNames().size(), 3);
    ASSERT_TRUE(containsVector(client.getSignalInNames(),
        { "a_in" , "b_in", "c_in_no_alias" }));

    ASSERT_EQ(client.getSignalOutNames().size(), 3);
    ASSERT_TRUE(containsVector(client.getSignalOutNames(),
        { "a_out" , "b_out", "c_out_no_alias" }));

    // check signal renaming
    ASSERT_TRUE(fep3::isOk(_component_registry->initialize()));
    ASSERT_TRUE(fep3::isOk(_registry->tense()));

    ASSERT_EQ(client.getSignalInNames().size(), 3);
    ASSERT_TRUE(containsVector(client.getSignalInNames(),
        { "a_in_alias" , "b_in_alias", "c_in_no_alias" }));

    ASSERT_EQ(client.getSignalOutNames().size(), 3);
    ASSERT_TRUE(containsVector(client.getSignalOutNames(),
        { "a_out_alias" , "b_out_alias", "c_out_no_alias" }));

    ASSERT_TRUE(fep3::isOk(_registry->relax()));
    ASSERT_TRUE(fep3::isOk(_component_registry->deinitialize()));
}

/*
 * @detail Test renaming with aliases a:c and b:c and register both
 * @req_id FEPDEV-5119
 */
TEST_F(NativeDataRegistry, testRenameSignalsSameAlias)
{
    // Test a:b and c:b is set and then a and c are registered
    if (auto property = std::dynamic_pointer_cast<fep3::base::arya::IPropertyWithExtendedAccess>
        (_configuration_service->getNode("data_registry")->getChild(FEP3_SIGNAL_RENAMING_INPUT_CONFIGURATION_PROPERTY)))
    {
        property->setValue("a:c,b:c");
        property->updateObservers();
    }

    fep3::base::StreamType some_stream_type{ fep3::base::StreamMetaType{"meta_type_raw"} };
    ASSERT_FEP3_NOERROR(_registry->registerDataIn("a", some_stream_type));
    ASSERT_FEP3_NOERROR(_registry->registerDataIn("b", some_stream_type));

    // Start test
    auto result = _component_registry->initialize();
    ASSERT_TRUE(fep3::isFailed(result));
    if (std::string(result.getDescription()) != "The input signal name 'a' alias 'c' is already registered as signal with same alias name."
        && std::string(result.getDescription()) != "The input signal name 'b' alias 'c' is already registered as signal with same alias name.")
    {
        FAIL() << "Expected: The input signal name '*' alias 'c' is already registered as signal with same alias name.";
    }
}

/*
 * @detail Test wrong value of property FEP3_SIGNAL_RENAMING_INPUT_CONFIGURATION_PROPERTY from the data registry.
 * @req_id FEPDEV-5119
 */
TEST_F(NativeDataRegistry, testRenamingSignalsWrongProperty)
{
    TestClient client(fep3::rpc::IRPCDataRegistryDef::getRPCDefaultName(),
        _service_bus->getRequester(fep3::native::testing::participant_name_default));

    ASSERT_TRUE(_configuration_service->getNode("data_registry/renaming_input"));

    // Set Properties
    if (auto property = std::dynamic_pointer_cast<fep3::base::arya::IPropertyWithExtendedAccess>
        (_configuration_service->getNode("data_registry")->getChild(FEP3_SIGNAL_RENAMING_INPUT_CONFIGURATION_PROPERTY)))
    {
        property->setValue("a_in:a_in_alias#b_in:b_in_alias");
        property->updateObservers();
    }

    // Start test
    auto result = _component_registry->initialize();
    ASSERT_TRUE(fep3::isFailed(result));
    EXPECT_EQ(std::string(result.getDescription()), "Line '0' ('a_in:a_in_alias#b_in:b_in_alias') "
        "doesn't contain a ':' separated key value pair 'original_name:renamed_name': 'a_in:a_in_alias#b_in:b_in_alias'");

    // Set Properties
    if (auto property = std::dynamic_pointer_cast<fep3::base::arya::IPropertyWithExtendedAccess>
        (_configuration_service->getNode("data_registry")->getChild(FEP3_SIGNAL_RENAMING_INPUT_CONFIGURATION_PROPERTY)))
    {
        property->setValue("a_in:a_in_alias,a_in:b_in_alias");
        property->updateObservers();
    }

    // Start test
    result = _component_registry->initialize();
    ASSERT_TRUE(fep3::isFailed(result));
    EXPECT_EQ(std::string(result.getDescription()), "The key 'a_in' is not unique. It's not possible to map 'a_in' "
        "to 'a_in_alias' and 'b_in_alias'");

}

class DataReaderDestruct: public ::testing::NiceMock<fep3::mock::DataReader>,
    public test::helper::Dieable
{
};

class DataWriterDestruct: public ::testing::NiceMock<fep3::mock::DataWriter>,
    public test::helper::Dieable
{
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
    EXPECT_CALL(*_simulation_bus.get(), startBlockingReception(::testing::_)).WillRepeatedly(::testing::Invoke(
        [](const std::function<void(void)> & callback) -> void
    {
        callback();
    }));
    EXPECT_CALL(*_simulation_bus.get(), stopBlockingReception()).WillRepeatedly(::testing::Return());

    ASSERT_TRUE(fep3::isOk(_component_registry->initialize()));
    ASSERT_TRUE(fep3::isOk(_component_registry->tense()));

    auto reader = new DataReaderDestruct();
    auto writer = new DataWriterDestruct();

    std::vector<std::string> alias_signals_in{ "a" };
    std::vector<std::string> alias_signals_out{ "b" };
    EXPECT_CALL(*_simulation_bus.get(), getReader_(ArrayEqual(alias_signals_in), ::testing::Matcher<const fep3::IStreamType&>(::testing::_), 1)).Times(1).WillRepeatedly(::testing::Invoke(
        [reader](const std::string&, const fep3::IStreamType&, size_t) -> fep3::ISimulationBus::IDataReader*
    {
        return reader;
    }));
    EXPECT_CALL(*_simulation_bus.get(), getWriter_(ArrayEqual(alias_signals_out), ::testing::Matcher<const fep3::IStreamType&>(::testing::_))).Times(1).WillRepeatedly(::testing::Invoke(
        [writer](const std::string&, const fep3::IStreamType&) -> fep3::ISimulationBus::IDataWriter*
    {
        return writer;
    }));
    
    // Run test
    fep3::base::StreamType some_stream_type{ fep3::base::StreamMetaType{"meta_type_raw"} };

    ASSERT_FEP3_NOERROR(_registry->registerDataIn("a", some_stream_type));
    ASSERT_FEP3_NOERROR(_registry->registerDataOut("b", some_stream_type));

    ASSERT_EQ(client.getSignalInNames().size(), 1);
    ASSERT_TRUE(containsVector(client.getSignalInNames(),
        { "a" }));
    ASSERT_EQ(client.getSignalOutNames().size(), 1);
    ASSERT_TRUE(containsVector(client.getSignalOutNames(),
        { "b" }));
    
    EXPECT_DESTRUCTION(*reader);
    EXPECT_DESTRUCTION(*writer);

    _registry->unregisterDataIn("a");
    _registry->unregisterDataOut("b");
}

/*
 * @detail Test that new stream_types are reflected by the DataRegistry and can be requested
 * @req_id FEPSDK-2929
 */
TEST_F(NativeDataCommunication, testCurrentStreamTypeViaRPC)
{
    auto& data_reg_sender = _sender._registry;
    auto& data_reg_receiver = _receiver._registry;

    ASSERT_TRUE(fep3::isOk(data_reg_sender->registerDataOut("data", fep3::base::StreamTypeString(0))));
    ASSERT_TRUE(fep3::isOk(data_reg_receiver->registerDataIn("data", fep3::base::StreamTypeString(0))));

    auto readerqueue_1 = data_reg_receiver->getReader("data", 1);
    auto writerqueue = data_reg_sender->getWriter("data");

    auto listener = std::make_shared<TestDataReceiver>();
    ASSERT_FEP3_NOERROR(data_reg_receiver->registerDataReceiveListener("data", listener));

    init_run();

    //Check current StreamType
    EXPECT_EQ(_receiver._registry->getStreamType("data").getMetaTypeName(), fep3::base::arya::meta_type_string.getName());
    EXPECT_EQ(_sender._registry->getStreamType("data").getMetaTypeName(), fep3::base::arya::meta_type_string.getName());

    writerqueue->write(fep3::base::StreamTypePlain<uint32_t>());
    writerqueue->flush();
    listener->waitForSampleUpdate(20);

    //Check new StreamType
    EXPECT_EQ(_receiver._registry->getStreamType("data").getMetaTypeName(), fep3::base::arya::meta_type_plain.getName());
    EXPECT_EQ(_sender._registry->getStreamType("data").getMetaTypeName(), fep3::base::arya::meta_type_plain.getName());
}

TEST_F(NativeDataRegistry, checkRegistrationOnServiceBus)
{
    const fep3::ComponentVersionInfo dummy_component_version_info{ "3.0.1","dummyPath", "3.1.0" };
    auto service_bus = std::make_shared<::testing::NiceMock<fep3::mock::ServiceBusComponent>>();
    auto server = std::make_shared<::testing::NiceMock<fep3::mock::RPCServer>>();

    EXPECT_CALL(*dynamic_cast<::testing::NiceMock<fep3::mock::ServiceBusComponent>*>(service_bus.get()), getServer()).WillRepeatedly(::testing::Return(server));
    EXPECT_CALL(*server.get(), registerService(::fep3::rpc::IRPCDataRegistryDef::getRPCDefaultName(), ::testing::NotNull())).Times(1).WillRepeatedly(::testing::Return(fep3::Result()));
    EXPECT_CALL(*server.get(), unregisterService(::fep3::rpc::IRPCDataRegistryDef::getRPCDefaultName())).Times(1).WillRepeatedly(::testing::Return(fep3::Result()));

    auto components = std::make_shared< fep3::ComponentRegistry >();
    components->registerComponent<fep3::IServiceBus>(service_bus, dummy_component_version_info);
    auto data_registry = std::make_shared<fep3::native::DataRegistry>();
    data_registry->createComponent(components);
    data_registry->initialize();
    data_registry->deinitialize();
    data_registry->destroyComponent();
}
