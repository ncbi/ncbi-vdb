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
* ==============================================================================
*
*/

#include <klib/misc.h> /* SetHttpTries */

static uint32_t s_tries = 0;
static bool s_tries_set = false;

LIB_EXPORT void CC SetHttpTries(uint32_t tries) {
    s_tries = tries;
    s_tries_set = true;
}

uint32_t GetHttpTries(bool *isTriesSet) {
    bool dummy = false;
    if (isTriesSet == NULL) {
        isTriesSet = &dummy;
    }
    *isTriesSet = s_tries_set;
    return s_tries;
}
