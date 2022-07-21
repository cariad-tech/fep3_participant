#include "platform_dep_name.h"
#include <boost/asio/ip/host_name.hpp>
#include <a_util/process.h>
#include <a_util/strings.h>
#include <sstream>
#include <thread>

const std::string makePlatformDepName(const std::string& original_name)
{
    std::string strModuleNameDep(original_name);

    std::stringstream ss;
    ss << std::this_thread::get_id();

    strModuleNameDep += "_" + boost::asio::ip::host_name();
    strModuleNameDep += "_" + a_util::strings::toString(a_util::process::getCurrentProcessId());
    strModuleNameDep += "_" + ss.str();
    return strModuleNameDep;
}
