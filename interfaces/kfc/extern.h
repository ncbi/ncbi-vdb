/*===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software/database is a "United States Government Work" under the
*  terms of the United States Copyright Act.  It was written as part of
*  the author's official duties as a United States Government employee and
*  thus cannot be copyrighted.  This software/database is freely available
*  to the public for use. The National Library of Medicine and the U.S.
*  Government have not placed any restriction on its use or reproduction.
*
*  Although all reasonable efforts have been taken to ensure the accuracy
*  and reliability of the software and data, the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data. The NLM and the U.S.
*  Government disclaim all warranties, express or implied, including
*  warranties of performance, merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
*/

#ifndef _h_kfc_extern_
#define _h_kfc_extern_

#ifndef _h_kfc_callconv_
#include <kfc/callconv.h>
#endif

#if ! defined EXPORT_LATCH && _LIBRARY

#define KFC_EXTERN LIB_EXPORT
#define EXPORT_LATCH 1

#else

#define KFC_EXTERN LIB_IMPORT

#endif

#if defined _MSC_VER

/* __declspec ( dllimport ) will cause creation of
   function pointers rather than thunks, which makes
   the code that imports unable to link statically
   against a library. we leave this symbol defined as
   "extern" to use thunks instead. as a result, all
   function addresses resolve to the thunk and not
   the actual function. */
#define LIB_IMPORT extern
#define LIB_IMPORT_DATA extern __declspec ( dllimport )
#define LIB_EXPORT __declspec ( dllexport )
#define LIB_EXPORT_DATA __declspec ( dllexport )

#else

#define LIB_IMPORT extern
#define LIB_IMPORT_DATA extern
#define LIB_EXPORT
#define LIB_EXPORT_DATA

#endif

#endif /* _h_kfc_extern_ */
