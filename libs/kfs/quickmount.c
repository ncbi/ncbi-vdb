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
#include <kfs/quickmount.h>
#include <kfs/directory.h>

#include <klib/rc.h>
#include <klib/printf.h>

#include <sysalloc.h>

#include <string.h>
#include <stdlib.h>
#include <assert.h>

/* ----------------------------------------------------------------------
 * 
 */

/* -----
 * define the specific types to be used in the templatish/inheritancish
 * definition of vtables and their elements
 */
#define KDIR_IMPL struct KQuickMountDir
#define KNAMELIST_IMPL   struct KQuickMountListing
#include <kfs/impl.h>

/* ======================================================================
 */
/*--------------------------------------------------------------------------
 * KQuickMountDir
 *  a directory inside an archive (tar or SRA for example)
 *
 * This type is being engineered to mimic KQuickMountDir.
 */
struct KQuickMountDir
{
    KDirectory   dad;
    int          root;
    const KFile* file;
    char *       mount;
    char         path[1];
    /* actual allocation is based on actual string length */
};



/* KQuickMountDirCanonPath
 */
static
rc_t KQuickMountDirCanonPath ( const KQuickMountDir *self, enum RCContext ctx, char *path, size_t psize )
{
    char *low, *dst, *last, *end = path + psize;
    low = dst = last = path + self -> root;

    while ( 1 )
    {
        char *src = strchr ( last + 1, '/' );
        if ( src == NULL )
            src = end;

        /* detect special sequences */
        switch ( src - last )
        {
        case 2:
            if ( last [ 1 ] == '.' )
            {
                /* skip over */
                last = src;
                if ( src != end )
                    continue;
            }
            break;

        case 3:
            if ( last [ 1 ] == '.' && last [ 2 ] == '.' )
            {
                /* remove previous leaf in path */
                dst [ 0 ] = 0;
                dst = strrchr ( path, '/' );
                if ( dst == NULL || dst < low )
                    return RC ( rcFS, rcDirectory, ctx, rcPath, rcInvalid );

                last = src;
                if ( src != end )
                    continue;
            }
            break;
        }

        /* if rewriting, copy leaf */
        assert ( src >= last );

        if ( dst != last )
            memmove ( dst, last, src - last );

        /* move destination ahead */
        dst += src - last;
        
        /* if we're done, go */
        if ( src == end )
                break;

        /* find next separator */
        last = src;
    }

    /* NUL terminate if modified */
    if ( dst != end )
        * dst = 0;

    return 0;
}

/* KQuickMountDirRelativePath
 *  makes "path" relative to "root"
 *  both "root" and "path" MUST be absolute
 *  both "root" and "path" MUST be canonical, i.e. have no "./" or "../" sequences
 */
static
rc_t KQuickMountDirRelativePath (const KQuickMountDir *self, enum RCContext ctx,
                                 const char *root, char *path, size_t path_max)
{
    int backup;
    size_t bsize, psize;

    const char *r = root + self -> root;
    const char *p = path + self -> root;

    assert ( r != NULL && r [ 0 ] == '/' );
    assert ( p != NULL && p [ 0 ] == '/' );

    for ( ; * r == * p; ++ r, ++ p )
    {
        /* disallow identical paths */
        if ( * r == 0 )
            return RC ( rcFS, rcDirectory, ctx, rcPath, rcInvalid );
    }

    /* paths are identical up to "r","p"
       if "r" is within a leaf name, then no backup is needed
       by counting every '/' from "r" to end, obtain backup count */
    for ( backup = 0; * r != 0; ++ r )
    {
        if ( * r == '/' )
            ++ backup;
    }

    /* the number of bytes to be inserted */
    bsize = backup * 3;

    /* align "p" to last directory separator */
    while ( p [ -1 ] != '/' ) -- p;

    /* the size of the remaining relative path */
    psize = strlen ( p );

    /* open up space if needed */
    if ( (size_t) (p - path) < bsize )
    {
        /* prevent overflow */
        if ( bsize + psize >= path_max )
            return RC ( rcFS, rcDirectory, ctx, rcPath, rcExcessive );
        memmove ( path + bsize, p, psize + 1 /* 1 for '\0'*/ );
    }

    /* insert backup sequences */
    for ( bsize = 0; backup > 0; bsize += 3, -- backup )
        memmove ( & path [ bsize ], "../", 3 );

    /* close gap */
    if ( (size_t) (p - path) > bsize )
        memmove ( & path [ bsize ], p, strlen ( p ) + 1 );

    return 0;
}



static __inline__
rc_t KQuickMountDirVMakePath (const KQuickMountDir * self, enum RCContext ctx, 
                              bool canon, char * buffer, size_t path_max,
                              const char * path, va_list args)
{
    size_t psize;
    size_t bsize;
    rc_t rc;

    if ( path == NULL )
        return RC ( rcFS, rcDirectory, ctx, rcPath, rcNull );
    if ( path [ 0 ] == 0 )
        return RC ( rcFS, rcDirectory, ctx, rcPath, rcInvalid );

    if ( args != NULL && path [ 0 ] == '%' )
    {
        rc = string_vprintf (buffer, path_max, &psize, path, args);

        if (psize > path_max)
            return RC ( rcFS, rcDirectory, ctx, rcPath, rcExcessive );
        if (rc)
            return rc;
        if ( buffer [ 0 ] != '/' )
        {
            bsize = (self->mount - self->path) - 1;
            if ( bsize + psize >= path_max )
                return RC ( rcFS, rcDirectory, ctx, rcPath, rcExcessive );
            memmove ( buffer + bsize, buffer, psize + 1 );
            assert ( self -> path [ bsize - 1 ] == '/' );
            memmove ( buffer, self -> path, bsize );
        }
        else if ( ( bsize = self -> root ) != 0 )
        {
            if ( bsize + psize >= path_max )
                return RC ( rcFS, rcDirectory, ctx, rcPath, rcExcessive );
            memmove ( buffer + bsize, buffer, psize + 1 );
            assert ( self -> path [ bsize - 1 ] != '/' );
            memmove ( buffer, self -> path, bsize );
        }
    }
    else
    {
        if ( path [ 0 ] != '/' )
        {
/*             assert ( self -> path [ self -> size - 1 ] == '/' ); */
            memmove ( buffer, self -> path, bsize = (self->mount - self->path) - 1 );
        }
        else if ( ( bsize = self -> root ) != 0 )
        {
            assert ( self -> path [ bsize - 1 ] != '/' );
            memmove ( buffer, self -> path, bsize );
        }


        rc = string_vprintf ( buffer + bsize, path_max - bsize, &psize, path, args );
        if (bsize + psize >= path_max)
            return RC ( rcFS, rcDirectory, ctx, rcPath, rcExcessive );
        if (rc)
            return rc;
    }

    if ( buffer [ bsize + psize ] == '/' )
        buffer [ bsize + -- psize ] = 0;

    if ( psize > 0 && ( canon || self -> root != 0 ) )
        return KQuickMountDirCanonPath ( self, ctx, buffer, bsize + psize );

    return 0;
}


static __inline__
rc_t KQuickMountDirMakePath (const KQuickMountDir * self, enum RCContext ctx, 
                             bool canon, char * buffer, size_t path_max,
                             const char * path, ...)
{
    va_list args;
    rc_t rc;

    va_start (args, path);
    rc = KQuickMountDirVMakePath (self, ctx, canon, buffer,
                                  path_max, path, args);
    va_end (args);
    return rc;
}


/* ----------------------------------------------------------------------
 * KQuickMountDirDestroy
 */
static rc_t CC KQuickMountDirDestroy (KQuickMountDir *self)
{
    rc_t rc;

    rc = KFileRelease (self->file);

    free (self);

    return rc;
}


/* ----------------------------------------------------------------------
 * KQuickMountDirList
 *  create a directory listing
 *
 *  "list" [ OUT ] - return parameter for list object
 *
 *  "path" [ IN, NULL OKAY ] - optional parameter for target
 *  directory. if NULL, interpreted to mean "."
 *
 * [RET] rc_t					0 for success; anything else for a failure
 *						see itf/klib/rc.h for general details
 * [IN]	 const KQuickMountDir *	self		Object oriented C; KQuickMountDir object for this method
 * [OUT] KNamelist **		listp,
 * [IN]  bool (* 		f	)(const KDirectory*,const char *, void *)
 * [IN]  void *			data
 * [IN]  const char *		path
 * [IN]  va_list		args
 */
static
rc_t CC KQuickMountDirList (const KQuickMountDir *self,
                       KNamelist **listp,
                       bool (CC* f) (const KDirectory *dir, const char *name, void *data),
                       void *data,
                       const char *path,
                       va_list args)
{
    return RC (rcFS, rcDirectory, rcAccessing, rcFunction, rcUnsupported);
}


/* ----------------------------------------------------------------------
 * KQuickMountDirVisit
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
rc_t CC KQuickMountDirVisit (const KQuickMountDir *self, 
                      bool recurse,
                      rc_t (CC* f) (const KDirectory *, uint32_t, const char *, void *), 
                      void *data,
                      const char *path,
                      va_list args)
{
    return RC (rcFS, rcDirectory, rcAccessing, rcFunction, rcUnsupported);
}


/* ----------------------------------------------------------------------
 * KQuickMountDirVisitUpdate
 */
static rc_t CC KQuickMountDirVisitUpdate (KQuickMountDir *self,
                                   bool recurse,
                                   rc_t (CC*f) (KDirectory *,uint32_t,const char *,void *),
                                   void *data,
                                   const char *path,
                                   va_list args)
{
    return RC (rcFS, rcDirectory, rcUpdating, rcFunction, rcUnsupported);
}


/* ----------------------------------------------------------------------
 * KQuickMountDirPathType
 *  returns a KPathType
 *
 *  "path" [ IN ] - NUL terminated string in directory-native character set
 * NOTE: Does not meet a design target of on stack (localized variable) allocation of single 4kb path
 *
 * [RET] uint32_t
 * [IN]  const KQuickMountDir *	self		Object oriented C; KQuickMountDir object for this method
 * [IN]  const char *		path
 * [IN]  va_list		args
 */
static uint32_t CC KQuickMountDirPathType (const KQuickMountDir *self, const char *path, va_list args)
{
    char buff [8192];
    rc_t rc;

    rc = KQuickMountDirVMakePath (self, rcAccessing, true, buff, sizeof buff, path, args);
    if (rc)
        return kptBadPath;

    if (strcmp (buff, self->path) == 0)
        return kptFile;

    return kptNotFound;
}


/* ----------------------------------------------------------------------
 * KQuickMountDirResolvePath
 *
 *  resolves path to an absolute or directory-relative path
 *
 * [IN]  const KQuickMountDir *self		Objected oriented self
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
static rc_t CC KQuickMountDirResolvePath (const KQuickMountDir *self,
                                          bool absolute,
                                          char *resolved,
                                          size_t rsize,
                                          const char *path,
                                          va_list args)
{
    char full [ 8192 ];
    rc_t rc = KQuickMountDirVMakePath ( self, rcResolving, true, full, sizeof full, path, args );
    if ( rc == 0 )
    {
        size_t path_size = strlen ( full );

        if ( absolute )
        {
            /* test buffer capacity */
            if ( path_size - self -> root >= rsize )
                return RC ( rcFS, rcDirectory, rcResolving, rcBuffer, rcInsufficient );

            /* ready to go */
            strcpy ( resolved, & full [ self -> root ] );
/*             assert ( resolved [ 0 ] == '/' ); */
        }
        else
        {
            rc = KQuickMountDirRelativePath ( self, rcResolving, self -> path, full, sizeof full /*path_size*/ );
            if ( rc == 0 )
            {
                path_size = strlen ( full );
                if ( path_size >= rsize )
                    return RC ( rcFS, rcDirectory, rcResolving, rcBuffer, rcInsufficient );
                strcpy ( resolved, full );
            }
        }
    }
    return rc;
}


/* ----------------------------------------------------------------------
 * KQuickMountDirResolveAlias
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
static rc_t CC KQuickMountDirResolveAlias (const KQuickMountDir * self, 
				 bool absolute,
				 char * resolved,
				 size_t rsize,
				 const char *alias,
				 va_list args)
{
    return RC (rcFS, rcDirectory, rcAccessing, rcFunction, rcUnsupported);
}

/* ----------------------------------------------------------------------
 * KQuickMountDirRename
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
rc_t CC KQuickMountDirRename (KQuickMountDir *self, bool force, const char *from, const char *to)
{
    assert (self != NULL);
    assert (from != NULL);
    assert (to != NULL);

    return RC (rcFS, rcDirectory, rcUpdating, rcSelf, rcUnsupported);
}

/* ----------------------------------------------------------------------
 * KQuickMountDirRemove
 *  remove an accessible object from its directory
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target object
 *
 *  "force" [ IN ] - if non-zero and target is a directory,
 *  remove recursively
 */
static
rc_t CC KQuickMountDirRemove (KQuickMountDir *self, bool force, const char *path, va_list args)
{
    assert (self != NULL);
    assert (path != NULL);

    return RC (rcFS, rcDirectory, rcUpdating, rcSelf, rcUnsupported);
}

/* ----------------------------------------------------------------------
 * KQuickMountDirClearDir
 *  remove all directory contents
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target directory
 *
 *  "force" [ IN ] - if non-zero and directory entry is a
 *  sub-directory, remove recursively
 */
static
rc_t CC KQuickMountDirClearDir (KQuickMountDir *self, bool force, const char *path, va_list args)
{
    assert (self != NULL);
    assert (path != NULL);

    return RC (rcFS, rcDirectory, rcUpdating, rcSelf, rcUnsupported);
}


/* ----------------------------------------------------------------------
 * KQuickMountDirAccess
 *  get access to object
 *
 *  "access" [ OUT ] - return parameter for Unix access mode
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target object
 * NOTE: Does not meet a design target of on stack (localized variable) allocation of single 4kb path
 */
static rc_t CC KQuickMountDirVAccess (const KQuickMountDir *self,
			    uint32_t *access,
			    const char *path,
			    va_list args)
{
    return RC (rcFS, rcDirectory, rcAccessing, rcFunction, rcUnsupported);
}


/* ----------------------------------------------------------------------
 * KQuickMountDirSetAccess
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
static rc_t CC KQuickMountDirSetAccess (KQuickMountDir *self,
			      bool recurse,
			      uint32_t access,
			      uint32_t mask,
			      const char *path,
			      va_list args)
{
    assert (self != NULL);
    assert (path != NULL);

    return RC (rcFS, rcDirectory, rcUpdating, rcSelf, rcUnsupported);
}


/* ----------------------------------------------------------------------
 * KQuickMountDirCreateAlias
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
rc_t CC KQuickMountDirCreateAlias (KQuickMountDir *self,
			 uint32_t access,
			 KCreateMode mode,
			 const char *targ,
			 const char *alias)
{
    assert (self != NULL);
    assert (targ != NULL);
    assert (alias != NULL);

    return RC (rcFS, rcDirectory, rcCreating, rcSelf, rcUnsupported);
}


/* ----------------------------------------------------------------------
 * KQuickMountDirOpenFileRead
 *  opens an existing file with read-only access
 *
 *  "f" [ OUT ] - return parameter for newly opened file
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target file
 * NOTE: Does not meet a design target of on stack (localized variable) allocation of single 4kb path
 */
static
rc_t CC KQuickMountDirOpenFileRead	(const KQuickMountDir *self,
					 const KFile **f,
					 const char *path,
					 va_list args)
{
    return RC (rcFS, rcDirectory, rcOpening, rcSelf, rcUnsupported);
}


/* ----------------------------------------------------------------------
 * KQuickMountDirOpenFileWrite
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
rc_t CC KQuickMountDirOpenFileWrite	(KQuickMountDir *self,
					 KFile **f,
					 bool update,
					 const char *path,
					 va_list args)
{
    assert (self != NULL);
    assert (f != NULL);
    assert (path != NULL);

    return RC (rcFS, rcDirectory, rcCreating, rcSelf, rcUnsupported);
}


/* ----------------------------------------------------------------------
 * KQuickMountDirCreateFile
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
rc_t CC KQuickMountDirCreateFile	(KQuickMountDir *self,
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

    return RC (rcFS, rcDirectory, rcCreating, rcSelf, rcUnsupported);
}


/* ----------------------------------------------------------------------
 * KQuickMountDirFileSize
 *  returns size in bytes of target file
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target file
 *
 *  "size" [ OUT ] - return parameter for file size
 * NOTE: Does not meet a design target of on stack (localized variable) allocation of single 4kb path
 */
static
rc_t CC KQuickMountDirFileSize		(const KQuickMountDir *self,
					 uint64_t *size,
					 const char *path,
					 va_list args)
{
    return RC (rcFS, rcDirectory, rcCreating, rcSelf, rcUnsupported);
}


/* ----------------------------------------------------------------------
 * KQuickMountDirSetFileSize
 *  sets size in bytes of target file
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target file
 *
 *  "size" [ IN ] - new file size
 */
static
rc_t CC KQuickMountDirSetFileSize	(KQuickMountDir *self,
					 uint64_t size,
					 const char *path,
					 va_list args)
{
    assert (self != NULL);
    assert (path != NULL);

    return RC (rcFS, rcDirectory, rcWriting, rcSelf, rcUnsupported);
}


/* ----------------------------------------------------------------------
 * KQuickMountDirOpenDirRead
 *
 *  opens a sub-directory
 *
 * [IN]  const KQuickMountDir *	self	Object Oriented C KQuickMountDir self
 * [OUT] const KDirectory **	subp	Where to put the new KDirectory/KQuickMountDir
 * [IN]  bool			chroot	Create a chroot cage for this new subdirectory
 * [IN]  const char *		path	Path to the directory to open
 * [IN]  va_list		args	So far the only use of args is possible additions to path
 */
static 
rc_t CC KQuickMountDirOpenDirRead	(const KQuickMountDir *self,
					 const KDirectory **subp,
					 bool chroot,
					 const char *path,
					 va_list args)
{
    return RC (rcFS, rcDirectory, rcOpening, rcSelf, rcUnsupported);
}


/* ----------------------------------------------------------------------
 * KQuickMountDirOpenDirUpdate
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
rc_t CC KQuickMountDirOpenDirUpdate	(KQuickMountDir *self,
					 KDirectory ** subp, 
					 bool chroot, 
					 const char *path, 
					 va_list args)
{
    assert (self != NULL);
    assert (subp != NULL);
    assert (path != NULL);

    return RC (rcFS, rcDirectory, rcUpdating, rcSelf, rcUnsupported);
}

/* ----------------------------------------------------------------------
 * KQuickMountDirCreateDir
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
rc_t CC KQuickMountDirCreateDir	(KQuickMountDir *self,
					 uint32_t access,
					 KCreateMode mode,
					 const char *path,
					 va_list args)
{
    assert (self != NULL);
    assert (path != NULL);

    return RC (rcFS, rcDirectory, rcCreating, rcSelf, rcUnsupported);
}


/* ----------------------------------------------------------------------
 * KQuickMountDirDestroyFile
 */
#if 0
static
rc_t CC KQuickMountDirDestroyFile	(KQuickMountDir *self,
					 KFile * f)
{
    assert (self != NULL);
    assert (f != NULL);

    return RC (rcFS, rcDirectory, rcDestroying, rcSelf, rcUnsupported);
}


static	rc_t CC KQuickMountDirDate (const KQuickMountDir *self,
					 KTime_t *date,
					 const char *path,
					 va_list args)
{
    return RC (rcFS, rcDirectory, rcAccessing, rcSelf, rcUnsupported);

}


static	rc_t CC KQuickMountDirSetDate		(KQuickMountDir *self,
					 bool recurse,
					 KTime_t date,
					 const char *path,
					 va_list args)
{
    assert (self != NULL);
    assert (path != NULL);

    return RC (rcFS, rcDirectory, rcUpdating, rcSelf, rcUnsupported);
}
#endif

/* ----------------------------------------------------------------------
 *
 */
static KDirectory_vt_v1 vtKQuickMountDir =
{
    /* version 1.0 */
    1, 0,

    /* start minor version 0 methods*/
    KQuickMountDirDestroy,
    KQuickMountDirList,
    KQuickMountDirVisit,
    KQuickMountDirVisitUpdate,
    KQuickMountDirPathType,
    KQuickMountDirResolvePath,
    KQuickMountDirResolveAlias,
    KQuickMountDirRename,
    KQuickMountDirRemove,
    KQuickMountDirClearDir,
    KQuickMountDirVAccess,
    KQuickMountDirSetAccess,
    KQuickMountDirCreateAlias,
    KQuickMountDirOpenFileRead,
    KQuickMountDirOpenFileWrite,
    KQuickMountDirCreateFile,
    KQuickMountDirFileSize,
    KQuickMountDirSetFileSize,
    KQuickMountDirOpenDirRead,
    KQuickMountDirOpenDirUpdate,
    KQuickMountDirCreateDir,
    NULL,
    /* end minor version 0 methods*/
    /* start minor version 1 methods*/
    NULL,
    NULL,
    NULL,
    /* end minor version 2 methods*/
    NULL,
    /* end minor version 2 methods*/
    /* end minor version 3 methods*/
    NULL,
    NULL
    /* end minor version 3 methods*/
};


static KQuickMountDir * KQuickMountDirAlloc (size_t path_size, size_t mount_size)
{
    return calloc (sizeof (KQuickMountDir) + path_size + mount_size + 1, 1);
}


KFS_EXTERN rc_t KQuickMountDirMake (const KDirectory * self, 
                                    const KDirectory ** pnewdir,
                                    const KFile * file,
                                    const char * path, size_t path_size,
                                    const char * mount, size_t mount_size)
{
    KQuickMountDir * newdir;
    char * pc;
    rc_t rc;
    bool endslash;
    char tbuff [8193];

    endslash = path[path_size-1] == '/';

    if (sizeof tbuff - 1 < mount_size)
        return RC (rcFS, rcDirectory, rcCreating, rcBuffer, rcInsufficient);

    memmove (tbuff, mount, mount_size);
    tbuff[mount_size] = '\0';

    while ((pc = strpbrk (tbuff, "\"\\/")) != NULL)
        *pc = '_';

    rc = KFileAddRef (file);
    if (rc == 0)
    {
        newdir = KQuickMountDirAlloc (path_size, mount_size);
        if (newdir == NULL)
            rc = RC (rcFS, rcDirectory, rcAllocating, rcMemory, rcExhausted);
        else
        {
            newdir->file = file;
            newdir->mount = newdir->path + path_size;
            rc = KQuickMountDirMakePath (newdir, rcCreating, true, newdir->path,
                                         path_size + 1 + mount_size + 1,
                                         endslash?"%s%s":"%s/%s", path, tbuff);
            if (rc == 0)
            {
                rc = KDirectoryInit (&newdir->dad, (const KDirectory_vt*) &vtKQuickMountDir,
                                     "KQuickMountDir", path?path:"(null)", false);
                if (rc == 0)
                {
                    newdir->mount = newdir->path + path_size + 1;
                    newdir->root = 0;
                    *pnewdir = &newdir->dad;
                    return 0;
                }
/*             rc = RC (rcFS, rcDirectory, rcInitializing, rcObject, rcInvalid); */
            }
            KQuickMountDirDestroy (newdir);
        }
        KFileRelease (file);
    }
    return rc;
}
                         


/* end of file quickmount.c */

