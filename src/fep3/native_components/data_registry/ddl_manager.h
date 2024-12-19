/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "fep3/fep3_errors.h"

#include <fep3/components/logging/easy_logger.h>

#include <ddl/dd/dd.h>

namespace fep3 {
namespace arya {
class DDLManager {
public:
    /// CTOR
    DDLManager();

    /**
     * @brief Loads a complete description from the string, replacing the internal description
     *
     * @param [in] ddl String containing a full DDL description
     *
     * @retval ERR_INVALID_ARG The description is invalid
     */
    fep3::Result loadDDL(const std::string& ddl, std::shared_ptr<fep3::arya::ILogger> _logger);

    /**
     * @brief Loads a complete description from the string, merging it into the internal description
     *
     * @param [in] ddl String containing a full DDL description
     *
     * @retval ERR_INVALID_ARG The description is invalid
     * @retval ERR_INVALID_TYPE A struct, enum or datatype is in conflict with its type definition
     */
    fep3::Result mergeDDL(const std::string& ddl, std::shared_ptr<fep3::arya::ILogger> _logger);

    /**
     * @brief Gets a complete but minimal DDL description that only contains the given type
     *
     * @param [in] type The name of a DDL struct
     * @param [out] description Destination string reference for the description
     *
     * @retval ERR_NOT_FOUND The type was not found in the description
     */
    fep3::Result resolveType(const std::string& type, std::string& description) const;

    /**
     * @brief Returns a const reference to the internal data definition object
     *
     * @return The current data definition
     */
    const ddl::dd::DataDefinition& getDDL() const noexcept;

private:
    /// The actual ddl description managed by this instance
    std::unique_ptr<ddl::dd::DataDefinition> _ddl;
};
} // namespace arya
using arya::DDLManager;
} // namespace fep3