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

#ifndef _h_kfc_xcgen_
#define _h_kfc_xcgen_

#ifndef _h_kfc_xcext_
#include <kfc/xcext.h>
#endif

#ifndef _h_kfc_rc_
#include <kfc/rc.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#undef XOBJ
#undef XOBJ_EXT
#undef XSTATE
#undef XSTATE_EXT
#undef XC
#undef XC_EXT

#define XOBJ( name, desc, rc )                          \
    const XCObj name = { # name, desc, NULL, rc }
#define XOBJ_EXT( name, supr, desc, rc )                \
    const XCObj name = { # name, desc, & supr, rc }

#define XSTATE( name, desc, rc )                        \
    const XCState name = { # name, desc, NULL, rc }
#define XSTATE_EXT( name, supr, desc, rc )              \
    const XCState name = { # name, desc, & supr, rc }

#define XC( name, obj, state )                          \
    const XCErr name = { # name, NULL, & obj, & state }
#define XC_EXT( name, supr )                            \
    const XCErr name = { # name, & supr }

#ifdef __cplusplus
}
#endif

#endif /* _h_kfc_xcgen_ */
