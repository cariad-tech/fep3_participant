/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <fep3/components/service_bus/rpc/fep_rpc_stubs_client.h>
#include <fep3/components/service_bus/service_bus_intf.h>
#include <fep3/core/participant_state_changer.h>
#include <fep3/rpc_services/base/fep_rpc_json_to_result.h>
#include <fep3/rpc_services/participant_statemachine/participant_statemachine_client_stub.h>
#include <fep3/rpc_services/participant_statemachine/participant_statemachine_rpc_intf_def.h>

namespace fep3 {
namespace core {
class StateMachineClient : public fep3::rpc::RPCServiceClient<
                               fep3::rpc_stubs::catelyn::RPCParticipantStateMachineClientStub,
                               fep3::rpc::catelyn::IRPCParticipantStateMachineDef> {
    typedef fep3::rpc::RPCServiceClient<
        fep3::rpc_stubs::catelyn::RPCParticipantStateMachineClientStub,
        fep3::rpc::catelyn::IRPCParticipantStateMachineDef>
        super;

public:
    StateMachineClient(const std::string& service_name,
                       const std::shared_ptr<fep3::arya::IRPCRequester>& rpc_requester)
        : super(service_name, rpc_requester)
    {
    }
};

class ParticipantStateChanger::Impl {
public:
    Impl(fep3::base::Participant& part)
        : _part(part),
          _sm_client(fep3::rpc::catelyn::IRPCParticipantStateMachineDef::DEFAULT_NAME,
                     part.getComponent<IServiceBus>()->getRequester(part.getName()))
    {
    }
    virtual ~Impl()
    {
    }

    fep3::base::Participant& _part;
    StateMachineClient _sm_client;
};

ParticipantStateChanger::ParticipantStateChanger(fep3::base::Participant& part)
    : _impl(std::make_unique<Impl>(part))
{
}

ParticipantStateChanger::ParticipantStateChanger(const ParticipantStateChanger& other)
    : _impl(std::make_unique<Impl>(*other._impl))
{
}

ParticipantStateChanger& ParticipantStateChanger::operator=(const ParticipantStateChanger& other)
{
    if (this != &other) {
        _impl.reset(new Impl(*other._impl));
    }
    return *this;
}

ParticipantStateChanger::~ParticipantStateChanger() = default;

Result ParticipantStateChanger::load()
{
    try {
        return fep3::rpc::arya::jsonToResult(_impl->_sm_client.load());
    }
    catch (...) {
        RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED, "Unknown C++ Exception occured");
    }
}

Result ParticipantStateChanger::unload()
{
    try {
        return fep3::rpc::arya::jsonToResult(_impl->_sm_client.unload());
    }
    catch (...) {
        RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED, "Unknown C++ Exception occured");
    }
}

Result ParticipantStateChanger::initialize()
{
    try {
        return fep3::rpc::arya::jsonToResult(_impl->_sm_client.initialize());
    }
    catch (...) {
        RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED, "Unknown C++ Exception occured");
    }
}

Result ParticipantStateChanger::deinitialize()
{
    try {
        return fep3::rpc::arya::jsonToResult(_impl->_sm_client.deinitialize());
    }
    catch (...) {
        RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED, "Unknown C++ Exception occured");
    }
}

Result ParticipantStateChanger::start()
{
    try {
        return fep3::rpc::arya::jsonToResult(_impl->_sm_client.start());
    }
    catch (...) {
        RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED, "Unknown C++ Exception occured");
    }
}

Result ParticipantStateChanger::pause()
{
    try {
        return fep3::rpc::arya::jsonToResult(_impl->_sm_client.pause());
    }
    catch (...) {
        RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED, "Unknown C++ Exception occured");
    }
}

Result ParticipantStateChanger::stop()
{
    try {
        return fep3::rpc::arya::jsonToResult(_impl->_sm_client.stop());
    }
    catch (...) {
        RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED, "Unknown C++ Exception occured");
    }
}

Result ParticipantStateChanger::shutdown()
{
    try {
        return fep3::rpc::arya::jsonToResult(_impl->_sm_client.exit());
    }
    catch (...) {
        RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED, "Unknown C++ Exception occured");
    }
}

} // namespace core
} // namespace fep3
