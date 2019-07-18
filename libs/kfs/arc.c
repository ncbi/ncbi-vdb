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
 */

#include <kfs/extern.h>
#include "karc-priv.h"
#include "toc-priv.h"

#include <kfs/arc.h>

#include <klib/debug.h>
#include <klib/log.h>
#include <klib/status.h>
#include <klib/out.h>
#include <klib/rc.h>
#include <klib/container.h>
#include <klib/vector.h>
#include <klib/pbstree.h>
#include <klib/text.h>
#include <klib/sort.h>
#include <klib/printf.h>
#include <klib/klib-priv.h> /* ReportRecordZombieFile */

#include <kfs/directory.h>
#include <kfs/file.h>
#include <kfs/toc.h>
#include <kfs/sra.h>
#include <kfs/cacheteefile.h>

#include <kns/http.h>

#include <sysalloc.h>

#include <atomic32.h>
#include <os-native.h>

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <va_copy.h>

/* SHOULD NOT INCLUDE THESE DIRECTLY */
#include <fcntl.h>
#include <sys/stat.h>

const char * get_mode_string (KCreateMode mode);
/* defined in dir_test.c */

extern const char *gRCModule_str[];
extern const char *gRCTarget_str[];
extern const char *gRCContext_str[];
extern const char *gRCObject_str[];
extern const char *gRCState_str[];
#define _LOG_RC(level,rc)	PLOGMSG(( level,			\
					  "Return Code:: $(M):$(T):$(C):$(O):$(S)", \
					  PLOG_5(PLOG_U32(M),PLOG_U32(T),PLOG_U32(C),PLOG_U32(O),PLOG_U32(S)), \
					  GetRCModule(rc),		\
					  GetRCTarget(rc),		\
					  GetRCContext(rc),		\
					  GetRCObject(rc),		\
					  GetRCState(rc)))
#define LOG_RC(level,rc)	PLOGMSG(( level,			\
					  "Return Code:: $(M):$(T):$(C):$(O):$(S)", \
					  PLOG_5(PLOG_S(M),PLOG_S(T),PLOG_S(C),PLOG_S(O),PLOG_S(S)), \
					  gRCModule_str[GetRCModule(rc)], \
					  gRCTarget_str[GetRCTarget(rc)], \
					  gRCContext_str[GetRCContext(rc)], \
					  (GetRCObject(rc)<rcLastTarget_v1_0)? \
					  gRCTarget_str[GetRCObject(rc)]: \
					  gRCObject_str[1+GetRCObject(rc)-rcLastTarget_v1_0], \
					  gRCState_str[GetRCState(rc)]))
/* #define LOG_RC(level,rc)	\ */
/*     PLOGMSG(( level, "m $(R)",PLOG_S(R),gRCModule_str[GetRCModule(rc)])),	\ */
/* 	PLOGMSG(( level, "t $(R)",PLOG_S(R),gRCTarget_str[GetRCTarget(rc)])), \ */
/* 	PLOGMSG(( level, "c $(R)",PLOG_S(R),gRCContext_str[GetRCContext(rc)])), \ */
/* 	PLOGMSG(( level, "o $(R)",PLOG_S(R),(GetRCObject(rc)<rcLastTarget_v1_0)? \ */
/* 		  gRCTarget_str[GetRCObject(rc)]:			\ */
/* 		  gRCObject_str[1+GetRCObject(rc)-rcLastTarget_v1_0])), \ */
/* 	PLOGMSG(( level, "s $(R)",PLOG_S(R),gRCState_str[GetRCState(rc)])) */


#ifdef _DEBUGGING
#define FUNC_ENTRY() DBGMSG (DBG_KFS, DBG_FLAG(DBG_KFS_ARCENTRY), ("Enter: %s\n", __func__))
#define KFF_DEBUG(msg) DBGMSG (DBG_KFS, DBG_FLAG(DBG_KFS_ARC), msg)
#else
#define FUNC_ENTRY()
#define KFF_DEBUG(msg)
#endif


/* ----------------------------------------------------------------------
 * 
 */

/* -----
 * define the specific types to be used in the templatish/inheritancish
 * definition of vtables and their elements
 */
#define KDIR_IMPL struct KArcDir
#define KFILE_IMPL struct KArcFile
#include <kfs/impl.h>

static rc_t KArcDirResolvePathNode (const KArcDir *self,
                                    enum RCContext ctx,
                                    const char *path,
                                    bool follow_sym_link,
                                    const KTocEntry ** pnode,
                                    KTocEntryType * ptype);



/* ======================================================================
 * Defines relevant to the whole compilation unit.
 */
/* arbitrary number that was originially set much much higher than expected needs. */
#define	KARC_LINK_RESOLVE_LOOPMAX		(16)

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
 * KArcDirIterator
 *
 * This very simple iterator steps in order through the nodes of the binary
 * search tree that is a "directory" in a KToc and thus steps through the
 * list of elements in a directory.  Only the name is made available at each
 * step.  That worked better for sysdir than arcdir from where this was 
 * borrowed (it was called SysDirEnum there)
 *
 * It is primarily used to build a directory listing.
 *
 * This iterator is a once through iterator.  Whack must be called after Init
 * or a dangling reference is left open.  Init can be called again but only if 
 * Whack was indeed called.
 *
 * NOTE:
 * This class unlike most in the KFS system is built in place and is not
 * constructed by default allocating memory in the constructor.
 * This means it can not be truly opaque as the structure of the base struct
 * has to be exposed so it can be created.
 */


/*--------------------------------------------------------------------------
 * KArcDirIterator
 *
 * a ArcDir directory enumerator/iterator getting the names of elements in
 * the TOC.
 *
 * const KToc *	  toc		a keep alive so the BSTree won't go
 *					away in the middle
 * union                  u		A union to allow pointers to base and
 *					derivative class
 *					without casting
 *   const BSTNode * 	  bnp		point to a node as a BSTNode
 *   const KTocEntry *  tnp		point to a node as a KTocEntry
 * bool                   isFirst;	upon creation this flag is set to true;
 *					set to false the first next call
 */
typedef struct KArcDirIterator 	
{
    const KToc * toc;	        /* a keep alive so the BSTree won't go away in the middle */
    union
    {
        const BSTNode * bnp;	/* point to a node as a BSTNode */
        const KTocEntry * tnp;	/* point to a node as a KTocEntry */
    }			u;
    bool isFirst;	            /* upon creation this flag is set to true
                                   set to false the first next call */
} KArcDirIterator;


/* ----------------------------------------------------------------------
 * KArcDirIteratorWhack
 * 	The Object destructor
 * [INOUT] KArcDirIterator *self	Iterator self reference: object oriented in C
 */
static
void KArcDirIteratorWhack (KArcDirIterator *self)
{
    KTocRelease ( self -> toc );
}


/* ----------------------------------------------------------------------
 * KArcDirIteratorInit
 * 
 * The object constructor
 *
 * [OUT] KArcDirIterator *	self		Iterator self reference: object oriented in C
 * [IN]  const KArcDir * 	arcdir		A directory that contains beneath it (or as it)
 *						the directory to be iterated.
 * [IN]	 const char * 		path		The directory to be iterated as referenced from
 *						arcdir.
 *
 * To iterate arcdir use a path of ".".
 */
static
rc_t KArcDirIteratorInit (KArcDirIterator *self, const KArcDir * arcdir, 
                          const char * path)
{
    const BSTree * 	tree;
    const KTocEntry * node;
    const KToc *	toc;
    KTocEntryType	type;
    rc_t		rc;

    /* is self parameter possibly bad? */
    if (self == NULL)
        return RC (rcFS, rcDirectory, rcConstructing, rcSelf, rcNull);

    self->toc = NULL;
    self->isFirst = true;	/* warning squelcher ? that breaks type opacity */
    self->u.bnp = NULL;

    /* is another parameter possibly bad? */
    if ((path == NULL)||(arcdir == NULL))
        return RC (rcFS, rcDirectory, rcConstructing, rcParam, rcNull);

    if (KArcDirGetTOC(arcdir,&toc) != 0)
        return RC (rcFS, rcDirectory, rcConstructing, rcParam, rcNotFound);

    self->toc = toc;
    KTocAddRef(toc);

    /* get the KTocEntry from which all things are possible */
    rc = KArcDirResolvePathNode (arcdir, rcConstructing, path, true, &node, &type);
    if ( rc == 0 )
    {
        if ((type == ktocentrytype_dir) || (type == ktocentrytype_hardlink))
        {
            rc = KTocEntryGetBSTree (node, &tree);
            if (rc == 0)
            {
                /* success */
                self->u.bnp = BSTreeFirst(tree);
                return 0;
            }
        }
        else
        {
            rc = RC (rcFS, rcDirectory, rcConstructing, rcParam, rcInvalid);
        }
    }

    /* failure */
    KTocRelease (self->toc);
    return rc;
}


/* ----------------------------------------------------------------------
 * KArcDirIteratorNext
 *
 * Go to the next node (or the first node if the iterator has not been used.
 *
 * [RET]   const char * 			The return is the name of the next element
 * [INOUT] KArcDirIterator *	self		Iterator self reference: object oriented in C
 */
static const char * KArcDirIteratorNext (KArcDirIterator *self)
{
    const char * name;

    assert (self != NULL);

    /* -----
     * The initialization put the first node's address into the pointer
     * so do do nothing about the pointer before using it except say we
     * need to update it next time.
     */
    if (self->isFirst)
    {
        self->isFirst = false;
    }
    /* -----
     * After the first time we need to bump the pointer until it can't be bumped.
     */
    else	
    {
        /* -----
         * It's not specified in klib/container.h but BSTNodeNext
         * returns NULL at end of tree
         * [ WHAT ELSE WOULD IT RETURN? ]
         */
        self->u.bnp = BSTNodeNext (self->u.bnp);
    }

    /* -----
     * The first test isn't necesary so letting KTocEntryGetName return NULL
     * could be used instead of the extra check.
     * Checking against NULL adds a little to each call while not checking
     * adds more but only to the last call.
     * Pick your poison.
     */
    if ((self->u.bnp == NULL) || (KTocEntryGetName (self->u.tnp, &name) != 0))
    {
        return NULL;
    }
    return name;
}


/* ======================================================================
 */
typedef struct KArcListing KArcListing;

#define KNAMELIST_IMPL struct KArcListing
#include <klib/impl.h>

/*--------------------------------------------------------------------------
 * KArcListing
 *  a directory listing
 *
 * NOTE:
 * This is nearly identical to KSysDirListing but both are currently private to
 * their compilation units.
 */
struct KArcListing
{
    KNamelist     dad;      /* base class */
    const char ** namelist; /* pointer to a malloced array of pointers to names */
    int           cnt;      /* specific size chosen by compiler for efficiency */
};


/* ----------------------------------------------------------------------
 * KArcListingWhack
 *
 * Class destructor
 *
 *
 * [RET] rc_t					0 for success; anything else for a failure
 *						see itf/klib/rc.h for general details
 * [INOUT] const KArcListing* self		Listing self reference: object oriented in C
 */
static
rc_t CC KArcListingWhack (const KArcListing *self)
{
    int ix;
    for ( ix = 0; ix < self->cnt; ++ix )
    {
        free ( (void*)self->namelist[ix] );
    }
    free ( (void*)self->namelist );
    return 0;
}

/* ----------------------------------------------------------------------
 * KArcListingDestroy
 * Class destructor
 *
 *
 * [RET] rc_t					0 for success; anything else for a failure
 *						see itf/klib/rc.h for general details
 * [INOUT] KArcListing *	self		Listing self reference: object oriented in C
 */
static
rc_t CC KArcListingDestroy (KArcListing *self)
{
    rc_t rc = KArcListingWhack (self);
    if (rc == 0)
        free (self);
    return rc;
}


/* ----------------------------------------------------------------------
 * KArcListingCount
 *
 *
 * [RET] rc_t					0 for success; anything else for a failure
 *						see itf/klib/rc.h for general details
 * [IN]  const KArcListing *	self		Listing self reference: object oriented in C
 * [OUT] uint32_t *		count		Where to put the count of names
 */
static rc_t CC KArcListingCount (const KArcListing *self, uint32_t *count)
{
    *count = self->cnt;
    return 0;
}


/* ----------------------------------------------------------------------
 * KArcListingGet
 *
 *
 * [RET] rc_t					0 for success; anything else for a failure
 *						see itf/klib/rc.h for general details
 * [IN]  const KArcListing *	self		Listing self reference: object oriented in C
 * [IN]  uint32_t		idx		?
 * [OUT] const char **		name		Where to put the name
 */
static rc_t CC KArcListingGet (const KArcListing *self, uint32_t idx, const char **name)
{
    if (idx >= (uint32_t)self->cnt)
        return RC (rcFS, rcNamelist, rcAccessing, rcParam, rcExcessive);
    * name = self -> namelist [ idx ];
    return 0;
}


/* ----------------------------------------------------------------------
 * KArcListingSort
 *
 * This function has the signature needed to use with the NameList base class for
 * KArcListings to determine the order of two names.  Matches the signature of
 * strcmp() and other functions suitable for use by qsort() and others
 *
 * [RET] int					0:  if a == b 
 *						<0: if a < b
 *						>0: if a > b
 * [IN] const void *		a
 * [IN] const void *		b
 *
 * Elements are typed as const void * to match the signature needed for 
 * a call to qsort() but they should be pointers to namelist elements
 */
static int64_t CC KArcListingSort (const void *a, const void *b, void * ignored)
{
    return strcmp (*(const char**)a, *(const char**)b);
}

static KNamelist_vt_v1 vtKArcListing =
{
    /* version 1.0 */
    1, 0,

    /* start minor version 0 methods */
    KArcListingDestroy,
    KArcListingCount,
    KArcListingGet
    /* end minor version 0 methods */
};


/* ----------------------------------------------------------------------
 * KArcListingInit
 *
 * [RET] rc_t					0 for success; anything else for a failure
 *						see itf/klib/rc.h for general details
 * [INOUT] KArcListing *	self		Listing self reference: object oriented in C
 * [IN]    const char *		path		?
 * [IN]    const KDirectory *	dir		?
 * [IN]    bool (* 		f	)(const KDirectory*, const char*, void*),
 *						This is a filter function - any listing element
 *						passed to this function will generate a true ot
 *						a false.  If flase that listing element is dropped.
 *						If this parameter is NULL all elements are kept.
 * [IN]	   void *		data		Ignored.  May use NULL if permitted
 *						by 'f'.
 */
static rc_t KArcListingInit (KArcListing *self,
                             const char *path,
                             const KDirectory *dir,
                             bool (CC* f) (const KDirectory*, const char*, void*),
                             void *data)
{
    rc_t rc;

    /* is self parameter possibly NULL? */
    if (self == NULL)
    {
        return  RC (rcFS, rcDirectory, rcConstructing, rcSelf, rcNull);
    }

    /* start with an empty name list */
    self->namelist = NULL;
    self->cnt = 0;

    /* initialize the Namelist base class */
    if ((rc = KNamelistInit (& self -> dad,
			     (const KNamelist_vt*)&vtKArcListing)) == 0)
    {
        /* -----
         * so we'll build a list iterator
         */
        KArcDirIterator listitr;

        if ((rc = KArcDirIteratorInit (& listitr, (const KArcDir*)dir, path)) == 0)
        {

#define	LEN_INCREMENT	(512)

            uint32_t len = LEN_INCREMENT;

            /* allocate heap for the default start size of a namelist */

            self->namelist = malloc (len * sizeof (self->namelist[0]));
            if (self->namelist == NULL)
            {
                rc = RC (rcFS, rcDirectory, rcListing, rcMemory, rcExhausted);
            }
            else
            {
                void *r;
                const char *name;

                /* -----
                 * loop through the directory (the BSTree for a Archive TOC)
                 */
                while ((name = KArcDirIteratorNext (&listitr)) != NULL)
                {
                    /* if there is a filter function, run it */
                    if (f != NULL)
                    {
                        /* skip if the filter doesn't say use it */
                        if (! (*f) (dir, name, data))
                            continue;
                    }
		    
                    /* if the buffer is full, reallocate it larger */
                    if (self->cnt == len)
                    {
                        len += LEN_INCREMENT;
                        if ( NULL == ( r = realloc ( (void*)self -> namelist,
                                                     len * sizeof self -> namelist [ 0 ] ) ) )
                        {
                            /* -----
                             * malloc failure so we fail too  - break not return so we can 
                             * free allocated memory
                             */
                            rc = RC (rcFS, rcDirectory, rcListing, rcMemory, rcExhausted);
                            break;
                        }
                        self->namelist = r;
                    }
                    
                    /* get ourselves memory for the name */
                    self->namelist[self->cnt] = malloc (strlen (name) + 1);
                    if (self->namelist[self->cnt] == NULL)
                    {
                        /* oops a failure so we fail */
                        rc = RC (rcFS, rcDirectory, rcListing, rcMemory, rcExhausted);
                        break;
                    }
                    strcpy ((char*)self->namelist[self->cnt], name);
                    ++self->cnt;
                }
		
                if (rc == 0)
                {
                    /* -----
                     * now that we have our list and know how big it should have been we'll shrink it
                     * if the count was zero this a effectively just a free()
                     */
                    r = realloc ( ( void* )self->namelist,
                                  self->cnt * sizeof ( self->namelist[0] ) );
                    /* -----
                     * if we have a new non-NULL pointer we know we have a list of non-zero length
                     * store the pointer and sort the list
                     */
                    if (r != NULL)
                    {
                        self -> namelist = r;
                        ksort ((void*)r, self -> cnt, sizeof self -> namelist [ 0 ], KArcListingSort, NULL);
                    }
                    /* -----
                     * If we have a NULL pointer but have a non-zero count, we have a malloc() heap
                     * space failure
                     */
                    else if (self -> cnt != 0)
                    {
                        rc = RC (rcFS, rcDirectory, rcListing, rcMemory, rcExhausted);
                    }
                    /* -----
                     * Or we have a NULL pointer and a count of zero so stow the NULL so its not
                     * a dangling bad pointer
                     */
                    else
                    {
                        self->namelist = r;
                    }
                }
                /* if we got here because of a problem, kill the listing */
                if (rc != 0)
                {
                    KArcListingWhack (self);
                    self->namelist = NULL;
                    self->cnt = 0;
                }
            }
            /* we're done with the iterator so kill it */
            KArcDirIteratorWhack (&listitr);
        }
    }
    return rc;
}


/* ======================================================================
 */

/*--------------------------------------------------------------------------
 * KArcDir
 *  a directory inside an archive (tar or SRA for example)
 *
 * This type is being engineered to mimic KSysDir.
 */
struct KArcDir
{
    KDirectory		dad;					/* parent class as in inheritance */

    const KDirectory *  parent;					/* parent object */
    const KToc *	toc;					/* table of contents class */
    const KTocEntry *	node;					/* table of contents entry NULL for top level */
    KArcFSType	arctype;
    union
    {
	const void *		v;
	const KFile *		f;
	const KDirectory * 	d;
    } archive;

    uint32_t	  	root;					/* offset of / in path to treat as root */
    uint32_t 	  	size;					/* length of path */		
    char 	  	path	 [KARC_DEFAULT_PATH_ALLOC];	/* name of archive = mount point */
   
/* actual allocation is based on actual string length */
};

static rc_t KArcDirMake (KArcDir ** self,
                         enum RCContext ctx,
                         const KDirectory * parent,
                         const KToc * toc,
                         const KTocEntry * node,
                         const void * archive,
                         KArcFSType baseType,
                         uint32_t dad_root,
                         const char *path,
                         uint32_t path_size,
                         bool update, /* ignored */
                         bool chroot);


/* ----------------------------------------------------------------------
 * KArcDirDestroy
 */
static rc_t CC KArcDirDestroy (KArcDir *self)
{
    if (self->toc != NULL)
        KTocRelease (self->toc);

    if (self->archive.v != NULL) switch (self->arctype)
	{
	default:
	    free ((void*)self->archive.v);
	    break;
	case tocKFile:
	    KFileRelease (self->archive.f);
	    break;
	case tocKDirectory:
	    KDirectoryRelease (self->archive.d);
	    break;
	}
    free (self);

    return 0;
}



/* ----------------------------------------------------------------------
 * KArcDirCanonPath
 *
 * In this context CanonPath means to make the path a pure /x/y/z with no back tracking 
 * by using ~/../~ or redundant ~/./~ or ~//~ here notations.  Not exactly the usage of 
 * canonical in describing a path in other places but consistent within KFS.  It matches
 * the common meaning of canonical path as the one true path except that processing out
 * of sym links isn't done here but would normally have been.  Not processing the 
 * links means potentially more than one canonical path can reach the same target 
 * violating the usual meaning of canonical path as the one true shortest path to any
 * element.
 *
 * const KArcDir *		self		Object oriented C; KArcDir object for this method
 * enum RCContext 		ctx
 * char * 			path
 * size_t			psize
 */
static
rc_t		KArcDirCanonPath	(const KArcDir *self,
					 enum RCContext ctx,
					 char *path,
					 size_t psize)
{
    char *	low;	/* a pointer to the root location in path; not changed after initialization */
    char *	dst;	/* a target reference for compressing a path to remove . and .. references */
    char *	last;	/* the end of the last processed facet of the path */
    char *	end;	/* absolute end of the incoming path */
    char * 	src;	/* the start of the current facet to be processed */

    /* end is the character after the end of the incoming path */
    end = path + psize;

    /* point all other temp pointers at the root point in the incoming path */
    last = path + self->root;

    /* handle windows / / network path starter */
    if ((last == path) && (last[0] == '/') && (last[1] == '/'))
      last ++;

    low = dst = last;

    for (;;)
    {

        /* point at the first / after the most recent processed facet */
        src = strchr (last + 1, '/');
        if (src == NULL)	/* if no '/' point to the end */
            src = end;

        /* detect special sequences */
        switch (src - last)
        {
        case 1: /* / / (with nothing between) is a superflouous / hurting us to parse later;
                 * /. is a here reference to the same directory as the previous */
            if ((last[1] == '/')||(last[1] == '.'))
            {
                /* skip over */
                last = src;
                if (src != end)
                    continue;
            }
            break;
        case 2: /* /./ is a "here" reference point and is omitted by not copying it over */
            if (last[1] == '.')
            {
                /* skip over */
                last = src;
                if (src != end)
                    continue;
            }
            break;

        case 3: /* /../ is a up one directory and is processed by deleting the last facet copied */
            if (last [1] == '.' && last [2] == '.')
            {
                /* remove previous leaf in path */
                dst [ 0 ] = 0;
                dst = strrchr (path, '/');
                /* can't up a directory past the root */
                if (dst == NULL || dst < low)
                {
                    return RC (rcFS, rcDirectory, ctx, rcPath, rcInvalid);
                }

                last = src;
                if (src != end)
                    continue;
            }
            break;
        }

        /* if rewriting, copy leaf */
        if (dst != last)
        {
            memmove (dst, last, src - last);
        }

        /* move destination ahead */
        dst += src - last;
        
        /* if we're done, go */
        if (src == end)
            break;

        /* find next separator */
        last = src;
    }

    /* NUL terminate if modified */
    if (dst != end)
        * dst = 0;

    /* say we did did it with no problems */
    return 0;
}


/* ----------------------------------------------------------------------
 * KArcDirMakePath
 *
 *  creates a full path from partial
 *
 * [IN]  const KArcDir *	self	object oriented self
 * [IN]  enum RCContext		ctx	a hint for context in building
 *					an error return rc_t
 * [IN]  bool			canon	Should the output be made 
 *					canonical per KDirectory's
 *					definition of canonical
 * [OUT] char **		pbuffer The output path
 * [IN]  const char *		path	The input path
 * [IN]  va_list		args	Possible additions to path
 */
static
rc_t KArcDirMakePath (const KArcDir *self,
		      enum RCContext ctx,
		      bool canon,
		      char ** pbuffer,
		      const char *path,
		      va_list args)
{
    int    psize;	/* 'printed' size - output from vsnprintf */
    size_t asize;	/* allocated buffer size */
    size_t bsize;	/* base size */
    char * buffer;

    /* -----
     * Validate parameters and fail if unusable
     */
    assert (path != NULL);
    assert (pbuffer != NULL);

    if (path == NULL)
    {
	return RC (rcFS, rcDirectory, ctx, rcPath, rcNull);
    }
    if (path[0] == 0)
    {
	return RC (rcFS, rcDirectory, ctx, rcPath, rcInvalid);
    }

    buffer = NULL;
    asize = KARC_DEFAULT_PATH_ALLOC;
    for (;;)
    {
	/* -----
	 * allocate a buffer of the default size or a better size determined below
	 */
	buffer = realloc (buffer, asize);
	*pbuffer = buffer;
	if (buffer == NULL)
	{
	    return RC (rcFS, rcDirectory, rcAllocating, rcPath, rcExhausted);
	}
	/* -----
	 * inherited from linux/sysdir.c
	 *
	 * if path starts with % we build something out of va_list?
	 *
	 * use vsnprintf (printf to string with size limit and argument list
	 *	pointer instead of a list of arguments:
	 * build path to buffer using a path that is a printf format string
	 * with a requirement that the first thing in the format be a format inducing %
	 */
	if ((args != NULL) && (path[0] == '%'))
	{
	    psize = vsnprintf (buffer, asize, path, args);

	    /* -----
	     * decoding of path failed due a failure of printf
	     * <0 is an unspecified failure unspecified (check errno?)
	     */
	    if (psize < 0)
            return RC (rcFS, rcDirectory, ctx, rcPath, rcUnknown);

	    /* -----
	     * decoding of path failed due to length truncation
	     * try to realloc using a larger size
	     */
	    if ( (size_t)psize >= asize )
	    {
            asize = psize;
            continue;
	    }

	    /* -----
	     * maybe the decoded path is a relative partial path
	     */
	    if (buffer[0] != '/')
	    {
            /* -----
             * if our KArcDir self has a mount point self->path of length more than 0
             * but that length isn't too long to prepend it to the derived path than
             * move the derived path over making room for the KArcDir base path and 
             * then insert that base at the beginning
             */
            bsize = self->size;
            if (bsize + psize >= asize)
            {
                asize = bsize + psize;
                continue;
            }
            /* make room */
            memmove (buffer + self->size, buffer, psize+1);
            /* fail if the kDirectory path doesn't end in /? (inherited) */
            assert (self->path[bsize-1] == '/');
            /* prepend self's path */
            memmove (buffer, self->path, bsize);
	    }

	    /* -----
	     * or maybe we have a path base for the KArcDir
	     */
	    else if ((bsize = self->root) != 0)
	    {
            /* -----
             * if we got a full path from the decoding do the same thing but only the
             * amount of the self's path that is up to an effective "chroot" point
             */
            if (bsize + psize >= asize)
            {
                asize = bsize + psize;
                continue;
            }
            memmove (buffer + self->size, buffer, psize+1);
            assert (self->path [bsize-1] != '/');
            memmove (buffer, self -> path, bsize);
	    }
	    /* -----
	     * or maybe the new path is cool as is.  unlikely?
	     */
	}
	else
	{
	    /* -----
	     * copy a partial/relative  path parameter into the buffer leaving room for the
	     * self's base path
	     */
	    if (path[0] != '/')
	    {
            assert (self->path[self->size - 1] == '/');
            memmove (buffer, self->path, bsize = self->size);
	    }
	    /* -----
	     * copy the pre-root portion of the self's path into the buffer
	     */
	    else if ((bsize = self->root) != 0)
	    {
            assert (self->path[bsize-1] != '/');
            memmove (buffer, self->path, bsize);
	    }

	    /* -----
	     * okay use snprintf to append the incoming path into the buffer after the
	     * self's base or root either using a simple string format or again assuming
	     * that the path has printf format symbols if there are arguments in the
	     * va_list
	     */
	    if (args == NULL)
            psize = snprintf (buffer+bsize, asize-bsize, "%s", path);
	    else
            psize = vsnprintf (buffer+bsize, asize-bsize, path, args);

	    /* -----
	     * decoding of path failed due a failure of printf
	     * <0 is an unspecified failure unspecified (check errno?)
	     */
	    if (psize < 0)
            return RC (rcFS, rcDirectory, ctx, rcPath, rcUnknown);

	    /* -----
	     * decoding of path failed due to length truncation
	     * try to realloc using a larger size
	     */
	    if (bsize+psize >= asize)
	    {
            asize = bsize+psize;
            continue;
	    }
	}
	break;
    }
    /* -----
     * if the last character is a '/' get rid of it?
     */
    if (buffer[bsize+psize] == '/')
        buffer[bsize+(--psize)] = 0;

    /* -----
     * if the incoming path turns out to be a single character (I think) AND
     * we either have the canon flag set or if the the KArcDir root is not empty
     * "canonize" it.
     */
    if (canon || (self->root != 0))
    {
        return KArcDirCanonPath (self, ctx, *pbuffer, bsize+psize);
    }
    return 0;
}


/* ----------------------------------------------------------------------
 * KArcDirList
 *  create a directory listing
 *
 *  "list" [ OUT ] - return parameter for list object
 *
 *  "path" [ IN, NULL OKAY ] - optional parameter for target
 *  directory. if NULL, interpreted to mean "."
 *
 * [RET] rc_t					0 for success; anything else for a failure
 *						see itf/klib/rc.h for general details
 * [IN]	 const KArcDir *	self		Object oriented C; KArcDir object for this method
 * [OUT] KNamelist **		listp,
 * [IN]  bool (* 		f	)(const KDirectory*,const char *, void *)
 * [IN]  void *			data
 * [IN]  const char *		path
 * [IN]  va_list		args
 */
static
rc_t CC KArcDirList (const KArcDir *self,
                     KNamelist **listp,
                     bool (CC* f) (const KDirectory *dir, const char *name, void *data),
                     void *data,
                     const char *path,
                     va_list args)
{
    char * full_path;
    rc_t rc;

    rc = KArcDirMakePath (self, rcListing, true,
			  &full_path, path, args);
    if (rc == 0)
    {
	const KTocEntry * pnode;
	KTocEntryType	type;
	KArcDir * full_dir;

	rc = KArcDirResolvePathNode(self, rcListing, full_path, true, &pnode, &type);
	if (rc == 0)
	{
	    rc = KArcDirMake (&full_dir,
			      rcListing,
			      self->parent,
			      self->toc,
			      pnode,
			      self->archive.v,
			      self->arctype,
			      self->root,
			      full_path,
			      (uint32_t)strlen(full_path),
			      false, 
			      0);
	    if (rc == 0)
	    {
		KArcListing *list = malloc (sizeof *list);

		if (list == NULL)
		{
		    rc = RC (rcFS, rcDirectory, rcListing, rcMemory, rcExhausted);
		}
		else
		{
		    rc = KArcListingInit (list,
					  full_dir->path,
					  &full_dir->dad,
					  f,
					  data);

		    if (rc != 0)
		    {
			free (list);
		    }
		    else
		    {
			*listp = &list->dad;
		    }
		}
		KArcDirDestroy (full_dir);
	    }
	}
	free (full_path);
    }
    return rc;
}


/* ----------------------------------------------------------------------
 * KArcDirFullPathType
 *
 * Get the KDirectory defined KPathType for whatever is referred to 
 * by path that is accessible through the KArcDir self.  The path's
 * root is based on self's root (?) and a relative path is relative to 
 * self.
 *
 * [RET] uint32_t			actually this is enum KPathType
 * [IN]  const KArcDir *	self	base KDirectory for the call to
 *					find the type of path
 * [IN]  const char * 		path	what path of which to type the
 *					last facet
 */
static
uint32_t KArcDirFullPathType (const KArcDir *self, const char * path)
{
    const KTocEntry * node;
    const char *	local_path = path;
    KTocEntryType	type;
    uint32_t		tt = 0;
    int			loopcount;
    rc_t		rc;

    assert (self != NULL);
    assert (path != NULL);

    for (loopcount = 0; loopcount < KARC_LINK_RESOLVE_LOOPMAX; ++loopcount)
    {
        rc = KArcDirResolvePathNode (self,
            rcConstructing, local_path, false, &node, &type); 
        if (rc != 0)
        {
            /* TODO: filter rc into kptBadPath or kptNotFound... */
            return (tt | kptNotFound);
        }
        switch (type)
        {
        default:
            return kptBadPath;

        case ktocentrytype_unknown:
            return kptBadPath;

        case ktocentrytype_hardlink:
            return kptDir;

        case ktocentrytype_dir:
            return tt | kptDir;

        case ktocentrytype_file:
        case ktocentrytype_emptyfile:
            return tt | kptFile;

        case ktocentrytype_chunked:
            return tt | kptFile;

        case ktocentrytype_softlink:
            tt = kptAlias;
            if (KTocEntryGetSoftTarget(node,&local_path) != 0)
                return kptAlias|kptNotFound;
            break;

        case ktocentrytype_zombiefile:
            PLOGMSG (klogWarn, (klogWarn,
                "zombie file detected: '$(P)'", "P=%s", path));
            ReportRecordZombieFile();
            return tt | kptZombieFile;

        }
    }
    return kptBadPath;
}


/* ----------------------------------------------------------------------
 * KArcDirPathType
 *  returns a KPathType
 *
 *  "path" [ IN ] - NUL terminated string in directory-native character set
 * NOTE: Does not meet a design target of on stack (localized variable) allocation of single 4kb path
 *
 * [RET] uint32_t
 * [IN]  const KArcDir *	self		Object oriented C; KArcDir object for this method
 * [IN]  const char *		path
 * [IN]  va_list		args
 */
static uint32_t CC KArcDirPathType (const KArcDir *self, const char *path, va_list args)
{
    rc_t	rc;
    uint32_t 	type;
    char * 	full;

    rc = KArcDirMakePath (self, rcAccessing, false, &full, path, args);

    if (rc == 0)
    {
        type = KArcDirFullPathType (self, full);
        free (full);
    }
    else
    {
        type = kptBadPath;
    }
    return type;
}


/* ----------------------------------------------------------------------
 * KArcDirVisitDir
 *
 * [IN] KArcDirVisitData *	pb
 */
typedef struct KArcDirVisitData		
{
    rc_t   (CC* f )(const KDirectory*, uint32_t, const char*, void*);
    void *      data;
    KArcDir *   dir;
    bool        recurse;
} KArcDirVisitData;


static
rc_t KArcDirVisitDir(KArcDirVisitData *pb)
{
    /* get a directory listing */
    KArcDirIterator 	listing;
    rc_t		rc; 

    assert (pb != NULL);

    rc = KArcDirIteratorInit (&listing, pb->dir, pb->dir->path);
    if (rc == 0)
    {
        const char * base;
        const char * name;
        char * full_name = NULL;
        uint32_t 	size;

        size = pb->dir->size;
        base = pb->dir->path;

        for(pb->dir->size = size, name = KArcDirIteratorNext (&listing);
            name != NULL; name = KArcDirIteratorNext (&listing))
        {
            uint32_t len = (uint32_t)strlen (name);
            free(full_name);
            full_name = malloc(size + len + 1);

            if(full_name == NULL) {
                rc = RC (rcFS, rcDirectory, rcVisiting, rcMemory, rcExhausted);
                break;
            } else {
                const KTocEntry *	pnode;
                KTocEntryType		type;
                /* -----
                * build up the path from the base and the name
                */
                memmove(full_name, base, size);
                memmove(full_name+size, name, len);
                full_name[size+len] = 0;

                rc = KArcDirResolvePathNode(pb->dir, rcVisiting, full_name,
                                            true, &pnode, &type);
                if(rc != 0) {
                    break;
                }
                /* -----
                * call the function per visit
                */
                /* type is wrong "type"  needs to be a kpt not karctoctype */
                {
                    uint32_t kptype;
                    kptype = KArcDirFullPathType(pb->dir, full_name);
                    rc = pb->f(&pb->dir->dad, kptype, name, pb->data);
                }
                if(rc != 0) {
                    break;
                }
                if(pb->recurse && ((type == ktocentrytype_dir)||(type == ktocentrytype_hardlink))) {
                    KArcDir * rdir;
                    KArcDirVisitData rpb;

                    rc = KArcDirMake(&rdir, rcVisiting, pb->dir->parent, pb->dir->toc,
                                     pnode, pb->dir->archive.v, pb->dir->arctype, pb->dir->root,
                                     full_name, (uint32_t)strlen ( full_name ), false, false);
                    if(rc != 0) {
                        break;
                    }
                    rpb.f = pb->f;
                    rpb.data = pb->data;
                    rpb.dir = rdir;
                    rpb.recurse = pb->recurse;
                    rc = KArcDirVisitDir(&rpb);
                    KArcDirDestroy(rdir);
                    if(rc != 0) {
                        break;
                    }
                }
            }
        }
        free(full_name);
        KArcDirIteratorWhack (& listing);
    }
    return rc;
}


/* ----------------------------------------------------------------------
 * KArcDirRelativePath
 *
 * KArcDirRelativePath
 *  makes "path" relative to "root"
 *  both "root" and "path" MUST be absolute
 *  both "root" and "path" MUST be canonical, i.e. have no "//", "/./" or "/../" sequences
 *
 * [RET] rc_t					0 for success; anything else for a failure
 *						see itf/klib/rc.h for general details
 * [IN] const KArcDir *		self		Object oriented C; KArcDir object for this method
 * [IN] enum RCContext 		ctx
 * [IN] const char *		root
 * [IN, OUT] char *			path
 * [IN] size_t			path_max
 */
static
rc_t KArcDirRelativePath (const KArcDir *self, enum RCContext ctx,
			  const char *root, char *path, size_t path_max)
{
    int backup;
    size_t bsize, psize;

    const char *r = root + self -> root;
    const char *p = path + self -> root;

    assert (r != NULL && r [ 0 ] == '/');
    assert (p != NULL && p [ 0 ] == '/');

    for (; * r == * p; ++ r, ++ p)
    {
        /* disallow identical paths */
        if (* r == 0)
            return RC (rcFS, rcDirectory, ctx, rcPath, rcInvalid);
    }

    /* paths are identical up to "r","p"
       if "r" is within a leaf name, then no backup is needed
       by counting every '/' from "r" to end, obtain backup count */
    for (backup = 0; * r != 0; ++ r)
    {
        if (* r == '/')
            ++ backup;
    }

    /* the number of bytes to be inserted */
    bsize = backup * 3;

    /* align "p" to last directory separator */
    while (p [ -1 ] != '/') -- p;

    /* the size of the remaining relative path */
    psize = strlen (p);

    /* open up space if needed */
    if ( (size_t)(p - path) < bsize )
    {
        /* prevent overflow */
        if (bsize + psize >= path_max)
            return RC (rcFS, rcDirectory, ctx, rcPath, rcExcessive);
        
        memmove (path + bsize, p, psize);
    }

    /* insert backup sequences */
    for (bsize = 0; backup > 0; bsize += 3, -- backup)
        memmove (& path [ bsize ], "../", 3);

    /* close gap */
    if ( (size_t)( p - path ) > bsize )
	{
		memmove (path + bsize, p, psize + 1);
	}

	return 0;
}


/* ----------------------------------------------------------------------
 * KArcDirResolvePathNode
 *
 * Resolve a path to a KToc Node with a triple return type of
 * success/failure, KTocEntry to use to access the leaf of the path,
 * and the type of the node that could have been easily obtained from 
 * that node
 *
 * [RET] rc_t					0 for success; anything else for a failure
 *						see itf/klib/rc.h for general details
 * [IN]  const KArcDir *      self	OOP in C self/this pointer
 * [IN]  const char *         path	the path to resolve
 * [IN]  bool		      follow	Follow any links
 * [OUT] const KTocEntry ** pnode	a pointer that will point to the 
 *					ArcTOCNode for the path
 * [OUT] KTocEntryType *    ptype	Archive specific type for the
 *					entry matching path
 * TODO: Make this prettier for human readabilty by breaking out into sub-functions?
 */
static
rc_t	KArcDirResolvePathNode	(const KArcDir *	self,
				 enum RCContext 	ctx,
				 const char *		path,
				 bool			follow,
				 const KTocEntry ** 	pnode,
				 KTocEntryType * ptype)
{
    rc_t		rc = 0;

    assert (self != NULL);
    assert (path != NULL);
    assert (pnode != NULL);
    assert (ptype != NULL);

    *pnode = NULL;
    *ptype = ktocentrytype_unknown;

    if (path[0] == 0)
    {
        rc = RC (rcFS, rcDirectory, ctx, rcPath, rcInvalid);
    }
    else
    {
	/* -----
	 * This loop is to replace recursive approaches to resolving sym links in 
	 * the path.before the last facet.
	 *
	 * we used temp_path as an alias for path because we can "recur" by 
	 * going through the loop iteratively instead of recursively changing
	 * temp_path each time where we would have called this function again
	 */
	const char *	temp_path = path;	/* no alloc */
	const KToc *	toc = self->toc;	/* no alloc */
	char *		allocated_path = NULL;	/* alloc; must free{} */
	char * 		abpath = NULL;		/* alloc; must free{} */

	while(rc == 0) /* loop as long as no failures or until break */
	{
	    /* -----
	     * clean up the path to its KFS canonical form
	     */
	    if (abpath != NULL)
            free (abpath);
	    rc = KArcDirMakePath (self, ctx, /* canon*/true, &abpath,
                              temp_path, NULL);
	    if (rc != 0)
	    {
            /* -----
             * If we couldn't "make the path" we'll just fail passing along
             * the fail reason from MakePath.
             */
            break;	/* TODO fix rc context? Object? */
	    }
	    else
	    {
            const String * pathstring;

            rc = KTocGetPath (toc, &pathstring);
            /* -----
             * validate that this canonical path is in the TOC
             */
            if (strncmp(abpath, pathstring->addr, pathstring->size) != 0)
            {
                /* -----
                 * This is a key return as it could be used to trigger a call to 
                 * a containing "parent" KDirectory to try again outside of this KArcDir
                 * be it another KArcDir, KSysDir, or other KDirectory interface 
                 * implementation.
                 */
                rc = RC (rcFS, rcDirectory, ctx, rcPath, rcOutOfKDirectory);
                break;
            }
            else
            {
                const KTocEntry *	node;		/* no alloc */
                const String *	toc_path_string;	/* no alloc */
                const char *	toc_path;		/* no alloc */
                const char *	left;		/* no alloc */
                KTocEntryType	type  = ktocentrytype_unknown;
                /* -----
                 * now get the path that is just that which is inside the TOC by pointing
                 * past the path of the TOC and get the relevant TOC.  Since the incoming path
                 * and the "makepath" path have all been ASCIZ so is this abbreviated path.
                 */


                rc = KTocGetPath (toc, &toc_path_string);
                toc_path = abpath + toc_path_string->size; /* point past toc's path to either NUL or '/' */
                if (toc_path[0] == '/')		/* if '/' move forward one char */
                    toc_path++;
                else if (toc_path[0] != 0x00)	/* if not NUL we are not in this TOC */
                {
                    rc = RC (rcFS, rcDirectory, ctx, rcPath, rcOutOfKDirectory);
                    break;
                }

                /* if the called resolve failed then this resolve fails too */
                rc = KTocResolvePathTocEntry (toc, &node, toc_path, strlen(toc_path), &type, &left);
                if (rc != 0)
                {
                    /* nothing? or fix ret? */
                }
                else if (type != ktocentrytype_softlink)
                {
                    /* -----
                     * This is the non soft link successful result
                     */
                    *ptype = type;
                    *pnode = node;
                    /* rc is 0 at this point */
                    break; /* out of for (;;) */
                }
                else
                {
                    size_t	lsize;
			
                    /* -----
                     * successful so far but there might still be some path left if we hit a soft link
                     */
                    lsize = strlen(left);	/* points to NUL for empty string if we got to the end */

                    if ((lsize == 0) && (! follow))
                    {
                        /* -----
                         * This is a soft link successful result
                         */
                        *ptype = type;
                        *pnode = node;
                        /* rc is 0 at this point */
                        break; /* out of for (;;) */
                    }
                    /* -----
                     * If we did hit a soft link with path remaining we have work to do resolving the
                     * facets in the path before the end.
                     *
                     * the remaining path could be as little as a single "/".
                     */
                    else
                    {
                        const char *	symlink_path;		/* no alloc */
                        rc = KTocEntryGetSoftTarget(node, &symlink_path);
                        if (rc != 0)
                        {
                            /* fix rc_t? */
                            break;
                        }
                        else
                        {
                            /* -----
                             * If the symbolic link's path starts with '/' it is an absolute path
                             */
                            if (symlink_path[0] == '/')
                            {
                                temp_path = symlink_path;
                                continue; /* back to for(;;) */
                            }
                            else
                            {
                                /* -----
                                 * if the first character isn't '/' it is relative to the current
                                 * directory within the TOC
                                 */
                                size_t		asize;		/* allocated size */
                                size_t		fsize;		/* first - before symlink - size */
                                size_t  	ssize;		/* symlink value size */
                                const char * 	backtrack;
                                /* -----
                                 * Backtrack across the last facet - which was the link name
                                 */
                                for (backtrack = left - 1; *backtrack != '/'; --backtrack)
                                    ;
                                backtrack++; /* get back the '/' */

                                /* -----
                                 * we need enough space for the 
                                 */
                                fsize = backtrack - abpath;	/* current path through start of link */
                                ssize = strlen(symlink_path);/* length of the link's replacement value */
                                asize = fsize+ssize+lsize+1;/* total of the three pieces and NUL */
                                allocated_path		/* get a temp buffer to build this new path */
                                    = realloc (allocated_path, asize);
                                if (allocated_path == NULL)
                                {
                                    rc = RC (rcFS, rcDirectory, ctx, rcMemory, rcExhausted);
                                    break; /* end for(;;) */
                                }
                                else
                                {
                                    char * pc;
                                    /* -----
                                     * TODO verify that there is or is not a problem
                                     * with  "~//~" showing up from these memcopies
                                     * and that a final NUL is added add extra buffer
                                     * space as needed to asize
                                     */
                                    memmove (allocated_path, abpath, fsize);
                                    pc = allocated_path + fsize;
                                    memmove (pc, symlink_path, ssize);
                                    pc += ssize;
                                    if (*(pc-1) == '/')
                                        --pc;
                                    memmove (pc, left, lsize+1);
                                    temp_path = allocated_path;
                                    continue;

                                } /* if (allocated_path == NULL) {} else { */
                            } /* if (symlink_path[0] == '/') {} else { */
                        } /* if (rc != 0) {} else { */
                    } /* if ((lsize == 0) && (! follow)) {} else { */
                } /* if (rc != 0) {} else if (type != ktocentrytype_softlink) {} else { */
            } /* if (strncmp(abpath, self->path, self->size) != 0) {} else { */
	    } /* if (rc != 0) {} else { */
	} /* while(rc == 0) */
	if (abpath != NULL)
	    free (abpath);
	if (allocated_path != NULL)
	    free (allocated_path);
    } /* if (path[0] == 0) {} else { */
    return rc;
}


/* ======================================================================
 * KArcFile
 *  a file inside an archive
 */

/*-----------------------------------------------------------------------
 * KArcFile
 *  an archive file including tar and sra
 */
struct KArcFile
{
    KFile		dad;
    const KToc *	toc;	/* not accessed directly but it owns the node so need a ref to it */
    const KTocEntry *	node;
    const KFile *	archive;
};

/*-----------------------------------------------------------------------
 * KArcFileDestroy
 *
 * Destructor for objects of class KArcFile.  Usually not called directly but instead called
 * when the reference count for the object drops to 0.
 *
 * [RET] rc_t					0 for success; anything else for a failure
 *						see itf/klib/rc.h for general details
 * [IN] KArcFile *		self		OOP self pointer - the object to be destroyed
 */
static
rc_t CC KArcFileDestroy (KArcFile *self)
{
    rc_t ret1, ret2;

    assert (self != NULL);

    ret1 = KFileRelease (self->archive);
    ret2 = KTocRelease (self->toc);
    free (self);

    /* -----
     * this seems a tad silly but pass along one or the other failure
     * We chose to call the second release even of the first failed
     * erring on the side of releasing as much as possible
     */
    return (ret1 != 0) ? ret1 : ret2;
}


/*-----------------------------------------------------------------------
 * KArcFileGetSysFile
 *
 *  returns an underlying system file object
 *  and starting offset to contiguous region
 *  suitable for memory mapping, or NULL if
 *  no such file is available.
 * Returns the KSysFile/KFile associated with a KArcFile.  NULL if not appropriate.
 *
 * [RET] struct KSysFile *
 * [IN]  const KArcFile *	self		Object oriented C; KArcFile object for this method
 * [OUT] uint64_t *		offset		An offset into the KSysfile where the KArcFile's
 *						data starts.
 */
static
struct KSysFile *CC KArcFileGetSysFile (const KArcFile *self, uint64_t *offset)
{
    rc_t rc;
    KTocEntryType type;
    struct KSysFile * fp;
    uint64_t fo;
    uint64_t ao;

    /* parameters must be non-NULL */
    assert (self != NULL);
    assert (offset != NULL);

    /* check the type which must be a contiguous file */
    rc = KTocEntryGetType (self->node, &type);
    if (rc == 0)
    {
	if (type == ktocentrytype_file)
	{
	    /* recur into the KFile to see if it allows this */
	    fp = KFileGetSysFile (self->archive, &ao);

	    /* -----
	     * if all this is true get the offset from the TOC entry and
	     * return it
	     */
	    if (fp != NULL)
	    {
		if (KTocEntryGetFileOffset (self->node, &fo) == 0)
		{
		    *offset = ao + fo;
		    return fp;
		}
	    }
	}
    }
    /* any failure alng the way leads to returning NULL */
    *offset = 0;
    return NULL;
}


/*-----------------------------------------------------------------------
 * KArcFileRandomAccess
 *
 * Returns zero if Random access is allowed for this KFile
 *
 * [RET] rc_t					0 for success; anything else for a failure
 *						see itf/klib/rc.h for general details		
 * [IN] const KArcFile *	self		Object oriented C; KArcFile object for this method
 */
static
rc_t CC KArcFileRandomAccess (const KArcFile *self)
{
    assert (self != NULL);
    return 0;
}


/*-----------------------------------------------------------------------
 * KArcFileType
 *  returns a KFileDesc
 *  not intended to be a content type,
 *  but rather an implementation class
 */
static
uint32_t CC KArcFileType ( const KArcFile *self )
{
    return KFileType ( self -> archive );
}


/*-----------------------------------------------------------------------
 * KArcFileSize
 *
 * [RET] rc_t					0 for success; anything else for a failure
 *						see itf/klib/rc.h for general details
 * [IN]  const KArcFile *	self		Object oriented C; KArcFile object for this method
 * [OUT] uint64_t *		size		Where to put the virtual size of the file
 */
static
rc_t CC KArcFileSize (const KArcFile *self, uint64_t *size)
{
    rc_t	rc;
    uint64_t	fsize;

    assert (self != NULL);
    assert (size != NULL);

    rc = KTocEntryGetFileSize(self->node, &fsize);
    if (rc == 0)
    {
	/* success */
	*size = fsize;
    }
    /* pass along RC value */
    return rc;
}


/*-----------------------------------------------------------------------
 * KArcFileSetSize
 *
 * Change the size of the KFile.  Not supported for files inside archives.
 *
 * [RET] rc_t					0 for success; anything else for a failure
 *						see itf/klib/rc.h for general details
 * [IN] KArcFile *		self		Object oriented C; KArcFile object for this method
 * [IN] uint64_t		size
 */
static
rc_t CC KArcFileSetSize (KArcFile *self, uint64_t size)
{
    return RC (rcFS, rcFile, rcUpdating, rcArc, rcUnsupported);
}



/*-----------------------------------------------------------------------
 * KArcFileRead
 *
 * Read bytes from a file represented by this KArcFile
 *
 * [RET] rc_t					0 for success; anything else for a failure
 *						see itf/klib/rc.h for general details
 * [IN]  const KArcFile *	self		Object oriented C; KArcFile object for this method
 * [IN]  uint64_t		pos		Offset with in the file from where to start reading
 * [OUT] void *			buffer		buffer to which to write the read bytes 
 * [IN]  size_t			bsize		how many bytes to read
 * [OUT] size_t *		num_read	how many bytes actually read. Will get written even
 *						in failure
 */
/* ----------------------------------------------------------------------
 * KArcFileReadContiguous
 *
 * Read data from a contiguously stored file at a specified position
 *
 * The parameters are the same as KArcFileRead.  This private function could be
 * inlined.  It is broken out for human readabilty.
 *
 * We just fix the offset by adding the offset to the file within the archive
 * (we fixed the number to read before we got here) and pass the read to the
 * archive.
 */
static
rc_t KArcFileReadContiguous ( const KArcFile * self, uint64_t pos,
                              void * buffer, size_t bsize, size_t * num_read )
{
    rc_t	rc;
    uint64_t	offset;

    assert ( self != NULL );
    assert ( buffer != NULL );
    assert ( num_read != NULL );
    assert ( bsize != 0 );

    rc = KTocEntryGetFileOffset ( self->node, &offset );
    if ( rc == 0 )
    {
        rc = KFileRead ( self->archive, pos + offset, buffer, bsize, num_read );
    }
    return rc;
}

/* ----------------------------------------------------------------------
 * KArcFileReadEmpty
 *
 * Read data from a file with no content.
 *
 * The parameters are the same as KArcFileRead.  This private function could be
 * inlined.  It is broken out for human readabilty.
 *
 * We just fix the offset by adding the offset to the file within the archive
 * (we fixed the number to read before we got here) and pass the read to the
 * archive.
 */
static
rc_t KArcFileReadEmpty (const KArcFile *self, uint64_t pos,
                        void *buffer, size_t bsize, size_t *num_read)
{
    assert (self != NULL);
    assert (buffer != NULL);
    assert (num_read != NULL);
    assert (bsize != 0);

    *num_read = 0;
    return 0;
}

/* ----------------------------------------------------------------------
 * KArcFileReadChunked
 *
 * Read data from a  chunked file at a specified position
 *
 * The parameters are the same as KArcFileRead.  This private function could be
 * inlined.  It is broken out for human readabilty.
 */
static
rc_t KArcFileReadChunked (const KArcFile *self,
			  uint64_t pos,
			  void *buffer,
			  size_t bsize,
			  size_t *num_read)
{
    const KTocChunk *	pchunk;		/* pointer to the chunk table */
    size_t		count;		/* how many to read/write in an action */
    uint32_t		num_chunks;	/* how many chunks in the array */
    rc_t		rc;		/* general purpose return from calls and pass along */

    assert (self != NULL);
    assert (buffer != NULL);
    assert (num_read != NULL);
    assert (bsize != 0);

    /* -----
     * assume no read/write will happen or rather start with having read none;
     * this write could be superfluous but we need to prepare *num_read for += operations
     */
    *num_read = 0;

    /* -----
     * Get the count of chunks and a pointer to the array of chunks for this file
     */
    rc = KTocEntryGetChunks (self->node, &num_chunks, &pchunk);
    if (rc == 0)
    {
	uint8_t *	pbuff;		/* access the buffer as an array of bytes */
	uint64_t	end;		/* this will be set to the end offset */

	pbuff = buffer;
	end = pos + bsize;	

	/* -----
	 * step through the chunks
	 */
	for (; (num_chunks) && (pos < end); --num_chunks, ++pchunk)
	{
	    uint64_t 	cend;		/* end offset of this chunk */

	    /* -----
	     * determine the end of this chunk
	     */
	    cend = pchunk->logical_position + pchunk->size;

	    /* -----
	     * if this chunk is entirely before the current position
	     * we are looking for
	     * skip to the next (if any) chunk
	     */
	    if (pos > cend)
		continue;

	    /* -----
	     * handle any needed zero fill section before the next chunk
	     */
	    if (pos < pchunk->logical_position)
	    {
		/* -----
		 * try to fake-read as many bytes of zero as possible
		 * so start assuming you need enough zeros to reach the next chunk
		 * but cut it back to the remaining requested if that was too many
		 */
		count = (size_t)( pchunk->logical_position - pos );
		if (count > bsize)
		    count = bsize;

		/* fake read the zeros */
		memset (pbuff, 0, count);

		/* update tracking variables */
		pbuff += count;
		pos += count;
		*num_read += count;
	    }

	    /* -----
	     * handle a chunk section
	     *
	     * if we are here, then we still have bytes to get and
	     * pos >= pchunk_logical_position
	     *
	     * Get the most we can from this chunk.
	     * If there are enough bytes in this chunk to finish the read: do so.
	     * Else read through the end of the chunk
	     */
	    count = (size_t)( (end <= cend) ? end - pos : cend - pos );

	    /* -----
	     * a little tricky is we call by value the wanted count and the function
	     * called will over write that with the actual read count
	     */
	    rc = KFileRead (self->archive, 
			    pchunk->source_position + (pchunk->logical_position - pos), 
			    pbuff, count, &count);

	    *num_read += count;
	    if (rc != 0)
	    {
		/* failure so abort */
		break;
	    }
	    pbuff += count;
	    pos += count;
	    *num_read += count;
	}
	/* -----
	 * If eveything so far is okay but we have more bytes to read
	 * yet no more chunks; then fill to the end with zeroes
	 */
	if ((rc == 0) && (pos < end))
	{
	    count = (size_t)( end - pos );
	    memset (pbuff, 0, count);
	    *num_read += count;
	}
    }
    return rc;
}
/* ----------------------------------------------------------------------
 * Read
 *  read file from known position
 *
 *  "pos" [ IN ] - starting position within file
 *
 *  "buffer" [ OUT ] and "bsize" [ IN ] - return buffer for read
 *
 *  "num_read" [ OUT, NULL OKAY ] - optional return parameter
 *  giving number of bytes actually read
 */
static
rc_t CC KArcFileRead	(const KArcFile *self,
			 uint64_t pos,
			 void *buffer,
			 size_t bsize,
			 size_t *num_read)
{
    KTocEntryType	type;
    rc_t		rc;

    /* -----
     * self and buffer were validated as not NULL before calling here
     *
     * So get the KTocEntry type: chunked files and contiguous files 
     * are read differently.
     */
    assert (self != NULL);
    assert (buffer != NULL);
    assert (num_read != NULL);
    assert (bsize != 0);

    rc = KTocEntryGetType(self->node, &type);

    assert ((type == ktocentrytype_file) || 
            (type == ktocentrytype_chunked) ||
            (type == ktocentrytype_emptyfile));

    if (rc == 0)
    {
	uint64_t	size;

	/* -----
	 * We have to validate the size to be read and will modify the number
	 * down if necessary
	 */
	rc = KTocEntryGetFileSize (self->node, &size);
	if (rc == 0)
	{
	    /* ----- 
	     * if we are seeking beyond the end match sysfile.c's use of pread
	     * and return number read as 0 to mark EOF
	     */
	    if (pos >= size)
	    {
		*num_read = 0;
	    }
	    else
	    {
		uint64_t	limit;

		limit = 
		    (pos + bsize > size)	/* if attempt to read beyond end of file */
		    ? size - pos		/* then force smaller read */
		    : bsize;			/* else allow full read */

		switch (type)
		{
		default:
		case ktocentrytype_unknown:
		case ktocentrytype_dir:
		case ktocentrytype_softlink:
		case ktocentrytype_hardlink:
		    /* -----
		     * We should not have been able to open these as a KArcFile
		     * so this is probably superfluous
		     */
		    rc = RC (rcFS, rcFile, rcReading, rcArc, rcUnexpected);
		    break;
		case ktocentrytype_file:
		    rc = KArcFileReadContiguous (self, pos, buffer, (size_t)limit, num_read);
		    break;
		case ktocentrytype_chunked:
		    rc = KArcFileReadChunked (self, pos, buffer, (size_t)limit, num_read);
		    break;
		case ktocentrytype_emptyfile:
		    rc = KArcFileReadEmpty (self, pos, buffer, (size_t)limit, num_read);
		    break;
		}
	    }
	}
    }
    return rc;
}



/*-----------------------------------------------------------------------
 * KArcFileWrite
 *
 * Write bytes to a file represented by this KArcFile.
 * Fails as unsupported for files inside archives.
 *
 * [RET] rc_t					0 for success; anything else for a failure
 *						see itf/klib/rc.h for general details
 * [IN]  KArcFile *		self		Object oriented C; KArcFile object for this method
 * [IN]  uint64_t 		pos		Offset within file to start writing (ignored)
 * [IN]  const void *		buffer		pointer to data to write (ignored)
 * [IN]  size_t 		size		how many bytes to write (ignored)
 * [OUT] size_t *		num_writ	how many bytes written - always set to 0
 */
static
rc_t CC KArcFileWrite (KArcFile *self, uint64_t pos,
		       const void *buffer, size_t size,
		       size_t *num_writ)
{
    assert (num_writ != NULL);

    /* duplicates write in KFileWrite() so this can be deleted */
    *num_writ = 0;

    return RC (rcFS, rcArc, rcWriting, rcSelf, rcUnsupported);
}



static	KFile_vt_v1	vtKArcFile =
{
    /* version */
    1, 1,

    /* 1.0 */
    KArcFileDestroy,
    KArcFileGetSysFile,
    KArcFileRandomAccess,
    KArcFileSize,
    KArcFileSetSize,
    KArcFileRead,
    KArcFileWrite,

    /* 1.1 */
    KArcFileType
};

/*-----------------------------------------------------------------------
 * KArcFileMake
 *
 * [RET] rc_t					0 for success; anything else for a failure
 *						see itf/klib/rc.h for general details
 * [OUT] KArcFile ** 		self		were to put a reference to the newly allocated
 *						KArcFile structure
 * [IN]  const KFile *		archive		The KFile for the archive containing this archive
 * [IN]  const KArcToc *	toc		The controlling TOC for the Archive File
 * [IN]  const KTocEntry *	node		The specific node somewhere in that TOC for this file
 */
static
rc_t KArcFileMake (KArcFile ** self,
		   const KFile * archive,
		   const KToc * toc,
		   const KTocEntry * node)
{
    rc_t	rc;
    KArcFile *	pF;
    uint64_t    size;

    /* -----
     * we can not accept any of the four parameters as NULL
     */
    assert (self != NULL);
    assert (archive != NULL);
    assert (toc != NULL);
    assert (node != NULL);

    /* -----
     * Proceed with non-NULL parameters
     */
    rc = KFileSize (archive, &size);
    if (GetRCState(rc) == rcUnsupported)
    {
        size = ~(uint64_t)0;
        rc = 0;
    }

    if (rc == 0)
    {
        /* we need to check chunked files here as well */
        if (((node->type == ktocentrytype_file) &&
             (node->u.contiguous_file.file_size > 0) &&
             (size < (node->u.contiguous_file.file_size +
                      node->u.contiguous_file.archive_offset))) ||
            ((node->type == ktocentrytype_chunked) &&
             (node->u.chunked_file.file_size > 0) &&
             (size < (node->u.chunked_file.chunks[node->u.chunked_file.num_chunks-1].source_position +
                      node->u.chunked_file.chunks[node->u.chunked_file.num_chunks-1].size))))
            rc = RC (rcFS, rcFile, rcConstructing, rcArc, rcIncomplete);
        else
        {
            /* get space for the object */
            pF = malloc (sizeof * pF);
            if (pF == NULL)	/* allocation failed */
            {
                /* fail */
                rc = RC (rcFS, rcFile, rcConstructing, rcMemory, rcExhausted);
            }
            else
            {
                rc = KFileInit (&pF->dad,				/* initialize base class */
                                (const KFile_vt*)&vtKArcFile,	/* VTable for KArcFile */
                                "KArcFile", "no-name",
                                true,				/* read allowed */
                                false);				/* write disallowed */
                if (rc == 0)
                {
                    /* succeed */
                    pF->toc = toc;					/* shared TOC */
                    KTocAddRef(toc);				/* keep alive reference */
                    pF->node = node;				/* file specific TOC entry */
                    pF->archive = archive;				/* shared archive file */
                    KFileAddRef(archive);				/* keep alive reference */
                    *self = pF;					/* and to the output */
                }
                if (rc != 0)
                    /* fail */
                    free (pF);
            }
	}
    }
    return rc;
}


/* ----------------------------------------------------------------------
 * KArcDirVisit
 *  visit each path under designated directory,
 *  recursively if so indicated
 *
 *  "recurse" [ IN ] - if non-zero, recursively visit sub-directories
 *
 *  "f" [ IN ] and "data" [ IN, OPAQUE ] - function to execute
 *  on each path. receives a base directory and relative path
 *  for each entry, where each path is also given the leaf name
 *  for convenience. if "f" returns non-zero, the iteration will
 *  terminate and that value will be returned. NB - "dir" will not
 *  be the same as "self".
 *
 *  "path" [ IN ] - NUL terminated string in directory-native character set
 */
static 
rc_t CC KArcDirVisit (const KArcDir *self, 
                      bool recurse,
                      rc_t (CC* f) (const KDirectory *, uint32_t, const char *, void *), 
                      void *data,
                      const char *path,
                      va_list args)
{
    char * full_path;
    rc_t   rc;


    /* -----
     * First fix the path to make it useable
     */
    rc = KArcDirMakePath (self, rcVisiting, true, &full_path, path, args);
    if (rc != 0)
    {
	LOGERR (klogInt, rc, "failed to make path in Visit");
    }
    else
    {
	const KTocEntry * pnode;
	KTocEntryType	type;

	/* -----
	 * Now find that path as a node and validate it is a directory
	 */
	rc = KArcDirResolvePathNode(self, rcListing, full_path, true, &pnode, &type);
	if (rc != 0)
	{
	    PLOGERR (klogInt, (klogInt, rc, "failed to resolve path $(P) in Visit", "P=%s", full_path));
	}
	else
	{
	    if ((type == ktocentrytype_dir) || (type == ktocentrytype_hardlink))
	    {
		KArcDir * full_dir;
		uint32_t path_size;

		/* -----
		 * make a locally accessible private KDirectory/KArcDir
		 */
		for ( path_size = (uint32_t)strlen ( full_path );
		      ( path_size > self->root ) && ( full_path[ path_size - 1 ] == '/' );
		      -- path_size )
		{}
		rc = KArcDirMake (&full_dir, 
				  rcVisiting,
				  self->parent,
				  self->toc,
				  pnode,
				  self->archive.v,
				  self->arctype,
				  self->root,
				  full_path,
				  path_size, 
				  true,
				  false);
		if (rc == 0)
		{
		    KArcDirVisitData pb;

		    pb.f = f;
		    pb.data = data;
		    pb.dir = full_dir;
		    pb.recurse = recurse;
/*		    pb.dir.path[--pb.dir.size] = 0; */

		    rc = KArcDirVisitDir (&pb);

		    KArcDirDestroy (full_dir);
		}
	    }
	    else
	    {
		rc = RC (rcFS, rcDirectory, rcVisiting, rcPath, rcIncorrect);
		LOGERR (klogInt, rc, "Type is not a directory");
	    }
	}
	free (full_path);
    }
    return rc;
}

/* ----------------------------------------------------------------------
 * KArcDirVisitUpdate
 */
static rc_t CC KArcDirVisitUpdate (KArcDir *self,
                                   bool recurse,
                                   rc_t (CC*f) (KDirectory *,uint32_t,const char *,void *),
                                   void *data,
                                   const char *path,
                                   va_list args)
{
    return RC (rcFS, rcDirectory, rcUpdating, rcSelf, rcUnsupported);
}

/* ----------------------------------------------------------------------
 * KArcDirResolvePath
 *
 *  resolves path to an absolute or directory-relative path
 *
 * [IN]  const KArcDir *self		Objected oriented self
 * [IN]	 bool 		absolute	if non-zero, always give a path starting
 *  					with '/'. NB - if the directory is 
 *					chroot'd, the absolute path
 *					will still be relative to directory root.
 * [OUT] char *		resolved	buffer for NUL terminated result path in 
 *					directory-native character set
 * [IN]	 size_t		rsize		limiting size of resolved buffer
 * [IN]  const char *	path		NUL terminated string in directory-native
 *					character set denoting target path. 
 *					NB - need not exist.
 *
 * NOTE: Does not meet a design target of on stack (localized variable) allocation of single 4kb path
 */
static rc_t CC KArcDirResolvePath (const KArcDir *self,
                                   bool absolute,
                                   char *resolved,
                                   size_t rsize,
                                   const char *path,
                                   va_list args)
{
    char * full;
    rc_t   rc; 

    assert (self != NULL);
    assert (resolved != NULL);
    assert (path != NULL);

    rc = KArcDirMakePath (self, rcResolving, true, &full, path, args);

    if (rc == 0)
    {
        uint32_t path_size = (uint32_t)strlen ( full );

        if (absolute)
        {
            /* test buffer capacity -  this is a limitation of KDirectory not in KArcDir */
            if ((path_size - self->root) >= rsize)
            {
                rc =  RC (rcFS, rcDirectory, rcResolving, rcBuffer, rcInsufficient);
            }
            else
            {
                /* ready to go */
                strcpy (resolved, & full[self->root]);
                assert (resolved[0] == '/');
            }
        }
        else
        {
            rc = KArcDirRelativePath (self, rcResolving, self->path, full, path_size);
            if (rc == 0)
            {
                path_size = (uint32_t)strlen ( full );
                /* test buffer capacity -  this is a limitation of KDirectory not in KArcDir */
                if (path_size >= rsize)
                {
                    rc = RC (rcFS, rcDirectory, rcResolving, rcBuffer, rcInsufficient);
                }
                else
                {
                    strcpy (resolved, full);
                }
            }
        }
    }
    if (full != NULL)
        free (full);

    return rc;
}

/* ----------------------------------------------------------------------
 * KArcDirResolveAlias
 *  resolves an alias path to its immediate target
 *  NB - the resolved path may be yet another alias
 *
 *  "alias" [ IN ] - NUL terminated string in directory-native
 *  character set denoting an object presumed to be an alias.
 *
 *  "resolved" [ OUT ] and "rsize" [ IN ] - buffer for
 *  NUL terminated result path in directory-native character set
 *
 * NOTE: Does not meet a design target of on stack (localized variable) allocation of single 4kb path
 */
static rc_t CC KArcDirResolveAlias (const KArcDir * self, 
				 bool absolute,
				 char * resolved,
				 size_t rsize,
				 const char *alias_fmt,
				 va_list args)
{
    char *		full_path;
    const char *	link_path;
    const KTocEntry * pnode;
    KTocEntryType	type;
    rc_t		rc;
    size_t		ssize;

    assert (self != NULL);
    assert (resolved != NULL);
    assert (alias_fmt != NULL);

    rc = KArcDirMakePath (self, rcResolving, true,
			  &full_path, alias_fmt, args);

    if (rc != 0)
    {
    	/* can't "fix" path */
	    /*rc = RC (rcFS, rcDirectory, rcResolving, rcPath, rcInvalid); ? or tweak it? */
    }
    else
    {
    	/* first find the node and it has to be an alias */ 
        char alias[4096];
        int size = ( args == NULL ) ?
            snprintf  ( alias, sizeof alias, "%s", alias_fmt ) : 
            vsnprintf ( alias, sizeof alias, alias_fmt, args );

        if ( size < 0 || size >= ( int ) sizeof alias )
            rc = RC ( rcFS, rcDirectory, rcResolving, rcPath, rcExcessive );
        else
        	rc = KArcDirResolvePathNode (self, rcResolving, alias, false, &pnode, &type);
    
    	if (rc != 0)
    	{
    	    /*rc = RC (rcFS, rcDirectory, rcResolving, rcPath, rcInvalid); ? or tweak it? */
    	}
    	else if (type != ktocentrytype_softlink)
    	{
    	    rc = RC (rcFS, rcDirectory, rcResolving, rcLink, rcInvalid);
    	}
    	else
    	{
    	    rc = KTocEntryGetSoftTarget(pnode, &link_path);
    	    if (rc != 0)
    	    {
        		/* can't "fix" path */
        		/*rc = RC (rcFS, rcDirectory, rcResolving, rcPath, rcInvalid); ? or tweak it? */
    	    }
    	    else
    	    {
    	    	ssize = strlen(link_path);
        		if (ssize > rsize)
        		{
    	    	    rc = RC (rcFS, rcDirectory,rcResolving, rcParam, rcInsufficient);
    		    }
        		else
        		{
    	    	    strcpy (resolved, link_path);
    		    }
    	    }
    	}
    }
    return rc;
}

/* ----------------------------------------------------------------------
 * KArcDirRename
 *  rename an object accessible from directory, replacing
 *  any existing target object of the same type
 *
 *  "from" [ IN ] - NUL terminated string in directory-native
 *  character set denoting existing object
 *
 *  "to" [ IN ] - NUL terminated string in directory-native
 *  character set denoting existing object
 */
static
rc_t CC KArcDirRename (KArcDir *self, bool force, const char *from, const char *to)
{
    assert (self != NULL);
    assert (from != NULL);
    assert (to != NULL);

    return RC (rcFS, rcArc, rcUpdating, rcSelf, rcUnsupported);
}

/* ----------------------------------------------------------------------
 * KArcDirRemove
 *  remove an accessible object from its directory
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target object
 *
 *  "force" [ IN ] - if non-zero and target is a directory,
 *  remove recursively
 */
static
rc_t CC KArcDirRemove (KArcDir *self, bool force, const char *path, va_list args)
{
    assert (self != NULL);
    assert (path != NULL);

    return RC (rcFS, rcArc, rcUpdating, rcSelf, rcUnsupported);
}

/* ----------------------------------------------------------------------
 * KArcDirClearDir
 *  remove all directory contents
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target directory
 *
 *  "force" [ IN ] - if non-zero and directory entry is a
 *  sub-directory, remove recursively
 */
static
rc_t CC KArcDirClearDir (KArcDir *self, bool force, const char *path, va_list args)
{
    assert (self != NULL);
    assert (path != NULL);

    return RC (rcFS, rcArc, rcUpdating, rcSelf, rcUnsupported);
}

/* ----------------------------------------------------------------------
 * KArcDirAccess
 *  get access to object
 *
 *  "access" [ OUT ] - return parameter for Unix access mode
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target object
 * NOTE: Does not meet a design target of on stack (localized variable) allocation of single 4kb path
 */
static rc_t CC KArcDirVAccess (const KArcDir *self,
			    uint32_t *access,
			    const char *path_fmt,
			    va_list args)
{
    rc_t rc;
    uint32_t acc;
    KTocEntryType type;
    char * full;
    const KTocEntry * entry;

    va_list args_copy;

    assert (self != NULL);
    assert (access != NULL);
    assert (path_fmt != NULL);

    /* MUST copy "args" if the intention is to use it twice */
    if ( args != NULL )
        va_copy ( args_copy, args );

    /* -----
     * by C standard the nested ifs (if A { if B { if C ... could have been if A && B && C
     */
    if ((rc = KArcDirMakePath (self, rcAccessing, false, &full, path_fmt, args)) == 0)
    {
        char path [ 4096 ];
        int size = ( args == NULL ) ?
            snprintf  ( path, sizeof path, "%s", path_fmt ) :
            vsnprintf ( path, sizeof path, path_fmt, args_copy );

        if ( size < 0 || size >= ( int ) sizeof path )
            rc = RC ( rcFS, rcDirectory, rcAccessing, rcPath, rcExcessive );
        else
        {
        	if ((rc = KArcDirResolvePathNode (self, rcAccessing, path, true, &entry, &type)) == 0)
        	{
        	    if ((rc = KTocEntryGetAccess (entry, &acc)) == 0)
        	    {
                    /*
                     * We want to filter the access because within an Archive
                     * a file is unwritable
                     */
            		*access = acc & ~(S_IWRITE|S_IWGRP|S_IWOTH);
            		rc = 0;
        	    }
        	}
        }
    }

    if ( args != NULL )
        va_end ( args_copy );

    if (full != NULL)
        free (full);
    return rc;
}

/* ----------------------------------------------------------------------
 * KArcDirSetAccess
 *  set access to object a la Unix "chmod"
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target object
 *
 *  "access" [ IN ] and "mask" [ IN ] - definition of change
 *  where "access" contains new bit values and "mask defines
 *  which bits should be changed.
 *
 *  "recurse" [ IN ] - if non zero and "path" is a directory,
 *  apply changes recursively.
 */
static rc_t CC KArcDirSetAccess (KArcDir *self,
			      bool recurse,
			      uint32_t access,
			      uint32_t mask,
			      const char *path,
			      va_list args)
{
    assert (self != NULL);
    assert (path != NULL);

    return RC (rcFS, rcArc, rcUpdating, rcSelf, rcUnsupported);
}


static	rc_t CC KArcDirVDate		(const KArcDir *self,
					 KTime_t *date,
					 const char *path_fmt,
					 va_list args)
{
/*     const KToc *	toc; */
    rc_t 		rc;
    KTime_t		ldate;
    KTocEntryType	type;
    char * 		full;
    const KTocEntry * node;

    va_list args_copy;

    assert (self != NULL);
    assert (date != NULL);
    assert (path_fmt != NULL);

    /* MUST copy "args" if the intention is to use it twice */
    if ( args != NULL )
        va_copy ( args_copy, args );

    /* -----
     * by C standard the nested ifs (if A { if B { if C ... could have been if A && B && C
     */
    if ((rc = KArcDirMakePath (self, rcAccessing, false, &full, path_fmt, args)) == 0)
    {
#if 0
        if ((rc = KArcDirGetTOC (self, &toc)) == 0)
#endif
        {
            /* THIS IS INCORRECT - IT SHOULD NOT RESOLVE ALIASES */
            char path [ 4096 ];
            int size = vsnprintf ( path, sizeof path, path_fmt, args_copy );

            if ( size < 0 || size >= ( int ) sizeof path )
                rc = RC ( rcFS, rcDirectory, rcAccessing, rcPath, rcExcessive );
            else
            {
                if ((rc = KArcDirResolvePathNode (self, rcAccessing, path, true, &node, &type)) == 0)
                {
                    if ((rc = KTocEntryGetTime (node, &ldate)) == 0)
                    {
                        *date =  ldate;
                        rc = 0;
                    }
                }
            }
        }
    }

    if ( args != NULL )
        va_end ( args_copy );

    if (full != NULL)
        free (full);
    return rc;
}

static	rc_t CC KArcDirSetDate		(KArcDir *self,
					 bool recurse,
					 KTime_t date,
					 const char *path,
					 va_list args)
{
    assert (self != NULL);
    assert (path != NULL);

    return RC (rcFS, rcArc, rcUpdating, rcSelf, rcUnsupported);
}

static
struct KSysDir *CC KArcDirGetSysDir ( const KArcDir *self )
{
    return NULL;
}

/* ----------------------------------------------------------------------
 * KArcDirCreateAlias
 *  creates a path alias according to create mode
 *
 *  "targ" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target object
 *
 *  "alias" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target alias
 *
 *  "access" [ IN ] - standard Unix directory access mode
 *  used when "mode" has kcmParents set and alias path does
 *  not exist.
 *
 *  "mode" [ IN ] - a creation mode (see explanation above).
 */
static
rc_t CC KArcDirCreateAlias (KArcDir *self,
			 uint32_t access,
			 KCreateMode mode,
			 const char *targ,
			 const char *alias)
{
    assert (self != NULL);
    assert (targ != NULL);
    assert (alias != NULL);

    return RC (rcFS, rcArc, rcCreating, rcSelf, rcUnsupported);
}

/* ----------------------------------------------------------------------
 * KArcDirOpenFileRead
 *  opens an existing file with read-only access
 *
 *  "f" [ OUT ] - return parameter for newly opened file
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target file
 * NOTE: Does not meet a design target of on stack (localized variable) allocation of single 4kb path
 */
static
rc_t CC KArcDirOpenFileRead	(const KArcDir *self,
					 const KFile **f,
					 const char *path,
					 va_list args)
{
    char *	full_path = NULL;
    rc_t	rc;

    assert (self != NULL);
    assert (f != NULL);
    assert (path != NULL);

    rc = KArcDirMakePath (self, rcOpening, true, &full_path, path, args);

    if (rc == 0)
    {
	const KTocEntry * pnode;
	KTocEntryType     type;

	rc = KArcDirResolvePathNode (self, rcOpening, full_path, true, &pnode, &type);

	if (rc == 0)
	{

	    switch (type)
	    {
	    case ktocentrytype_unknown:
	    case ktocentrytype_dir:
	    case ktocentrytype_softlink:
	    case ktocentrytype_hardlink:
	    default:
		rc = RC (rcFS, rcDirectory, rcOpening, rcFile, rcInvalid);
		break;
	    case ktocentrytype_emptyfile:
	    case ktocentrytype_file:
	    case ktocentrytype_chunked:
		rc = KArcFileMake ((KArcFile**)f, self->archive.v, self->toc, pnode);
		break;
	    }
	}
	free (full_path);
    }    
    return rc;
}

/* ----------------------------------------------------------------------
 * KArcDirOpenFileWrite
 *  opens an existing file with write access
 *
 *  "f" [ OUT ] - return parameter for newly opened file
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target file
 *
 *  "update" [ IN ] - if non-zero, open in read/write mode
 *  otherwise, open in write-only mode
 */
static
rc_t CC KArcDirOpenFileWrite	(KArcDir *self,
					 KFile **f,
					 bool update,
					 const char *path,
					 va_list args)
{
    assert (self != NULL);
    assert (f != NULL);
    assert (path != NULL);

    return RC (rcFS, rcArc, rcCreating, rcSelf, rcUnsupported);
}

/* ----------------------------------------------------------------------
 * KArcDirCreateFile
 *  opens a file with write access
 *
 *  "f" [ OUT ] - return parameter for newly opened file
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target file
 *
 *  "access" [ IN ] - standard Unix access mode, e.g. 0664
 *
 *  "update" [ IN ] - if non-zero, open in read/write mode
 *  otherwise, open in write-only mode
 *
 *  "mode" [ IN ] - a creation mode (see explanation above).
 */
static
rc_t CC KArcDirCreateFile	(KArcDir *self,
					 KFile **f,
					 bool update,
					 uint32_t access,
					 KCreateMode cmode,
					 const char *path,
					 va_list args)
{
    assert (self != NULL);
    assert (f != NULL);
    assert (path != NULL);

    return RC (rcFS, rcArc, rcCreating, rcSelf, rcUnsupported);
}

/* ----------------------------------------------------------------------
 * KArcDirFileLocator
 *  returns locator in bytes of target file
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target file
 *
 *  "locator" [ OUT ] - return parameter for file locator
 * NOTE: Does not meet a design target of on stack (localized variable) allocation of single 4kb path
 */
static
rc_t CC KArcDirFileLocator		(const KArcDir *self,
					 uint64_t *locator,
					 const char *path,
					 va_list args)
{
    char *		full_path;
    rc_t		rc;

    assert (self != NULL);
    assert (locator != NULL);
    assert (path != NULL);

    rc = KArcDirMakePath (self, rcResolving, true,
			  &full_path, path, args);

    if (rc != 0)
    {
	/* can't "fix" path */
	/*rc = RC (rcFS, rcDirectory, rcResolving, rcPath, rcInvalid); ? or tweak it? */
    }
    else
    {
	const KTocEntry * pnode;
	KTocEntryType     type;

	rc = KArcDirResolvePathNode (self, rcResolving, full_path, /*follow links*/true, &pnode, &type);

	if (rc != 0)
	{
	    /* can't resolve path */
	    /*rc = RC (rcFS, rcDirectory, rcResolving, rcPath, rcInvalid); ? or tweak it? */
	}
	else
	{
	    uint64_t	flocator;

	    rc = KTocEntryGetFileLocator(pnode, &flocator);
	    if (rc != 0)
	    {
		/* can't "fix" path */
		/*rc = RC (rcFS, rcDirectory, rcResolving, rcPath, rcInvalid); ? or tweak it? */
	    }
	    else
	    {
		*locator = flocator;
	    }
	}
	free (full_path);
    }
    return rc;
}

/* ----------------------------------------------------------------------
 * KArcDirFileSize
 *  returns size in bytes of target file
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target file
 *
 *  "size" [ OUT ] - return parameter for file size
 * NOTE: Does not meet a design target of on stack (localized variable) allocation of single 4kb path
 */
static
rc_t CC KArcDirFileSize		(const KArcDir *self,
					 uint64_t *size,
					 const char *path,
					 va_list args)
{
    char *		full_path;
    rc_t		rc;

    assert (self != NULL);
    assert (size != NULL);
    assert (path != NULL);

    rc = KArcDirMakePath (self, rcResolving, true,
			  &full_path, path, args);

    if (rc != 0)
    {
	/* can't "fix" path */
	/*rc = RC (rcFS, rcDirectory, rcResolving, rcPath, rcInvalid); ? or tweak it? */
    }
    else
    {
	const KTocEntry * pnode;
	KTocEntryType     type;

	rc = KArcDirResolvePathNode (self, rcResolving, full_path, /*follow links*/true, &pnode, &type);

	if (rc != 0)
	{
	    /* can't resolve path */
	    /*rc = RC (rcFS, rcDirectory, rcResolving, rcPath, rcInvalid); ? or tweak it? */
	}
	else
	{
	    uint64_t	fsize;

	    rc = KTocEntryGetFileSize(pnode, &fsize);
	    if (rc != 0)
	    {
		/* can't "fix" path */
		/*rc = RC (rcFS, rcDirectory, rcResolving, rcPath, rcInvalid); ? or tweak it? */
	    }
	    else
	    {
		*size = fsize;
	    }
	}
	free (full_path);
    }
    return rc;
}

/* ----------------------------------------------------------------------
 * KArcDirFileSize
 *  returns size in bytes of target file
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target file
 *
 *  "size" [ OUT ] - return parameter for file size
 * NOTE: Does not meet a design target of on stack (localized variable) allocation of single 4kb path
 */
static
rc_t CC KArcDirFilePhysicalSize		(const KArcDir *self,
					 uint64_t *size,
					 const char *path,
					 va_list args)
{
    char *		full_path;
    rc_t		rc;

    assert (self != NULL);
    assert (size != NULL);
    assert (path != NULL);

    rc = KArcDirMakePath (self, rcResolving, true,
			  &full_path, path, args);

    if (rc != 0)
    {
	/* can't "fix" path */
	/*rc = RC (rcFS, rcDirectory, rcResolving, rcPath, rcInvalid); ? or tweak it? */
    }
    else
    {
	const KTocEntry * pnode;
	KTocEntryType     type;

	rc = KArcDirResolvePathNode (self, rcResolving, full_path, /*follow links*/true, &pnode, &type);

	if (rc != 0)
	{
	    /* can't resolve path */
	    /*rc = RC (rcFS, rcDirectory, rcResolving, rcPath, rcInvalid); ? or tweak it? */
	}
	else
	{
	    uint64_t	fsize;

	    rc = KTocEntryGetFilePhysicalSize(pnode, &fsize);
	    if (rc != 0)
	    {
		/* can't "fix" path */
		/*rc = RC (rcFS, rcDirectory, rcResolving, rcPath, rcInvalid); ? or tweak it? */
	    }
	    else
	    {
		*size = fsize;
	    }
	}
	free (full_path);
    }
    return rc;
}

/* ----------------------------------------------------------------------
 * KArcDirSetFileSize
 *  sets size in bytes of target file
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target file
 *
 *  "size" [ IN ] - new file size
 */
static
rc_t CC KArcDirSetFileSize	(KArcDir *self,
					 uint64_t size,
					 const char *path,
					 va_list args)
{
    assert (self != NULL);
    assert (path != NULL);

    return RC (rcFS, rcArc, rcWriting, rcSelf, rcUnsupported);
}

/* ----------------------------------------------------------------------
 * KArcDirOpenDirRead
 *
 *  opens a sub-directory
 *
 * [IN]  const KArcDir *	self	Object Oriented C KArcDir self
 * [OUT] const KDirectory **	subp	Where to put the new KDirectory/KArcDir
 * [IN]  bool			chroot	Create a chroot cage for this new subdirectory
 * [IN]  const char *		path	Path to the directory to open
 * [IN]  va_list		args	So far the only use of args is possible additions to path
 */
static 
rc_t CC KArcDirOpenDirRead	(const KArcDir *self,
					 const KDirectory **subp,
					 bool chroot,
					 const char *path,
					 va_list args)
{
    char * full;
    rc_t rc;

    assert (self != NULL);
    assert (subp != NULL);
    assert (path != NULL);

    rc = KArcDirMakePath (self, rcOpening, true, &full, path, args);
    if (rc == 0)
    {
	const KTocEntry *	pnode;
	KTocEntryType		type;
	size_t path_size = strlen (full);

	/* -----
	 * get rid of any extra '/' characters at the end of path
	 */
	while (path_size > 0 && full [ path_size - 1 ] == '/')
	    full [ -- path_size ] = 0;

	/* -----
	 * get the node for this path 
	 */
	rc = KArcDirResolvePathNode (self, rcOpening, full, true, &pnode, &type);
	if (rc == 0)
	{
            switch (type)
            {
            default:
		/* fail */
		rc = RC (rcFS, rcDirectory, rcOpening, rcPath, rcIncorrect);
                break;
            case ktocentrytype_dir:
            case ktocentrytype_hardlink:
	    {
		KArcDir *	sub;

		rc = KArcDirMake (&sub,
				  rcOpening,
				  self->parent,
				  self->toc,
				  pnode,
				  self->archive.v,
				  self->arctype,
				  self->root,
				  full,
				  (uint32_t)path_size,
				  false,
				  chroot);
		if (rc == 0)
		{
		    /* succeed */
		    *subp = &sub->dad;
		}
	    }
            }
	}
	free (full);
    }
    return rc;
}

/* ----------------------------------------------------------------------
 * KArcDirOpenDirUpdate
 *  opens a sub-directory
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target directory
 *
 *  "chroot" [ IN ] - if non-zero, the new directory becomes
 *  chroot'd and will interpret paths beginning with '/'
 *  relative to itself.
 */
static
rc_t CC KArcDirOpenDirUpdate	(KArcDir *self,
					 KDirectory ** subp, 
					 bool chroot, 
					 const char *path, 
					 va_list args)
{
    assert (self != NULL);
    assert (subp != NULL);
    assert (path != NULL);

    return RC (rcFS, rcArc, rcUpdating, rcSelf, rcUnsupported);
}

/* ----------------------------------------------------------------------
 * KArcDirCreateDir
 *  create a sub-directory
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target directory
 *
 *  "access" [ IN ] - standard Unix directory permissions
 *
 *  "mode" [ IN ] - a creation mode (see explanation above).
 */
static
rc_t CC KArcDirCreateDir	(KArcDir *self,
					 uint32_t access,
					 KCreateMode mode,
					 const char *path,
					 va_list args)
{
    assert (self != NULL);
    assert (path != NULL);

    return RC (rcFS, rcArc, rcCreating, rcSelf, rcUnsupported);
}

/* ----------------------------------------------------------------------
 * KArcDirDestroyFile
 */
static
rc_t CC KArcDirDestroyFile	(KArcDir *self,
					 KFile * f)
{
    assert (self != NULL);
    assert (f != NULL);

    return RC (rcFS, rcArc, rcDestroying, rcSelf, rcUnsupported);
}

/* ----------------------------------------------------------------------
 * KArcDirFileContiguous
 *  
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target file
 *
 *  "contiguous" [ OUT ] - return parameter for file status
 * NOTE: Does not meet a design target of on stack (localized variable) allocation of single 4kb path
 */
static
rc_t CC KArcDirFileContiguous		(const KArcDir *self,
                                         bool * contiguous,
					 const char *path,
					 va_list args)
{
    char *		full_path;
    rc_t		rc;

    assert (self != NULL);
    assert (contiguous != NULL);
    assert (path != NULL);

    rc = KArcDirMakePath (self, rcResolving, true,
			  &full_path, path, args);

    if (rc != 0)
    {
	/* can't "fix" path */
	/*rc = RC (rcFS, rcDirectory, rcResolving, rcPath, rcInvalid); ? or tweak it? */
    }
    else
    {
	const KTocEntry * pnode;
	KTocEntryType     type;

	rc = KArcDirResolvePathNode (self, rcResolving, full_path, /*follow links*/true, &pnode, &type);

	if (rc != 0)
	{
	    /* can't resolve path */
	    /*rc = RC (rcFS, rcDirectory, rcResolving, rcPath, rcInvalid); ? or tweak it? */
	}
	else
	{
	    switch (type)
	    {
	    default:
                *contiguous = false;
		break;
	    case ktocentrytype_emptyfile:
	    case ktocentrytype_file:
                *contiguous = true;
		break;
	    }
	}
	free (full_path);
    }
    return rc;
}

/* ----------------------------------------------------------------------
 *
 */
static KDirectory_vt_v1 vtKArcDir =
{
    /* version 1.0 */
    1, 3,

    /* start minor version 0 methods*/
    KArcDirDestroy,
    KArcDirList,
    KArcDirVisit,
    KArcDirVisitUpdate,
    KArcDirPathType,
    KArcDirResolvePath,
    KArcDirResolveAlias,
    KArcDirRename,
    KArcDirRemove,
    KArcDirClearDir,
    KArcDirVAccess,
    KArcDirSetAccess,
    KArcDirCreateAlias,
    KArcDirOpenFileRead,
    KArcDirOpenFileWrite,
    KArcDirCreateFile,
    KArcDirFileSize,
    KArcDirSetFileSize,
    KArcDirOpenDirRead,
    KArcDirOpenDirUpdate,
    KArcDirCreateDir,
    KArcDirDestroyFile,
    /* end minor version 0 methods*/

    /* start minor version 1 methods*/
    KArcDirVDate,
    KArcDirSetDate,
    KArcDirGetSysDir,
    /* end minor version 1 methods*/

    /* start minor version 2 methods*/
    KArcDirFileLocator,
    /* end minor version 2 methods*/

    /* start minor version 3 methods*/
    KArcDirFilePhysicalSize,
    KArcDirFileContiguous
    /* end minor version 3 methods*/
};

/* ----------------------------------------------------------------------
 * KArcDirMake
 *
 * [RET] rc_t					0 for success; anything else for a failure
 *						see itf/klib/rc.h for general details
 * [IN] KArcDir **		self		objected oriented c "self"
 * [IN] enum RCContext 		ctx		context to use when passing along rc_t
 * [IN] const KDirectory * 	parent		KDirectory type for path before the archive
 * [IN] const KToc * 	toc		table of contents for the archive
 * [IN] const KTocEntry * 	node		this directory's node within the TOC
 * [IN] const KFile * 		archive		open archive file as KFile
 * [IN] uint32_t 		dad_root	offset of end of "root" within the path
 * [IN] const char *		path		path all the way back to file system root not
 *						effective but real root
 * [IN] uint32_t		path_size	length of path
 * [IN] bool			update		!read_only -- ignored for now and forced read_only
 * [IN] bool			chroot		make this a chroot to dad root?
 */
static
rc_t	KArcDirMake	(KArcDir ** self,
                         enum RCContext ctx,
                         const KDirectory * parent,
                         const KToc * toc,
                         const KTocEntry * node,
                         const void * archive,
                         KArcFSType baseType,
                         uint32_t dad_root,
                         const char *path,
                         uint32_t path_size,
                         bool update, /* ignored */
                         bool chroot)
{
    KArcDir * dir;
    rc_t rc;

    assert (self != NULL);
    assert (parent != NULL);
    assert (toc != NULL);
    /* node will be NULL at base archive file */
    assert (archive != NULL);
    assert (path != NULL);

    dir = malloc ((sizeof(KArcDir) - sizeof dir->path) + path_size + 2);

    if (dir == NULL)
    {
        *self = NULL;
        return RC (rcFS, rcArc, rcCreating, rcMemory, rcExhausted);
    }

    dir->toc = NULL;
    dir->archive.v = NULL;

    rc = KDirectoryInit (&dir->dad, (const KDirectory_vt*) &vtKArcDir,
                         "KArcDir", path,
			 /* update*/ false); /* force KDirectory to read_only here */
    if (rc != 0)
    {
        free (dir);
        return ResetRCContext (rc, rcFS, rcDirectory, ctx);
    }

    dir->parent = parent;
    dir->toc = toc;
    KTocAddRef(toc);
    dir->node = node;
    dir->arctype = baseType;
    dir->archive.v = archive;

    switch (baseType)
    {
    default:
        free (dir);
        return RC (rcFS, rcArc, rcConstructing, rcParam, rcInvalid);
    case tocKFile:
        KFileAddRef (archive);
        break;
    case tocKDirectory:
        KDirectoryAddRef (archive);
        break;
    }
    memmove (dir->path, path, path_size);
    dir->root = chroot ? path_size : dad_root;
    dir->size = path_size+1; /* make space for the next two operations */
    dir->path [path_size] = '/';
    dir->path [path_size+1] = 0;
    *self = dir;
    return 0;
}


/* ----------------------------------------------------------------------
 * KArcDirGetTOC
 *
 * [IN]  const KArcDir * 	self	Object oriented C
 * [OUT] const KToc ** 	toc	Where to put a pointer to the TOC for the KArcDir
 *
 * Set a pointer to point the TOC for this KArcDir(KDirectory)
 */
rc_t KArcDirGetTOC (const KArcDir * self,
                    const struct KToc ** toc)
{
    assert (self != NULL);
    assert (toc != NULL);

    *toc = self->toc;
    return 0;
}


/* ----------------------------------------------------------------------
 * KDirectoryToKArcDir
 *
 * [IN]  const KDirectory * 	self	Object oriented C
 * [OUT] const KArcDir * 	cast	Object oriented C
 *
 *
 * Get a reference to a KArcDir from a KDirectory as a cast.  It is a new reference.
 */

LIB_EXPORT rc_t CC KDirectoryToKArcDir (const KDirectory * self, const KArcDir ** cast)
{
    rc_t rc;
    if ( cast == NULL )
        rc = RC ( rcFS, rcArc, rcCasting, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcFS, rcArc, rcCasting, rcSelf, rcNull );
        else
        {
            if (self->vt != (const KDirectory_vt*)&vtKArcDir)
                rc = RC ( rcFS, rcArc, rcCasting, rcSelf, rcIncorrect );
            else
            {
                rc = KDirectoryAddRef ( self );
                if ( rc == 0 )
                {
                    * cast = (const KArcDir*)self;
                    return 0;
                }
            }
        }

        * cast = NULL;
    }

    return rc;
}


static
rc_t KDirectoryOpenArcDirRead_intern( const KDirectory * self,
                                      const KDirectory ** pdir,
                                      bool chroot,
                                      bool silent,
                                      const char * path,
                                      KArcFSType baseType,
                                      void * _archive,
                                      rc_t ( CC * parse )( KToc *, const void *,
                                                           bool( CC* )( const KDirectory*,
                                                                        const char *, void*),
                                                           void * ),
                                      bool( CC* filter )( const KDirectory*, const char *, void * ),
                                      void * filterparam )
{
    union
    {
        const void * v;
        const KFile * f;
        const KDirectory * d;
    } archive;
    KArcDir * arcdir = NULL;
    KToc   * toc = NULL;
    String   spath;
    size_t   pathlen;
    rc_t     rc = 0;
    rc_t     rcaux = 0;
    char     cpath [ 4096 ];
    uint32_t type;

    archive.v = _archive;
    /* -----
     * sanity check parameters  The first bunch is from the base class KDirectory.
     */
    if ( pdir == NULL )
    {
        if ( !silent )
            PLOGMSG ( klogErr, ( klogErr,
                      "NULL parameter for the root KArcDir for $(file)",
                       PLOG_S ( file ),
                       path ) );
        return RC ( rcFS, rcDirectory, rcOpening, rcDirectory, rcNull );
    }
    *pdir = NULL;

    if ( self == NULL )
    {
        if ( !silent )
            PLOGMSG ( klogErr, ( klogErr,
                      "NULL parameter for self for $(file)",
                      PLOG_S ( file ),
                      path ) );
        return RC ( rcFS, rcDirectory, rcOpening, rcSelf, rcNull );
    }

    if ( path == NULL )
    {
        if ( !silent )
            PLOGMSG ( klogErr,( klogErr,
                      "NULL parameter for the root KArcDir for $(file)",
                      PLOG_S ( file ),
                      path ) );
        return RC ( rcFS, rcDirectory, rcAccessing, rcPath, rcNull );
    }

    if ( path[0] == 0x00 )
    {
        if ( !silent )
            PLOGMSG ( klogErr,( klogErr,
                      "NULL string for the root KArcDir for $(file)",
                      PLOG_S ( file ),
                      path ) );
        return RC ( rcFS, rcDirectory, rcAccessing, rcPath, rcInvalid );
    }

    if (_archive == NULL)
    {
        /* -----
         * we got a local String type mixed in along with ASCIZ cstrings
         */
        rc = KDirectoryResolvePath ( self, true, cpath, sizeof (cpath), "%s", path );
        if ( rc == 0 )
        {
            size_t ln;
            ln = strlen (cpath);
            if ((cpath[ln-1] == '.')&&(cpath[ln-2] == '/'))
                cpath[ln-2] = '\0';
        }
        else
            return rc;
    }
    else
    {
        size_t ln;

        rc = string_printf (cpath, sizeof cpath, &ln, "%s", path);
        if ((rc == 0) && (ln > sizeof cpath))
            rc = RC (rcFS, rcArc, rcOpening, rcBuffer, rcInsufficient);
    }

    StringInitCString ( &spath, cpath );
    pathlen = strlen ( cpath );

    if (_archive != NULL)
    {
        switch (baseType)
        {
        case tocKFile:
            rc = KFileAddRef (_archive);
            break;
        case tocKDirectory:
            rc = KDirectoryAddRef (_archive);
            break;
        default:
            /* i dunno */
            break;
        }
        if (rc)
            return rc;
    }
    else
    {
        type = KDirectoryPathType ( self, "%s", cpath );
        switch ( type & ~kptAlias )
        {
        default:
            rc = RC ( rcFS, rcArc, rcOpening, rcFile, rcUnexpected );
            if ( !silent )
                LOGERR (klogErr, rc, "Unusable file type" );
            break;
        case kptNotFound:
        case kptBadPath:
            rc = RC ( rcFS, rcArc, rcOpening, rcFile, rcNotFound );
            break;

        case kptFile:
            /* -----
             * Open the archive file as a KFILE for internal use
             *
             * Fail / quit if we couldn't
             */
            if ( baseType != tocKFile )
            {
                rc = RC ( rcFS, rcArc, rcOpening, rcFile, rcIncorrect );
                if ( !silent )
                    LOGERR ( klogErr, rc, "Mismatch of file type and expected type" );
                return rc;
            }

            rc = KDirectoryOpenFileRead ( self, &archive.f, "%s", cpath );
            if ( rc != 0 && !silent )
            {
                PLOGERR ( klogErr, ( klogErr, rc,
                                     "Failed to open archive file $(file)",
                                     PLOG_S (file),
                                     path ) );
            }
        
            break;

        case kptDir:
            if ( baseType != tocKDirectory )
            {
                rc = RC ( rcFS, rcArc, rcOpening, rcFile, rcInconsistent );
                if ( !silent )
                    LOGERR ( klogErr, rc, "Mismatch of file type and expected type" );
                return rc;
            }

            rc = KDirectoryOpenDirRead ( self, &archive.d, false, "%s", cpath );
            if ( rc != 0 && !silent )
            {
                PLOGMSG ( klogErr,
                          ( klogErr, "Failed to open archive directory $(file)",
                            PLOG_S ( file ), path ) );
            }
            break;
        } /* switch ( type & ~kptAlias ) */
    }
    if ( rc == 0 )
    {
        /* -----
         * Build the TOC necessary to get to all internal files and subdirectories
         * Any sub KArcDir or KFile created will also need to reference the same TOC
         */
        rc = KTocInit ( &toc, &spath, baseType, archive.v, sraAlign4Byte );
        if ( rc != 0 )
        {
            if ( !silent )
                PLOGERR ( klogErr, 
                     ( klogErr, rc, "Failed to initialize Table of Contents for $(path)", 
                     PLOG_S ( path ), cpath ) );
        }
        else
        {
            const KTocEntry * node;

            node = KTocGetRoot( toc );

            rc = KArcDirMake (&arcdir,      /* where to build the KArcDir */
                              rcAccessing,  /* funky RC replacement thingie */
                              self,         /* parent KDirectory - for previous path */
                              toc,          /* table of contents structure for this archive */
                              node,         /* archive "root" has no node */
                              archive.v,    /* KFile for the archive */
                              baseType,     /* base type o archive */
                              0,            /* dad_root */
                              cpath,        /* path to archive will be the path of the directory */
                              (uint32_t)pathlen, /* length of path (the Init will append "/") */
                              true,         /* readonly */
                              false);       /* not chroot */
            if ( rc != 0 )
            {
                if ( !silent )
                PLOGERR ( klogErr, 
                          ( klogErr, rc, "Failed to allocate for the root KArcDir for $(file)",
                          PLOG_S ( file ), path ) );
            }
            else
            {
                rc = parse( toc,archive.v, filter, filterparam );
                if ( rc == SILENT_RC ( rcFS, rcArc, rcParsing, rcToc, rcIncomplete ) )
                {
                    rcaux = rc;
                    rc = 0;
                }
                if ( rc != 0 )
                {
                    if ( !silent )
                        PLOGERR ( klogErr, 
                             ( klogErr, rc, "Failed to parse $(file)",
                             PLOG_S ( file ), cpath ) );
                }
                else
                {
                    *pdir = &arcdir->dad;
                }
            }
        }
    }
    /* -----
     * if rc is non-zero we failed somewhere above.
     * Release allocated memory and exit with that error code.
     */
    if ( archive.v != NULL )
    {
        switch ( baseType )
        {
        default:
            free ( (void*)archive.v );
            break;
        case tocKFile:
            KFileRelease ( archive.f );
            break;
        case tocKDirectory:
            KDirectoryRelease ( archive.d );
            break;
        }
    }

    rc = rc | rcaux;

    if ( rc != 0 )
    {
        if ( arcdir != NULL )
            KDirectoryRelease ( &arcdir->dad );
        *pdir = NULL;
    }

    if ( toc != NULL )
        KTocRelease ( toc );

    return rc;
}


/* ----------------------------------------------------------------------
 * KDirectoryOpenArcDirRead
 *  Open an archive file as a KDirectory derived type: made to match 
 *  KDirectoryOpenDirRead() where parse could be the first element of arg
 *
 * Much of the code in this function is copied directly from KDirectoryVOpenDirRead
 *
 * [IN]  dir   A KDirectory (of any derived type) to reach the archive file
 * [OUT] pdir  The KDirectory (of type KArcDir)
 * [IN]  chroot if non-zero, the new directory becomes chroot'd and interprets paths
 *       stating with '/'
 * [IN]  path   The path to the archive - this will decome the directory path
 * [IN]  parse  A pointer to the function needed to build a TOC by parsing the file
 *
 * This function is made difficult because a path is not a global type.  Where we
 * would like to be able to take path and args and have them parsed nicely as they
 * are for KSysDir or KArcDir with nearly identical functions those functions are
 * not available here as self is a KDirectory but we do not know and should not know
 * whether it is a KSysDir or KArcDir.  Unfortunately KSysDir was made overly opaque.
 * [this is a result of providing full encapsulation, i.e. KDirectory is ONLY an
 *  interface and there is no implementation provided.][well very little]
 * If we could take path and args (path and ...) and pass them to a function
 * equivalent to KSysDirMakePath we would be able to do this right.  Or if we could
 * ask the self what it's path was and what it considered size and root we could do
 * the path build here and tidy up properly what we got.  .As is, we have  to be more
 * limited.
 *
 * We will have to wait and fix this right in directory.c where we can instead have
 * an attempt to open using KDirectory[V]OpenDirRead() check if when it tries to
 * open a directory but instead finds it has been given a path it will try to parse 
 * the file using a known set of parse functions to see if the file can be treated
 * as a directory. Then this function can be called from KDirectoryOpenDirRead.
 */
LIB_EXPORT rc_t CC KDirectoryOpenArcDirRead( const KDirectory * self,
    const KDirectory ** pdir,
    bool chroot,
    const char * path,
    KArcFSType baseType,
    rc_t ( CC * parse )( KToc *, const void *,
        bool( CC * )( const KDirectory *, const char *, void * ), void * ),
    bool ( CC * filter )( const KDirectory*, const char *, void *),
    void * filterparam )
{
    return KDirectoryOpenArcDirRead_intern( self,
        pdir,
        chroot, 
        false,
        path,
        baseType,
        NULL,
        parse,
        filter,
        filterparam );
}


LIB_EXPORT rc_t CC KDirectoryOpenArcDirRead_silent( const KDirectory * self,
    const KDirectory ** pdir,
    bool chroot,
    const char * path,
    KArcFSType baseType,
    rc_t ( CC * parse )( KToc *, const void *,
        bool( CC * )( const KDirectory *, const char *, void * ), void * ),
    bool ( CC * filter )( const KDirectory*, const char *, void *),
    void * filterparam )
{
    return KDirectoryOpenArcDirRead_intern( self,
        pdir,
        chroot, 
        true,
        path, 
        baseType,
        NULL,
        parse,
        filter,
        filterparam );
}
LIB_EXPORT rc_t CC KDirectoryOpenArcDirRead_silent_preopened( const KDirectory * self,
    const KDirectory ** pdir,
    bool chroot,
    const char * path,
    KArcFSType baseType,
    void * archive,
    rc_t ( CC * parse )( KToc *, const void *,
        bool( CC * )( const KDirectory *, const char *, void * ), void * ),
    bool ( CC * filter )( const KDirectory*, const char *, void *),
    void * filterparam )
{
    return KDirectoryOpenArcDirRead_intern( self,
        pdir,
        chroot, 
        true,
        path, 
        baseType,
        archive,
        parse,
        filter,
        filterparam );
}


/* ======================================================================
 * 
 */
typedef struct KArcDirPersistVisitFuncData
{
    Vector * vector;
    KArcDir * dir;
    char path[4096];
} KArcDirPersistVisitFuncData;


static
rc_t CC KArcDirPersistVisitFunc (const KDirectory * dir, uint32_t unused_type, const char * name, void * data)
{
    KPathType type;
    KArcDirPersistVisitFuncData * vdata;
    char * path;
    size_t len = 0;
    rc_t rc;

    rc = 0;
    vdata = data;

    type = (KPathType)KDirectoryPathType (dir, "%s", name);
    switch (type)
    {
    default:
	return 0;
    case kptDir:
	len = strlen (vdata->path);
	if (len > 0)
	{
	    vdata->path[len] = '/';
	    strcpy (vdata->path+len+1, name);
	}
	else
	    strcpy (vdata->path, name);


/* this needs cleaning up - dir moved along but path did not.  path handling is weak through out */
/* 	rc = KArcDirVisit ((const KArcDir*)dir, false, KArcDirPersistVisitFunc, data, vdata->path, NULL); */
	rc = KArcDirVisit ((const KArcDir*)dir, false, KArcDirPersistVisitFunc, data, name, NULL);
	if (rc != 0)
	{
	    LOGERR (klogInt, rc, "KArcDirPersist Visit failed");
	}
	vdata->path[len] = '\0';
	return rc;
    case kptFile:
    case kptFile|kptAlias:
	len = strlen (vdata->path);
	if (len > 0)
	{
	    vdata->path[len] = '/';
	    strcpy (vdata->path+len+1, name);
	}
	else
	    strcpy (vdata->path, name);
	path = malloc (strlen(vdata->path)+1);
	if (path == NULL)
	{
	    rc = RC (rcFS, rcArc, rcReindexing, rcMemory, rcExhausted);
	}
	else
	{
	    strcpy (path, vdata->path);
	    rc = VectorAppend (vdata->vector, NULL, path);
	}
	vdata->path[len] = '\0';

	return rc;
    }
}

static
void CC whack ( void *item, void *data )
{
    free (item);
}

/* ========================================
 * Build a Persisted version of a TOC in a buffer
 * return the allocated buffer
 */
#ifdef _DEBUGGING
static
void CC write_entry( void * item, void *data)
{
    const char * path = item;
    uint32_t * count = data;

    TOC_SORT(("%3u: %s\n", *count, path));

    (*count)++;
}
#endif
rc_t KArcDirPersistHeader (const KArcDir * self,
                           void ** buffer,
                           size_t * buffer_size,
                           uint64_t * file_size,
                           KSRAFileAlignment align,
                           rc_t (CC*usort)(const KDirectory*, Vector*))
{
    rc_t         rc;
    const KToc * toc;

    FUNC_ENTRY();

    if (self == NULL)
    {
	rc = RC (rcFS, rcDirectory, rcPersisting, rcSelf, rcNull);
	LOGERR (klogInt, rc, "KArcDirPersist Self is NULL");
	return rc;
    }
    if (buffer == NULL)
    {
	rc = RC (rcFS, rcDirectory, rcPersisting, rcParam, rcNull);
	LOGERR (klogInt, rc, "KArcDirPersist buffer is NULL");
	return rc;
    }
    rc = KArcDirGetTOC (self, &toc);
    if (rc != 0)
        LOGERR (klogInt, rc, "KArcDirPersist TOC retrieval failed");
    else
    {
        /* mutable field as it only applies to this specific operation */
        rc = KTocAlignmentSet ((KToc*)toc, align);
        if (rc != 0)
            LOGERR (klogInt, rc, "KArcDirPersist TOC alignment failed");
        else
        {
            Vector filevector;
            KArcDirPersistVisitFuncData	data;

            VectorInit (&filevector, 0, 0);

            data.path[0] = '\0'; /* looks like "" */
            data.vector = &filevector;


            rc = KArcDirVisit (self, false, KArcDirPersistVisitFunc, &data, ".", NULL);
            if (rc != 0)
            {
                LOGERR (klogInt, rc, "KArcDirPersist Visit failed");
            }
            else
            {

#ifdef _DEBUGGING
/* this loop and the next just can be used in a debug build to verify the sorting function used */
                {
                    uint32_t ix;
                    TOC_SORT (("Pre-sort order:\n"));

                    ix = 1;
                    VectorForEach ( &filevector, false, write_entry, &ix);
                }
#endif

                if (usort)
                    rc = usort(&self->dad, &filevector);

#ifdef _DEBUGGING
                {
                    uint32_t ix;
                    TOC_SORT (("Post-sort order:\n"));

                    ix = 1;
                    VectorForEach ( &filevector, false, write_entry, &ix);
                }
#endif


                if (rc != 0)
                {
                    LOGERR (klogInt, rc, "KArcDirPersist user sort failed failed");
                }
                else
                {
                    rc = KTocPersist (toc, buffer, buffer_size, file_size, &filevector);
                    if (rc != 0)
                        LOGERR (klogInt, rc, "KArcDirPersist TOC retrieval failed");
                }
            }
/*     free (data.path); */
            VectorWhack (&filevector, whack, NULL);
	}
    }
 
    return rc;
}

KFS_EXTERN bool CC KDirectoryIsKArcDir ( const KDirectory * self )
{
    return self != NULL && &self -> vt -> v1 == &vtKArcDir;
}

KFS_EXTERN bool CC KArcDirIsFromRemote ( const KArcDir * self )
{
    return self != NULL && self -> arctype == tocKFile &&
            ( KFileIsKCacheTeeFile ( self -> archive . f ) || KFileIsKHttpFile ( self -> archive . f ) );
}


/* end of file arc.c */

