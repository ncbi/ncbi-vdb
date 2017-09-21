/*===========================================================================
 *
 *                            Public DOMAIN NOTICE
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
 */

#include <kfs/extern.h>
#include <atomic32.h>
#include <klib/container.h>
#include <klib/vector.h>
#include <klib/pbstree.h>
#include <klib/text.h>
#include <kfs/arc.h>
#include <kfs/toc.h>
#include <kfs/directory.h>
#include <kfs/file.h>
#include <klib/log.h>
#include <klib/debug.h>
#include <klib/rc.h>
#include <klib/sort.h>
#include <sysalloc.h>

#include "toc-priv.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <byteswap.h>


/* ======================================================================
 * KTocEntry
 *
 * A private class used to keep track of a single entry (
 */


/* ======================================================================
 * KTocEntry method
 *
 * This class is not set up with a vtable at this point.  This API exposes
 * (due to the nature of using a c struct as a class) much of it's opaque 
 * elements by providing accessor functions to most of them.  Since they 
 * are static in this file they are expected to be used only by a class 
 * that is like a C++ friend class by being implemented in the same 
 * compilation unit.
 */

/* NOTE data is not used but required to match the signature for a pointer to function */
void CC KTocEntryWhack ( BSTNode * n, void * data )
{
    KTocEntry * p = (KTocEntry*)n;
    /**(rc_t*)data = */ KTocEntryDelete (p);
}

/* ----------------------------------------------------------------------
 * KTocEntryDelete
 * [INOUT] self the entry to delete
 */
rc_t KTocEntryDelete ( KTocEntry * self )
{
    /* defensive programming ensure minimally valid pointer */
    if (self == NULL)
    {
        return 0;
/* 	return RC (rcFS, rcToc, rcConstructing, rcSelf, rcNull); */
    }

    switch (self->type)
    {
    case ktocentrytype_unknown:
    case ktocentrytype_file:
    case ktocentrytype_chunked:
    case ktocentrytype_softlink:
    case ktocentrytype_hardlink:
    case ktocentrytype_zombiefile:
    default:
	/* -----
	 * at this point in the implementation there is nothing we need to
	 * do with these entry types except free their "self" allocated memory.
	 *
	 * Used a switch in case we did for one or more of these.
	 */
	break;
    case ktocentrytype_dir:
	/* -----
	 * directories have a BSTree below them that need to be 
	 * deleted first
	 */
	BSTreeWhack (&self->u.dir.tree, KTocEntryWhack, NULL);
	break;
    }
    free (self);
    return 0;
}

/* ----------------------------------------------------------------------
 * KTocEntryCmp2
 *
 * [RET] int					>0: if n > p
 *						0:  if n == p
 *						<0: if n < p
 * [IN]  const BSTNode * 	n		where the pointer to a node matching the key is put
 * [IN]  const BSTNode * 	p		where the pointer to a node matching the key is put
 * This function fits the function signature needed for BSTreeInsert
 *
 * can not inline or make into a macro as it is referenced via a pointer
 */
int64_t CC KTocEntryCmp2 ( const BSTNode * n, const BSTNode * p )
{

    KTocEntry * nn;
    KTocEntry * pp;
    int64_t		  ii;

    nn = (KTocEntry *)n;
    pp =  (KTocEntry *)p;
    ii = StringCompare (&nn->name, &pp->name);
    return ii;
}

/* ----------------------------------------------------------------------
 * KTocEntryCmpVoid
 *
 * [RET] int					>0: if n > p
 *						0:  if n == p
 *						<0: if n < p
 * [IN]  const void * 		item		'key' to find a matching bject in the BSTree
 * [OUT] const BSTNode *	node		where the pointer to a node matching the key is put
 * This function fits the function signature needed for BSTreeFind
 *
 * can not inline or make into a macro as it is referenced via a pointer
 */
int64_t CC KTocEntryCmpVoid ( const void * item, const BSTNode * node )
{
    const BSTNode * _item = item;
    return KTocEntryCmp2 (_item, node);
}

/* ----------------------------------------------------------------------
 * KTocEntryNew
 *
 * [RET] rc_t					0 for success; anything else for a failure
 *						see itf/klib/rc.h for general details
 * [OUT] KTocEntry ** 	new_entry	where to put a pointer to the new TOC Entry
 * [IN]  const char * 		name		name of the entry (file, link, directory...)
 *						(not assumed to be ASCIZ)
 * [IN]  size_t			name_size	length of name
 * [IN]  uint32_t 		access		unix/posix style permission flags
 * [IN]  size_t 		entry_specific	specific initialyers by entry type
 */
static
rc_t		KTocEntryNew		(KTocEntry ** new_entry,
					 const char * name,
					 size_t name_size,
					 KTime_t mtime,
					 uint32_t access,
					 size_t entry_specific)
{
    KTocEntry * entry;
    size_t	  nsize;
    char *	  namep;

    nsize = entry_specific + name_size + 1; /* we want a NUL at end of name */

    entry = malloc (nsize);
    if (entry == NULL)
    {
	LOGMSG (klogErr,
		"Failed to allocate for a TOC File entry");
	return RC (rcFS, rcToc, rcAllocating, rcMemory, rcInsufficient);
    }

    /* entry->entry is fine left as undefined */
    namep =(char*)entry + entry_specific;
    string_copy (namep, name_size+1, name, name_size);
    StringInit (&(entry->name), namep, name_size, (uint32_t)name_size);

    entry->mtime = mtime;
    entry->access = access;

    *new_entry = entry;
    return 0;
}

/* ----------------------------------------------------------------------
 * KTocEntryNewFile
 *
 * [RET] rc_t					0 for success; anything else for a failure
 *						see itf/klib/rc.h for general details
 * [OUT] KTocEntry **		new_entry	where to put a pointer to the new TOC Entry
 * [IN]  const char *		name		name of the file (not path) (not assumed to be ASCIZ)
 * [IN]  size_t 		name_size	length of name
 * [IN]  uint32_t 		access		unix/posix style permission flags
 * [IN]  uint64_t		offset		starting offset within the archive file of this file
 * [IN]  uint64_t 		size		number of bytes in the file
 */
rc_t KTocEntryNewFile ( KTocEntry ** new_entry,
                        const char * name,
                        size_t name_size,
                        KTime_t mtime,
                        uint32_t access,
                        uint64_t offset,
                        uint64_t size )
{
    rc_t	rc;

    TOC_SORT (("%s: %s %lu %u\n", __func__, name, mtime, access));

    rc = KTocEntryNew (new_entry, name, name_size, mtime, access, 
		       sizeof(KTocEntry) 
		       - sizeof(union KTocEntryUnion)
		       + sizeof(struct KTocEntryFile));

    if (rc)
	return rc;

    if (size == 0)
        (*new_entry)->type = ktocentrytype_emptyfile;

    else
    {
        (*new_entry)->type = ktocentrytype_file;

        (*new_entry)->u.contiguous_file.archive_offset = offset;
        (*new_entry)->u.contiguous_file.file_size = size;
    }

    return 0;
}


/* ----------------------------------------------------------------------
 * KTocEntryNewZombieFile
 *
 * A zombie file is a file whose directory is in the archive but whose 
 * storage is not
 *
 * [RET] rc_t					0 for success; anything else
 *						for a failure
 *                                              see itf/klib/rc.h for general
 *                                              details
 * [OUT] KTocEntry **		new_entry	where to put a pointer to the
 *                                              new TOC Entry
 * [IN]  const char *		name		name of the file (not path)
 *                                              (not assumed to be ASCIZ)
 * [IN]  size_t 		name_size	length of name
 * [IN]  uint32_t 		access		unix/posix style permission
 *                                              flags
 * [IN]  uint64_t		offset		starting offset within the
 *                                              archive file of this file
 * [IN]  uint64_t 		size		number of bytes in the file
 */
rc_t KTocEntryNewZombieFile ( KTocEntry ** new_entry,
                              const char * name,
                              size_t name_size,
                              KTime_t mtime,
                              uint32_t access,
                              uint64_t offset,
                              uint64_t size )
{
    rc_t	rc;
    
/*     TOC_DEBUG (("%s: %s %lu\n", __func__, name, mtime)); */

    rc = KTocEntryNew (new_entry, name, name_size, mtime, access, 
		       sizeof(KTocEntry) 
		       - sizeof(union KTocEntryUnion)
		       + sizeof(struct KTocEntryFile));

    if (rc)
	return rc;

    if (size == 0)
        (*new_entry)->type = ktocentrytype_emptyfile;

    else
    {
        (*new_entry)->type = ktocentrytype_zombiefile;

        (*new_entry)->u.contiguous_file.archive_offset = offset;
        (*new_entry)->u.contiguous_file.file_size = size;
    }

    return 0;
}

/* ----------------------------------------------------------------------
 * KTocEntryNewChunked
 *
 * [RET] rc_t					0 for success; anything else for a failure
 *						see itf/klib/rc.h for general details
 * [OUT] KTocEntry **		new_entry	where to put a pointer to the new TOC Entry
 * [IN]  const char *		name		name of the file (not path) (not assumed to be ASCIZ)
 * [IN]  size_t			name_size	length of name
 * [IN]  uint32_t		access		unix/posix style permission flags
 * [IN]  uint64_t 		size		virtual number of bytes in the file: not actual
 *						number of stored bytes
 * [IN]  const KTocChunk * 	chunks		pointer to an array of chunk structures	
 * [IN]  uint32_t 		num_chunks	number of chunks in the array above
 */
static
int64_t CC chunkcmp (const void * a, const void * b, void * ignored)
{
    const KTocChunk *A = a;
    const KTocChunk *B = b;
    /* -----
     * We can't rely on a cast down to int not getting messed up 
     * by overflow nor even on uint64_t - uint64_t not overflowing int64_t
     */
    if (A->logical_position == B->logical_position)
	return 0;
    else if (A->logical_position > B->logical_position)
	return 1;
    else
	return -1;
}

rc_t KTocEntryNewChunked ( KTocEntry ** new_entry,
					 const char * name,
					 size_t name_size,
					 KTime_t mtime,
					 uint32_t access,
					 uint64_t size,
					 const KTocChunk * chunks,
					 uint32_t num_chunks )
{
    rc_t	rc;
    KTocChunk * chunkp;
    size_t	nsize;
    size_t	csize;

    /* -----
     * This is a bit ugly...
     *
     * first (Compile time optimizations does much of the heavy lifting) figure out how
     * much is the extra malloc amount
     *
     * Take size of a generic entry - the size of the union part but add back the size of
     * the chunked file part
     *
     * Add to that the size of a 64 bit integer.  This is 8 bytes extra from what is
     * needed by the header alone.
     * 
     * Mask that against the binary bit inverse of 1 less tha the size of a 64 bit integer.
     *
     * Now you have the size of the header plus the number of bytes needed to get to a 8
     * byte address 0.  This is possibly more than is needed as 8 byte quantities could be
     * read from 4 byte boundaries in many cases.
     *
     * Then add to that the size in bytes of the chunked data (all 64 bit numbers).
     */
    nsize = ~( ( size_t ) sizeof(uint64_t)-1) & 
	(sizeof(KTocEntry)
	 - sizeof(union KTocEntryUnion)
	 + sizeof(struct KTocEntryChunkFile)
	 + sizeof(uint64_t));
    csize = sizeof(KTocChunk) * num_chunks;

    if ((rc = KTocEntryNew (new_entry, name, name_size, mtime, access, 
			    nsize + csize))
	!= 0)
    {
	return rc;
    }

    chunkp = (KTocChunk*)((char*)*new_entry + nsize);
    (*new_entry)->type = ktocentrytype_chunked;
    (*new_entry)->u.chunked_file.file_size = size;
    (*new_entry)->u.chunked_file.chunks = chunkp;
    (*new_entry)->u.chunked_file.num_chunks = num_chunks;
    memmove(chunkp, chunks, csize);
    ksort (chunkp, num_chunks, sizeof(KTocChunk), chunkcmp, NULL);
    /* -----
     * TODO: We currently do no validation of the chunks.
     * We accept that after the sort (which is probably superfluous)
     * that for each chunk 
     *
     *	chunkp[N].logical_position + chunkp[N].size <= chunkp[N+1].logical_position
     *
     * We should probably verify this.
     */
    return 0;
}

/* ----------------------------------------------------------------------
 * KTocEntryNewSoft
 *
 * [RET] rc_t					0 for success; anything else for a failure
 *						see itf/klib/rc.h for general details
 * [OUT] KTocEntry **		new_entry	where to put a pointer to the new TOC Entry
 * [IN]  const char * 		name		name of the file (not path) (not assumed to be ASCIZ)
 * [IN]  size_t 		name_size	length of name
 * [IN]  uint32_t		access		unix/posix style permission flags
 * [IN]  const char * 		link		character array (string) holding the name of the
 *						links target (not assumed to be ASCIZ)
 * [IN]  size_t 		link_size	length of the target string
 * Link is run time resolved
 */
rc_t KTocEntryNewSoft ( KTocEntry ** new_entry,
					 const char * name,
					 size_t name_size,
					 KTime_t mtime,
					 uint32_t access,
					 const char * link,
					 size_t link_size )
{
    rc_t	rc;
    char *	linkp;

    rc = KTocEntryNew (new_entry, name, name_size, mtime, access, 
		       (sizeof(KTocEntry)
			- sizeof(union KTocEntryUnion))
		       + sizeof(struct KTocEntrySoftLink) + link_size + 1);
    if (rc != 0)
    {
	return rc;
    }
    (*new_entry)->type = ktocentrytype_softlink;
    linkp =(char*)(*new_entry) + sizeof(KTocEntry) - sizeof(union KTocEntryUnion)
	+ sizeof(struct KTocEntrySoftLink);
    string_copy (linkp, link_size+1, link, link_size);
    StringInit ( &((*new_entry)->u.symbolic_link.link_path), linkp, link_size, (uint32_t)link_size );
    return 0;
}

/* ----------------------------------------------------------------------
 * KTocEntryNewHard
 *
 * [RET] rc_t					0 for success; anything else for a failure
 *						see itf/klib/rc.h for general details
 * [OUT] KTocEntry ** 	new_entry	where to put a pointer to the new TOC Entry
 * [IN]  const char * 		name		name of the file (not path) (not assumed to be ASCIZ)
 * [IN]  size_t			name_size	length of name
 * [IN]  uint32_t		access		unix/posix style permission flags
 * Resolution of the link is not set up in this function
 */
rc_t KTocEntryNewHard ( KTocEntry ** new_entry,
                        const char * name,
                        size_t name_size,
                        KTime_t mtime,
                        uint32_t access,
                        const KTocEntry * targ )
{
    rc_t	rc;

    if ((rc = KTocEntryNew (new_entry, name, name_size, mtime, access, 
			    sizeof(KTocEntry)
			    - sizeof(union KTocEntryUnion)
			    + sizeof(struct KTocEntryHardLink)))
	!= 0)
    {
	return rc;
    }
    (*new_entry)->type = ktocentrytype_hardlink;
    (*new_entry)->u.hard_link.ref = targ;

    return 0;
}

/* ----------------------------------------------------------------------
 * KTocEntryNewDirectory
 *
 * [RET] rc_t					0 for success; anything else for a failure
 *						see itf/klib/rc.h for general details
 * [OUT] KTocEntry **		new_entry	where to put a pointer to the new TOC Entry
 * [IN]  const char *		name		name of the directory (not assumed to be ASCIZ)
 * [IN]  size_t			name_size	length of name
 * [IN]  uint32_t		access		unix/posix style permission flags
 */
rc_t KTocEntryNewDirectory ( KTocEntry ** new_entry,
					 const char * name,
					 size_t name_size,
					 KTime_t mtime,
					 uint32_t access )
{
    rc_t	rc;

    /* -----
     * get rid of trailing '/' characters in a file name
     */
    while (name[name_size-1] == '/')
	--name_size;

    if ((rc = KTocEntryNew (new_entry, name, name_size, mtime, access, 
			    sizeof(KTocEntry)
			    - sizeof(union KTocEntryUnion)
			    + sizeof(struct KTocEntryDir)))
	!= 0)
    {
	return rc;
    }
    (*new_entry)->type = ktocentrytype_dir;
    BSTreeInit(&(*new_entry)->u.dir.tree);	/* start with an empty tree */
    return 0;
}

/* ----------------------------------------------------------------------
 *
 */
rc_t KTocEntryGetTime ( const KTocEntry *self, KTime_t *mtime )
{
    if (self == NULL)
    {
	return RC (rcFS, rcToc, rcAccessing, rcSelf, rcNull);
    }
    *mtime = self->mtime;
    return 0;
}

/* ----------------------------------------------------------------------
 *
 */
rc_t KTocEntryGetAccess ( const KTocEntry *self, uint32_t *access )
{
    if (self == NULL)
    {
	return RC (rcFS, rcToc, rcAccessing, rcSelf, rcNull);
    }
    *access = self->access;
    return 0;
}

/* ----------------------------------------------------------------------
 *
 */
rc_t KTocEntryGetType ( const KTocEntry *self, KTocEntryType* type )
{
    if (self == NULL)
    {
	return RC (rcFS, rcToc, rcAccessing, rcSelf, rcNull);
    }
    *type = self->type;
    return 0;
}

/* ----------------------------------------------------------------------
 *
 */
rc_t KTocEntryGetFileSize ( const KTocEntry * self, uint64_t * size )
{
    int	loopcount;
    const KTocEntry * target;

    assert (self != NULL);
    assert (size != NULL);

    *size = 0;

    for (loopcount = 0; loopcount < KARC_LINK_RESOLVE_LOOPMAX; ++loopcount)
    {
	switch (self->type)
	{
	default:
	case ktocentrytype_unknown:
	case ktocentrytype_notfound:
	    return RC (rcFS, rcFile, rcAccessing, rcFile, rcInvalid);

	case ktocentrytype_dir:
	    return RC (rcFS, rcFile, rcAccessing, rcFileDesc, rcIncorrect);

	case ktocentrytype_file:
	    *size = self->u.contiguous_file.file_size;
	    return 0;

	case ktocentrytype_chunked:
	    *size = self->u.chunked_file.file_size;
	    return 0;

	case ktocentrytype_zombiefile:
	    *size = self->u.zombie_file.file_size;
	    return 0;

	case ktocentrytype_softlink:
	    /* architect called for KArcDir to handle symbolic link resolution */
	    return RC (rcFS, rcFile, rcAliasing, rcLink, rcNoErr);

	case ktocentrytype_hardlink:
	    if (KTocEntryGetHardTarget(self, &target) != 0)
		return RC (rcFS, rcFile, rcAccessing, rcFile, rcInvalid);
	    /* this breaks "object oriented rules" but lets us loop instead of recur */
	    self = target; /* catch bad returns at reiteration of switch */
	    /* only non-terminal path within the loop */
	    break;

        case ktocentrytype_emptyfile:
	    *size = 0;
	    return 0;

	}
    }
    return RC (rcFS, rcFile, rcAccessing, rcFile, rcInvalid);
}

rc_t KTocEntryGetFilePhysicalSize ( const KTocEntry * self, uint64_t * size )
{
    int	loopcount;
    const KTocEntry * target;

    assert (self != NULL);
    assert (size != NULL);

    *size = 0;

    for (loopcount = 0; loopcount < KARC_LINK_RESOLVE_LOOPMAX; ++loopcount)
    {
	switch (self->type)
	{
	default:
	case ktocentrytype_unknown:
	case ktocentrytype_notfound:
	    return RC (rcFS, rcFile, rcAccessing, rcFile, rcInvalid);

	case ktocentrytype_dir:
	    return RC (rcFS, rcFile, rcAccessing, rcFileDesc, rcIncorrect);

	case ktocentrytype_file:
	    *size = self->u.contiguous_file.file_size;
	    return 0;

	case ktocentrytype_chunked:
        {
            uint64_t size_;
            uint32_t ix;
            for (size_ = 0, ix = 0; ix < self->u.chunked_file.num_chunks; ++ix)
                size_ += self->u.chunked_file.chunks[ix].size;
            *size = size_;
	    return 0;
        }
	case ktocentrytype_zombiefile:
	    *size = self->u.zombie_file.file_size;
	    return 0;

	case ktocentrytype_softlink:
	    /* architect called for KArcDir to handle symbolic link resolution */
	    return RC (rcFS, rcFile, rcAliasing, rcLink, rcNoErr);

	case ktocentrytype_hardlink:
	    if (KTocEntryGetHardTarget(self, &target) != 0)
		return RC (rcFS, rcFile, rcAccessing, rcFile, rcInvalid);
	    /* this breaks "object oriented rules" but lets us loop instead of recur */
	    self = target; /* catch bad returns at reiteration of switch */
	    /* only non-terminal path within the loop */
	    break;

        case ktocentrytype_emptyfile:
	    *size = 0;
	    return 0;

	}
    }
    return RC (rcFS, rcFile, rcAccessing, rcFile, rcInvalid);
}

/* ----------------------------------------------------------------------
 *
 */
rc_t KTocEntryGetFileLocator ( const KTocEntry * self, uint64_t * locator )
{
    int	loopcount;
    const KTocEntry * target;

    assert (self != NULL);
    assert (locator != NULL);

    *locator = 0;

    for (loopcount = 0; loopcount < KARC_LINK_RESOLVE_LOOPMAX; ++loopcount)
    {
	switch (self->type)
	{
	default:
	case ktocentrytype_unknown:
	case ktocentrytype_notfound:
	    return RC (rcFS, rcFile, rcAccessing, rcFile, rcInvalid);

	case ktocentrytype_dir:
	    return RC (rcFS, rcFile, rcAccessing, rcFileDesc, rcIncorrect);

	case ktocentrytype_file:
	case ktocentrytype_zombiefile:
	    *locator = self->u.contiguous_file.archive_offset;
	    return 0;
	case ktocentrytype_chunked:
        {
            /* find lowest offset chunk */
            if (self->u.chunked_file.num_chunks > 0)
            {
                uint64_t loc;
                uint32_t ix;

                *locator = self->u.chunked_file.chunks[0].source_position;
                for (ix = 1; ix < self->u.chunked_file.num_chunks; ++ix)
                {
                    loc = self->u.chunked_file.chunks[ix].source_position;
                    if (loc < *locator)
                        *locator = loc;
                }
            }
            else
                *locator = 0;

	    return 0;
        }
	case ktocentrytype_softlink:
	    /* architect called for KArcDir to handle symbolic link resolution */
	    return RC (rcFS, rcFile, rcAliasing, rcLink, rcNoErr);

	case ktocentrytype_hardlink:
	    if (KTocEntryGetHardTarget(self, &target) != 0)
		return RC (rcFS, rcFile, rcAccessing, rcFile, rcInvalid);
	    /* this breaks "object oriented rules" but lets us loop instead of recur */
	    self = target; /* catch bad returns at reiteration of switch */
	    /* only non-terminal path within the loop */
	    break;

        case ktocentrytype_emptyfile:
	    *locator = 0;
	    return 0;

	}
    }
    return RC (rcFS, rcFile, rcAccessing, rcFile, rcInvalid);
}

/* ----------------------------------------------------------------------
 *
 */
rc_t KTocEntryGetChunks ( const KTocEntry * self,
                          uint32_t * num_chunks,
                          const KTocChunk ** chunks )
{
    int	loopcount;
    const KTocEntry * target;

    *chunks = NULL;
    *num_chunks = 0;

    for (loopcount = 0; loopcount < KARC_LINK_RESOLVE_LOOPMAX; ++loopcount)
    {
	switch (self->type)
	{
	default:
	case ktocentrytype_unknown:
	case ktocentrytype_notfound:
	    return RC (rcFS, rcFile, rcAccessing, rcFile, rcInvalid);

	case ktocentrytype_dir:
	case ktocentrytype_file:
	    return RC (rcFS, rcFile, rcAccessing, rcFileDesc, rcIncorrect);

	case ktocentrytype_chunked:
	    *chunks = self->u.chunked_file.chunks;
	    *num_chunks = self->u.chunked_file.num_chunks;
	    return 0;

	case ktocentrytype_softlink:
	    /* architect called for KArcDir to handle symbolic link resolution */
	    return RC (rcFS, rcFile, rcAliasing, rcLink, rcNoErr);

	case ktocentrytype_hardlink:
	    if (KTocEntryGetHardTarget(self, &target) != 0)
		return RC (rcFS, rcFile, rcAccessing, rcFile, rcInvalid);
	    /* this breaks "object oriented rules" but lets us loop instead of recur */
	    self = target; /* catch bad returns at reiteration of switch */
	    /* only non-terminal path within the loop */
	    break;
	}
    }
    return RC (rcFS, rcFile, rcAccessing, rcFile, rcInvalid);
}

/* ----------------------------------------------------------------------
 *
 */
rc_t KTocEntryGetFileOffset ( const KTocEntry * self, uint64_t * offset )
{
    int			loopcount;
    const KTocEntry * target;

    *offset = 0;

    for (loopcount = 0; loopcount < KARC_LINK_RESOLVE_LOOPMAX; ++loopcount)
    {
	switch (self->type)
	{
	default:
	case ktocentrytype_unknown:
	case ktocentrytype_notfound:
	    return RC (rcFS, rcTocEntry, rcAccessing, rcFile, rcUnknown);

	case ktocentrytype_dir:
	case ktocentrytype_chunked:
	    return RC (rcFS, rcTocEntry, rcAccessing, rcSelf, rcUnsupported);

	case ktocentrytype_file:
	    *offset = self->u.contiguous_file.archive_offset;
	    return 0;

        case ktocentrytype_emptyfile:
	    *offset = 0;
	    return 0;

	case ktocentrytype_softlink:
	    /* architect called for KArcDir to handle symbolic link resolution */
	    return RC (rcFS, rcTocEntry, rcAliasing, rcLink, rcNoErr);

	case ktocentrytype_hardlink:
	    if (KTocEntryGetHardTarget(self, &target) != 0)
		return RC (rcFS, rcTocEntry, rcAccessing, rcSelf, rcInvalid);

	    /* this breaks "object oriented rules" but lets us loop instead of recur */
	    self = target; /* catch bad returns at reiteration of switch */
	    /* only non-terminal path within the loop */
	    break;
	}
    }
    return RC (rcFS, rcToc, rcAccessing, rcSelf, rcInvalid);	/* loop fail - too many hard links */
}

/* ----------------------------------------------------------------------
 *
 */
rc_t KTocEntryGetBSTree ( const KTocEntry * self,const BSTree ** ptree )
{
    KTocEntryType	type;

    if (self == NULL)
    {
	return RC (rcFS, rcToc, rcAccessing, rcSelf, rcNull);
    }
    if (KTocEntryGetType(self,&type) != 0)
    {
	return RC  (rcFS, rcToc, rcAccessing, rcParam, rcInvalid);
    }
    if (type != ktocentrytype_dir)
    {
	return RC  (rcFS, rcToc, rcAccessing, rcParam, rcInvalid);
    }
    *ptree = &self->u.dir.tree;
    return 0;
}

/* ----------------------------------------------------------------------
 *
 */
rc_t KTocEntryGetHardTarget ( const KTocEntry *self, const KTocEntry ** target )
{
    if (self == NULL)
    {
	return RC (rcFS, rcToc, rcAccessing, rcSelf, rcNull);
    }
    if (target == NULL)
    {
	return RC (rcFS, rcToc, rcAccessing, rcParam, rcNull);
    }
    *target = (self->type == ktocentrytype_hardlink) ? self->u.hard_link.ref : NULL;
    return 0;
}

/* ----------------------------------------------------------------------
 *
 */
rc_t KTocEntryGetSoftTarget ( const KTocEntry *self,
				const char ** target )
{
    if (self == NULL)
    {
	return RC (rcFS, rcToc, rcAccessing, rcSelf, rcNull);
    }
    if (self->type != ktocentrytype_softlink)
    {
	return RC  (rcFS, rcToc, rcAccessing, rcSelf, rcInvalid);
    }
    *target = self->u.symbolic_link.link_path.addr;
    return 0;
}

/* ----------------------------------------------------------------------
 *
 */
rc_t KTocEntryGetName ( const KTocEntry *self,
			  const char ** name )
{
    if (self == NULL)
    {
	return RC (rcFS, rcToc, rcAccessing, rcSelf, rcNull);
    }
    *name = self->name.addr;
    return 0;
}

/* ======================================================================
 */
const char * KTocEntryTypeGetString(KTocEntryType t)
{
    static const char * entryTypeString[] = 
	{
	    "ktocentrytype_unknown",
	    "ktocentrytype_notfound",
	    "ktocentrytype_dir",
	    "ktocentrytype_file",
	    "ktocentrytype_chunked",
	    "ktocentrytype_softlink",
	    "ktocentrytype_hardlink",
	    "ktocentrytype_emptyfile" 
	};

    switch (t)
    {
    case ktocentrytype_unknown:
    case ktocentrytype_dir:
    case ktocentrytype_file:
    case ktocentrytype_chunked:
    case ktocentrytype_softlink:
    case ktocentrytype_hardlink:
    case ktocentrytype_emptyfile:
	return entryTypeString[t+1];
    default:
	return "ktocentrytype_error";
    }
}

/*
 * preferred behavior of this needs to be determined
 */
LIB_EXPORT rc_t CC KTocEntryPersistWriteFunc ( void * param,
                                               const void * buffer,
                                               size_t size,
                                               size_t * num_writ )
{
    KTocEntryPersistWriteFuncData * data;
    rc_t rc;
    size_t to_write;

    assert (param != NULL);
    assert (buffer != NULL);
    assert (num_writ != NULL);

    rc = 0;
    *num_writ = 0;
    data = param;
    if (size != 0)
    {
	if ((data->buffptr + size) > data->limit)
	{
	    to_write = data->limit - data->buffptr;
	    rc = RC (rcFS, rcTocEntry, rcPersisting, rcBuffer, rcTooShort);
	}
	else
	    to_write = size;
	memmove (data->buffptr, buffer, to_write);
	data->buffptr += to_write;
	*num_writ = to_write;
#if 0
	{
	    size_t ix;

	    for (ix = 0; ix < to_write; ix ++)
	    {
/* please do not delete commented out code */
/* 		if ((ix & 0xF) == 0x0) */
/* 		    printf ("%.08x : ", ix); */
/* 		printf( "%.02x ", ((uint8_t*)buffer)[ix]); */

/* 		if ((ix & 0xF) == 0xF) */
/* 		    printf ("\n"); */
	    }
/*  	    printf ("\n"); */
	}
#endif
    }
    return rc;
}


static
rc_t KTocEntryPersistNodeCommon (void * param, const KTocEntry * n,
				 size_t * num_writ, PTWriteFunc write,
				 void * write_param)
{
    size_t all_written;
    size_t written;
    uint16_t nsize;
    const char * name;
    rc_t rc;
    KTocEntryType t;
    uint8_t b;
    KTime_t mtime;
    uint32_t access;

    rc = KTocEntryGetName (n, &name);
    if (rc != 0)
	return rc;
    TOC_DEBUG (("%s %s\n", __func__, name));
    if (write)
    {

/* please do not delete commented out code */
/* 	printf ("----------\n"); */
	nsize = (uint16_t)strlen(name);
	rc = KTocEntryGetTime (n, &mtime);
	if (rc != 0)
	    return rc;
	rc = KTocEntryGetAccess (n, &access);
	if (rc != 0)
	    return rc;
	rc = KTocEntryGetType (n, &t);
	if (rc != 0)
	    return rc;
	b = (uint8_t)t;
/* 	printf ("name size\t"); */
	rc = (*write) (write_param, &nsize, sizeof nsize, &all_written);
	if (rc == 0)
	{
/* 	    printf("name %s\t", name); */
	    rc = (*write) (write_param, name, nsize, &written);
	    all_written += written;
	    if (rc == 0)
	    {
/* 		printf("mtime\t"); */
		rc = (*write) (write_param, &mtime, sizeof mtime, &written);
		all_written += written;
		if (rc == 0)
		{
/* 		    printf("access\t"); */
		    rc = (*write) (write_param, &access, sizeof access, &written);
		    all_written += written;
		    if (rc == 0)
		    {
/* 			printf("type\t"); */
			rc = (*write) (write_param, &b, sizeof b, &written);
			all_written += written;
		    }
		}
	    }
	}
	*num_writ = all_written;
	return rc;
    }
    else
    {
	*num_writ = (sizeof (nsize) + strlen (name) + sizeof (mtime) +
		     sizeof (access) + sizeof (b));
	return 0;
    }
}

rc_t KTocEntryPersistNodeDir ( void *param, const KTocEntry * n,
			      size_t * num_writ, 
			      PTWriteFunc write, void * write_param )
{
    rc_t rc;

/*     size_t	start,end; */

    TOC_FUNC_ENTRY();


/*     start = *num_writ; */

    rc = BSTreePersist (&n->u.dir.tree,
			num_writ,
			write,
			write_param,
			KTocEntryPersist,
			NULL);
    if (rc != 0)
    {
        TOC_DEBUG (("KTocEntryPersistNodeDir: failure return from BSTreePersist"));
    }

/*     end = *num_writ; */
    return rc;
}

static
rc_t KTocEntryPersistNodeFile (void *param, const KTocEntry * n,
			       size_t * num_writ, 
			       PTWriteFunc write, void * write_param)
{
    TOC_FUNC_ENTRY();

    if (write)
    {
	rc_t rc;
	size_t all_written;
	size_t written;

/* please do not delete commented out code */
/* 	printf("file offset\t"); */
	rc = (*write) (write_param, &n->u.contiguous_file.archive_offset,
		       sizeof n->u.contiguous_file.archive_offset, &all_written);
	if (rc == 0)
	{
/* 	    printf("file size\t"); */
	    rc = (*write) (write_param, &n->u.contiguous_file.file_size,
			   sizeof n->u.contiguous_file.file_size, &written);
            all_written += written;
	}
	*num_writ = all_written;
	return rc;
    }
    else
    {
	*num_writ = (sizeof n->u.contiguous_file.archive_offset +
		     sizeof n->u.contiguous_file.file_size);
	return 0;
    }
}

static
rc_t KTocEntryPersistNodeChunked (void *param, const KTocEntry * n,
				  size_t * num_writ, 
				  PTWriteFunc write, void * write_param)
{
    KTocChunk * chunks;
    uint32_t count;

    count = n->u.chunked_file.num_chunks; /* used with write and no write */
    chunks = n->u.chunked_file.chunks;
    if (write)
    {
	rc_t rc;
	size_t all_written;
	size_t written;

/* please do not delete commented out code */
/* 	printf("file size\t"); */
	rc = (*write) (write_param, &n->u.chunked_file.file_size,
		       sizeof n->u.chunked_file.file_size, &all_written);
	if (rc == 0)
	{
/* 	    printf("chunk count\t"); */
	    rc = (*write) (write_param, &count, sizeof count, &written);
	    all_written += written;
	    if (rc == 0)
	    {
		for ( ; count--; chunks++)
		{
/* 		    printf("chunk l position\t"); */
 		    rc = (*write) (write_param, &chunks->logical_position,
 				   sizeof chunks->logical_position, &written);
		    all_written += written;
		    if (rc != 0)
			break;
/* 		    printf("chunk s position\t"); */
		    rc = (*write) (write_param, &chunks->source_position,
				   sizeof chunks->source_position, &all_written);
		    all_written += written;
		    if (rc != 0)
			break;
/* 		    printf("chunk size\t"); */
		    rc = (*write) (write_param, &chunks->size,
				   sizeof chunks->size, &written);
		    all_written += written;
		    if (rc != 0)
			break;
		}
	    }
	}
	*num_writ = all_written;
	return rc;
    }
    else
    {
	*num_writ = (sizeof n->u.chunked_file.file_size + sizeof count +
		     count * (sizeof chunks->logical_position +
			      sizeof chunks->source_position +
			      sizeof chunks->size));
	return 0;
    }
}

static
rc_t KTocEntryPersistNodeSym (void *param, const KTocEntry * n,
			      size_t * num_writ, PTWriteFunc write,
			      void * write_param)
{
    uint16_t nsize;

    nsize = (uint16_t)n->u.symbolic_link.link_path.size;
    if (write)
    {
	rc_t rc;
	size_t all_written;
	size_t written;

	rc = (*write) (write_param, &nsize, sizeof nsize, &all_written);
	if (rc == 0)
	{
	    rc = (*write) (write_param, 
			   n->u.symbolic_link.link_path.addr,
			   nsize,
			   &written);
	    all_written += written;
	}
	*num_writ = all_written;
	return rc;
    }
    else
    {
	*num_writ = (sizeof (nsize) + nsize);
	return 0;
    }
}

static
rc_t KTocEntryPersistNodeLink (void *param, const KTocEntry * n,
			       size_t * num_writ, 
			       PTWriteFunc write, void * write_param)
{
    uint16_t nsize;

    nsize = (uint16_t)n->u.hard_link.ref->name.size;
    if (write)
    {
	rc_t rc;
	size_t all_written;
	size_t written;

	rc = (*write) (write_param, &nsize, sizeof nsize, &all_written);
	if (rc == 0)
	{
	    rc = (*write) (write_param, 
			   n->u.hard_link.ref->name.addr,
			   nsize,
			   &written);
	    all_written += written;
	}
	*num_writ = all_written;
	return rc;
    }
    else
    {
	*num_writ = (sizeof (nsize) + nsize);
	return 0;
    }
}

LIB_EXPORT rc_t CC KTocEntryPersist ( void *param, const void * node,
                                      size_t * num_writ, 
                                      PTWriteFunc write, void * write_param )
{
    rc_t rc;
    size_t all_written;
    size_t written;
    const KTocEntry * n;

    TOC_FUNC_ENTRY();

    all_written = 0;
    n = (const KTocEntry *)node;
    rc = KTocEntryPersistNodeCommon (param, n, &all_written, write, write_param);
    if (rc == 0)
    {
	written = 0;
	switch (n->type)
	{
	default:
	    rc = RC (rcFS, rcTocEntry, rcPersisting, rcTocEntry, rcInvalid );
	    LOGERR (klogInt, rc, "malformed node with bad type");
	    break;

 	case ktocentrytype_notfound:
	    rc = RC (rcFS, rcTocEntry, rcPersisting, rcTocEntry, rcCorrupt );
	    LOGERR (klogInt, rc, "malformed tree node not found ");
 	    break;

	case ktocentrytype_dir:
	    /* recur */
/* please do not delete commented out code */
/* 	    printf("KTocEntryPersist Directory\n"); */
	    rc = KTocEntryPersistNodeDir (param, n, &written, write, write_param);
	    break;

	case ktocentrytype_file:
/* 	    printf("KTocEntryPersist File\n"); */
	    rc = KTocEntryPersistNodeFile (param, n, &written, write, write_param);
	    break;

	case ktocentrytype_emptyfile:
/* 	    printf("KTocEntryPersist Empty File\n"); */
	    break;

	case ktocentrytype_chunked:
/* 	    printf("KTocEntryPersist chunked File\n"); */
	    rc = KTocEntryPersistNodeChunked (param, n, &written, write, write_param);
	    break;

	case ktocentrytype_softlink:
/* 	    printf("KTocEntryPersist soft link\n"); */
	    rc = KTocEntryPersistNodeSym (param, n, &written, write, write_param);
	    break;

	case ktocentrytype_hardlink:
/* 	    printf("KTocEntryPersist hard link\n"); */
	    rc = KTocEntryPersistNodeLink (param, n, &written, write, write_param);
	    break;
	}
	all_written += written;
    }
    *num_writ = all_written;
    return rc;
}

typedef
struct KTocEntryInflateData
{
    KToc * toc;
    const char * path;
    uint64_t arcsize;
    uint64_t offset;
    rc_t rc;
    bool rev;
} KTocEntryInflateData;

typedef
struct KTocEntryInflateCommon
{
    char * name;
    KTime_t mtime;
    uint32_t access;
    KTocEntryType type;
} KTocEntryInflateCommon;
/* TBD: replace the list of parameters in the inflatenode functions
typedef
struct KTocEntryInflateNodeData
{
    KToc * toc;
    const void * ptr;
    const void * limit;
    char * name;
    KTime_t mtime;
    uint32_t access;
    KTocEntryType type;
    bool rev;
} KTocEntryInflateNodeData;
*/

static
bool check_limit (const void * ptr, const void * limit, size_t size)
{
    const uint8_t * p = ptr;
    const uint8_t * l = limit;
    return ((p + size) > l);
}

#define read_scalar(N,T,S)	     \
    static rc_t N (const void ** _ptr, const void * limit, bool rev, T * pout) \
    {									\
	const T * ptr;							\
									\
	if (check_limit (*_ptr, limit, sizeof (T)))			\
	    return RC (rcFS, rcTocEntry, rcParsing, rcBuffer, rcTooShort); \
	    								\
	ptr = *_ptr;							\
									\
	if (rev)							\
	{								\
	    T t;							\
	    memmove (&t, ptr, sizeof (T));				\
	    *pout = S (t);						\
	}								\
	else								\
	    memmove (pout, ptr, sizeof (T));                             \
	*_ptr = ++ptr;							\
	return 0;							\
    }

read_scalar (read_u16,uint16_t,bswap_16)
read_scalar (read_u32,uint32_t,bswap_32)
read_scalar (read_u64,uint64_t,bswap_64)
read_scalar (read_i64,int64_t,bswap_64)

static
rc_t read_u8 (const void ** _ptr, const void * limit, uint8_t * pout)
{
    const uint8_t * ptr;

    if (check_limit (*_ptr, limit, sizeof (uint8_t)))
	return RC (rcFS, rcTocEntry, rcParsing, rcBuffer, rcTooShort);

    ptr = *_ptr;
    *pout = *ptr++;
    *_ptr = ptr;
    return 0;
}

static
rc_t KTocEntryInflateNodeCommon (const void ** ptr,
				 const void * limit,
				 KTocEntryInflateCommon * common,
				 const char * path,
				 bool rev)
{
    rc_t rc;
    uint16_t plen;
    uint16_t nlen;

    rc = read_u16 (ptr, limit, rev, &nlen);
    if (rc)
	return rc;

    if (check_limit (*ptr, limit, nlen))
	return RC (rcFS, rcTocEntry, rcParsing, rcBuffer, rcTooShort);;

    plen = (uint16_t)strlen (path);
    if (plen == 0)
    {
	common->name = malloc (nlen+1);
	if (common->name == NULL)
	{
	    return RC (rcFS, rcTocEntry, rcInflating, rcMemory, rcExhausted);
	}
	memmove (common->name, *ptr, nlen);
	common->name[nlen] = '\0';
    }
    else
    {
	common->name = malloc (plen + 1 + nlen+1);
	if (common->name == NULL)
	{
	    return RC (rcFS, rcTocEntry, rcInflating, rcMemory, rcExhausted);
	}
	memmove (common->name, path, plen);
	common->name[plen] = '/';
	memmove (common->name+plen+1, *ptr, nlen);
	common->name[plen + nlen + 1] = '\0';
    }

    *ptr = ((uint8_t*)*ptr) + nlen;

    rc = read_i64 (ptr, limit, rev, &common->mtime);
    if (rc == 0)
    {
	rc =read_u32 (ptr, limit, rev, &common->access);
	if (rc == 0)
	{
	    uint8_t type;
	    rc = read_u8 (ptr, limit, &type);
	    if (rc == 0)
	    {
		common->type = type;
		return 0;
	    }
	}
    }
    free (common->name);
    common->name = NULL;
    common->mtime = 0;
    common->access = 0;
    common->type = ktocentrytype_unknown;
    return rc;
}


static
rc_t KTocEntryInflateNodeDir (KToc * toc, KTocEntryInflateCommon * common, 
			      const void ** ptr, uint64_t offset, uint64_t arcsize, const void * limit, bool rev)
{
    rc_t rc;

    rc = KTocCreateDir (toc,
			common->mtime,
			common->access,
			(KCreateMode)(kcmInit|kcmParents),
			common->name);
    if (rc == 0)
    {
	rc = KTocInflatePBSTree (toc, arcsize, *ptr, (uint32_t)( (uint8_t*)limit - (uint8_t*)*ptr ),
                                 offset, rev, common->name);
    }
    return rc;
}

static
rc_t KTocEntryInflateNodeFile (KToc * toc, const KTocEntryInflateCommon * common, 
			       const void ** ptr, uint64_t offset, uint64_t arcsize, const void * limit, bool rev)
{
    rc_t rc;
    uint64_t size;
    uint64_t foffset;

    rc = read_u64 (ptr, limit, rev, &foffset);
    if (rc == 0)
    {
	rc = read_u64 (ptr, limit, rev, &size);
	if (rc == 0)
	{
/*             KOutMsg ("%s %s %lu %lu %lu\n", __func__, common->name, size, offset + foffset, arcsize); */
            if (arcsize >= offset + foffset + size )

                rc = KTocCreateFile (toc,
                                 offset + foffset,
                                 size,
                                 common->mtime,
                                 common->access,
                                 (KCreateMode)(kcmInit|kcmParents),
                                 common->name);
            else
/*                 KOutMsg ("ZOMBIE: %s\n", common->name), */

                rc = KTocCreateZombieFile (toc,
                                           offset + foffset,
                                           size,
                                           common->mtime,
                                           common->access,
                                           (KCreateMode)(kcmInit|kcmParents),
                                           common->name);
	}
    }
    return rc;
}

static
rc_t KTocEntryInflateNodeEmptyFile (KToc * toc, const KTocEntryInflateCommon * common, 
                                    const void ** ptr, uint64_t offset, const void * limit, bool rev)
{
    rc_t rc;

    rc = KTocCreateFile (toc,
                         0,
                         0,
                         common->mtime,
                         common->access,
                         (KCreateMode)(kcmInit|kcmParents),
                         common->name);
    return rc;
}

static
rc_t KTocEntryInflateNodeChunked (KToc * toc, const KTocEntryInflateCommon * common, 
				  const void ** ptr, uint64_t offset, const void * limit, bool rev)
{
    rc_t rc;
    uint64_t size;
    uint32_t count;
    KTocChunk * chunks;

    rc = read_u64 (ptr, limit, rev, &size);
    if (rc == 0)
    {
	rc = read_u32 (ptr, limit, rev, &count);
	if (rc == 0)
	{
	    chunks = malloc (sizeof (KTocChunk) * count);
	    if (chunks == NULL)
		rc = RC (rcFS, rcTocEntry, rcParsing, rcMemory, rcExhausted);
	    else
	    {
		uint32_t ix;
		for (ix = 0; (rc == 0) && (ix < count); ++ix)
		{
		    if (rc == 0)
			rc = read_u64 (ptr, limit, rev, &chunks[ix].logical_position);
		    if (rc == 0)
		    {
			rc = read_u64 (ptr, limit, rev, &chunks[ix].source_position);
			chunks[ix].source_position += offset;
		    }
		    if (rc == 0)
			rc = read_u64 (ptr, limit, rev, &chunks[ix].size);
		}
		if (rc == 0)
		    rc = KTocCreateChunkedFile (toc,
						size,
						common->mtime,
						common->access,
						count,
						chunks,
						(KCreateMode)(kcmInit|kcmParents),
						common->name);
		free (chunks);
	    }
	}
    }
    return rc;
}

static
rc_t KTocEntryInflateNodeHardLink (KToc * toc, const KTocEntryInflateCommon * common, 
				   const void ** ptr, uint64_t offset, const void * limit, bool rev)
{
    rc_t rc;
    uint16_t llen;
    char * link;

    rc = read_u16 (ptr, limit, rev, &llen);
    if (rc)
	return rc;

    if (check_limit (*ptr, limit, llen))
	return RC (rcFS, rcTocEntry, rcParsing, rcBuffer, rcTooShort);;

    link = malloc (llen + 1);
    if (link == NULL)
	return RC (rcFS, rcTocEntry, rcParsing, rcMemory, rcExhausted);

    memmove (link, ptr, llen);
    link[llen] = '\0';

    rc = KTocCreateHardLink (toc, 
			     common->mtime,
			     common->access,
			     (KCreateMode)(kcmInit|kcmParents),
			     link,
			     common->name);
    
    free (link);
    return rc;
}

static
rc_t KTocEntryInflateNodeSoftLink (KToc * toc, const KTocEntryInflateCommon * common, 
				  const void ** ptr, uint64_t offset, const void * limit, bool rev)
{
    rc_t rc;
    uint16_t llen;
    char * link;

    rc = read_u16 (ptr, limit, rev, &llen);
    if (rc)
	return rc;

    if (check_limit (*ptr, limit, llen))
	return RC (rcFS, rcTocEntry, rcParsing, rcBuffer, rcTooShort);;

    link = malloc (llen + 1);
    if (link == NULL)
	return RC (rcFS, rcTocEntry, rcParsing, rcMemory, rcExhausted);

    memmove (link, *ptr, llen);
    link[llen] = '\0';

    rc = KTocCreateSoftLink (toc, 
			     common->mtime,
			     common->access,
			     (KCreateMode)(kcmInit|kcmParents),
			     link,
			     common->name);
    
    free (link);
    return rc;
}

static
void CC KTocEntryInflate (PBSTNode * n, void * _data)
{
    KTocEntryInflateData * data;
    const void * ptr;
    const void * limit;
    KTocEntryInflateCommon common;
    rc_t rc;

    data = _data;
    if (data->rc != 0)
	return;
    ptr = n->data.addr;
    limit = (uint8_t*)ptr + n->data.size;
    rc = KTocEntryInflateNodeCommon (&ptr, limit, &common, data->path, data->rev);
    if (rc == 0)
    {
	switch (common.type)
	{
	default:
	case ktocentrytype_unknown:
	case ktocentrytype_notfound:
	    rc = RC (rcFS, rcTocEntry, rcParsing, rcFile, rcCorrupt);
	    break;
	case ktocentrytype_dir:
	    rc = KTocEntryInflateNodeDir (data->toc, &common, &ptr, data->offset,
                                          data->arcsize, limit, data->rev);
	    break;
	case ktocentrytype_file:
            rc = KTocEntryInflateNodeFile (data->toc, &common, &ptr, data->offset,
                                           data->arcsize, limit, data->rev);
	    break;
	case ktocentrytype_emptyfile:
	    rc = KTocEntryInflateNodeEmptyFile (data->toc, &common, &ptr, data->offset,
                                                limit, data->rev);
	    break;
	case ktocentrytype_chunked:
	    rc = KTocEntryInflateNodeChunked (data->toc, &common, &ptr, data->offset, limit, data->rev);
	    break;
	case ktocentrytype_softlink:
	    rc = KTocEntryInflateNodeSoftLink (data->toc, &common, &ptr, data->offset, limit, data->rev);
	    break;
	case ktocentrytype_hardlink:
	    rc = KTocEntryInflateNodeHardLink (data->toc, &common, &ptr, data->offset, limit, data->rev);
	    break;
	}
	free (common.name);
    }    
    data->rc = rc; /* return */
}

rc_t KTocInflatePBSTree ( KToc * self, uint64_t arcsize, const void * treestart, uint32_t maxsize,
			 uint64_t offset, bool rev, const char * path )
{
    rc_t rc;
    PBSTree * pbst;

    rc = PBSTreeMake (&pbst, treestart, maxsize, rev);
    if (rc == 0)
    {
        KTocEntryInflateData data;

        data.toc = self;
        data.path = path;
        data.arcsize = arcsize;
        data.rc = 0;
        data.rev = rev;
        data.offset = offset;
        
        PBSTreeForEach (pbst, false, KTocEntryInflate, &data);

        rc = data.rc;

        PBSTreeWhack (pbst);
    }
    return rc;
}


/* end of file tocentry.c */

