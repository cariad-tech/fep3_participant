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

#include "../../logging/default_severity_from_env_variable.h"
#include "component_registry_rpc/component_registry_rpc.h"
#include "console_logger.h"
#include "state_machine/participant_state_machine.h"

#include <fep3/base/component_registry/component_registry_factory.h>
#include <fep3/components/logging/easy_logger.h>
#include <fep3/components/service_bus/service_bus_intf.h>
#include <fep3/participant/participant.h>
#include <fep3/rpc_services/base/fep_rpc_result_to_json.h>
#include <fep3/rpc_services/participant_statemachine/participant_statemachine_rpc_intf_def.h>
#include <fep3/rpc_services/participant_statemachine/participant_statemachine_service_stub.h>

#include <condition_variable>

#define FEP3_PARTICIPANT_COMPONENTS_FILE_PATH_ENVIRONMENT_VARIABLE                                 \
    "FEP3_PARTICIPANT_COMPONENTS_FILE_PATH"

namespace fep3 {
namespace base {

class Participant::Impl
    : public rpc::RPCService<fep3::rpc_stubs::catelyn::RPCParticipantStateMachineServiceStub,
                             fep3::rpc::catelyn::IRPCParticipantStateMachineDef> {
public:
    /**
     * @brief A runner functor to encapsulate the multithread access.
     * Note: Encapsulation is necessary to support moving of the participant.
     */
    class Runner {
    public:
        void operator()(const std::weak_ptr<fep3::ParticipantStateMachine>&
                            weak_ptr_to_participant_state_machine);
        void notify();

    private:
        std::mutex _mutex;
        std::condition_variable _cv;
    };

    Impl(const std::string& name,
         const std::string& version_info,
         const std::string& system_name,
         const std::shared_ptr<ComponentRegistry>& comp_registry,
         const std::shared_ptr<const base::IElementFactory>& elem_factory,
         const std::shared_ptr<ILogger>& default_logger)
        : _name(name),
          _version_info(version_info),
          _system_name(system_name),
          _component_registry(comp_registry),
          _element_factory(elem_factory),
          _default_logger(default_logger),
          _component_registry_service(
              std::make_shared<fep3::native::ComponentRegistryRpcService>(_component_registry))
    {
    }

    Runner _runner;
    std::shared_ptr<fep3::ParticipantStateMachine> _participant_state_machine;

    Json::Value load()
    {
        Json::Value ret;

        if (_participant_state_machine) {
            ret = fep3::rpc::arya::resultToJson(_participant_state_machine->load());
        }
        else {
            ret = fep3::rpc::arya::resultToJson(
                CREATE_ERROR_DESCRIPTION(ERR_POINTER, "the participant state machine is invalid"));
        }
        return ret;
    }

    Json::Value unload()
    {
        Json::Value ret;

        if (_participant_state_machine) {
            ret = fep3::rpc::arya::resultToJson(_participant_state_machine->unload());
        }
        else {
            ret = fep3::rpc::arya::resultToJson(
                CREATE_ERROR_DESCRIPTION(ERR_POINTER, "the participant state machine is invalid"));
        }
        return ret;
    }

    Json::Value initialize()
    {
        Json::Value ret;

        if (_participant_state_machine) {
            ret = fep3::rpc::arya::resultToJson(_participant_state_machine->initialize());
        }
        else {
            ret = fep3::rpc::arya::resultToJson(
                CREATE_ERROR_DESCRIPTION(ERR_POINTER, "the participant state machine is invalid"));
        }
        return ret;
    }

    Json::Value deinitialize()
    {
        Json::Value ret;

        if (_participant_state_machine) {
            ret = fep3::rpc::arya::resultToJson(_participant_state_machine->deinitialize());
        }
        else {
            ret = fep3::rpc::arya::resultToJson(
                CREATE_ERROR_DESCRIPTION(ERR_POINTER, "the participant state machine is invalid"));
        }
        return ret;
    }

    Json::Value stop()
    {
        Json::Value ret;

        if (_participant_state_machine) {
            ret = fep3::rpc::arya::resultToJson(_participant_state_machine->stop());
        }
        else {
            ret = fep3::rpc::arya::resultToJson(
                CREATE_ERROR_DESCRIPTION(ERR_POINTER, "the participant state machine is invalid"));
        }
        return ret;
    }

    Json::Value start()
    {
        Json::Value ret;

        if (_participant_state_machine) {
            ret = fep3::rpc::arya::resultToJson(_participant_state_machine->start());
        }
        else {
            ret = fep3::rpc::arya::resultToJson(
                CREATE_ERROR_DESCRIPTION(ERR_POINTER, "the participant state machine is invalid"));
        }
        return ret;
    }

    Json::Value pause()
    {
        Json::Value ret;

        if (_participant_state_machine) {
            ret = fep3::rpc::arya::resultToJson(_participant_state_machine->pause());
        }
        else {
            ret = fep3::rpc::arya::resultToJson(
                CREATE_ERROR_DESCRIPTION(ERR_POINTER, "the participant state machine is invalid"));
        }
        return ret;
    }

    std::string getCurrentStateName()
    {
        return _participant_state_machine ? _participant_state_machine->getCurrentStateName() : "";
    }
    Json::Value exit()
    {
        Json::Value ret;

        if (_participant_state_machine) {
            auto result = _participant_state_machine->exit();
            if (result) {
                _runner.notify();
            }
            ret = fep3::rpc::arya::resultToJson(result);
        }
        else {
            // if participant state machine is invalid, exitting is considered successful
            ret = fep3::rpc::arya::resultToJson(fep3::Result{});
        }
        return ret;
    }

    std::shared_ptr<ComponentRegistry> _component_registry;
    std::shared_ptr<const base::IElementFactory> _element_factory;
    std::shared_ptr<ILogger> _default_logger;
    std::string getName() const
    {
        return _name;
    }
    std::string getSystemName() const
    {
        return _system_name;
    }
    std::string getVersionInfo() const
    {
        return _version_info;
    }

    std::shared_ptr<fep3::native::ComponentRegistryRpcService> getComponentRegistryRpcService()
    {
        return _component_registry_service;
    }

private:
    std::string _name;
    std::string _system_name;
    std::string _version_info;
    std::shared_ptr<fep3::native::ComponentRegistryRpcService> _component_registry_service;
};

void Participant::Impl::Runner::operator()(
    const std::weak_ptr<fep3::ParticipantStateMachine>& weak_ptr_to_participant_state_machine)
{
    std::unique_lock<std::mutex> lock(_mutex);
    // wait until participant state machine is finalized
    _cv.wait(lock, [weak_ptr_to_participant_state_machine] {
        if (const auto& participant_state_machine = weak_ptr_to_participant_state_machine.lock()) {
            return participant_state_machine->isFinalized();
        }
        else {
            // if the participant state machine is invalid, it is considered to be finalized
            return true;
        }
    });
}

void Participant::Impl::Runner::notify()
{
    _cv.notify_one();
}

/********************************************************************
 *
 *
 *
 ********************************************************************/

Participant::Participant(const fep3::IComponents& components, std::shared_ptr<Impl> impl)
    : _components(&components), _impl(std::move(impl))
{
}

Participant::~Participant() = default;

Participant::Participant(Participant&&) = default;

Participant& Participant::operator=(Participant&& other) = default;

int Participant::exec(const std::function<void()>& start_up_callback)
{
    const auto& component_registry = _impl->_component_registry;
    const auto& component_registry_creation_result = component_registry->create();

    FEP3_LOGGER_LOG_RESULT(_impl->_default_logger, component_registry_creation_result);
    if (component_registry_creation_result) {
        auto logging_service = _impl->_component_registry->getComponent<ILoggingService>();
        std::shared_ptr<ILogger> participant_logger;
        if (logging_service) {
            // if logging service is available, use it for further logging ...
            FEP3_LOGGER_LOG_DEBUG(participant_logger,
                                  "Logging Service Component is now available; all logs will go to "
                                  "the Logging Service from now on")
            participant_logger = logging_service->createLogger("participant");
        }
        else {
            // ... otherwise keep using the logger of the participant implementation
            FEP3_LOGGER_LOG_DEBUG(
                participant_logger,
                "No Logging Service Component available; keep logging to the default logger")
            participant_logger = _impl->_default_logger;
        }
        // the most important part
        // we do not work without a servicebus ???
        auto service_bus = _impl->_component_registry->getComponent<IServiceBus>();

        if (!service_bus) {
            FEP3_LOGGER_LOG_ERROR(participant_logger, "No Service Bus Component available")
        }
        else {
            // the default access must be created while createParticipant!
            fep3::ElementManager man(_impl->_element_factory);
            _impl->_participant_state_machine = std::make_shared<fep3::ParticipantStateMachine>(
                std::move(man), _impl->_component_registry, participant_logger);

            auto server = service_bus->getServer();
            // here we need to think about throwing or return error
            if (server) {
                server->registerService(rpc::IRPCParticipantStateMachineDef::getRPCDefaultName(),
                                        _impl);

                server->registerService(fep3::rpc::IRPCComponentRegistryDef::getRPCDefaultName(),
                                        _impl->getComponentRegistryRpcService());

                if (start_up_callback) {
                    start_up_callback();
                }
                _impl->_runner(_impl->_participant_state_machine);

                server->unregisterService(rpc::IRPCParticipantStateMachineDef::getRPCDefaultName());
                server->unregisterService(fep3::rpc::IRPCComponentRegistryDef::getRPCDefaultName());

                // we release the logger
                participant_logger.reset();

                if (component_registry->destroy()) {
                    return 0;
                }
            }
        }
    }
    return 1;
}

Participant createParticipant(const std::string& name,
                              const std::string& version_info,
                              const std::string& system_name,
                              const std::shared_ptr<const base::IElementFactory>& factory,
                              const std::string& server_address_url)
{
    // a logger that is capable to log before the components (i. e. the Logging Service) are
    // created, to enable logging during creation of Component Registry and Components For now we
    // enable only error and fatal, maybe in the future the severity will be configurable (see
    // FEPSDK-2599)
    std::shared_ptr<ILogger> console_logger =
        std::make_shared<ConsoleLogger>(getDefaultLoggingSeverity());
    std::shared_ptr<ComponentRegistry> components = ComponentRegistryFactory::createRegistry(
        console_logger.get(), FEP3_PARTICIPANT_COMPONENTS_FILE_PATH_ENVIRONMENT_VARIABLE);

    // initialize the service bus here!!
    auto service_bus = components->getComponent<IServiceBus>();
    if (!service_bus) {
        FEP3_LOGGER_LOG_ERROR(console_logger,
                              "No Service Bus Component available; Participant will not be visible "
                              "on the Service Bus")
    }
    else {
        auto result_system_access_creation = service_bus->createSystemAccess(
            system_name, IServiceBus::ISystemAccess::_use_default_url, true);
        FEP3_LOGGER_LOG_RESULT(console_logger, result_system_access_creation);
        if (result_system_access_creation) {
            auto system_access = service_bus->getSystemAccess(system_name);
            std::string use_url = server_address_url;
            if (use_url.empty()) {
                use_url = IServiceBus::ISystemAccess::_use_default_url;
            }
            const auto result_server_creation = system_access->createServer(name, use_url);
            if (!result_server_creation) {
                throw std::runtime_error(
                    std::string("can not create participant ") + name +
                    " Error:" + a_util::strings::toString(result_server_creation.getErrorCode()) +
                    " Description: " + result_server_creation.getDescription());
            }
        }
    }

    return Participant(*components,
                       std::make_shared<Participant::Impl>(
                           name, version_info, system_name, components, factory, console_logger));
}

std::string Participant::getName() const
{
    return _impl->getName();
}

std::string Participant::getSystemName() const
{
    return _impl->getSystemName();
}

std::string Participant::getVersionInfo() const
{
    return _impl->getVersionInfo();
}

} // namespace base
} // namespace fep3
