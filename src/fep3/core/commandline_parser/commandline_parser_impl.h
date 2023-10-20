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

#pragma once

#include <fep3/core/commandline_parser.h>

namespace fep3 {
namespace core {
struct ParseToolAdapterIntf;

class CommandLineParserImpl : public CommandLineParser {
public:
    template <typename T>
    CommandLineParserImpl(T parser, const core::ParserDefaultValues& default_values);

    virtual void parseArgs(int argc, char const* const* argv);
    void printHelp();

    virtual std::string getParticipantName() override;
    virtual std::string getSystemName() override;
    virtual std::string getServerAddressUrl() override;

private:
    bool _show_help{false};
    bool _show_version{false};
    std::string _participant_name;
    std::string _system_name;
    std::string _server_address_url;
    std::unique_ptr<ParseToolAdapterIntf> _cli_adapter;
};
} // namespace core
} // namespace fep3
