/**
 * @file
 * @copyright
 * @verbatim
Copyright @ 2021 VW Group. All rights reserved.

    This Source Code Form is subject to the terms of the Mozilla
    Public License, v. 2.0. If a copy of the MPL was not distributed
    with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

If it is not possible or desirable to put the notice in a particular file, then
You may include the notice in a location (such as a LICENSE file in a
relevant directory) where a recipient would be likely to look for such a notice.

You may add additional accurate notices of copyright ownership.

@endverbatim
 */


#include "commandline_parser_impl.h"

#include <iostream>

#include <fep3/fep3_participant_version.h>

namespace fep3
{
namespace arya
{
CommandLineParserImpl::CommandLineParserImpl(clara::Parser cli, const ParserDefaultValues& default_values) :
    _cli(cli),
    _participant_name(default_values.participant_name),
    _system_name(default_values.system_name),
    _server_address_url(default_values.server_address_url)
{
    _cli |= clara::Help(_show_help);
    _cli |= clara::Opt(_show_version)
        ["-v"]["--version"]
        ("Print FEP SDK version.");

    // Make participant name argument mandatory if no default value has been given
    if (_participant_name.empty())
    {
        _cli |= clara::Arg(_participant_name, "participant").required()
            ("Set participant name.");
    }
    else
    {
        _cli |= clara::Opt(_participant_name, "string")
            ["-n"]["--name"]
            ("Set participant name.");
    }

    // Make system name argument mandatory if no default value has been given
    if (_system_name.empty())
    {
        _cli |= clara::Arg(_system_name, "system").required()
            ("Set the system of the participant.");
    }
    else
    {
        _cli |= clara::Opt(_system_name, "string")
            ["-s"]["--system"]
            ("Set the system of the participant.");
    }

    _cli |= clara::Opt(_server_address_url, "string")
        ["-u"]["--url"]
        ("Set the server address url");
}

void CommandLineParserImpl::parseArgs(int argc, char const *const *argv)
{
    // The version of clara we use is not const correct so we need to cast away the const
    auto result = _cli.parse(clara::Args(argc, const_cast<char**>(argv)));
    if (!result)
    {
        std::cerr << "Error in command line: " << result.errorMessage() << std::endl << std::endl;
        printHelp();
        std::exit(1);
    }

    // Clara does not check if required arguments were passed, so we have to do it ourselves
    if (_participant_name.empty())
    {
        std::cerr << "Error in command line: Missing required argument: participant name" << std::endl << std::endl;
        printHelp();
        std::exit(1);
    }

    if (_system_name.empty())
    {
        std::cerr << "Error in command line: Missing required argument: system name" << std::endl << std::endl;
        printHelp();
        std::exit(1);
    }

    if (_show_help)
    {
        printHelp();
        std::exit(0);
    }

    if (_show_version)
    {
        std::cout << FEP3_PARTICIPANT_LIBRARY_VERSION_STR << std::endl;
        std::exit(0);
    }
}

void CommandLineParserImpl::printHelp()
{
    std::cout << _cli << std::endl;
}

std::string CommandLineParserImpl::getParticipantName()
{
    return _participant_name;
}

std::string CommandLineParserImpl::getSystemName()
{
    return _system_name;
}

std::string CommandLineParserImpl::getServerAddressUrl()
{
    return _server_address_url;
}

std::unique_ptr<CommandLineParser> CommandLineParserFactory::create(clara::Parser cli, ParserDefaultValues default_values)
{
    return std::make_unique<CommandLineParserImpl>(cli, default_values);
}

} // namespace arya
} // namespace fep3