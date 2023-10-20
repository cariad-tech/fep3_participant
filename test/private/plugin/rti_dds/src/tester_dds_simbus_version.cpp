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

#include "detail/test_read_write_test_class.hpp"
#include "detail/test_receiver.hpp"
#include "detail/test_samples.hpp"

#include <fep3/fep3_participant_version.h>

using namespace fep3;

Json::Value read_json(const data_read_ptr<const IDataSample>& sample)
{
    JSONCPP_STRING err;
    Json::Value root;

    const Json::CharReaderBuilder builder;
    const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    RawDataSample data;
    sample->read(data);

    const std::string json = data.getString();
    EXPECT_TRUE(reader->parse(json.c_str(), json.c_str() + json.size(), &root, &err));
    return root;
}

void check_version_info(
    Json::Value json, const std::string& participant_name, int major, int minor, int patch)
{
    EXPECT_EQ(json["participant_name"], participant_name);
    EXPECT_EQ(json["fep_version"]["major"].asInt(), major);
    EXPECT_EQ(json["fep_version"]["minor"].asInt(), minor);
    EXPECT_EQ(json["fep_version"]["patch"].asInt(), patch);
}

TYPED_TEST_SUITE(ReaderWriterTestSimulationBus, SimulationBusTypes);

/**
 * @detail We access the internal topic _built_in_topic_businfo to test the discovered information
 * of the simbus
 * @req_id
 */
TYPED_TEST(ReaderWriterTestSimulationBus, TestAvailableInformations)
{
    // We need a small sync delay because we don't call startReception(getSimulationBus());
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Use the hidden
    auto simbus_1_businfo_reader = this->getSimulationBus()->getReader(
        "_built_in_topic_businfo", base::StreamTypePlain<uint32_t>());
    ASSERT_TRUE(simbus_1_businfo_reader) << "failed to get reader from simlution bus";

    TestReceiver simbus_1_businfo_reciever;
    simbus_1_businfo_reader->pop(simbus_1_businfo_reciever);

    ASSERT_EQ(simbus_1_businfo_reciever._samples.size(), 1);
    auto json = read_json(simbus_1_businfo_reciever._samples[0]);
    ASSERT_EQ(json.isArray(), true);
    check_version_info(json[0],
                       this->_sim_participant_name_2,
                       FEP3_PARTICIPANT_LIBRARY_VERSION_MAJOR,
                       FEP3_PARTICIPANT_LIBRARY_VERSION_MINOR,
                       FEP3_PARTICIPANT_LIBRARY_VERSION_PATCH);

    // Add a late joiner
    auto simbus3 =
        this->createSimulationBus(this->_sim_participant_name_3, this->_sim_test_system_name);
    ASSERT_TRUE(simbus3) << "failed to create simulation bus";
    simbus_1_businfo_reciever.clear();

    // 1 second will fail almost every 10-20 times. Reason not found
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Read an check information from simbus_1
    simbus_1_businfo_reader->pop(simbus_1_businfo_reciever);

    ASSERT_EQ(simbus_1_businfo_reciever._samples.size(), 1);
    json = read_json(simbus_1_businfo_reciever._samples[0]);
    ASSERT_EQ(json.isArray(), true);

    check_version_info(json[0],
                       this->_sim_participant_name_2,
                       FEP3_PARTICIPANT_LIBRARY_VERSION_MAJOR,
                       FEP3_PARTICIPANT_LIBRARY_VERSION_MINOR,
                       FEP3_PARTICIPANT_LIBRARY_VERSION_PATCH);
    check_version_info(json[1],
                       this->_sim_participant_name_3,
                       FEP3_PARTICIPANT_LIBRARY_VERSION_MAJOR,
                       FEP3_PARTICIPANT_LIBRARY_VERSION_MINOR,
                       FEP3_PARTICIPANT_LIBRARY_VERSION_PATCH);

    // Read an check information from simbus_3 (late joiner)
    auto simbus_3_businfo_reader =
        dynamic_cast<ISimulationBus*>(simbus3.get())
            ->getReader("_built_in_topic_businfo", base::StreamTypePlain<uint32_t>());
    ASSERT_TRUE(simbus_3_businfo_reader) << "failed to get reader from simluation bus";
    TestReceiver simbus_3_businfo_reciever;
    // We have two updates so we take the last
    simbus_3_businfo_reader->pop(simbus_3_businfo_reciever);
    simbus_3_businfo_reader->pop(simbus_3_businfo_reciever);
    ASSERT_EQ(simbus_3_businfo_reciever._samples.size(), 2);
    json = read_json(simbus_3_businfo_reciever._samples[1]);

    // late joiner detected all participants
    check_version_info(json[0],
                       this->_sim_participant_name_1,
                       FEP3_PARTICIPANT_LIBRARY_VERSION_MAJOR,
                       FEP3_PARTICIPANT_LIBRARY_VERSION_MINOR,
                       FEP3_PARTICIPANT_LIBRARY_VERSION_PATCH);
    check_version_info(json[1],
                       this->_sim_participant_name_2,
                       FEP3_PARTICIPANT_LIBRARY_VERSION_MAJOR,
                       FEP3_PARTICIPANT_LIBRARY_VERSION_MINOR,
                       FEP3_PARTICIPANT_LIBRARY_VERSION_PATCH);

    ASSERT_TRUE(simbus3->stop());
    ASSERT_TRUE(simbus3->relax());
    ASSERT_TRUE(simbus3->deinitialize());
}
