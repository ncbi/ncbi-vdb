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

#include <klib/refcount.h>

#include <klib/container.h>
#include <klib/rc.h>

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------
 * forwards
 */

struct KNamelist;

/*--------------------------------------------------------------------------
 * KMDataNode
 *   base structure for KMDataNode implementations
 */
typedef struct KMDataNode KMDataNode;

typedef struct KMDataNode_vt KMDataNode_vt;
struct KMDataNode_vt
{
    /* Public API */
    rc_t ( CC * whack )         ( KMDataNode * self  );
    rc_t ( CC * addRef )        ( const KMDataNode * self );
    rc_t ( CC * release )       ( const KMDataNode * self );
    rc_t ( CC * byteOrder )     ( const KMDataNode * self, bool * reversed );
    rc_t ( CC * read )          ( const KMDataNode * self,size_t offset, void * buffer, size_t bsize, size_t * num_read, size_t * remaining );
    rc_t ( CC * openNodeRead )  ( const KMDataNode * self, const KMDataNode **node, const char *path, va_list args );
    rc_t ( CC * readAttr )      ( const KMDataNode * self, const char * name,char * buffer, size_t bsize, size_t *size );
    rc_t ( CC * compare )       ( const KMDataNode * self, KMDataNode const *other, bool *equal );
    rc_t ( CC * addr )          ( const KMDataNode * self, const void **addr, size_t *size );
    rc_t ( CC * listAttr )      ( const KMDataNode * self, struct KNamelist **names );
    rc_t ( CC * listChildren )  ( const KMDataNode * self, struct KNamelist **names );
};

// KMDataNodeListChildren

// Public write side-only API

struct KMDataNode
{
    BSTNode n;

    const KMDataNode_vt * vt;

    KRefcount refcount;
};

rc_t CC KMDataNodeWhack ( KMDataNode * node );

// default implelentations where exist
extern rc_t CC KMDataNodeBaseAddRef ( const KMDataNode *self );
extern rc_t CC KMDataNodeBaseWhack ( KMDataNode *self );
extern rc_t CC KMDataNodeBaseRelease ( const KMDataNode *self );

/* Attach
 * Sever
 *  like AddRef/Release, except called internally
 *  indicates that a child object is letting go...
 */
KMDataNode *KMDataNodeAttach ( const KMDataNode *self );
rc_t KMDataNodeSever ( const KMDataNode *self );

// write side only public API
// ...

#ifdef __cplusplus
}
#endif

