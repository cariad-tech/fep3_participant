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

#include <chrono>
#include <memory>
#include <string>

namespace fep3 {
namespace native {

class ServiceDiscTopicWriter;
class DdsSdParticipant;
class ServiceDiscoveryDDS {
public:
    /**
     * @brief CTOR
     * @detail The ServiceDiscoveryDDS writes the dds discovery keyed topic.
     *
     * @param location_url  Address of the service as well-formed URL(ex. http://0.0.0.0:9090) where
     * the service is located and the flag indicating if this is the default url address
     * @param unique_service_name Unique name of that service
     */
    ServiceDiscoveryDDS(std::pair<std::string, bool> location_url,
                        std::string unique_service_name,
                        const DdsSdParticipant& dds_sd_participant);

    ~ServiceDiscoveryDDS();

    /**
     * @brief Will send a *NOTIFY* message to all DDS participants within the domain.
     *
     * @remark The *NOTIFY* messages will NOT contain "UPnP/1.1"
     *         as version within the *SERVER:* tag!
     * @retval true sent notify without errors
     * @retval false sent notify with errors, check with getLastSendErrors
     */
    bool sendNotifyAlive();

    /**
     * @brief Will send a *NOTIFY* message to all DDS participants within the domain.
     *
     * @retval true sent notify without errors
     * @retval false sent notify with errors, check with getLastSendErrors
     */
    bool sendNotifyByeBye();

    /**
     * @brief This is a no op.
     *
     * @retval true always
     */
    bool checkForMSearchAndSendResponse(std::chrono::milliseconds);
    std::string getLastSendErrors() const;

private:
    std::unique_ptr<ServiceDiscTopicWriter> _service_disc_topic_writer;
};

} // namespace native
} // namespace fep3
