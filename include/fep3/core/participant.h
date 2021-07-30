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

#include <fep3/participant/participant.h>
#include <fep3/core/commandline_parser.h>

namespace fep3
{
namespace core
{
namespace arya
{
using fep3::arya::Participant;
using fep3::arya::createParticipant;

/**
* @brief Creates a participant and parses command line arguments
* @param[in] argc Command line argument count
* @param[in] argv Command line argument values
* @param[in] parser The parser that will be used to parse the command line arguments
* @param[in] version_info Version information of the participant to be created
* @param[in] factory factory instance which is able to create the element during load command
* @return Participant The created participant
*/
Participant createParticipant(int argc,
    char const *const *argv,
    const std::string& version_info,
    const std::shared_ptr<const IElementFactory>& factory,
    std::unique_ptr<CommandLineParser> parser);

/**
* @brief Creates a participant and parses command line arguments
* @tparam element_factory type of the fatory which is able to create the element
* @param[in] argc Command line argument count
* @param[in] argv Command line argument values
* @param[in] parser The parser that will be used to parse the command line arguments
* @param[in] version_info Version information of the participant to be created
* @return Participant The created participant
*/
template<typename element_factory>
Participant createParticipant(int argc,
    char const *const *argv,
    const std::string& version_info,
    std::unique_ptr<CommandLineParser> parser)
{
    const std::shared_ptr<const IElementFactory> factory = std::make_shared<element_factory>();
    return createParticipant(argc, argv, version_info, factory, std::move(parser));
}
/**
* @brief Creates a participant and parses command line arguments
* @param[in] argc Command line argument count
* @param[in] argv Command line argument values
* @param[in] version_info Version information of the participant to be created
* @param[in] factory factory instance which is able to create the element during load command
* @param[in] default_values Name of the participant,name of the server the participant belongs to
*        and the server address url if not specified by @p argv
* @return Participant The created participant
*/
Participant createParticipant(int argc,
    char const *const *argv,
    const std::string& version_info,
    const std::shared_ptr<const IElementFactory>& factory,
    const ParserDefaultValues& default_values = {});

/**
* @brief Creates a participant and parses command line arguments
* @tparam element_factory type of the fatory which is able to create the element
* @param[in] argc Command line argument count
* @param[in] argv Command line argument values
* @param[in] version_info Version information of the participant to be created
* @param[in] default_values Name of the participant,name of the server the participant belongs to
*        and the server address url if not specified by @p argv
* @return Participant The created participant
*/
template<typename element_factory>
Participant createParticipant(int argc,
    char const *const *argv,
    const std::string& version_info,
    const ParserDefaultValues& default_values = {})
{
    const std::shared_ptr<const IElementFactory> factory = std::make_shared<element_factory>();
    return createParticipant(argc, argv, version_info, factory, default_values);
}
} // namespace arya
using arya::Participant;
using arya::createParticipant;

} // namespace core
} // namespace fep3
