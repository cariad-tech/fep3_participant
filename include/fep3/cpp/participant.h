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

#include <fep3/core/element_factory.h>
#include <fep3/core/participant.h>
#include <fep3/fep3_participant_version.h>

namespace fep3 {
namespace cpp {

using fep3::core::Participant;

/**
 * @brief Creates a special particpant without a named system access (it will be part of the
 * "default_system")
 * @param[in] name Name of the participant to be created
 * @tparam element_type type of the element to create
 * @return The created participant
 */
template <typename element_type>
Participant createParticipant(const std::string& name)
{
    auto elem_factory = std::make_shared<fep3::core::ElementFactory<element_type>>();
    return fep3::core::createParticipant(
        name, FEP3_PARTICIPANT_LIBRARY_VERSION_STR, std::string(), elem_factory);
}

/**
 * @brief Creates a special particpant
 * @param[in] name Name of the participant to be created
 * @param[in] system_name Name of the system the participant belongs to
 * @tparam element_type type of the element to create
 * @return The created participant
 */
template <typename element_type>
Participant createParticipant(const std::string& name, const std::string& system_name)
{
    auto elem_factory = std::make_shared<fep3::core::ElementFactory<element_type>>();
    return fep3::core::createParticipant(
        name, FEP3_PARTICIPANT_LIBRARY_VERSION_STR, system_name, elem_factory);
}

/**
 * @brief Creates a special participant with commandline parsing
 * @param[in] argc Command line argument count
 * @param[in] argv Command line argument values
 * @param[in] default_name Name of the participant if not specified by @p argv
 * @param[in] default_system_name Name of the system the participant belongs to if not specified by
 * @p argv
 * @tparam element_type type of the element to create
 * @return The created participant
 */
template <typename element_type>
Participant createParticipant(int argc,
                              char const* const* argv,
                              const std::string& default_name,
                              const std::string& default_system_name)
{
    auto elem_factory = std::make_shared<fep3::core::ElementFactory<element_type>>();
    return fep3::core::createParticipant(
        argc,
        argv,
        FEP3_PARTICIPANT_LIBRARY_VERSION_STR,
        elem_factory,
        core::ParserDefaultValues{default_name, default_system_name, ""});
}

/**
 * @brief Creates a special participant with commandline parsing
 * @param[in] argc Command line argument count
 * @param[in] argv Command line argument values
 * @param[in] parser A parser that can be extended with user defined command line arguments
 * beforehand
 * @tparam element_type type of the element to create
 * @return The created participant
 */
template <typename element_type>
Participant createParticipant(int argc,
                              char const* const* argv,
                              std::unique_ptr<core::CommandLineParser> parser)
{
    auto elem_factory = std::make_shared<fep3::core::ElementFactory<element_type>>();
    return fep3::core::createParticipant(
        argc, argv, FEP3_PARTICIPANT_LIBRARY_VERSION_STR, elem_factory, std::move(parser));
}

/// @cond nodoc
namespace arya {
template <typename... Args>
[[deprecated("Since 3.1, fep3::cpp::arya::createParticipant is deprecated. Please use "
             "fep3::cpp::createParticipant")]] auto
createParticipant(Args&&... args)
    -> decltype(fep3::cpp::createParticipant(std::forward<Args>(args)...))
{
    return fep3::cpp::createParticipant(std::forward<Args>(args)...);
}

} // namespace arya
/// @endcond nodoc

} // namespace cpp
} // namespace fep3
