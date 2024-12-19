/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

// FIXME: Must be removed after ODAUTIL-615 is fixed
#ifdef _WIN32
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif // NOMINMAX
#endif     // _WIN32

#include <dds/dds.hpp>

namespace fep3 {
namespace native {
class DdsSdParticipant {
public:
    DdsSdParticipant();
    // don't really want to copy, DomainParticipant is a heavyweight object
    DdsSdParticipant(DdsSdParticipant const&) = delete;
    void operator=(DdsSdParticipant const&) = delete;

    const dds::all::DomainParticipant& getDomainParticipant() const;

    template <typename T>
    dds::topic::Topic<T> getTopic(std::string topic_name) const
    {
        auto topic_found =
            dds::topic::find<dds::topic::Topic<T>>(getDomainParticipant(), topic_name);
        if (topic_found == dds::core::null) {
            return dds::topic::Topic<T>(getDomainParticipant(), std::move(topic_name));
        }
        else {
            return topic_found;
        }
    }

private:
    dds::all::DomainParticipant _dds_participant;

    static dds::all::DomainParticipant createParticipant();
};

} // namespace native
} // namespace fep3
