/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "rpc_clock_sync_service.h"

#include "clock_main_event_sink_intf.h"
#include "clock_service.h"

#include <fep3/components/clock/clock_service_intf.h>

namespace fep3 {
namespace rpc {

using namespace a_util::strings;

RPCClockSyncService::RPCClockSyncService(IClockMainEventSink& clock_main_event_sink,
                                         fep3::arya::IClockService& service)
    : _clock_main_event_sink(clock_main_event_sink), _service(service)
{
}

int RPCClockSyncService::registerSyncSlave(int event_id_flag, const std::string& slave_name)
{
    const auto result = _clock_main_event_sink.registerClient(slave_name, event_id_flag);
    if (result) {
        FEP3_LOG_DEBUG(format("Successfully registered timing slave '%s'.", slave_name.c_str()));

        return 0;
    }

    FEP3_LOG_ERROR(format("Failure during registration of timing slave '%s'.", slave_name.c_str()));
    FEP3_LOG_RESULT(result);

    return -1;
}

int RPCClockSyncService::unregisterSyncSlave(const std::string& slave_name)
{
    const auto result = _clock_main_event_sink.unregisterClient(slave_name);
    if (result) {
        FEP3_LOG_DEBUG(format("Successfully unregistered timing slave '%s'.", slave_name.c_str()));

        return 0;
    }

    FEP3_LOG_ERROR(
        format("Failure during deregistration of timing slave '%s'.", slave_name.c_str()));
    FEP3_LOG_RESULT(result);

    return -1;
}

int RPCClockSyncService::slaveSyncedEvent(const std::string& new_time,
                                          const std::string& slave_name)
{
    using a_util::strings::toInt64;
    const auto result =
        _clock_main_event_sink.receiveClientSyncedEvent(slave_name, Timestamp{toInt64(new_time)});

    if (result) {
        FEP3_LOG_DEBUG(format("Successfully receveived synced event of timing slave '%s'.",
                              slave_name.c_str()));

        return 0;
    }

    FEP3_LOG_ERROR(
        format("Failure during receiving synced event of timing slave '%s'.", slave_name.c_str()));
    FEP3_LOG_RESULT(result);

    return -1;
}

std::string RPCClockSyncService::getMasterTime()
{
    auto current_time = toString(_service.getTime().count());

    FEP3_LOG_DEBUG(a_util::strings::format("Retrieved master time request. Responding '%s'.",
                                           current_time.c_str()));

    return current_time;
}

int RPCClockSyncService::getMasterType()
{
    const auto main_clock_type = static_cast<int>(_service.getType());

    FEP3_LOG_DEBUG(format("Retrieved master clock type request. Responding '%d' (%s).",
                          main_clock_type,
                          0 == main_clock_type ? "continuous" : "discrete"));

    return main_clock_type;
}

} // namespace rpc
} // namespace fep3
