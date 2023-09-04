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

#pragma once

#include "dbmgr-priv.h"

#ifdef __cplusplus
extern "C" {
#endif

rc_t KDBManagerVOpenDBReadInt_noargs ( const KDBManager *self, const struct KDatabase **dbp,
                                const KDirectory *wd, bool try_srapath,
                                const char *path, ... );
rc_t KDBManagerVOpenTableReadInt_noargs ( const KDBManager *self,
    const KTable **tblp, const KDirectory *wd, bool try_srapath,
    const char *path, bool tryEnvAndAd, const struct VPath *vpath,
    ... );
rc_t KDBManagerVOpenColumnReadInt_noargs ( const KDBManager *self,
    const struct KColumn **colp, const KDirectory *wd, bool try_srapath,
    const char *path, ... );

#ifdef __cplusplus
}
#endif

