

/*
WARNING: THIS FILE IS AUTO-GENERATED. DO NOT MODIFY.

This file was generated from DdsServiceDiscoveryTopic.idl
using RTI Code Generator (rtiddsgen) version 3.1.0.
The rtiddsgen tool is part of the RTI Connext DDS distribution.
For more information, type 'rtiddsgen -help' at a command shell
or consult the Code Generator User's Manual.
*/

#ifndef DdsServiceDiscoveryTopic_48526812_hpp
#define DdsServiceDiscoveryTopic_48526812_hpp

// FIXME: Remove after ODAUTIL-615 is fixed
#ifdef _WIN32
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif // NOMINMAX
#endif     // _WIN32

#include <iosfwd>

#if (defined(RTI_WIN32) || defined(RTI_WINCE) || defined(RTI_INTIME)) &&                           \
    defined(NDDS_USER_DLL_EXPORT)
    /* If the code is building on Windows, start exporting symbols.
     */
    #undef RTIUSERDllExport
    #define RTIUSERDllExport __declspec(dllexport)
#endif

// clang-format off
#include <dds/domain/DomainParticipant.hpp>
#include <dds/topic/TopicTraits.hpp>
#include <dds/core/SafeEnumeration.hpp>
#include <dds/core/String.hpp>
#include <dds/core/array.hpp>
#include <dds/core/vector.hpp>
#include <dds/core/Optional.hpp>
#include <dds/core/xtypes/DynamicType.hpp>
#include <dds/core/xtypes/StructType.hpp>
#include <dds/core/xtypes/UnionType.hpp>
#include <dds/core/xtypes/EnumType.hpp>
#include <dds/core/xtypes/AliasType.hpp>
#include <rti/core/array.hpp>
#include <rti/core/BoundedSequence.hpp>
#include <rti/util/StreamFlagSaver.hpp>
#include <rti/domain/PluginSupport.hpp>
#include <rti/core/LongDouble.hpp>
#include <dds/core/External.hpp>
#include <rti/core/Pointer.hpp>
#include <rti/topic/TopicTraits.hpp>
// clang-format on

#if (defined(RTI_WIN32) || defined(RTI_WINCE) || defined(RTI_INTIME)) &&                           \
    defined(NDDS_USER_DLL_EXPORT)
    /* If the code is building on Windows, stop exporting symbols.
     */
    #undef RTIUSERDllExport
    #define RTIUSERDllExport
#endif

#if (defined(RTI_WIN32) || defined(RTI_WINCE) || defined(RTI_INTIME)) &&                           \
    defined(NDDS_USER_DLL_EXPORT)
    /* If the code is building on Windows, start exporting symbols.
     */
    #undef NDDSUSERDllExport
    #define NDDSUSERDllExport __declspec(dllexport)
#endif

class NDDSUSERDllExport ParticipantData {
public:
    ParticipantData();

    ParticipantData(const std::string& service_name, const std::string& host_url);

#ifdef RTI_CXX11_RVALUE_REFERENCES
    #ifndef RTI_CXX11_NO_IMPLICIT_MOVE_OPERATIONS
    ParticipantData(ParticipantData&&) = default;
    ParticipantData& operator=(ParticipantData&&) = default;
    ParticipantData& operator=(const ParticipantData&) = default;
    ParticipantData(const ParticipantData&) = default;
    #else
    ParticipantData(ParticipantData&& other_) OMG_NOEXCEPT;
    ParticipantData& operator=(ParticipantData&& other_) OMG_NOEXCEPT;
    #endif
#endif

    std::string& service_name() OMG_NOEXCEPT
    {
        return m_service_name_;
    }

    const std::string& service_name() const OMG_NOEXCEPT
    {
        return m_service_name_;
    }

    void service_name(const std::string& value)
    {
        m_service_name_ = value;
    }

    void service_name(std::string&& value)
    {
        m_service_name_ = std::move(value);
    }

    std::string& host_url() OMG_NOEXCEPT
    {
        return m_host_url_;
    }

    const std::string& host_url() const OMG_NOEXCEPT
    {
        return m_host_url_;
    }

    void host_url(const std::string& value)
    {
        m_host_url_ = value;
    }

    void host_url(std::string&& value)
    {
        m_host_url_ = std::move(value);
    }

    bool operator==(const ParticipantData& other_) const;
    bool operator!=(const ParticipantData& other_) const;

    void swap(ParticipantData& other_) OMG_NOEXCEPT;

private:
    std::string m_service_name_;
    std::string m_host_url_;
};

inline void swap(ParticipantData& a, ParticipantData& b) OMG_NOEXCEPT
{
    a.swap(b);
}

NDDSUSERDllExport std::ostream& operator<<(std::ostream& o, const ParticipantData& sample);

enum class ServiceDiscoveryResponseType
{
    ResponseDiscover,
    ResponseBye,
    ResponseAlive
};

NDDSUSERDllExport std::ostream& operator<<(std::ostream& o,
                                           const ServiceDiscoveryResponseType& sample);

class NDDSUSERDllExport DdsServiceDiscovery {
public:
    DdsServiceDiscovery();

    DdsServiceDiscovery(const std::string& id,
                        const ParticipantData& content,
                        const ServiceDiscoveryResponseType& response_type);

#ifdef RTI_CXX11_RVALUE_REFERENCES
    #ifndef RTI_CXX11_NO_IMPLICIT_MOVE_OPERATIONS
    DdsServiceDiscovery(DdsServiceDiscovery&&) = default;
    DdsServiceDiscovery& operator=(DdsServiceDiscovery&&) = default;
    DdsServiceDiscovery& operator=(const DdsServiceDiscovery&) = default;
    DdsServiceDiscovery(const DdsServiceDiscovery&) = default;
    #else
    DdsServiceDiscovery(DdsServiceDiscovery&& other_) OMG_NOEXCEPT;
    DdsServiceDiscovery& operator=(DdsServiceDiscovery&& other_) OMG_NOEXCEPT;
    #endif
#endif

    std::string& id() OMG_NOEXCEPT
    {
        return m_id_;
    }

    const std::string& id() const OMG_NOEXCEPT
    {
        return m_id_;
    }

    void id(const std::string& value)
    {
        m_id_ = value;
    }

    void id(std::string&& value)
    {
        m_id_ = std::move(value);
    }

    ParticipantData& content() OMG_NOEXCEPT
    {
        return m_content_;
    }

    const ParticipantData& content() const OMG_NOEXCEPT
    {
        return m_content_;
    }

    void content(const ParticipantData& value)
    {
        m_content_ = value;
    }

    void content(ParticipantData&& value)
    {
        m_content_ = std::move(value);
    }

    ServiceDiscoveryResponseType& response_type() OMG_NOEXCEPT
    {
        return m_response_type_;
    }

    const ServiceDiscoveryResponseType& response_type() const OMG_NOEXCEPT
    {
        return m_response_type_;
    }

    void response_type(const ServiceDiscoveryResponseType& value)
    {
        m_response_type_ = value;
    }

    void response_type(ServiceDiscoveryResponseType&& value)
    {
        m_response_type_ = std::move(value);
    }

    bool operator==(const DdsServiceDiscovery& other_) const;
    bool operator!=(const DdsServiceDiscovery& other_) const;

    void swap(DdsServiceDiscovery& other_) OMG_NOEXCEPT;

private:
    std::string m_id_;
    ParticipantData m_content_;
    ServiceDiscoveryResponseType m_response_type_;
};

inline void swap(DdsServiceDiscovery& a, DdsServiceDiscovery& b) OMG_NOEXCEPT
{
    a.swap(b);
}

NDDSUSERDllExport std::ostream& operator<<(std::ostream& o, const DdsServiceDiscovery& sample);

namespace dds {
namespace topic {

template <>
struct topic_type_name<ParticipantData> {
    NDDSUSERDllExport static std::string value()
    {
        return "ParticipantData";
    }
};

template <>
struct is_topic_type<ParticipantData> : public ::dds::core::true_type {
};

template <>
struct topic_type_support<ParticipantData> {
    NDDSUSERDllExport static void register_type(::dds::domain::DomainParticipant& participant,
                                                const std::string& type_name);

    NDDSUSERDllExport static std::vector<char>& to_cdr_buffer(
        std::vector<char>& buffer,
        const ParticipantData& sample,
        ::dds::core::policy::DataRepresentationId representation =
            ::dds::core::policy::DataRepresentation::auto_id());

    NDDSUSERDllExport static void from_cdr_buffer(ParticipantData& sample,
                                                  const std::vector<char>& buffer);
    NDDSUSERDllExport static void reset_sample(ParticipantData& sample);

    NDDSUSERDllExport static void allocate_sample(ParticipantData& sample, int, int);

    static const ::rti::topic::TypePluginKind::type type_plugin_kind =
        ::rti::topic::TypePluginKind::STL;
};

template <>
struct topic_type_name<DdsServiceDiscovery> {
    NDDSUSERDllExport static std::string value()
    {
        return "DdsServiceDiscovery";
    }
};

template <>
struct is_topic_type<DdsServiceDiscovery> : public ::dds::core::true_type {
};

template <>
struct topic_type_support<DdsServiceDiscovery> {
    NDDSUSERDllExport static void register_type(::dds::domain::DomainParticipant& participant,
                                                const std::string& type_name);

    NDDSUSERDllExport static std::vector<char>& to_cdr_buffer(
        std::vector<char>& buffer,
        const DdsServiceDiscovery& sample,
        ::dds::core::policy::DataRepresentationId representation =
            ::dds::core::policy::DataRepresentation::auto_id());

    NDDSUSERDllExport static void from_cdr_buffer(DdsServiceDiscovery& sample,
                                                  const std::vector<char>& buffer);
    NDDSUSERDllExport static void reset_sample(DdsServiceDiscovery& sample);

    NDDSUSERDllExport static void allocate_sample(DdsServiceDiscovery& sample, int, int);

    static const ::rti::topic::TypePluginKind::type type_plugin_kind =
        ::rti::topic::TypePluginKind::STL;
};

} // namespace topic
} // namespace dds

namespace rti {
namespace topic {
#ifndef NDDS_STANDALONE_TYPE
template <>
struct dynamic_type<ParticipantData> {
    typedef ::dds::core::xtypes::StructType type;
    NDDSUSERDllExport static const ::dds::core::xtypes::StructType& get();
};
#endif

template <>
struct extensibility<ParticipantData> {
    static const ::dds::core::xtypes::ExtensibilityKind::type kind =
        ::dds::core::xtypes::ExtensibilityKind::EXTENSIBLE;
};

#ifndef NDDS_STANDALONE_TYPE
template <>
struct default_enumerator<ServiceDiscoveryResponseType> {
    static const ServiceDiscoveryResponseType value;
};

template <>
struct dynamic_type<ServiceDiscoveryResponseType> {
    typedef ::dds::core::xtypes::EnumType type;
    NDDSUSERDllExport static const ::dds::core::xtypes::EnumType& get();
};
#endif

template <>
struct extensibility<ServiceDiscoveryResponseType> {
    static const ::dds::core::xtypes::ExtensibilityKind::type kind =
        ::dds::core::xtypes::ExtensibilityKind::EXTENSIBLE;
};

#ifndef NDDS_STANDALONE_TYPE
template <>
struct dynamic_type<DdsServiceDiscovery> {
    typedef ::dds::core::xtypes::StructType type;
    NDDSUSERDllExport static const ::dds::core::xtypes::StructType& get();
};
#endif

template <>
struct extensibility<DdsServiceDiscovery> {
    static const ::dds::core::xtypes::ExtensibilityKind::type kind =
        ::dds::core::xtypes::ExtensibilityKind::EXTENSIBLE;
};

} // namespace topic
} // namespace rti

#if (defined(RTI_WIN32) || defined(RTI_WINCE) || defined(RTI_INTIME)) &&                           \
    defined(NDDS_USER_DLL_EXPORT)
    /* If the code is building on Windows, stop exporting symbols.
     */
    #undef NDDSUSERDllExport
    #define NDDSUSERDllExport
#endif

#endif // DdsServiceDiscoveryTopic_48526812_hpp
