

/*
WARNING: THIS FILE IS AUTO-GENERATED. DO NOT MODIFY.

This file was generated from DdsServiceDiscoveryTopic.idl
using RTI Code Generator (rtiddsgen) version 3.1.0.
The rtiddsgen tool is part of the RTI Connext DDS distribution.
For more information, type 'rtiddsgen -help' at a command shell
or consult the Code Generator User's Manual.
*/

#include "../include/DdsServiceDiscoveryTopic.hpp"

#include "../include/DdsServiceDiscoveryTopicPlugin.hpp"

#include <rti/topic/cdr/Serialization.hpp>
#include <rti/util/ostream_operators.hpp>

#include <iomanip>
#include <iosfwd>

// ---- ParticipantData:

ParticipantData::ParticipantData() : m_service_name_(""), m_host_url_("")
{
}

ParticipantData::ParticipantData(const std::string& service_name, const std::string& host_url)
    : m_service_name_(service_name), m_host_url_(host_url)
{
}

#ifdef RTI_CXX11_RVALUE_REFERENCES
    #ifdef RTI_CXX11_NO_IMPLICIT_MOVE_OPERATIONS
ParticipantData::ParticipantData(ParticipantData&& other_) OMG_NOEXCEPT
    : m_service_name_(std::move(other_.m_service_name_)),
      m_host_url_(std::move(other_.m_host_url_))
{
}

ParticipantData& ParticipantData::operator=(ParticipantData&& other_) OMG_NOEXCEPT
{
    ParticipantData tmp(std::move(other_));
    swap(tmp);
    return *this;
}
    #endif
#endif

void ParticipantData::swap(ParticipantData& other_) OMG_NOEXCEPT
{
    using std::swap;
    swap(m_service_name_, other_.m_service_name_);
    swap(m_host_url_, other_.m_host_url_);
}

bool ParticipantData::operator==(const ParticipantData& other_) const
{
    if (m_service_name_ != other_.m_service_name_) {
        return false;
    }
    if (m_host_url_ != other_.m_host_url_) {
        return false;
    }
    return true;
}
bool ParticipantData::operator!=(const ParticipantData& other_) const
{
    return !this->operator==(other_);
}

std::ostream& operator<<(std::ostream& o, const ParticipantData& sample)
{
    ::rti::util::StreamFlagSaver flag_saver(o);
    o << "[";
    o << "service_name: " << sample.service_name() << ", ";
    o << "host_url: " << sample.host_url();
    o << "]";
    return o;
}

std::ostream& operator<<(std::ostream& o, const ServiceDiscoveryResponseType& sample)
{
    ::rti::util::StreamFlagSaver flag_saver(o);
    switch (sample) {
    case ServiceDiscoveryResponseType::ResponseDiscover:
        o << "ServiceDiscoveryResponseType::ResponseDiscover"
          << " ";
        break;
    case ServiceDiscoveryResponseType::ResponseBye:
        o << "ServiceDiscoveryResponseType::ResponseBye"
          << " ";
        break;
    case ServiceDiscoveryResponseType::ResponseAlive:
        o << "ServiceDiscoveryResponseType::ResponseAlive"
          << " ";
        break;
    }
    return o;
}

// ---- DdsServiceDiscovery:

DdsServiceDiscovery::DdsServiceDiscovery()
    : m_id_(""), m_response_type_(ServiceDiscoveryResponseType::ResponseDiscover)
{
}

DdsServiceDiscovery::DdsServiceDiscovery(const std::string& id,
                                         const ParticipantData& content,
                                         const ServiceDiscoveryResponseType& response_type)
    : m_id_(id), m_content_(content), m_response_type_(response_type)
{
}

#ifdef RTI_CXX11_RVALUE_REFERENCES
    #ifdef RTI_CXX11_NO_IMPLICIT_MOVE_OPERATIONS
DdsServiceDiscovery::DdsServiceDiscovery(DdsServiceDiscovery&& other_) OMG_NOEXCEPT
    : m_id_(std::move(other_.m_id_)),
      m_content_(std::move(other_.m_content_)),
      m_response_type_(std::move(other_.m_response_type_))
{
}

DdsServiceDiscovery& DdsServiceDiscovery::operator=(DdsServiceDiscovery&& other_) OMG_NOEXCEPT
{
    DdsServiceDiscovery tmp(std::move(other_));
    swap(tmp);
    return *this;
}
    #endif
#endif

void DdsServiceDiscovery::swap(DdsServiceDiscovery& other_) OMG_NOEXCEPT
{
    using std::swap;
    swap(m_id_, other_.m_id_);
    swap(m_content_, other_.m_content_);
    swap(m_response_type_, other_.m_response_type_);
}

bool DdsServiceDiscovery::operator==(const DdsServiceDiscovery& other_) const
{
    if (m_id_ != other_.m_id_) {
        return false;
    }
    if (m_content_ != other_.m_content_) {
        return false;
    }
    if (m_response_type_ != other_.m_response_type_) {
        return false;
    }
    return true;
}
bool DdsServiceDiscovery::operator!=(const DdsServiceDiscovery& other_) const
{
    return !this->operator==(other_);
}

std::ostream& operator<<(std::ostream& o, const DdsServiceDiscovery& sample)
{
    ::rti::util::StreamFlagSaver flag_saver(o);
    o << "[";
    o << "id: " << sample.id() << ", ";
    o << "content: " << sample.content() << ", ";
    o << "response_type: " << sample.response_type();
    o << "]";
    return o;
}

// --- Type traits: -------------------------------------------------

namespace rti {
namespace topic {

#ifndef NDDS_STANDALONE_TYPE
template <>
struct native_type_code<ParticipantData> {
    static DDS_TypeCode* get()
    {
        using namespace ::rti::topic::interpreter;

        static RTIBool is_initialized = RTI_FALSE;

        static DDS_TypeCode ParticipantData_g_tc_service_name_string;
        static DDS_TypeCode ParticipantData_g_tc_host_url_string;

        static DDS_TypeCode_Member ParticipantData_g_tc_members[2] = {

            {(char*)"service_name", /* Member name */
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
            {(char*)"host_url", /* Member name */
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

        static DDS_TypeCode ParticipantData_g_tc = {{
            DDS_TK_STRUCT,                /* Kind */
            DDS_BOOLEAN_FALSE,            /* Ignored */
            -1,                           /*Ignored*/
            (char*)"ParticipantData",     /* Name */
            NULL,                         /* Ignored */
            0,                            /* Ignored */
            0,                            /* Ignored */
            NULL,                         /* Ignored */
            2,                            /* Number of members */
            ParticipantData_g_tc_members, /* Members */
            DDS_VM_NONE,                  /* Ignored */
            RTICdrTypeCodeAnnotations_INITIALIZER,
            DDS_BOOLEAN_TRUE, /* _isCopyable */
            NULL,             /* _sampleAccessInfo: assigned later */
            NULL              /* _typePlugin: assigned later */
        }};                   /* Type code for ParticipantData*/

        if (is_initialized) {
            return &ParticipantData_g_tc;
        }

        ParticipantData_g_tc_service_name_string = initialize_string_typecode((255L));
        ParticipantData_g_tc_host_url_string = initialize_string_typecode((255L));

        ParticipantData_g_tc._data._annotations._allowedDataRepresentationMask = 5;

        ParticipantData_g_tc_members[0]._representation._typeCode =
            (RTICdrTypeCode*)&ParticipantData_g_tc_service_name_string;
        ParticipantData_g_tc_members[1]._representation._typeCode =
            (RTICdrTypeCode*)&ParticipantData_g_tc_host_url_string;

        /* Initialize the values for member annotations. */
        ParticipantData_g_tc_members[0]._annotations._defaultValue._d = RTI_XCDR_TK_STRING;
        ParticipantData_g_tc_members[0]._annotations._defaultValue._u.string_value = (DDS_Char*)"";

        ParticipantData_g_tc_members[1]._annotations._defaultValue._d = RTI_XCDR_TK_STRING;
        ParticipantData_g_tc_members[1]._annotations._defaultValue._u.string_value = (DDS_Char*)"";

        ParticipantData_g_tc._data._sampleAccessInfo = sample_access_info();
        ParticipantData_g_tc._data._typePlugin = type_plugin_info();

        is_initialized = RTI_TRUE;

        return &ParticipantData_g_tc;
    }

    static RTIXCdrSampleAccessInfo* sample_access_info()
    {
        static RTIBool is_initialized = RTI_FALSE;

        ParticipantData* sample;

        static RTIXCdrMemberAccessInfo ParticipantData_g_memberAccessInfos[2] = {
            RTIXCdrMemberAccessInfo_INITIALIZER};

        static RTIXCdrSampleAccessInfo ParticipantData_g_sampleAccessInfo =
            RTIXCdrSampleAccessInfo_INITIALIZER;

        if (is_initialized) {
            return (RTIXCdrSampleAccessInfo*)&ParticipantData_g_sampleAccessInfo;
        }

        RTIXCdrHeap_allocateStruct(&sample, ParticipantData);
        if (sample == NULL) {
            return NULL;
        }

        ParticipantData_g_memberAccessInfos[0].bindingMemberValueOffset[0] =
            (RTIXCdrUnsignedLong)((char*)&sample->service_name() - (char*)sample);

        ParticipantData_g_memberAccessInfos[1].bindingMemberValueOffset[0] =
            (RTIXCdrUnsignedLong)((char*)&sample->host_url() - (char*)sample);

        ParticipantData_g_sampleAccessInfo.memberAccessInfos = ParticipantData_g_memberAccessInfos;

        {
            size_t candidateTypeSize = sizeof(ParticipantData);

            if (candidateTypeSize > RTIXCdrLong_MAX) {
                ParticipantData_g_sampleAccessInfo.typeSize[0] = RTIXCdrLong_MAX;
            }
            else {
                ParticipantData_g_sampleAccessInfo.typeSize[0] =
                    (RTIXCdrUnsignedLong)candidateTypeSize;
            }
        }

        ParticipantData_g_sampleAccessInfo.useGetMemberValueOnlyWithRef = RTI_XCDR_TRUE;

        ParticipantData_g_sampleAccessInfo.getMemberValuePointerFcn =
            interpreter::get_aggregation_value_pointer<ParticipantData>;

        ParticipantData_g_sampleAccessInfo.languageBinding = RTI_XCDR_TYPE_BINDING_CPP_11_STL;

        RTIXCdrHeap_freeStruct(sample);
        is_initialized = RTI_TRUE;
        return (RTIXCdrSampleAccessInfo*)&ParticipantData_g_sampleAccessInfo;
    }

    static RTIXCdrTypePlugin* type_plugin_info()
    {
        static RTIXCdrTypePlugin ParticipantData_g_typePlugin = {
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

        return &ParticipantData_g_typePlugin;
    }
}; // native_type_code
#endif

const ::dds::core::xtypes::StructType& dynamic_type<ParticipantData>::get()
{
    return static_cast<const ::dds::core::xtypes::StructType&>(
        ::rti::core::native_conversions::cast_from_native<::dds::core::xtypes::DynamicType>(
            *(native_type_code<ParticipantData>::get())));
}

#ifndef NDDS_STANDALONE_TYPE
const ServiceDiscoveryResponseType default_enumerator<ServiceDiscoveryResponseType>::value =
    ServiceDiscoveryResponseType::ResponseDiscover;
template <>
struct native_type_code<ServiceDiscoveryResponseType> {
    static DDS_TypeCode* get()
    {
        using namespace ::rti::topic::interpreter;

        static RTIBool is_initialized = RTI_FALSE;

        static DDS_TypeCode_Member ServiceDiscoveryResponseType_g_tc_members[3] = {

            {(char*)"ResponseDiscover", /* Member name */
             {
                 0,                 /* Ignored */
                 DDS_BOOLEAN_FALSE, /* Is a pointer? */
                 -1,                /* Bitfield bits */
                 NULL               /* Member type code is assigned later */
             },
             static_cast<int>(ServiceDiscoveryResponseType::ResponseDiscover),
             0,                       /* Ignored */
             0,                       /* Ignored */
             NULL,                    /* Ignored */
             RTI_CDR_REQUIRED_MEMBER, /* Is a key? */
             DDS_PRIVATE_MEMBER,      /* Member visibility */

             1,
             NULL, /* Ignored */
             RTICdrTypeCodeAnnotations_INITIALIZER},
            {(char*)"ResponseBye", /* Member name */
             {
                 0,                 /* Ignored */
                 DDS_BOOLEAN_FALSE, /* Is a pointer? */
                 -1,                /* Bitfield bits */
                 NULL               /* Member type code is assigned later */
             },
             static_cast<int>(ServiceDiscoveryResponseType::ResponseBye),
             0,                       /* Ignored */
             0,                       /* Ignored */
             NULL,                    /* Ignored */
             RTI_CDR_REQUIRED_MEMBER, /* Is a key? */
             DDS_PRIVATE_MEMBER,      /* Member visibility */

             1,
             NULL, /* Ignored */
             RTICdrTypeCodeAnnotations_INITIALIZER},
            {(char*)"ResponseAlive", /* Member name */
             {
                 0,                 /* Ignored */
                 DDS_BOOLEAN_FALSE, /* Is a pointer? */
                 -1,                /* Bitfield bits */
                 NULL               /* Member type code is assigned later */
             },
             static_cast<int>(ServiceDiscoveryResponseType::ResponseAlive),
             0,                       /* Ignored */
             0,                       /* Ignored */
             NULL,                    /* Ignored */
             RTI_CDR_REQUIRED_MEMBER, /* Is a key? */
             DDS_PRIVATE_MEMBER,      /* Member visibility */

             1,
             NULL, /* Ignored */
             RTICdrTypeCodeAnnotations_INITIALIZER}};

        static DDS_TypeCode ServiceDiscoveryResponseType_g_tc = {{
            DDS_TK_ENUM,                               /* Kind */
            DDS_BOOLEAN_FALSE,                         /* Ignored */
            -1,                                        /*Ignored*/
            (char*)"ServiceDiscoveryResponseType",     /* Name */
            NULL,                                      /* Base class type code is assigned later */
            0,                                         /* Ignored */
            0,                                         /* Ignored */
            NULL,                                      /* Ignored */
            3,                                         /* Number of members */
            ServiceDiscoveryResponseType_g_tc_members, /* Members */
            DDS_VM_NONE,                               /* Type Modifier */
            RTICdrTypeCodeAnnotations_INITIALIZER,
            DDS_BOOLEAN_TRUE, /* _isCopyable */
            NULL,             /* _sampleAccessInfo: assigned later */
            NULL              /* _typePlugin: assigned later */
        }};                   /* Type code for ServiceDiscoveryResponseType*/

        if (is_initialized) {
            return &ServiceDiscoveryResponseType_g_tc;
        }

        ServiceDiscoveryResponseType_g_tc._data._annotations._allowedDataRepresentationMask = 5;

        /* Initialize the values for annotations. */
        ServiceDiscoveryResponseType_g_tc._data._annotations._defaultValue._d = RTI_XCDR_TK_ENUM;
        ServiceDiscoveryResponseType_g_tc._data._annotations._defaultValue._u.long_value = 0;

        ServiceDiscoveryResponseType_g_tc._data._sampleAccessInfo = sample_access_info();
        ServiceDiscoveryResponseType_g_tc._data._typePlugin = type_plugin_info();

        is_initialized = RTI_TRUE;

        return &ServiceDiscoveryResponseType_g_tc;
    }

    static RTIXCdrSampleAccessInfo* sample_access_info()
    {
        static RTIBool is_initialized = RTI_FALSE;

        static RTIXCdrMemberAccessInfo ServiceDiscoveryResponseType_g_memberAccessInfos[1] = {
            RTIXCdrMemberAccessInfo_INITIALIZER};

        static RTIXCdrSampleAccessInfo ServiceDiscoveryResponseType_g_sampleAccessInfo =
            RTIXCdrSampleAccessInfo_INITIALIZER;

        if (is_initialized) {
            return (RTIXCdrSampleAccessInfo*)&ServiceDiscoveryResponseType_g_sampleAccessInfo;
        }

        ServiceDiscoveryResponseType_g_memberAccessInfos[0].bindingMemberValueOffset[0] = 0;

        ServiceDiscoveryResponseType_g_sampleAccessInfo.memberAccessInfos =
            ServiceDiscoveryResponseType_g_memberAccessInfos;

        {
            size_t candidateTypeSize = sizeof(ServiceDiscoveryResponseType);

            if (candidateTypeSize > RTIXCdrLong_MAX) {
                ServiceDiscoveryResponseType_g_sampleAccessInfo.typeSize[0] = RTIXCdrLong_MAX;
            }
            else {
                ServiceDiscoveryResponseType_g_sampleAccessInfo.typeSize[0] =
                    (RTIXCdrUnsignedLong)candidateTypeSize;
            }
        }

        ServiceDiscoveryResponseType_g_sampleAccessInfo.useGetMemberValueOnlyWithRef =
            RTI_XCDR_TRUE;

        ServiceDiscoveryResponseType_g_sampleAccessInfo.getMemberValuePointerFcn =
            interpreter::get_aggregation_value_pointer<ServiceDiscoveryResponseType>;

        ServiceDiscoveryResponseType_g_sampleAccessInfo.languageBinding =
            RTI_XCDR_TYPE_BINDING_CPP_11_STL;

        is_initialized = RTI_TRUE;
        return (RTIXCdrSampleAccessInfo*)&ServiceDiscoveryResponseType_g_sampleAccessInfo;
    }

    static RTIXCdrTypePlugin* type_plugin_info()
    {
        static RTIXCdrTypePlugin ServiceDiscoveryResponseType_g_typePlugin = {
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

        return &ServiceDiscoveryResponseType_g_typePlugin;
    }
}; // native_type_code
#endif

const ::dds::core::xtypes::EnumType& dynamic_type<ServiceDiscoveryResponseType>::get()
{
    return static_cast<const ::dds::core::xtypes::EnumType&>(
        ::rti::core::native_conversions::cast_from_native<::dds::core::xtypes::DynamicType>(
            *(native_type_code<ServiceDiscoveryResponseType>::get())));
}

#ifndef NDDS_STANDALONE_TYPE
template <>
struct native_type_code<DdsServiceDiscovery> {
    static DDS_TypeCode* get()
    {
        using namespace ::rti::topic::interpreter;

        static RTIBool is_initialized = RTI_FALSE;

        static DDS_TypeCode DdsServiceDiscovery_g_tc_id_string;

        static DDS_TypeCode_Member DdsServiceDiscovery_g_tc_members[3] = {

            {(char*)"id", /* Member name */
             {
                 0,                 /* Representation ID */
                 DDS_BOOLEAN_FALSE, /* Is a pointer? */
                 -1,                /* Bitfield bits */
                 NULL               /* Member type code is assigned later */
             },
             0,                  /* Ignored */
             0,                  /* Ignored */
             0,                  /* Ignored */
             NULL,               /* Ignored */
             RTI_CDR_KEY_MEMBER, /* Is a key? */
             DDS_PUBLIC_MEMBER,  /* Member visibility */
             1,
             NULL, /* Ignored */
             RTICdrTypeCodeAnnotations_INITIALIZER},
            {(char*)"content", /* Member name */
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
            {(char*)"response_type", /* Member name */
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

        static DDS_TypeCode DdsServiceDiscovery_g_tc = {{
            DDS_TK_STRUCT,                    /* Kind */
            DDS_BOOLEAN_FALSE,                /* Ignored */
            -1,                               /*Ignored*/
            (char*)"DdsServiceDiscovery",     /* Name */
            NULL,                             /* Ignored */
            0,                                /* Ignored */
            0,                                /* Ignored */
            NULL,                             /* Ignored */
            3,                                /* Number of members */
            DdsServiceDiscovery_g_tc_members, /* Members */
            DDS_VM_NONE,                      /* Ignored */
            RTICdrTypeCodeAnnotations_INITIALIZER,
            DDS_BOOLEAN_TRUE, /* _isCopyable */
            NULL,             /* _sampleAccessInfo: assigned later */
            NULL              /* _typePlugin: assigned later */
        }};                   /* Type code for DdsServiceDiscovery*/

        if (is_initialized) {
            return &DdsServiceDiscovery_g_tc;
        }

        DdsServiceDiscovery_g_tc_id_string = initialize_string_typecode((255L));

        DdsServiceDiscovery_g_tc._data._annotations._allowedDataRepresentationMask = 5;

        DdsServiceDiscovery_g_tc_members[0]._representation._typeCode =
            (RTICdrTypeCode*)&DdsServiceDiscovery_g_tc_id_string;
        DdsServiceDiscovery_g_tc_members[1]._representation._typeCode =
            (RTICdrTypeCode*)&::rti::topic::dynamic_type<ParticipantData>::get().native();
        DdsServiceDiscovery_g_tc_members[2]._representation._typeCode =
            (RTICdrTypeCode*)&::rti::topic::dynamic_type<ServiceDiscoveryResponseType>::get()
                .native();

        /* Initialize the values for member annotations. */
        DdsServiceDiscovery_g_tc_members[0]._annotations._defaultValue._d = RTI_XCDR_TK_STRING;
        DdsServiceDiscovery_g_tc_members[0]._annotations._defaultValue._u.string_value =
            (DDS_Char*)"";

        DdsServiceDiscovery_g_tc_members[2]._annotations._defaultValue._d = RTI_XCDR_TK_ENUM;
        DdsServiceDiscovery_g_tc_members[2]._annotations._defaultValue._u.enumerated_value = 0;

        DdsServiceDiscovery_g_tc._data._sampleAccessInfo = sample_access_info();
        DdsServiceDiscovery_g_tc._data._typePlugin = type_plugin_info();

        is_initialized = RTI_TRUE;

        return &DdsServiceDiscovery_g_tc;
    }

    static RTIXCdrSampleAccessInfo* sample_access_info()
    {
        static RTIBool is_initialized = RTI_FALSE;

        DdsServiceDiscovery* sample;

        static RTIXCdrMemberAccessInfo DdsServiceDiscovery_g_memberAccessInfos[3] = {
            RTIXCdrMemberAccessInfo_INITIALIZER};

        static RTIXCdrSampleAccessInfo DdsServiceDiscovery_g_sampleAccessInfo =
            RTIXCdrSampleAccessInfo_INITIALIZER;

        if (is_initialized) {
            return (RTIXCdrSampleAccessInfo*)&DdsServiceDiscovery_g_sampleAccessInfo;
        }

        RTIXCdrHeap_allocateStruct(&sample, DdsServiceDiscovery);
        if (sample == NULL) {
            return NULL;
        }

        DdsServiceDiscovery_g_memberAccessInfos[0].bindingMemberValueOffset[0] =
            (RTIXCdrUnsignedLong)((char*)&sample->id() - (char*)sample);

        DdsServiceDiscovery_g_memberAccessInfos[1].bindingMemberValueOffset[0] =
            (RTIXCdrUnsignedLong)((char*)&sample->content() - (char*)sample);

        DdsServiceDiscovery_g_memberAccessInfos[2].bindingMemberValueOffset[0] =
            (RTIXCdrUnsignedLong)((char*)&sample->response_type() - (char*)sample);

        DdsServiceDiscovery_g_sampleAccessInfo.memberAccessInfos =
            DdsServiceDiscovery_g_memberAccessInfos;

        {
            size_t candidateTypeSize = sizeof(DdsServiceDiscovery);

            if (candidateTypeSize > RTIXCdrLong_MAX) {
                DdsServiceDiscovery_g_sampleAccessInfo.typeSize[0] = RTIXCdrLong_MAX;
            }
            else {
                DdsServiceDiscovery_g_sampleAccessInfo.typeSize[0] =
                    (RTIXCdrUnsignedLong)candidateTypeSize;
            }
        }

        DdsServiceDiscovery_g_sampleAccessInfo.useGetMemberValueOnlyWithRef = RTI_XCDR_TRUE;

        DdsServiceDiscovery_g_sampleAccessInfo.getMemberValuePointerFcn =
            interpreter::get_aggregation_value_pointer<DdsServiceDiscovery>;

        DdsServiceDiscovery_g_sampleAccessInfo.languageBinding = RTI_XCDR_TYPE_BINDING_CPP_11_STL;

        RTIXCdrHeap_freeStruct(sample);
        is_initialized = RTI_TRUE;
        return (RTIXCdrSampleAccessInfo*)&DdsServiceDiscovery_g_sampleAccessInfo;
    }

    static RTIXCdrTypePlugin* type_plugin_info()
    {
        static RTIXCdrTypePlugin DdsServiceDiscovery_g_typePlugin = {
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

        return &DdsServiceDiscovery_g_typePlugin;
    }
}; // native_type_code
#endif

const ::dds::core::xtypes::StructType& dynamic_type<DdsServiceDiscovery>::get()
{
    return static_cast<const ::dds::core::xtypes::StructType&>(
        ::rti::core::native_conversions::cast_from_native<::dds::core::xtypes::DynamicType>(
            *(native_type_code<DdsServiceDiscovery>::get())));
}

} // namespace topic
} // namespace rti

namespace dds {
namespace topic {
void topic_type_support<ParticipantData>::register_type(
    ::dds::domain::DomainParticipant& participant, const std::string& type_name)
{
    ::rti::domain::register_type_plugin(
        participant, type_name, ParticipantDataPlugin_new, ParticipantDataPlugin_delete);
}

std::vector<char>& topic_type_support<ParticipantData>::to_cdr_buffer(
    std::vector<char>& buffer,
    const ParticipantData& sample,
    ::dds::core::policy::DataRepresentationId representation)
{
    // First get the length of the buffer
    unsigned int length = 0;
    RTIBool ok =
        ParticipantDataPlugin_serialize_to_cdr_buffer(NULL, &length, &sample, representation);
    ::rti::core::check_return_code(ok ? DDS_RETCODE_OK : DDS_RETCODE_ERROR,
                                   "Failed to calculate cdr buffer size");

    // Create a vector with that size and copy the cdr buffer into it
    buffer.resize(length);
    ok =
        ParticipantDataPlugin_serialize_to_cdr_buffer(&buffer[0], &length, &sample, representation);
    ::rti::core::check_return_code(ok ? DDS_RETCODE_OK : DDS_RETCODE_ERROR,
                                   "Failed to copy cdr buffer");

    return buffer;
}

void topic_type_support<ParticipantData>::from_cdr_buffer(ParticipantData& sample,
                                                          const std::vector<char>& buffer)
{
    RTIBool ok = ParticipantDataPlugin_deserialize_from_cdr_buffer(
        &sample, &buffer[0], static_cast<unsigned int>(buffer.size()));
    ::rti::core::check_return_code(ok ? DDS_RETCODE_OK : DDS_RETCODE_ERROR,
                                   "Failed to create ParticipantData from cdr buffer");
}

void topic_type_support<ParticipantData>::reset_sample(ParticipantData& sample)
{
    sample.service_name("");
    sample.host_url("");
}

void topic_type_support<ParticipantData>::allocate_sample(ParticipantData& sample, int, int)
{
    ::rti::topic::allocate_sample(sample.service_name(), -1, 255L);
    ::rti::topic::allocate_sample(sample.host_url(), -1, 255L);
}

void topic_type_support<DdsServiceDiscovery>::register_type(
    ::dds::domain::DomainParticipant& participant, const std::string& type_name)
{
    ::rti::domain::register_type_plugin(
        participant, type_name, DdsServiceDiscoveryPlugin_new, DdsServiceDiscoveryPlugin_delete);
}

std::vector<char>& topic_type_support<DdsServiceDiscovery>::to_cdr_buffer(
    std::vector<char>& buffer,
    const DdsServiceDiscovery& sample,
    ::dds::core::policy::DataRepresentationId representation)
{
    // First get the length of the buffer
    unsigned int length = 0;
    RTIBool ok =
        DdsServiceDiscoveryPlugin_serialize_to_cdr_buffer(NULL, &length, &sample, representation);
    ::rti::core::check_return_code(ok ? DDS_RETCODE_OK : DDS_RETCODE_ERROR,
                                   "Failed to calculate cdr buffer size");

    // Create a vector with that size and copy the cdr buffer into it
    buffer.resize(length);
    ok = DdsServiceDiscoveryPlugin_serialize_to_cdr_buffer(
        &buffer[0], &length, &sample, representation);
    ::rti::core::check_return_code(ok ? DDS_RETCODE_OK : DDS_RETCODE_ERROR,
                                   "Failed to copy cdr buffer");

    return buffer;
}

void topic_type_support<DdsServiceDiscovery>::from_cdr_buffer(DdsServiceDiscovery& sample,
                                                              const std::vector<char>& buffer)
{
    RTIBool ok = DdsServiceDiscoveryPlugin_deserialize_from_cdr_buffer(
        &sample, &buffer[0], static_cast<unsigned int>(buffer.size()));
    ::rti::core::check_return_code(ok ? DDS_RETCODE_OK : DDS_RETCODE_ERROR,
                                   "Failed to create DdsServiceDiscovery from cdr buffer");
}

void topic_type_support<DdsServiceDiscovery>::reset_sample(DdsServiceDiscovery& sample)
{
    sample.id("");
    ::rti::topic::reset_sample(sample.content());
    sample.response_type(ServiceDiscoveryResponseType::ResponseDiscover);
}

void topic_type_support<DdsServiceDiscovery>::allocate_sample(DdsServiceDiscovery& sample, int, int)
{
    ::rti::topic::allocate_sample(sample.id(), -1, 255L);
    ::rti::topic::allocate_sample(sample.content(), -1, -1);
    ::rti::topic::allocate_sample(sample.response_type(), -1, -1);
}

} // namespace topic
} // namespace dds
