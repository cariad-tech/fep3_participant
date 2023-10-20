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

#include "participant_state_machine.h"

#include "participant_states.h"

namespace fep3 {

class ParticipantStateMachine::Impl {
public:
    Impl(ElementManager element_manager,
         const std::shared_ptr<ComponentRegistry>& component_registry,
         const std::shared_ptr<ILogger>& logger)
        : _element_manager(std::move(element_manager)),
          _component_registry(component_registry),
          _logger(logger),
          _state(std::make_unique<Unloaded>(_element_manager, _component_registry, _logger))
    {
    }

    Result load()
    {
        auto result = _state->load();
        if (result.state) {
            _state = std::move(result.state);
        }
        return result.result;
    }

    Result unload()
    {
        auto result = _state->unload();
        if (result.state) {
            _state = std::move(result.state);
        }
        return result.result;
    }

    Result initialize()
    {
        auto result = _state->initialize();
        if (result.state) {
            _state = std::move(result.state);
        }
        return result.result;
    }

    Result deinitialize()
    {
        auto result = _state->deinitialize();
        if (result.state) {
            _state = std::move(result.state);
        }
        return result.result;
    }

    Result start()
    {
        auto result = _state->start();
        if (result.state) {
            _state = std::move(result.state);
        }
        return result.result;
    }

    Result stop()
    {
        auto result = _state->stop();
        if (result.state) {
            _state = std::move(result.state);
        }
        return result.result;
    }

    Result pause()
    {
        auto result = _state->pause();
        if (result.state) {
            _state = std::move(result.state);
        }
        return result.result;
    }

    Result exit()
    {
        auto result = _state->exit();
        if (result.state) {
            _state = std::move(result.state);
        }
        return result.result;
    }

    std::string getCurrentStateName() const
    {
        return _state->getName();
    }

private:
    ElementManager _element_manager;
    std::shared_ptr<ComponentRegistry> _component_registry;
    std::shared_ptr<ILogger> _logger;
    std::unique_ptr<State> _state;
};

ParticipantStateMachine::ParticipantStateMachine(
    ElementManager element_manager,
    const std::shared_ptr<ComponentRegistry>& component_registry,
    const std::shared_ptr<ILogger>& participant_logger)
    : _impl(std::make_unique<Impl>(
          std::move(element_manager), component_registry, participant_logger))
{
}

ParticipantStateMachine::~ParticipantStateMachine() = default;

bool ParticipantStateMachine::isFinalized() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _finalized;
}

std::string ParticipantStateMachine::getCurrentStateName() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _impl->getCurrentStateName();
}

Result ParticipantStateMachine::load()
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _impl->load();
}

Result ParticipantStateMachine::unload()
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _impl->unload();
}

Result ParticipantStateMachine::initialize()
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _impl->initialize();
}

Result ParticipantStateMachine::deinitialize()
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _impl->deinitialize();
}

Result ParticipantStateMachine::stop()
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _impl->stop();
}

Result ParticipantStateMachine::start()
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _impl->start();
}

Result ParticipantStateMachine::pause()
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _impl->pause();
}

Result ParticipantStateMachine::exit()
{
    std::lock_guard<std::mutex> lock(_mutex);
    auto result = _impl->exit();
    if (result) {
        _finalized = true;
    }
    return result;
}

} // namespace fep3