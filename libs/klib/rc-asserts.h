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

#define RC_MOD_BITS (5)
#define RC_TARG_BITS (6)
#define RC_CTX_BITS (7)
#define RC_MOD_TARG_CTX_BITS (RC_MOD_BITS + RC_TARG_BITS + RC_CTX_BITS)
#define RC_OBJ_BITS (8)
#define RC_STATE_BITS (32 - (RC_MOD_TARG_CTX_BITS + RC_OBJ_BITS))

#define ASSERTION_RC_MOD_COUNT (((int)rcLastModule_v1_2) <= (1 << RC_MOD_BITS))
#define ASSERTION_RC_MOD_EXTENSION_1 (((int)rcKFG) == (((int)rcSRA) + 1))
#define ASSERTION_RC_MOD_EXTENSION_2 (((int)rcExitCode) == (((int)rcVFS) + 1))
#define ASSERTION_RC_MOD_EXTENSION (ASSERTION_RC_MOD_EXTENSION_1 && ASSERTION_RC_MOD_EXTENSION_2)

#define ASSERTION_RC_TARG_COUNT (((int)rcLastTarget_v1_2) <= (1 << RC_TARG_BITS))
#define ASSERTION_RC_TARG_EXTENSION_1 (((int)rcProduction) == (((int)rcExpression) + 1))
#define ASSERTION_RC_TARG_EXTENSION_2 (((int)rcProvider) == (((int)rcUri) + 1))
#define ASSERTION_RC_TARG_EXTENSION (ASSERTION_RC_TARG_EXTENSION_1 && ASSERTION_RC_TARG_EXTENSION_2)

#define ASSERTION_RC_CTX_COUNT (((int)rcLastContext_v1_1) <= (1 << RC_CTX_BITS))
#define ASSERTION_RC_CTX_EXTENSION_1 (((int)rcFlushing) == (((int)rcInflating) + 1))
#define ASSERTION_RC_CTX_EXTENSION (ASSERTION_RC_CTX_EXTENSION_1)

#define ASSERTION_RC_OBJ_COUNT (((int)rcLastObject_v1_2) <= (1 << RC_OBJ_BITS))
#define ASSERTION_RC_OBJ_FIRST (((int)rcLink) == (((int)rcUri) + 1))
#define ASSERTION_RC_OBJ_EXTENSION_1 (((int)rcItem) == (((int)rcLibrary) + 1))
#define ASSERTION_RC_OBJ_EXTENSION_2 (((int)rcCloudProvider) == (((int)rcRefcount) + 1))
#define ASSERTION_RC_OBJ_EXTENSION (ASSERTION_RC_OBJ_EXTENSION_1 && ASSERTION_RC_OBJ_EXTENSION_2)

#define ASSERTION_RC_STATE_COUNT (((int)rcLastState_v1_1) <= (1 << RC_STATE_BITS))
#define ASSERTION_RC_STATE_EXTENSION_1 (((int)rcOpen) == (((int)rcOutofrange) + 1))
#define ASSERTION_RC_STATE_EXTENSION (ASSERTION_RC_STATE_EXTENSION_1)
