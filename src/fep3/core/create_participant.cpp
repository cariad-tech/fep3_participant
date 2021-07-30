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

#include <fep3/core/participant.h>

namespace fep3
{
namespace core
{
namespace arya
{

Participant createParticipant(int argc,
    char const *const *argv,
    const std::string& version_info,
    const std::shared_ptr<const IElementFactory>& factory,
    std::unique_ptr<CommandLineParser> parser)
{
    parser->parseArgs(argc, argv);

    return createParticipant(
        parser->getParticipantName(),
        version_info,
        parser->getSystemName(),
        factory,
        parser->getServerAddressUrl()
    );
}

Participant createParticipant(int argc,
    char const *const *argv,
    const std::string& version_info,
    const std::shared_ptr<const IElementFactory>& factory,
    const ParserDefaultValues& default_values)
{
    return createParticipant(
        argc,
        argv,
        version_info,
        factory,
        CommandLineParserFactory::create(clara::Parser{}, default_values)
    );
}

} //arya
} // namespace core
} // namespace fep3
