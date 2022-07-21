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

namespace fep3
{

    class ComponentVersionInfo {
    public:
        explicit ComponentVersionInfo(std::string version,
            std::string file_path,
            std::string participant_library_version)
            : _version(std::move(version))
            , _file_path(std::move(file_path))
            , _participant_library_version(std::move(participant_library_version))
        {
        }
        ComponentVersionInfo() = default;

        std::string getVersion() const { return _version; }
        std::string getFilePath() const { return _file_path; }
        std::string getParticipantLibraryVersion() const { return _participant_library_version; }

    private:
        std::string _version;
        std::string _file_path;
        std::string _participant_library_version;
    };
}// namespace fep3
