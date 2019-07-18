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

#include <klib/defs.h>
#include <byteswap.h>
#include <klib/rc.h>
#include <kfs/file.h>
#include <kfs/sra.h>
/* #include <kfs/toc.h> */
#include <klib/log.h>
#include "toc-priv.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sysalloc.h>


struct KToc;
struct KDirectory;



static
const KSraHeader ksraheader_v1 =
{
    { 'N', 'C','B', 'I' },
    { '.', 's','r', 'a' },
    eSraByteOrderTag,
    1
};

LIB_EXPORT size_t CC SraHeaderSize ( const KSraHeader * self )
{
    /* self can be NULL */
    return sizeof ( KSraHeader );
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

static void	SraHeaderInit (KSraHeader * self, size_t treesize, KSRAFileAlignment alignment)
{
    memmove (self, &ksraheader_v1, sizeof ksraheader_v1);
    self->u.v1.file_offset = add_filler (sizeof * self + treesize, alignment);
}

rc_t SraHeaderMake (KSraHeader ** pself, size_t treesize, KSRAFileAlignment alignment)
{
    KSraHeader * self = malloc (sizeof (KSraHeader));
    if (self == NULL)
	return RC ( rcFS, rcToc, rcConstructing, rcMemory, rcExhausted );
    SraHeaderInit (self, treesize, alignment);
    *pself = self;
    return 0;
}


static rc_t  SraHeaderValidate ( const KSraHeader * self, bool * reverse, uint32_t * _version, size_t bytes_available )
{
    uint64_t offset;
    rc_t rc;
    uint32_t version;
    bool rev;

    if (reverse != NULL)
	*reverse = false;
    if (_version != NULL)
	*_version = 0;

    if (bytes_available < 8)
        return RC (rcFS, rcArc, rcParsing, rcHeader, rcInsufficient);

    if (memcmp ((void*)self, &ksraheader_v1,
		sizeof (ksraheader_v1.ncbi) + sizeof (ksraheader_v1.sra)) != 0)
	return RC (rcFS, rcArc, rcParsing, rcHeader, rcInvalid);

    if (bytes_available < 12)
        return 0;

    switch (self->byte_order)
    {
    default:
	return RC (rcFS, rcArc, rcParsing, rcHeader, rcCorrupt);
    case eSraByteOrderTag:
	rev = false;
	break;
    case eSraByteOrderReverse:
	rev = true;
	break;
    }
    rc = 0;

    if (bytes_available < 16)
        return 0;

    version = rev ? bswap_32 (self->version) : self->version;
    offset = rev ? bswap_64 (self->u.v1.file_offset) : self->u.v1.file_offset;

    if (version > FS_SRA_CUR_VERSION)
	rc =  RC (rcFS, rcArc, rcParsing, rcHeader, rcUnsupported);

    /* 4 is minimum size for a TOC */
    else if (offset < (sizeof ksraheader_v1 + 4))
	return RC (rcFS, rcArc, rcParsing, rcHeader, rcCorrupt);

    if (reverse != NULL)
	*reverse = rev;
    if (_version != NULL)
	*_version = version;

    return rc;
}

LIB_EXPORT uint64_t CC SraHeaderGetFileOffset ( const KSraHeader * self )
{
    if ( self -> byte_order == eSraByteOrderReverse )
        return bswap_64 ( self->u.v1.file_offset );
    return self->u.v1.file_offset;
}

static
rc_t KTocParseRead (const KFile * f, uint64_t p, void * b, size_t z, size_t * n)
{
    rc_t rc;
    size_t a;
    size_t t;

    rc = 0;
    for (a = 0; a < z; a += t)
    {
	rc = KFileRead (f, p + a, (uint8_t*)b + a, z - a, &t);
	if (rc != 0)
	    return rc;
	if (t == 0)
	    break;
    }
    if (a < z)
	rc = RC (rcFS, rcToc, rcParsing, rcFile, rcTooShort);
    *n = a;
    return rc;
}


static
rc_t KTocParseReadPBSTree (struct KToc * self, const KFile * file, uint64_t endpos, void ** header)
{
    rc_t rc;
    uint64_t toc_pos;
    uint64_t end_pos;
    size_t num_read;
    size_t tree_size;

    rc = 0;
    toc_pos = SraHeaderSize(NULL);
    end_pos = endpos;

    if (end_pos <= toc_pos)
    {
        /* 	rc = RC (rcFS, rcToc, rc */
    }
    tree_size = (size_t)(end_pos - toc_pos);

    *header = malloc (tree_size);
    if (*header == NULL)
    {
        rc = RC (rcFS, rcToc, rcParsing, rcMemory, rcExhausted);
    }
    else
    {
        rc = KTocParseRead (file, toc_pos, *header, tree_size, &num_read);
        if (rc == 0)
        {
            return 0;
        }
        free (*header);
        *header = NULL;
    }
    return rc;
}

static
rc_t CC KArcParseSRAInt ( struct KToc * self,
                          const void * kvoid,
                          bool ( CC * ignored ) ( const struct KDirectory *,  const char *, void * ),
                          void * ignored_data, 
                          bool unbounded,
                          bool silent )
{
    uint64_t arcsize;
    rc_t rc;
    bool reverse;
    uint32_t version;
    KSraHeader header;
    size_t num_read;
    size_t all_read;
    const KFile * file;
    void * pbstreeBuffer;
    /*     uint64_t toc_offset; */
    /*     uint64_t file_offset; */

    rc = 0;
    if ( kvoid == NULL )
    {
        rc = RC ( rcFS, rcToc, rcParsing, rcParam, rcNull );
        if ( !silent )
            LOGERR ( klogErr, rc, "File parameter null for parsing .sra" );
        return rc;
    }
    file = kvoid;

    rc = KFileSize ( file, &arcsize );
    if (rc)
    {
        if ( unbounded )
            arcsize = ( (uint64_t)0 ) - 1;
        else
        {
            rc = RC ( rcFS, rcToc, rcParsing, rcFile, rcIncorrect );
            if ( !silent )
                LOGERR ( klogErr, rc, "Can't determine archive file size" );
            return rc;
        }
    }

    for ( num_read = all_read = 0; all_read < sizeof (header); all_read += num_read )
    {
        rc = KTocParseRead ( file, 0, &header, sizeof (header), &num_read );
        if ( num_read == 0 )
        {
            rc = RC ( rcFS, rcToc, rcParsing, rcFile, rcTooShort );
            break;
        }
    }
    if ( rc != 0 )
    {
        if ( !silent )
            LOGERR ( klogErr, rc, "error reading file to parse .sra" );
        return rc;
    }

    /* version is ignored at this point as there is only one version */
    rc = SraHeaderValidate ( &header, &reverse, &version, sizeof header );
    if ( rc != 0 )
    {
        if ( !silent )
            LOGERR ( klogErr, rc, "File header invalid for .sra header" );
        return rc;
    }

    switch ( version )
    {
    default:
        rc = RC ( rcFS, rcToc, rcParsing, rcFileFormat, rcBadVersion );
        if ( !silent )
            LOGERR ( klogInt, rc, "Incompatible file version" );
        break;
    case 1:
        rc = KTocParseReadPBSTree ( self, file, SraHeaderGetFileOffset (&header),
                                   &pbstreeBuffer );
        if ( rc != 0 )
        {
            if ( !silent )
                LOGERR ( klogErr, rc, "File TOC not read for .sra header" );
            break;
        }
        else
        {
            uint64_t offset;
            offset = SraHeaderGetFileOffset (&header);
            rc = KTocInflatePBSTree ( self, arcsize, pbstreeBuffer, 
                                     (uint32_t)(offset - sizeof (header)),
                                     offset,
                                     reverse, "" );
            free ( pbstreeBuffer );
            if ( rc != 0 && !silent )
            {
                LOGERR (klogErr, rc, "File TOC not valid for .sra");
            }
            /* 	    else */
            /* 	    { */
            /* 		rc = KTocParseSraDirectory (self, bst, ""); */
            /* 		if (rc == 0) */
            /* 		    return 0; */
            /* 	    } */
        }
        break;
    }
    return rc;
}

LIB_EXPORT rc_t CC KArcParseSRA ( struct KToc * self,
                                  const void * kvoid,
                                  bool (CC*ignored) (const struct KDirectory*,  const char*, void*),
                                  void *ignored_data )
{
    return KArcParseSRAInt ( self, kvoid, ignored, ignored_data, false, false );
}


LIB_EXPORT rc_t CC KArcParseSRA_silent ( struct KToc * self,
                                  const void * kvoid,
                                  bool (CC*ignored) (const struct KDirectory*,  const char*, void*),
                                  void *ignored_data )
{
    return KArcParseSRAInt ( self, kvoid, ignored, ignored_data, false, true );
}

LIB_EXPORT rc_t CC KArcParseSRAUnbounded ( struct KToc * self,
                                  const void * kvoid,
                                  bool (CC*ignored) (const struct KDirectory*,  const char*, void*),
                                  void *ignored_data )
{
    return KArcParseSRAInt ( self, kvoid, ignored, ignored_data, true, false );
}

LIB_EXPORT rc_t CC KArcParseSRAUnbounded_silent ( struct KToc * self,
                                  const void * kvoid,
                                  bool (CC*ignored) (const struct KDirectory*,  const char*, void*),
                                  void *ignored_data )
{
    return KArcParseSRAInt ( self, kvoid, ignored, ignored_data, true, false );
}


LIB_EXPORT rc_t CC KDirectoryVOpenSraArchiveRead ( struct KDirectory const *self,
    struct KDirectory const **sra_dir, int chroot, const char *path, va_list args )
{
    /* respond properly to va_list */
    char full [ 4096 ];
    int len = vsnprintf ( full, sizeof full, path, args );
    if ( len < 0 )
        return RC ( rcFS, rcDirectory, rcOpening, rcPath, rcExcessive );

    /* putting off parameter validation into this call */
    return KDirectoryOpenArcDirRead (self, sra_dir, chroot, full, tocKFile,
                                     KArcParseSRA, NULL, NULL);
}


LIB_EXPORT rc_t CC KDirectoryOpenSraArchiveRead ( struct KDirectory const *self,
    struct KDirectory const **sra_dir, int chroot, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDirectoryVOpenSraArchiveRead ( self, sra_dir, chroot, path, args );
    va_end ( args );

    return rc;
}


LIB_EXPORT rc_t CC KDirectoryVOpenSraArchiveRead_silent ( struct KDirectory const *self,
    struct KDirectory const **sra_dir, int chroot, const char *path, va_list args )
{
    /* respond properly to va_list */
    char full [ 4096 ];
    int len = vsnprintf ( full, sizeof full, path, args );
    if ( len < 0 )
        return RC ( rcFS, rcDirectory, rcOpening, rcPath, rcExcessive );

    /* putting off parameter validation into this call */
    return KDirectoryOpenArcDirRead_silent ( self, sra_dir, false, full, tocKFile,
                                     KArcParseSRA_silent, NULL, NULL );
}


LIB_EXPORT rc_t CC KDirectoryOpenSraArchiveRead_silent ( struct KDirectory const *self,
    struct KDirectory const **sra_dir, int chroot, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDirectoryVOpenSraArchiveRead_silent ( self, sra_dir, chroot, path, args );
    va_end ( args );

    return rc;
}


LIB_EXPORT rc_t CC KDirectoryVOpenSraArchiveReadUnbounded_silent_preopened ( struct KDirectory const *self,
    struct KDirectory const **sra_dir, int chroot, const struct KFile * f, const char *path, va_list args )
{
    /* respond properly to va_list */
    char full [ 4096 ];
    int len = vsnprintf ( full, sizeof full, path, args );
    if ( len < 0 || len >= sizeof full )
        return RC ( rcFS, rcDirectory, rcOpening, rcPath, rcExcessive );

    /* putting off parameter validation into this call */
    return KDirectoryOpenArcDirRead_silent_preopened ( self, sra_dir, false, full, tocKFile,
        ( void* ) f, KArcParseSRAUnbounded_silent, NULL, NULL );
}


LIB_EXPORT rc_t CC KDirectoryOpenSraArchiveReadUnbounded_silent_preopened ( struct KDirectory const *self,
    struct KDirectory const **sra_dir, int chroot, const struct KFile * f, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDirectoryVOpenSraArchiveReadUnbounded_silent_preopened ( self, sra_dir, chroot, f, path, args );
    va_end ( args );

    return rc;
}


LIB_EXPORT rc_t CC KDirectoryVOpenSraArchiveReadUnbounded ( struct KDirectory const *self,
    struct KDirectory const **sra_dir, int chroot, const char *path, va_list args )
{
    /* respond properly to va_list */
    char full [ 4096 ];
    int len = vsnprintf ( full, sizeof full, path, args );
    if ( len < 0 )
        return RC ( rcFS, rcDirectory, rcOpening, rcPath, rcExcessive );

    /* putting off parameter validation into this call */
    return KDirectoryOpenArcDirRead (self, sra_dir, false, full, tocKFile,
                                     KArcParseSRAUnbounded, NULL, NULL);
}


LIB_EXPORT rc_t CC KDirectoryOpenSraArchiveReadUnbounded ( struct KDirectory const *self,
    struct KDirectory const **sra_dir, int chroot, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDirectoryVOpenSraArchiveReadUnbounded (self, sra_dir, chroot, path, args);
    va_end ( args );

    return rc;
}


LIB_EXPORT rc_t CC KFileIsSRA (const char * b, size_t z)
{
    const KSraHeader * h = (const KSraHeader *)b;

    if (SraHeaderValidate (h, NULL, NULL, z) == 0)
        return 0;
    
    return RC (rcFS, rcFile, rcIdentifying, rcBuffer, rcWrongType);
}

/* end of file */
