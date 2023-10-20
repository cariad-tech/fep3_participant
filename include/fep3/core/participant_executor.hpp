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

#include <fep3/core/participant_state_changer.h>

#include <future>

namespace fep3 {
namespace core {

/**
 * This template makes sure to execute an instance of a participant in a separate thread.
 * It will call the exec function asynchronously, so it is possible to continue testing.
 * Additionally, you can send state machine change requests.
 */
class ParticipantExecutor : public core::ParticipantStateChanger {
public:
    /**
     * @brief CTOR
     * Do not forget to call @c exec afterwards!
     *
     * @param participant the participant to execute
     */
    explicit ParticipantExecutor(fep3::core::Participant& participant)
        : _participant(participant), core::ParticipantStateChanger(participant)
    {
    }

    /**
     * @brief Deleted Copy CTOR
     */
    ParticipantExecutor(const ParticipantExecutor&) = delete;

    /**
     * @brief Deleted Move CTOR
     */
    ParticipantExecutor(ParticipantExecutor&&) = delete;

    /**
     * @brief Deleted Copy assignment operator
     *
     * @return ParticipantExecutor&
     */
    ParticipantExecutor& operator=(const ParticipantExecutor&) = delete;

    /**
     * @brief Deleted Move assignment operator
     *
     * @return ParticipantExecutor&
     */
    ParticipantExecutor& operator=(ParticipantExecutor&&) = delete;

    /**
     * @brief DTOR
     */
    ~ParticipantExecutor()
    {
        stop();
        deinitialize();
        unload();
        shutdown();
        if (_exec_wait_thread.joinable()) {
            _exec_wait_thread.join();
        }
    }

    /**
     * @brief executor function which will call the @c Participant::exec function in a separated
     * thread.
     * @throws std::runtime_error this function throws if something went wrong
     */
    void exec()
    {
        if (_exec_wait_thread.joinable()) {
            throw std::runtime_error("invalid state of executor");
        }

        std::promise<bool> exec_loop_started;
        auto started = exec_loop_started.get_future();
        _exec_wait_thread = std::thread([&] {
            auto ret = _participant.exec([&] { exec_loop_started.set_value(true); });
            if (ret != 0) {
                exec_loop_started.set_value(false);
            }
        });

        auto started_value = started.get();
        if (started_value) {
            // everything went fine
        }
        else {
            throw std::runtime_error("executor start error");
        }
    }

private:
    /**
     * @cond no_documentation
     */
    fep3::core::Participant& _participant;
    std::thread _exec_wait_thread;
    /**
     * @endcond no_documentation
     */
};

/// @cond nodoc
namespace arya {
using ParticipantExecutor [[deprecated("Since 3.1, fep3::core::arya::ParticipantExecutor is "
                                       "deprecated. Please use fep3::core::ParticipantExecutor")]] =
    fep3::core::ParticipantExecutor;
} // namespace arya
/// @endcond nodoc
} // namespace core
} // namespace fep3
