

/*
WARNING: THIS FILE IS AUTO-GENERATED. DO NOT MODIFY.

This file was generated from DdsServiceDiscoveryTopic.idl
using RTI Code Generator (rtiddsgen) version 3.1.0.
The rtiddsgen tool is part of the RTI Connext DDS distribution.
For more information, type 'rtiddsgen -help' at a command shell
or consult the Code Generator User's Manual.
*/

#ifndef DdsServiceDiscoveryTopicPlugin_48526812_h
#define DdsServiceDiscoveryTopicPlugin_48526812_h

#include "DdsServiceDiscoveryTopic.hpp"

struct RTICdrStream;

#ifndef pres_typePlugin_h
    #include <pres/pres_typePlugin.h>
#endif

#if (defined(RTI_WIN32) || defined(RTI_WINCE) || defined(RTI_INTIME)) &&                           \
    defined(NDDS_USER_DLL_EXPORT)
    /* If the code is building on Windows, start exporting symbols.
     */
    #undef NDDSUSERDllExport
    #define NDDSUSERDllExport __declspec(dllexport)
#endif

#define ParticipantDataPlugin_get_sample PRESTypePluginDefaultEndpointData_getSample

#define ParticipantDataPlugin_get_buffer PRESTypePluginDefaultEndpointData_getBuffer
#define ParticipantDataPlugin_return_buffer PRESTypePluginDefaultEndpointData_returnBuffer

#define ParticipantDataPlugin_create_sample PRESTypePluginDefaultEndpointData_createSample
#define ParticipantDataPlugin_destroy_sample PRESTypePluginDefaultEndpointData_deleteSample

/* --------------------------------------------------------------------------------------
Support functions:
* -------------------------------------------------------------------------------------- */

NDDSUSERDllExport extern ParticipantData* ParticipantDataPluginSupport_create_data_w_params(
    const struct DDS_TypeAllocationParams_t* alloc_params);

NDDSUSERDllExport extern ParticipantData* ParticipantDataPluginSupport_create_data_ex(
    RTIBool allocate_pointers);

NDDSUSERDllExport extern ParticipantData* ParticipantDataPluginSupport_create_data(void);

NDDSUSERDllExport extern RTIBool ParticipantDataPluginSupport_copy_data(ParticipantData* out,
                                                                        const ParticipantData* in);

NDDSUSERDllExport extern void ParticipantDataPluginSupport_destroy_data_w_params(
    ParticipantData* sample, const struct DDS_TypeDeallocationParams_t* dealloc_params);

NDDSUSERDllExport extern void ParticipantDataPluginSupport_destroy_data_ex(
    ParticipantData* sample, RTIBool deallocate_pointers);

NDDSUSERDllExport extern void ParticipantDataPluginSupport_destroy_data(ParticipantData* sample);

NDDSUSERDllExport extern void ParticipantDataPluginSupport_print_data(const ParticipantData* sample,
                                                                      const char* desc,
                                                                      unsigned int indent);

/* ----------------------------------------------------------------------------
Callback functions:
* ---------------------------------------------------------------------------- */

NDDSUSERDllExport extern PRESTypePluginParticipantData
ParticipantDataPlugin_on_participant_attached(
    void* registration_data,
    const struct PRESTypePluginParticipantInfo* participant_info,
    RTIBool top_level_registration,
    void* container_plugin_context,
    RTICdrTypeCode* typeCode);

NDDSUSERDllExport extern void ParticipantDataPlugin_on_participant_detached(
    PRESTypePluginParticipantData participant_data);

NDDSUSERDllExport extern PRESTypePluginEndpointData ParticipantDataPlugin_on_endpoint_attached(
    PRESTypePluginParticipantData participant_data,
    const struct PRESTypePluginEndpointInfo* endpoint_info,
    RTIBool top_level_registration,
    void* container_plugin_context);

NDDSUSERDllExport extern void ParticipantDataPlugin_on_endpoint_detached(
    PRESTypePluginEndpointData endpoint_data);

NDDSUSERDllExport extern void ParticipantDataPlugin_return_sample(
    PRESTypePluginEndpointData endpoint_data, ParticipantData* sample, void* handle);

NDDSUSERDllExport extern RTIBool ParticipantDataPlugin_copy_sample(
    PRESTypePluginEndpointData endpoint_data, ParticipantData* out, const ParticipantData* in);

/* ----------------------------------------------------------------------------
(De)Serialize functions:
* ------------------------------------------------------------------------- */

NDDSUSERDllExport extern RTIBool ParticipantDataPlugin_serialize_to_cdr_buffer(
    char* buffer,
    unsigned int* length,
    const ParticipantData* sample,
    ::dds::core::policy::DataRepresentationId representation =
        ::dds::core::policy::DataRepresentation::xcdr());

NDDSUSERDllExport extern RTIBool ParticipantDataPlugin_deserialize(
    PRESTypePluginEndpointData endpoint_data,
    ParticipantData** sample,
    RTIBool* drop_sample,
    struct RTICdrStream* stream,
    RTIBool deserialize_encapsulation,
    RTIBool deserialize_sample,
    void* endpoint_plugin_qos);

NDDSUSERDllExport extern RTIBool ParticipantDataPlugin_deserialize_from_cdr_buffer(
    ParticipantData* sample, const char* buffer, unsigned int length);

NDDSUSERDllExport extern unsigned int ParticipantDataPlugin_get_serialized_sample_max_size(
    PRESTypePluginEndpointData endpoint_data,
    RTIBool include_encapsulation,
    RTIEncapsulationId encapsulation_id,
    unsigned int current_alignment);

/* --------------------------------------------------------------------------------------
Key Management functions:
* -------------------------------------------------------------------------------------- */
NDDSUSERDllExport extern PRESTypePluginKeyKind ParticipantDataPlugin_get_key_kind(void);

NDDSUSERDllExport extern unsigned int ParticipantDataPlugin_get_serialized_key_max_size(
    PRESTypePluginEndpointData endpoint_data,
    RTIBool include_encapsulation,
    RTIEncapsulationId encapsulation_id,
    unsigned int current_alignment);

NDDSUSERDllExport extern unsigned int ParticipantDataPlugin_get_serialized_key_max_size_for_keyhash(
    PRESTypePluginEndpointData endpoint_data,
    RTIEncapsulationId encapsulation_id,
    unsigned int current_alignment);

NDDSUSERDllExport extern RTIBool ParticipantDataPlugin_deserialize_key(
    PRESTypePluginEndpointData endpoint_data,
    ParticipantData** sample,
    RTIBool* drop_sample,
    struct RTICdrStream* stream,
    RTIBool deserialize_encapsulation,
    RTIBool deserialize_key,
    void* endpoint_plugin_qos);

/* Plugin Functions */
NDDSUSERDllExport extern struct PRESTypePlugin* ParticipantDataPlugin_new(void);

NDDSUSERDllExport extern void ParticipantDataPlugin_delete(struct PRESTypePlugin*);

/* ----------------------------------------------------------------------------
(De)Serialize functions:
* ------------------------------------------------------------------------- */

NDDSUSERDllExport extern unsigned int
ServiceDiscoveryResponseTypePlugin_get_serialized_sample_max_size(
    PRESTypePluginEndpointData endpoint_data,
    RTIBool include_encapsulation,
    RTIEncapsulationId encapsulation_id,
    unsigned int current_alignment);

/* --------------------------------------------------------------------------------------
Key Management functions:
* -------------------------------------------------------------------------------------- */

NDDSUSERDllExport extern unsigned int
ServiceDiscoveryResponseTypePlugin_get_serialized_key_max_size(
    PRESTypePluginEndpointData endpoint_data,
    RTIBool include_encapsulation,
    RTIEncapsulationId encapsulation_id,
    unsigned int current_alignment);

NDDSUSERDllExport extern unsigned int
ServiceDiscoveryResponseTypePlugin_get_serialized_key_max_size_for_keyhash(
    PRESTypePluginEndpointData endpoint_data,
    RTIEncapsulationId encapsulation_id,
    unsigned int current_alignment);

/* ----------------------------------------------------------------------------
Support functions:
* ---------------------------------------------------------------------------- */

NDDSUSERDllExport extern void ServiceDiscoveryResponseTypePluginSupport_print_data(
    const ServiceDiscoveryResponseType* sample, const char* desc, int indent_level);

/* The type used to store keys for instances of type struct
 * AnotherSimple.
 *
 * By default, this type is struct DdsServiceDiscovery
 * itself. However, if for some reason this choice is not practical for your
 * system (e.g. if sizeof(struct DdsServiceDiscovery)
 * is very large), you may redefine this typedef in terms of another type of
 * your choosing. HOWEVER, if you define the KeyHolder type to be something
 * other than struct AnotherSimple, the
 * following restriction applies: the key of struct
 * DdsServiceDiscovery must consist of a
 * single field of your redefined KeyHolder type and that field must be the
 * first field in struct DdsServiceDiscovery.
 */
typedef class DdsServiceDiscovery DdsServiceDiscoveryKeyHolder;

#define DdsServiceDiscoveryPlugin_get_sample PRESTypePluginDefaultEndpointData_getSample

#define DdsServiceDiscoveryPlugin_get_buffer PRESTypePluginDefaultEndpointData_getBuffer
#define DdsServiceDiscoveryPlugin_return_buffer PRESTypePluginDefaultEndpointData_returnBuffer

#define DdsServiceDiscoveryPlugin_get_key PRESTypePluginDefaultEndpointData_getKey
#define DdsServiceDiscoveryPlugin_return_key PRESTypePluginDefaultEndpointData_returnKey

#define DdsServiceDiscoveryPlugin_create_sample PRESTypePluginDefaultEndpointData_createSample
#define DdsServiceDiscoveryPlugin_destroy_sample PRESTypePluginDefaultEndpointData_deleteSample

/* --------------------------------------------------------------------------------------
Support functions:
* -------------------------------------------------------------------------------------- */

NDDSUSERDllExport extern DdsServiceDiscovery* DdsServiceDiscoveryPluginSupport_create_data_w_params(
    const struct DDS_TypeAllocationParams_t* alloc_params);

NDDSUSERDllExport extern DdsServiceDiscovery* DdsServiceDiscoveryPluginSupport_create_data_ex(
    RTIBool allocate_pointers);

NDDSUSERDllExport extern DdsServiceDiscovery* DdsServiceDiscoveryPluginSupport_create_data(void);

NDDSUSERDllExport extern RTIBool DdsServiceDiscoveryPluginSupport_copy_data(
    DdsServiceDiscovery* out, const DdsServiceDiscovery* in);

NDDSUSERDllExport extern void DdsServiceDiscoveryPluginSupport_destroy_data_w_params(
    DdsServiceDiscovery* sample, const struct DDS_TypeDeallocationParams_t* dealloc_params);

NDDSUSERDllExport extern void DdsServiceDiscoveryPluginSupport_destroy_data_ex(
    DdsServiceDiscovery* sample, RTIBool deallocate_pointers);

NDDSUSERDllExport extern void DdsServiceDiscoveryPluginSupport_destroy_data(
    DdsServiceDiscovery* sample);

NDDSUSERDllExport extern void DdsServiceDiscoveryPluginSupport_print_data(
    const DdsServiceDiscovery* sample, const char* desc, unsigned int indent);

NDDSUSERDllExport extern DdsServiceDiscovery* DdsServiceDiscoveryPluginSupport_create_key_ex(
    RTIBool allocate_pointers);

NDDSUSERDllExport extern DdsServiceDiscovery* DdsServiceDiscoveryPluginSupport_create_key(void);

NDDSUSERDllExport extern void DdsServiceDiscoveryPluginSupport_destroy_key_ex(
    DdsServiceDiscoveryKeyHolder* key, RTIBool deallocate_pointers);

NDDSUSERDllExport extern void DdsServiceDiscoveryPluginSupport_destroy_key(
    DdsServiceDiscoveryKeyHolder* key);

/* ----------------------------------------------------------------------------
Callback functions:
* ---------------------------------------------------------------------------- */

NDDSUSERDllExport extern PRESTypePluginParticipantData
DdsServiceDiscoveryPlugin_on_participant_attached(
    void* registration_data,
    const struct PRESTypePluginParticipantInfo* participant_info,
    RTIBool top_level_registration,
    void* container_plugin_context,
    RTICdrTypeCode* typeCode);

NDDSUSERDllExport extern void DdsServiceDiscoveryPlugin_on_participant_detached(
    PRESTypePluginParticipantData participant_data);

NDDSUSERDllExport extern PRESTypePluginEndpointData DdsServiceDiscoveryPlugin_on_endpoint_attached(
    PRESTypePluginParticipantData participant_data,
    const struct PRESTypePluginEndpointInfo* endpoint_info,
    RTIBool top_level_registration,
    void* container_plugin_context);

NDDSUSERDllExport extern void DdsServiceDiscoveryPlugin_on_endpoint_detached(
    PRESTypePluginEndpointData endpoint_data);

NDDSUSERDllExport extern void DdsServiceDiscoveryPlugin_return_sample(
    PRESTypePluginEndpointData endpoint_data, DdsServiceDiscovery* sample, void* handle);

NDDSUSERDllExport extern RTIBool DdsServiceDiscoveryPlugin_copy_sample(
    PRESTypePluginEndpointData endpoint_data,
    DdsServiceDiscovery* out,
    const DdsServiceDiscovery* in);

/* ----------------------------------------------------------------------------
(De)Serialize functions:
* ------------------------------------------------------------------------- */

NDDSUSERDllExport extern RTIBool DdsServiceDiscoveryPlugin_serialize_to_cdr_buffer(
    char* buffer,
    unsigned int* length,
    const DdsServiceDiscovery* sample,
    ::dds::core::policy::DataRepresentationId representation =
        ::dds::core::policy::DataRepresentation::xcdr());

NDDSUSERDllExport extern RTIBool DdsServiceDiscoveryPlugin_deserialize(
    PRESTypePluginEndpointData endpoint_data,
    DdsServiceDiscovery** sample,
    RTIBool* drop_sample,
    struct RTICdrStream* stream,
    RTIBool deserialize_encapsulation,
    RTIBool deserialize_sample,
    void* endpoint_plugin_qos);

NDDSUSERDllExport extern RTIBool DdsServiceDiscoveryPlugin_deserialize_from_cdr_buffer(
    DdsServiceDiscovery* sample, const char* buffer, unsigned int length);

NDDSUSERDllExport extern unsigned int DdsServiceDiscoveryPlugin_get_serialized_sample_max_size(
    PRESTypePluginEndpointData endpoint_data,
    RTIBool include_encapsulation,
    RTIEncapsulationId encapsulation_id,
    unsigned int current_alignment);

/* --------------------------------------------------------------------------------------
Key Management functions:
* -------------------------------------------------------------------------------------- */
NDDSUSERDllExport extern PRESTypePluginKeyKind DdsServiceDiscoveryPlugin_get_key_kind(void);

NDDSUSERDllExport extern unsigned int DdsServiceDiscoveryPlugin_get_serialized_key_max_size(
    PRESTypePluginEndpointData endpoint_data,
    RTIBool include_encapsulation,
    RTIEncapsulationId encapsulation_id,
    unsigned int current_alignment);

NDDSUSERDllExport extern unsigned int
DdsServiceDiscoveryPlugin_get_serialized_key_max_size_for_keyhash(
    PRESTypePluginEndpointData endpoint_data,
    RTIEncapsulationId encapsulation_id,
    unsigned int current_alignment);

NDDSUSERDllExport extern RTIBool DdsServiceDiscoveryPlugin_deserialize_key(
    PRESTypePluginEndpointData endpoint_data,
    DdsServiceDiscovery** sample,
    RTIBool* drop_sample,
    struct RTICdrStream* stream,
    RTIBool deserialize_encapsulation,
    RTIBool deserialize_key,
    void* endpoint_plugin_qos);

NDDSUSERDllExport extern RTIBool DdsServiceDiscoveryPlugin_instance_to_key(
    PRESTypePluginEndpointData endpoint_data,
    DdsServiceDiscoveryKeyHolder* key,
    const DdsServiceDiscovery* instance);

NDDSUSERDllExport extern RTIBool DdsServiceDiscoveryPlugin_key_to_instance(
    PRESTypePluginEndpointData endpoint_data,
    DdsServiceDiscovery* instance,
    const DdsServiceDiscoveryKeyHolder* key);

NDDSUSERDllExport extern RTIBool DdsServiceDiscoveryPlugin_serialized_sample_to_keyhash(
    PRESTypePluginEndpointData endpoint_data,
    struct RTICdrStream* stream,
    DDS_KeyHash_t* keyhash,
    RTIBool deserialize_encapsulation,
    void* endpoint_plugin_qos);

/* Plugin Functions */
NDDSUSERDllExport extern struct PRESTypePlugin* DdsServiceDiscoveryPlugin_new(void);

NDDSUSERDllExport extern void DdsServiceDiscoveryPlugin_delete(struct PRESTypePlugin*);

#if (defined(RTI_WIN32) || defined(RTI_WINCE) || defined(RTI_INTIME)) &&                           \
    defined(NDDS_USER_DLL_EXPORT)
    /* If the code is building on Windows, stop exporting symbols.
     */
    #undef NDDSUSERDllExport
    #define NDDSUSERDllExport
#endif

#endif /* DdsServiceDiscoveryTopicPlugin_48526812_h */
