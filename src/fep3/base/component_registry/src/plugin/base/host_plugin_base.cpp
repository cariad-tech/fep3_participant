/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "../../../include/plugin/base/host_plugin_base.h"

#include <sstream>

namespace fep3 {
namespace plugin {

ParticipantLibraryVersion::ParticipantLibraryVersion(
    const std::string& id, int32_t major, int32_t minor, int32_t patch, int32_t build)
    : _id(id), _major(major), _minor(minor), _patch(patch), _build(build)
{
}

ParticipantLibraryVersion::ParticipantLibraryVersion(
    const fep3_plugin_base_ParticipantLibraryVersion& participant_library_version)
    : _id(participant_library_version._id),
      _major(participant_library_version._major),
      _minor(participant_library_version._minor),
      _patch(participant_library_version._patch),
      _build(participant_library_version._build)
{
}

bool ParticipantLibraryVersion::operator==(const ParticipantLibraryVersion& other) const
{
    return _id == other._id && _major == other._major && _minor == other._minor &&
           _patch == other._patch && _build == other._build;
}

std::string ParticipantLibraryVersion::toString() const
{
    std::stringstream version_stream;
    version_stream << _id << " @ " << _major << "." << _minor << "." << _patch << " build "
                   << _build;
    return version_stream.str();
}

HostPluginBase::HostPluginBase(const std::string& file_path, bool prevent_unloading)
    //: boost::dll::shared_library(file_path, boost::dll::load_mode::append_decorations)
    : SharedLibrary(file_path, prevent_unloading)
{
    auto get_plugin_version_function =
        get<void(void(void*, const char*), void*)>(SYMBOL_fep3_plugin_getPluginVersion);
    if (!get_plugin_version_function) {
        throw std::runtime_error("The plugin '" + file_path +
                                 "' does not provide an appropriate '" +
                                 SYMBOL_fep3_plugin_getPluginVersion + "' function.");
    }
    get_plugin_version_function(
        [](void* destination, const char* plugin_version) {
            *static_cast<decltype(_plugin_version)*>(destination) = plugin_version;
        },
        static_cast<void*>(&_plugin_version));
}

HostPluginBase::~HostPluginBase()
{
}

std::string HostPluginBase::getPluginVersion() const
{
    return _plugin_version;
}

ParticipantLibraryVersion HostPluginBase::getParticipantLibraryVersion() const
{
    return _participant_library_version;
}

ComponentVersionInfo HostPluginBase::getPluginInfo() const
{
    return ComponentVersionInfo(
        getPluginVersion(), getFilePath(), getParticipantLibraryVersion().toString());
}

} // namespace plugin
} // namespace fep3