/**
 * @file
 * @copyright
 * @verbatim
Copyright 2023 CARIAD SE.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
@endverbatim
 */

#pragma once

#include <fep3/core/commandline_parser.h>
#include <fep3/participant/participant.h>

namespace fep3 {
namespace core {

using fep3::base::createParticipant;
using fep3::base::Participant;

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
                              char const* const* argv,
                              const std::string& version_info,
                              const std::shared_ptr<const base::IElementFactory>& factory,
                              std::unique_ptr<CommandLineParser> parser);

/**
 * @brief Creates a participant and parses command line arguments
 * @tparam element_factory type of the factory which is able to create the element
 * @param[in] argc Command line argument count
 * @param[in] argv Command line argument values
 * @param[in] parser The parser that will be used to parse the command line arguments
 * @param[in] version_info Version information of the participant to be created
 * @return Participant The created participant
 */
template <typename element_factory>
Participant createParticipant(int argc,
                              char const* const* argv,
                              const std::string& version_info,
                              std::unique_ptr<CommandLineParser> parser)
{
    const std::shared_ptr<const base::IElementFactory> factory =
        std::make_shared<element_factory>();
    return createParticipant(argc, argv, version_info, factory, std::move(parser));
}

/**
 * @brief Creates a participant and parses command line arguments
 * @param[in] argc Command line argument count
 * @param[in] argv Command line argument values
 * @param[in] version_info Version information of the participant to be created
 * @param[in] factory factory instance which is able to create the element during load command
 * @param[in] default_values Name of the participant, name of the server the participant belongs to
 *        and the server address url if not specified by @p argv
 * @return Participant The created participant
 */
Participant createParticipant(int argc,
                              char const* const* argv,
                              const std::string& version_info,
                              const std::shared_ptr<const base::IElementFactory>& factory,
                              const ParserDefaultValues& default_values = {});

/**
 * @brief Creates a participant and parses command line arguments
 * @tparam element_factory type of the fatory which is able to create the element
 * @param[in] argc Command line argument count
 * @param[in] argv Command line argument values
 * @param[in] version_info Version information of the participant to be created
 * @param[in] default_values Name of the participant, name of the server the participant belongs to
 *        and the server address url if not specified by @p argv
 * @return Participant The created participant
 */
template <typename element_factory>
Participant createParticipant(int argc,
                              char const* const* argv,
                              const std::string& version_info,
                              const ParserDefaultValues& default_values = {})
{
    const std::shared_ptr<const base::IElementFactory> factory =
        std::make_shared<element_factory>();
    return createParticipant(argc, argv, version_info, factory, default_values);
}

/// @cond nodoc
namespace arya {
template <typename... Args>
[[deprecated("Since 3.1, fep3::core::arya::createParticipant is deprecated. Please use "
             "fep3::core::createParticipant")]] auto
createParticipant(Args&&... args)
    -> decltype(fep3::core::createParticipant(std::forward<Args>(args)...))
{
    return fep3::core::createParticipant(std::forward<Args>(args)...);
}

} // namespace arya
/// @endcond nodoc

} // namespace core
} // namespace fep3
