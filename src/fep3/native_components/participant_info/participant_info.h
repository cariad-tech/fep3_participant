/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once
#include <fep3/components/base/component.h>
#include <fep3/components/participant_info/participant_info_intf.h>

namespace fep3 {
namespace native {
class ParticipantInfo : public base::Component<fep3::arya::IParticipantInfo> {
public:
    ParticipantInfo();
    ~ParticipantInfo();
    ParticipantInfo(const ParticipantInfo&) = delete;
    ParticipantInfo(ParticipantInfo&&) = delete;
    ParticipantInfo& operator=(const ParticipantInfo&) = delete;
    ParticipantInfo& operator=(ParticipantInfo&&) = delete;

public: // override states fep3::base::Component
    Result create() override;
    Result destroy() override;

public: // implements fep3::arya::IParticipantInfo
    std::string getName() const override final;
    std::string getSystemName() const override final;

private:
    class Impl;
    std::unique_ptr<Impl> _impl;
};
} // namespace native
} // namespace fep3