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


#ifndef _FEP3_PARTICIPANT_EXPORT_H_INCLUDED_
#define _FEP3_PARTICIPANT_EXPORT_H_INCLUDED_

#ifdef _FEP3_PARTICIPANT_INCLUDED_STATIC
    /// Macro switching between export / import of the fep sdk shared object
    #define FEP3_PARTICIPANT_EXPORT /**/

#else //_FEP3_PARTICIPANT_INCLUDED_STATIC

    #ifdef WIN32
        #ifdef _FEP3_PARTICIPANT_DO_EXPORT
            /// Macro switching between export / import of the fep sdk shared object
            #define FEP3_PARTICIPANT_EXPORT __declspec( dllexport )
        #else   // _FEP3_PARTICIPANT_DO_EXPORT
            /// Macro switching between export / import of the fep sdk shared object
            #define FEP3_PARTICIPANT_EXPORT __declspec( dllimport )
        #endif
    #else   // WIN32
        #ifdef _FEP3_PARTICIPANT_DO_EXPORT
            /// Macro switching between export / import of the fep sdk shared object
            #define FEP3_PARTICIPANT_EXPORT __attribute__ ((visibility("default")))
        #else   // _FEP3_PARTICIPANT_DO_EXPORT
            /// Macro switching between export / import of the fep sdk shared object
            #define FEP3_PARTICIPANT_EXPORT
        #endif
    #endif
#endif //_FEP3_PARTICIPANT_INCLUDED_STATIC

#endif // _FEP3_PARTICIPANT_EXPORT_H_INCLUDED_
