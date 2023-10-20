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

#include <string>

namespace fep3 {
/**
 * @brief Holds metadata of a loaded compoent
 */
class ComponentVersionInfo {
public:
    /**
     * @brief Constructor.
     *
     * @param version component version
     * @param file_path the path of the plugin file.
     * @param participant_library_version The participant library version that the component was
     * compiled with.
     */
    explicit ComponentVersionInfo(std::string version,
                                  std::string file_path,
                                  std::string participant_library_version)
        : _version(std::move(version)),
          _file_path(std::move(file_path)),
          _participant_library_version(std::move(participant_library_version))
    {
    }

    /**
     * @brief Constructor.
     */

    ComponentVersionInfo() = default;
    /**
     * @brief Returns the component version
     *
     * @return The component version
     */

    std::string getVersion() const
    {
        return _version;
    }

    /**
     * @brief Returns the file path of the plugin file that the component was loaded from.
     *
     * @return The path of the plugin file.
     */
    std::string getFilePath() const
    {
        return _file_path;
    }

    /**
     * @brief The participant library version that the component was compiled with.
     *
     * @return The participant library version
     */
    std::string getParticipantLibraryVersion() const
    {
        return _participant_library_version;
    }

private:
    /// @cond nodoc
    std::string _version;
    std::string _file_path;
    std::string _participant_library_version;
    ///@endcond nodoc
};
} // namespace fep3
