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
* ============================================================================*/

#include <vdb/quality.h> /* VQuality */
#include <vfs/services.h> /* VQuality */

struct KConfig;
struct KNSManager;
struct KSrvResponse;
struct KSrvRun;
struct String;
struct VPath;
struct VFSManager;

typedef struct ServicesCache ServicesCache;

/* Make */
rc_t ServicesCacheMake(ServicesCache ** self, const struct VFSManager * vfs,
    const struct KNSManager * kns, const struct KConfig * kfg,
    int64_t projectId, const char * quality);

/* Whack */
rc_t ServicesCacheWhack(ServicesCache * self);

/* Add a remote location to ServicesCache */
rc_t ServicesCacheAddRemote(ServicesCache * self, const struct VPath * path);

/* Add an accession to ServicesCache (used when remote repository is off) */
rc_t ServicesCacheAddId(ServicesCache * self, const char * acc);

/* All path-s/accessions are added. Now prepare results. */
rc_t ServicesCacheComplete(ServicesCache * self,
    const char * outDir, const char * outFile, bool skipLocal);

/* Return local and cache locations for remote (to be used by file iterator) */
rc_t ServicesCacheResolve(ServicesCache * self, const struct VPath * remote,
    const struct VPath ** local, const struct VPath ** cache);

/* TODO rc_t ServicesCacheGetResponse(const ServicesCache * self,
    const char * acc, const struct KSrvResponse ** response); */

/* Get the first run or the first run from tree from ServicesCache */
rc_t ServicesCacheGetRun(const ServicesCache * self, bool tree,
    const struct KSrvRun ** run, KSrvRunIterator * it);
