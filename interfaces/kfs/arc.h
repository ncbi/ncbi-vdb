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

#ifndef _h_kfs_arc_
#define _h_kfs_arc_

#ifndef _h_kfs_extern_
#include <kfs/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#include <kfs/toc.h>

#ifdef __cplusplus
extern "C" {
#endif

struct KDirectory;
struct KFile;
struct KToc;
struct vector;

/* validate_header_offsets
 *	This function is a special friend function to the archive package used
 *	to validate that the compiler used to build the package matches 
 *	used structure offsets "correctly" to enable the parsing of TAR files.
 * NOTE:
 *	This function should not be used except by ktartest (klib/tools/ktartest.c).
 *	It is built only in a special logging version of the libraries.
 */
KFS_EXTERN bool CC validate_header_offsets(void);

typedef struct KArcDir KArcDir;
typedef struct KArcFile KArcFile;

typedef uint32_t KArcFSType;
enum eKArcFSType
{
    tocUnknown,
    tocKFile,
    tocKDirectory,
    tocKVirtual
};


/* OpenArcDirRead
 *
 * This should probably move into the toc-priv with a specific call to open each archive type
 * with the parse private and not needing the dad-gummed CC and KFS_EXTERN
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
KFS_EXTERN rc_t CC KDirectoryOpenArcDirRead(const struct KDirectory * self, 
    const struct KDirectory ** pdir, bool chroot, const char * path, KArcFSType baseType,
    rc_t ( CC * parse )(struct KToc *,const void *,
        bool( CC * )(const struct KDirectory*, const char *, void*), void*),
        bool (CC* filter )(const KDirectory*, const char *, void *), void * filterparam);

KFS_EXTERN rc_t CC KDirectoryOpenArcDirRead_silent(const struct KDirectory * self, 
    const struct KDirectory ** pdir, bool chroot, const char * path, KArcFSType baseType,
    rc_t ( CC * parse )(struct KToc *,const void *,
        bool( CC * )(const struct KDirectory*, const char *, void*), void*),
        bool (CC* filter )(const KDirectory*, const char *, void *), void * filterparam);

KFS_EXTERN rc_t CC KDirectoryOpenArcDirRead_silent_preopened(const struct KDirectory * self, 
    const struct KDirectory ** pdir, bool chroot, const char * path, KArcFSType baseType,
    void * f,
    rc_t ( CC * parse )(struct KToc *,const void *,
        bool( CC * )(const struct KDirectory*, const char *, void*), void*),
        bool (CC* filter )(const KDirectory*, const char *, void *), void * filterparam);





#if 0
/* ----------------------------------------------------------------------
 * KDirectoryToKArcDir
 *
 * [IN]  const KDirectory * 	self	Object oriented C
 * [OUT] const KArcDir * 	cast	Object oriented C
 *
 *
 * Get a reference to a KArcDir from a KDirectory as a cast.  It is a new reference.
 */

KFS_EXTERN rc_t CC KDirectoryToKArcDir (const KDirectory * self, const KArcDir ** cast);

KFS_EXTERN rc_t CC KArcDirList ( const KArcDir *self, struct KNamelist **list,
    bool ( CC * f ) ( const KArcDir *dir, const char *name, void *data ),
    void *data, const char *path, ... );
KFS_EXTERN rc_t CC KArcDirVList ( const KArcDir *self, struct KNamelist **list,
    bool ( CC * f ) ( const KArcDir *dir, const char *name, void *data ),
    void *data, const char *path, va_list args );
#endif



#ifdef __cplusplus
}
#endif

#endif /* _h_kfs_arc_ */
