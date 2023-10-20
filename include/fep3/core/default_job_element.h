/**
 * @file
 * @copyright
 * @verbatim
Copyright @ 2023 VW Group. All rights reserved.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
@endverbatim
 */

#pragma once

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
        _components = &components;

        FEP3_RETURN_IF_FAILED(initLogger(components, "DefaultJobElement.element"));

        const auto config_service = components.getComponent<IConfigurationService>();
        if (!config_service) {
            RETURN_ERROR_DESCRIPTION(fep3::ERR_NO_INTERFACE,
                                     "%s is not part of the given component registry",
                                     IConfigurationService::getComponentIID());
        }

        const auto job_registry = components.getComponent<IJobRegistry>();
        if (!job_registry) {
            RETURN_ERROR_DESCRIPTION(fep3::ERR_NO_INTERFACE,
                                     "%s is not part of the given component registry",
                                     IJobRegistry::getComponentIID());
        }

        FEP3_RETURN_IF_FAILED(_impl->initLogger(*_components, getTypename() + ".element"));
        FEP3_RETURN_IF_FAILED(_impl->initConfiguration(*config_service));

        // init job
        auto [result, job, job_config] = _impl->createJob();

        _job = job;

        FEP3_RETURN_IF_FAILED(result);
        FEP3_RETURN_IF_FAILED(_job->initLogger(*_components, _job->getName() + ".job"));
        FEP3_RETURN_IF_FAILED(_job->initConfiguration(*config_service));
        FEP3_RETURN_IF_FAILED(job_registry->addJob(_job->getName(), _job, *job_config));

        _job->setDataIOContainer(*_io_container);

        // custom load
        return _impl->load(*_components);
    }

    /// @copydoc fep3::base::IElement::initialize
    fep3::Result initialize() override final
    {
        _impl->updatePropertyVariables();

        _job->updatePropertyVariables();

        const auto job_registry = _components->getComponent<IJobRegistry>();
        if (!job_registry) {
            RETURN_ERROR_DESCRIPTION(fep3::ERR_NO_INTERFACE,
                                     "%s is not part of the given component registry",
                                     IJobRegistry::getComponentIID());
        }

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

        _job->createDataIOs(*_components, *_io_container, it->getConfig());

        const auto data_registry = _components->getComponent<IDataRegistry>();
        if (!data_registry) {
            RETURN_ERROR_DESCRIPTION(fep3::ERR_NO_INTERFACE,
                                     "%s is not part of the given component registry",
                                     IDataRegistry::getComponentIID());
        }

        auto clock_service = _components->getComponent<IClockService>();
        if (!clock_service) {
            RETURN_ERROR_DESCRIPTION(fep3::ERR_NO_INTERFACE,
                                     "%s is not part of the given component registry",
                                     IClockService::getComponentIID());
        }

        FEP3_RETURN_IF_FAILED(_io_container->addToDataRegistry(*data_registry, *clock_service));

        FEP3_RETURN_IF_FAILED(_job->initialize(*_components));

        // custom initialize
        return _impl->initialize(*_components);
    }

    /// @copydoc fep3::base::IElement::run
    fep3::Result run() override final
    {
        _impl->updatePropertyVariables();
        _job->updatePropertyVariables();
        FEP3_RETURN_IF_FAILED(_job->start());

        return _impl->run();
    }

    /// @copydoc fep3::base::IElement::stop
    void stop() override final
    {
        _impl->stop();
        _job->stop();
    }

    /// @copydoc fep3::base::IElement::deinitialize
    void deinitialize() override final
    {
        // custom deinitialize
        _impl->deinitialize(*_components);

        const auto data_registry = _components->getComponent<IDataRegistry>();
        if (!data_registry) {
            FEP3_LOG_FATAL(a_util::strings::format("%s is not part of the given component registry",
                                                   IDataRegistry::getComponentIID()));
            return;
        }
        _io_container->removeFromDataRegistry(*data_registry);
    }

    /// @copydoc fep3::base::IElement::unloadElement
    void unloadElement() override final
    {
        // custom unload
        _impl->unload(*_components);

        const auto job_registry = _components->getComponent<IJobRegistry>();
        if (!job_registry) {
            FEP3_LOG_FATAL(a_util::strings::format("%s is not part of the given component registry",
                                                   IJobRegistry::getComponentIID()));
        }

        job_registry->removeJob(_job->getName());

        _impl->destroyJob();

        _job->deinitConfiguration();
        _impl->deinitConfiguration();
        _job->deinitLogger();
        _impl->deinitLogger();
        deinitLogger();

        _components = nullptr;
    }

private:
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
