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
#include <memory>
#include <functional>

#include <fep3/fep3_participant_types.h>
#include <fep3/fep3_errors.h>

#include "element_factory_intf.h"
#include "element_intf.h"
#include <fep3/components/logging/logger_intf.h>


namespace fep3
{
namespace arya
{

/**
 * @brief class declaring a Participant
 *
 * The participant embeds an Element into the FEP context and will provide a access point to the service bus and simulation bus.
 */
class FEP3_PARTICIPANT_EXPORT Participant final
{
private:
    class Impl;
    /**
     * CTOR
     * @param[in] components The components which are managed by this participant
     * @param[in] impl implementation of participant
     */
    Participant(const arya::IComponents& components, const std::shared_ptr<Impl> impl);

    /**
     * @brief friend class declaration
     *
     */
    friend Participant FEP3_PARTICIPANT_EXPORT createParticipant(const std::string& name,
                                         const std::string& version_info,
                                         const std::string& system_name,
                                         const std::shared_ptr<const arya::IElementFactory>& factory,
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
    const arya::IComponents*  _components;
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
Participant FEP3_PARTICIPANT_EXPORT createParticipant(const std::string& name,
    const std::string& version_info,
    const std::string& system_name,
    const std::shared_ptr<const arya::IElementFactory>& factory,
    const std::string& server_address_url = std::string() );
/**
 * @brief Creates a participant
 * @tparam element_factory type of the factory which is able to create the element
 * @param[in] name Name of the participant to be created
 * @param[in] system_name Name of the system this participant belongs to
 * @param[in] version_info Version information of the participant to be created
 * @param[in] server_address_url forced server URL for the participant where it is reachable
 * @return Participant The created participant
*/
template<typename element_factory>
Participant createParticipant(const std::string& name,
    const std::string& version_info,
    const std::string& system_name,
    const std::string& server_address_url = std::string())
{
    const std::shared_ptr<const arya::IElementFactory> factory = std::make_shared<element_factory>();
    return createParticipant(name, version_info, system_name, factory, server_address_url);
}
} // namespace arya
using arya::Participant;
using arya::createParticipant;

} // namespace fep3
