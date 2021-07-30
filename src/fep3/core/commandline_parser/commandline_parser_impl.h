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


#pragma once

#include <fep3/core/commandline_parser.h>

namespace fep3
{
namespace arya
{
class CommandLineParserImpl : public CommandLineParser
{
public:
    CommandLineParserImpl(clara::Parser cli, const ParserDefaultValues& default_values);

    virtual void parseArgs(int argc, char const *const *argv);
    void printHelp();

    virtual std::string getParticipantName() override;
    virtual std::string getSystemName() override;
    virtual std::string getServerAddressUrl() override;

private:
    clara::Parser _cli;
    bool _show_help{ false };
    bool _show_version{ false };
    std::string _participant_name;
    std::string _system_name;
    std::string _server_address_url;
};
} // namespace arya
} // namespace fep3