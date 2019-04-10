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

/** @brief: the id-to-key map entry
 **/
typedef struct IdMapEntry IdMapEntry;
struct IdMapEntry {
    char const *name;
    int64_t firstId, lastId;
};

/** @brief: holds key values
 **
 ** @note: buffer CAN NOT be realloc'ed
 **/
typedef struct BufferListEntry BufferListEntry;
struct BufferListEntry {
    BufferListEntry *prv;
    char *buffer;
    size_t used;
};

typedef struct KHTIndexHdr IndexHeader;
struct KHTIndexHdr {
    KIndexFileHeader_v3 dad; /* index-cmn.h:65 */
    uint64_t entries;
    int64_t numIds, firstId, lastId;
    uint64_t cmp_idsize, cmp_spansize;
    uint64_t keysize, cmp_keysize;
};

/** @brief: measure a nul-terminated string
 **/
static size_t cstring_byte_count(char const *const cstring)
{
    size_t n = 0;
    while (cstring[n] != '\0')
        ++n;
    return n;
}

/** @brief: add the key of an entry in the id-to-key map to the hashtable
 **/
static bool addEntryToHashTable(KHTIndex_v5 *const self, size_t const i, rc_t *rc)
{
    IdMapEntry const *const entry = self->entries + i;
    *rc = KHashTableAdd(self->hashtable, entry->name, KHashCStr(entry->name), &i);
    return *rc == 0;
}
