/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "result_printer.h"

#include <boost/algorithm/string/join.hpp>

#include <iostream>

namespace {
void printProps(const std::vector<fep3::base::PropertyInfo>& props);
void printSignalInfo(const std::vector<fep3::base::SignalInfo>& signal_infos);
void printSignalProperties(const std::vector<fep3::base::StreamTypeProperty>& properties);
std::string getDefaultPropertyValue(const fep3::base::PropertyInfo& property_info);
template <typename T>
std::vector<std::string> formatIIDs(const T& t);
} // namespace

namespace fep3::base {

void print(const std::vector<ComponentMetaModelInfo>& meta_info)
{
    std::cout << "\n\nAnalysis Result\n\n\n";

    std::cout << "components:\n";
    for (auto& comp: meta_info) {
        std::cout << "  - iid:\n";
        auto iids = boost::algorithm::join(formatIIDs(comp._iids), "\n");
        std::cout << iids;
        std::cout << "\n";

        std::cout << "    type: cpp-plugin\n";
        std::cout << "    path: " << comp._path << "\n";

        std::cout << "    required_components:\n";
        std::string dep_iids = boost::algorithm::join(formatIIDs(comp._required_components), "\n");
        std::cout << dep_iids;
        std::cout << "\n";

        if (!comp._property_info.empty()) {
            std::cout << "    configure:\n" << std::flush;
            printProps(comp._property_info);
        }

        if (!comp._signal_infos.empty()) {
            std::cout << "    signals:\n";
            auto input_signals = comp.getSignalsWithDirection(SignalFlow::input);
            if (!input_signals.empty()) {
                std::cout << "      inputs:\n";
                printSignalInfo(input_signals);
            }
            auto output_signals = comp.getSignalsWithDirection(SignalFlow::output);
            if (!output_signals.empty()) {
                std::cout << "      outputs:\n";
                printSignalInfo(output_signals);
            }
        }
    }
}

} // namespace fep3::base

namespace {
template <typename ContainerType>
std::vector<std::string> formatIIDs(const ContainerType& t)
{
    std::vector<std::string> formatted_strings;
    std::transform(t.begin(),
                   t.end(),
                   std::back_inserter(formatted_strings),
                   [](const std::string& s) { return "      - " + s; });

    return formatted_strings;
}

void printProps(const std::vector<fep3::base::PropertyInfo>& props)
{
    for (const auto& prop: props) {
        std::cout << "      " << prop._path << ":\n";
        std::cout << "        description: \"\"\n";
        std::cout << "        type: " << prop._type << "\n";
        std::cout << "        available_in_state: " << prop._available_in_state << "\n";
        std::cout << "        default_value: " << getDefaultPropertyValue(prop) << "\n"
                  << std::flush;
    }
}

std::string getDefaultPropertyValue(const fep3::base::PropertyInfo& property_info)
{
    if ((property_info._type.find("array") != std::string::npos) &&
        ((property_info._default_value.empty() || (property_info._default_value == "\"\"")))) {
        return "[]";
    }
    else {
        return property_info._default_value;
    }
}

void printSignalInfo(const std::vector<fep3::base::SignalInfo>& signal_infos)
{
    for (const auto& signal_info: signal_infos) {
        std::cout << "        " << signal_info._name << ":\n";
        std::cout << "          stream_type:\n";
        std::cout << "            stream_meta_type: " << signal_info._stream_meta_type << "\n";
        std::cout << "            stream_type_properties:\n";
        printSignalProperties(signal_info._properties);
    }
}

void printSignalProperties(const std::vector<fep3::base::StreamTypeProperty>& properties)
{
    for (const auto& property: properties) {
        std::cout << "              " << property._name << ": "
                  << "'" << property._value << "'"
                  << "\n";
    }
}
} // namespace
