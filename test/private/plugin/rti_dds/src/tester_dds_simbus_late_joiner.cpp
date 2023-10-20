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
#include "detail/test_submitter.hpp"

#include <fep3/base/sample/mock/mock_data_sample.h>
#include <fep3/base/stream_type/mock/mock_stream_type.h>
#include <fep3/components/simulation_bus/mock_simulation_bus.h>

#include <helper/gmock_async_helper.h>

/**
 * @detail Test send and receive of samples with one listener on a different domain
 * @req_id FEPSDK-Sample
 */

TYPED_TEST_SUITE(ReaderWriterTestSimulationBus, SimulationBusTypes);

TYPED_TEST(ReaderWriterTestSimulationBus, SendAndReceiveSamplesLateJoiner)
{
    std::string topic = makePlatformDepName("breadcrumb");

    uint32_t sparrow_domain_id = this->randomDomainId();

    /*----------------------------------------------------------------------------
     *  create the simulation_buses for the birds
     *----------------------------------------------------------------------------*/
    auto sparrow_simulation_bus_sheila = this->createSimulationBusDep(sparrow_domain_id, "Sheila");
    auto sparrow_simulation_bus_scot = this->createSimulationBusDep(sparrow_domain_id, "Scot");

    {
        std::vector<uint32_t> data_sample_value_numbers{0, 1, 2};
        std::vector<std::string> data_sample_value_strings{"zero", "one", "two"};

        /*----------------------------------------------------------------------------
         *  add Sheila
         *----------------------------------------------------------------------------*/
        TestSubmitter sparrow_submitter_sheila(
            dynamic_cast<ISimulationBus*>(sparrow_simulation_bus_sheila.get()),
            topic,
            fep3::base::StreamTypePlain<uint32_t>());

        /*----------------------------------------------------------------------------
         *  add Scot
         *----------------------------------------------------------------------------*/
        auto sparrow_reader_scot = dynamic_cast<ISimulationBus*>(sparrow_simulation_bus_scot.get())
                                       ->getReader(topic, fep3::base::StreamTypePlain<uint32_t>());
        const auto& mock_sparrow_receiver_scot =
            std::make_shared<::testing::StrictMock<fep3::mock::SimulationBus::DataReceiver>>();
        sparrow_reader_scot->reset(mock_sparrow_receiver_scot);
        test::helper::Notification sparrow_receiver_scot_all_items_received;
        { // setting of expectations for mock_sparrow_receiver_scot
            ::testing::InSequence sequence;
            EXPECT_CALL(*mock_sparrow_receiver_scot.get(),
                        call(::testing::Matcher<const data_read_ptr<const IStreamType>&>(
                            fep3::mock::StreamTypeSmartPtrMatcher(
                                std::make_shared<fep3::base::StreamTypePlain<uint32_t>>()))))
                .WillOnce(::testing::Return()) // initial stream type
                .WillOnce(::testing::Return()) // first transmission of type by sheila
                ;
            EXPECT_CALL(
                *mock_sparrow_receiver_scot.get(),
                call(
                    ::testing::Matcher<const data_read_ptr<const IDataSample>&>
                    // DataSampleType currently doesn't handle timestamp and counter correctly (see
                    // FEPSDK-2668) thus we only check the value -> DataSampleSmartPtr*Value*Matcher
                    // TODO change to use DataSampleSmartPtrMatcher once FEPSDK-2668 is resolved
                    (fep3::mock::DataSampleSmartPtrValueMatcher(
                        std::make_shared<fep3::base::DataSampleType<uint32_t>>(
                            data_sample_value_numbers[0])))))
                .WillOnce(::testing::Return());
            EXPECT_CALL(
                *mock_sparrow_receiver_scot.get(),
                call(
                    ::testing::Matcher<const data_read_ptr<const IDataSample>&>
                    // DataSampleType currently doesn't handle timestamp and counter correctly (see
                    // FEPSDK-2668) thus we only check the value -> DataSampleSmartPtr*Value*Matcher
                    // TODO change to use DataSampleSmartPtrMatcher once FEPSDK-2668 is resolved
                    (fep3::mock::DataSampleSmartPtrValueMatcher(
                        std::make_shared<fep3::base::DataSampleType<uint32_t>>(
                            data_sample_value_numbers[1])))))
                .WillOnce(::testing::Return());
            EXPECT_CALL(
                *mock_sparrow_receiver_scot.get(),
                call(
                    ::testing::Matcher<const data_read_ptr<const IDataSample>&>
                    // DataSampleType currently doesn't handle timestamp and counter correctly (see
                    // FEPSDK-2668) thus we only check the value -> DataSampleSmartPtr*Value*Matcher
                    // TODO change to use DataSampleSmartPtrMatcher once FEPSDK-2668 is resolved
                    (fep3::mock::DataSampleSmartPtrValueMatcher(
                        std::make_shared<fep3::base::DataSampleType<uint32_t>>(
                            data_sample_value_numbers[2])))))
                .WillOnce(::testing::Return());
            // change to stream type string
            EXPECT_CALL(*mock_sparrow_receiver_scot.get(),
                        call(::testing::Matcher<const data_read_ptr<const IStreamType>&>(
                            fep3::mock::StreamTypeSmartPtrMatcher(
                                std::make_shared<fep3::base::StreamTypeString>(42)))))
                .WillOnce(::testing::Return());
            EXPECT_CALL(
                *mock_sparrow_receiver_scot.get(),
                call(
                    ::testing::Matcher<const data_read_ptr<const IDataSample>&>
                    // DataSampleType currently doesn't handle timestamp and counter correctly (see
                    // FEPSDK-2668) thus we only check the value -> DataSampleSmartPtr*Value*Matcher
                    // TODO change to use DataSampleSmartPtrMatcher once FEPSDK-2668 is resolved
                    (fep3::mock::DataSampleSmartPtrValueMatcher(
                        std::make_shared<fep3::base::DataSampleType<std::string>>(
                            data_sample_value_strings[0])))))
                .WillOnce(::testing::Return());
            EXPECT_CALL(
                *mock_sparrow_receiver_scot.get(),
                call(
                    ::testing::Matcher<const data_read_ptr<const IDataSample>&>
                    // DataSampleType currently doesn't handle timestamp and counter correctly (see
                    // FEPSDK-2668) thus we only check the value -> DataSampleSmartPtr*Value*Matcher
                    // TODO change to use DataSampleSmartPtrMatcher once FEPSDK-2668 is resolved
                    (fep3::mock::DataSampleSmartPtrValueMatcher(
                        std::make_shared<fep3::base::DataSampleType<std::string>>(
                            data_sample_value_strings[1])))))
                .WillOnce(::testing::Return());
            EXPECT_CALL(
                *mock_sparrow_receiver_scot.get(),
                call(
                    ::testing::Matcher<const data_read_ptr<const IDataSample>&>
                    // DataSampleType currently doesn't handle timestamp and counter correctly (see
                    // FEPSDK-2668) thus we only check the value -> DataSampleSmartPtr*Value*Matcher
                    // TODO change to use DataSampleSmartPtrMatcher once FEPSDK-2668 is resolved
                    (fep3::mock::DataSampleSmartPtrValueMatcher(
                        std::make_shared<fep3::base::DataSampleType<std::string>>(
                            data_sample_value_strings[2])))))
                .WillOnce(Notify(&sparrow_receiver_scot_all_items_received));
        }
        this->startReception(dynamic_cast<ISimulationBus*>(sparrow_simulation_bus_scot.get()));

        /*----------------------------------------------------------------------------
         *  Sheila starts talking
         *----------------------------------------------------------------------------*/

        sparrow_submitter_sheila._writer->write(fep3::base::StreamTypePlain<uint32_t>());
        sparrow_submitter_sheila._writer->write(
            base::DataSampleType<uint32_t>(data_sample_value_numbers[0]));
        sparrow_submitter_sheila._writer->write(
            base::DataSampleType<uint32_t>(data_sample_value_numbers[1]));
        sparrow_submitter_sheila._writer->write(
            base::DataSampleType<uint32_t>(data_sample_value_numbers[2]));

        sparrow_submitter_sheila._writer->transmit();

        // std::this_thread::sleep_for(std::chrono::seconds(1000));

        /*----------------------------------------------------------------------------
         *  Simon arrives
         *----------------------------------------------------------------------------*/
        auto sparrow_simulation_bus_simon =
            this->createSimulationBusDep(sparrow_domain_id, "Simon");

        {
            auto sparrow_reader_simon =
                dynamic_cast<ISimulationBus*>(sparrow_simulation_bus_simon.get())
                    ->getReader(topic, fep3::base::StreamTypePlain<uint32_t>());
            const auto& mock_sparrow_receiver_simon =
                std::make_shared<::testing::StrictMock<fep3::mock::SimulationBus::DataReceiver>>();
            sparrow_reader_simon->reset(mock_sparrow_receiver_simon);
            test::helper::Notification sparrow_receiver_simon_all_items_received;
            { // setting of expectations for mock_sparrow_receiver_simon
                ::testing::InSequence sequence;
                EXPECT_CALL(*mock_sparrow_receiver_simon.get(),
                            call(::testing::Matcher<const data_read_ptr<const IStreamType>&>(
                                fep3::mock::StreamTypeSmartPtrMatcher(
                                    std::make_shared<fep3::base::StreamTypePlain<uint32_t>>()))))
                    .WillOnce(::testing::Return()) // initial stream type
                    .WillOnce(::testing::Return()) // first transmission of type by sheila
                    ;
                // change to stream type string
                EXPECT_CALL(*mock_sparrow_receiver_simon.get(),
                            call(::testing::Matcher<const data_read_ptr<const IStreamType>&>(
                                fep3::mock::StreamTypeSmartPtrMatcher(
                                    std::make_shared<fep3::base::StreamTypeString>(42)))))
                    .WillOnce(::testing::Return());
                EXPECT_CALL(
                    *mock_sparrow_receiver_simon.get(),
                    call(
                        ::testing::Matcher<const data_read_ptr<const IDataSample>&>
                        // DataSampleType currently doesn't handle timestamp and counter correctly
                        // (see FEPSDK-2668) thus we only check the value ->
                        // DataSampleSmartPtr*Value*Matcher
                        // TODO change to use DataSampleSmartPtrMatcher once FEPSDK-2668 is resolved
                        (fep3::mock::DataSampleSmartPtrValueMatcher(
                            std::make_shared<fep3::base::DataSampleType<std::string>>(
                                data_sample_value_strings[0])))))
                    .WillOnce(::testing::Return());
                EXPECT_CALL(
                    *mock_sparrow_receiver_simon.get(),
                    call(
                        ::testing::Matcher<const data_read_ptr<const IDataSample>&>
                        // DataSampleType currently doesn't handle timestamp and counter correctly
                        // (see FEPSDK-2668) thus we only check the value ->
                        // DataSampleSmartPtr*Value*Matcher
                        // TODO change to use DataSampleSmartPtrMatcher once FEPSDK-2668 is resolved
                        (fep3::mock::DataSampleSmartPtrValueMatcher(
                            std::make_shared<fep3::base::DataSampleType<std::string>>(
                                data_sample_value_strings[1])))))
                    .WillOnce(::testing::Return());
                EXPECT_CALL(
                    *mock_sparrow_receiver_simon.get(),
                    call(
                        ::testing::Matcher<const data_read_ptr<const IDataSample>&>
                        // DataSampleType currently doesn't handle timestamp and counter correctly
                        // (see FEPSDK-2668) thus we only check the value ->
                        // DataSampleSmartPtr*Value*Matcher
                        // TODO change to use DataSampleSmartPtrMatcher once FEPSDK-2668 is resolved
                        (fep3::mock::DataSampleSmartPtrValueMatcher(
                            std::make_shared<fep3::base::DataSampleType<std::string>>(
                                data_sample_value_strings[2])))))
                    .WillOnce(Notify(&sparrow_receiver_simon_all_items_received));
            }
            this->startReception(dynamic_cast<ISimulationBus*>(sparrow_simulation_bus_simon.get()));

            /*----------------------------------------------------------------------------
             *  Sheila continues talking
             *----------------------------------------------------------------------------*/

            sparrow_submitter_sheila._writer->write(fep3::base::StreamTypeString(42));

            // We are switching StreamType and we are using DDS_VOLATILE_DURABILITY_QOS in our
            // profiles So we need some time until all reader are connected
            std::this_thread::sleep_for(std::chrono::seconds(1));

            sparrow_submitter_sheila._writer->write(
                base::DataSampleType<std::string>(data_sample_value_strings[0]));
            sparrow_submitter_sheila._writer->write(
                base::DataSampleType<std::string>(data_sample_value_strings[1]));
            sparrow_submitter_sheila._writer->write(
                base::DataSampleType<std::string>(data_sample_value_strings[2]));
            sparrow_submitter_sheila._writer->transmit();

            /*----------------------------------------------------------------------------
             *  Scot listens
             *----------------------------------------------------------------------------*/
            EXPECT_TRUE(sparrow_receiver_scot_all_items_received.waitForNotificationWithTimeout(
                std::chrono::seconds(5)));

            /*----------------------------------------------------------------------------
             *  Simon listens
             *----------------------------------------------------------------------------*/
            EXPECT_TRUE(sparrow_receiver_simon_all_items_received.waitForNotificationWithTimeout(
                std::chrono::seconds(10)));

            this->stopReception(dynamic_cast<ISimulationBus*>(sparrow_simulation_bus_simon.get()));
        }
        this->TearDownComponent(*sparrow_simulation_bus_simon);
        this->stopReception(dynamic_cast<ISimulationBus*>(sparrow_simulation_bus_scot.get()));
    }

    this->TearDownComponent(*sparrow_simulation_bus_scot);
    this->TearDownComponent(*sparrow_simulation_bus_sheila);
}
