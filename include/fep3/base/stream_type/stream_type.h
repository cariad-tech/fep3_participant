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

// Guideline - FEP System Library API Exception
#ifndef _FEP3_BASE_STREAM_TYPE_H_
#define _FEP3_BASE_STREAM_TYPE_H_

#include <fep3/base/properties/properties.h>
#include <fep3/base/stream_type/stream_type_intf.h>

#include <algorithm>
#include <list>

namespace fep3 {
namespace base {
namespace arya {
/**
 * @brief Implementation for a Stream Meta Type representation.
 * This representation contains the name of the meta type and it will also
 * contain a list of properties which are mandatory to describe this kind of meta type
 */
class StreamMetaType {
protected:
    /**
     * @brief Construct a new Stream Meta Type object
     */
    StreamMetaType() = default;

public:
    /**
     * @brief Construct a new Stream Meta Type object
     */
    StreamMetaType(StreamMetaType&&) = default;

    /**
     * @brief Construct a new Stream Meta Type object
     */
    StreamMetaType(const StreamMetaType&) = default;

    /**
     * @brief Default move operator
     *
     * @return StreamMetaType&
     */
    StreamMetaType& operator=(StreamMetaType&&) = default;

    /**
     * default operator=
     * @return the streammetatype
     */
    StreamMetaType& operator=(const StreamMetaType&) = default;

    /**
     * @brief Default DTOR
     */
    virtual ~StreamMetaType() = default;

    /**
     * @brief Construct a new Stream Meta Type object
     *
     * @param[in] meta_type_name
     */
    StreamMetaType(std::string meta_type_name) : _meta_type_name(std::move(meta_type_name))
    {
    }

    /**
     * @brief Construct a new Stream Meta Type object
     *
     * @param[in] meta_type_name the name if the metatype
     * @param[in] required_properties a list of required mandatory property names for the metatype
     */
    StreamMetaType(std::string meta_type_name, std::list<std::string> required_properties)
        : _meta_type_name(std::move(meta_type_name)),
          _required_properties(std::move(required_properties))
    {
    }

    /**
     * @brief Gets the Name of the meta type
     *
     * @return const char* the name of the meta type
     */
    const char* getName() const
    {
        return _meta_type_name.c_str();
    }

    /**
     * @brief compares (only) the names of the meta types
     *
     * @param[in] other the meta type to compare to
     * @return @c true if the name is equal, @c false otherwise
     */
    bool operator==(const StreamMetaType& other) const
    {
        return _meta_type_name == other.getName();
    }

    /**
     * @brief compares (only) the names of the meta types
     *
     * @param[in] other the meta type to compare to
     * @return @c true if the name is equal, @c false otherwise
     */
    bool operator==(const fep3::arya::IStreamType& other) const
    {
        return _meta_type_name == other.getMetaTypeName();
    }

private:
    /// storage variable of the metatypename
    std::string _meta_type_name;
    /// storage variable of the required mandatory properties of this metatype
    std::list<std::string> _required_properties;
};

/**
 * Helper function to check if the given stream type is part of the given list with streammetatypes.
 * The given @p type is supported if its metatype is part of the given metatype list.
 * @param[in] supported_list list of metatypes to check
 * @param[in] type stream_type to check if it is part of the supported list.
 * @return @c true if the type is supported, @c false otherwise
 */
inline bool isSupportedMetaType(const std::vector<arya::StreamMetaType>& supported_list,
                                const fep3::arya::IStreamType& type)
{
    return std::any_of(
        supported_list.begin(), supported_list.end(), [&type](const arya::StreamMetaType& smt) {
            return smt.getName() == type.getMetaTypeName();
        });
}

/**
 * @brief Representation class of a Stream Meta Type instance
 * This class will represent one instance of a Stream Meta Type.
 * It will have set the values for the meta types required mandatory properties
 * @see fep::StreamMetaType
 */
class StreamType : public base::arya::Properties<fep3::arya::IStreamType> {
protected:
    /**
     * @brief Construct a new Stream Type object
     */
    StreamType() = default;

public:
    /**
     * @brief Construct a new Stream Type object
     */
    StreamType(StreamType&&) = default;

    /**
     * @brief Construct a new Stream Type object
     */
    StreamType(const StreamType&) = default;

    /**
     * @brief
     *
     * @return StreamType&
     */
    StreamType& operator=(StreamType&&) = default;

    /**
     * @brief
     *
     * @return StreamType&
     */
    StreamType& operator=(const StreamType&) = default;

    /**
     * @brief Default DTOR
     */
    virtual ~StreamType() = default;

    /**
     * @brief Construct a new Stream Type object
     *
     * @param[in] meta_type the metatype the stream type is instantiating
     */
    StreamType(arya::StreamMetaType meta_type) : _meta_type_name(std::move(meta_type))
    {
    }

    /**
     * @brief Construct a new Stream Type object
     *
     * @param[in] stream_type stream type as interface
     */
    StreamType(const fep3::arya::IStreamType& stream_type)
        : _meta_type_name(stream_type.getMetaTypeName())
    {
        stream_type.copyTo(*this);
    }

    /**
     * @brief assignment operator
     *
     * @param[in] stream_type the other stream type as interface
     * @return StreamType& the stream type itself
     */
    StreamType& operator=(const fep3::arya::IStreamType& stream_type)
    {
        _meta_type_name = StreamMetaType(stream_type.getMetaTypeName());
        stream_type.copyTo(*this);
        return *this;
    }

    using base::arya::Properties<fep3::arya::IStreamType>::getProperty;
    using base::arya::Properties<fep3::arya::IStreamType>::getPropertyType;
    using base::arya::Properties<fep3::arya::IStreamType>::setProperty;
    using base::arya::Properties<fep3::arya::IStreamType>::copyTo;
    using base::arya::Properties<fep3::arya::IStreamType>::isEqual;

    /**
     * @brief Get the Meta Type Name of the stream type
     *
     * @return const char* the meta type name
     */
    std::string getMetaTypeName() const override
    {
        return _meta_type_name.getName();
    }

    /**
     * @brief Get a copy of the Meta Type object
     *
     * @return StreamMetaType
     */
    arya::StreamMetaType getMetaType() const
    {
        return _meta_type_name;
    }

private:
    /// the local stream meta type decription
    arya::StreamMetaType _meta_type_name;
};
} // namespace arya
using arya::StreamMetaType;
using arya::StreamType;
} // namespace base
} // namespace fep3

/**
 * @brief Bool operator to compare stream types.
 * Streamtypes are equal if the name of the stream metatypes and the set properties are equal.
 * @param[in] lhs Left stream type
 * @param[in] rhs Right stream type
 * @return @c true if equal, @c false otherweise
 */
inline bool operator==(const fep3::arya::IStreamType& lhs, const fep3::arya::IStreamType& rhs)
{
    if (std::string(lhs.getMetaTypeName()) != rhs.getMetaTypeName()) {
        return false;
    }
    else {
        return lhs.isEqual(rhs);
    }
}

#endif //_FEP3_BASE_STREAM_TYPE_H_
