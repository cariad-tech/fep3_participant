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

#include <clipp.h>

namespace fep3 {
namespace core {
/// Struct that holds the default values for the parser
struct ParserDefaultValues {
    /// The default name of the participant
    std::string participant_name;
    /// The default system the participant is a part of
    std::string system_name;
    /// A specific default server address url
    std::string server_address_url;
};

/**
 * @brief Class for a command line parser that can be passed to a createParticipant function
 *
 * The following natively supported command line arguments are already defined:
 *   -?, -h, --help           display usage information
 *   -v, --version            Print FEP SDK version.
 *   -n, --name \<string\>      Set participant name.
 *   -s, --system \<string\>    Set the system of the participant.
 *   -u, --url \<string\>       Set the server address url
 */
class CommandLineParser {
public:
    /**
     * @brief Method to parse command line arguments
     * exits after printing message for erroneous parameters, help or version request
     *
     * @param[in] argc number of command line arguments (including executable name)
     * @param[in] argv command line arguments in C strings (including executable name)
     */
    virtual void parseArgs(int argc, char const* const* argv) = 0;

    /**
     * @brief Method to fetch participant name from arguments
     *
     * @return participant name
     */
    virtual std::string getParticipantName() = 0;

    /**
     * @brief Method to fetch system name from arguments
     *
     * @return system name
     */
    virtual std::string getSystemName() = 0;

    /**
     * @brief Method to fetch server address URL name from arguments
     *
     * @return server address URL
     */
    virtual std::string getServerAddressUrl() = 0;

    /// DTOR
    virtual ~CommandLineParser() = 0;
};
/**
 * @brief Factory class for the command line parser
 */
class CommandLineParserFactory {
public:
    /**
     * @brief Factory function to create a command line parser
     *
     * @param[in] cli Used to add user defined command line arguments to the command line parser
     * @param[in] default_values Default values for the natively supported command line arguments
     *
     * @return A smart pointer to a fully initialized parser that can be passed to a
     * createParticipant function
     */
    static std::unique_ptr<CommandLineParser> create(clipp::group cli,
                                                     ParserDefaultValues default_values = {});

    /**
     * @brief Factory function to create a command line parser
     *
     * @param[in] cli Used to add user defined command line arguments to the command line parser
     * @param[in] default_values Default values for the natively supported command line arguments
     *
     * @return A smart pointer to a fully initialized parser that can be passed to a
     * createParticipant function
     */
    static std::unique_ptr<CommandLineParser> create(clipp::parameter cli,
                                                     ParserDefaultValues default_values = {});
};
} // namespace core

namespace arya {
/// @cond nodoc
using CommandLineParserFactory
    [[deprecated("Since 3.1, fep3::arya::CommandLineParserFactory is deprecated. Please use "
                 "fep3::core::CommandLineParserFactory")]] = fep3::core::CommandLineParserFactory;

using CommandLineParser [[deprecated("Since 3.1, fep3::arya::CommandLineParser is deprecated. "
                                     "Please use fep3::core::CommandLineParser")]] =
    fep3::core::CommandLineParser;

using ParserDefaultValues [[deprecated("Since 3.1, fep3::arya::ParserDefaultValues is deprecated. "
                                       "Please use fep3::core::ParserDefaultValues")]] =
    fep3::core::ParserDefaultValues;

/// @endcond nodoc
} // namespace arya
} // namespace fep3
