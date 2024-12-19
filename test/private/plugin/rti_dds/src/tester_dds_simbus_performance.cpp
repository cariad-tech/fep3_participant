/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "detail/test_read_write_test_class.hpp"

#include <fep3/base/sample/data_sample.h>
#include <fep3/base/sample/mock/mock_data_sample.h>
#include <fep3/components/simulation_bus/mock_simulation_bus.h>

#include <helper/gmock_async_helper.h>

TEST_F(ReaderWriterTestClass, TestParticipantDetection)
{
    test::helper::Notification all_items_received;
    const auto& mock_receiver =
        std::make_shared<::testing::StrictMock<fep3::mock::SimulationBus::DataReceiver>>();

    uint32_t test_sample_value = 6;
    const data_read_ptr<const IDataSample> test_sample =
        std::make_shared<fep3::base::DataSampleType<uint32_t>>(test_sample_value);

    { // setting of expectations
        EXPECT_CALL(
            *mock_receiver.get(),
            call(::testing::Matcher<const data_read_ptr<const IDataSample>&>
                 // DataSampleType currently doesn't handle timestamp and counter correctly (see
                 // FEPSDK-2668) thus we only check the value -> DataSampleSmartPtr*Value*Matcher
                 // TODO change to use DataSampleSmartPtrMatcher once FEPSDK-2668 is resolved
                 (fep3::mock::DataSampleSmartPtrValueMatcher(test_sample))))
            .WillOnce(Notify(&all_items_received));
        // ignore stream types
        EXPECT_CALL(*mock_receiver.get(),
                    call(::testing::Matcher<const data_read_ptr<const IStreamType>&>(::testing::_)))
            .WillRepeatedly(::testing::Return());
    }

    _reader->reset(mock_receiver);

    startReception(getSimulationBus());
    _writer->write(*test_sample.get());
    _writer->transmit();
    EXPECT_TRUE(all_items_received.waitForNotificationWithTimeout(std::chrono::seconds(5)));
    stopReception(getSimulationBus());
}
