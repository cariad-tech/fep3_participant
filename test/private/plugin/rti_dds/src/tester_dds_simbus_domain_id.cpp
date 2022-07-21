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

#include <helper/gmock_async_helper.h>
#include <helper/platform_dep_name.h>
#include <fep3/components/simulation_bus/mock/mock_simulation_bus.h>
#include <fep3/base/sample/mock/mock_data_sample.h>

#include "detail/test_read_write_test_class.hpp"
#include "detail/test_submitter.hpp"
#include "detail/test_receiver.hpp"

MATCHER_P(DataSampleSmartPtrValueMatcher, pointer_to_expected_value, "Matcher for value of smart pointer to IDataSample")
{
    return arg->getCounter() == *pointer_to_expected_value;
}

/**
 * @detail Test send and receive of samples with one listener on a different domain
 * @req_id FEPSDK-Sample
 */
TEST_F(TestConnextDDSSimulationBus, SendAndReceiveSamplesMultipleDomains)
{
    std::string topic = makePlatformDepName("breadcrumb");

    uint32_t sparrow_domain_id = randomDomainId();
    uint32_t sparrow_data_sample_count = 5;

    uint32_t blackbird_domain_id = randomDomainId();

    /*----------------------------------------------------------------------------
     *  Make sure they have a different domain_id
     *----------------------------------------------------------------------------*/
    while (blackbird_domain_id == sparrow_domain_id)
    {
        blackbird_domain_id = randomDomainId();
    }

    /*----------------------------------------------------------------------------
     *  create the simulation_buses for the birds
     *----------------------------------------------------------------------------*/

    auto sparrow_simulation_bus = createSimulationBusDep(sparrow_domain_id, "Sheila");
    auto sparrow_simulation_bus2 = createSimulationBusDep(sparrow_domain_id, "Scot");
    auto blackbird_simulation_bus = createSimulationBusDep(blackbird_domain_id, "Brad");

    {
        /*----------------------------------------------------------------------------
         *  add the birds
         *----------------------------------------------------------------------------*/
        TestSubmitter sparrow_submitter(dynamic_cast<ISimulationBus*>(sparrow_simulation_bus.get()),
                                        topic,
                                        fep3::base::StreamTypePlain<uint32_t>());

        auto sparrow_reader = dynamic_cast<ISimulationBus*>(sparrow_simulation_bus2.get())
            ->getReader(topic, fep3::base::StreamTypePlain<uint32_t>());
        const auto& mock_sparrow_receiver = std::make_shared<::testing::StrictMock<fep3::mock::DataReceiver>>();
        sparrow_reader->reset(mock_sparrow_receiver);
        test::helper::Notification sparrow_all_items_received;
        { // setting of expectations for mock_sparrow_receiver
            ::testing::InSequence sequence;
            EXPECT_CALL(*mock_sparrow_receiver.get(), call(::testing::Matcher<const data_read_ptr<const IStreamType>&>
                (::testing::_))).WillOnce(::testing::Return());
            EXPECT_CALL(*mock_sparrow_receiver.get(), call(::testing::Matcher<const data_read_ptr<const IDataSample>&>(::testing::_)))
                .Times(sparrow_data_sample_count - 1)
                .WillRepeatedly(::testing::Return())
                ;
            EXPECT_CALL(*mock_sparrow_receiver.get(), call(::testing::Matcher<const data_read_ptr<const IDataSample>&>(::testing::_)))
                .WillOnce(Notify(&sparrow_all_items_received))
                ;
        }
        startReception(dynamic_cast<ISimulationBus*>(sparrow_simulation_bus2.get()));

        auto blackbird_reader = dynamic_cast<ISimulationBus*>(blackbird_simulation_bus.get())
            ->getReader(topic, fep3::base::StreamTypePlain<uint32_t>());
        const auto& mock_blackbird_receiver = std::make_shared<::testing::StrictMock<fep3::mock::DataReceiver>>();
        blackbird_reader->reset(mock_blackbird_receiver);
        { // setting of expectations for mock_blackbird_receiver
            // expect to receive nothing because blackbird is in a different domain
            EXPECT_CALL(*mock_blackbird_receiver.get(), call(::testing::Matcher<const data_read_ptr<const IStreamType>&>
                (::testing::_))).Times(0);
            EXPECT_CALL(*mock_blackbird_receiver.get(), call(::testing::Matcher<const data_read_ptr<const IDataSample>&>
                (::testing::_))).Times(0);
        }
        startReception(dynamic_cast<ISimulationBus*>(blackbird_simulation_bus.get()));

        /*----------------------------------------------------------------------------
         *  add sparrow data samples
         *----------------------------------------------------------------------------*/
        for (unsigned int value = 0; value < sparrow_data_sample_count; ++value)
        {
            sparrow_submitter.addDataSample(fep3::base::DataSampleType<uint32_t>(value));
        }
        sparrow_submitter.submitDataSamples();

        /*----------------------------------------------------------------------------
         *  listen for sparrow samples and make sure blackbird received nothing
         *----------------------------------------------------------------------------*/
        EXPECT_TRUE(sparrow_all_items_received.waitForNotificationWithTimeout(std::chrono::seconds(5)));

        stopReception(dynamic_cast<ISimulationBus*>(blackbird_simulation_bus.get()));
        stopReception(dynamic_cast<ISimulationBus*>(sparrow_simulation_bus2.get()));
        stopReception(dynamic_cast<ISimulationBus*>(sparrow_simulation_bus.get()));
    }

    TearDownComponent(*blackbird_simulation_bus);
    TearDownComponent(*sparrow_simulation_bus2);
    TearDownComponent(*sparrow_simulation_bus);
}

TEST_F(TestConnextDDSSimulationBus, SendAndReceiveSamplesMultipleSystemNames)
{
    std::string topic = makePlatformDepName("breadcrumb");

    uint32_t sparrow_data_sample_count = 5;
    uint32_t domain_id = randomDomainId();

    /*----------------------------------------------------------------------------
     *  create the simulation_buses for the birds
     *----------------------------------------------------------------------------*/
    auto sparrow_simulation_bus = createSimulationBusDep(domain_id, "Sheila", "sparrow");
    auto sparrow_simulation_bus2 = createSimulationBusDep(domain_id, "Scot", "sparrow");
    auto blackbird_simulation_bus = createSimulationBusDep(domain_id, "Brad", "blackbird");
    {
        /*----------------------------------------------------------------------------
         *  add the birds
         *----------------------------------------------------------------------------*/
        TestSubmitter sparrow_submitter(dynamic_cast<ISimulationBus*>(sparrow_simulation_bus.get()),
                                        topic,
                                        fep3::base::StreamTypePlain<uint32_t>());

        auto sparrow_reader = dynamic_cast<ISimulationBus*>(sparrow_simulation_bus.get())
                                  ->getReader(topic, fep3::base::StreamTypePlain<uint32_t>());
        const auto& mock_sparrow_receiver = std::make_shared<::testing::StrictMock<fep3::mock::DataReceiver>>();
        sparrow_reader->reset(mock_sparrow_receiver);
        test::helper::Notification sparrow_all_items_received;
        { // setting of expectations for mock_sparrow_receiver
            ::testing::InSequence sequence;
            EXPECT_CALL(*mock_sparrow_receiver.get(), call(::testing::Matcher<const data_read_ptr<const IStreamType>&>
                (::testing::_))).WillOnce(::testing::Return());
            EXPECT_CALL(*mock_sparrow_receiver.get(), call(::testing::Matcher<const data_read_ptr<const IDataSample>&>(::testing::_)))
                .Times(sparrow_data_sample_count - 1)
                .WillRepeatedly(::testing::Return())
                ;
            EXPECT_CALL(*mock_sparrow_receiver.get(), call(::testing::Matcher<const data_read_ptr<const IDataSample>&>(::testing::_)))
                .WillOnce(Notify(&sparrow_all_items_received))
                ;
        }
        startReception(dynamic_cast<ISimulationBus*>(sparrow_simulation_bus.get()));

        auto blackbird_reader = dynamic_cast<ISimulationBus*>(blackbird_simulation_bus.get())
                                    ->getReader(topic, fep3::base::StreamTypePlain<uint32_t>());
        const auto& mock_blackbird_receiver = std::make_shared<::testing::StrictMock<fep3::mock::DataReceiver>>();
        blackbird_reader->reset(mock_blackbird_receiver);
        test::helper::Notification blackbird_all_items_received;
        // note: blackbird reader is expected to receive nothing because it's part of a different system
        startReception(dynamic_cast<ISimulationBus*>(blackbird_simulation_bus.get()));

        /*----------------------------------------------------------------------------
         *  add sparrow data samples
         *----------------------------------------------------------------------------*/
        for (unsigned int value = 0; value < sparrow_data_sample_count; ++value)
        {
            sparrow_submitter.addDataSample(fep3::base::DataSampleType<uint32_t>(value));
        }
        sparrow_submitter.submitDataSamples();

        /*----------------------------------------------------------------------------
         *  listen for sparrow samples and make sure blackbird recieved nothing
         *----------------------------------------------------------------------------*/
        EXPECT_TRUE(sparrow_all_items_received.waitForNotificationWithTimeout(std::chrono::seconds(5)));

        stopReception(dynamic_cast<ISimulationBus*>(blackbird_simulation_bus.get()));
        stopReception(dynamic_cast<ISimulationBus*>(sparrow_simulation_bus2.get()));
        stopReception(dynamic_cast<ISimulationBus*>(sparrow_simulation_bus.get()));
    }

    TearDownComponent(*blackbird_simulation_bus);
    TearDownComponent(*sparrow_simulation_bus2);
    TearDownComponent(*sparrow_simulation_bus);
}

