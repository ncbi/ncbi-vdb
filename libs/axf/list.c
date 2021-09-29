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

static int name_cmp(char const *name, unsigned const qlen, char const *qry)
{
    unsigned i;
    for (i = 0; i < qlen; ++i) {
        int const a = name[i];
        int const b = qry[i];
        int const d = a - b;
        if (a == 0) return d;
        if (d == 0) continue;
        return d;
    }
    return name[qlen] - '\0';
}

static bool find(List *list, unsigned *at, unsigned const qlen, char const *qry)
{
    unsigned f = 0;
    unsigned e = list->entries;

    while (f < e) {
        unsigned const m = f + (e - f) / 2;
        int const d = name_cmp(list->entry[m].name, qlen, qry);
        if (d == 0) {
            *at = m;
            return true;
        }
        if (d < 0)
            f = m + 1;
        else
            e = m;
    }
    *at = f; // it could be inserted here
    return false;
}

static void undo_insert(List *list, unsigned at)
{
    --list->entries;
    memmove(&list->entry[at], &list->entry[at + 1], sizeof(*list->entry) * (list->entries - at));
}

static Entry *insert(List *list, unsigned at, unsigned const namelen, char const *name)
{
    if (list->entries >= list->allocated) {
        unsigned const new_alloc = list->allocated == 0 ? 16 : (list->allocated * 2);
        void *tmp = realloc(list->entry, new_alloc * sizeof(*list->entry));
        if (tmp == NULL)
            return NULL;
        list->entry = tmp;
        list->allocated = new_alloc;
    }
    memmove(&list->entry[at + 1], &list->entry[at], sizeof(*list->entry) * (list->entries - at));
    memset(&list->entry[at], 0, sizeof(list->entry[at]));
    ++list->entries;

    list->entry[at].name = malloc(namelen + 1);
    if (list->entry[at].name == NULL) {
        undo_insert(list, at);
        return NULL;
    }
    memmove(list->entry[at].name, name, namelen);
    list->entry[at].name[namelen] = '\0';

    return &list->entry[at];
}
