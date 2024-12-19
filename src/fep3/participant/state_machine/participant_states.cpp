/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "participant_states.h"

namespace {
void logInfoOrError(const std::shared_ptr<fep3::ILogger>& logger,
                    const char* success_message,
                    const char* failure_message,
                    const fep3::Result& result)
{
    if (result) {
        FEP3_LOGGER_LOG_INFO(logger, std::string(success_message));
    }
    else {
        FEP3_LOGGER_LOG_ERROR(logger,
                              std::string(failure_message) + " : " +
                                  a_util::result::toString(result) + " - " +
                                  result.getDescription());
    }
}
} // namespace

namespace fep3 {

State::State(fep3::ElementManager& element_manager,
             std::shared_ptr<ComponentRegistry> component_registry,
             std::shared_ptr<ILogger> logger)
    : _element_manager(element_manager), _component_registry(component_registry), _logger(logger)
{
}

TransitionResult State::load()
{
    return {nullptr,
            CREATE_ERROR_DESCRIPTION(ERR_INVALID_STATE,
                                     "Transition 'load' is not allowed from state '%s'.",
                                     getName().c_str())};
}

TransitionResult State::unload()
{
    return {nullptr,
            CREATE_ERROR_DESCRIPTION(ERR_INVALID_STATE,
                                     "Transition 'unload' is not allowed from state '%s'.",
                                     getName().c_str())};
}

TransitionResult State::initialize()
{
    return {nullptr,
            CREATE_ERROR_DESCRIPTION(ERR_INVALID_STATE,
                                     "Transition 'initialize' is not allowed from state '%s'.",
                                     getName().c_str())};
}

TransitionResult State::deinitialize()
{
    return {nullptr,
            CREATE_ERROR_DESCRIPTION(ERR_INVALID_STATE,
                                     "Transition 'deinitialize' is not allowed from state '%s'.",
                                     getName().c_str())};
}

TransitionResult State::start()
{
    return {nullptr,
            CREATE_ERROR_DESCRIPTION(ERR_INVALID_STATE,
                                     "Transition 'start' is not allowed from state '%s'.",
                                     getName().c_str())};
}

TransitionResult State::stop()
{
    return {nullptr,
            CREATE_ERROR_DESCRIPTION(ERR_INVALID_STATE,
                                     "Transition 'stop' is not allowed from state '%s'.",
                                     getName().c_str())};
}

TransitionResult State::pause()
{
    return {nullptr,
            CREATE_ERROR_DESCRIPTION(ERR_INVALID_STATE,
                                     "Transition 'pause' is not allowed from state '%s'.",
                                     getName().c_str())};
}

TransitionResult State::exit()
{
    return {nullptr,
            CREATE_ERROR_DESCRIPTION(ERR_INVALID_STATE,
                                     "Transition 'exit' is not allowed from state '%s'.",
                                     getName().c_str())};
}

Finalized::Finalized(fep3::ElementManager& element_manager,
                     std::shared_ptr<ComponentRegistry> component_registry,
                     std::shared_ptr<ILogger> logger)
    : State(element_manager, component_registry, logger)
{
}

std::string Finalized::getName() const
{
    return "Finalized";
}

Unloaded::Unloaded(fep3::ElementManager& element_manager,
                   std::shared_ptr<ComponentRegistry> component_registry,
                   std::shared_ptr<ILogger> logger)
    : State(element_manager, component_registry, logger)
{
}

TransitionResult Unloaded::exit()
{
    return TransitionResult{
        std::make_unique<Finalized>(_element_manager, _component_registry, _logger), {}};
}

TransitionResult Unloaded::load()
{
    auto res = _element_manager.loadElement(*_component_registry);
    logInfoOrError(_logger, "Successfully loaded element", "Failed to load element", res);
    if (!res) {
        return TransitionResult{nullptr, res};
    }

    return TransitionResult{
        std::make_unique<Loaded>(_element_manager, _component_registry, _logger), res};
}

std::string Unloaded::getName() const
{
    return "Unloaded";
}

Loaded::Loaded(fep3::ElementManager& element_manager,
               std::shared_ptr<ComponentRegistry> component_registry,
               std::shared_ptr<ILogger> logger)
    : State(element_manager, component_registry, logger)
{
}

TransitionResult Loaded::unload()
{
    _element_manager.unloadElement();
    FEP3_LOGGER_LOG_INFO(_logger, "Successfully unloaded element");

    return TransitionResult{
        std::make_unique<Unloaded>(_element_manager, _component_registry, _logger), fep3::Result()};
}

TransitionResult Loaded::initialize()
{
    auto res = _element_manager.initializeElement();
    logInfoOrError(
        _logger, "Successfully initialized element", "Failed to initialize element", res);
    if (!res) {
        return TransitionResult{nullptr, res};
    }

    res = _component_registry->initialize();
    logInfoOrError(_logger,
                   "Successfully initialized components",
                   "Failed to initialize components (rolling back initialization of element)",
                   res);
    if (!res) {
        _element_manager.deinitializeElement();
        FEP3_LOGGER_LOG_INFO(_logger, "Successfully deinitialized element");
        return TransitionResult{nullptr, res};
    }

    res = _component_registry->tense();
    logInfoOrError(_logger,
                   "Successfully tensed components",
                   "Failed to tense components (rolling back initialization "
                   "of components and element)",
                   res);
    if (!res) {
        const auto tmp_res = _component_registry->deinitialize();
        logInfoOrError(_logger,
                       "Successfully deinitialized components",
                       "Deinitialized components with error",
                       tmp_res);
        _element_manager.deinitializeElement();
        FEP3_LOGGER_LOG_INFO(_logger, "Successfully deinitialized element");
        return TransitionResult{nullptr, res};
    }

    return TransitionResult{
        std::make_unique<Initialized>(_element_manager, _component_registry, _logger), res};
}

std::string Loaded::getName() const
{
    return "Loaded";
}

Initialized::Initialized(fep3::ElementManager& element_manager,
                         std::shared_ptr<ComponentRegistry> component_registry,
                         std::shared_ptr<ILogger> logger)
    : State(element_manager, component_registry, logger)
{
}

TransitionResult Initialized::deinitialize()
{
    auto res = _component_registry->relax();
    logInfoOrError(_logger, "Successfully relaxed components", "Failed to relax components", res);

    res = _component_registry->deinitialize();
    logInfoOrError(
        _logger, "Successfully deinitialized components", "Failed to deinitialize components", res);

    _element_manager.deinitializeElement();
    FEP3_LOGGER_LOG_INFO(_logger, "Successfully deinitialized element");

    return TransitionResult{
        std::make_unique<Loaded>(_element_manager, _component_registry, _logger), res};
}

TransitionResult Initialized::start()
{
    auto res = _element_manager.runElement();
    logInfoOrError(_logger, "Successfully ran element", "Failed to run element", res);
    if (!res) {
        return TransitionResult{nullptr, res};
    }

    res = _component_registry->start();
    logInfoOrError(_logger,
                   "Successfully started components",
                   "Failed to start components (rolling back start of element)",
                   res);
    if (!res) {
        _element_manager.stopElement();
        FEP3_LOGGER_LOG_INFO(_logger, "Successfully stopped element");
        return TransitionResult{nullptr, res};
    }

    return TransitionResult{
        std::make_unique<Running>(_element_manager, _component_registry, _logger), res};
}

TransitionResult Initialized::pause()
{
    auto res = _element_manager.runElement();
    logInfoOrError(_logger, "Successfully ran element", "Failed to run element", res);
    if (!res) {
        return TransitionResult{nullptr, res};
    }

    res = _component_registry->pause();
    logInfoOrError(_logger,
                   "Successfully paused components",
                   "Failed to pause components (rolling back start of element)",
                   res);
    if (!res) {
        _element_manager.stopElement();
        FEP3_LOGGER_LOG_INFO(_logger, "Successfully stopped element");
        return TransitionResult{nullptr, res};
    }

    return TransitionResult{
        std::make_unique<Paused>(_element_manager, _component_registry, _logger), res};
}

std::string Initialized::getName() const
{
    return "Initialized";
}

Running::Running(fep3::ElementManager& element_manager,
                 std::shared_ptr<ComponentRegistry> component_registry,
                 std::shared_ptr<ILogger> logger)
    : State(element_manager, component_registry, logger)
{
}

TransitionResult Running::pause()
{
    auto res = _component_registry->pause();
    logInfoOrError(_logger, "Successfully paused components", "Failed to pause components", res);
    if (!res) {
        return TransitionResult{nullptr, res};
    }

    return TransitionResult{
        std::make_unique<Paused>(_element_manager, _component_registry, _logger), res};
}

TransitionResult Running::stop()
{
    auto res = _component_registry->stop();
    logInfoOrError(_logger, "Successfully stopped components", "Failed to stop components", res);

    _element_manager.stopElement();
    FEP3_LOGGER_LOG_INFO(_logger, "Successfully stopped element");

    return TransitionResult{
        std::make_unique<Initialized>(_element_manager, _component_registry, _logger), res};
}

std::string Running::getName() const
{
    return "Running";
}

Paused::Paused(fep3::ElementManager& element_manager,
               std::shared_ptr<ComponentRegistry> component_registry,
               std::shared_ptr<ILogger> logger)
    : State(element_manager, component_registry, logger)
{
}

TransitionResult Paused::start()
{
    return {nullptr,
            CREATE_ERROR_DESCRIPTION(ERR_INVALID_STATE, "Pause state is not supported yet.")};
}

TransitionResult Paused::stop()
{
    return {nullptr,
            CREATE_ERROR_DESCRIPTION(ERR_INVALID_STATE, "Pause state is not supported yet.")};
}

std::string Paused::getName() const
{
    return "Paused";
}

} // namespace fep3