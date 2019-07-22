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

#include <kdb/extern.h>

#include "windex-priv.h"

#include <kdb/index.h>
#include <kfs/directory.h>
#include <kfs/file.h>
#include <kfs/md5.h>
#include <kfs/mmap.h>
#include <klib/pbstree.h>
#include <klib/rc.h>
#include <os-native.h>
#include <sysalloc.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct KU64Index_PNode_struct {
    uint64_t key;
    uint64_t key_size;
    int64_t id;
    uint64_t id_qty;
} KU64Index_PNode;

typedef struct KU64Index_Node_struct {
    BSTNode node;
    uint64_t key;
    uint64_t key_size;
    int64_t id;
    uint64_t id_qty;
} KU64Index_Node;

static
int64_t CC KU64Index_NodeSort( const BSTNode *item, const BSTNode *node )
{
    const KU64Index_Node* i = (const KU64Index_Node*)item;
    const KU64Index_Node* n = (const KU64Index_Node*)node;

    if( i->key < n->key ) {
        return -1;
    } else if( i->key > n->key ) {
        return 1;
    }
    return 0;
}

static
int64_t CC KU64Index_NodeSortUnique( const BSTNode *item, const BSTNode *node )
{
    const KU64Index_Node* i = (const KU64Index_Node*)item;
    const KU64Index_Node* n = (const KU64Index_Node*)node;

    if( (i->key + i->key_size - 1) < n->key ) {
        return -1;
    } else if( i->key > (n->key + n->key_size - 1) ) {
        return 1;
    }
    return 0;
}

static
bool CC KU64Index_UnrollPersisted( PBSTNode *n, void *data )
{
    KU64Index_v3* self = data;
    const KU64Index_PNode* pn = n->data.addr;

    KU64Index_Node* node = calloc(1, sizeof(KU64Index_Node));
    if( node == NULL ) {
        self->rc = RC(rcExe, rcNode, rcConstructing, rcMemory, rcInsufficient);
    } else {
        node->key = pn->key;
        node->key_size = pn->key_size;
        node->id = pn->id;
        node->id_qty = pn->id_qty;
        self->rc = BSTreeInsert(&self->tree, &node->node, KU64Index_NodeSort);
    }
    if( self->rc != 0 ) {
        free(node);
    }
    return self->rc == 0 ? false : true;
}

rc_t KU64IndexOpen_v3(KU64Index_v3* self, struct KMMap const *mm, bool byteswap)
{
    rc_t rc = 0;
    const char* maddr;
    size_t msize;
    PBSTree* ptree = NULL;

    self->rc = 0;
    BSTreeInit(&self->tree);

    /* when opened for create, there will be no existing index */
    if( mm == NULL ) {
        return 0;
    }

    /* open the prior index in persisted mode */
    rc = KMMapAddrRead(mm, (const void**)&maddr);
    if( rc != 0 ) {
        return rc;
    }
    rc = KMMapSize(mm, &msize);
    if( rc != 0 ) {
        return rc;
    }
    if( msize <= sizeof(struct KIndexFileHeader_v3) ) {
        return 0;
    }

    rc = PBSTreeMake(&ptree, (const void**)(maddr + sizeof(struct KIndexFileHeader_v3)),
                     msize - sizeof(struct KIndexFileHeader_v3), byteswap);
    if( rc != 0 ) {
        return rc;
    }

    PBSTreeDoUntil(ptree, false, KU64Index_UnrollPersisted, self);
    rc = self->rc;

    PBSTreeWhack(ptree);

    if( rc != 0 ) {
        KU64IndexWhack_v3(self);
    }
    return rc;
}

static
void CC KU64Index_WhackBSTree( BSTNode *n, void *data )
{
    free((KU64Index_Node*)n);
}

rc_t KU64IndexWhack_v3(KU64Index_v3* self)
{
    self->rc = 0;
    BSTreeWhack(&self->tree, KU64Index_WhackBSTree, NULL);
    return 0;
}

rc_t KU64IndexInsert_v3(KU64Index_v3* self, bool unique, uint64_t key, uint64_t key_size, int64_t id, uint64_t id_qty)
{
    KU64Index_Node* node = calloc(1, sizeof(KU64Index_Node));
    self->rc = 0;

    if( node == NULL ) {
        self->rc = RC(rcExe, rcNode, rcConstructing, rcMemory, rcInsufficient);
    } else {
        node->key = key;
        node->key_size = key_size;
        node->id = id;
        node->id_qty = id_qty;
        if( unique ) {
            self->rc = BSTreeInsertUnique(&self->tree, &node->node, NULL, KU64Index_NodeSortUnique);
        } else {
            self->rc = BSTreeInsert(&self->tree, &node->node, KU64Index_NodeSort);
        }
    }
    if( self->rc != 0 ) {
        free(node);
    }
    return self->rc;
}

static
int64_t CC KU64Index_Cmp4Delete( const void *item, const BSTNode *node )
{
    const KU64Index_Node* i = (const KU64Index_Node*)item;
    const KU64Index_Node* n = (const KU64Index_Node*)node;

    if( i->key < n->key ) {
        return -1;
    } else if( i->key > n->key ) {
        return 1;
    }
    return 0;
}

rc_t KU64IndexDelete_v3(KU64Index_v3* self, uint64_t key)
{
    KU64Index_Node node;
    BSTNode* n = NULL;

    self->rc = 0;
    node.key = key;
    n = BSTreeFind(&self->tree, &node, KU64Index_Cmp4Delete);
    if( n != NULL ) {
        if( !BSTreeUnlink(&self->tree, n) ) {
            self->rc = RC(rcDB, rcIndex, rcDestroying, rcId, rcCorrupt);
        }
    } else {
        self->rc = RC(rcDB, rcIndex, rcDestroying, rcId, rcNotFound);
    }
    return self->rc;
}

typedef struct KU64Index_PersistData_struct
{
    uint64_t pos;
    KFile *file;
    KMD5File *file_md5;
} KU64Index_PersistData;

static
rc_t CC KU64Index_WriteFunc( void *param, const void *buffer, size_t size, size_t *num_writ )
{
    KU64Index_PersistData* pd = param;
    rc_t rc = KFileWrite(pd->file, pd->pos, buffer, size, num_writ);
    pd->pos += *num_writ;
    return rc;
}

static
rc_t CC KU64Index_AuxFunc(void *param, const void *node, size_t *num_writ, PTWriteFunc write, void *write_param )
{
    rc_t rc = 0;
    const KU64Index_Node* n = (const KU64Index_Node*)node;
    int sz = sizeof(KU64Index_Node) - sizeof(BSTNode);

    if( write != NULL ) {
        rc = (*write)(write_param, &n->key, sz, num_writ);
    } else {
        *num_writ = sz;
    }
    return rc;
}

rc_t KU64IndexPersist_v3(KU64Index_v3* self, bool proj, KDirectory *dir, const char *path, bool use_md5)
{
    KU64Index_PersistData pd;
    char tmpname[256];
    char tmpmd5name[256];
    char md5path[256];

    self->rc = 0;
    memset(&pd, 0, sizeof(KU64Index_PersistData));

    self->rc = KDirectoryResolvePath(dir, false, tmpname, sizeof(tmpname), "%s.tmp", path);

    if( self->rc == 0 ) {
        self->rc = KDirectoryCreateFile(dir, &pd.file, true, 0664, kcmInit, "%s", tmpname);

        if(use_md5 && self->rc == 0 ) {

            KMD5SumFmt *km = NULL;
            size_t tmplen = snprintf(tmpmd5name, sizeof(tmpmd5name), "%s.md5", tmpname);
            KFile* kf = NULL;

            if( tmplen >= sizeof(tmpmd5name) ) {
                self->rc = RC(rcDB, rcIndex, rcPersisting, rcName, rcExcessive);
            } else {

                tmplen = snprintf(md5path, sizeof(md5path), "%s.md5", path);
                if( tmplen >= sizeof(md5path) ) {
                    self->rc = RC(rcDB, rcIndex, rcPersisting, rcName, rcExcessive);
                } else {

                    self->rc = KDirectoryCreateFile(dir, &kf, true, 0664, kcmInit, "%s", tmpmd5name);
                    if( self->rc == 0 ) {
                        self->rc = KMD5SumFmtMakeUpdate(&km, kf);
                        if( self->rc == 0 ) {
                            KMD5File * k5f;
                            kf = NULL;
                            self->rc = KMD5FileMakeWrite(&k5f, pd.file, km, path);
                            if( self->rc == 0 ) {
                                pd.file_md5 = k5f;
                                pd.file = KMD5FileToKFile(k5f);
                            }
                            /* release pass or fail */
                            KMD5SumFmtRelease(km);
                        } else {
                            KFileRelease ( kf );
                        }
                    } else {
                        KFileRelease ( kf );
                    }
                }
            }
            if( self->rc != 0 ) {
                KFileRelease(pd.file);
            }
        }

        if( self->rc == 0 ) {
            struct KIndexFileHeader_v3 head;
            size_t writ = 0;

            KDBHdrInit(&head.h, 3);
            head.index_type = kitU64;
            self->rc = KFileWrite(pd.file, pd.pos, &head, sizeof(struct KIndexFileHeader_v3), &writ);
            if( self->rc == 0 ) {
                pd.pos += writ;
                if( use_md5 ) {
                    KMD5FileBeginTransaction(pd.file_md5);
                }
                self->rc = BSTreePersist(&self->tree, NULL, KU64Index_WriteFunc, &pd, KU64Index_AuxFunc, &pd);
            }
            KFileRelease(pd.file);
            pd.file = NULL;
        }
    }

    if( self->rc == 0 ) {
        self->rc = KDirectoryRename(dir, false, tmpname, path);
        if( self->rc == 0 ) {
            if ( use_md5 ) {
                self->rc = KDirectoryRename(dir, false, tmpmd5name, md5path);
            }
            if( self->rc == 0 ) {
                /* done */
                return 0;
            }
        }
    }

    /* whack temporary file */
    KDirectoryRemove(dir, false, "%s", tmpname);
    if( use_md5 ) {
        KDirectoryRemove(dir, false, "%s", tmpmd5name);
    }
    return self->rc;
}

typedef struct KU64Index_GrepData_struct {
    rc_t rc;
    rc_t (CC*func)(uint64_t key, uint64_t key_size, int64_t id, uint64_t id_qty, void* data);
    void* data;
    KU64Index_Node search;
    uint64_t* key;
    uint64_t* key_size;
    int64_t* id;
    uint64_t* id_qty;
} KU64Index_GrepData;

/*
 * return true: if found or break DoUntil for FindAll
 */
static
bool CC KU64Index_Grep(BSTNode *node, void *data)
{
    KU64Index_Node* n = (KU64Index_Node*)node;
    KU64Index_GrepData* d = data;

    if( d->search.key >= n->key && (d->search.key - n->key) < n->key_size ) {
        if( d->func ) {
            d->rc = (*d->func)(n->key, n->key_size, n->id, n->id_qty, d->data);
            if( d->rc != 0 ) {
                return true;
            }
        } else {
            *d->key = n->key;
            *d->key_size = n->key_size;
            *d->id = n->id;
            *d->id_qty = n->id_qty;
            return true;
        }
    }
    return false;
}

rc_t KU64IndexFind_v3( const KU64Index_v3* self, uint64_t offset, uint64_t* key, uint64_t* key_size, int64_t* id, uint64_t* id_qty )
{
    KU64Index_GrepData d;

    memset(&d, 0, sizeof(KU64Index_GrepData));
    d.search.key = offset;
    d.key = key;
    d.key_size = key_size;
    d.id = id;
    d.id_qty = id_qty;
    if( !BSTreeDoUntil(&self->tree, false, KU64Index_Grep, &d) ) {
        d.rc = RC(rcDB, rcIndex, rcSelecting, rcId, rcNotFound);
    }
    return d.rc;
}


rc_t KU64IndexFindAll_v3( const KU64Index_v3* self, uint64_t offset, 
    rc_t (CC*f)(uint64_t key, uint64_t key_size, int64_t id, uint64_t id_qty, void* data), void* data)
{
    KU64Index_GrepData d;

    memset(&d, 0, sizeof(KU64Index_GrepData));
    d.func = f;
    d.data = data;
    d.search.key = offset;
    BSTreeDoUntil(&self->tree, false, KU64Index_Grep, &d);
    return d.rc;
}
