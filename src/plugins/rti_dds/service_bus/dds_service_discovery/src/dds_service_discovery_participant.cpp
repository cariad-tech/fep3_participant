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

#include "../include/dds_service_discovery_participant.h"

#include <a_util/process/process.h>

#include <mutex>

namespace {
const constexpr char* fep3_domain_id_env_variable = "FEP3_SERVICE_BUS_DOMAIN_ID";
const int32_t DEFAULT_SERVICE_DISCOVERY_DDS_DOMAIN = 0;
int32_t getDomainId()
{
    std::string env_variable = a_util::process::getEnvVar(fep3_domain_id_env_variable, "");
    if (env_variable.empty()) {
        return DEFAULT_SERVICE_DISCOVERY_DDS_DOMAIN;
    }
    else {
        try {
            int32_t domain_id = std::stoi(env_variable);
            return domain_id;
        }
        catch (std::exception&) {
            using namespace std::literals::string_literals;
            throw std::runtime_error("Error while converting environment variable "s +
                                     fep3_domain_id_env_variable + " = " + env_variable +
                                     " to integer");
        }
    }
}
} // namespace

namespace fep3 {
namespace native {

DdsSdParticipant::DdsSdParticipant() : _dds_participant(DdsSdParticipant::createParticipant())
{
}

const dds::all::DomainParticipant& DdsSdParticipant::getDomainParticipant() const
{
    return _dds_participant;
}

dds::all::DomainParticipant DdsSdParticipant::createParticipant()
{
    // It is not safe to create a dds::domain::DomainParticipant while another thread may be
    // simultaneously creating another dds::domain::DomainParticipant.
    auto participant_qos = dds::core::QosProvider::Default().participant_qos();
    auto& d = participant_qos.policy<rti::core::policy::Database>();
    d.shutdown_cleanup_period(dds::core::Duration(0));
    return dds::all::DomainParticipant(getDomainId(), participant_qos);
}

} // namespace native
} // namespace fep3
