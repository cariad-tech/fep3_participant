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

#include "data_signal_renaming.h"

#include <fep3/components/data_registry/data_registry_intf.h>

#include <algorithm>

using namespace fep3;
using namespace native;
using namespace native::arya;

Result parse_string_to_map(const std::string& lines, std::map<std::string, std::string>& map)
{
    int line_num = 0;
    for (const auto line : a_util::strings::split(lines, ","))
    {
        auto name_value = a_util::strings::split(line, ":");
        if (name_value.size() != 2)
        {
            return CREATE_ERROR_DESCRIPTION(ERR_INVALID_ARG
                ,"Line '%d' ('%s') doesn't contain a ':' separated key value pair 'original_name:renamed_name': '%s'"
                ,line_num
                ,line.c_str()
                ,lines.c_str());
        }

        const auto key = name_value[0];
        const auto value = name_value[1];
        if(map.find(key) != map.end())
        {
            return CREATE_ERROR_DESCRIPTION(ERR_INVALID_ARG
                , "The key '%s' is not unique. It's not possible to map '%s' to '%s' and '%s'"
                , key.c_str()
                , key.c_str()
                , map[key].c_str()
                , value.c_str());
        }
        else
        {
			FEP3_RETURN_IF_FAILED(DataSignalRenaming::checkName(value));
			map[key] = value;
        }
        
        line_num++;
    }

    return {};
}

Result DataSignalRenaming::registerPropertyVariables(base::Configuration& configuration)
{
    FEP3_RETURN_IF_FAILED(configuration.registerPropertyVariable(_renaming_input, FEP3_SIGNAL_RENAMING_INPUT_CONFIGURATION_PROPERTY));
    FEP3_RETURN_IF_FAILED(configuration.registerPropertyVariable(_renaming_output, FEP3_SIGNAL_RENAMING_OUTPUT_CONFIGURATION_PROPERTY));
    return {};
}

Result DataSignalRenaming::unregisterPropertyVariables(base::Configuration& configuration)
{
    FEP3_RETURN_IF_FAILED(configuration.unregisterPropertyVariable(_renaming_input, FEP3_SIGNAL_RENAMING_INPUT_CONFIGURATION_PROPERTY));
    FEP3_RETURN_IF_FAILED(configuration.unregisterPropertyVariable(_renaming_output, FEP3_SIGNAL_RENAMING_OUTPUT_CONFIGURATION_PROPERTY));
    return {};
}

std::string DataSignalRenaming::getAliasInputName(const std::string& name) const
{
    if (_map_renaming_input.count(name) > 0)
    {
        return _map_renaming_input.at(name);
    }
    return name;
}

std::string DataSignalRenaming::getAliasOutputName(const std::string& name) const
{
    if (_map_renaming_output.count(name) > 0)
    {
        return _map_renaming_output.at(name);
    }
    return name;
}

Result DataSignalRenaming::parseProperties()
{
    _map_renaming_input.clear();
    _map_renaming_output.clear();

    FEP3_RETURN_IF_FAILED(parse_string_to_map(static_cast<std::string>(_renaming_input), _map_renaming_input));
    FEP3_RETURN_IF_FAILED(parse_string_to_map(static_cast<std::string>(_renaming_output), _map_renaming_output));
    return {};
}

Result DataSignalRenaming::checkName(const std::string& name)
{
	if (!std::regex_match(name, std::regex("^[A-Za-z0-9_]+$")))
	{
		RETURN_ERROR_DESCRIPTION(ERR_NOT_SUPPORTED, "Signal name '%s' is not supported. Use alphanumeric characters and underscore only!", name.c_str());
	}
	return {};
}