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

#include <sra/wsradb.h>
#include <os-native.h>

#define KONST
#include "sra-priv.h"
#include <sra/types.h>
#include <vdb/manager.h>
#include <vdb/schema.h>
#include <vdb/table.h>
#include <vdb/cursor.h>
#include <vdb/vdb-priv.h>
#include <kdb/manager.h>
#include <klib/refcount.h>
#include <klib/log.h>
#include <klib/rc.h>
#include <klib/text.h>

#include <stdlib.h>
#include <sysalloc.h>

#define RC_MODULE (rcSRA)
#define RC_TARGET (rcMgr)
#define CLASS "SRAMgr"


/* MakeUpdate
 *  create library handle for read/write access
 *
 *  "wd" [ IN, NULL OKAY ] - optional working directory for
 *  accessing the file system. mgr will attach its own reference.
 *
 *  NB - not implemented in read-only library,
 *  and the read-only library may not be mixed with read/write
 */
LIB_EXPORT rc_t CC SRAMgrMakeUpdate ( SRAMgr **mgrp, struct KDirectory *wd )
{
    rc_t rc;

    if ( mgrp == NULL )
        rc = RC ( rcSRA, rcMgr, rcConstructing, rcParam, rcNull );
    else
    {
        VDBManager *vmgr;
        rc = VDBManagerMakeUpdate ( & vmgr, wd );
        if ( rc == 0 )
        {
            SRAMgr *mgr;
            rc = SRAMgrMake ( & mgr, vmgr , NULL );
            if ( rc == 0 )
            {
                mgr -> read_only = false;
                * mgrp = mgr;
                return 0;
            }

            VDBManagerRelease ( vmgr );
        }

        * mgrp = NULL;
    }

    return rc;
}


/* Drop
 *  drop an existing table
 *
 *  "force" [ IN ] - if true, make every attempt to remove table
 *
 *  "path" [ IN ]  - NUL terminated table name
 */
LIB_EXPORT rc_t CC SRAMgrDropTable( SRAMgr *self, bool force, const char *path, ... ) {
    va_list va;
    rc_t rc;
    
    va_start(va, path);
    rc = SRAMgrVDropTable(self, force, path, va);
    va_end(va);
    return rc;
}

LIB_EXPORT rc_t CC SRAMgrVDropTable( SRAMgr *self, bool force, const char *path, va_list args ) {
    if (self == NULL)
        return RC(RC_MODULE, RC_TARGET, rcRemoving, rcSelf, rcNull);
    if (path == NULL)
        return RC(RC_MODULE, RC_TARGET, rcRemoving, rcParam, rcNull);
    return VDBManagerVDrop(self->vmgr, kptTable, path, args);
}

LIB_EXPORT rc_t CC SRAMgrVLock ( SRAMgr *self, const char *path, va_list args ) {
    if (self == NULL)
        return RC(RC_MODULE, RC_TARGET, rcRemoving, rcSelf, rcNull);
    if (path == NULL)
        return RC(RC_MODULE, RC_TARGET, rcRemoving, rcParam, rcNull);
    return VDBManagerVLock(self->vmgr, path, args);
}

LIB_EXPORT rc_t CC SRAMgrLock ( SRAMgr *self, const char *path, ... ) {
    va_list va;
    rc_t rc;
    
    va_start(va, path);
    rc = SRAMgrVLock(self, path, va);
    va_end(va);
    return rc;
}

LIB_EXPORT rc_t CC SRAMgrVUnlock ( SRAMgr *self, const char *path, va_list args ) {
    if (self == NULL)
        return RC(RC_MODULE, RC_TARGET, rcRemoving, rcSelf, rcNull);
    if (path == NULL)
        return RC(RC_MODULE, RC_TARGET, rcRemoving, rcParam, rcNull);
    return VDBManagerVUnlock(self->vmgr, path, args);
}

LIB_EXPORT rc_t CC SRAMgrUnlock ( SRAMgr *self, const char *path, ... ) {
    va_list va;
    rc_t rc;
    
    va_start(va, path);
    rc = SRAMgrVUnlock(self, path, va);
    va_end(va);
    return rc;
}


/* SetMD5Mode
 *  sets default MD5 file mode for all objects
 *  opened for update under table
 */
LIB_EXPORT rc_t CC SRAMgrSetMD5Mode( SRAMgr *self, bool useMD5 ) {
    if (self == NULL)
        return RC(RC_MODULE, RC_TARGET, rcUpdating, rcSelf, rcNull);
    self->mode = useMD5 ? (self->mode | kcmMD5) : (self->mode & ~kcmMD5);
    return 0;
}
