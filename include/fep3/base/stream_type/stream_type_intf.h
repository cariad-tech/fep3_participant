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

 //Guideline - FEP System Library API Exception
#ifndef _FEP3_BASE_STREAM_TYPE_INTF_H_
#define _FEP3_BASE_STREAM_TYPE_INTF_H_

#include <string>
#include "../properties/properties_intf.h"

namespace fep3
{
namespace arya
{
/**
 * Definition of a stream type interface.
 * The stream type is a composition of properties
 * (name value pairs) to describe a stream or data sample content.
 *
 * @see page_stream_type
 */
class IStreamType
    : public arya::IProperties

{
protected:
    /**
     * @brief DTOR
     * @note This DTOR is explicitly protected to prevent destruction via this interface.
     */
    ~IStreamType() = default;

public:
    /**
     * @brief Returns the streammetatypes name (@see fep3::base::StreamMetaType)
     * @return std::string The streammetatype name
     */
    virtual std::string getMetaTypeName() const = 0;
};

}
using arya::IStreamType;
}

#endif //_FEP3_BASE_STREAM_TYPE_INTF_H_



