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

#include <fep3/core/participant.h>

namespace fep3 {
namespace core {

Participant createParticipant(int argc,
                              char const* const* argv,
                              const std::string& version_info,
                              const std::shared_ptr<const base::IElementFactory>& factory,
                              std::unique_ptr<CommandLineParser> parser)
{
    parser->parseArgs(argc, argv);

    return createParticipant(parser->getParticipantName(),
                             version_info,
                             parser->getSystemName(),
                             factory,
                             parser->getServerAddressUrl());
}

Participant createParticipant(int argc,
                              char const* const* argv,
                              const std::string& version_info,
                              const std::shared_ptr<const base::IElementFactory>& factory,
                              const core::ParserDefaultValues& default_values)
{
    return createParticipant(argc,
                             argv,
                             version_info,
                             factory,
                             CommandLineParserFactory::create(clipp::group{}, default_values));
}

} // namespace core
} // namespace fep3
