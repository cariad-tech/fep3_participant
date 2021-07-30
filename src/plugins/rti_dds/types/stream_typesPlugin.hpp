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


/*
WARNING: THIS FILE IS AUTO-GENERATED. DO NOT MODIFY.

This file was generated from stream_types.idl using "rtiddsgen".
The rtiddsgen tool is part of the RTI Connext distribution.
For more information, type 'rtiddsgen -help' at a command shell
or consult the RTI Connext manual.
*/

#ifndef stream_typesPlugin_783819646_h
#define stream_typesPlugin_783819646_h

#include "stream_types.hpp"

struct RTICdrStream;

#ifndef pres_typePlugin_h
#include "pres/pres_typePlugin.h"
#endif

#if (defined(RTI_WIN32) || defined (RTI_WINCE) || defined(RTI_INTIME)) && defined(NDDS_USER_DLL_EXPORT)
/* If the code is building on Windows, start exporting symbols.
*/
#undef NDDSUSERDllExport
#define NDDSUSERDllExport __declspec(dllexport)
#endif

namespace fep3 {
    namespace ddstypes {

        #define PropertyPlugin_get_sample PRESTypePluginDefaultEndpointData_getSample

        #define PropertyPlugin_get_buffer PRESTypePluginDefaultEndpointData_getBuffer
        #define PropertyPlugin_return_buffer PRESTypePluginDefaultEndpointData_returnBuffer

        #define PropertyPlugin_create_sample PRESTypePluginDefaultEndpointData_createSample
        #define PropertyPlugin_destroy_sample PRESTypePluginDefaultEndpointData_deleteSample

        /* --------------------------------------------------------------------------------------
        Support functions:
        * -------------------------------------------------------------------------------------- */

        NDDSUSERDllExport extern Property*
        PropertyPluginSupport_create_data_w_params(
            const struct DDS_TypeAllocationParams_t * alloc_params);

        NDDSUSERDllExport extern Property*
        PropertyPluginSupport_create_data_ex(RTIBool allocate_pointers);

        NDDSUSERDllExport extern Property*
        PropertyPluginSupport_create_data(void);

        NDDSUSERDllExport extern RTIBool
        PropertyPluginSupport_copy_data(
            Property *out,
            const Property *in);

        NDDSUSERDllExport extern void
        PropertyPluginSupport_destroy_data_w_params(
            Property *sample,
            const struct DDS_TypeDeallocationParams_t * dealloc_params);

        NDDSUSERDllExport extern void
        PropertyPluginSupport_destroy_data_ex(
            Property *sample,RTIBool deallocate_pointers);

        NDDSUSERDllExport extern void
        PropertyPluginSupport_destroy_data(
            Property *sample);

        NDDSUSERDllExport extern void
        PropertyPluginSupport_print_data(
            const Property *sample,
            const char *desc,
            unsigned int indent);

        /* ----------------------------------------------------------------------------
        Callback functions:
        * ---------------------------------------------------------------------------- */

        NDDSUSERDllExport extern PRESTypePluginParticipantData
        PropertyPlugin_on_participant_attached(
            void *registration_data,
            const struct PRESTypePluginParticipantInfo *participant_info,
            RTIBool top_level_registration,
            void *container_plugin_context,
            RTICdrTypeCode *typeCode);

        NDDSUSERDllExport extern void
        PropertyPlugin_on_participant_detached(
            PRESTypePluginParticipantData participant_data);

        NDDSUSERDllExport extern PRESTypePluginEndpointData
        PropertyPlugin_on_endpoint_attached(
            PRESTypePluginParticipantData participant_data,
            const struct PRESTypePluginEndpointInfo *endpoint_info,
            RTIBool top_level_registration,
            void *container_plugin_context);

        NDDSUSERDllExport extern void
        PropertyPlugin_on_endpoint_detached(
            PRESTypePluginEndpointData endpoint_data);

        NDDSUSERDllExport extern void
        PropertyPlugin_return_sample(
            PRESTypePluginEndpointData endpoint_data,
            Property *sample,
            void *handle);

        NDDSUSERDllExport extern RTIBool
        PropertyPlugin_copy_sample(
            PRESTypePluginEndpointData endpoint_data,
            Property *out,
            const Property *in);

        /* ----------------------------------------------------------------------------
        (De)Serialize functions:
        * ------------------------------------------------------------------------- */

        NDDSUSERDllExport extern RTIBool
        PropertyPlugin_serialize_to_cdr_buffer(
            char * buffer,
            unsigned int * length,
            const Property *sample,
            ::dds::core::policy::DataRepresentationId representation
            = ::dds::core::policy::DataRepresentation::xcdr());

        NDDSUSERDllExport extern RTIBool
        PropertyPlugin_deserialize(
            PRESTypePluginEndpointData endpoint_data,
            Property **sample,
            RTIBool * drop_sample,
            struct RTICdrStream *stream,
            RTIBool deserialize_encapsulation,
            RTIBool deserialize_sample,
            void *endpoint_plugin_qos);

        NDDSUSERDllExport extern RTIBool
        PropertyPlugin_deserialize_from_cdr_buffer(
            Property *sample,
            const char * buffer,
            unsigned int length);

        NDDSUSERDllExport extern unsigned int
        PropertyPlugin_get_serialized_sample_max_size(
            PRESTypePluginEndpointData endpoint_data,
            RTIBool include_encapsulation,
            RTIEncapsulationId encapsulation_id,
            unsigned int current_alignment);

        /* --------------------------------------------------------------------------------------
        Key Management functions:
        * -------------------------------------------------------------------------------------- */
        NDDSUSERDllExport extern PRESTypePluginKeyKind
        PropertyPlugin_get_key_kind(void);

        NDDSUSERDllExport extern unsigned int
        PropertyPlugin_get_serialized_key_max_size(
            PRESTypePluginEndpointData endpoint_data,
            RTIBool include_encapsulation,
            RTIEncapsulationId encapsulation_id,
            unsigned int current_alignment);

        NDDSUSERDllExport extern unsigned int
        PropertyPlugin_get_serialized_key_max_size_for_keyhash(
            PRESTypePluginEndpointData endpoint_data,
            RTIEncapsulationId encapsulation_id,
            unsigned int current_alignment);

        NDDSUSERDllExport extern RTIBool
        PropertyPlugin_deserialize_key(
            PRESTypePluginEndpointData endpoint_data,
            Property ** sample,
            RTIBool * drop_sample,
            struct RTICdrStream *stream,
            RTIBool deserialize_encapsulation,
            RTIBool deserialize_key,
            void *endpoint_plugin_qos);

        /* Plugin Functions */
        NDDSUSERDllExport extern struct PRESTypePlugin*
        PropertyPlugin_new(void);

        NDDSUSERDllExport extern void
        PropertyPlugin_delete(struct PRESTypePlugin *);

        #define StreamTypePlugin_get_sample PRESTypePluginDefaultEndpointData_getSample

        #define StreamTypePlugin_get_buffer PRESTypePluginDefaultEndpointData_getBuffer
        #define StreamTypePlugin_return_buffer PRESTypePluginDefaultEndpointData_returnBuffer

        #define StreamTypePlugin_create_sample PRESTypePluginDefaultEndpointData_createSample
        #define StreamTypePlugin_destroy_sample PRESTypePluginDefaultEndpointData_deleteSample

        /* --------------------------------------------------------------------------------------
        Support functions:
        * -------------------------------------------------------------------------------------- */

        NDDSUSERDllExport extern StreamType*
        StreamTypePluginSupport_create_data_w_params(
            const struct DDS_TypeAllocationParams_t * alloc_params);

        NDDSUSERDllExport extern StreamType*
        StreamTypePluginSupport_create_data_ex(RTIBool allocate_pointers);

        NDDSUSERDllExport extern StreamType*
        StreamTypePluginSupport_create_data(void);

        NDDSUSERDllExport extern RTIBool
        StreamTypePluginSupport_copy_data(
            StreamType *out,
            const StreamType *in);

        NDDSUSERDllExport extern void
        StreamTypePluginSupport_destroy_data_w_params(
            StreamType *sample,
            const struct DDS_TypeDeallocationParams_t * dealloc_params);

        NDDSUSERDllExport extern void
        StreamTypePluginSupport_destroy_data_ex(
            StreamType *sample,RTIBool deallocate_pointers);

        NDDSUSERDllExport extern void
        StreamTypePluginSupport_destroy_data(
            StreamType *sample);

        NDDSUSERDllExport extern void
        StreamTypePluginSupport_print_data(
            const StreamType *sample,
            const char *desc,
            unsigned int indent);

        /* ----------------------------------------------------------------------------
        Callback functions:
        * ---------------------------------------------------------------------------- */

        NDDSUSERDllExport extern PRESTypePluginParticipantData
        StreamTypePlugin_on_participant_attached(
            void *registration_data,
            const struct PRESTypePluginParticipantInfo *participant_info,
            RTIBool top_level_registration,
            void *container_plugin_context,
            RTICdrTypeCode *typeCode);

        NDDSUSERDllExport extern void
        StreamTypePlugin_on_participant_detached(
            PRESTypePluginParticipantData participant_data);

        NDDSUSERDllExport extern PRESTypePluginEndpointData
        StreamTypePlugin_on_endpoint_attached(
            PRESTypePluginParticipantData participant_data,
            const struct PRESTypePluginEndpointInfo *endpoint_info,
            RTIBool top_level_registration,
            void *container_plugin_context);

        NDDSUSERDllExport extern void
        StreamTypePlugin_on_endpoint_detached(
            PRESTypePluginEndpointData endpoint_data);

        NDDSUSERDllExport extern void
        StreamTypePlugin_return_sample(
            PRESTypePluginEndpointData endpoint_data,
            StreamType *sample,
            void *handle);

        NDDSUSERDllExport extern RTIBool
        StreamTypePlugin_copy_sample(
            PRESTypePluginEndpointData endpoint_data,
            StreamType *out,
            const StreamType *in);

        /* ----------------------------------------------------------------------------
        (De)Serialize functions:
        * ------------------------------------------------------------------------- */

        NDDSUSERDllExport extern RTIBool
        StreamTypePlugin_serialize_to_cdr_buffer(
            char * buffer,
            unsigned int * length,
            const StreamType *sample,
            ::dds::core::policy::DataRepresentationId representation
            = ::dds::core::policy::DataRepresentation::xcdr());

        NDDSUSERDllExport extern RTIBool
        StreamTypePlugin_deserialize(
            PRESTypePluginEndpointData endpoint_data,
            StreamType **sample,
            RTIBool * drop_sample,
            struct RTICdrStream *stream,
            RTIBool deserialize_encapsulation,
            RTIBool deserialize_sample,
            void *endpoint_plugin_qos);

        NDDSUSERDllExport extern RTIBool
        StreamTypePlugin_deserialize_from_cdr_buffer(
            StreamType *sample,
            const char * buffer,
            unsigned int length);

        NDDSUSERDllExport extern unsigned int
        StreamTypePlugin_get_serialized_sample_max_size(
            PRESTypePluginEndpointData endpoint_data,
            RTIBool include_encapsulation,
            RTIEncapsulationId encapsulation_id,
            unsigned int current_alignment);

        /* --------------------------------------------------------------------------------------
        Key Management functions:
        * -------------------------------------------------------------------------------------- */
        NDDSUSERDllExport extern PRESTypePluginKeyKind
        StreamTypePlugin_get_key_kind(void);

        NDDSUSERDllExport extern unsigned int
        StreamTypePlugin_get_serialized_key_max_size(
            PRESTypePluginEndpointData endpoint_data,
            RTIBool include_encapsulation,
            RTIEncapsulationId encapsulation_id,
            unsigned int current_alignment);

        NDDSUSERDllExport extern unsigned int
        StreamTypePlugin_get_serialized_key_max_size_for_keyhash(
            PRESTypePluginEndpointData endpoint_data,
            RTIEncapsulationId encapsulation_id,
            unsigned int current_alignment);

        NDDSUSERDllExport extern RTIBool
        StreamTypePlugin_deserialize_key(
            PRESTypePluginEndpointData endpoint_data,
            StreamType ** sample,
            RTIBool * drop_sample,
            struct RTICdrStream *stream,
            RTIBool deserialize_encapsulation,
            RTIBool deserialize_key,
            void *endpoint_plugin_qos);

        /* Plugin Functions */
        NDDSUSERDllExport extern struct PRESTypePlugin*
        StreamTypePlugin_new(void);

        NDDSUSERDllExport extern void
        StreamTypePlugin_delete(struct PRESTypePlugin *);

    } /* namespace ddstypes  */
} /* namespace fep3  */

#if (defined(RTI_WIN32) || defined (RTI_WINCE) || defined(RTI_INTIME)) && defined(NDDS_USER_DLL_EXPORT)
/* If the code is building on Windows, stop exporting symbols.
*/
#undef NDDSUSERDllExport
#define NDDSUSERDllExport
#endif

#endif /* stream_typesPlugin_783819646_h */

