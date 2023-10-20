#include "http_url_getter.h"

#include <a_util/process/process.h>

#include <cxx_url.h>

namespace fep3 {
namespace helper {
const constexpr char* fep3_servicebus_system_url_env = "FEP3_SERVICEBUS_SYSTEM_URL";
const constexpr char* fep3_servicebus_server_url_env = "FEP3_SERVICEBUS_SERVER_URL";

std::string getInputOrEnvVariable(const std::string& input_url, const char* env_variable_name)
{
    std::string env_variable = a_util::process::getEnvVar(env_variable_name, "");
    if (env_variable.empty()) {
        return input_url;
    }
    else {
        return env_variable;
    }
}

fep3::Result evaluateUrl(const std::string& input_system_url)
{
    fep3::helper::Url url(input_system_url);
    if (url.scheme() != "http") {
        RETURN_ERROR_DESCRIPTION(
            ERR_INVALID_ARG,
            "This service bus does only support 'http' protocol, but it is called with '%s'",
            input_system_url.c_str());
    }
    else {
        return {};
    }
}

std::pair<fep3::Result, std::string> getSystemUrl(
    const std::string& input_url,
    const fep3::base::SystemAccessBase::ISystemAccessBaseDefaultUrls& service_bus_system_default)
{
    std::string input_system_url = getInputOrEnvVariable(input_url, fep3_servicebus_system_url_env);
    std::string used_system_url;
    fep3::Result parseResult{};

    if (input_system_url == fep3::IServiceBus::ISystemAccess::_use_default_url) {
        used_system_url = service_bus_system_default.getDefaultSystemUrl();
    }
    else if (input_system_url.empty()) {
        // is valid we do not want to use discovery
        // we know all addresses from outside!
        used_system_url = {};
    }
    else {
        used_system_url = input_system_url;
        parseResult = evaluateUrl(input_system_url);
    }

    return std::make_pair(parseResult, used_system_url);
}

std::string getServerUrl(
    const std::string& input_url,
    const fep3::base::SystemAccessBase::ISystemAccessBaseDefaultUrls& service_bus_system_default)
{
    std::string server_url = getInputOrEnvVariable(input_url, fep3_servicebus_server_url_env);

    if (server_url == IServiceBus::ISystemAccess::_use_default_url) {
        server_url = service_bus_system_default.getDefaultServerUrl();
    }
    else {
        fep3::helper::Url url(server_url);
        if (url.scheme() != "http") {
            throw std::runtime_error(
                a_util::strings::format("service bus: can not create server . Server does only "
                                        "support http, but it is called with '%s'",
                                        server_url.c_str()));
        }
    }

    if (server_url.empty()) {
        throw std::runtime_error(a_util::strings::format(
            "service bus: can not create server with url '%s' ", server_url.c_str()));
    }

    return server_url;
}
} // namespace helper
} // namespace fep3
