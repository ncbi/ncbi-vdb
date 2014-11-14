/*===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software/database is a "United States Government Work" under the
*  terms of the United States Copyright Act.  It was readten as part of
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

#include <align/extern.h>

#include <klib/rc.h>
#include <klib/container.h>
#include <klib/text.h>
#include <klib/printf.h>
#include <klib/log.h>
#include <kdb/manager.h>
#include <kfg/config.h>
#include <insdc/insdc.h>
#include <vdb/database.h>
#include <vdb/vdb-priv.h>
#include <vdb/cursor.h>
#include <align/refseq-mgr.h>
#include <os-native.h>
#include <sysalloc.h>

#include "refseq-mgr-priv.h"
#include "debug.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <assert.h>

/*#define ALIGN_DBG KOutMsg*/

struct RefSeqMgr {
    BSTree tree;
    KConfig *kfg;
    const VDBManager* vmgr;
    uint32_t reader_options;
    size_t cache;
    uint32_t num_open_max;
    uint32_t num_open;
    uint64_t usage;
    RefSeq* last_used;
    BSTree vdbs;
};

struct RefSeq {
    BSTNode dad;
    const RefSeqMgr* mgr;
    const TableReaderRefSeq* reader;
    uint64_t usage;
    uint32_t accession_sz;
    /* must be last member of struct */
    char accession[1];
};

typedef struct RefSeqMgr_Db_struct {
    BSTNode dad;
    char key[4096];
    const VDatabase* db;
} RefSeqMgr_Db;

static
rc_t RefSeqMgr_ConfigValue ( const KConfig *kfg, const char *node_path, char *value, size_t value_size )
{
    const KConfigNode *node;
    rc_t rc = KConfigOpenNodeRead ( kfg, & node, "%s", node_path );
    if ( rc == 0 )
    {
        size_t num_read, remaining;
        rc = KConfigNodeRead ( node, 0, value, value_size - 1, & num_read,  & remaining );
        if ( rc == 0 )
        {
            if ( remaining != 0 )
                rc = RC ( rcSRA, rcMgr, rcConstructing, rcString, rcExcessive );
            else
                value [ num_read ] = 0;
        }
        
        KConfigNodeRelease ( node );
    }
    return rc;
}

static
rc_t RefSeqMgr_KfgReadRepositories(const KConfig* kfg, char* paths, size_t paths_sz)
{
    /* servers are children of refseq/repository, e.g.:             /refseq/repository/main="..." */ 
    /* volumes are in refseq/repository/<serverName>/volumes, e.g.: /refseq/repository/main/volumes="..." */
    /* all server/volume combinations are returned in paths separated by ':' */
    
    rc_t rc = 0;
    const KConfigNode *node;
#define KFG_PATH "/refseq/repository/"
    paths[0] = 0;
    
    rc = KConfigOpenNodeRead ( kfg, & node, KFG_PATH );
    if ( rc == 0 )
    {
        KNamelist* children;
        rc = KConfigNodeListChild ( node, &children );
        if ( rc == 0 )
        {
            uint32_t count;
            rc = KNamelistCount ( children, &count );
            if ( rc == 0 )
            {
                uint32_t i;
                for (i = 0; i < count; ++i) /* for all servers */
                {
                    const char* name;
                    rc = KNamelistGet ( children, i, &name );
                    if ( rc == 0 )
                    {
#define BufSize 4096
                        char server[ BufSize ];
                        char buf[ BufSize ];
                        size_t num_writ;
                        
                        rc = string_printf(buf, BufSize, &num_writ, KFG_PATH "%s", name);
                        if (rc == 0)
                        {
                            rc = RefSeqMgr_ConfigValue ( kfg, buf, server, sizeof(server) );
                            if (rc == 0)
                            {
                                rc = string_printf(buf, BufSize, &num_writ, KFG_PATH "%s/volumes", name);
                                if (rc == 0)
                                {
                                    char volumes[ BufSize ];
                                    rc = RefSeqMgr_ConfigValue ( kfg, buf, volumes, sizeof(volumes) );
                                    if (rc == 0)
                                    {   /* create a server/volume pair for every combination, append to paths, ':' - separate */ 
                                        char *vol_rem = volumes;
                                        char *vol_sep;
                                        
                                        do {
                                            char const *volume = vol_rem;
                                            vol_sep = string_chr(volume, string_size(volume), ':');
                                            if(vol_sep) {
                                                vol_rem = vol_sep + 1;
                                                *vol_sep = 0;
                                            }
                                            string_copy(paths + string_size(paths), paths_sz - string_size(paths), server, string_size(server));
                                            if (paths[string_size(paths)-1] != '/')
                                            {
                                                string_copy(paths + string_size(paths), paths_sz - string_size(paths), "/", 1);
                                            }
                                            string_copy(paths + string_size(paths), paths_sz - string_size(paths), volume, string_size(volume));
                                            string_copy(paths + string_size(paths), paths_sz - string_size(paths), ":", 1);
                                        } while(vol_sep);
                                    }
                                }
                            }
                        }
#undef BufSize
                    }
                    if ( rc != 0 )
                    {
                        break;
                    }
                }
            }
            KNamelistRelease ( children );
        }
        
        KConfigNodeRelease ( node );
    }
    if (GetRCState(rc) == rcNotFound)
    {
        paths[0] = '\0';
        return 0;
    }
    return 0;
}

static
rc_t RefSeqMgr_KfgReadStr(const KConfig* kfg, const char* path, char* value, size_t value_sz)
{
    rc_t rc = 0;
    const KConfigNode *node;
    
    if ( (rc = KConfigOpenNodeRead(kfg, &node, "%s", path)) == 0 ) {
        size_t num_read, remaining;
        if( (rc = KConfigNodeRead(node, 0, value, value_sz - 1, &num_read, &remaining)) == 0 ) {
            if( remaining != 0 ) {
                rc = RC(rcAlign, rcIndex, rcConstructing, rcString, rcTooLong);
            } else {
                value[num_read] = '\0';
            }
        }
        KConfigNodeRelease(node);
    } else if( GetRCState(rc) == rcNotFound ) {
        rc = 0;
        value[0] = '\0';
    }
    return rc;
}

rc_t RefSeqMgr_ForEachVolume(const RefSeqMgr* cself, RefSeqMgr_ForEachVolume_callback cb, void *data)
{
    rc_t rc = 0;
    char servers[4096];
    char volumes[4096];
    
    if( cself == NULL || cb == NULL ) {
        rc = RC(rcAlign, rcType, rcConstructing, rcParam, rcNull);
    } else if( cb(".", NULL, data) ) {
        /* found in local dir */
    } else if( (rc = RefSeqMgr_KfgReadStr(cself->kfg, "refseq/paths", servers, sizeof(servers))) != 0 ) {
        ALIGN_DBGERRP("%s", rc, "RefSeqMgr_KfgReadStr(paths)");
    } else {
        bool found = false;
        if( servers[0] != '\0' ) {
            char *srv_sep;
            char *srv_rem = servers;
            do {
                char const* server = srv_rem;
                
                srv_sep = strchr(server, ':');
                if(srv_sep) {
                    srv_rem = srv_sep + 1;
                    *srv_sep = 0;
                }
                if( cb(server, NULL, data) ) {
                    found = true;
                    break;
                }
            } while(srv_sep);
        }
        if( !found ) {
            /* locate refseq servers/volumes in possibly multiple repositories */
            if( (rc = RefSeqMgr_KfgReadRepositories(cself->kfg, servers, sizeof(servers))) != 0 ) {
                ALIGN_DBGERRP("%s", rc, "RefSeqMgr_KfgReadStr(refseq/repository/*)");
            };
            if( servers[0] != '\0' ) {
                char *srv_sep;
                char *srv_rem = servers;
                do {
                    char const* server = srv_rem;
                    
                    srv_sep = strchr(server, ':');
                    if(srv_sep) {
                        srv_rem = srv_sep + 1;
                        *srv_sep = 0;
                    }
                    if( cb(server, NULL, data) ) {
                        found = true;
                        break;
                    }
                } while(srv_sep);
            }
        }
        if( !found ) {
            if ( (rc = RefSeqMgr_KfgReadStr(cself->kfg, "refseq/servers", servers, sizeof(servers))) != 0 ||
                (rc = RefSeqMgr_KfgReadStr(cself->kfg, "refseq/volumes", volumes, sizeof(volumes))) != 0 ) {
                ALIGN_DBGERRP("%s", rc, "RefSeqMgr_KfgReadStr(servers/volumes)");
            } 
            /* servers and volumes are deprecated and optional */
            if( rc == 0 && (servers[0] != '\0' || volumes[0] != '\0') ) {
                char *srv_sep;
                char *srv_rem = servers;
                do {
                    char vol[ 4096 ];
                    char const *server = srv_rem;
                    char *vol_rem = vol;
                    char *vol_sep;
                    
                    string_copy ( vol, sizeof vol, volumes, string_size( volumes ) );
                    srv_sep = strchr(server, ':');
                    if(srv_sep) {
                        srv_rem = srv_sep + 1;
                        *srv_sep = 0;
                    }
                    do {
                        char const *volume = vol_rem;
                        
                        vol_sep = strchr(volume, ':');
                        if(vol_sep) {
                            vol_rem = vol_sep + 1;
                            *vol_sep = 0;
                        }
                        found = cb(server, volume, data);
                    } while(!found && vol_sep);
                } while(!found && srv_sep);
            }
        }
    }
    return rc;
}


static void RefSeqMgr_WhackAllReaders(RefSeqMgr *const mgr);

LIB_EXPORT rc_t CC RefSeqMgr_SetCache(RefSeqMgr const *const cself, size_t cache, uint32_t keep_open_num)
{
    if (cself) {
        RefSeqMgr *const self = (RefSeqMgr *)cself;
        
        RefSeqMgr_WhackAllReaders(self);
        assert(self->num_open == 0);
        self->cache = cache;
        self->num_open_max = keep_open_num;
    }
    return 0;
}

LIB_EXPORT rc_t CC RefSeqMgr_Make( const RefSeqMgr** cself, const VDBManager* vmgr,
                                   uint32_t reader_options, size_t cache, uint32_t keep_open_num )
{
    rc_t rc = 0;
    RefSeqMgr* obj = NULL;
    
    if ( cself == NULL || vmgr == NULL )
    {
        rc = RC( rcAlign, rcIndex, rcConstructing, rcParam, rcNull );
    }
    else
    {
        obj = calloc( 1, sizeof( *obj ) );
        if ( obj == NULL )
        {
            rc = RC( rcAlign, rcIndex, rcConstructing, rcMemory, rcExhausted );
        }
        else
        {
            rc = KConfigMake( &obj->kfg, NULL );
            if ( rc == 0 )
            {
                rc = VDBManagerAddRef( vmgr );
                if ( rc == 0 )
                {
                    BSTreeInit( &obj->tree );
                    BSTreeInit( &obj->vdbs );
                    obj->vmgr = vmgr;
                    obj->cache = cache;
                    obj->num_open_max = keep_open_num;
                    obj->reader_options = reader_options;
                }
            }
        }
    }

    if ( rc == 0 )
    {
        *cself = obj;
/*        ALIGN_DBG( "max_open: %u", obj->num_open_max ); */
    }
    else
    {
        RefSeqMgr_Release( obj );
        ALIGN_DBGERR( rc );
    }
    return rc;
}

static
void CC RefSeqMgr_RefSeqWhack(BSTNode *n, void *data)
{
    RefSeq* self = (RefSeq*)n;
    TableReaderRefSeq_Whack(self->reader);
    free(self);
}

static
void CC RefSeqMgr_VDbRelease(BSTNode *n, void *data)
{
    RefSeqMgr_Db* self = (RefSeqMgr_Db*)n;
    VDatabaseRelease(self->db);
    free(self);
}

LIB_EXPORT rc_t CC RefSeqMgr_Release(const RefSeqMgr* cself)
{
    if( cself != NULL ) {
        RefSeqMgr* self = (RefSeqMgr*)cself;
        BSTreeWhack(&self->tree, RefSeqMgr_RefSeqWhack, NULL);
        BSTreeWhack(&self->vdbs, RefSeqMgr_VDbRelease, NULL);
        VDBManagerRelease(self->vmgr);
        KConfigRelease(self->kfg);
        free(self);
    }
    return 0;
}

LIB_EXPORT rc_t RefSeqMgr_Exists(const RefSeqMgr* cself, const char* accession, uint32_t accession_sz, char** path)
{
    rc_t rc = 0;

    if( cself == NULL || accession == NULL || accession_sz == 0 ) {
        rc = RC(rcAlign, rcIndex, rcAccessing, rcParam, rcNull);
    }
    else {
        VTable const *tbl = NULL;

        /* if "accession" is not a path,
           prepend special scheme to tell VResolver
           to treat WGS accessions as Refseq */
        if ( string_chr ( accession, accession_sz, '/' ) == NULL )
            rc = VDBManagerOpenTableRead(cself->vmgr, &tbl, NULL, "ncbi-acc:%.*s?vdb-ctx=refseq", accession_sz, accession);
        else
            rc = VDBManagerOpenTableRead(cself->vmgr, &tbl, NULL, "%.*s", accession_sz, accession);
        if (tbl == NULL) {
            ALIGN_DBG("failed to open %.*s", accession_sz, accession);
            ALIGN_DBGERR(rc);
        }
        VTableRelease(tbl);
    }
    return rc;
}

static
int CC RefSeq_Cmp(const void* item, const BSTNode* node)
{
    const String* i = (const String*)item;
    const RefSeq* n = (const RefSeq*)node;

    if( i->size == n->accession_sz ) {
        return strncasecmp(i->addr, n->accession, i->size);
    }
    return i->size - n->accession_sz;
}

static
int CC RefSeq_Sort(const BSTNode* item, const BSTNode* node)
{
    const RefSeq* i = (const RefSeq*)item;
    const RefSeq* n = (const RefSeq*)node;

    if( i->accession_sz == n->accession_sz ) {
        return strncasecmp(i->accession, n->accession, i->accession_sz);
    }
    return i->accession_sz - n->accession_sz;
}

LIB_EXPORT rc_t CC RefSeqMgr_Read(const RefSeqMgr* cself, const char* seq_id, uint32_t seq_id_sz,
                                  INSDC_coord_zero offset, INSDC_coord_len len,
                                  uint8_t* buffer, INSDC_coord_len* written)
{
    rc_t rc = 0;
    const RefSeq* obj;

    if( (rc = RefSeqMgr_GetSeq(cself, &obj, seq_id, seq_id_sz)) == 0 ) {
        rc = RefSeq_Read(obj, offset, len, buffer, written);
        RefSeq_Release(obj);
    }
    return rc;
}

static void RefSeqMgr_WhackReader(RefSeqMgr *const mgr, RefSeq *const obj)
{
    if (obj->reader) {
        TableReaderRefSeq_Whack(obj->reader);
        obj->reader = NULL;
        --mgr->num_open;
    }
}

static
void CC RefSeq_ReaderWhacker(BSTNode *node, void *Data)
{
    RefSeq *const obj = (RefSeq *)node;
    RefSeqMgr *const mgr = (RefSeqMgr *)Data;
    
    RefSeqMgr_WhackReader(mgr, obj);
}

static void RefSeqMgr_WhackAllReaders(RefSeqMgr *const mgr)
{
    BSTreeForEach(&mgr->tree, false, RefSeq_ReaderWhacker, mgr);
}

static
void CC RefSeq_Oldest( BSTNode *node, void *Data )
{
    RefSeq const *const n = (RefSeq const *)node;
    RefSeq const **const rslt = (RefSeq const **)Data;
    RefSeq const *const d = *rslt;
    
    if (n->reader != NULL && (d == NULL || n->usage < d->usage))
        *rslt = n;
}

static
rc_t RefSeqMgr_GetReader(RefSeqMgr *const mgr, RefSeq *const obj)
{
    rc_t rc = 0;
    
    if (obj->reader == NULL) {
        VTable const *tbl;
        
        while (mgr->num_open_max > 0 && mgr->num_open >= mgr->num_open_max) {
            RefSeq *old;
            
        REMOVE_OLDEST:
            old = NULL;
            BSTreeForEach(&mgr->tree, false, RefSeq_Oldest, &old);
            if (old == NULL)
                break;
            
            RefSeqMgr_WhackReader(mgr, old);
        }
        /* if "accession" is not a path,
           prepend special scheme to tell VResolver
           to treat WGS accessions as Refseq */
        if ( string_chr ( obj -> accession, obj -> accession_sz, '/' ) == NULL )
            rc = VDBManagerOpenTableRead(mgr->vmgr, &tbl, NULL, "ncbi-acc:%.*s?vdb-ctx=refseq", obj->accession_sz, obj->accession);
        else
            rc = VDBManagerOpenTableRead(mgr->vmgr, &tbl, NULL, "%.*s", obj->accession_sz, obj->accession);

        if (rc == 0)
            rc = TableReaderRefSeq_MakeTable(&obj->reader, mgr->vmgr, tbl,
                                             mgr->reader_options, mgr->cache);
        if (rc == 0) {
            VTableRelease(tbl);
            ++mgr->num_open;
        }
        else if ( GetRCObject(rc) == rcMemory &&
                 (GetRCState (rc) == rcInsufficient || GetRCState(rc) == rcExhausted))
        {
            if (mgr->num_open_max == 0 || mgr->num_open < mgr->num_open_max)
                return rc;
            rc = 0;
            goto REMOVE_OLDEST;
        }
        else
            return rc;
    }
    mgr->last_used = obj;
    return rc;
}

static
rc_t RefSeqMgr_GetSeqInternal(RefSeqMgr *const mgr, RefSeq const **const cself,
                              char const seq_id[], unsigned const seq_id_sz)
{
    
    rc_t rc;
    RefSeq *obj;
    String s;
    
    StringInit(&s, seq_id, seq_id_sz, seq_id_sz);
    
    *cself = NULL;
    if (mgr->last_used != NULL && RefSeq_Cmp(&s, &mgr->last_used->dad) == 0)
        obj = mgr->last_used;
    else {
        obj = (RefSeq*)BSTreeFind(&mgr->tree, &s, RefSeq_Cmp);
        if (obj == NULL) {
            obj = calloc(1, sizeof(*obj) + seq_id_sz);
            if (obj) {
                memcpy(obj->accession, seq_id, seq_id_sz);
                obj->accession_sz = seq_id_sz;
                obj->mgr = mgr;
                rc = BSTreeInsertUnique(&mgr->tree, &obj->dad, NULL, RefSeq_Sort);
                if (rc) {
                    free(obj);
                    return rc;
                }
            }
            else
                return RC(rcAlign, rcIndex, rcConstructing, rcMemory, rcExhausted);
        }
    }
    rc = RefSeqMgr_GetReader(mgr, obj);
    *cself = obj;
    
    return rc;
}

LIB_EXPORT rc_t CC RefSeqMgr_GetSeq(const RefSeqMgr* cmgr, const RefSeq** cself, const char* seq_id, uint32_t seq_id_sz)
{
    rc_t rc;

    if (cmgr == NULL || cself == NULL || seq_id == NULL)
        rc = RC(rcAlign, rcIndex, rcConstructing, rcParam, rcNull);
    else
        rc = RefSeqMgr_GetSeqInternal((RefSeqMgr *)cmgr, cself, seq_id, seq_id_sz);

    if (rc)
        ALIGN_DBGERRP("SEQ_ID: '%.*s'", rc, seq_id_sz, seq_id);

    return rc;
}

LIB_EXPORT rc_t CC RefSeq_Read(const RefSeq* cself, INSDC_coord_zero offset, INSDC_coord_len len,
                               uint8_t* buffer, INSDC_coord_len* written)
{
    rc_t rc = 0;

    if( cself == NULL || buffer == NULL || written == NULL ) {
        rc = RC(rcAlign, rcFile, rcReading, rcParam, rcNull);
    }
    else {
        RefSeq *const self = (RefSeq *)cself;
        RefSeqMgr *const mgr = (RefSeqMgr *)self->mgr;
        
        rc = RefSeqMgr_GetReader(mgr, self);
        if (rc == 0) {
            TableReaderRefSeq const *const reader = self->reader;
            
            rc = TableReaderRefSeq_Read(reader, offset, len, buffer, written);
            if (rc == 0)
                self->usage = ++mgr->usage;
        }
    }
    ALIGN_DBGERR(rc);
    return rc;
}

LIB_EXPORT rc_t CC RefSeq_Circular(const RefSeq* cself, bool* circular)
{
    rc_t rc = 0;

    if( cself == NULL ) {
        rc = RC(rcAlign, rcFile, rcReading, rcParam, rcNull);
    } else {
        rc = TableReaderRefSeq_Circular(cself->reader, circular);
    }
    ALIGN_DBGERR(rc);
    return rc;
}

LIB_EXPORT rc_t CC RefSeq_SeqLength(const RefSeq* cself, INSDC_coord_len* len)
{
    rc_t rc = 0;

    if( cself == NULL ) {
        rc = RC(rcAlign, rcFile, rcReading, rcParam, rcNull);
    } else {
        rc = TableReaderRefSeq_SeqLength(cself->reader, len);
    }
    ALIGN_DBGERR(rc);
    return rc;
}

LIB_EXPORT rc_t CC RefSeq_MD5(const RefSeq* cself, const uint8_t** md5)
{
    rc_t rc = 0;

    if( cself == NULL ) {
        rc = RC(rcAlign, rcFile, rcReading, rcParam, rcNull);
    } else {
        rc = TableReaderRefSeq_MD5(cself->reader, md5);
    }
    ALIGN_DBGERR(rc);
    return rc;
}

LIB_EXPORT rc_t CC RefSeq_Release(const RefSeq* cself)
{
    return 0;
}

