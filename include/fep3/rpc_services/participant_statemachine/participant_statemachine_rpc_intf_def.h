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

#ifndef FEP3_RPC_IID_STATEMACHINE
#define FEP3_RPC_IID_STATEMACHINE

// very important to have this relative! system library!
#include "./../base/fep_rpc_iid.h"

namespace fep3 {
namespace rpc {
namespace arya {

/**
 * @brief definition of the external service interface of the participant service
 * see also the delivered participant.json file
 */
class IRPCParticipantStateMachineDef {
protected:
    /// DTOR
    ~IRPCParticipantStateMachineDef() = default;

public:
    /// definiton of the FEP rpc service iid for the state machine
    FEP_RPC_IID("participant_statemachine.arya.rpc.fep3.iid", "participant_statemachine");
};

} // namespace arya

namespace catelyn {

/**
 * @brief definition of the external service interface of the participant service
 * see also the delivered participant.json file
 */
class IRPCParticipantStateMachineDef {
protected:
    /// DTOR
    ~IRPCParticipantStateMachineDef() = default;

public:
    /// definiton of the FEP rpc service iid for the state machine
    FEP_RPC_IID("participant_statemachine.catelyn.rpc.fep3.iid", "participant_statemachine");
};

} // namespace catelyn
using catelyn::IRPCParticipantStateMachineDef;

} // namespace rpc
} // namespace fep3

#endif // FEP3_RPC_IID_STATEMACHINE
