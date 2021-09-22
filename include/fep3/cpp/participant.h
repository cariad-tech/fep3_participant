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

#include <string>
#include <memory>

#include <fep3/fep3_participant_version.h>
#include <fep3/fep3_participant_export.h>
#include <fep3/fep3_errors.h>

#include "../core/participant.h"
#include "../core/element_factory.h"

namespace fep3
{
namespace cpp
{
namespace arya
{
using fep3::core::arya::Participant;

/**
 * @brief Creates a special particpant without a named system access (it will be part of the "default_system")
 * @param[in] name Name of the participant to be created
 * @tparam element_type type of the element to create
 * @return The created participant
*/
template<typename element_type>
Participant createParticipant(const std::string& name)
{
    auto elem_factory = std::make_shared<fep3::core::arya::ElementFactory<element_type>>();
    return fep3::core::arya::createParticipant(name,
        FEP3_PARTICIPANT_LIBRARY_VERSION_STR,
        std::string(),
        elem_factory);
}
/**
 * @brief Creates a special particpant
 * @param[in] name Name of the participant to be created
 * @param[in] system_name Name of the system the participant belongs to
 * @tparam element_type type of the element to create
 * @return The created participant
*/
template<typename element_type>
Participant createParticipant(const std::string& name,
                              const std::string& system_name)
{
    auto elem_factory = std::make_shared<fep3::core::arya::ElementFactory<element_type>>();
    return fep3::core::arya::createParticipant(name,
        FEP3_PARTICIPANT_LIBRARY_VERSION_STR,
        system_name,
        elem_factory);
}

/**
 * @brief Creates a special participant with commandline parsing
 * @param[in] argc Command line argument count
 * @param[in] argv Command line argument values
 * @param[in] default_name Name of the participant if not specified by @p argv
 * @param[in] default_system_name Name of the system the participant belongs to if not specified by @p argv
 * @tparam element_type type of the element to create
 * @return The created participant
 */
template<typename element_type>
Participant createParticipant(int argc,
                              char const *const *argv,
                              const std::string& default_name,
                              const std::string& default_system_name)
{
    auto elem_factory = std::make_shared<fep3::core::arya::ElementFactory<element_type>>();
    return fep3::core::arya::createParticipant(argc,
        argv,
        FEP3_PARTICIPANT_LIBRARY_VERSION_STR,
        elem_factory,
        ParserDefaultValues{default_name, default_system_name, ""});
}

/**
 * @brief Creates a special participant with commandline parsing
 * @param[in] argc Command line argument count
 * @param[in] argv Command line argument values
 * @param[in] parser A parser that can be extended with user defined command line arguments beforehand
 * @tparam element_type type of the element to create
 * @return The created participant
*/
template<typename element_type>
Participant createParticipant(int argc,
    char const *const *argv,
    std::unique_ptr<CommandLineParser> parser)
{
    auto elem_factory = std::make_shared<fep3::core::arya::ElementFactory<element_type>>();
    return fep3::core::arya::createParticipant(argc,
        argv,
        FEP3_PARTICIPANT_LIBRARY_VERSION_STR,
        elem_factory,
        std::move(parser));
}

} // namespace arya
using arya::Participant;
using arya::createParticipant;
} // namespace cpp
} // namespace fep3
