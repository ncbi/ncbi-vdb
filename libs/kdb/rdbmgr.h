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

#include "dbmgr.h"

#include <kdb/meta.h>

struct KTable;
struct KRIndex;
struct KRMetadata;

#ifdef __cplusplus
extern "C" {
#endif

rc_t KDBRManagerVOpenDBReadInt_noargs ( const KDBManager *self, const struct KDatabase **dbp,
                                const KDirectory *wd, bool try_srapath,
                                const char *path, ... );
rc_t KDBRManagerVOpenTableReadInt_noargs ( const KDBManager *self,
    const struct KTable **tblp, const KDirectory *wd, bool try_srapath,
    const char *path, bool tryEnvAndAd, const struct VPath *vpath,
    ... );
rc_t KDBRManagerVOpenColumnReadInt_noargs ( const KDBManager *self,
    const struct KColumn **colp, const KDirectory *wd, bool try_srapath,
    const char *path, ... );

rc_t KDBRManagerOpenMetadataReadInt ( const KDBManager *self, struct KRMetadata **metap,
    const KDirectory *wd, uint32_t rev, bool prerelease );

rc_t KDBRManagerOpenIndexReadInt ( const KDBManager *self, struct KRIndex **idxp, const KDirectory *wd, const char *path );

/// @brief Get Path Contents, from va_list
rc_t KDBRManagerVPathContents(const KDBManager *self, KDBContents const **result, int levelOfDetail, const char *path, va_list args);

/// @brief Get Path Contents, from var_args
rc_t KDBRManagerPathContents(const KDBManager *self, KDBContents const **result, int levelOfDetail, const char *path, ...);

#ifdef __cplusplus
}
#endif

