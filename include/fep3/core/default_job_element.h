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

#include "step_unwinding.h"

#include <fep3/base/properties/propertynode.h>
#include <fep3/components/base/component.h>
#include <fep3/components/base/components_intf.h>
#include <fep3/components/clock/clock_service_intf.h>
#include <fep3/components/configuration/configuration_service_intf.h>
#include <fep3/components/data_registry/data_registry_intf.h>
#include <fep3/components/job_registry/job_registry_intf.h>
#include <fep3/components/logging/easy_logger.h>
#include <fep3/core/custom_job_element.h>
#include <fep3/core/data_io_container.h>
#include <fep3/core/default_job.h>
#include <fep3/participant/participant.h>

#include <type_traits>

namespace fep3::core {

/**
 * @brief The default job element for creating element with job.
 * @tparam UserJobElement
 */
template <typename UserJobElement>
class DefaultJobElement : public fep3::base::IElement, public fep3::base::arya::EasyLogging {
public:
    /// @cond nodoc
    using IDataRegistry = fep3::arya::IDataRegistry;
    using IConfigurationService = fep3::arya::IConfigurationService;
    using IJobRegistry = fep3::catelyn::IJobRegistry;
    using IClockService = fep3::arya::IClockService;
    /// @endcond nodoc

    /**
     * @brief CTOR
     * @param user_impl user implemented job element, must be inherited from
     *                  fep3::core::CustomJobElement
     */
    explicit DefaultJobElement(std::unique_ptr<UserJobElement> user_impl)
    {
        static_assert(std::is_base_of<fep3::core::CustomJobElement, UserJobElement>::value,
                      "Type not supported. Please inherit from CustomJobElement");
        _impl = std::move(user_impl);
        _io_container = std::make_unique<fep3::core::DataIOContainer>();
    }

    /**
     * @brief CTOR
     * @param user_impl unique pointer of user implemented job element,
     *                  must be inherited from fep3::core::CustomJobElement
     * @param io_container unique pointer of data io container
     */
    DefaultJobElement(std::unique_ptr<UserJobElement> user_impl,
                      std::unique_ptr<fep3::core::IDataIOContainer> io_container)
    {
        _impl = std::move(user_impl);
        _io_container = std::move(io_container);
    }

public:
    /// @copydoc fep3::base::IElement::getTypename
    std::string getTypename() const override final
    {
        return _impl->getTypename();
    }

    /// @copydoc fep3::base::IElement::getVersion
    std::string getVersion() const override final
    {
        return _impl->getVersion();
    }

    /// @copydoc fep3::base::IElement::loadElement
    fep3::Result loadElement(const fep3::IComponents& components) override final
    {
        fep3::core::CustomJobElement::JobConfigPtr job_configuration;
        auto load_transition = getLoadTransition(components, job_configuration);
        return load_transition.doTransition();
    }

    /// @copydoc fep3::base::IElement::initialize
    fep3::Result initialize() override final
    {
        auto initialize_transition = getInitializeTransition();
        return initialize_transition.doTransition();
    }

    /// @copydoc fep3::base::IElement::run
    fep3::Result run() override final
    {
        auto run_transition = getRunTransition();
        return run_transition.doTransition();
    }

    /// @copydoc fep3::base::IElement::stop
    void stop() override final
    {
        auto run_transition = getRunTransition();
        run_transition.doReverseTransition();
    }

    /// @copydoc fep3::base::IElement::deinitialize
    void deinitialize() override final
    {
        auto initialize_transition = getInitializeTransition();
        initialize_transition.doReverseTransition();
    }

    /// @copydoc fep3::base::IElement::unloadElement
    void unloadElement() override final
    {
        // custom unload
        fep3::core::CustomJobElement::JobConfigPtr job_configuration;
        auto load_transition = getLoadTransition(*_components, job_configuration);
        return load_transition.doReverseTransition();
    }

private:
    fep3::core::detail::Transition getLoadTransition(
        const fep3::IComponents& components,
        fep3::core::CustomJobElement::JobConfigPtr& job_configuration)
    {
        fep3::core::detail::Transition load_transition;
        load_transition.addAction([&]() { _components = &components; },
                                  [&]() { _components = nullptr; });

        load_transition.addAction(
            [&components, this]() { return initLogger(components, "DefaultJobElement.element"); },
            [this]() { deinitLogger(); });

        load_transition.addAction(
            [&components, this]() {
                return _impl->initLogger(components, getTypename() + ".element");
            },
            [this]() { _impl->deinitLogger(); });

        load_transition.addAction(
            [&components, this]() {
                auto [res, config_service] =
                    base::getComponentHelper<IConfigurationService>(components);
                FEP3_RETURN_IF_FAILED(res);
                return _impl->initConfiguration(*config_service);
            },
            [this]() { _impl->deinitConfiguration(); });

        load_transition.addAction(
            [&]() {
                auto [result, job, job_config] = _impl->createJob(*_components);
                _job = job;
                job_configuration = std::move(job_config);
                return result;
            },
            []() {});

        load_transition.addAction(
            [this]() { return _job->initLogger(*_components, _job->getName() + ".job"); },
            [this]() { _job->deinitLogger(); });

        load_transition.addAction(
            [this]() {
                auto [res, config_service] =
                    base::getComponentHelper<IConfigurationService>(*_components);
                FEP3_RETURN_IF_FAILED(res);
                return _job->initConfiguration(*config_service);
            },
            [&]() { _job->deinitConfiguration(); });

        load_transition.addAction([&]() { return _job->createDefaultPropertyVariables(); },
                                  [&]() { _job->removeDefaultPropertyVariables(); });

        load_transition.addAction(
            [this, &job_configuration]() {
                auto [res, job_registry] = base::getComponentHelper<IJobRegistry>(*_components);
                FEP3_RETURN_IF_FAILED(res);
                _job->setDataIOContainer(*_io_container);
                FEP3_RETURN_IF_FAILED(
                    job_registry->addJob(_job->getName(), _job, *job_configuration));
                return fep3::Result{};
            },
            [&]() {
                auto [res, job_registry] = base::getComponentHelper<IJobRegistry>(*_components);
                if (!res) {
                    FEP3_LOG_FATAL(res.getDescription());
                }
                else {
                    FEP3_LOG_RESULT(job_registry->removeJob(_job->getName()));
                }
                _impl->destroyJob();
            });

        load_transition.addAction([this]() { return _impl->load(*_components); },
                                  [this]() { _impl->unload(*_components); });
        return load_transition;
    }

    fep3::core::detail::Transition getInitializeTransition()
    {
        fep3::core::detail::Transition initialize_transition;
        initialize_transition.addAction(
            [&_impl = _impl]() { return _impl->updatePropertyVariables(); },
            fep3::core::detail::NoAction{});

        initialize_transition.addAction(
            [&_job = _job]() { return _job->updatePropertyVariables(); },
            fep3::core::detail::NoAction{});

        initialize_transition.addAction([this]() { return createJobDataIOs(); },
                                        fep3::core::detail::NoAction{});
        initialize_transition.addAction([this]() { return addJobIOsToDataRegistry(); },
                                        [this]() {
                                            auto [res_data_registry, data_registry] =
                                                base::getComponentHelper<IDataRegistry>(
                                                    *_components);
                                            FEP3_RETURN_IF_FAILED(res_data_registry)
                                            _io_container->removeFromDataRegistry(*data_registry);
                                            return fep3::Result{};
                                        });

        initialize_transition.addAction([this]() { return _job->initialize(*_components); },
                                        [this]() {
                                            if (const auto result = _job->deinitialize(); !result) {
                                                FEP3_LOG_RESULT(result);
                                            }
                                        });
        initialize_transition.addAction([this]() { return _impl->initialize(*_components); },
                                        [this]() { _impl->deinitialize(*_components); });

        return initialize_transition;
    }

    fep3::Result createJobDataIOs()
    {
        auto [res, job_registry] = base::getComponentHelper<IJobRegistry>(*_components);
        FEP3_RETURN_IF_FAILED(res);
        auto job_infos = job_registry->getJobInfosCatelyn();
        auto it =
            std::find_if(std::begin(job_infos), std::end(job_infos), [&](const auto& job_info) {
                return job_info.getName() == _job->getName();
            });

        if (it == job_infos.end()) {
            RETURN_ERROR_DESCRIPTION(fep3::ERR_NOT_FOUND,
                                     "job %s is not found in job registry",
                                     _job->getName().c_str());
        }

        FEP3_RETURN_IF_FAILED(_job->applyDefaultPropertyVariables());

        _job->createDataIOs(*_components, *_io_container, it->getConfig());
        return {};
    }

    fep3::Result addJobIOsToDataRegistry()
    {
        auto [res_data_registry, data_registry] =
            base::getComponentHelper<IDataRegistry>(*_components);
        FEP3_RETURN_IF_FAILED(res_data_registry);

        auto [res_clock_service, clock_service] =
            base::getComponentHelper<IClockService>(*_components);
        FEP3_RETURN_IF_FAILED(res_clock_service);

        FEP3_RETURN_IF_FAILED(_io_container->addToDataRegistry(*data_registry, *clock_service));
        return {};
    }

    fep3::core::detail::Transition getRunTransition()
    {
        fep3::core::detail::Transition run_transition;

        run_transition.addAction([this]() { _impl->updatePropertyVariables(); },
                                 fep3::core::detail::NoAction{});

        run_transition.addAction([this]() { _job->updatePropertyVariables(); },
                                 fep3::core::detail::NoAction{});

        run_transition.addAction([this]() { return _job->start(); },
                                 [this]() {
                                     if (const auto result = _job->stop(); !result) {
                                         FEP3_LOG_RESULT(result);
                                     }
                                     _job->logIOInfo();
                                 });
        run_transition.addAction([this]() { return _impl->run(); }, [this]() { _impl->stop(); });

        return run_transition;
    }

    /// custom job element
    std::unique_ptr<fep3::core::CustomJobElement> _impl{};
    /// default job
    std::shared_ptr<fep3::core::DefaultJob> _job;
    /// data IO container
    std::unique_ptr<fep3::core::IDataIOContainer> _io_container;
    /// components
    const fep3::IComponents* _components{};
};
} // namespace fep3::core
