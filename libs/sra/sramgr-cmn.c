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

#include <klib/namelist.h>
#include <klib/refcount.h>
#include <klib/log.h>
#include <klib/rc.h>
#include <kfs/directory.h>
#include <kfg/config.h>
#include <vfs/manager.h>
#include <vfs/resolver.h>
#include <vfs/path.h>
#include <kdb/manager.h>
#include <kdb/kdb-priv.h>
#include <vdb/manager.h>
#include <vdb/schema.h>
#include <vdb/vdb-priv.h>
#include <sra/sradb.h>
#include <sra/sraschema.h>
#include <sra/srapath.h>
#include <sra/sradb-priv.h>
#include <sysalloc.h>
#include <atomic.h>

#include "sra-priv.h"

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>


/* Whack
 *  will not refuse request, and ignores errors
 */
static
rc_t SRAMgrWhack ( const SRAMgr *that )
{
    SRAMgr *self = ( SRAMgr* ) that;

    VSchemaRelease ( self -> schema );
    VDBManagerRelease ( self -> vmgr );
    SRACacheWhack ( self -> cache );
    
    /* must check here for NULL because
       SRAPathRelease is weak-linked */
    if ( self -> _pmgr != NULL )
    {
#if OLD_SRAPATH_MGR
        SRAPathRelease ( self -> _pmgr );
#else
        VResolverRelease ( ( const VResolver* ) self -> _pmgr );
#endif
    }

    free ( self );
    return 0;
}


/* Release
 *  releases reference to object
 *  obtained from MakeRead, MakeUpdate,
 *  or AddRef(see above)
 */
LIB_EXPORT rc_t CC SRAMgrRelease ( const SRAMgr *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDrop ( & self -> refcount, "SRAMgr" ) )
        {
        case krefWhack:
            return SRAMgrWhack ( ( SRAMgr* ) self );
        case krefNegative:
            return RC ( rcSRA, rcMgr, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
}


/* AddRef
 *  attach a new reference to an existing object
 *  ( see above)
 */
LIB_EXPORT rc_t CC SRAMgrAddRef ( const SRAMgr *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAdd ( & self -> refcount, "SRAMgr" ) )
        {
        case krefLimit:
            return RC ( rcSRA, rcMgr, rcAttaching, rcRange, rcExcessive );
        }
    }
    return 0;
}

/* Attach
 * Sever
 */
SRAMgr *SRAMgrAttach ( const SRAMgr *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAddDep ( & self -> refcount, "SRAMgr" ) )
        {
        case krefLimit:
            return NULL;
        }
    }
    return ( SRAMgr* ) self;
}

rc_t SRAMgrSever ( const SRAMgr *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDropDep ( & self -> refcount, "SRAMgr" ) )
        {
        case krefWhack:
            return SRAMgrWhack ( ( SRAMgr* ) self );
        case krefNegative:
            return RC ( rcSRA, rcMgr, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
}


/* Make
 */
static
rc_t SRAMgrInitPath ( SRAMgr *mgr, KConfig *kfg, const KDirectory *wd )
{
    VFSManager *vfs;
    rc_t rc = VFSManagerMake ( & vfs );
    if ( rc == 0 )
    {
        rc = VFSManagerMakeResolver ( vfs, ( VResolver** ) & mgr -> _pmgr, kfg );
        VFSManagerRelease ( vfs );
    }

    if ( rc != 0 )
        mgr -> _pmgr = NULL;

    return 0;
}

rc_t SRAMgrMake ( SRAMgr **mgrp,
    const VDBManager *vmgr, const KDirectory *wd )
{
    rc_t rc;

    /* require sraschema object */
    SRAMgr *mgr = malloc ( sizeof * mgr );
    if ( mgr == NULL )
        rc = RC ( rcSRA, rcMgr, rcConstructing, rcMemory, rcExhausted );
    else
    {
        VSchema *schema;
        rc = VDBManagerMakeSRASchema ( vmgr, & schema );
        if ( rc == 0 )
        {
            KConfig* kfg;
            rc = KConfigMake ( & kfg, wd );
            if ( rc == 0 )
            {
                rc = SRAMgrInitPath ( mgr, kfg, wd );
                if ( rc == 0 )
                {
                    rc = SRACacheInit ( & mgr -> cache, kfg );
                    if ( rc == 0 )
                    {
                        KRefcountInit ( & mgr -> refcount, 1, "SRAMgr", "SRAMgrMake", "sramgr" );
                        mgr -> vmgr = vmgr;
                        mgr -> schema = schema;
                        mgr -> mode = kcmCreate; /* TBD - should this include parents? */
                        mgr -> read_only = true;
                        * mgrp = mgr;
                        KConfigRelease ( kfg );
                        return 0;
                    }
                }
                KConfigRelease ( kfg );
            }

            VSchemaRelease ( schema );
        }

        free ( mgr );
    }

    * mgrp = NULL;
    return rc;
}


/* Writable
 *  returns 0 if table is writable
 *  rcLocked if locked, rcReadonly if read-only
 *  other code upon error
 *
 *  "path" [ IN ] - NUL terminated table path
 */
LIB_EXPORT rc_t CC SRAMgrVWritable ( const SRAMgr *self,
        const char *path, va_list args )
{
    if ( self == NULL )
        return RC ( rcSRA, rcMgr, rcAccessing, rcSelf, rcNull );
    return VDBManagerVWritable ( self -> vmgr, path, args );
}

LIB_EXPORT rc_t CC SRAMgrWritable ( const SRAMgr *self,
        const char *path, ... )
{
    rc_t rc;

    va_list args;
    va_start ( args, path );

    rc = SRAMgrVWritable ( self, path, args );

    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC SRAMgrResolve( const SRAMgr *self, const char* acc, char* buf, size_t buf_size )
{
    if ( self == NULL )
        return RC ( rcSRA, rcMgr, rcResolving, rcSelf, rcNull );
    if ( acc == NULL || buf == NULL)
        return RC ( rcSRA, rcMgr, rcResolving, rcParam, rcNull );
    else
    {
        VFSManager *vfs;
        rc_t rc = VFSManagerMake ( & vfs );
        if (rc == 0)
        {
			rc_t rc2;
            VPath* path;
            rc = VFSManagerMakePath(vfs, &path, "ncbi-acc:%s", acc);
            if (rc == 0)
            {
                const VPath* resolved;
                rc = VResolverQuery ( (const struct VResolver*)self->_pmgr, 0, path, &resolved, NULL, NULL );
                if (rc == 0)
                {
                    rc = VPathReadPath ( resolved, buf, buf_size, NULL );
                    rc2 = VPathRelease(resolved);
                    if (rc == 0)
                        rc = rc2;
                }
                
                rc2 = VPathRelease(path);
                if (rc == 0)
                    rc = rc2;
            }
            rc2 = VFSManagerRelease(vfs);
            if (rc == 0)
                rc = rc2;
        }
        
        return rc;
    }
}

LIB_EXPORT rc_t CC SRAMgrConfigReload( const SRAMgr *cself, const KDirectory *wd )
{
#if OLD_SRAPATH_MGR
    SRAMgr *self = cself;

    /* create a new SRAPath object */
    SRAPath *pnew;
    rc_t rc = SRAPathMake ( & pnew, wd );
    if ( rc == 0 )
    {
        /* swap with the old guy */
        SRAPath *pold = self -> _pmgr;
        if ( atomic_test_and_set_ptr ( ( void *volatile* ) & self -> _pmgr, pnew, pold ) == ( void* ) pold )
            SRAPathRelease ( pold );
        else
            SRAPathRelease ( pnew );
    }

    return rc;
#else
    return 0;
#endif

#if 0


    /* do not reload VDBManager config for now
       it cannot reload properly and grows in memory */
    rc_t rc = 0;
    /* (not thread) safely re-instanciate sra path config */
    SRAMgr *self = (SRAMgr*)cself;
    SRAPath* p = cself->_pmgr;
    self->_pmgr = NULL;
    if( (rc = SRAMgrInitPath(self, wd)) == 0 ) {
        SRAPathRelease(p);
    } else {
        /* roll back */
        self->_pmgr = p;
    }
    return rc;
#endif

}

/* GetVDBManager
 *  returns a new reference to VDBManager used by SRAMgr
 */
LIB_EXPORT rc_t CC SRAMgrGetVDBManagerRead ( const SRAMgr *self, const VDBManager **vmgr )
{
    rc_t rc;

    if ( vmgr == NULL )
        rc = RC ( rcSRA, rcMgr, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcSRA, rcMgr, rcAccessing, rcSelf, rcNull );
        else
        {
            * vmgr = self -> vmgr;
            return VDBManagerAddRef ( * vmgr );
        }

        * vmgr = NULL;
    }

    return rc;
}

/* GetKDBManager
 *  returns a new reference to KDBManager used indirectly by SRAMgr
 */
LIB_EXPORT rc_t CC SRAMgrGetKDBManagerRead ( const SRAMgr *self,
        struct KDBManager const **kmgr )
{
    rc_t rc;

    if ( kmgr == NULL )
        rc = RC ( rcSRA, rcMgr, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcSRA, rcMgr, rcAccessing, rcSelf, rcNull );
        else
        {
            return VDBManagerGetKDBManagerRead ( self -> vmgr, kmgr );
        }

        * kmgr = NULL;
    }

    return rc;
}


/* ModDate
 *  return a modification timestamp for table
 */
LIB_EXPORT rc_t CC SRAMgrVGetTableModDate ( const SRAMgr *self,
    KTime_t *mtime, const char *spec, va_list args )
{
    rc_t rc;

    if ( mtime == NULL )
        rc = RC ( rcSRA, rcMgr, rcAccessing, rcParam, rcNull );
    else
    {
        * mtime = 0;
        if ( self == NULL )
            rc = RC ( rcSRA, rcMgr, rcAccessing, rcSelf, rcNull );
        else
        {
            char path[4096];
            rc = ResolveTablePath(self, path, sizeof path, spec, args);
            if( rc == 0 ) {
                struct KDBManager const *kmgr;
                rc = VDBManagerGetKDBManagerRead ( self -> vmgr, & kmgr );
                if ( rc == 0 )
                {
                    rc = KDBManagerGetTableModDate(kmgr, mtime, "%s", path);
                    KDBManagerRelease ( kmgr );
                }
            }
        }
    }

    return rc;
}

LIB_EXPORT rc_t CC SRAMgrGetTableModDate ( const SRAMgr *self,
    KTime_t *mtime, const char *spec, ... )
{
    rc_t rc;

    va_list args;
    va_start ( args, spec );
    rc = SRAMgrVGetTableModDate ( self, mtime, spec, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC SRAMgrSingleFileArchiveExt(const SRAMgr *self, const char* spec, const bool lightweight, const char** ext)
{
    rc_t rc;

    if( self == NULL || spec == NULL || ext == NULL ) {
        rc = RC(rcSRA, rcFile, rcConstructing, rcParam, rcNull);
    } else {
        char buf[4096];
        va_list args;

        if( (rc = ResolveTablePath(self, buf, sizeof(buf), spec, args)) == 0 ) {
            const KDBManager* kmgr;
            if( (rc = SRAMgrGetKDBManagerRead(self, &kmgr)) == 0 ) {
                int type = KDBManagerPathType(kmgr, "%s", buf) & ~kptAlias;
                if( type == kptDatabase ) {
                    *ext = CSRA_EXT(lightweight);
                } else if( type == kptTable ) {
                    *ext = SRA_EXT(lightweight);
                } else {
                    rc = RC(rcSRA, rcPath, rcResolving, rcType, rcUnknown);        
                }
                KDBManagerRelease(kmgr);
            }
        }
    }
    return rc;
}

/* 
 * FlushPath
 * FlushRun
 * RunBGTasks
 *  stubbed functions to manipulate a cache, if implemented
 */
LIB_EXPORT rc_t CC SRAMgrGetCacheUsage( const SRAMgr *self, struct SRACacheUsage* stats )
{
    rc_t rc = 0;
    if ( self == NULL || stats == NULL || self->cache == NULL)
        rc = RC(rcSRA, rcMgr, rcAccessing, rcParam, rcNull);
    else
        rc = SRACacheGetUsage(self->cache, stats);
    return rc;
}
 
LIB_EXPORT rc_t CC SRAMgrFlush ( struct SRAMgr const *self, const struct SRACacheMetrics* thr )
{
    rc_t rc = 0;
    if( self == NULL  || self->cache == NULL)
        rc = RC(rcSRA, rcFile, rcProcessing, rcSelf, rcNull);
    else if( thr == NULL )
        rc = RC(rcSRA, rcFile, rcProcessing, rcParam, rcNull);
    else 
    {
        SRACacheMetrics saved;
        rc = SRACacheGetSoftThreshold(self->cache, &saved);
        if (rc == 0)
        {
            rc = SRACacheSetSoftThreshold(self->cache, thr);
            if (rc == 0)
            {
                rc_t rc2;
                rc = SRACacheFlush(self->cache);
                rc2 = SRACacheSetSoftThreshold(self->cache, &saved);
                if (rc == 0)
                    rc = rc2;
            }
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC SRAMgrRunBGTasks ( const SRAMgr *self )
{
    rc_t rc;

    if( self == NULL  )
        rc = RC(rcSRA, rcMgr, rcProcessing, rcSelf, rcNull);
        
    rc = SRACacheFlush(self->cache);
    
    return rc;
}

LIB_EXPORT rc_t CC SRAMgrConfigureCache( const SRAMgr *self,  int32_t soft_threshold, int32_t hard_threshold )
{
    rc_t rc = 0;
    if( self == NULL  || self->cache == NULL)
        rc = RC(rcSRA, rcFile, rcProcessing, rcSelf, rcNull);
    else if( soft_threshold < -1  || hard_threshold < -1 )
        rc = RC(rcSRA, rcFile, rcProcessing, rcParam, rcNull);
    else 
    {
        if ( soft_threshold > -1 )
        {
            SRACacheMetrics metrics;
            memset(&metrics, 0, sizeof(metrics));
            metrics.elements = soft_threshold;
            rc = SRACacheSetSoftThreshold(self->cache, &metrics);
        }
        if ( rc == 0 && hard_threshold > -1 )
        {
            SRACacheMetrics metrics;
            memset(&metrics, 0, sizeof(metrics));
            metrics.elements = hard_threshold;
            rc = SRACacheSetHardThreshold(self->cache, &metrics);
        }
        /* if a new threshold is low enough to cause a flush, do it right away */
        rc = SRACacheFlush(self->cache);
    }
    return rc;
}

/*--------------------------------------------------------------------------
 * SRANamelist
 *  redirecting functions
 */


LIB_EXPORT rc_t CC SRANamelistAddRef ( const SRANamelist *self )
{
    return KNamelistAddRef ( ( const KNamelist* ) self );
}

LIB_EXPORT rc_t CC SRANamelistRelease ( const SRANamelist *self )
{
    return KNamelistRelease ( ( const KNamelist* ) self );
}

LIB_EXPORT rc_t CC SRANamelistCount ( const SRANamelist *self, uint32_t *count )
{
    return KNamelistCount ( ( const KNamelist* ) self, count );
}

LIB_EXPORT rc_t CC SRANamelistGet ( const SRANamelist *self, uint32_t idx, const char **name )
{
    return KNamelistGet ( ( const KNamelist* ) self, idx, name );
}
