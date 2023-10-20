

/*
WARNING: THIS FILE IS AUTO-GENERATED. DO NOT MODIFY.

This file was generated from stream_types.idl using "rtiddsgen".
The rtiddsgen tool is part of the RTI Connext distribution.
For more information, type 'rtiddsgen -help' at a command shell
or consult the RTI Connext manual.
*/

#include <iomanip>
#include <iosfwd>

#pragma warning(push)
#pragma warning(disable : 4800)
#include <rti/topic/cdr/Serialization.hpp>
#pragma warning(pop)

#include "stream_types.hpp"
#include "stream_typesPlugin.hpp"

#include <rti/util/ostream_operators.hpp>

namespace fep3 {

namespace ddstypes {

// ---- Property:

Property::Property() : m_name_(""), m_type_(""), m_value_("")
{
}

Property::Property(const std::string& name, const std::string& type, const std::string& value)
    : m_name_(name), m_type_(type), m_value_(value)
{
}

#ifdef RTI_CXX11_RVALUE_REFERENCES
    #ifdef RTI_CXX11_NO_IMPLICIT_MOVE_OPERATIONS
Property::Property(Property&& other_) OMG_NOEXCEPT : m_name_(std::move(other_.m_name_)),
                                                     m_type_(std::move(other_.m_type_)),
                                                     m_value_(std::move(other_.m_value_))
{
}

Property& Property::operator=(Property&& other_) OMG_NOEXCEPT
{
    Property tmp(std::move(other_));
    swap(tmp);
    return *this;
}
    #endif
#endif

void Property::swap(Property& other_) OMG_NOEXCEPT
{
    using std::swap;
    swap(m_name_, other_.m_name_);
    swap(m_type_, other_.m_type_);
    swap(m_value_, other_.m_value_);
}

bool Property::operator==(const Property& other_) const
{
    if (m_name_ != other_.m_name_) {
        return false;
    }
    if (m_type_ != other_.m_type_) {
        return false;
    }
    if (m_value_ != other_.m_value_) {
        return false;
    }
    return true;
}
bool Property::operator!=(const Property& other_) const
{
    return !this->operator==(other_);
}

std::ostream& operator<<(std::ostream& o, const Property& sample)
{
    ::rti::util::StreamFlagSaver flag_saver(o);
    o << "[";
    o << "name: " << sample.name() << ", ";
    o << "type: " << sample.type() << ", ";
    o << "value: " << sample.value();
    o << "]";
    return o;
}

// ---- StreamType:

StreamType::StreamType() : m_metatype_("")
{
}

StreamType::StreamType(const std::string& metatype,
                       const std::vector<fep3::ddstypes::Property>& properties)
    : m_metatype_(metatype), m_properties_(properties)
{
}

#ifdef RTI_CXX11_RVALUE_REFERENCES
    #ifdef RTI_CXX11_NO_IMPLICIT_MOVE_OPERATIONS
StreamType::StreamType(StreamType&& other_) OMG_NOEXCEPT
    : m_metatype_(std::move(other_.m_metatype_)),
      m_properties_(std::move(other_.m_properties_))
{
}

StreamType& StreamType::operator=(StreamType&& other_) OMG_NOEXCEPT
{
    StreamType tmp(std::move(other_));
    swap(tmp);
    return *this;
}
    #endif
#endif

void StreamType::swap(StreamType& other_) OMG_NOEXCEPT
{
    using std::swap;
    swap(m_metatype_, other_.m_metatype_);
    swap(m_properties_, other_.m_properties_);
}

bool StreamType::operator==(const StreamType& other_) const
{
    if (m_metatype_ != other_.m_metatype_) {
        return false;
    }
    if (m_properties_ != other_.m_properties_) {
        return false;
    }
    return true;
}
bool StreamType::operator!=(const StreamType& other_) const
{
    return !this->operator==(other_);
}

std::ostream& operator<<(std::ostream& o, const StreamType& sample)
{
    ::rti::util::StreamFlagSaver flag_saver(o);
    o << "[";
    o << "metatype: " << sample.metatype() << ", ";
    o << "properties: " << sample.properties();
    o << "]";
    return o;
}

} // namespace ddstypes

} // namespace fep3

// --- Type traits: -------------------------------------------------

namespace rti {
namespace topic {

#ifndef NDDS_STANDALONE_TYPE

template <>
struct native_type_code<fep3::ddstypes::Property> {
    static DDS_TypeCode* get()
    {
        using namespace ::rti::topic::interpreter;

        static RTIBool is_initialized = RTI_FALSE;

        static DDS_TypeCode Property_g_tc_name_string;
        static DDS_TypeCode Property_g_tc_type_string;
        static DDS_TypeCode Property_g_tc_value_string;

        static DDS_TypeCode_Member Property_g_tc_members[3] = {

            {(char*)"name", /* Member name */
             {
                 0,                 /* Representation ID */
                 DDS_BOOLEAN_FALSE, /* Is a pointer? */
                 -1,                /* Bitfield bits */
                 NULL               /* Member type code is assigned later */
             },
             0,                       /* Ignored */
             0,                       /* Ignored */
             0,                       /* Ignored */
             NULL,                    /* Ignored */
             RTI_CDR_REQUIRED_MEMBER, /* Is a key? */
             DDS_PUBLIC_MEMBER,       /* Member visibility */
             1,
             NULL, /* Ignored */
             RTICdrTypeCodeAnnotations_INITIALIZER},
            {(char*)"type", /* Member name */
             {
                 1,                 /* Representation ID */
                 DDS_BOOLEAN_FALSE, /* Is a pointer? */
                 -1,                /* Bitfield bits */
                 NULL               /* Member type code is assigned later */
             },
             0,                       /* Ignored */
             0,                       /* Ignored */
             0,                       /* Ignored */
             NULL,                    /* Ignored */
             RTI_CDR_REQUIRED_MEMBER, /* Is a key? */
             DDS_PUBLIC_MEMBER,       /* Member visibility */
             1,
             NULL, /* Ignored */
             RTICdrTypeCodeAnnotations_INITIALIZER},
            {(char*)"value", /* Member name */
             {
                 2,                 /* Representation ID */
                 DDS_BOOLEAN_FALSE, /* Is a pointer? */
                 -1,                /* Bitfield bits */
                 NULL               /* Member type code is assigned later */
             },
             0,                       /* Ignored */
             0,                       /* Ignored */
             0,                       /* Ignored */
             NULL,                    /* Ignored */
             RTI_CDR_REQUIRED_MEMBER, /* Is a key? */
             DDS_PUBLIC_MEMBER,       /* Member visibility */
             1,
             NULL, /* Ignored */
             RTICdrTypeCodeAnnotations_INITIALIZER}};

        static DDS_TypeCode Property_g_tc = {{
            DDS_TK_STRUCT,                     /* Kind */
            DDS_BOOLEAN_FALSE,                 /* Ignored */
            -1,                                /*Ignored*/
            (char*)"fep3::ddstypes::Property", /* Name */
            NULL,                              /* Ignored */
            0,                                 /* Ignored */
            0,                                 /* Ignored */
            NULL,                              /* Ignored */
            3,                                 /* Number of members */
            Property_g_tc_members,             /* Members */
            DDS_VM_NONE,                       /* Ignored */
            RTICdrTypeCodeAnnotations_INITIALIZER,
            DDS_BOOLEAN_TRUE, /* _isCopyable */
            NULL,             /* _sampleAccessInfo: assigned later */
            NULL              /* _typePlugin: assigned later */
        }};                   /* Type code for Property*/

        if (is_initialized) {
            return &Property_g_tc;
        }

        Property_g_tc_name_string = initialize_string_typecode((256));
        Property_g_tc_type_string = initialize_string_typecode((64));
        Property_g_tc_value_string = initialize_string_typecode(RTIXCdrLong_MAX);

        Property_g_tc._data._annotations._allowedDataRepresentationMask = 5;

        Property_g_tc_members[0]._representation._typeCode =
            (RTICdrTypeCode*)&Property_g_tc_name_string;
        Property_g_tc_members[1]._representation._typeCode =
            (RTICdrTypeCode*)&Property_g_tc_type_string;
        Property_g_tc_members[2]._representation._typeCode =
            (RTICdrTypeCode*)&Property_g_tc_value_string;

        /* Initialize the values for member annotations. */
        Property_g_tc_members[0]._annotations._defaultValue._d = RTI_XCDR_TK_STRING;
        Property_g_tc_members[0]._annotations._defaultValue._u.string_value = (DDS_Char*)"";

        Property_g_tc_members[1]._annotations._defaultValue._d = RTI_XCDR_TK_STRING;
        Property_g_tc_members[1]._annotations._defaultValue._u.string_value = (DDS_Char*)"";

        Property_g_tc_members[2]._annotations._defaultValue._d = RTI_XCDR_TK_STRING;
        Property_g_tc_members[2]._annotations._defaultValue._u.string_value = (DDS_Char*)"";

        Property_g_tc._data._sampleAccessInfo = sample_access_info();
        Property_g_tc._data._typePlugin = type_plugin_info();

        is_initialized = RTI_TRUE;

        return &Property_g_tc;
    }

    static RTIXCdrSampleAccessInfo* sample_access_info()
    {
        static RTIBool is_initialized = RTI_FALSE;

        fep3::ddstypes::Property* sample;

        static RTIXCdrMemberAccessInfo Property_g_memberAccessInfos[3] = {
            RTIXCdrMemberAccessInfo_INITIALIZER};

        static RTIXCdrSampleAccessInfo Property_g_sampleAccessInfo =
            RTIXCdrSampleAccessInfo_INITIALIZER;

        if (is_initialized) {
            return (RTIXCdrSampleAccessInfo*)&Property_g_sampleAccessInfo;
        }

        RTIXCdrHeap_allocateStruct(&sample, fep3::ddstypes::Property);
        if (sample == NULL) {
            return NULL;
        }

        Property_g_memberAccessInfos[0].bindingMemberValueOffset[0] =
            (RTIXCdrUnsignedLong)((char*)&sample->name() - (char*)sample);

        Property_g_memberAccessInfos[1].bindingMemberValueOffset[0] =
            (RTIXCdrUnsignedLong)((char*)&sample->type() - (char*)sample);

        Property_g_memberAccessInfos[2].bindingMemberValueOffset[0] =
            (RTIXCdrUnsignedLong)((char*)&sample->value() - (char*)sample);

        Property_g_sampleAccessInfo.memberAccessInfos = Property_g_memberAccessInfos;

        {
            size_t candidateTypeSize = sizeof(fep3::ddstypes::Property);

            if (candidateTypeSize > RTIXCdrUnsignedLong_MAX) {
                Property_g_sampleAccessInfo.typeSize[0] = RTIXCdrUnsignedLong_MAX;
            }
            else {
                Property_g_sampleAccessInfo.typeSize[0] = (RTIXCdrUnsignedLong)candidateTypeSize;
            }
        }

        Property_g_sampleAccessInfo.useGetMemberValueOnlyWithRef = RTI_XCDR_TRUE;

        Property_g_sampleAccessInfo.getMemberValuePointerFcn =
            interpreter::get_aggregation_value_pointer<fep3::ddstypes::Property>;

        Property_g_sampleAccessInfo.languageBinding = RTI_XCDR_TYPE_BINDING_CPP_11_STL;

        RTIXCdrHeap_freeStruct(sample);
        is_initialized = RTI_TRUE;
        return (RTIXCdrSampleAccessInfo*)&Property_g_sampleAccessInfo;
    }

    static RTIXCdrTypePlugin* type_plugin_info()
    {
        static RTIXCdrTypePlugin Property_g_typePlugin = {
            NULL, /* serialize */
            NULL, /* serialize_key */
            NULL, /* deserialize_sample */
            NULL, /* deserialize_key_sample */
            NULL, /* skip */
            NULL, /* get_serialized_sample_size */
            NULL, /* get_serialized_sample_max_size_ex */
            NULL, /* get_serialized_key_max_size_ex */
            NULL, /* get_serialized_sample_min_size */
            NULL, /* serialized_sample_to_key */
            NULL,
            NULL,
            NULL,
            NULL};

        return &Property_g_typePlugin;
    }
}; // native_type_code
#endif

const ::dds::core::xtypes::StructType& dynamic_type<fep3::ddstypes::Property>::get()
{
    return static_cast<const ::dds::core::xtypes::StructType&>(
        ::rti::core::native_conversions::cast_from_native<::dds::core::xtypes::DynamicType>(
            *(native_type_code<fep3::ddstypes::Property>::get())));
}

#ifndef NDDS_STANDALONE_TYPE

template <>
struct native_type_code<fep3::ddstypes::StreamType> {
    static DDS_TypeCode* get()
    {
        using namespace ::rti::topic::interpreter;

        static RTIBool is_initialized = RTI_FALSE;

        static DDS_TypeCode StreamType_g_tc_metatype_string;
        static DDS_TypeCode StreamType_g_tc_properties_sequence;

        static DDS_TypeCode_Member StreamType_g_tc_members[2] = {

            {(char*)"metatype", /* Member name */
             {
                 0,                 /* Representation ID */
                 DDS_BOOLEAN_FALSE, /* Is a pointer? */
                 -1,                /* Bitfield bits */
                 NULL               /* Member type code is assigned later */
             },
             0,                       /* Ignored */
             0,                       /* Ignored */
             0,                       /* Ignored */
             NULL,                    /* Ignored */
             RTI_CDR_REQUIRED_MEMBER, /* Is a key? */
             DDS_PUBLIC_MEMBER,       /* Member visibility */
             1,
             NULL, /* Ignored */
             RTICdrTypeCodeAnnotations_INITIALIZER},
            {(char*)"properties", /* Member name */
             {
                 1,                 /* Representation ID */
                 DDS_BOOLEAN_FALSE, /* Is a pointer? */
                 -1,                /* Bitfield bits */
                 NULL               /* Member type code is assigned later */
             },
             0,                       /* Ignored */
             0,                       /* Ignored */
             0,                       /* Ignored */
             NULL,                    /* Ignored */
             RTI_CDR_REQUIRED_MEMBER, /* Is a key? */
             DDS_PUBLIC_MEMBER,       /* Member visibility */
             1,
             NULL, /* Ignored */
             RTICdrTypeCodeAnnotations_INITIALIZER}};

        static DDS_TypeCode StreamType_g_tc = {{
            DDS_TK_STRUCT,                       /* Kind */
            DDS_BOOLEAN_FALSE,                   /* Ignored */
            -1,                                  /*Ignored*/
            (char*)"fep3::ddstypes::StreamType", /* Name */
            NULL,                                /* Ignored */
            0,                                   /* Ignored */
            0,                                   /* Ignored */
            NULL,                                /* Ignored */
            2,                                   /* Number of members */
            StreamType_g_tc_members,             /* Members */
            DDS_VM_NONE,                         /* Ignored */
            RTICdrTypeCodeAnnotations_INITIALIZER,
            DDS_BOOLEAN_TRUE, /* _isCopyable */
            NULL,             /* _sampleAccessInfo: assigned later */
            NULL              /* _typePlugin: assigned later */
        }};                   /* Type code for StreamType*/

        if (is_initialized) {
            return &StreamType_g_tc;
        }

        StreamType_g_tc_metatype_string = initialize_string_typecode(RTIXCdrLong_MAX);
        StreamType_g_tc_properties_sequence =
            initialize_sequence_typecode<std::vector<fep3::ddstypes::Property>>(RTIXCdrLong_MAX);

        StreamType_g_tc._data._annotations._allowedDataRepresentationMask = 5;

        StreamType_g_tc_properties_sequence._data._typeCode =
            (RTICdrTypeCode*)&::rti::topic::dynamic_type<fep3::ddstypes::Property>::get().native();
        StreamType_g_tc_members[0]._representation._typeCode =
            (RTICdrTypeCode*)&StreamType_g_tc_metatype_string;
        StreamType_g_tc_members[1]._representation._typeCode =
            (RTICdrTypeCode*)&StreamType_g_tc_properties_sequence;

        /* Initialize the values for member annotations. */
        StreamType_g_tc_members[0]._annotations._defaultValue._d = RTI_XCDR_TK_STRING;
        StreamType_g_tc_members[0]._annotations._defaultValue._u.string_value = (DDS_Char*)"";

        StreamType_g_tc._data._sampleAccessInfo = sample_access_info();
        StreamType_g_tc._data._typePlugin = type_plugin_info();

        is_initialized = RTI_TRUE;

        return &StreamType_g_tc;
    }

    static RTIXCdrSampleAccessInfo* sample_access_info()
    {
        static RTIBool is_initialized = RTI_FALSE;

        fep3::ddstypes::StreamType* sample;

        static RTIXCdrMemberAccessInfo StreamType_g_memberAccessInfos[2] = {
            RTIXCdrMemberAccessInfo_INITIALIZER};

        static RTIXCdrSampleAccessInfo StreamType_g_sampleAccessInfo =
            RTIXCdrSampleAccessInfo_INITIALIZER;

        if (is_initialized) {
            return (RTIXCdrSampleAccessInfo*)&StreamType_g_sampleAccessInfo;
        }

        RTIXCdrHeap_allocateStruct(&sample, fep3::ddstypes::StreamType);
        if (sample == NULL) {
            return NULL;
        }

        StreamType_g_memberAccessInfos[0].bindingMemberValueOffset[0] =
            (RTIXCdrUnsignedLong)((char*)&sample->metatype() - (char*)sample);

        StreamType_g_memberAccessInfos[1].bindingMemberValueOffset[0] =
            (RTIXCdrUnsignedLong)((char*)&sample->properties() - (char*)sample);

        StreamType_g_sampleAccessInfo.memberAccessInfos = StreamType_g_memberAccessInfos;

        {
            size_t candidateTypeSize = sizeof(fep3::ddstypes::StreamType);

            if (candidateTypeSize > RTIXCdrUnsignedLong_MAX) {
                StreamType_g_sampleAccessInfo.typeSize[0] = RTIXCdrUnsignedLong_MAX;
            }
            else {
                StreamType_g_sampleAccessInfo.typeSize[0] = (RTIXCdrUnsignedLong)candidateTypeSize;
            }
        }

        StreamType_g_sampleAccessInfo.useGetMemberValueOnlyWithRef = RTI_XCDR_TRUE;

        StreamType_g_sampleAccessInfo.getMemberValuePointerFcn =
            interpreter::get_aggregation_value_pointer<fep3::ddstypes::StreamType>;

        StreamType_g_sampleAccessInfo.languageBinding = RTI_XCDR_TYPE_BINDING_CPP_11_STL;

        RTIXCdrHeap_freeStruct(sample);
        is_initialized = RTI_TRUE;
        return (RTIXCdrSampleAccessInfo*)&StreamType_g_sampleAccessInfo;
    }

    static RTIXCdrTypePlugin* type_plugin_info()
    {
        static RTIXCdrTypePlugin StreamType_g_typePlugin = {
            NULL, /* serialize */
            NULL, /* serialize_key */
            NULL, /* deserialize_sample */
            NULL, /* deserialize_key_sample */
            NULL, /* skip */
            NULL, /* get_serialized_sample_size */
            NULL, /* get_serialized_sample_max_size_ex */
            NULL, /* get_serialized_key_max_size_ex */
            NULL, /* get_serialized_sample_min_size */
            NULL, /* serialized_sample_to_key */
            NULL,
            NULL,
            NULL,
            NULL};

        return &StreamType_g_typePlugin;
    }
}; // native_type_code
#endif

const ::dds::core::xtypes::StructType& dynamic_type<fep3::ddstypes::StreamType>::get()
{
    return static_cast<const ::dds::core::xtypes::StructType&>(
        ::rti::core::native_conversions::cast_from_native<::dds::core::xtypes::DynamicType>(
            *(native_type_code<fep3::ddstypes::StreamType>::get())));
}

} // namespace topic
} // namespace rti

namespace dds {
namespace topic {
void topic_type_support<fep3::ddstypes::Property>::register_type(
    ::dds::domain::DomainParticipant& participant, const std::string& type_name)
{
    ::rti::domain::register_type_plugin(participant,
                                        type_name,
                                        fep3::ddstypes::PropertyPlugin_new,
                                        fep3::ddstypes::PropertyPlugin_delete);
}

std::vector<char>& topic_type_support<fep3::ddstypes::Property>::to_cdr_buffer(
    std::vector<char>& buffer,
    const fep3::ddstypes::Property& sample,
    ::dds::core::policy::DataRepresentationId representation)
{
    // First get the length of the buffer
    unsigned int length = 0;
    RTIBool ok = PropertyPlugin_serialize_to_cdr_buffer(NULL, &length, &sample, representation);
    ::rti::core::check_return_code(ok ? DDS_RETCODE_OK : DDS_RETCODE_ERROR,
                                   "Failed to calculate cdr buffer size");

    // Create a vector with that size and copy the cdr buffer into it
    buffer.resize(length);
    ok = PropertyPlugin_serialize_to_cdr_buffer(&buffer[0], &length, &sample, representation);
    ::rti::core::check_return_code(ok ? DDS_RETCODE_OK : DDS_RETCODE_ERROR,
                                   "Failed to copy cdr buffer");

    return buffer;
}

void topic_type_support<fep3::ddstypes::Property>::from_cdr_buffer(fep3::ddstypes::Property& sample,
                                                                   const std::vector<char>& buffer)
{
    RTIBool ok = PropertyPlugin_deserialize_from_cdr_buffer(
        &sample, &buffer[0], static_cast<unsigned int>(buffer.size()));
    ::rti::core::check_return_code(ok ? DDS_RETCODE_OK : DDS_RETCODE_ERROR,
                                   "Failed to create fep3::ddstypes::Property from cdr buffer");
}

void topic_type_support<fep3::ddstypes::Property>::reset_sample(fep3::ddstypes::Property& sample)
{
    sample.name("");
    sample.type("");
    sample.value("");
}

void topic_type_support<fep3::ddstypes::Property>::allocate_sample(fep3::ddstypes::Property& sample,
                                                                   int,
                                                                   int)
{
    RTIOsapiUtility_unusedParameter(sample); // [[maybe_unused]]

    ::rti::topic::allocate_sample(sample.name(), -1, 256);
    ::rti::topic::allocate_sample(sample.type(), -1, 64);
    ::rti::topic::allocate_sample(sample.value(), -1, -1);
}

void topic_type_support<fep3::ddstypes::StreamType>::register_type(
    ::dds::domain::DomainParticipant& participant, const std::string& type_name)
{
    ::rti::domain::register_type_plugin(participant,
                                        type_name,
                                        fep3::ddstypes::StreamTypePlugin_new,
                                        fep3::ddstypes::StreamTypePlugin_delete);
}

std::vector<char>& topic_type_support<fep3::ddstypes::StreamType>::to_cdr_buffer(
    std::vector<char>& buffer,
    const fep3::ddstypes::StreamType& sample,
    ::dds::core::policy::DataRepresentationId representation)
{
    // First get the length of the buffer
    unsigned int length = 0;
    RTIBool ok = StreamTypePlugin_serialize_to_cdr_buffer(NULL, &length, &sample, representation);
    ::rti::core::check_return_code(ok ? DDS_RETCODE_OK : DDS_RETCODE_ERROR,
                                   "Failed to calculate cdr buffer size");

    // Create a vector with that size and copy the cdr buffer into it
    buffer.resize(length);
    ok = StreamTypePlugin_serialize_to_cdr_buffer(&buffer[0], &length, &sample, representation);
    ::rti::core::check_return_code(ok ? DDS_RETCODE_OK : DDS_RETCODE_ERROR,
                                   "Failed to copy cdr buffer");

    return buffer;
}

void topic_type_support<fep3::ddstypes::StreamType>::from_cdr_buffer(
    fep3::ddstypes::StreamType& sample, const std::vector<char>& buffer)
{
    RTIBool ok = StreamTypePlugin_deserialize_from_cdr_buffer(
        &sample, &buffer[0], static_cast<unsigned int>(buffer.size()));
    ::rti::core::check_return_code(ok ? DDS_RETCODE_OK : DDS_RETCODE_ERROR,
                                   "Failed to create fep3::ddstypes::StreamType from cdr buffer");
}

void topic_type_support<fep3::ddstypes::StreamType>::reset_sample(
    fep3::ddstypes::StreamType& sample)
{
    sample.metatype("");
    ::rti::topic::reset_sample(sample.properties());
}

void topic_type_support<fep3::ddstypes::StreamType>::allocate_sample(
    fep3::ddstypes::StreamType& sample, int, int)
{
    RTIOsapiUtility_unusedParameter(sample); // [[maybe_unused]]

    ::rti::topic::allocate_sample(sample.metatype(), -1, -1);
    ::rti::topic::allocate_sample(sample.properties(), -1, -1);
}

} // namespace topic
} // namespace dds
