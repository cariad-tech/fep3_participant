/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <fep3/base/component_registry/component_registry.h>
#include <fep3/components/logging/logging_service_intf.h>
#include <fep3/participant/element_manager/element_manager.h>

#include <mutex>

namespace fep3 {
namespace native {

/**
 * @brief Class implementing the participant state machine
 */
class ParticipantStateMachine {
public:
    /**
     * CTOR
     */
    ParticipantStateMachine(ElementManager element_manager,
                            const std::shared_ptr<ComponentRegistry>& component_registry,
                            const std::shared_ptr<ILogger>& participant_logger);
    /**
     * DTOR
     */
    virtual ~ParticipantStateMachine();

    /**
     * Gets the finalized state of the participant state machine
     *
     * @return @c true if the participant state machine has finalized (i. e. is in its final state),
     * @c false otherwise
     */
    virtual bool isFinalized() const final;

    /**
     * Gets the name of the state the participant current is in
     */
    virtual std::string getCurrentStateName() const final;

    /**
     * Triggers the event "exit"
     *
     * @return fep3::Result
     */
    virtual fep3::Result exit() final;

    /**
     * Triggers the event "load"
     *
     * @return fep3::Result
     */
    virtual fep3::Result load() final;

    /**
     * Triggers the event "unload"
     *
     * @return fep3::Result
     */
    virtual fep3::Result unload() final;

    /**
     * Triggers the event "initialize"
     *
     * @return fep3::Result
     */
    virtual fep3::Result initialize() final;

    /**
     * Triggers the event "deinitialize"
     *
     * @return fep3::Result
     */
    virtual fep3::Result deinitialize() final;

    /**
     * Triggers the event "reinitialize"
     *
     * @return fep3::Result
     */
    virtual fep3::Result stop() final;

    /**
     * Triggers the event "run"
     *
     * @return fep3::Result
     */
    virtual fep3::Result start() final;

    /**
     * Triggers the event "hold"
     *
     * @return fep3::Result
     */
    virtual fep3::Result pause() final;

private:
    mutable std::mutex _mutex;
    bool _finalized{false};
    class Impl;
    std::unique_ptr<Impl> _impl;
};

} // namespace native
using native::ParticipantStateMachine;
} // namespace fep3
