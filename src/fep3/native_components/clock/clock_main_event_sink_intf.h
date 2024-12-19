/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <fep3/components/clock/clock_intf.h>
#include <fep3/fep3_result_decl.h>

namespace fep3 {
namespace rpc {
class IClockMainEventSink : public fep3::experimental::IClock::IEventSink {
public:
    virtual fep3::Result registerClient(const std::string& client_name, int event_id_flag) = 0;
    virtual fep3::Result unregisterClient(const std::string& client_name) = 0;
    virtual fep3::Result receiveClientSyncedEvent(const std::string& client_name,
                                                  Timestamp time) = 0;
    virtual fep3::Result updateTimeout(std::chrono::nanoseconds rpc_timeout) = 0;

    virtual ~IClockMainEventSink() = default;
};

} // namespace rpc
} // namespace fep3
