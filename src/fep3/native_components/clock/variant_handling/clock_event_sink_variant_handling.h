/**
 * @file
 * @copyright
 * @verbatim
Copyright @ 2023 VW Group. All rights reserved.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
@endverbatim
 */

#pragma once

#include <fep3/components/clock/clock_intf.h>
#include <fep3/components/clock/clock_service_intf.h>

#include <functional>
#include <variant>

namespace fep3::native {

class AryaToCatelynEventSinkAdapter : public fep3::experimental::IClock::IEventSink {
public:
    AryaToCatelynEventSinkAdapter(std::weak_ptr<fep3::arya::IClock::IEventSink> event_sink);

    void timeUpdateBegin(fep3::arya::Timestamp old_time, fep3::arya::Timestamp new_time) override;
    void timeUpdating(fep3::arya::Timestamp new_time,
                      std::optional<fep3::arya::Timestamp>) override;
    void timeUpdateEnd(fep3::arya::Timestamp new_time) override;
    void timeResetBegin(fep3::arya::Timestamp old_time, fep3::arya::Timestamp new_time) override;
    void timeResetEnd(fep3::arya::Timestamp new_time) override;
    bool isEqual(std::weak_ptr<fep3::arya::IClock::IEventSink> event_sink) const;

private:
    std::weak_ptr<fep3::arya::IClock::IEventSink> _event_sink;
};

class CatelynToAryaEventSinkAdapter : public fep3::arya::IClock::IEventSink {
public:
    CatelynToAryaEventSinkAdapter(std::weak_ptr<fep3::experimental::IClock::IEventSink> event_sink);

    void timeUpdateBegin(fep3::arya::Timestamp old_time, fep3::arya::Timestamp new_time) override;

    void timeUpdating(fep3::arya::Timestamp new_time) override;

    void timeUpdateEnd(fep3::arya::Timestamp new_time) override;

    void timeResetBegin(fep3::arya::Timestamp old_time, fep3::arya::Timestamp new_time) override;

    void timeResetEnd(fep3::arya::Timestamp new_time) override;

    bool isEqual(std::weak_ptr<fep3::experimental::IClock::IEventSink> event_sink) const;

private:
    std::weak_ptr<fep3::experimental::IClock::IEventSink> _event_sink;
};

std::shared_ptr<fep3::experimental::IClock::IEventSink> getCatelynEventSinkPointer(
    std::weak_ptr<fep3::arya::IClock::IEventSink> event_sink);

struct GenericEventSinkAdapter {
    using SinkVariantType = std::variant<std::weak_ptr<fep3::experimental::IClock::IEventSink>,
                                         std::weak_ptr<fep3::arya::IClock::IEventSink>>;

    GenericEventSinkAdapter(std::weak_ptr<fep3::experimental::IClock::IEventSink> event_sink);

    GenericEventSinkAdapter(std::weak_ptr<fep3::arya::IClock::IEventSink> event_sink);

    std::shared_ptr<fep3::experimental::IClock::IEventSink> getPtr() const;

    template <typename T>
    bool isEqual(std::shared_ptr<T> event_sink) const
    {
        return std::visit(
            [&](auto& arg) -> bool {
                using SinkType = typename std::remove_reference_t<decltype(arg)>::element_type;

                if constexpr (std::is_same_v<SinkType, T>)
                    return arg.lock() == event_sink;
                else
                    return false;
            },
            _variant);
    }

private:
    SinkVariantType _variant;
};

} // namespace fep3::native
