/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <fep3/base/properties/propertynode.h>

namespace fep3 {
namespace native {
namespace arya {
/**
 * Class providing alias names for input and output signals
 * based on the property FEP3_SIGNAL_RENAMING_INPUT_CONFIGURATION_PROPERTY
 * and FEP3_SIGNAL_RENAMING_OUTPUT_CONFIGURATION_PROPERTY
 */
class DataSignalRenaming {
public:
    DataSignalRenaming() = default;
    DataSignalRenaming(DataSignalRenaming&&) = delete;
    DataSignalRenaming(const DataSignalRenaming&) = delete;
    DataSignalRenaming& operator=(DataSignalRenaming&&) = delete;
    DataSignalRenaming& operator=(const DataSignalRenaming&) = delete;

    virtual ~DataSignalRenaming() = default;

    fep3::Result registerPropertyVariables(base::Configuration&);
    fep3::Result unregisterPropertyVariables(base::Configuration&);

    std::string getAliasInputName(const std::string&) const;
    std::string getAliasOutputName(const std::string&) const;

    fep3::Result parseProperties();
    static fep3::Result checkName(const std::string& name); // used at renaming

private:
    base::PropertyVariable<std::string> _renaming_input{""};
    base::PropertyVariable<std::string> _renaming_output{""};

    std::map<std::string, std::string> _map_renaming_input;
    std::map<std::string, std::string> _map_renaming_output;
};
} // namespace arya
using arya::DataSignalRenaming;
} // namespace native
} // namespace fep3
