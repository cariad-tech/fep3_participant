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

#include "commandline_parser_impl.h"

#include <fep3/fep3_participant_version.h>

#include <iostream>

namespace fep3 {
namespace core {

struct ParseToolAdapterIntf {
    virtual ~ParseToolAdapterIntf() = default;
    virtual bool parseArgs(int argc, char const* const* argv) = 0;
    virtual void printHelp() const = 0;
    virtual void printError() const = 0;
};

struct ParseToolAdapterClipp final : ParseToolAdapterIntf {
    template <typename T>
    ParseToolAdapterClipp(T& cli,
                          bool& show_help,
                          bool& show_version,
                          std::string& participant_name,
                          std::string& system_name,
                          std::string& server_address_url)
    {
        _cli =
            (cli,
             clipp::option("-v", "--version").set(show_version).doc("Print FEP SDK version."),
             (clipp::option("-n", "--name", "--element_name") &
              clipp::value("participant_name", participant_name)) %
                 ("Set participant name."),
             (clipp::option("-s", "--system", "--system_name") &
              clipp::value("system_name", system_name)) %
                 ("Set the system of the participant."),
             (clipp::option("-u", "--url") &
              clipp::value("server_address_url", server_address_url)) %
                 ("the server address url"),
             (clipp::option("-?", "-h", "--help").set(show_help).doc("display usage information")));
    }

    bool parseArgs(int argc, char const* const* argv)
    {
        _last_result = clipp::parse(argc, const_cast<char**>(argv), _cli);
        return static_cast<bool>(_last_result);
    };

    void printHelp() const
    {
        std::cout << clipp::make_man_page(_cli, "Participant");
    }
    void printError() const
    {
        clipp::debug::print(std::cerr, _last_result);
    }

private:
    clipp::group _cli;
    clipp::parsing_result _last_result;
};

// helper for preventing any other instantiation
template <class>
inline constexpr bool always_false_v = false;

template <typename T, typename... Args>
std::unique_ptr<ParseToolAdapterIntf> createParseToolAdapterIntf(T& cli, Args&&... args)
{
    if constexpr (std::is_same_v<clipp::group, T> || std::is_same_v<clipp::parameter, T>) {
        return std::make_unique<ParseToolAdapterClipp>(cli, std::forward<Args>(args)...);
    }
    else
        static_assert(always_false_v<T>, "not supported template parameter");
}

template <typename T>
CommandLineParserImpl::CommandLineParserImpl(T parser,
                                             const core::ParserDefaultValues& default_values)
    : _participant_name(default_values.participant_name),
      _system_name(default_values.system_name),
      _server_address_url(default_values.server_address_url),
      _cli_adapter(createParseToolAdapterIntf(
          parser, _show_help, _show_version, _participant_name, _system_name, _server_address_url))
{
}

void CommandLineParserImpl::parseArgs(int argc, char const* const* argv)
{
    const bool result = _cli_adapter->parseArgs(argc, argv);

    if (!result) {
        std::cerr << "Error in command line: ";
        _cli_adapter->printError();
        std::cerr << std::endl << std::endl;
        ;
        printHelp();
        std::exit(1);
    }

    // Make participant name argument mandatory if no default value has been given
    if (_participant_name.empty()) {
        std::cerr << "Error in command line: Missing required argument: participant name"
                  << std::endl
                  << std::endl;
        printHelp();
        std::exit(1);
    }

    // Make system name argument mandatory if no default value has been given
    if (_system_name.empty()) {
        std::cerr << "Error in command line: Missing required argument: system name" << std::endl
                  << std::endl;
        printHelp();
        std::exit(1);
    }

    if (_show_help) {
        printHelp();
        std::exit(0);
    }

    if (_show_version) {
        std::cout << FEP3_PARTICIPANT_LIBRARY_VERSION_STR << std::endl;
        std::exit(0);
    }
}

void CommandLineParserImpl::printHelp()
{
    _cli_adapter->printHelp();
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

std::unique_ptr<CommandLineParser> CommandLineParserFactory::create(
    clipp::group cli, ParserDefaultValues default_values)
{
    return std::make_unique<CommandLineParserImpl>(cli, default_values);
}

std::unique_ptr<CommandLineParser> CommandLineParserFactory::create(
    clipp::parameter cli, ParserDefaultValues default_values)
{
    return std::make_unique<CommandLineParserImpl>(cli, default_values);
}

} // namespace core
} // namespace fep3