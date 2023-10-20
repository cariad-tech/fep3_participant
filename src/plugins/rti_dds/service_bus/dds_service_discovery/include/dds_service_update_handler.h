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

#include "../../../../../fep3/native_components/service_bus/rpc/service_discovery/service_discovery_intf/logger_proxy.h"
#include "../../../../../fep3/native_components/service_bus/rpc/service_discovery/service_discovery_intf/service_discovery_intf.h"
#include "DdsServiceDiscoveryTopic.hpp"
#include "threaded_executor.h"

#include <boost/asio/io_service.hpp>
#include <boost/lockfree/stack.hpp>

namespace fep3 {
namespace native {

class IHostNameResolver;

class DssServiceUpdateHandler {
public:
    using WorkFunction = std::function<void(DdsServiceDiscovery)>;
    DssServiceUpdateHandler(std::unique_ptr<IHostNameResolver> host_name_resolver,
                            std::shared_ptr<ILogger> logger);
    ~DssServiceUpdateHandler();

    void addWork(const DdsServiceDiscovery&);
    std::vector<ServiceUpdateEvent> getProcessedUpdates();

    DssServiceUpdateHandler(DssServiceUpdateHandler&&) = delete;
    DssServiceUpdateHandler& operator=(DssServiceUpdateHandler&&) = delete;
    DssServiceUpdateHandler& operator=(const DssServiceUpdateHandler&) = delete;

private:
    void processServiceUpdate(DdsServiceDiscovery);

    std::unique_ptr<ThreadPoolExecutor> _thread_executor;
    std::unique_ptr<IHostNameResolver> _host_name_resolver;
    boost::lockfree::stack<ServiceUpdateEvent> _processed_event_stack;
    std::shared_ptr<ILogger> _logger;
    std::atomic<bool> _working{true};
};

} // namespace native
} // namespace fep3
