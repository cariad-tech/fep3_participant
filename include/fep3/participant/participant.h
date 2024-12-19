/**
 * @file
 * @copyright
 * @verbatim
Copyright 2023 CARIAD SE.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
@endverbatim
 */

#pragma once

#include <fep3/fep3_participant_export.h>          // for FEP3_PARTICIPANT_EXPORT
#include <fep3/participant/element_factory_intf.h> // for IElementFactory

#include <functional> // for function

namespace fep3 {
namespace base {

/**
 * @brief class declaring a Participant
 *
 * The participant embeds an Element into the FEP context and will provide a access point to the
 * service bus and simulation bus.
 */
class FEP3_PARTICIPANT_EXPORT Participant final {
private:
    class Impl;

    /**
     * CTOR
     * @param[in] components The components which are managed by this participant
     * @param[in] impl implementation of participant
     */
    Participant(const fep3::IComponents& components, const std::shared_ptr<Impl> impl);

    /**
     * @brief friend class declaration
     */
    friend Participant FEP3_PARTICIPANT_EXPORT
    createParticipant(const std::string& name,
                      const std::string& version_info,
                      const std::string& system_name,
                      const std::shared_ptr<const base::IElementFactory>& factory,
                      const std::string& server_address_url);

public:
    /**
     * Deleted copy CTOR
     */
    Participant(const Participant&) = delete;

    /**
     * Deleted copy assignment operator
     *
     * @return Reference to this
     */
    Participant& operator=(const Participant&) = delete;

    /**
     * Move CTOR
     */
    Participant(Participant&&);

    /**
     * move assignment operator
     *
     * @return Reference to this
     */
    Participant& operator=(Participant&&);

    /**
     * DTOR
     */
    virtual ~Participant();

    // access to components
    /**
     * Gets a component from a participant
     *
     * @tparam T Type of the component to get
     * @return Result 'NO_ERROR' if succeeded, error code otherwise
     */
    template <class T>
    T* getComponent() const
    {
        return _components->getComponent<T>();
    }

    /**
     * Blocking call to execute this participant.
     *
     * @param[in] start_up_callback optional parameter if the startup is done
     * @return int value to return to stdout if wanted
     * @retval 0 successful and peaceful shutdown of participant reached
     */
    int exec(const std::function<void()>& start_up_callback = nullptr);

    /**
     * Gets the name of the participant
     * @return The name of the participant
     */
    std::string getName() const;

    /**
     * Gets the system name of the participant
     * @return The system name of the participant
     */
    std::string getSystemName() const;

    /**
     * Gets the version information of the participant
     * @return The version information of the participant
     */
    std::string getVersionInfo() const;

private:
    std::shared_ptr<Impl> _impl;
    const fep3::IComponents* _components;
};

/**
 * @brief Creates a participant
 * @param[in] name Name of the participant to be created
 * @param[in] system_name Name of the system this participant belongs to
 * @param[in] version_info Version information of the participant to be created
 * @param[in] factory factory instance which is able to create the element during load command
 * @param[in] server_address_url forced server URL for the participant where it is reachable
 * @return Participant The created participant
 */
Participant FEP3_PARTICIPANT_EXPORT
createParticipant(const std::string& name,
                  const std::string& version_info,
                  const std::string& system_name,
                  const std::shared_ptr<const base::IElementFactory>& factory,
                  const std::string& server_address_url = std::string());

/**
 * @brief Creates a participant
 * @tparam element_factory type of the factory which is able to create the element
 * @param[in] name Name of the participant to be created
 * @param[in] system_name Name of the system this participant belongs to
 * @param[in] version_info Version information of the participant to be created
 * @param[in] server_address_url forced server URL for the participant where it is reachable
 * @return Participant The created participant
 */
template <typename element_factory>
Participant createParticipant(const std::string& name,
                              const std::string& version_info,
                              const std::string& system_name,
                              const std::string& server_address_url = std::string())
{
    const std::shared_ptr<const base::IElementFactory> factory =
        std::make_shared<element_factory>();
    return createParticipant(name, version_info, system_name, factory, server_address_url);
}
} // namespace base

/// @cond nodoc
namespace arya {
using Participant [[deprecated(
    "Since 3.1, fep3::arya::Participant is deprecated. Please use fep3::base::Participant")]] =
    fep3::base::Participant;

template <typename... Args>
[[deprecated("Since 3.1, fep3::arya::createParticipant is deprecated. Please use "
             "fep3::base::createParticipant")]] auto
createParticipant(Args&&... args)
    -> decltype(fep3::base::createParticipant(std::forward<Args>(args)...))
{
    return fep3::base::createParticipant(std::forward<Args>(args)...);
}
} // namespace arya

using Participant [[deprecated(
    "Since 3.1, fep3::Participant is deprecated. Please use fep3::base::Participant")]] =
    fep3::base::Participant;

template <typename... Args>
[[deprecated("Since 3.1, fep3::createParticipant is deprecated. Please use "
             "fep3::base::createParticipant")]] auto
createParticipant(Args&&... args)
    -> decltype(fep3::base::createParticipant(std::forward<Args>(args)...))
{
    return fep3::base::createParticipant(std::forward<Args>(args)...);
}
/// @endcond nodoc

} // namespace fep3
