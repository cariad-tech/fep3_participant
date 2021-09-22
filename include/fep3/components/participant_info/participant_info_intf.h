/**
 * @file
 * @copyright
 * @verbatim
Copyright @ 2021 VW Group. All rights reserved.

    This Source Code Form is subject to the terms of the Mozilla
    Public License, v. 2.0. If a copy of the MPL was not distributed
    with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

If it is not possible or desirable to put the notice in a particular file, then
You may include the notice in a location (such as a LICENSE file in a
relevant directory) where a recipient would be likely to look for such a notice.

You may add additional accurate notices of copyright ownership.

@endverbatim
 */


#pragma once

#include <fep3/fep3_errors.h>
#include <fep3/fep3_participant_export.h>
#include <fep3/components/base/component_iid.h>

namespace fep3
{
namespace arya
{
    /**
     * @brief Participant component interface definition
     *
     */
    class IParticipantInfo
    {
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
         * @return the participant name
         * @retval the current set participant name
         */
        virtual std::string getName() const = 0;

        /**
         * @brief the system name which the participant participate
         * @return the system name
         * @retval the participate system name
         */
        virtual std::string getSystemName() const = 0;

    };
}

using arya::IParticipantInfo;
}


