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

#include <fep3/components/logging/logging_service_intf.h>

#include <map>

namespace fep3 {
namespace native {

struct LoggerFilterInternal {
    LoggerSeverity _severity;
    std::map<std::string, std::shared_ptr<ILoggingService::ILoggingSink>> _logging_sinks;
};

class LoggingFilterTree {
private:
    class Node {
    public:
        Node(const LoggerFilterInternal& filter) : _filter(filter)
        {
        }

        Node(std::vector<std::string>& name,
             const LoggerFilterInternal& filter,
             const Node& parent);

        void setLoggerFilter(std::vector<std::string>& name,
                             const LoggerFilterInternal& filter,
                             bool overwrite = true);

        const LoggerFilterInternal& getLoggerFilter(std::vector<std::string>& name) const;

    private:
        LoggerFilterInternal _filter;
        std::map<std::string, Node> _child_nodes;
    };

public:
    LoggingFilterTree();

    /**
     * @brief Sets the filter for a logger domain.
     *
     * This will also overwrite all already existing filters with a lower hierarchy level than this
     * domain. The overwrite behaviour can be disabled with the overwrite parameter.
     *
     * @param [in] logger_name Name of the logger domain which filter shall be set.
     * @param [in] filter      The settings of the filter.
     * @param [in] overwrite   Turns the overwrite mechanic on or off. Default is on.
     */
    void setLoggerFilter(const std::string& logger_name,
                         const LoggerFilterInternal& filter,
                         bool overwrite = true);

    /**
     * @brief Returns a reference to the logging filter set for the logger name.
     *
     * If no specific filter has been set, it will return the filter of the next higher hierarchy
     * level and so on. If there is no filter for any level it will return the default filter of the
     * configuration.
     *
     * @param[in] logger_name The logger domain name for which filter will be returned
     *
     * @return A reference to a logging filter.
     */
    const LoggerFilterInternal& getLoggerFilter(const std::string& logger_name) const;

private:
    /// The root node holds the default filter and has no name
    std::unique_ptr<Node> _root_node;
};
} // namespace native
} // namespace fep3
