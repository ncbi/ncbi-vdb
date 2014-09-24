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

#include <sra/extern.h>
#include <klib/rc.h>
#include <klib/defs.h>
#include <klib/refcount.h>
#include <sra/srapath.h>
#include <sra/sradb-priv.h>
#include <sysalloc.h>
#include <os-native.h>

#include <sra/impl.h>

#include <string.h>
#include <assert.h>
#include <stdio.h>


/*--------------------------------------------------------------------------
 * SRAPath
 *  manages accession -> path conversion
 */

/* Make
 *  create path manager
 *
 *  the path manager should already be configured with
 *  standard search paths, but can be augmented by using
 *  the Add*Path messages.
 *
 *  "dir" [ IN, NULL OKAY ] - optional root directory to use
 *  attaches a new reference
 */
LIB_EXPORT rc_t CC SRAPathMake ( SRAPath **pm, struct KDirectory const *dir )
{
    rc_t rc;

    if ( pm == NULL )
        rc = RC ( rcSRA, rcMgr, rcConstructing, rcParam, rcNull );
    else
    {
        rc = SRAPathMakeImpl( pm, dir );
        if ( rc == 0 )
            return 0;

        * pm = NULL;
    }

    return rc;
}


/* AddRef
 * Release
 */
LIB_EXPORT rc_t CC SRAPathAddRef ( const SRAPath *self )
{
    if ( self != NULL )
    {
        switch ( self -> vt -> v1 . maj )
        {
        case 1:
        case 2:
            return self -> vt -> v1 . addref ( self );
        }

        return RC ( rcSRA, rcMgr, rcReleasing, rcInterface, rcBadVersion );
    }
    return 0;
}

LIB_EXPORT rc_t CC SRAPathRelease ( const SRAPath *self )
{
    if ( self != NULL )
    {
        switch ( self -> vt -> v1 . maj )
        {
        case 1:
        case 2:
            return self -> vt -> v1 . release ( self );
        }

        return RC ( rcSRA, rcMgr, rcReleasing, rcInterface, rcBadVersion );
    }
    return 0;
}


/* Version
 *  returns the library version
 */
LIB_EXPORT rc_t CC SRAPathVersion ( const SRAPath *self, uint32_t *version )
{
    if ( version == NULL )
        return RC ( rcSRA, rcMgr, rcAccessing, rcParam, rcNull );

    * version = 0;

    if ( self == NULL )
        return RC ( rcSRA, rcMgr, rcAccessing, rcSelf, rcNull );

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
    case 2:
        return self -> vt -> v1 . version ( self, version );
    }

    return RC ( rcSRA, rcMgr, rcAccessing, rcInterface, rcBadVersion );
}


/* Clear
 *  forget all existing server and volume paths
 */
LIB_EXPORT rc_t CC SRAPathClear ( SRAPath *self )
{
    if ( self == NULL )
        return RC ( rcSRA, rcMgr, rcResetting, rcSelf, rcNull );

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
    case 2:
        return self -> vt -> v1 . clear ( self );
    }

    return RC ( rcSRA, rcMgr, rcResetting, rcInterface, rcBadVersion );
}


/* AddRepPath
 *  add a replication path
 *
 *  "rep" [ IN ] - NUL-terminated server search path
 *  may be a compound path with ':' separator characters, e.g.
 *  "/panfs/traces01:/panfs/traces31"
 *
 *  NB - servers are searched in the order provided,
 *  first to last, until one of them satisfies a request,
 *  at which time the successful server is placed at the
 *  head of the search path.
 */
LIB_EXPORT rc_t CC SRAPathAddRepPath ( SRAPath *self, const char *path )
{
    if ( self == NULL )
        return RC ( rcSRA, rcMgr, rcUpdating, rcSelf, rcNull );
    if ( path == NULL )
        return RC ( rcSRA, rcMgr, rcUpdating, rcString, rcNull );
    if ( path [ 0 ] == 0 )
        return RC ( rcSRA, rcMgr, rcUpdating, rcString, rcEmpty );

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
    case 2:
        return self -> vt -> v1 . addRepPath ( self, path );
    }

    return RC ( rcSRA, rcMgr, rcUpdating, rcInterface, rcBadVersion );
}


/* AddVolPath
 *  add a volume path
 *
 *  "vol" [ IN ] - NUL-terminated volume search path
 *  may be a compound path with ':' separator characters, e.g.
 *  "sra2:sra1:sra0"
 *
 *  NB - volumes are searched in the order provided,
 *  first to last. they are never re-ordered.
 */
LIB_EXPORT rc_t CC SRAPathAddVolPath ( SRAPath *self, const char *path )
{
    if ( self == NULL )
        return RC ( rcSRA, rcMgr, rcUpdating, rcSelf, rcNull );
    if ( path == NULL )
        return RC ( rcSRA, rcMgr, rcUpdating, rcString, rcNull );
    if ( path [ 0 ] == 0 )
        return RC ( rcSRA, rcMgr, rcUpdating, rcString, rcEmpty );

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
    case 2:
        return self -> vt -> v1 . addVolPath ( self, path );
    }

    return RC ( rcSRA, rcMgr, rcUpdating, rcInterface, rcBadVersion );
}


/* Full
 *  creates full path from server, volume & accession
 *
 *  "rep" [ IN ] - NUL terminated full path of replication
 *  server, e.g. "/panfs/traces01"
 *
 *  "vol" [ IN ] - NUL terminated relative path of volume,
 *  e.g. "sra2"
 *
 *  "accession" [ IN ] - NUL terminated run accession,
 *  e.g. "SRR000001"
 *
 *  "path" [ OUT ] and "path_max" [ IN ] - return buffer for
 *  NUL-terminated full path to accession.
 */
LIB_EXPORT rc_t CC SRAPathFull ( const SRAPath *self, const char *rep,
    const char *vol, const char *accession, char *path, size_t path_max )
{
    if ( path_max == 0 )
        return RC ( rcSRA, rcMgr, rcAccessing, rcBuffer, rcInsufficient );
    if ( path == NULL )
        return RC ( rcSRA, rcMgr, rcAccessing, rcBuffer, rcNull );

    /* prepare return for failure */
    path [ 0 ] = 0;

    /* test remainder */
    if ( self == NULL )
        return RC ( rcSRA, rcMgr, rcAccessing, rcSelf, rcNull );
    if ( accession == NULL )
        return RC ( rcSRA, rcMgr, rcAccessing, rcPath, rcNull );
    if ( accession [ 0 ] == 0 )
        return RC ( rcSRA, rcMgr, rcAccessing, rcPath, rcEmpty );

    /* turn NULL prefix parts into empty string */
    if ( rep == NULL )
        rep = "";
    if ( vol == NULL )
        vol = "";

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
    case 2:
        return self -> vt -> v1 . fullPath ( self, rep, vol, accession, path, path_max );
    }

    return RC ( rcSRA, rcMgr, rcAccessing, rcInterface, rcBadVersion );
}


/* Test
 *  returns true if path appears to be accession
 *  the test is a heuristic, and may return false positives
 *
 *  "path" [ IN ] - NUL terminated path to run
 */
LIB_EXPORT bool CC SRAPathTest ( const SRAPath *self, const char *path )
{
    if ( self == NULL )
        return false;
    if ( path == NULL || path [ 0 ] == 0 )
        return false;

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
    case 2:
        return self -> vt -> v1 . testPath ( self, path );
    }

    return false;
}


/* Find
 *  finds location of run within rep-server/volume matrix
 *
 *  "accession" [ IN ] - NUL terminated run accession,
 *   e.g. "SRR000001"
 *
 *  "path" [ OUT ] and "path_max" [ IN ] - return buffer for
 *  NUL-terminated full path to accession.
 *
 *  returns 0 if path exists, rc state rcNotFound if
 *  path cannot be found, and rcInsufficient if buffer is
 *  too small.
 */
LIB_EXPORT rc_t CC SRAPathFindWithRepLen ( const SRAPath *self,
    const char *accession, char *path, size_t path_max, size_t *rep_len )
{
    if ( path == NULL )
        return RC ( rcSRA, rcMgr, rcSelecting, rcBuffer, rcNull );
    if ( path_max == 0 )
        return RC ( rcSRA, rcMgr, rcSelecting, rcBuffer, rcInsufficient );

    /* looking for illegal character in the accession 'name'
     * this will be the RC from something like "ncbi-acc:./SRR000001"
     * or if this is called as a default resolution in the old manner
     */
    if ( strchr ( accession, '/' ) != NULL )
        return RC ( rcSRA, rcUri, rcResolving, rcPath, rcIncorrect );

    path [ 0 ] = 0;

    if ( self == NULL )
        return RC ( rcSRA, rcMgr, rcSelecting, rcSelf, rcNull );
    if ( accession == NULL )
        return RC ( rcSRA, rcMgr, rcSelecting, rcPath, rcNull );
    if ( accession [ 0 ] == 0 )
        return RC ( rcSRA, rcMgr, rcSelecting, rcPath, rcEmpty );

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
    case 2:
        return self -> vt -> v1 . findPath ( self, accession, path, path_max, rep_len );
    }

    return RC ( rcSRA, rcMgr, rcSelecting, rcInterface, rcBadVersion );
}

LIB_EXPORT rc_t CC SRAPathFind ( const SRAPath *self,
    const char *accession, char *path, size_t path_max )
{
    return SRAPathFindWithRepLen ( self, accession, path, path_max, NULL );
}


/* List (Deprecated)
 *  list all runs known to SRAPath
 *  this can be VERY SLOW due to the large number of runs
 *
 *  "runs" [ OUT ] - return parameter for ordered list of accessions
 *
 *  "deep" [ IN ] - when false, list only first responding server
 */
LIB_EXPORT rc_t CC SRAPathList ( const SRAPath *self, struct KNamelist **runs, bool deep )
{
    if ( runs == NULL )
        return RC ( rcSRA, rcMgr, rcListing, rcParam, rcNull );

    * runs = NULL;

    if ( self == NULL )
        return RC ( rcSRA, rcMgr, rcListing, rcSelf, rcNull );

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        return RC ( rcSRA, rcMgr, rcListing, rcInterface, rcUnsupported ); /* removed from v1 */
    }

    return RC ( rcSRA, rcMgr, rcListing, rcInterface, rcBadVersion );
}
