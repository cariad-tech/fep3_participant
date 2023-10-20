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

#include "logging_config.h"

#include <a_util/strings.h>

using namespace fep3;
using namespace fep3::native;

LoggingFilterTree::Node::Node(std::vector<std::string>& name,
                              const LoggerFilterInternal& filter,
                              const Node& parent)
{
    if (name.empty()) {
        _filter = filter;
    }
    else {
        // This is not the final node so we use the parent's filter
        _filter = parent._filter;

        std::string last_name = name.back();
        name.pop_back();
        _child_nodes.emplace(last_name, Node(name, filter, *this));
    }
}

void LoggingFilterTree::Node::setLoggerFilter(std::vector<std::string>& name,
                                              const LoggerFilterInternal& filter,
                                              bool overwrite)
{
    if (name.empty()) {
        _filter = filter;

        if (overwrite) {
            for (auto& node: _child_nodes) {
                node.second.setLoggerFilter(name, filter);
            }
        }
    }
    else {
        std::string last_name = name.back();
        name.pop_back();
        auto it = _child_nodes.find(last_name);
        if (it == _child_nodes.end()) {
            _child_nodes.emplace(last_name, Node(name, filter, *this));
        }
        else {
            it->second.setLoggerFilter(name, filter, overwrite);
        }
    }
}

const LoggerFilterInternal& LoggingFilterTree::Node::getLoggerFilter(
    std::vector<std::string>& name) const
{
    if (name.empty()) {
        return _filter;
    }
    else {
        std::string last_name = name.back();
        auto it = _child_nodes.find(last_name);
        if (it == _child_nodes.end()) {
            return _filter;
        }
        else {
            name.pop_back();
            return it->second.getLoggerFilter(name);
        }
    }
}

LoggingFilterTree::LoggingFilterTree()
{
    // This is just to initialize the unique pointer. The default filter is set in the
    // LoggingService constructor.
    LoggerFilterInternal init_root_filter = {LoggerSeverity::off, {}};
    _root_node = std::make_unique<Node>(init_root_filter);
}

void LoggingFilterTree::setLoggerFilter(const std::string& logger_name,
                                        const LoggerFilterInternal& filter,
                                        bool overwrite)
{
    std::vector<std::string> name_parts = a_util::strings::split(logger_name, ".");
    _root_node->setLoggerFilter(name_parts, filter, overwrite);
}

const LoggerFilterInternal& LoggingFilterTree::getLoggerFilter(const std::string& logger_name) const
{
    std::vector<std::string> name_parts = a_util::strings::split(logger_name, ".");
    return _root_node->getLoggerFilter(name_parts);
}
