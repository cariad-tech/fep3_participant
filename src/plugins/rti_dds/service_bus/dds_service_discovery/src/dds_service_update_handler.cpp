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

#include "../include/dds_service_update_handler.h"

#include "host_name_resolver.h"

#include <boost/algorithm/string.hpp>

#include <cxx_url.h>

namespace {
const constexpr size_t initial_stack_size = 10;
} // namespace

namespace fep3 {
namespace native {

DssServiceUpdateHandler::DssServiceUpdateHandler(
    std::unique_ptr<IHostNameResolver> host_name_resolver, std::shared_ptr<ILogger> logger)
    : _thread_executor{std::make_unique<ThreadPoolExecutor>()},
      _host_name_resolver(std::move(host_name_resolver)),
      _processed_event_stack(initial_stack_size),
      _logger(std::move(logger))
{
    _thread_executor->start();
}

DssServiceUpdateHandler::~DssServiceUpdateHandler()
{
    _working = false;
    _thread_executor.reset();
}

void DssServiceUpdateHandler::addWork(const DdsServiceDiscovery& participant_data)
{
    if (_working) {
        using namespace std::literals::string_literals;
        _logger->logDebug("received discovery sample from server "s + participant_data.id() +
                          " , with url " + participant_data.content().host_url() +
                          " ,response type " +
                          std::to_string(static_cast<int>(participant_data.response_type())));
        _thread_executor->post([=]() { processServiceUpdate(participant_data); });
    }
}

std::vector<ServiceUpdateEvent> DssServiceUpdateHandler::getProcessedUpdates()
{
    std::vector<ServiceUpdateEvent> processed_events;
    _processed_event_stack.consume_all(
        [&](const ServiceUpdateEvent& e) { processed_events.push_back(e); });
    return processed_events;
}

void DssServiceUpdateHandler::processServiceUpdate(DdsServiceDiscovery participant_data)
{
    fep3::helper::Url url(participant_data.content().host_url());
    std::string resolved_ip;
    using namespace std::string_literals;

    try {
        resolved_ip = _host_name_resolver->findIp(url.host(), url.port());
    }
    catch (std::exception& e) {
        _logger->logWarning("exception during resolving host  "s +
                            participant_data.content().host_url() + ", error:" + e.what());
    }

    if (resolved_ip.empty()) {
        _logger->logWarning("Unresolved host  "s + participant_data.content().host_url());
    }
    else {
        ServiceUpdateEvent up_event;
        switch (participant_data.response_type()) {
        case ServiceDiscoveryResponseType::ResponseAlive: {
            up_event._event_id = fep3::IServiceBus::ServiceUpdateEventType::notify_alive;
            break;
        }
        case ServiceDiscoveryResponseType::ResponseBye: {
            up_event._event_id = fep3::IServiceBus::ServiceUpdateEventType::notify_byebye;
            break;
        }
        case ServiceDiscoveryResponseType::ResponseDiscover: {
            up_event._event_id = fep3::IServiceBus::ServiceUpdateEventType::response;
            break;
        }
        }
        up_event._service_name = participant_data.content().service_name();
        up_event._host_url = participant_data.content().host_url();
        boost::ireplace_first(up_event._host_url, url.host(), resolved_ip);
        _processed_event_stack.push(up_event);
        _logger->logDebug(a_util::strings::format(
            "Processed update event from server: %s, with url: %s,service name: %s, ,response "
            "type: %s, resolved ip: %s",
            participant_data.id().c_str(),
            participant_data.content().host_url().c_str(),
            participant_data.content().service_name().c_str(),
            std::to_string(static_cast<int>(participant_data.response_type())).c_str(),
            resolved_ip.c_str()));
    }
}

} // namespace native
} // namespace fep3
