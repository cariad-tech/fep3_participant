/**
 * @file
 * @copyright
 * @verbatim
Copyright 2023 CARIAD SE.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
@endverbatim
 */

#pragma once

#include <fep3/components/base/component_iid.h>
#include <fep3/fep3_errors.h>

namespace fep3 {
namespace arya {
/**
 * @brief Participant component interface definition
 */
class IParticipantInfo {
protected:
    /**
     * @brief DTOR
     * @note This DTOR is explicitly protected to prevent destruction via this interface.
     */
    ~IParticipantInfo() = default;

public:
    /**
     * @brief definition of the component interface identifier for the IParticipantInfo
     * @see IComponents
     */
    FEP_COMPONENT_IID("participant_info.arya.fep3.iid");

public:
    /**
     * @brief get the current participant name
     * @return name of the participant
     * @retval the current set participant name
     */
    virtual std::string getName() const = 0;

    /**
     * @brief the system name which the participant participates
     * @return name of the system
     * @retval the participate system name
     */
    virtual std::string getSystemName() const = 0;
};
} // namespace arya

using arya::IParticipantInfo;
} // namespace fep3
