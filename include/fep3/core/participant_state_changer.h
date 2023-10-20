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

#include <fep3/core/participant.h>

namespace fep3 {
namespace base {
class Participant; // forward declaration
} // namespace base

namespace core {

/**
 * @brief helper class to change a participant's state from system side
 * Since the participant does not have another possibility to change the state than from service bus
 * this testing helper will add state machine change functionality for you.
 */
class ParticipantStateChanger {
public:
    /**
     * @brief CTOR
     *
     * @param[in] part participant to manage
     * @remark the participants service bus will be used to obtain the participant requester
     */
    ParticipantStateChanger(fep3::base::Participant& part);

    /**
     * @brief DTOR
     */
    virtual ~ParticipantStateChanger();

    /**
     * @brief Copy CTOR
     *
     * @param[in] other the other to copy from
     */
    ParticipantStateChanger(const ParticipantStateChanger& other);

    /**
     * @brief Copy operator
     *
     * @param[in] other the other to copy from
     * @return the copied ParticipantStateChanger
     */
    ParticipantStateChanger& operator=(const ParticipantStateChanger& other);

    /**
     * @brief Deleted Move CTOR
     */
    ParticipantStateChanger(ParticipantStateChanger&&) = delete;

    /**
     * @brief Deleted Move operator
     *
     * @return the moved ParticipantStateChanger
     */
    ParticipantStateChanger& operator=(ParticipantStateChanger&&) = delete;

    /**
     * @brief sends a load request
     *
     * @return fep3::Result ERR_NOERROR if succeeded, otherwise error code indicating occurred error
     */
    fep3::Result load();

    /**
     * @brief sends an unload request
     *
     * @return fep3::Result ERR_NOERROR if succeeded, otherwise error code indicating occurred error
     */
    fep3::Result unload();

    /**
     * @brief sends a initialize request
     *
     * @return fep3::Result ERR_NOERROR if succeeded, otherwise error code indicating occurred error
     */
    fep3::Result initialize();

    /**
     * @brief sends a deinitialize request
     *
     * @return fep3::Result ERR_NOERROR if succeeded, otherwise error code indicating occurred error
     */
    fep3::Result deinitialize();

    /**
     * @brief sends a start request
     *
     * @return fep3::Result ERR_NOERROR if succeeded, otherwise error code indicating occurred error
     */
    fep3::Result start();

    /**
     * @brief sends a pause request
     *
     * @return fep3::Result ERR_NOERROR if succeeded, otherwise error code indicating occurred error
     */
    fep3::Result pause();

    /**
     * @brief sends a stop request
     *
     * @return fep3::Result ERR_NOERROR if succeeded, otherwise error code indicating occurred error
     */
    fep3::Result stop();

    /**
     * @brief sends an exit request
     *
     * @return fep3::Result ERR_NOERROR if succeeded, otherwise error code indicating occurred error
     */
    fep3::Result shutdown();

private:
    /**
     * @cond no_documentation
     */
    class Impl;
    std::unique_ptr<Impl> _impl;
    /**
     * @endcond no_documentation
     */
};

/// @cond nodoc
namespace arya {
using ParticipantStateChanger
    [[deprecated("Since 3.1, fep3::arya::core::ParticipantStateChanger is deprecated. Please use "
                 "fep3::core::ParticipantStateChanger")]] = fep3::core::ParticipantStateChanger;
} // namespace arya
/// @endcond nodoc

} // namespace core
} // namespace fep3
