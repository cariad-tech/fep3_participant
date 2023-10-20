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

#pragma once

#include <fep3/core/job.h>

#include <gmock/gmock.h>

namespace fep3 {
namespace mock {
namespace core {

struct Job : public fep3::core::Job {
    using fep3::core::Job::Job;

    MOCK_METHOD1(executeDataIn, fep3::Result(Timestamp));
    MOCK_METHOD1(execute, fep3::Result(Timestamp));
    MOCK_METHOD1(executeDataOut, fep3::Result(Timestamp));

    void setDefaultBehaviour()
    {
        using namespace ::testing;

        ON_CALL(*this, executeDataIn(_)).WillByDefault(Return(Result{}));
        ON_CALL(*this, execute(_)).WillByDefault(Return(Result{}));
        ON_CALL(*this, executeDataOut(_)).WillByDefault(Return(Result{}));
    }
};

} // namespace core
} // namespace mock
} // namespace fep3
