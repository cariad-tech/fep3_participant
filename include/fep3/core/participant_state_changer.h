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

#include <string>
#include <memory>

#include <fep3/fep3_participant_types.h>
#include <fep3/fep3_errors.h>
#include <fep3/core/participant.h>

namespace fep3
{
namespace core
{
namespace arya
{

/**
* @brief helper class to change a participants state from system side
* Since the participant does not have another possibility to change the state than from service bus
* this testing helper will add state machine change functionality for you.
*/
class ParticipantStateChanger
{
public:
    /**
     * @brief CTOR
     *
     * @param[in] part participant to manage
     * @remark the participants servicebus will be used to obtain the participant requester
     */
    ParticipantStateChanger(fep3::arya::Participant& part);
    /**
     * @brief DTOR
     *
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
     * @brief move CTOR
     *
     * @param[in] other the other to move from
     */
    ParticipantStateChanger(ParticipantStateChanger&& other) = default;
    /**
     * @brief move operator
     *
     * @param[in] other the other to move from
     * @return the moved ParticipantStateChanger
     */
    ParticipantStateChanger& operator=(ParticipantStateChanger&& other) = default;

    /**
     * @brief sends a load request
     *
     * @return @c true if the request reached participant, @c false otherwise
     */
    bool load();
    /**
     * @brief sends a unload request
     *
     * @return @c true if the request reached participant, @c false otherwise
     */
    bool unload();
    /**
     * @brief sends a initialize request
     *
     * @return @c true if the request reached participant, @c false otherwise
     */
    bool initialize();
    /**
     * @brief sends a deinitialize request
     *
     * @return @c true if the request reached participant, @c false otherwise
     */
    bool deinitialize();
    /**
     * @brief sends a start request
     *
     * @return @c true if the request reached participant, @c false otherwise
     */
    bool start();
    /**
     * @brief sends a pause request
     *
     * @return @c true if the request reached participant, @c false otherwise
     */
    bool pause();
    /**
     * @brief sends a stop request
     *
     * @return @c true if the request reached participant, @c false otherwise
     */
    bool stop();
    /**
     * @brief sends a exit request
     *
     * @return @c true if the request reached participant, @c false otherwise
     */
    bool shutdown();

private:
    /**
     * @cond no_documentation
     *
     */
    class Impl;
    std::unique_ptr<Impl> _impl;
    /**
     * @endcond no_documentation
     *
     */
};

} // namespace arya
} // namespace core
} // namespace fep3
