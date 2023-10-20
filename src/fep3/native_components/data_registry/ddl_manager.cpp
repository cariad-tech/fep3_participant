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

#include "ddl_manager.h"

#include <a_util/strings/strings_functions.h>
#include <ddl/dd/dd_typeinfomodel.h>
#include <ddl/dd/ddstring.h>

namespace {

bool hasDynamicStructs(const ddl::dd::DataDefinition& data_definition) noexcept
{
    const ddl::dd::DataDefinition::StructTypes& struct_types = data_definition.getStructTypes();
    const auto it =
        std::find_if(struct_types.cbegin(), struct_types.cend(), [](const auto& struct_type) {
            // template is a disambiguator, see Paragraph 14.2/4 of the C++11 standard
            const auto type_info = struct_type.second->template getInfo<ddl::dd::TypeInfo>();
            return type_info && type_info->isDynamic();
        });
    return it != struct_types.cend();
}

} // namespace

using namespace fep3;

DDLManager::DDLManager() : _ddl{std::make_unique<ddl::dd::DataDefinition>()}
{
}

fep3::Result DDLManager::loadDDL(const std::string& ddl,
                                 std::shared_ptr<fep3::arya::ILogger> _logger)
{
    ddl::dd::DataDefinition data_definition;
    try {
        data_definition = ddl::DDString::fromXMLString(ddl);
    }
    catch (const ddl::dd::Error& e) {
        RETURN_ERROR_DESCRIPTION(
            ERR_INVALID_ARG,
            "%s Problems: %s",
            e.what(),
            a_util::strings::join(ddl::dd::transformProblemList(e.problems()), "\n").c_str());
    }
    try {
        if (hasDynamicStructs(data_definition)) {
            FEP3_ARYA_LOGGER_LOG_WARNING(
                _logger,
                "DDL: File contains dynamic arrays, DDL mapping for this structure will not be "
                "supported. Refer to the documentation for correct serialization of structures "
                "containing "
                "a dynamic array");
        }
        *_ddl = std::move(data_definition);
    }
    catch (const ddl::dd::Error& e) {
        RETURN_ERROR_DESCRIPTION(ERR_INVALID_ARG, e.what());
    }

    return {};
}

fep3::Result DDLManager::mergeDDL(const std::string& ddl,
                                  std::shared_ptr<fep3::arya::ILogger> _logger)
{
    ddl::dd::DataDefinition data_definition;
    try {
        data_definition = ddl::DDString::fromXMLString(ddl);
    }
    catch (const ddl::dd::Error& e) {
        RETURN_ERROR_DESCRIPTION(
            ERR_INVALID_ARG,
            "%s Problems: %s",
            e.what(),
            a_util::strings::join(ddl::dd::transformProblemList(e.problems()), "\n").c_str());
    }
    try {
        if (hasDynamicStructs(data_definition)) {
            FEP3_ARYA_LOGGER_LOG_WARNING(
                _logger,
                "DDL: File contains dynamic arrays, DDL mapping for this structure will not be "
                "supported. Refer to the documentation for correct serialization of structures "
                "containing "
                "a dynamic array");
        }
        _ddl->add(data_definition);
    }
    catch (const ddl::dd::Error& e) {
        RETURN_ERROR_DESCRIPTION(ERR_INVALID_ARG, e.what());
    }
    return {};
}

fep3::Result DDLManager::resolveType(const std::string& type, std::string& description) const
{
    // Make sure type is a struct because only them are supported in the sdk
    const auto& struct_types = _ddl->getStructTypes();
    if (!struct_types.contains(type)) {
        RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND,
                                 "%s%s%s",
                                 "Unable to find the struct type ",
                                 type.c_str(),
                                 " in the ddl");
    }
    description = ddl::DDString::toXMLString(type, *_ddl);
    return {};
}

const ddl::dd::DataDefinition& DDLManager::getDDL() const noexcept
{
    return *_ddl;
}