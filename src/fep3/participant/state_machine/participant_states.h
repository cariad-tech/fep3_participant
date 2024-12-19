/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <fep3/base/component_registry/component_registry.h>
#include <fep3/components/logging/easy_logger.h>
#include <fep3/components/logging/logging_service_intf.h>
#include <fep3/participant/element_manager/element_manager.h>

#include <a_util/result/result_util.h>

#include <memory>

namespace fep3 {

class State;

struct TransitionResult {
    std::unique_ptr<State> state;
    fep3::Result result;
};

class State {
protected:
    State(fep3::ElementManager& element_manager,
          std::shared_ptr<ComponentRegistry> component_registry,
          std::shared_ptr<ILogger> logger);

public:
    [[nodiscard]] virtual TransitionResult load();
    [[nodiscard]] virtual TransitionResult unload();
    [[nodiscard]] virtual TransitionResult initialize();
    [[nodiscard]] virtual TransitionResult deinitialize();
    [[nodiscard]] virtual TransitionResult start();
    [[nodiscard]] virtual TransitionResult stop();
    [[nodiscard]] virtual TransitionResult pause();
    [[nodiscard]] virtual TransitionResult exit();
    virtual std::string getName() const = 0;

    virtual ~State() = default;

protected:
    fep3::ElementManager& _element_manager;
    std::shared_ptr<ComponentRegistry> _component_registry;
    std::shared_ptr<ILogger> _logger;
};

class Finalized final : public State {
public:
    Finalized(fep3::ElementManager& element_manager,
              std::shared_ptr<ComponentRegistry> component_registry,
              std::shared_ptr<ILogger> logger);

    std::string getName() const;
};

class Unloaded final : public State {
public:
    Unloaded(fep3::ElementManager& element_manager,
             std::shared_ptr<ComponentRegistry> component_registry,
             std::shared_ptr<ILogger> logger);

    TransitionResult exit() override;
    TransitionResult load() override;
    std::string getName() const override;
};

class Loaded final : public State {
public:
    Loaded(fep3::ElementManager& element_manager,
           std::shared_ptr<ComponentRegistry> component_registry,
           std::shared_ptr<ILogger> logger);

    TransitionResult unload() override;
    TransitionResult initialize() override;
    std::string getName() const override;
};

class Initialized final : public State {
public:
    Initialized(fep3::ElementManager& element_manager,
                std::shared_ptr<ComponentRegistry> component_registry,
                std::shared_ptr<ILogger> logger);

    TransitionResult deinitialize() override;
    TransitionResult start() override;
    TransitionResult pause() override;
    std::string getName() const override;
};

class Running final : public State {
public:
    Running(fep3::ElementManager& element_manager,
            std::shared_ptr<ComponentRegistry> component_registry,
            std::shared_ptr<ILogger> logger);

    TransitionResult pause() override;
    TransitionResult stop() override;
    std::string getName() const override;
};

class Paused final : public State {
public:
    Paused(fep3::ElementManager& element_manager,
           std::shared_ptr<ComponentRegistry> component_registry,
           std::shared_ptr<ILogger> logger);

    TransitionResult start() override;
    TransitionResult stop() override;
    std::string getName() const override;
};

} // namespace fep3
