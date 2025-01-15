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

typedef struct WGS WGS;
typedef struct WGS_ListEntry WGS_ListEntry;
typedef struct WGS_List WGS_List;

struct WGS {
    struct VPath const *url;
    struct VCursor const *curs;
    uint32_t colID;
    uint64_t lastAccessStamp;
};

#define LIST_OBJECT WGS
#define LIST_ENTRY WGS_ListEntry
#include "list.h"

struct WGS_List {
    LIST;
    unsigned openCount;
    unsigned const openCountLimit;
};
#undef LIST
#undef LIST_ENTRY
#undef LIST_OBJECT

WGS_ListEntry *WGS_Find(WGS_List *list, unsigned const qlen, char const *qry);
WGS_ListEntry *WGS_Insert(WGS_List *list, unsigned const qlen, char const *qry, VPath const *url, VDatabase const *db, rc_t *prc);
void WGS_ListInit(WGS_List *list, unsigned openLimit);
void WGS_ListFree(WGS_List *list);
char const *WGS_Scheme(void);
unsigned WGS_splitName(int64_t *prow, unsigned namelen, char const *name);
unsigned WGS_getBases(WGS *self, uint8_t *dst, unsigned start, unsigned len, int64_t row);
void WGS_close(WGS *self);
rc_t WGS_reopen(WGS *self, VDBManager const *mgr, unsigned seq_id_len, char const *seq_id);
void WGS_limitOpen(WGS_List *list);
