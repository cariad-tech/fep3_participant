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
#ifndef _FEP3_COMP_STREAM_TYPE_DEFAULT_H_
#define _FEP3_COMP_STREAM_TYPE_DEFAULT_H_

#include "stream_type.h"
#include <map>
#include <string>
#include <list>

namespace fep3 {
namespace base {
namespace arya {

/// value to define the plain type within the @ref meta_type_plain and @ref  meta_type_plain_array
const std::string meta_type_prop_name_datatype = "datatype";

///value to define max array amount within all array stream types
const std::string meta_type_prop_name_max_array_size = "max_array_size";

///value to define max byte size of a stream type
const std::string meta_type_prop_name_max_byte_size = "max_byte_size";

/**
 * @brief the meta type for plain old datatype
 *
 * name: "plain-ctype"
 * properties: "datatype"
 *
 * @see StreamTypePlain
 */
const StreamMetaType meta_type_plain{"plain-ctype", std::list<std::string>{ meta_type_prop_name_datatype }};
/**
 * @brief the meta type for plain old datatype as array
 *
 * name: "plain-array-ctype"
 * properties: "datatype", "max_array_size"
 *
 * @see StreamTypePlain
 */
const StreamMetaType meta_type_plain_array{"plain-array-ctype", std::list<std::string>{ meta_type_prop_name_datatype, meta_type_prop_name_max_array_size }};
/**
 * @brief the meta type for strings
 *
 * name: "ascii-string"
 * properties: "max_array_size"
 *
 *
 */
const StreamMetaType meta_type_string{"ascii-string", std::list<std::string>{ meta_type_prop_name_max_byte_size }};
/**
 * @brief the meta type for video
 *
 * name: "video"
 * properties: "max_byte_size", "height", "width", "pixelformat"
 *
 *
 */
const StreamMetaType meta_type_video{"video", std::list<std::string>{ "height", "width", "pixelformat", meta_type_prop_name_max_byte_size }};
/**
* @brief the meta type for audio
*
* name: "audio"
* properties: "max_byte_size"
*
*
*/
const StreamMetaType meta_type_audio{"audio", std::list<std::string>{meta_type_prop_name_max_byte_size}};

/**
 * @brief the meta type for raw memory types which are not typed!!
 *
 * name: "anonymous"
 * properties:
 *
 * @see StreamTypeRaw
 */
const StreamMetaType meta_type_raw{"anonymous", std::list<std::string>{ }};


/// value to define the struct type within the @ref meta_type_ddl
const std::string    meta_type_prop_name_ddlstruct = "ddlstruct";
///value to define the whole type definition within the @ref meta_type_ddl
const std::string    meta_type_prop_name_ddldescription = "ddldescription";
///value to define a file reference to the whole type definition within the @ref meta_type_ddl
const std::string    meta_type_prop_name_ddlfileref = "ddlfileref";
/**
 * @brief Meta type for structured memory types which are described by DDL. Description has to be loaded from a file.
 *
 * name: "ddl-fileref"
 * properties: "ddlstruct", "ddlfileref"
 *
 * @see meta_type_prop_name_ddlstruct, meta_type_prop_name_ddlfileref
 * @see StreamTypeDDLFileRef
 */
const StreamMetaType meta_type_ddl_fileref{"ddl-fileref", std::list<std::string>{ meta_type_prop_name_ddlstruct, meta_type_prop_name_ddlfileref }};

/**
 * @brief Meta type for structured memory types which are described by DDL. Description is shipped within a StreamType property.
 *
 * name: "ddl"
 * properties: "ddlstruct", "ddldescription"
 *
 * @see meta_type_prop_name_ddlstruct, meta_type_prop_name_ddldescription
 * @see StreamTypeDDL
 */
const StreamMetaType meta_type_ddl{"ddl", std::list<std::string>{ meta_type_prop_name_ddlstruct, meta_type_prop_name_ddldescription }};

/**
 * @brief The meta type for structured array memory types
 *
 * name: "ddl"
 * properties: "ddlstruct", "ddlfileref", "max_array_size"
 *
 * @see meta_type_prop_name_ddlstruct, meta_type_prop_name_ddlfileref, meta_type_prop_name_max_array_size
 * @see fep::StreamTypeArrayDDL
 */
const StreamMetaType meta_type_ddl_array_fileref{"ddl-fileref-array", std::list<std::string>{ meta_type_prop_name_ddlstruct, meta_type_prop_name_ddlfileref, meta_type_prop_name_max_array_size }};

/**
 * @brief The meta type for structured array memory types
 *
 * name: "ddl"
 * properties: "ddlstruct", "ddldescription", "max_array_size"
 *
 * @see meta_type_prop_name_ddlstruct, meta_type_prop_name_ddldescription, meta_type_prop_name_max_array_size
 * @see fep::StreamTypeArrayDDLFileRef
 */
const StreamMetaType meta_type_ddl_array{"ddl-array", std::list<std::string>{ meta_type_prop_name_ddlstruct, meta_type_prop_name_ddldescription, meta_type_prop_name_max_array_size }};

/**
 * @brief Instance of a raw meta type.
 *
 * @see meta_type_raw
 */
class StreamTypeRaw : public arya::StreamType
{
public:
    /**
     * @brief Construct a new Stream Type Raw object
     *
     */
    StreamTypeRaw() : arya::StreamType(meta_type_raw)
    {
    }
};

/**
 * @brief StreamType class for meta_type_ddl_fileref
 * @see meta_type_ddl_fileref
 * @see StreamTypeDDL
 */
class StreamTypeDDLFileRef : public arya::StreamType
{
public:
    /**
     * @brief Construct a new Stream Type DDL object
     *
     * @param[in] ddlstruct Value for ddl struct this StreamType describes
     * @param[in] fileref Reference to a file which contains the whole ddl description content. Your @p ddlstruct has to be included in this description.
     */
    StreamTypeDDLFileRef(const std::string& ddlstruct, const std::string& fileref) : arya::StreamType(meta_type_ddl_fileref)
    {
        setProperty(meta_type_prop_name_ddlstruct, ddlstruct, "string");
        setProperty(meta_type_prop_name_ddlfileref, fileref, "string");
    }
};

/**
 * @brief StreamType class for meta_type_ddl
 * @see meta_type_ddl
 * @see StreamTypeDDLFileRef
 */
class StreamTypeDDL : public arya::StreamType
{
public:
    /**
     * @brief Construct a new Stream Type DDL object
     *
     * @param[in] ddlstruct Value for ddl struct this StreamType describes
     * @param[in] ddldescription Contains the whole ddl description content. Your @p ddlstruct has to be included in this description.
     */
    StreamTypeDDL(const std::string& ddlstruct, const std::string& ddldescription) : arya::StreamType(meta_type_ddl)
    {
        setProperty(meta_type_prop_name_ddlstruct, ddlstruct, "string");
        setProperty(meta_type_prop_name_ddldescription, ddldescription, "string");
    }
};

/**
 * @brief StreamType class for meta_type_ddl_array
 * @see meta_type_ddl_array
 */
class StreamTypeDDLArrayFileRef : public arya::StreamType
{
public:
    /**
     * @brief Construct an Array Stream Type which uses a single struct from a DDL
     *
     * @param[in] ddlstruct An element of your array
     * @param[in] fileref Contains the whole ddl description content. Your @p ddlstruct has to be included in this description.
     * @param[in] max_array_size Max element amount
     */
    StreamTypeDDLArrayFileRef(const std::string& ddlstruct, const std::string& fileref, uint32_t max_array_size) : arya::StreamType(meta_type_ddl_array_fileref)
    {
        setProperty(meta_type_prop_name_ddlstruct, ddlstruct, "string");
        setProperty(meta_type_prop_name_ddlfileref, fileref, "string");
        setProperty(meta_type_prop_name_max_array_size, std::to_string(max_array_size), "int");
    }
};

/**
 * @brief StreamType class for meta_type_ddl_array
 * @see meta_type_ddl_array
 */
class StreamTypeDDLArray : public arya::StreamType
{
public:
    /**
     * @brief Construct an Array Stream Type which uses a single struct from a DDL
     *
     * @param[in] ddlstruct An element of your array
     * @param[in] ddldescription Contains the whole ddl description content. Your @p ddlstruct has to be included in this description.
     * @param[in] max_array_size Max element amount
     */
    StreamTypeDDLArray(const std::string& ddlstruct, const std::string& ddldescription, uint32_t max_array_size) : arya::StreamType(meta_type_ddl_array)
    {
        setProperty(meta_type_prop_name_ddlstruct, ddlstruct, "string");
        setProperty(meta_type_prop_name_ddldescription, ddldescription, "string");
        setProperty(meta_type_prop_name_max_array_size, std::to_string(max_array_size), "int");
    }
};

namespace
{

/**
 * @brief Type traits providing string representations of plain c types
 *
 * @note Trait name follows std::to_string
 */
template<typename type> struct to_string{};
template<> struct to_string<int8_t>{std::string operator()() const{ return "int8_t"; }};
template<> struct to_string<int16_t>{std::string operator()(){ return "int16_t"; }};
template<> struct to_string<int32_t>{std::string operator()(){ return "int32_t"; }};
template<> struct to_string<int64_t>{std::string operator()(){ return "int64_t"; }};
template<> struct to_string<uint8_t>{std::string operator()(){ return "uint8_t"; }};
template<> struct to_string<uint16_t>{std::string operator()(){ return "uint16_t"; }};
template<> struct to_string<uint32_t>{std::string operator()(){ return "uint32_t"; }};
template<> struct to_string<uint64_t>{std::string operator()(){ return "uint64_t"; }};
template<> struct to_string<float>{std::string operator()(){ return "float"; }};
template<> struct to_string<double>{std::string operator()(){ return "double"; }};

} // namespace

/**
 * @brief StreamType class for any plain c meta type
 *
 * @tparam T the plain c type
 * @see meta_type_raw
 */
template<typename T>
class StreamTypePlain : public arya::StreamType
{
public:
    /**
     * @brief Construct a new Stream Type Plain object
     *
     */
    StreamTypePlain() : arya::StreamType(meta_type_plain)
    {
        setProperty("datatype", to_string<T>()(), "string");
    }
};

/**
 * @brief StreamType class for any plain c array meta type
 *
 * @tparam T the plain c type
 * @see meta_type_raw
 */
template<typename plain_element_type>
class StreamTypePlainArray : public arya::StreamType
{
public:
    /**
     * @brief Construct a new Stream Type Plain Array object
     *
     * @param max_array_size The maximum size of the array a signal of this type can transfer
     */
    StreamTypePlainArray(uint32_t max_array_size) : arya::StreamType(meta_type_plain_array)
    {
        setProperty("datatype", to_string<plain_element_type>()(), "string");
        setProperty(meta_type_prop_name_max_array_size, std::to_string(max_array_size), "uint32_t");
    }
};

/**
 * @brief specialized StreamTypePlain for type string
 */
class StreamTypeString : public arya::StreamType
{
public:
    /**
     * @brief Construct a new Stream Type object for strings
     * @param[in] max_byte_size maximum size of a string
     * @remark max_byte_size=0 means bus default (or dynamic)
     */
    StreamTypeString(size_t max_byte_size = 0) : arya::StreamType(meta_type_string)
    {
        setProperty(meta_type_prop_name_max_byte_size, std::to_string(max_byte_size), "uint32_t");
    }
};
} // namespace arya

using arya::StreamTypeDDL;
using arya::StreamTypeDDLArray;
using arya::StreamTypeDDLArrayFileRef;
using arya::StreamTypeDDLFileRef;
using arya::StreamTypePlain;
using arya::StreamTypePlainArray;
using arya::StreamTypeRaw;
using arya::StreamTypeString;
} // namespace base
} // namespace fep3

#endif //_FEP3_COMP_STREAM_TYPE_DEFAULT_H_
