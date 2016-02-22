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
#include <kfs/sra.h>
#include <kfs/toc.h>
#include <kfs/directory.h>
#include <kfs/file.h>
#include <klib/log.h>
#include <klib/debug.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include "toc-priv.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <os-native.h>


/* ======================================================================
 * Defines relevant to the whole compilation unit.
 */

/* !!!!!!!! WHY ISN'T THIS IN A COMMON PRIVATE HEADER FILE? !!!!!!! */
rc_t SraHeaderMake (KSraHeader ** pself, size_t treesize, KSRAFileAlignment alignment);

/* ======================================================================
 * KTocEntryStack
 *
 * A private class used to keep track of directories passed through while adding
 * entries to the TOC.
 *
 * private little class for handling a stack of pointers to TOC entrys
 */
struct KTocEntryStack
{
    unsigned		count;
    unsigned		allocated;
    KTocEntry **	stack;
};

/* ----------------------------------------------------------------------
 * KTocEntryStackNew
 * [OUT] KTocEntryStack ** 	pself		object oriented object reference where
 * construct a Entry Stack at where the pointer to pointer so self points
 *						to put the reference to the new object
 */
rc_t KTocEntryStackNew( KTocEntryStack ** self )
{
#define ARBITRARY_STACK_SIZE	(16)
    if (self == NULL)
        return RC (rcFS, rcToc, rcConstructing, rcParam, rcNull);

    *self = malloc (sizeof (KTocEntryStack));
    (*self)->count = 0;
    (*self)->allocated = ARBITRARY_STACK_SIZE;
    (*self)->stack = malloc ((*self)->allocated * sizeof (KTocEntry*));
    if ((*self)->stack == NULL)
        return RC (rcFS, rcToc, rcAllocating, rcMemory, rcInsufficient);
    return 0;
}

/* ----------------------------------------------------------------------
 * KTocEntryStackDel
 *
 * destruct a Entry Stack
 *
 *
 * [RET] rc_t					0 for success; anything else for a failure
 *						see itf/klib/rc.h for general details
 * [IN] KTocEntryStack *	self		object oriented object reference what to destroy
 */
rc_t KTocEntryStackDel( KTocEntryStack * self )
{

    /* empty stack there so just return ok for now */
    if (self == NULL)
        return 0;

    if (self->stack != NULL)
        free (self->stack);

    free (self);
    return 0;
}

/* ----------------------------------------------------------------------
 * KTocEntryStackDepth
 *
 * [RET] rc_t					0 for success; anything else for a failure
 *						see itf/klib/rc.h for general details
 * [IN]  KTocEntryStack * 	self		object oriented object reference
 * [OUT] int			depth		return the current depth/length of the stack
 *						(int allows compiler flexibilty to use
 *						efficient type)
 */
rc_t KTocEntryStackDepth( KTocEntryStack * self, int * depth )
{
    *depth = self->count;
    return 0;
}

/* ----------------------------------------------------------------------
 * KTocEntryStackPush
 *
 * adds a reference to KTocEntry to the stack
 *
 *
 * [RET] rc_t					0 for success; anything else for a failure
 *						see itf/klib/rc.h for general details
 * [IN] KTocEntryStack *	self		object oriented object reference what to destroy
 * [IN] KTocEntry * 		pathaddr	the entry to add
 */
rc_t KTocEntryStackPush( KTocEntryStack * self, KTocEntry * pathaddr )
{
#define ARBITRARY_INCREMENT_SIZE	(8)

    if (self == NULL)
        return RC (rcFS, rcToc, rcInserting, rcSelf, rcNull);

    if (self->count >= self->allocated)
    {
        KTocEntry **	stack;
        int	new_allocated = self->allocated + ARBITRARY_INCREMENT_SIZE;
        stack = realloc (self->stack, self->allocated * sizeof (KTocEntry*));
        if (stack == NULL)
        {
            return RC (rcFS, rcToc, rcAllocating, rcMemory, rcInsufficient);
        }
        self->allocated = new_allocated;
        self->stack = stack;
    }
    self->stack[self->count++] = pathaddr;
    return 0;
}

/* ----------------------------------------------------------------------
 * KTocEntryStackPop
 *
 * removess a reference to KTocEntry to the stack (not C++STLish but more normal stack operation)
 *
 *
 * [RET] rc_t					0 for success; anything else for a failure
 *						see itf/klib/rc.h for general details
 * [IN]  KTocEntryStack * 	self		object oriented object reference what to destroy
 * [OUT] KTocEntry ** 	pathaddr	where to put the reference to the entry removed
 */
rc_t KTocEntryStackPop( KTocEntryStack * self, KTocEntry ** pathaddr )
{
    if (self == NULL)
        return RC (rcFS, rcToc, rcReading, rcSelf, rcNull);

    if (pathaddr == NULL)
        return RC (rcFS, rcToc, rcReading, rcParam, rcNull);

    if (self->count == 0)
    {
        /* -----
         * this means empty stack and is not considered an error
         * though it should be looked for by the caller
         */
        *pathaddr = NULL;
    }
    else
    {
        *pathaddr = self->stack[--(self->count)];
    }
    return 0;
}


static
int64_t CC KTocEntryIndexCmp2 (const BSTNode * n, const BSTNode * p)
{
    uint64_t nos;
    uint64_t noe;
    uint64_t pos;
    uint64_t poe;

    assert (n != NULL);
    assert (p != NULL);
    {
        const KTocEntryIndex * ne = (const KTocEntryIndex*)n;
        nos = ne->entry->u.contiguous_file.archive_offset;
        noe = nos + ne->entry->u.contiguous_file.file_size;
    }
    {
        const KTocEntryIndex * pe = (const KTocEntryIndex*)p;
        pos = pe->entry->u.contiguous_file.archive_offset;
        poe = pos + pe->entry->u.contiguous_file.file_size;
    }
    if ((nos == pos)&&(noe == poe))
        return 0;
    if (noe <= pos)
        return -1;
    return 1;
}

static
uint64_t add_filler (uint64_t z, KSRAFileAlignment a)
{
    if (a > 1)
    {
        uint64_t m = (uint64_t)a - 1;
        return (z + m) & ~ m;
    }
    return z;
}

static
rc_t KTocCreateEntryIndex (KToc * self, const char * path, uint64_t * file_offset)
{
    rc_t rc;
    size_t len;
    union idx
    {
        void * v;
        KTocEntryIndex * i;
        char * b;
    } idx;
    KTocEntryType type;
/*     const KTocEntry * entry; */
    const char * left;

    TOC_FUNC_ENTRY();

    assert (self != NULL);
    assert (path != NULL);

    rc = 0;

    len = strlen (path);
    idx.v = malloc (sizeof (*idx.i) + len + 1);
    if (idx.v == NULL)
    {
        rc = RC (rcFS, rcToc, rcReindexing, rcMemory, rcExhausted);
        LOGERR( klogErr, rc, "Out of memory creating new path");
    }
    else
    {
        char * tmp;

        type = ktocentrytype_unknown;
        tmp = idx.b + sizeof (*idx.i);
        strcpy (tmp, path);
        StringInitCString (&idx.i->fullpath, tmp);
        rc = KTocResolvePathTocEntry (self, (const KTocEntry**)&idx.i->entry, path, len, &type, &left);
        if (rc != 0)
            LOGERR( klogErr, rc, "Unable to resolve path");
        else
        {
    /* 	    if (left != 0) */
    /* 		rc = RC (rcFS, rcToc, rcAccessing, rcPath, rcNotFound); */
    /* 	    else */
            {
                uint64_t offset;
                uint64_t new_offset;
                switch (type)
                {
                case ktocentrytype_dir:
                case ktocentrytype_softlink:
                case ktocentrytype_hardlink:
                case ktocentrytype_emptyfile:
                    goto no_entry;
                case ktocentrytype_file:
                            offset = add_filler (*file_offset, self->alignment);
                            idx.i->entry->u.contiguous_file.archive_offset = offset;
                            new_offset = offset + idx.i->entry->u.contiguous_file.file_size;
                    break;
                case ktocentrytype_chunked:
                            offset = add_filler (*file_offset, self->alignment);
                    idx.i->entry->u.chunked_file.archive_offset = offset;
                    new_offset = offset + idx.i->entry->u.chunked_file.file_size;
                    break;
                case ktocentrytype_unknown:
                    rc = RC (rcFS, rcToc, rcReindexing, rcTocEntry, rcUnknown);
                    break;
                case ktocentrytype_notfound:
                    rc = RC (rcFS, rcToc, rcReindexing, rcTocEntry, rcNotFound);
                    break;
                default:
                    rc = RC (rcFS, rcToc, rcReindexing, rcTocEntry, rcInvalid);
                    break;
                }
                if (rc != 0)
                    LOGERR( klogErr, rc, "Unable to handle path");
                else
                {
                    rc = BSTreeInsert (&self->offset_index, &idx.i->node, KTocEntryIndexCmp2);
                    if (rc == 0)
                    {
                        *file_offset = new_offset;
                        return 0;
                    }
                }
            }
        }
        no_entry:
        free (idx.v);
    }
    return rc;
}

static
void CC KTocEntryIndexWhack (BSTNode * n, void * data)
{
    free (n);
}

struct ugliness
{
    const KToc * toc;
    uint64_t     offset;
    uint64_t     foffset;
};

static int64_t CC KTocEntryIndexCmpOffset (const void * item /* offset */,
    const BSTNode * n)
{
    struct ugliness * ugly;
    uint64_t nos;
    uint64_t noe;
    uint64_t po;
    assert (item != NULL);
    assert (n != NULL);
    ugly = (struct ugliness*)item;
    {
        const KTocEntryIndex * ne = (const KTocEntryIndex*)n;
        nos = ne->entry->u.contiguous_file.archive_offset;
        noe = add_filler (nos + ne->entry->u.contiguous_file.file_size,
                          ugly->toc->alignment);
    }
    po = ugly->offset;

    TOC_DEBUG (("%s: %lu %lu %lu\n", __func__, po, nos, noe));

    ugly->foffset = 0;
    if (po < nos)
        return -1;
    else if (po < noe)
    {
        ugly->foffset = po - nos;
        return 0;
    }
    else
        return 1;
}

/* -----
 * We use MAX_PATH if it is available but not religiously.  We handle most
 * buffer over-runs with in the module only truncating paths at the border
 * of the module where KDirectory doesn't handle path's longer than MAX_PATH
 */
#ifdef MAX_PATH
#define	KARC_DEFAULT_PATH_ALLOC			(MAX_PATH)
#else
#define	KARC_DEFAULT_PATH_ALLOC			(4096)
#endif

/* ======================================================================
 * KToc method prototypes and defines
 */

/*--------------------------------------------------------------------------
 * KToc
 *  archive table of contents
 *
 *  the "CreateXXX" series of messages take a KCreateMode (see kfs/directory.h).
 *  when mode == kcmOpen, create if needed, overwrite if exists.
 *  when mode == kcmInit, create if needed, clear if exists.
 *  when mode == kcmCreate, fail if exists, create otherwise.
 *  if mode has kcmParents set, fill in missing parents on create.
 */


/* ======================================================================
 * KToc method bodies
 */

/* ----------------------------------------------------------------------
 * KToc
 *  archive table of contents
 *
 *  the "CreateXXX" series of messages take a KCreateMode (see kfs/directory.h).
 *  when mode == kcmOpen, create if needed, overwrite if exists.
 *  when mode == kcmInit, create if needed, clear if exists.
 *  when mode == kcmCreate, fail if exists, create otherwise.
 *  if mode has kcmParents set, fill in missing parents on create.
 */

rc_t KTocInit ( KToc ** self,
                const String * path,
                KArcFSType arctype,
                const void * archive,
                KSRAFileAlignment alignment )
{
    char * str_data;
    rc_t rc;

    /* -----
     * expected failure mode is NULL references for needed initializers
     */
    if (self == NULL)
    {
        return RC (rcFS, rcToc, rcConstructing, rcSelf, rcNull);
    }
    if ((path == NULL)||(archive == NULL))
    {
        return RC (rcFS, rcToc, rcConstructing, rcParam, rcNull);
    }

    /* -----
     * get memory for the TOC and for its path
     */
    *self = malloc (sizeof(KToc) + StringSize(path) + 1);

    /* -----
     * expected failure mode is memory allocation failure
     */
    if (*self == NULL)
    {
        return RC (rcFS, rcToc, rcAllocating, rcMemory, rcInsufficient);
    }

    /* -----
     * point past the KToc structure in the allocated memory for the
     * string path of this TOC
     */
    str_data = (char *)*self + sizeof (KToc);

    /* -----
     * reference the existing KFile
     */
    switch ((*self)->arctype = arctype)
    {
    default:
        free (self);
        return RC (rcFS, rcToc, rcConstructing, rcParam, rcInvalid);
    case tocKFile:
        KFileAddRef ((*self)->archive.v = archive);
        break;
    case tocKDirectory:
        KDirectoryAddRef ((*self)->archive.v = archive);
        break;
    }

    /* need to set back pointer karchive */

    /* -----
     * We added here by default
     */
    atomic32_set (&(*self)->refcount, 1);

    /* -----
     * a tad clunky
     */
    string_copy (str_data, 1 + StringSize(path), path->addr, StringSize(path));
    StringInit (&((*self)->path), str_data, StringSize(path), StringLength(path));
    rc = (KTocAlignmentSet (*self, alignment));
    if (rc == 0)
    /* -----
     * Build the "root directory" structure for the TOC initialized to empty
     */
    {
        KTocEntry * pentry = &(*self)->entry;
        const char *  pchar = strrchr ((*self)->path.addr, '/') + 1;
        size_t	      size = (*self)->path.size - (pchar - (*self)->path.addr);

        if (pchar == NULL)
            pchar = (*self)->path.addr;

        StringInit ( &pentry->name, pchar, size, (uint32_t)size );
        pentry->type = ktocentrytype_dir;
        BSTreeInit(&pentry->u.dir.tree);
        BSTreeInit(&(*self)->offset_index);
        (*self)->header = NULL;
    }
    return rc;
}

/* ----------------------------------------------------------------------
 * AddRef
 *  ignores NULL references
 */
rc_t KTocAddRef ( const KToc *self )
{
    KToc *	mutable_self = (KToc*)self; /* strip const protection for mutable field */

    if (mutable_self != NULL)
        atomic32_inc (&mutable_self->refcount);

    return 0;
}
/* ----------------------------------------------------------------------
 * AddRef
 * Release
 *  ignores NULL references
 */
rc_t KTocRelease ( const KToc *self )
{
    KToc *	mutable_self = (KToc*)self; /* strip const protection for mutable field */
    rc_t	rc = 0;

    if (mutable_self == NULL)
    {
        return RC (rcFS, rcToc, rcReleasing, rcSelf, rcNull);
    }

    if (atomic32_dec_and_test (&mutable_self->refcount))
    {
        switch (self->arctype)
        {
        case tocUnknown:
        case tocKVirtual:
            free((void*)self->archive.v);
            break;

        case tocKFile:
            KFileRelease (self->archive.f);
            break;
        case tocKDirectory:
            KDirectoryRelease (self->archive.d);
            break;
        }
        BSTreeWhack (&mutable_self->entry.u.dir.tree, KTocEntryWhack, &rc);
        BSTreeWhack (&mutable_self->offset_index, KTocEntryIndexWhack, &rc);
        free (mutable_self);
    }
/*     else */
/*     { */
/*     } */
    return rc;
}


/* ----------------------------------------------------------------------
 * KTocCreateSubdirs
 *
 * This is a constructor/parse helper method (the p starting the name is meant to 
 * imply a c++/Java private situation)
 *
 * [RET] rc_t					0 for success; anything else for a failure
 *						see itf/klib/rc.h for general details
 * [INOUT]KToc * 		self		TOC self reference: object oriented in C
 * [IN]   const char * 		_path		start of directory path
 * [IN]   const char * 		end		end of directory path
 * [IN]   KCreateMode  		mode		whether or not to document the existing?
 * [INOUT]KTocEntryStack * 	pathstack	created outside this function but this is
 *						where it is primarily filled in.  This is
 *						created for use in KTocResolveHardLink
 */
static
rc_t	KTocCreateSubdirs (KToc *    self,	   /* TOC self reference: object oriented in C */
			   const char * _path,   /* start of directory path */
			   const char * end, 	   /* end of directory path */
			   KTime_t mtime,
			   uint32_t access,
			   KCreateMode  mode,    /* whether or not to document the existing? */
			   KTocEntryStack * pathstack)
{
    const
	char * 	path = _path;		/* pointer to head of remaining path to parse: starts at the beginning of course */
    const
	char *  slash;			/* pointer to the next '/' (or NUL) past the next facet of the path */
    BSTree *	tree = &self->entry.u.dir.tree;	/* current directory tree we are in */
    rc_t	rc = 0;		/* general purpose rc_t for calls and our own return */
    BSTNode *   exists = NULL;		/* is there already a entry with our name? this will be it */
    bool	success = true;		/*  */

    /* -----
     * special case of absolute position being turned into relative
     * skip past initial "/" or "//" in a path
     */
    while (*path == '/')
    {
        PLOGMSG (klogWarn, (klogWarn,
                            "skipping initial '/' in $(path)",
                            PLOG_S(path),
                            path));
        ++path;
    }

    /* -----
     * Now continue on with each facet of the path one at a time
     */
    for (;path < end; path = slash+1)
    {
        KTocEntry * newentry;			/* new entry for the new name entry: might be a duplicate */
        bool	      do_insert = false;	/* do we insert a new entry? short cut for later */

        /* -----
         * find the next (first) '/' in the remaining path
         */
        slash = strchr (path, '/');

        /* -----
         * we've got the last directory in the path if there are no
         * more '/' characters or if the next '/' is the last character in the
         * path (we don't do white space checks!)
         */
        if (slash == NULL)
        {
            slash = end;
        }

        /* -----
         * first we'll compare our new path against the special '.' and '..'
         * special directory pointers
         */
        if ((path[0] == '.')&&(path+1 == slash))	/* . */
        {
            /* -----
             * just ignore "here" directory references
             */
            continue;
        }
        else if ((path[0] == '.')&&(path[1] == '.')&&(path+2 == slash))	/* .. */
        {
            /* -----
             * for up to parent directory references we try to change to that directory
             */
            int depth;

            rc = KTocEntryStackDepth (pathstack, &depth);
            if (depth <= 0)
            {
                /* FAILURE */
                rc = RC (rcFS, rcToc, rcConstructing, rcLink, rcViolated);
                success = false;
                break;
            }
            else
            {
                rc = KTocEntryStackPop (pathstack, &newentry);
            }
        }
        else
        {
            /* -----
             * Create a new directory entry
             *
             * access isn't really implemented yet so make a directory
             * default to read + execute for anyone
             *
             * we might only be using this directory entry for comparisons <shrug>
             */
            rc = KTocEntryNewDirectory (&newentry, path, slash - path, mtime, access);

            /* -----
             * pass along failures - no need to change any parts at this point
             * but that time may come
             */
            if (rc != 0)
            {
                LOGMSG (klogErr, "failed to construct a directory entry: memory allocation");
                return rc;
            }

            exists = BSTreeFind (tree, newentry, KTocEntryCmpVoid);

            /* -----
             * the expected is to insert the entry if it wasn't there but will check
             * special circumstances
             *
             * so plan to insert or not based on whether it is there and expect
             * all to be well.
             *
             * never insert if it already exists
             */
            do_insert = (bool)((exists == NULL) ?  true : false);
            rc = 0;

            /* -----
             * Open/init/create with no make parents:
             */
            if ((exists == NULL)&& !(mode & kcmParents))
            {
                /* parent did not exist so we will fail */
                rc = RC (rcFS, rcToc, rcCreating, rcSelf, rcInconsistent);
                do_insert = false;
                success = false;
            }

            /* -----
             * if rc is set, we had a problem and will fail
             */
            if (rc != 0)
            {
                LOGERR	(klogErr, rc,
                         "directory parse/creation failed");
                (void)KTocEntryDelete(newentry);
                break;
            }

            if (do_insert)
            {
                rc = BSTreeInsert (tree, &newentry->node, KTocEntryCmp2);
                if (rc)
                {
                    LOGMSG (klogErr, "directory parse/creation failed 2");
                    (void)KTocEntryDelete(newentry);
                    break;
                }
                TOC_DEBUG (("%s: inserted new directory into TOC %S\n", __func__, newentry->name));
            }
            else
            {
                (void)KTocEntryDelete(newentry);
                newentry = (KTocEntry*)exists;
            }

            /* -----
             * move into the subdirectory for the next directory in the path
             */
            if ((rc = KTocEntryStackPush (pathstack, newentry)) != 0)
            {
                success = false;
                break;
            }

            tree = &newentry->u.dir.tree;
        }
    }
    return success ? 0 : rc;
}


/* ----------------------------------------------------------------------
 * KTocCreate
 *
 *
 * [RET] rc_t					0 for success; anything else for a failure
 *						see itf/klib/rc.h for general details
 * [IN] KToc *		self		TOC self reference: object oriented in C
 * [IN] KCreateMode 		mode		KDirectory defines that define what to so with
 *						"files" that already existed or did not including
 *						missing subdirectories in the path
 * [IN] const char *		ppath		the path name for the new entry relative to the
 *						base of the TOC
 * [IN] const KTocEntryParam*	pparams		specific parameters for this type of entry - the
 *						type of entry is included in this structure
 */
static rc_t KTocCreate (KToc *self,
			KCreateMode mode,
			const char *ppath,
			const KTocEntryParam * pparams)
{
    rc_t		rc;
    char *		pend;
    size_t		name_size;
    BSTree *		ptree;
    KTocEntry * 	pnewentry = NULL;	/* init to kill a warning */
    KTocEntry * 	pexistingentry;
    KTocEntry * 	pwd = NULL;
    KTocEntryStack *	pdirstack;

    assert (self != NULL);
    assert (ppath != NULL);
    assert (pparams != NULL);

    ptree = &self->entry.u.dir.tree;

    if ((rc = KTocEntryStackNew (&pdirstack)) != 0)
    {
        return rc;
    }
    /* -----
     * look for a '/' that isn't the last character in the path
     */
    pend = string_rchr (ppath, strlen(ppath)-1, '/');
    if (pend != NULL)
    {

        rc = KTocCreateSubdirs (self, ppath, pend, pparams->mtime,
                                pparams->access, mode, pdirstack);

        rc = KTocEntryStackPop (pdirstack, &pwd);
        if (pwd)
        {
            ptree = &(pwd->u.dir.tree);
        }

        ppath = pend+1;
    }

    name_size = strlen(ppath);
    switch (pparams->type)
    {
    default:
        break;
    case ktocentrytype_dir:
        rc = KTocEntryNewDirectory (&pnewentry,
                                    ppath,
                                    name_size,
                                    pparams->mtime,
                                    pparams->access);
        break;
    case ktocentrytype_file:
        rc = KTocEntryNewFile (&pnewentry,
                               ppath,
                               name_size,
                               pparams->mtime,
                               pparams->access,
                               pparams->u.file.source_position,
                               pparams->u.file.size);
        break;
    case ktocentrytype_zombiefile:
        rc = KTocEntryNewZombieFile (&pnewentry,
                                     ppath,
                                     name_size,
                                     pparams->mtime,
                                     pparams->access,
                                     pparams->u.file.source_position,
                                     pparams->u.file.size);
        break;
    case ktocentrytype_chunked:
        rc = KTocEntryNewChunked (&pnewentry,
                                  ppath,
                                  name_size,
                                  pparams->mtime,
                                  pparams->access,
                                  pparams->u.chunked.size,
                                  pparams->u.chunked.chunks,
                                  pparams->u.chunked.num_chunks);
        break;
    case ktocentrytype_softlink:
        rc = KTocEntryNewSoft (&pnewentry,
                               ppath,
                               name_size,
                               pparams->mtime,
                               pparams->access,
                               pparams->u.softlink.targ,
                               strlen (pparams->u.softlink.targ));
        break;
    case ktocentrytype_hardlink:
        rc = KTocEntryNewHard (&pnewentry,
                               ppath,
                               name_size,
                               pparams->mtime,
                               pparams->access,
                               pparams->u.hardlink.ref);
        break;
    }
/*     if (rc != 0) */
/* 	; */

    pexistingentry = (KTocEntry*)BSTreeFind (ptree, pnewentry, KTocEntryCmpVoid);

    if (pexistingentry != NULL)
    {
        switch (mode & kcmValueMask)
        {
        default:
            LOGMSG (klogInt, "invalis create mode flag for Toc Entry");
        case kcmOpen:	/* use existing entry if it exists */
            KTocEntryDelete (pnewentry);
            break;

        case kcmInit:
            BSTreeUnlink (ptree, &pexistingentry->node); /*?*/
            KTocEntryDelete (pexistingentry);
            goto insert;
            break;

        case kcmCreate:
            KTocEntryDelete (pnewentry);
            rc = RC (rcFS, rcToc, rcInserting, rcDirEntry, rcDuplicate);
            break;
        }
    }
    else
    {
    insert:
        rc = BSTreeInsert (ptree, &pnewentry->node, KTocEntryCmp2);
        TOC_DEBUG (("%s: inserted new %s into TOC %s\n", __func__,
                    KTocEntryTypeGetString(pparams->type),
                    pnewentry->name.addr));
    }
#if 0
    if (pparams->type == ktocentrytype_hardlink)
    {

        /* -----
         * Now the ugly specific part of a hard link - resolve the link
         */
        rc = KTocResolveHardLink (self, pparams->u.hardlink.targ, pdirstack, pwd, &(pnewentry->u.hard_link.ref));
        if (rc  != 0)
        {
            /* kill off node */
            BSTreeUnlink (ptree, &pnewentry->node);
            rc = RC (rcFS, rcToc, rcAliasing, rcArcHardLink, rcNotFound);
        }
    }
#endif
    KTocEntryStackDel (pdirstack);
    return rc;
}


/* ----------------------------------------------------------------------
 * Constructors/factories
 */
static
rc_t createPath (char ** newpath, const char * path, va_list args)
{
    rc_t rc;
    char * p;
    char * pp;
    size_t l;
    int i;

    rc = 0;
    p = NULL;
    l = 4096;
    *newpath = p;

    /* not trusting C99 version of vsnprintf is in place rather than SUSv2 */
    for (;;)
    {
        pp = realloc (p,l);
        if (pp == NULL)
        {
            rc = RC (rcFS, rcToc, rcConstructing, rcMemory, rcExhausted);
            return rc;
        }
        if (args == NULL)
        {
            i = (int)strlen ( path );
            if ( i < (int)l )
            strcpy ( pp, path );
        }
        else
            i = vsnprintf (pp, l, path, args);
        if (i < 0)
        {
            rc = RC (rcFS, rcToc, rcConstructing, rcFormat, rcInvalid);
            free (pp);
            return rc;
        }
        p = pp;
        if ( i < (int)l )
            break;
        l = i + 1;
    } 

    *newpath = pp;

    return rc;
}



/* ---------
 * CreateDir
 *  create a sub-directory
 *
 *  "mode" [ IN ] - a creation mode (see explanation in kfs/directory.h).
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target directory
 */
rc_t KTocCreateDir ( KToc *self,KTime_t mtime, uint32_t access,
		    KCreateMode mode, const char *path, ... )
{
    va_list 	args;
    rc_t	rc;

    va_start (args, path);
    rc = KTocVCreateDir (self, mtime, access, mode, path, args);
    va_end (args);
    return rc;
}

rc_t KTocVCreateDir ( KToc *self,KTime_t mtime, uint32_t access,
		     KCreateMode mode, const char *path, va_list args )
{
    KTocEntryParam	params;
    char * npath;
    rc_t rc;

    TOC_DEBUG (("%s: Name: %s Mode: %s\n", __func__,
	     path, get_mode_string(mode)));

    rc = createPath (&npath, path, args);
    if (rc)
        return rc;

    params.type = ktocentrytype_dir;
    params.mtime = mtime;
    params.access = access;

    rc = KTocCreate (self, mode, npath, &params);
    free (npath);
    return rc;
}

/* ----------
 * CreateFile
 *  create an entry for a contiguous file
 *
 *  "source_position" [ IN ] and "size" [ IN ] - description of the contiguous
 *  region of the archive file occupied by the file
 *
 *  "mode" [ IN ] - a creation mode (see explanation above).
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target file
 */
rc_t KTocCreateFile ( KToc *self,
		     uint64_t source_position, uint64_t size,
		    KTime_t mtime, uint32_t access, 
		     KCreateMode mode, const char *path, ... )
{
    va_list 	args;
    rc_t	rc;

    va_start (args, path);
    rc = KTocVCreateFile (self, source_position, size,
			  mtime, access, mode, path, args);
    va_end (args);
    return rc;
}

rc_t KTocVCreateFile ( KToc *self,
                       uint64_t source_position, uint64_t size,
                       KTime_t mtime, uint32_t access, 
                       KCreateMode mode, const char *path, va_list args )
{
    KTocEntryParam	params;
    char * npath;
    rc_t rc;

    TOC_DEBUG (("%s: Name: %s Mode: %sn Time: %lu\n", __func__,
                path, get_mode_string(mode), mtime));


    rc = createPath (&npath, path, args);
    if (rc)
        return rc;

    params.type = ktocentrytype_file;
    params.mtime = mtime;
    params.access = access;
    params.u.file.size = size;
    params.u.file.source_position = source_position;

    rc = KTocCreate (self, mode, npath, &params);
    free (npath);
    return rc;
}

rc_t KTocCreateZombieFile ( KToc *self,
                            uint64_t source_position, uint64_t size,
                            KTime_t mtime, uint32_t access, 
                            KCreateMode mode, const char *path, ... )
{
    va_list 	args;
    rc_t	rc;

    va_start (args, path);
    rc = KTocVCreateZombieFile (self, source_position, size,
                                mtime, access, mode, path, args);
    va_end (args);
    return rc;
}

rc_t KTocVCreateZombieFile ( KToc *self,
                             uint64_t source_position, uint64_t size,
                             KTime_t mtime, uint32_t access, 
                             KCreateMode mode, const char *path, va_list args )
{
    KTocEntryParam	params;
    char * npath;
    rc_t rc;

    TOC_DEBUG (("%s: Name: %s Mode: %sn Time: %lu\n", __func__,
                path, get_mode_string(mode), mtime));


    rc = createPath (&npath, path, args);
    if (rc)
        return rc;

    params.type = ktocentrytype_zombiefile;
    params.mtime = mtime;
    params.access = access;
    params.u.file.size = size;
    params.u.file.source_position = source_position;

    rc = KTocCreate (self, mode, npath, &params);
    free (npath);
    return rc;
}

/* ---------
 * CreateChunkedFile
 */
rc_t KTocCreateChunkedFile ( KToc *self, uint64_t size,KTime_t mtime,
                             uint32_t access, uint32_t num_chunks,
                             const KTocChunk chunks [], KCreateMode mode,
                             const char *path, ... )
{
    va_list 	args;
    rc_t	rc;

    va_start (args, path);
    rc = KTocVCreateChunkedFile (self, size, mtime, access, num_chunks,
				 chunks, mode, path, args);
    va_end (args);
    return rc;
}

rc_t KTocVCreateChunkedFile ( KToc *self, uint64_t size,KTime_t mtime,
                              uint32_t access, uint32_t num_chunks,
                              const KTocChunk chunks [], KCreateMode mode,
                              const char *path, va_list args )
{
    KTocEntryParam	params;
    unsigned int	ix;
    rc_t		rc;
    char * 		npath;

    TOC_DEBUG (("KToc Create Chunked File:: Name: %s Size: %ju Chunk count: %u Mode %s\n",
                path, size, num_chunks, get_mode_string(mode)));

    for (ix = 0; ix < num_chunks; ++ix)
    {
        TOC_DEBUG (( "KToc Create Chunk %u: 0x%jx 0x%jx %ju\n",
                     ix,
                     chunks[ix].logical_position,
                     chunks[ix].source_position,
                     chunks[ix].size));
    }

    rc = createPath (&npath, path, args);
    if (rc)
        return rc;

    params.type = ktocentrytype_chunked;
    params.u.chunked.size = size;
    params.u.chunked.chunks = chunks;
    params.u.chunked.num_chunks = num_chunks;

    rc = KTocCreate (self, mode, npath, &params);
    free (npath);
    return rc;
}

/* ----------------------------------------------------------------------
 * CreateHardLink
 */
rc_t KTocCreateHardLink ( KToc *self,KTime_t mtime, uint32_t access,
                          KCreateMode mode, const char *targ,
                          const char *link, ... )
{
    va_list 	args;
    rc_t	rc;

    va_start (args, link);
    rc = KTocVCreateHardLink(self, mtime, access, mode, targ,
			     link, args);
    va_end (args);
    return rc;
}

rc_t KTocVCreateHardLink ( KToc *self,KTime_t mtime, uint32_t access,
			  KCreateMode mode, const char *targ,
			  const char *link_fmt, va_list args )
{
#if 0
    KTocEntryParam	params;
    char * npath;
    rc_t rc;



    return rc;
#else
    KTocEntryParam	params;
    char * npath;
    const char * excess_path;
    const KTocEntry * targ_entry;
    KTocEntryType targ_type;
    rc_t rc;

    TOC_DEBUG (("Hard link_fmt:: Name: %s Link: %s Mode: %s\n",
                link_fmt, targ, get_mode_string(mode)));

    /* figure out where we are supposed to point */
    rc = KTocResolvePathTocEntry (self, &targ_entry, targ, string_size (targ),
                                  &targ_type, &excess_path);
    if (rc == 0)
    {
        /* hard links to directories are kept as hard links
         * while other types become true unix style links */
        char link[4096];
        int size;

        switch (targ_type)
        {
        case ktocentrytype_dir:
            rc = createPath (&npath, link_fmt, args);
            if (rc)
                return rc;
            params.type = ktocentrytype_hardlink;
            params.mtime = mtime;
            params.access = access;
            params.u.hardlink.ref = targ_entry;
            rc = KTocCreate (self, mode, npath, &params);
            free (npath);
            return rc;

            /* resolve by one indirection */
        case ktocentrytype_hardlink:
            rc = createPath (&npath, link_fmt, args);
            if (rc)
                return rc;
            params.type = ktocentrytype_hardlink;
            params.mtime = mtime;
            params.access = access;
            params.u.hardlink.ref = targ_entry->u.hard_link.ref;
            rc = KTocCreate (self, mode, npath, &params);
            free (npath);
            return rc;

        case ktocentrytype_file:
        case ktocentrytype_zombiefile:
            size = (args == NULL) ?
                snprintf  ( link, sizeof link, "%s", link_fmt ) :
                vsnprintf ( link, sizeof link, link_fmt, args );
            if (size < 0 || size >= ( int ) sizeof link)
                rc = RC (rcFS, rcToc, rcConstructing, rcLink, rcExcessive);
            return rc ? rc : KTocCreateFile (self, targ_entry->u.contiguous_file.archive_offset,
                                   targ_entry->u.contiguous_file.file_size,
                                   mtime, access, mode, link);

        case ktocentrytype_emptyfile:
            size = (args == NULL) ?
                snprintf  ( link, sizeof link, "%s", link_fmt ) :
                vsnprintf ( link, sizeof link, link_fmt, args );
            if (size < 0 || size >= ( int ) sizeof link)
                rc = RC (rcFS, rcToc, rcConstructing, rcLink, rcExcessive);
            return rc ? rc : KTocCreateFile (self, 0, 0, mtime, access, mode, link);

        case ktocentrytype_chunked:
            size = (args == NULL) ?
                snprintf  ( link, sizeof link, "%s", link_fmt ) :
                vsnprintf ( link, sizeof link, link_fmt, args );
            if (size < 0 || size >= ( int ) sizeof link)
                rc = RC (rcFS, rcToc, rcConstructing, rcLink, rcExcessive);
            return rc ? rc : KTocCreateChunkedFile (self, targ_entry->u.chunked_file.file_size, mtime,
                                          access, targ_entry->u.chunked_file.num_chunks,
                                          targ_entry->u.chunked_file.chunks, mode, link);

        case ktocentrytype_softlink:
            size = (args == NULL) ?
                snprintf  ( link, sizeof link, "%s", link_fmt ) :
                vsnprintf ( link, sizeof link, link_fmt, args );
            if (size < 0 || size >= ( int ) sizeof link)
                rc = RC (rcFS, rcToc, rcConstructing, rcLink, rcExcessive);
            return rc ? rc : KTocCreateSoftLink (self, mtime, access, mode, link,
                                       targ_entry->u.symbolic_link.link_path.addr);

        default:
        case ktocentrytype_notfound:
            /* silent ignore? */
            break;
        }
    }
    return rc;
#endif
}

/* ----------------------------------------------------------------------
 * CreateSoftLink
 */
rc_t KTocCreateSoftLink ( KToc *self,KTime_t mtime, uint32_t access,
			 KCreateMode mode, const char *targ,
			 const char *alias, ... )
{
    va_list 	args;
    rc_t	rc;

    va_start (args, alias);
    rc = KTocVCreateSoftLink (self, mtime, access, mode,
			      targ, alias, args);
    va_end (args);
    return rc;
}

rc_t KTocVCreateSoftLink ( KToc *self,KTime_t mtime, uint32_t access,
			  KCreateMode mode, const char *targ,
			  const char *alias, va_list args )
{
    KTocEntryParam	params;
    char * npath;
    rc_t rc;

    TOC_DEBUG (("Soft link:: Name: %s Link: %s Mode: %s\n",
                alias, targ, get_mode_string(mode)));

    rc = createPath (&npath, alias, args);
    if (rc)
        return rc;

    params.type = ktocentrytype_softlink;
    params.mtime = mtime;
    params.access = access;
    params.u.softlink.targ = targ;

    rc = KTocCreate (self, mode, npath, &params);
    free (npath);
    return rc;
}

/* ----------------------------------------------------------------------
 *
 */
const KTocEntry * KTocGetRoot	(const KToc * self)
{
    if (self == NULL)
        return NULL;

    return &self->entry;
}

rc_t KTocGetPath ( const KToc * self, const String ** ppath )
{
    assert (self != NULL);
    *ppath = &self->path;
    return 0;
}

KSRAFileAlignment KTocAlignmentGet( KToc * self )
{
    return self->alignment;
}

rc_t KTocAlignmentSet( KToc * self,KSRAFileAlignment align )
{
    if ((align != 0) && (align == ((align)&~(align-1))))
    {
        self->alignment = align;
        return 0;
    }
    return RC (rcFS, rcToc, rcConverting, rcParam, rcInvalid);
}


/* const char * KTocGetRootPath (const KToc * self, size_t * len) */
/* { */
/*     if (self == NULL) */
/* 	return NULL; */
/*     *len = self->path.len; */
/*     return &self->path.addr;; */
/* } */
/*****
 ***** CURRENTLY DOES NOT SUPPORT any form of crossing of
 ***** KDirectory type paths
 *****/
rc_t KTocResolvePathTocEntry ( const KToc *self,
                               const KTocEntry ** pentry,
                               const char *path,	/* must be entirely in the TOC */
                               size_t path_len,
                               KTocEntryType * ptype,
                               const char ** unusedpath )
{
    const char *  	slash;		/* points to the / following the current facet */
    const char *  	next_facet;	/* points to the start of the current facet */
    const char *	end;		/* points to the character after the path */
    const KTocEntry * dentry;		/* the current entry we are at during the walk through */
    KTocEntry * 	tentry;		/* points to a Temporary ENTRY built for comparisons */
    union
    {
        const BSTNode     * b;		/* access to the BSTree Node starting the KToc entry */
        const KTocEntry * k;		/* access to the whole of the entry */
    }		  	fentry;		/* Found ENTRY: two ways to access to skip casts */
    rc_t	  	rc;		/* temporary storage for the return from many calls */
    size_t	  	facet_size;	/* length of a single facet (part of a path dir or file) */
    size_t		path_size;	/* temporary size (shrinks as we go through the path */	
    /* int		  	loopcount;	counter for loop limiting hardlink resolution */
    KTocEntryType 	type = ktocentrytype_unknown;	/* type of a entry found for a facet (init to kill a warning */
    bool		is_last_facet = false;
    bool		is_facet_dir = false; /* if the path ends in / we know the last facet is a dir */
    int                 outer_loopcount;
    /* -----
     * point to one character past the path - usually it will be a NUL but we
     * are not making that a requirement here
     */
    next_facet = path;
    path_size = path_len;
    end = next_facet + path_size;
    dentry = &self->entry;

    /* -----
     * look first for here references:
     * either no path (not NULL but "") or just "."
     */
    if ((path_len == 0) || ((path_len == 1) && (path[0] == '.')))
    {
        *pentry = dentry;
        *ptype = ktocentrytype_dir;
        *unusedpath = end;
        return 0;
    }


    /* -----
     * now start wending our way down through subdirectories
     */
    for (outer_loopcount = 0; next_facet < end; ++ outer_loopcount)
    {
        TOC_DEBUG (("%s: stepping through subs (%d) (%s)\n", __func__, outer_loopcount, next_facet));

        /* -----
         * look for the end of the next facet in the path
         */
        slash = strchr (next_facet, '/');

        /* -----
         * if there was no '/' found then point to the end as that is the end of the facet
         * but also mark that we know this is the last one (be it a directory, link or file)
         */
        if (slash == NULL)			
        {
            slash = end;
            is_last_facet = true;
        }
        /* -----
         * handle the special case of a path ending in / which also makes the last facet
         * known to be a directory
         */
        else if (slash + 1 == end)
        {
            slash = end;
            is_last_facet = true;
            is_facet_dir = true;
        }
        facet_size = slash - next_facet;	/* how many characters in this facet */

        /* -----
         * build a temporary entry for comparisons
         */
        rc = KTocEntryNewDirectory (&tentry, next_facet, facet_size, 0, 0555);
        if (rc != 0)
        {
            *pentry = NULL;		/* if we couldn't make then fail */
            return rc;
        }
        fentry.b = BSTreeFind (&dentry->u.dir.tree, tentry, KTocEntryCmpVoid);
        KTocEntryDelete(tentry);	/* clean up the temporary entry */

        if (fentry.b == NULL)
        {
            /* -----
             * we failed to find this facet.
             */
            *pentry = NULL;		/* if we couldn't make it fail */
            *unusedpath = next_facet;	/* the name we couldn't find */
            *ptype = ktocentrytype_notfound;

            TOC_DEBUG (("%s: couldn't find (%s)\n", __func__, next_facet));

            return SILENT_RC (rcFS, rcArc, rcResolving, rcParam, rcNotFound);
        }

        /* loopcount = 0; */

        /* check the type to see if we're okay with it */
        if ((rc = KTocEntryGetType (fentry.k, &type)) != 0)
        {
            if (type == ktocentrytype_notfound)
            {
                *ptype = type;
            }
            else
            {
                *ptype = ktocentrytype_unknown;
            }
            *pentry = NULL;		/* if we couldn't then fail */
            *unusedpath = next_facet;	/* the name we couldn't find */
            return rc;
        }
        *ptype = type;
        switch (type)
        {
            /* -----
             * resolve a hardlink immediately 
             * re-resolve until we've gone too many hops or the
             * resolution is to something that isn't another hardlink
             *
             * re-enter this switch with the type of the resolved entry
             */
        case ktocentrytype_hardlink:
            if (is_last_facet)
            {
                *pentry = fentry.k->u.hard_link.ref;
                *unusedpath = end;
                /* successful arrival at the end at a directory */
                return 0;
            }
            else
            {
                /* -----
                 * this is the only path that continues through the loop
                 */
                next_facet = slash + 1;	 /* point past the slash */
                dentry = fentry.k->u.hard_link.ref; /* make the found entry our current entry */
            }
            continue;	/* back to for(;;) */

        case ktocentrytype_dir:
            if (is_last_facet)
            {
                *pentry = fentry.k;
                *unusedpath = end;
                /* successful arrival at the end at a directory */
                return 0;
            }
            else
            {
                /* -----
                 * this is the only path that continues through the loop
                 */
                next_facet = slash + 1;	/* point past the slash */
                dentry = fentry.k;	/* make the found entry our current entry */
            }
            continue;	/* back to for(;;) */

        default:
            *pentry = fentry.k;
            *ptype = ktocentrytype_unknown;
            *unusedpath = next_facet;
            /* failure return */
            return RC (rcFS, rcToc, rcResolving, rcParam, rcInvalid);

        case ktocentrytype_emptyfile:
        case ktocentrytype_file:
        case ktocentrytype_chunked:
        case ktocentrytype_zombiefile:
            *pentry = fentry.k;

            /* -----
             * success if we are on the last facet that wasn't supposed to be a directory
             * failure if we are not
             */
            if (is_last_facet && (! is_facet_dir))
            {
                *unusedpath = end;
                return 0;
            }
            else
            {
                *unusedpath = next_facet;
                return RC (rcFS, rcToc, rcResolving, rcParam, rcInvalid);
            }

        case ktocentrytype_softlink:
            /* -----
             * We got a softlink entry in the path.
             */
            *pentry = fentry.k;
            *unusedpath = slash;
            return 0;	/* This is a bit clunky but is a valid incomplete return */
        }
    }
    /* should never get here */
    return RC (rcFS, rcToc, rcResolving, rcParam, rcUnexpected);
}

/* SRA tocfile only not tar or other archives only */
rc_t KTocResolvePathFromOffset ( const KToc *self,
                                 const char ** path,
                                 uint64_t * ppos,
                                 uint64_t _offset )
{
    rc_t rc;
    union u
    {
        const BSTNode * n;
        const KTocEntryIndex * i;
    } u;
    uint64_t offset;
    struct ugliness thisIsReallyUgly;

    rc = 0;
    offset = _offset - SraHeaderGetFileOffset (self->header);

    TOC_DEBUG(("KTocResolvePathFromOffset _offset %lu offset %lu\n",
               _offset, offset));

    *path = NULL;
    *ppos = 0;
    thisIsReallyUgly.toc = self;
    thisIsReallyUgly.offset = offset;

    u.n =  BSTreeFind (&self->offset_index, &thisIsReallyUgly, KTocEntryIndexCmpOffset);
    if (u.n != NULL)
    {
        *path = u.i->fullpath.addr;
        *ppos = thisIsReallyUgly.foffset;
    }
    return rc;
}

const void * KTocGetArchive( const KToc * self )
{
    if (self == NULL)
        return NULL;
    return self->archive.v;
}
				



typedef struct KDBHdr KDBHdr;

typedef
struct PersistFilesIndexData
{
    uint64_t offset;
    const KToc * toc;
    rc_t rc;
} PersistFilesIndexData;

LIB_EXPORT void CC PersistFilesIndex ( void * vitem, void * vdata )
{
    PersistFilesIndexData * data;
    const char * path;

    TOC_FUNC_ENTRY();

    data = vdata;
    if (data->rc != 0)
    {
        TOC_DEBUG(("PersistFilesIndex leave early %R\n", data->rc));
        return;
    }
    path = (char*)vitem;

    TOC_DEBUG(("PersistFilesIndex %s\n", path));

    data->rc = KTocCreateEntryIndex ((KToc*)data->toc, path, &data->offset);
}

rc_t KTocPersist ( const KToc * self,
                   void ** buffer, size_t * buffer_size,
                   uint64_t * virtual_file_size,
                   const Vector * files )
{
    rc_t rc;
    size_t treesize;
    KSraHeader * header;
    uint8_t * bbuffer;
    uint64_t filesize;

    TOC_FUNC_ENTRY();

    rc = 0;
    treesize = 0;
    bbuffer = NULL;
    *buffer = NULL;
    *buffer_size = 0;
    *virtual_file_size = 0;

    rc = KTocEntryPersistNodeDir (NULL, &self->entry, &treesize, NULL, NULL);
    if (rc != 0)
    {
        LOGMSG (klogErr, "Failure to Persist Toc Root Entry");
    }
    else
    {
        rc = SraHeaderMake (&header, treesize, self->alignment);
        if (header == NULL)
        {
            rc = RC (rcFS, rcToc, rcPersisting, rcMemory, rcExhausted);
            LOGERR (klogErr, rc, "Failure to allocate buffer for header");
        }
        else
        {
                TOC_DEBUG (("KTocPersist: treesize %ju\n", treesize));

            bbuffer = realloc ( header, (size_t)SraHeaderGetFileOffset( header ) );
            if (bbuffer == NULL)
            {
                free (header);
                rc = RC (rcFS, rcToc, rcPersisting, rcMemory, rcExhausted);
                LOGERR (klogErr, rc, "Failure to allocate buffer for persisted header");
            }
            else
            {
                PersistFilesIndexData data;

                header = (KSraHeader*)bbuffer;
                data.offset = 0;
                data.toc = self;
                data.rc = 0;

                VectorForEach (files, false, PersistFilesIndex, &data);
                filesize = SraHeaderGetFileOffset(header) + data.offset;
                rc = data.rc;
                if (rc == 0)
                {
                            KTocEntryPersistWriteFuncData wdata;
                            wdata.buffptr = bbuffer + SraHeaderSize(NULL);
                            wdata.limit = bbuffer + SraHeaderGetFileOffset(header);
                            rc = KTocEntryPersistNodeDir (NULL, &self->entry, &treesize,
                                                          KTocEntryPersistWriteFunc, 
                                                          &wdata);
                }
            }
        }
    }
    if (rc == 0)
    {
        ((KToc*)self)->header = (KSraHeader *)bbuffer;
        *buffer = bbuffer;
        *buffer_size = treesize + SraHeaderSize(NULL);
        *virtual_file_size = filesize;
        return 0;
    }
    else if (bbuffer != NULL)
    {
        TOC_DEBUG (("Free called in KTocPersist\n"));
        free (bbuffer);
    }
    return rc;
}


/* end of file toc.c */
