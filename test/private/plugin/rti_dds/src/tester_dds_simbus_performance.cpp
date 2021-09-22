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

#include <helper/gmock_async_helper.h>
#include <fep3/base/stream_type/default_stream_type.h>
#include <fep3/base/sample/data_sample.h>
#include <fep3/base/sample/raw_memory.h>
#include <fep3/components/simulation_bus/mock/mock_simulation_bus.h>
#include <fep3/base/sample/mock/mock_data_sample.h>
#include <fep3/base/stream_type/mock/mock_stream_type.h>

#include "detail/test_receiver.hpp"
#include "detail/test_samples.hpp"
#include "detail/test_read_write_test_class.hpp"

TEST_F(ReaderWriterTestClass, TestParticipantDetection)
{
    test::helper::Notification all_items_received;
    const auto& mock_receiver = std::make_shared<::testing::StrictMock<fep3::mock::DataReceiver>>();

    uint32_t test_sample_value = 6;
    const data_read_ptr<const IDataSample> test_sample = std::make_shared<fep3::base::DataSampleType<uint32_t>>(test_sample_value);

    { // setting of expectations
        EXPECT_CALL(*mock_receiver.get(), call(::testing::Matcher<const data_read_ptr<const IDataSample>&>
            // DataSampleType currently doesn't handle timestamp and counter correctly (see FEPSDK-2668)
            // thus we only check the value -> DataSampleSmartPtr*Value*Matcher
            // TODO change to use DataSampleSmartPtrMatcher once FEPSDK-2668 is resolved
            (fep3::mock::DataSampleSmartPtrValueMatcher(test_sample)))).WillOnce(Notify(&all_items_received));
        // ignore stream types
        EXPECT_CALL(*mock_receiver.get(), call(::testing::Matcher<const data_read_ptr<const IStreamType>&>
            (::testing::_))).WillRepeatedly(::testing::Return());
    }

    _reader->reset(mock_receiver);

    startReception(getSimulationBus());
    _writer->write(*test_sample.get());
    _writer->transmit();
    EXPECT_TRUE(all_items_received.waitForNotificationWithTimeout(std::chrono::seconds(5)));
    stopReception(getSimulationBus());
}
