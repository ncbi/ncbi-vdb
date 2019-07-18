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
 
#include <kfs/extern.h>
#include "toc-priv.h"
#include <klib/rc.h>
#include <klib/defs.h>
#include <klib/namelist.h>
#include <kfs/directory.h>
#include <kfs/toc.h>
#include <klib/debug.h>
#include <klib/log.h>
#include <klib/out.h>
#include <sysalloc.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

static
rc_t KTocProcessOneNameCommonAtt (const KDirectory * dir,
				  const char * path,
				  KTime_t * mtime,
				  uint32_t * access)
{
    rc_t rc;

    *mtime = 0;
    *access = 0;
    rc = KDirectoryDate (dir, mtime, "%s", path);
    if (rc != 0)
    {
	PLOGERR (klogErr, (klogErr, rc, "Failure getting mtime of $(p)", PLOG_S(p), path));
    }
    else
    {
        rc = KDirectoryAccess (dir, access, "%s", path);
	if (rc != 0)
	{
	    PLOGERR (klogErr, (klogErr, rc, "Failure getting access of $(p)", PLOG_S(p), path));
	}
    }
    TOC_DEBUG (("%s path %s ktime_t %lu access %o\n", __func__, path, *mtime, *access));
    return rc;
}


/* ----------------------------------------------------------------------
 * KTocParseKDirRecurOnePath
 *
 * take a single path name and add it to the TOC
 *
 * KToc *			self	read/write	the TOC being built
 * const KDirectory *		dir	read		the directory owning the path
 * bool (*)(const KDirectory*,
 *          const char *,
 *          void *)		filter	read/execute	a filtering function for path inclusion
 * void *			data	read		passed in as third parameter to filter
 * const char *			name	read		the path to be added
 *
 * filter and data can be NULL
 */
static
rc_t KTocParseKDirRecur	(KToc * self,
			 const KDirectory * dir,
			 const char * path,
			 bool (CC* filter )(const KDirectory *,const char *, void*),
			 void *  data);
             
static
rc_t KTocParseKDirRecurOnePath (KToc * self,
				   const KDirectory * dir,
				   const char * path,
				   bool (CC* filter )(const KDirectory *,const char *, void*),
				   void *  data)
{
    rc_t rc;
    KPathType type;
    KTime_t mtime;
    uint32_t access;
    uint64_t size;

    assert (self != NULL);
    assert (dir != NULL);
    assert (path != NULL);

    TOC_DEBUG (("%s: recur path for %s\n", __func__, path));

    type = (KPathType)KDirectoryPathType (dir, "%s", path);

    if (type & kptAlias)
    {
	size = 0;
	rc = KTocProcessOneNameCommonAtt (dir, path, &mtime, &access);
	if (rc == 0)
	{
	    char * res;
	    char * tres;
	    size_t res_sz;

	    res = NULL;
	    rc = RC ( rcFS, rcDirectory, rcResolving, rcBuffer, rcInsufficient );
	    for (res_sz = 128; 
		 rc == RC ( rcFS, rcDirectory, rcResolving, rcBuffer, rcInsufficient );
		 res_sz <<= 1)
	    {
		tres = realloc (res, res_sz);
		if (tres == NULL)
		{
		    if (res != NULL)
			free (res);
		    return RC (rcFS, rcToc, rcParsing, rcMemory, rcExhausted);
		}
		res = tres;
        rc = KDirectoryResolveAlias (dir, false, res, res_sz, "%s", path );
	    }
	    if (rc != 0)
	    {
		PLOGERR (klogWarn, (klogWarn, rc, "Failure getting alias target of $(p)", PLOG_S(p), path));
	    }
	    else
	    {
		rc = KTocCreateSoftLink (self, mtime, access, kcmCreate, res, path);
	    }
	    if (res != NULL)
		free (res);
	}
    }
    else
    {
	switch (type)
	{
	default:
	    /* -----
	     * fail on anything we don't understand
	     */
	    rc = RC (rcFS, rcToc, rcConstructing, rcToc, rcUnexpected);
	    PLOGERR (klogInt, (klogInt, rc, "Failure getting type for $(p)", PLOG_S(p), path));
	    break;
	case kptCharDev:
	case kptBlockDev:
	case kptFIFO:
	    /* -----
	     * silently drop all devices and fifo/queues
	     */
	    rc = 0;
	    break;
	case kptFile:
		/* TBD: add support here for chunked files and for hard links
		 *
		 * use lstat (not stat) to see if there are more than one link.  
		 * if so then this inode will end up in a bstree of possible doubly
		 * linked files.  first time we find it put in in the toc as a file,
		 * while on subsequent times put it in as a hardlink
		 *
		 * Don't know how to detect sparse files yet (other than this from wikipedia)
		 *
		 * http://en.wikipedia.org/wiki/Sparse_files#Detecting_sparse_files_in_Unix
		 * Sparse files have different apparent and actual file sizes. This can be
		 * detected by comparing the output of:
		 *
		 *	du -s -B1 --apparent-size sparse-file
		 *
		 * and:
		 *
		 *	du -s -B1 sparse-file
		 *
		 */
	    rc = KDirectoryFileSize (dir, &size, "%s", path);
	    if (rc != 0)
	    {
		PLOGERR (klogErr, (klogErr, rc, "Failure getting size of $(p)", PLOG_S(p), path));
	    }
	    else
	    {
		rc = KTocProcessOneNameCommonAtt (dir, path, &mtime, &access);
		if (rc == 0)
		{
/* eventually we can choose to check for a compression */
		    rc = KTocCreateFile (self, 0/*place holder*/, size, mtime, 
                                          access, kcmCreate, path);
		}
	    }
	    break;
	case kptDir:
	    size = 0;
	    rc = KTocProcessOneNameCommonAtt (dir, path, &mtime, &access);
	    if (rc == 0)
	    {
		rc = KTocCreateDir (self, mtime, access, kcmCreate, path);
		if (rc == 0)
		{
		    rc = KTocParseKDirRecur (self, dir, path, filter, data);
		}
	    }
	    break;
	} /* switch (type) */
    } /* if (type & kptAlias) ... else ... */
    return rc;
}


static
rc_t KTocParseKDirRecur	(KToc * self,
			 const KDirectory * dir,
			 const char * path,
			 bool (CC* filter )(const KDirectory *,const char *, void*),
			 void *  data)
{
    rc_t rc;
    KNamelist * names;

    assert (self != NULL);
    assert (dir != NULL);
    assert (path != NULL);

    TOC_DEBUG (("%s: recur path for %s\n", __func__, path));

    /* -----
     * get a list of files in this directory
     */
    rc = KDirectoryList (dir, &names, NULL, NULL, "%s", path);
    if (rc != 0)
    {
	LOGERR (klogErr, rc, "Failure to build a name list");
    }
    else
    {
	/* -----
	 * get the length of the list
	 */
	uint32_t limit;

	rc = KNamelistCount (names, &limit);
	if (rc != 0)
	{
	    LOGERR (klogErr, rc, "Failure to count elements of a name list");
	}
	else
	{
	    /* -----
	     * loop through the list
	     */
	    uint32_t idx;
	    size_t pathlen;

            TOC_DEBUG (("%s KNamelistCount counted %u\n", __func__, limit));
	    pathlen = strlen (path);

	    for (idx = 0; idx < limit; ++idx)
	    {
		/* -----
		 * get the nth path name from the list
		 */
		const char * name;

		rc = KNamelistGet (names, idx, &name);
		if (rc != 0)
		{
		    LOGERR (klogErr, rc, "Failure to pull name from name list");
		}
		else
		{
		    char * new_path;
		    size_t namelen;
		    size_t new_pathlen;

                    TOC_DEBUG(("%s KNamelistCount  %s\n", __func__, name));
            
		    namelen = strlen (name);
		    /* length of previous path + '/' + length of name */
		    new_pathlen = pathlen + 1 + namelen;
		    new_path = malloc (new_pathlen + 1);  /* + NUL */

		    if (new_path == NULL)
		    {
			rc = RC (rcFS, rcToc, rcConstructing, rcMemory, rcExhausted);
			LOGERR (klogErr, rc, "out of memory building paths for TOC");
		    }
		    else
		    {
			bool use_name;
			char * recur_path;
                        size_t recur_path_z;

			memmove (new_path, path, pathlen);
			new_path[pathlen] = '/';
			memmove (new_path + pathlen + 1, name, namelen);
			new_path[pathlen+1+namelen] = '\0';

#if 1
                        recur_path_z = pathlen + 1 + namelen + 1;
#else
                        recur_path_z = 4096;
#endif
 			recur_path = malloc (recur_path_z);
			if (recur_path == NULL)
			{
			    rc = RC (rcFS, rcToc, rcConstructing, rcMemory, rcExhausted);
			    LOGERR (klogErr, rc, "out of memory building paths for TOC");
			}
			else
			{
			    rc = KDirectoryResolvePath (dir, false, recur_path, recur_path_z,
                                            "%s", new_path);
                            TOC_DEBUG (("%s: resolved path %s for %s\n", __func__, recur_path, new_path));
                            TOC_DEBUG (("%s: filter %p\n", __func__, filter));
			    if (rc == 0)
			    {
				if (filter != NULL)
				{
				    use_name = filter (dir, recur_path, data);
				}
				else
				    use_name = true;

				if (use_name)
				{
                                    TOC_DEBUG (("%s: use_name true for %s\n", __func__, recur_path));
				    rc = KTocParseKDirRecurOnePath (self, dir, recur_path, filter, data);
				    if (rc != 0)
				    {
					PLOGERR (klogErr, (klogErr, rc, "Failure to process name from name list $(p)",
                                                           PLOG_S(p), name));
				    }
				}
			    }
			    free (recur_path);
			}
			free (new_path);
		    }
		}
		if (rc != 0)
		    break;
	    } /* for (idx = 0; idx < limit; ++idx) */
	}
	KNamelistRelease (names);
    }
    return rc;
}

/* ======================================================================
 * KTocParseKDir
 *
 * This matches the standard signature for creating a KToc with a
 * KDirectory (any implementation including another KArcDir) as the
 * source to parse.
 *
 * Filter is a reference to a function that given a directory, a path
 * as a char ASCIZ string and a pointer to some form of data probably 
 * a struct will return 0 for do not include and 1 for include that
 * path in the TOC.
 *
 * Filter can be NULL and no filtering function will be called.
 * Data can be NULL is there is no Filter that expects it to b non-null.
 */
LIB_EXPORT rc_t CC KArcParseKDir ( KToc * self,
                                   const void * kvoid,
                                   bool(CC*filter)(const KDirectory*,const char *, void*),
                                   void * data )
{
    const KDirectory * kdir;
    rc_t rc;

    kdir = kvoid;
    rc = 0;

    if (self == NULL)
    {
	rc = RC (rcFS, rcArc, rcParsing, rcSelf, rcNull);;
	LOGERR (klogErr, rc, "KTocParseKDir: self was NULL");
    }
    else if (kdir == NULL)
    {
	rc = RC (rcFS, rcArc, rcParsing, rcParam, rcNull);;
	LOGERR (klogErr, rc, "KTocParseKDir: kdir was NULL");
    }
    else
    {
        TOC_DEBUG (("Start Parse\n"));
	rc = KTocParseKDirRecur (self, kdir, ".", filter, data);
    }
    return rc;
}

/* end of file */
