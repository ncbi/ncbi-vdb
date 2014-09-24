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

#ifndef _h_kfs_toc_priv_h_
#define _h_kfs_toc_priv_h_

#include <klib/container.h>
#include <klib/text.h>
#include <kfs/arc.h>
#include <kfs/toc.h>
#include <atomic.h>

#ifdef _DEBUGGING
#define TOC_FUNC_ENTRY() DBGMSG (DBG_KFS, DBG_FLAG(DBG_KFS_TOCENTRY), ("Enter: %s\n", __func__))
#define TOC_SORT(msg) DBGMSG (DBG_KFS, DBG_FLAG(DBG_KFS_SRASORT), msg)
#define TOC_DEBUG(msg) DBGMSG (DBG_KFS, DBG_FLAG(DBG_KFS_TOC), msg)
#else
#define TOC_FUNC_ENTRY()
#define TOC_SORT(msg)
#define TOC_DEBUG(msg)
#endif



/*--------------------------------------------------------------------------
 * KTocEntry
 */
typedef struct KTocEntry KTocEntry;
typedef enum KTocEntryType
{
    ktocentrytype_unknown = -1,
    ktocentrytype_notfound,
    ktocentrytype_dir,
    ktocentrytype_file,
    ktocentrytype_chunked,
    ktocentrytype_softlink,
    ktocentrytype_hardlink,
    ktocentrytype_emptyfile,
    ktocentrytype_zombiefile /* zombie in the sense that it is somewhere between live and dead */
} KTocEntryType;

/* arbitrary number that was originially set much much higher than expected needs. */
#define	KARC_LINK_RESOLVE_LOOPMAX		(16)

typedef struct KTocPageChunk
{
    uint64_t file_offset;
    uint32_t page_id;
    uint32_t page_offset;
} KTocPageChunk;



struct KTocEntry
{
    /* -----
     * In C putting this first allows a weak form of polymorphism for
     * a entry class.  A pointer to a BSTNode can be an alias to a pointer to 
     * this form of node.
     */
    BSTNode		node;

    /* -----
     * The name of the directory/file/link that this Entry represents.  It is not
     * the full path which must be assembled going up the tree.
     *
     * Be careful what String functions you try as some want to reallocate memory.
     */
    String		name;

    /* -----
     * Unix style file modification time (same as a 64-bit time_t)
     */
    KTime_t		mtime;

    /* -----
     * Unix style file access
     */
    uint32_t		access;

    /* -----
     * what type of entry is this?
     */
    KTocEntryType	type;

    union KTocEntryUnion
    {
	struct KTocEntryDir
	{
	    /* -----
	     * Tree of directories, files and links below this directory
	     */
            BSTree      tree;
	} dir;
	struct KTocEntryFile
	{
	    /*
	     * how long is this file?
	     */
	    uint64_t	file_size;
	    /* 
	     * location within the archive where this file's data starts
	     */
	    uint64_t	archive_offset;
	} contiguous_file;
	struct KTocEntryZombieFile
	{
/* KEEP the first two fields here match the first two fields above
 * file_size and archive_offset so the chunked is sort of a derived type */
	    /*
	     * how long is this file supposed to be?
	     */
	    uint64_t	file_size;
	    /* 
	     * location within the archive where this file's data would have started
	     */
	    uint64_t	archive_offset;
	} zombie_file;
	struct KTocEntryChunkFile
	{
/* KEEP the first two fields here match the first two fields above
 * file_size and archive_offset so the chunked is sort of a derived type */
	    /*
	     * Size of the whole file including all chunks and any zero byte holes
	     * between chunks
	     */
	    uint64_t	file_size;
	    /* 
	     * location within the archive where this file's data starts if viewws
	     * as a virtual "*.sra"
	     */
	    uint64_t	archive_offset;

	    KTocChunk *	chunks;			/* this will piggy back in the same allocation */
	    uint32_t	num_chunks;	    
	} chunked_file;
        struct KTocEntryPagedChunkFile
        {
            uint64_t file_size;
        } paged_chunk_file;
	struct KTocEntrySoftLink
	{
	    String	link_path;
	} symbolic_link;
	struct KTocEntryHardLink
	{
	    const struct KTocEntry * ref;
	    /* -----
	     * This approach can lead to failures if the target of the link is deleted and this
	     * reference is left dangling.
	     */
	} hard_link;
    } u;
};

typedef struct KTocEntryIndex KTocEntryIndex;
struct KTocEntryIndex
{
    BSTNode	node;
    KTocEntry *	entry;
    String	fullpath;
};


/* ----------------------------------------------------------------------
 * KTocEntryParam
 *
 * An internal structure used only with in this compilation unit / module
 */
/* ----------------------------------------------------------------------
 * NOTE: This oddness of specific to general to specific can probably be 
 * refactored into specific functions (the published API) that uses common
 * functions instead of a common target function.
 */
typedef struct KTocEntryParam	KTocEntryParam;
struct KTocEntryParam
{
    KTocEntryType	type;
    KTime_t		mtime;
    uint32_t		access;
    union
    {
	struct
	{
	    uint64_t		size;
	    uint64_t		source_position;
	} file;
	struct
	{
	    uint64_t		size;
	    uint32_t		num_chunks;
	    const KTocChunk *	chunks;
	} chunked;
	struct
	{
            const KTocEntry *   ref;
	} hardlink;
	struct
	{
	    const char *	targ;
	} softlink;
    } u;
};

/* ----------------------------------------------------------------------
 * KArcTOCNodeWhack
 * [IN] BSTNode * 		n	
 * [IN] void *    		data	
 *
 * This function matches the signature for a function pointer in a 
 * call to BSTreeWhack().  It ignores the data pointer second parameter
 * which can safely be called with NULL. 
 */
void	CC KTocEntryWhack	(BSTNode * n, void * data);
int	CC KTocEntryCmpVoid (const void * item, const BSTNode * node);
rc_t	KTocEntryDelete	(KTocEntry * self);
int	CC KTocEntryCmp2 (const BSTNode * n, const BSTNode * p);
rc_t	KTocEntryNewDirectory	(KTocEntry ** new_entry,
					 const char * name,
					 size_t name_size,
					 KTime_t mtime,
					 uint32_t access);
rc_t		KTocEntryNewFile	(KTocEntry ** new_entry,
					 const char * name,
					 size_t name_size,
					 KTime_t mtime,
					 uint32_t access,
					 uint64_t offset,
					 uint64_t size);
rc_t		KTocEntryNewChunked	(KTocEntry ** new_entry,
					 const char * name,
					 size_t name_size,
					 KTime_t mtime,
					 uint32_t access,
					 uint64_t size,
					 const KTocChunk * chunks,
					 uint32_t num_chunks);
rc_t		KTocEntryNewSoft	(KTocEntry ** new_entry,
					 const char * name,
					 size_t name_size,
					 KTime_t mtime,
					 uint32_t access,
					 const char * link,
					 size_t link_size);
rc_t		KTocEntryNewHard	(KTocEntry ** new_entry,
					 const char * name,
					 size_t name_size,
					 KTime_t mtime,
					 uint32_t access,
                                         const KTocEntry * targ);
rc_t		KTocEntryNewZombieFile	(KTocEntry ** new_entry,
					 const char * name,
					 size_t name_size,
					 KTime_t mtime,
					 uint32_t access,
					 uint64_t offset,
					 uint64_t size);


typedef struct	KTocEntryStack 	KTocEntryStack;
rc_t KTocEntryStackNew(KTocEntryStack ** self);
rc_t KTocEntryStackDel(KTocEntryStack * self);
rc_t KTocEntryStackDepth(KTocEntryStack * self, int * depth);
rc_t KTocEntryStackPush(KTocEntryStack * self, KTocEntry * pathaddr);
rc_t KTocEntryStackPop(KTocEntryStack * self, KTocEntry ** pathaddr);

const char * KTocEntryTypeGetString(KTocEntryType t);
const char * get_mode_string (KCreateMode mode);

typedef struct KTocEntryPersistWriteFuncData
{
    uint8_t *	buffptr;
    uint8_t *	limit;
} KTocEntryPersistWriteFuncData;
KFS_EXTERN rc_t CC KTocEntryPersistWriteFunc (void * param, const void * buffer, size_t size,
				size_t * num_writ);

rc_t KTocEntryPersistNodeDir (void *param, const KTocEntry * n,
			      size_t * num_writ, 
			      PTWriteFunc write, void * write_param);

/* ======================================================================
 * KToc struct
 */
struct KArcDir;

struct KToc
{
    KArcFSType	arctype;
    KSRAFileAlignment alignment;
    /* -----
     * File containing the archive: details of its type hidden behind KFile.
     * We should be able to do an archive in an archive by having this KFile
     * point to a KArcFile with no problems.
     */
    union
    {
	const void *		v;
	struct KFile const *f;
	const KDirectory * 	d;
    } archive;

    /* -----
     * KArcDir that implements the KDirectory interface for the contents of 
     * the file opened as archive.  This link is needed to get to the 
     * KDirectory interface from a KFile symbolic link.
     *
     * This reference is not counted in refcount as it is a link back to the
     * tracked reference from the KArcDir to the KARcTOC.
     */
/*     const struct KArcDir *	karchive; */

    /* -----
     * count of references to this opened archive
     */
    atomic32_t		refcount;

    /* -----
     * A entry outside of a tree: instantiated as a special KTocEntry that is part of
     * no BSTree with type set to ktocentrytype_dir (directory).
     * Does not use the base dir BSTNode but makes processing easier to have a 
     *"entry" type...
     */
    KTocEntry		entry;

    /* -----
     * These next two are used only in the KFile interface on top of a KToc
     * on a KDirectory.  This needs to be rewritten as an interface so these
     * don't exist in all KToc implmentations.
     *
     * Index  The main tree in 'entry' is sorted by names with in directories.
     * This index is a directory less tree sorted by offset with in the archive.
     *
     * This is not currently used in tar file versions of the TOC archive.
     *
     * Each entry is of type KTocEntryIndex;
     */
    BSTree		offset_index;
    KSraHeader *	header;


    /* -----
     * This is the full path of the archive file as used to open it as a KFile.
     */
    String		path;

};

/* four fields are common to all entries.*/


/* GetName
 *
 *  [RET] rc_t                  0 for success; anything else for a failure
 *                       see itf/klib/rc.h for general details
 *  [IN]     const KTocEntry *  self        object oriented object reference
 *  [OUT] const char **         name        where to put a reference to the name
 */
rc_t KTocEntryGetName(const KTocEntry *self,
                     const char ** name);

/* GetTime
 *
 * [RET] rc_t                   0 for success; anything else for a failure
 *                      see itf/klib/rc.h for general details
 * [IN]  const KTocEntry *  self        object oriented object reference
 * [OUT] KTime_t *      mtime       where to put unix/posix style permission flags
 */
rc_t KTocEntryGetTime(const KTocEntry *self,
                     KTime_t *mtime);

/* GetAccess
 *
 * [RET] rc_t                   0 for success; anything else for a failure
 *                             see itf/klib/rc.h for general details
 * [IN]  const KTocEntry *  self        object oriented object reference
 * [OUT] uint32_t       access      where to put unix/posix style permission flags
 */
rc_t KTocEntryGetAccess(const KTocEntry * self, uint32_t * access);


/* GetType
 *
 * [RET] rc_t                   0 for success; anything else for a failure
 *                      see itf/klib/rc.h for general details
 * [IN]  const KTocEntry *  self        object oriented object reference
 * [OUT] KTocEntryType *    type        where to put the type specific node type (differs
 *                      from KDirectory type)
 */
rc_t KTocEntryGetType(const KTocEntry *self,
                     KTocEntryType * type);
/* the type retrieved by GetType will permit several more fields specialized by type */
/* for a directory the only specialzied field is the "BSTree" */

#if 0
const char * KTocEntryGetTypeString(KTocEntryType type);
#endif

/* GetBSTree
 *
 * Get a reference to the BSTree of a TocEntry that represents a directory.
 * This will fail for all ites taht are not directories.
 *
 * [RET] rc_t                   0 for success; anything else for a failure
 *                      see itf/klib/rc.h for general details
 * [IN]  const KTocEntry *  self        object oriented object reference
 * [OUT] const BSTree **    ptree       where to put a reference to the kcont/BSTree of a
 *                      directory node
 */
rc_t KTocEntryGetBSTree( const KTocEntry * self,
                         struct BSTree const ** ptree);

/* for a file there are two fields: a container offset and a file size */


/* GetFileLocator (works for contiguous or chunked files)
 *
 * [RET] rc_t                   0 for success; anything else for a failure
 *                      see itf/klib/rc.h for general details
 * [IN]  const KTocEntry *  self        object oriented object reference
 * [OUT] uint64 *       locator        where to put the locator of a referenced node
 */
rc_t KTocEntryGetFileLocator( const KTocEntry * self,
                              uint64_t * locator);


/* GetFileSize (works for contiguous or chunked files)
 *
 * [RET] rc_t                   0 for success; anything else for a failure
 *                      see itf/klib/rc.h for general details
 * [IN]  const KTocEntry *  self        object oriented object reference
 * [OUT] uint64 *       size        where to put the size of a referenced node
 */
rc_t KTocEntryGetFileSize( const KTocEntry * self,
                     uint64_t * size);


/* GetFilePhysicalSize (works for contiguous or chunked files)
 *
 * [RET] rc_t                   0 for success; anything else for a failure
 *                      see itf/klib/rc.h for general details
 * [IN]  const KTocEntry *  self        object oriented object reference
 * [OUT] uint64 *       size        where to put the size of a referenced node
 */
rc_t KTocEntryGetFilePhysicalSize( const KTocEntry * self,
                                   uint64_t * size);


/* GetFileContiguous (works for contiguous or chunked files)
 *
 * [RET] rc_t                   0 for success; anything else for a failure
 *                      see itf/klib/rc.h for general details
 * [IN]  const KTocEntry *  self        object oriented object reference
 * [OUT] uint64 *       size        where to put the size of a referenced node
 */
rc_t KTocEntryGetFileContiguous( const KTocEntry * self,
                                 bool * contiguous);


/* GetFileOffset
 *
 * For normal contiguous file nodes 'offset' gets the offset to the start of the file.
 * For all other nodes, 'offset' gets 0 with an appropriate rc_t return.
 *
 *
 * [RET] rc_t                   0 for success; anything else for a failure
 *                      see itf/klib/rc.h for general details
 * [IN]  const KTocEntry *  self        object oriented object reference
 * [OUT] uint64 *       offset      where to put the offset of a referenced node
 */
rc_t KTocEntryGetFileOffset( const KTocEntry * self,
                     uint64_t * offset);


/* GetHardTarget
 *
 * [RET] rc_t                   0 for success; anything else for a failure
 *                      see itf/klib/rc.h for general details
 * [IN]  const KTocEntry *  self        object oriented object reference
 * [OUT] const KTocEntry ** target      where to put a reference from a hard link node
 */
rc_t KTocEntryGetHardTarget( const KTocEntry *self,
                     const KTocEntry ** target);

/* there are two specialzied fields for chunked files - both retrieved with the saem method */


/* GetChunks
 *
 * [RET] rc_t                   0 for success; anything else for a failure
 *                      see itf/klib/rc.h for general details
 * [IN]  const KTocEntry *  self        object oriented object reference
 * [OUT] const KTocChunk *  chunks      where to put the size of a referenced node
 */
rc_t KTocEntryGetChunks (const KTocEntry * self,
                     uint32_t * num_chunks,
                     const KTocChunk ** chunks);

/* GetSoftTarget
 *
 * [RET] rc_t                   0 for success; anything else for a failure
 *                      see itf/klib/rc.h for general details
 * [IN]  const KTocEntry *  self        object oriented object reference
 * [OUT] const char **      target      where to put a reference from a hard link node
 */
rc_t KTocEntryGetSoftTarget (const KTocEntry *self,
                     const char ** target);


/*--------------------------------------------------------------------------
 * KToc
 *  archive table of contents
 *
 *  the "CreateXXX" series of methodss take a KCreateMode ( see kfs/directory.h ).
 *  when mode == kcmOpen, create if needed, overwrite if exists.
 *  when mode == kcmInit, create if needed, clear if exists.
 *  when mode == kcmCreate, fail if exists, create otherwise.
 *  if mode has kcmParents set, fill in missing parents on create.
 *
 * These is the only open API into a TOC with in the archive package of classes.
 * Functions to parse archives (Tar was the first format but is not expected to 
 * be the only format) use these methods to add entries to the TOC.  New parsers
 * should fit the signature of 
 *
 *  rc_t PARSER (KToc * <TOC>, const KFile * <ARCHIVE>)
 *
 * "PARSER" [IN] is the parsing function
 *
 * "TOC" [OUT] is a opaque class that is never exposed other than to these create
 * entry methods
 *
 * "FILE" [IN] is an archive opaquely opened as a KFile and then given to a
 * KDirectory and should not be accessed from outside this parsing function.
 *
 * An Archive is opened via a call to friend function.  It has by design been
 * made to mimic the signature of KDirectoryOpenDirRead with only a change in
 * parameters after path.  No "..." or va_list is supported.  To use that
 * functionality one would have to call KDirectoryResolvePath and use the
 * new path generated by that call as the path parameter for this function.
 *
 * rc_t KDirectoryOpenArcDirRead (const KDirectory * dir, 
 *                    KDirectory ** pdir,
 *                    bool chroot,
 *                    const char * path,
 *                    rc_t (CC* parse )(KToc *,const KFile*));
 *
 * "dir" [IN] is an existing KDirectory in which to find the archive file.
 *
 * "pdir" [OUT] is a pointer which will after the function is called point to the new
 * KDirectory that is the archive opened as a directory with the same name as
 * its containing file.
 *
 * "chroot" [IN] is false(1)/true(1 [or any non zero]) as to whether the archive will
 * treat its contents as a virtual root directory ('/') or not.
 *
 * "path" [IN] is the address with in directory of the archive.
 *
 * "parse" [IN] is the function that will create TOC entries for that archive.
 *
 * No member element or function of an object of class TOC will be exposed
 * other than these create entry methods.
 */
typedef struct KToc KToc;

/* Init
 *  construct a TOC tied to a KFile with a given path
 * At this point, this method has been made private and will only be called by 
 * KDirectoryOpenArcDirRead().
 */

rc_t KTocInit( KToc**self, struct String const * path, KArcFSType arctype,
        const void * archive, KSRAFileAlignment alignment);

/* AddRef
 * Release
 *  ignores NULL references
 */
rc_t KTocAddRef( const KToc *self );
rc_t KTocRelease( const KToc *self );


/* AddRef
 * Release
 *  ignores NULL references
 * At this point, these methods have been made private and will only be called by 
 * with in the archive package.  References to the TOC will be handled within 
 * the constructors and destructors for KDirectorys and KFiles created by
 * KDirectory OpenArcDirRead() and then any KDirectory OpenFileRead() and 
 * KDirectory OpenDirRead() with in that initial KDirectory().  At this point
 * archives are read only so OpenFileWrite() OpenDirWrite() CreateDir() CreateFile()
 * and other such methods are not supported for archives.
 */
rc_t KTocAddRef ( const KToc *self );
rc_t KTocRelease ( const KToc *self );


/* ResolvePathNode
 *
 *
 * [RET] rc_t                   0 for success; anything else for a failure
 *                      see itf/klib/rc.h for general details
 * [IN]  const KToc *   self        TOC self reference: object oriented in C
 * [IN]  const char *       path        the path to be parsed - it will start at the root 
 *                      of the TOC's address
 * [OUT] const KTocEntry **     pnode       the address of the final node traversed is put here
 * [OUT] uint32_t *         type        the type of node for the final node
 * [OUT] char **        unusedpath  This will point to any part of the path not used up
 *                      in parsing against the TOC.
 *                      NULL if the leaf node was found
 *                      the start of a node that could not be parsed
 *                      the start of the node after a softlink (or NULL)
 */
rc_t KTocResolvePathTocEntry( const KToc *self,
                         const KTocEntry ** pnode,
                         const char *path,
                         size_t path_len,
                         KTocEntryType * ptype,
                         const char ** unusedpath);

/*--------------------------------------------------------------------------
 * Constructors/factories
 */

/* CreateDir
 *  create a sub-directory
 *
 *  "mode" [ IN ] - a creation mode ( see explanation above ).
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target directory
 */
rc_t KTocCreateDir ( KToc *self, KTime_t mtime, uint32_t access, 
        KCreateMode mode, const char *path, ... );
rc_t KTocVCreateDir ( KToc *self, KTime_t mtime, uint32_t access,
        KCreateMode mode, const char *path, va_list args );

/* CreateFile
 *  create an entry for a contiguous file
 *
 *  "source_position" [ IN ] and "size" [ IN ] - description of the contiguous
 *  region of the archive file occupied by the file
 *
 *  "mode" [ IN ] - a creation mode ( see explanation above ).
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target file
 *
 * This is used also to create an empty file.  When size is zero the underlying 
 * library might create a special zero length entry type.
 */
rc_t KTocCreateFile ( KToc *self, uint64_t source_position, uint64_t size,
              KTime_t mtime, uint32_t access, KCreateMode mode,
              const char *path, ... );
rc_t KTocVCreateFile ( KToc *self, uint64_t source_position, uint64_t size,
               KTime_t mtime, uint32_t access, KCreateMode mode,
               const char *path, va_list args );
rc_t KTocCreateZombieFile ( KToc *self, uint64_t source_position, uint64_t size,
              KTime_t mtime, uint32_t access, KCreateMode mode,
              const char *path, ... );
rc_t KTocVCreateZombieFile ( KToc *self, uint64_t source_position, uint64_t size,
               KTime_t mtime, uint32_t access, KCreateMode mode,
               const char *path, va_list args );

/* CreateChunkedFile
 *  create an entry for a chunked file
 *
 *  a chunked file consists of 0 or more possibly discontiguous chunks
 *  positioned within the logical file
 *
 *  "size" [ IN ] - the size of the logical file
 *
 *  "num_chunks" [ IN ] - the number of chunks in "chunks"
 *
 *  "chunks" [ IN ] - an array of chunk descriptors, where each chunk
 *  contains valid data that are mapped to form file
 *
 *  "mode" [ IN ] - a creation mode ( see explanation above ).
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target file
 */
rc_t KTocCreateChunkedFile ( KToc *self, uint64_t size, KTime_t mtime,
                 uint32_t access, uint32_t num_chunks,
                 const KTocChunk chunks [], KCreateMode mode,
                 const char *path, ... );
rc_t KTocVCreateChunkedFile ( KToc *self, uint64_t size, KTime_t mtime,
                  uint32_t access, uint32_t num_chunks,
                  const KTocChunk chunks [], KCreateMode mode,
                  const char *path, va_list args );

/* CreateHardLink
 *  creates a hard link to an existing file within TOC
 *
 *  "mode" [ IN ] - a creation mode ( see explanation above ).
 *
 *  "targ" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target object
 *
 *  "link" [ IN ] - NUL terminated string in directory-native
 *  character set denoting alias
 */
rc_t KTocCreateHardLink ( KToc *self, KTime_t mtime, uint32_t access,
              KCreateMode mode, const char *targ,
              const char *link, ... );
rc_t KTocVCreateHardLink ( KToc *self, KTime_t mtime, uint32_t access,
               KCreateMode mode, const char *targ,
               const char *link, va_list args );

/* CreateSoftLink
 *  creates a soft link to any file or directory
 *  that is interpreted upon acces.
 *
 *  "mode" [ IN ] - a creation mode ( see explanation above ).
 *
 *  "targ" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target object
 *
 *  "alias" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target alias
 */
rc_t KTocCreateSoftLink ( KToc * self, KTime_t mtime, uint32_t access,
              KCreateMode mode, const char * targ,
              const char * alias, ... );
rc_t KTocVCreateSoftLink ( KToc * self, KTime_t mtime, uint32_t access,
               KCreateMode mode, const char * targ,
               const char * alias, va_list args );

/* OpenArcDirRead
 *  Open an archive file as a KDirectory derived type: made to match 
 *  KDirectoryOpenDirRead() where parse could be the first element of arg
 *
 * [IN]  dir    A KDirectory (of any derived type) to reach the archive file
 * [OUT] pdir   The KDirectory (of type KArcDir) that will be created
 * [IN]  chroot if non-zero, the new directory becomes chroot'd and interprets paths
 *      stating with '/'
 * [IN]  path   The path to the archive - this will become the directory path
 * [IN]  parse  A pointer to the function needed to build a TOC by parsing the file
 *
 * parameters to parse
 * REQUIRED:    KToc *
 * REQUIRED:    void *  KFile* | KDirectory* | ???
 * OPTIONAL:    bool (*)(const char*)           Filter: include or skip this path
 * OPTIONAL:    rc_t (*)(???)               Sort: reorder name list
 */

KFS_EXTERN
rc_t CC KDirectoryOpenArcDirRead (const KDirectory * dir, 
            const KDirectory ** pdir,
            bool chroot,
            const char * path,
            KArcFSType baseType,
            rc_t (CC*parse)(KToc*,const void *,
                bool(CC*)(const KDirectory*,const char *, void*),
                void *),
                bool (CC* filter )(const KDirectory*, const char *, void *),
            void * filterparam );


#if 0
rc_t KTocResolvePathTocEntry( const KToc *self,
                 const KTocEntry ** pnode,
                 const char *path,  /* must be entirely in the TOC */
                 size_t path_len,
                 KTocEntryType * ptype,
                 const char ** unusedpath);
#endif



const KTocEntry* KTocGetRoot( const KToc * self );

#if 0
const char * KTocGetRootPath( const KToc * self, size_t * len );
#endif

rc_t KTocGetPath( const KToc * self, struct String const ** path );
rc_t KTocResolvePathFromOffset( const KToc *self,
                const char ** path,
                uint64_t * ppos,
                uint64_t offset );

KSRAFileAlignment KTocAlignmentGet( KToc * self );
rc_t KTocAlignmentSet( KToc * self,KSRAFileAlignment align );

const void * KTocGetArchive( const KToc * self );

KFS_EXTERN rc_t KTocPersist( const KToc * self,
                             void ** buffer, size_t * buffer_size,
                             uint64_t * virtual_file_size,
                             struct Vector const * files );

KFS_EXTERN rc_t CC KTocEntryPersist( void *param, const void * node,
                                     size_t * num_writ, 
                                     PTWriteFunc write, void * write_param );

rc_t KTocInflatePBSTree (KToc * self, uint64_t arcsize, const void * treestart,
                         uint32_t maxsize, uint64_t offset,
                         bool rev, const char * path);

rc_t KArcDirPersistHeader	(const KArcDir * self,
    void ** buffer, size_t * buffer_size, uint64_t * file_sile, uint32_t align,
    rc_t ( CC *sort )(const struct KDirectory*, struct Vector*) );


/* ----------------------------------------------------------------------
 * KArcDirGetTOC
 *
 * [RET] rc_t					0 for success; anything else for a failure
 *						see itf/klib/rc.h for general details
 * [IN]  const KArcDir *	self		Object oriented C; KArcDir object for this method
 * [OUT] const KToc **		toc
 */
rc_t  KArcDirGetTOC (const struct KArcDir * self, const struct KToc ** toc);



#endif /* #ifndef _h_kfs_toc_priv_h_ */
/* end of file */
