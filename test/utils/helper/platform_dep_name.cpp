/**
 * @copyright
 * @verbatim
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * @endverbatim
 */
#include <a_util/process.h>
#include <a_util/strings.h>

#include <boost/asio/ip/host_name.hpp>

#include <sstream>
#include <thread>

const std::string makePlatformDepName(const std::string& original_name)
{
    std::string strModuleNameDep(original_name);

    std::stringstream ss;
    ss << std::this_thread::get_id();

    strModuleNameDep += "_" + boost::asio::ip::host_name();
    strModuleNameDep += "_" + std::to_string(a_util::process::getCurrentProcessId());
    strModuleNameDep += "_" + ss.str();
    return strModuleNameDep;
}
