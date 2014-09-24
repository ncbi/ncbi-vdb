/*=======================================================================================
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

struct KMD5File;
#define KFILE_IMPL struct KMD5File

#include <kfs/extern.h>
#include <kfs/md5.h>
#include <kfs/impl.h>
#include <klib/checksum.h>
#include <klib/container.h>
#include <klib/text.h>
#include <klib/debug.h>
#include <klib/log.h>
#include <klib/rc.h>
#include <os-native.h>
#include <sysalloc.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <endian.h>
#include <byteswap.h>

#define MD5_DBG_REF(msg) DBGMSG(DBG_LEGREF,DBG_FLAG(DBG_LEGREF_MD5), msg)


/*--------------------------------------------------------------------------
 * KMD5SumEntry
 *  describes an entry into an MD5 checksum file
 */
typedef struct KMD5SumEntry KMD5SumEntry;
struct KMD5SumEntry
{
    SLNode n;
    uint8_t digest [ 16 ];
    bool bin;
    char path [ 1 ];
};

static
void CC KMD5SumEntryWhack ( SLNode *n, void *ignore )
{
    free ( n );
}

typedef struct KMD5SumEntryFlushData KMD5SumEntryFlushData;
struct KMD5SumEntryFlushData
{
    uint64_t pos;
    KFile *f;
    rc_t rc;
};

static
bool CC KMD5SumEntryFlush ( SLNode *n, void *data )
{
    KMD5SumEntryFlushData *pb = data;
    const KMD5SumEntry *self = ( const KMD5SumEntry* ) n;
    int i, len;
    size_t num_writ, total;
    char line [ 4096 + 40 ];

    for ( total = 0, i = 0; i < 16; ++ i )
    {
        len = snprintf ( & line [ total ], sizeof line - total,
            "%02x", self -> digest [ i ] );
        assert ( len == 2 );
        total += len;
    }

    len = snprintf ( & line [ total ], sizeof line - total,
        " %c%s\n", self -> bin ? '*' : ' ', self -> path );

    if ( len < 0 )
    {
        pb -> rc = RC ( rcFS, rcFile, rcFormatting, rcString, rcExcessive );
        return true;
    }

    total += len;
    if ( total >= sizeof line )
    {
        pb -> rc = RC ( rcFS, rcFile, rcFormatting, rcString, rcExcessive );
        return true;
    }

    pb -> rc = KFileWrite ( pb -> f, pb -> pos, line, total, & num_writ );
    if ( pb -> rc != 0 )
        return true;

    pb -> pos += num_writ;

    if ( num_writ == total )
        return false;

    pb -> rc = RC ( rcFS, rcFile, rcFormatting, rcTransfer, rcIncomplete );
    return true;
}

static
rc_t KMD5SumEntryMake ( KMD5SumEntry **ep,
    const char *path, const uint8_t digest [ 16 ], bool bin )
{
    KMD5SumEntry *entry = malloc ( sizeof * entry + strlen ( path ) );
    if ( entry == NULL )
        return RC ( rcFS, rcFile, rcUpdating, rcMemory, rcExhausted );

    memcpy ( entry -> digest, digest, sizeof entry -> digest );
    entry -> bin = bin;
    strcpy ( entry -> path, path );

    * ep = entry;
    return 0;
}


/*--------------------------------------------------------------------------
 * KMD5SumFmt
 *  a formatter for reading or writing an md5sum-style checksum file
 */
struct KMD5SumFmt
{
    SLList entries;
    KFile *f;
    atomic32_t refcount;
    uint32_t count;
    bool read_only;
    bool dirty;
};

/* Flush
 */
LIB_EXPORT rc_t CC KMD5SumFmtFlush ( KMD5SumFmt *self )
{
    if ( self != NULL && self -> dirty )
    {
        KMD5SumEntryFlushData pb;
        pb . pos = 0;
        pb . f = self -> f;
        pb . rc = 0;

        SLListDoUntil ( & self -> entries, KMD5SumEntryFlush, & pb );

        if ( pb . rc != 0 )
            return pb . rc;

        self -> dirty = false;
    }
    return 0;
}


/* Whack
 */
static
rc_t KMD5SumFmtWhack ( KMD5SumFmt *self )
{
    rc_t rc;

    atomic32_set ( & self -> refcount, 1 );

    rc = KMD5SumFmtFlush ( self );
    if ( rc == 0 )
        rc = KFileRelease ( self -> f );
    if ( rc != 0 )
        return rc;

    SLListWhack ( & self -> entries, KMD5SumEntryWhack, NULL );
    free ( self );
    return 0;
}


/* AddRef
 * Release
 */
LIB_EXPORT rc_t CC KMD5SumFmtAddRef ( const KMD5SumFmt *cself )
{
    if ( cself != NULL )
    {
        MD5_DBG_REF (("%s self %p prior count %u\n", __func__, cself, atomic32_read ( & cself -> refcount ) ));
        atomic32_inc ( & ( ( KMD5SumFmt* ) cself ) -> refcount );
    }
    else
    {
        MD5_DBG_REF(("%s self %p\n", __func__, cself));
    }
    return 0;
}

LIB_EXPORT rc_t CC KMD5SumFmtRelease ( const KMD5SumFmt *cself )
{
    KMD5SumFmt *self = ( KMD5SumFmt* ) cself;
    if ( cself != NULL )
    {
        MD5_DBG_REF (("%s self %p prior count %u\n", __func__, cself, atomic32_read ( & cself -> refcount ) ));
        if ( atomic32_dec_and_test ( & self -> refcount ) )
            return KMD5SumFmtWhack ( self );
    }
    else
    {
        MD5_DBG_REF(("%s self %p\n", __func__, cself));
    }
    return 0;
}


/* Extract
 *  extract a single line
 */
static
int hex_to_int ( char hex )
{
    int i = hex - '0';
    if ( hex > '9' )
    {
        if ( hex < 'a' )
            i = hex - 'A' + 10;
        else
            i = hex - 'a' + 10;

        if ( i > 15 )
            return -1;
    }
    return i;
}

static
rc_t KMD5SumFmtExtract ( KMD5SumFmt *self, const char *line, size_t size )
{
    rc_t rc;
    KMD5SumEntry *entry;

    /* check for a DOS condition not caught by parse */
    if ( line [ size - 1 ] == '\r' )
    {
        /* drop CR from line */
        size -= 1;
    }

    /* create entry */
    entry = malloc ( sizeof * entry + size - ( 32 /* cs */ + 2 /* sp + mode */ ) );
    if ( entry == NULL )
        rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
    else
    {
        int i;

        /* parse checksum */
        for ( rc = 0, i = 0; i < 16; ++ i )
        {
            int l, u = hex_to_int ( line [ i + i + 0 ] );
            l = hex_to_int ( line [ i + i + 1 ] );
            if ( u < 0 || l < 0 )
            {
                rc = RC ( rcFS, rcFile, rcConverting, rcFormat, rcInvalid );
                break;
            }

            entry -> digest [ i ] = ( uint8_t ) ( ( u << 4 ) | l );
        }

        /* parse mode */
        if ( rc == 0 )
        {
            if ( line [ 32 ] != ' ' )
                rc = RC ( rcFS, rcFile, rcConverting, rcFormat, rcInvalid );
            else if ( line [ 33 ] == ' ' )
                entry -> bin = false;
            else if ( line [ 33 ] == '*' )
                entry -> bin = true;
            else
                rc = RC ( rcFS, rcFile, rcConverting, rcFormat, rcInvalid );
        }

        /* the rest is path */
        if ( rc == 0 )
        {
            string_copy ( entry -> path, size, & line [ 32 + 2 ], size - ( 32 + 2 ) );
            SLListPushTail ( & self -> entries, & entry -> n );
            ++ self -> count;
            return 0;
        }

        free ( entry );
    }

    return rc;
}


/* Parse
 *  parse *.md5 file
 */
static
rc_t KMD5SumFmtParse ( KMD5SumFmt *self, const char *start, const char *stop )
{
    rc_t rc;
    const char *end;

    for ( rc = 0; start < stop; start = end + 1 )
    {
        end = string_chr ( start, stop - start, '\n' );
        if ( end == NULL )
            end = stop;

        /* allow '#' comments and blank lines */
        if ( ( end - start ) == 0 )
            continue;
        if ( start [ 0 ] == '#' )
            continue;

        /* line must have minimum length */
        if ( ( end - start ) < ( 32 /* cs */ + 2 /* sp + mode */ + 1 /* path */ ) )
        {
            /* check for empty line ending in CR-LF */
            if ( end [ -1 ] == '\r' && ( end - start ) == 1 )
                continue;

            /* other stuff is garbage */
            rc = RC ( rcFS, rcFile, rcParsing, rcData, rcInsufficient );
            break;
        }

        /* extract entry */
        rc = KMD5SumFmtExtract ( self, start, end - start );
        if ( rc != 0 )
            break;
    }

    return rc;
}


/* Populate
 *  reads all lines of input from *.md5 file
 */
static
rc_t KMD5SumFmtPopulate ( KMD5SumFmt *self, const KFile *in )
{
    uint64_t eof;
    rc_t rc = KFileSize ( in, & eof );
    if ( rc == 0 )
    {
        char *buff = malloc ( ( size_t ) eof );
        if ( buff == NULL )
            rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
        else
        {
            size_t num_read, total;
            for ( total = 0; total < ( size_t ) eof; total += num_read )
            {
                rc = KFileRead ( in, total,
                    & buff [ total ], ( size_t ) eof - total, & num_read );
                if ( rc != 0 )
                    break;
                if ( num_read == 0 )
                {
                    rc = RC ( rcFS, rcFile, rcReading, rcTransfer, rcIncomplete );
                    break;
                }
            }

            if ( rc == 0 )
                rc = KMD5SumFmtParse ( self, buff, buff + total );

            free ( buff );
        }
    }

    return rc;
}


/* Make
 *  creates a formatted accessor to textual checksum file
 *  takes over ownership of "in"/"out" file reference
 */
LIB_EXPORT rc_t CC KMD5SumFmtMakeRead ( const KMD5SumFmt **fp, const KFile *in )
{
    rc_t rc;
    if ( fp == NULL )
        rc = RC ( rcFS, rcFile, rcConstructing, rcParam, rcNull );
    else
    {
        if ( in == NULL )
            rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcNull );
        else if ( ! in -> read_enabled )
            rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcNoPerm );
        else
        {
            KMD5SumFmt *f = malloc ( sizeof * f );
            if ( f == NULL )
                rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
            else
            {
                SLListInit ( & f -> entries );
                atomic32_set ( & f -> refcount, 1 );
                f -> f = NULL;
                f -> count = 0;
                f -> read_only = true;
                f -> dirty = false;

                rc = KMD5SumFmtPopulate ( f, in );
                if ( rc == 0 )
                {
                    f -> f = ( KFile* ) in;
                    * fp = f;
                    return 0;
                }
                
                KMD5SumFmtWhack ( f );
            }
        }

        * fp = NULL;
    }

    return rc;
}

/* Make
 *  creates a formatted accessor to textual checksum file
 *  takes over ownership of "in"/"out" file reference
 */
LIB_EXPORT rc_t CC KMD5SumFmtMakeUpdate ( KMD5SumFmt **fp, KFile *out )
{
    rc_t rc;
    if ( fp == NULL )
        rc = RC ( rcFS, rcFile, rcConstructing, rcParam, rcNull );
    else
    {
        if ( out == NULL )
            rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcNull );
        else if ( ! out -> write_enabled )
        {
            if ( out -> read_enabled )
                rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcReadonly );
            else
                rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcNoPerm );
        }
        else
        {
            KMD5SumFmt *f = malloc ( sizeof * f );
            if ( f == NULL )
                rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
            else
            {
                SLListInit ( & f -> entries );
                atomic32_set ( & f -> refcount, 1 );
                f -> f = NULL;
                f -> count = 0;
                f -> read_only = false;
                f -> dirty = false;

                rc = 0;
                if ( out -> read_enabled )
                    rc = KMD5SumFmtPopulate ( f, out );
                if ( rc == 0 )
                {
                    f -> f = out;
                    * fp = f;
                    return 0;
                }
                
                KMD5SumFmtWhack ( f );
            }
        }

        * fp = NULL;
    }

    return rc;
}


/* Count
 *  return the number of checksums in file
 */
LIB_EXPORT rc_t CC KMD5SumFmtCount ( const KMD5SumFmt *self, uint32_t *count )
{
    rc_t rc;
    if ( count == NULL )
        rc = RC ( rcFS, rcFile, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcFS, rcFile, rcAccessing, rcSelf, rcNull );
        else
        {
            * count = self -> count;
            return 0;
        }

        * count = 0;
    }
    return rc;
}


/* Get
 *  retrieve an indexed checksum line
 *
 *  "idx" [ IN ] - zero-based index of checksum to access
 *
 *  "path" [ OUT ] and "size" [ IN ] - return buffer for file path
 *
 *  "digest" [ OUT ] - return parameter for the MD5 checksum
 *
 *  "bin" [ OUT, NULL OKAY ] - true if fopen mode used to
 *  calculate "digest" was binary, false if text. pertinent if
 *  digest was calculated using DOS text mode.
 */
LIB_EXPORT rc_t CC KMD5SumFmtGet ( const KMD5SumFmt *self, uint32_t idx,
    char *path, size_t size, uint8_t digest [ 16 ], bool *bin )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcFS, rcFile, rcAccessing, rcSelf, rcNull );
    else if ( idx >= self -> count )
        rc = RC ( rcFS, rcFile, rcAccessing, rcId, rcNotFound );
    else
    {
        uint32_t i;
        const KMD5SumEntry *entry = ( const KMD5SumEntry* )
            SLListHead ( & self -> entries );
        for ( i = 0; entry != NULL && i < idx; ++ i )
        {
            entry = ( const KMD5SumEntry* )
                SLNodeNext ( & entry -> n );
        }

        if ( entry == NULL )
            rc = RC ( rcFS, rcFile, rcAccessing, rcId, rcNotFound );
        else if ( path == NULL )
            rc = RC ( rcFS, rcFile, rcAccessing, rcBuffer, rcNull );
        else if ( digest == NULL )
            rc = RC ( rcFS, rcFile, rcAccessing, rcParam, rcNull );
        else
        {
            /* copy everything out */
            memcpy ( digest, entry -> digest, sizeof entry -> digest );
            if ( bin != NULL )
                * bin = entry -> bin;

            /* return success if buffer large enough */
            if ( string_copy_measure ( path, size, entry -> path ) < size )
                return 0;

            rc = RC ( rcFS, rcFile, rcAccessing, rcBuffer, rcInsufficient );
        }
    }

    if ( path != NULL && size != 0 )
        path [ 0 ] = 0;

    if ( bin != NULL )
        * bin = false;

    return rc;
}


/* Find
 *  retrieve checksum line by path
 *
 *  "path" [ IN ] - NUL terminated string in OS-native char set
 *  denoting path to file
 *
 *  "digest" [ OUT ] - return parameter for the MD5 checksum
 *
 *  "bin" [ OUT, NULL OKAY ] - true if fopen mode used to
 *  calculate "digest" was binary, false if text. pertinent if
 *  digest was calculated using DOS text mode.
 */
LIB_EXPORT rc_t CC KMD5SumFmtFind ( const KMD5SumFmt *self,
    const char *path, uint8_t digest [ 16 ], bool *bin )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcFS, rcFile, rcAccessing, rcSelf, rcNull );
    else if ( path == NULL )
        rc = RC ( rcFS, rcFile, rcAccessing, rcPath, rcNull );
    else if ( path [ 0 ] == 0 )
        rc = RC ( rcFS, rcFile, rcAccessing, rcPath, rcEmpty );
    else
    {
        const KMD5SumEntry *entry = ( const KMD5SumEntry* )
            SLListHead ( & self -> entries );
        while ( entry != NULL )
        {
            if ( strcmp ( path, entry -> path ) == 0 )
                break;

            entry = ( const KMD5SumEntry* )
                SLNodeNext ( & entry -> n );
        }

        if ( entry == NULL )
            rc = RC ( rcFS, rcFile, rcAccessing, rcPath, rcNotFound );
        else if ( digest == NULL )
            rc = RC ( rcFS, rcFile, rcAccessing, rcParam, rcNull );
        else
        {
            /* copy everything out */
            memcpy ( digest, entry -> digest, sizeof entry -> digest );
            if ( bin != NULL )
                * bin = entry -> bin;

            return 0;
        }
    }

    if ( bin != NULL )
        * bin = false;

    return rc;
}


/* Delete
 *  remove an existing line
 *
 *  "path" [ IN ] - NUL terminated string in OS-native char set
 *  denoting path to file
 *
 *  returns rcNotFound if not present
 */
LIB_EXPORT rc_t CC KMD5SumFmtDelete ( KMD5SumFmt *self, const char *path )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcFS, rcFile, rcRemoving, rcSelf, rcNull );
    else if ( path == NULL )
        rc = RC ( rcFS, rcFile, rcRemoving, rcPath, rcNull );
    else if ( path [ 0 ] == 0 )
        rc = RC ( rcFS, rcFile, rcRemoving, rcPath, rcEmpty );
    else
    {
        KMD5SumEntry *entry = ( KMD5SumEntry* )
            SLListHead ( & self -> entries );
        while ( entry != NULL )
        {
            if ( strcmp ( path, entry -> path ) == 0 )
                break;

            entry = ( KMD5SumEntry* )
                SLNodeNext ( & entry -> n );
        }

        if ( entry == NULL )
            rc = RC ( rcFS, rcFile, rcRemoving, rcPath, rcNotFound );
        else
        {
            /* pop entry from list */
            SLListUnlink ( & self -> entries, & entry -> n );

            /* delete entry */
            KMD5SumEntryWhack ( & entry -> n, NULL );

            /* mark the modification */
            self -> dirty = true;
            assert ( self -> count > 0 );
            -- self -> count;

            return 0;
        }
    }

    return rc;
}


/* Update
 *  add a checksum line to file or update an existing line
 *
 *  "path" [ IN ] - NUL terminated string in OS-native char set
 *  denoting path to file
 *
 *  "digest" [ IN ] - calculated MD5 checksum
 *
 *  "bin" [ IN ] - true if checksum was calculated on all bytes
 *  in file, false if fopen mode used to read file was text.
 *
 * NB - the md5sum tool operates with the assumption that files
 *  are opened using "fopen". it records the mode used by prepending
 *  '*' to indicate binary and ' ' to indicate text mode within the
 *  checksum line.
 *
 *  unfortunately, this creates a situation where text-mode files
 *  checksummed on DOS cannot be properly checked on any other
 *  platform without the aid of some filter to drop out '\r' from
 *  a '\r\n' sequence.
 *
 *  fortunately, most users are unaware of the binary/text mode
 *  switches available with md5sum and use the default mode, which
 *  is binary on DOS and text on Unix, which happens to be equivalent
 *  to binary ( making one wonder why binary is not instead the
 *  universal default ).
 *
 *  the end result is that we need to indicate whether CR stripping
 *  was performed while at the same time behaving like md5sum.
 */
LIB_EXPORT rc_t CC KMD5SumFmtUpdate ( KMD5SumFmt *self, const char *path,
    const uint8_t digest [ 16 ], bool bin )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcFS, rcFile, rcUpdating, rcSelf, rcNull );
    else if ( path == NULL )
        rc = RC ( rcFS, rcFile, rcUpdating, rcString, rcNull );
    else if ( path [ 0 ] == 0 )
        rc = RC ( rcFS, rcFile, rcUpdating, rcString, rcEmpty );
    else if ( digest == NULL )
        rc = RC ( rcFS, rcFile, rcUpdating, rcParam, rcNull );
    else
    {
        /* create an entry */
        KMD5SumEntry *entry;
        rc = KMD5SumEntryMake ( & entry, path, digest, bin );
        if ( rc == 0 )
        {
            /* scan for existing entry */
            KMD5SumEntry *exist = ( KMD5SumEntry* )
                SLListHead ( & self -> entries );
            while ( exist != NULL )
            {
                if ( strcmp ( exist -> path, entry -> path ) == 0 )
                    break;
                exist = ( KMD5SumEntry* )
                    SLNodeNext ( & exist -> n );
            }

            /* if new entry, insert */
            if ( exist == NULL )
            {
                SLListPushTail ( & self -> entries, & entry -> n );
                self -> dirty = true;
                ++ self -> count;
            }
            else
            {
                /* if existing, perform update */
                if ( memcmp ( exist -> digest, entry -> digest, sizeof exist -> digest ) != 0 ||
                     exist -> bin != entry -> bin )
                {
                    memcpy ( exist -> digest, entry -> digest, sizeof exist -> digest );
                    exist -> bin = entry -> bin;
                    self -> dirty = true;
                }
                KMD5SumEntryWhack ( & entry -> n, NULL );
            }
        }
    }

    return rc;
}

/* Rename
 * Rename the checksum line to reflect a file name change but no content change.
 *
 *  "newpath" [ IN ] - NUL terminated string in OS-native char set
 *  denoting path to file's new name
 *
 *  "oldpath" [ IN ] - NUL terminated string in OS-native char set
 *  denoting path to file's old name
 *
 */
LIB_EXPORT rc_t CC KMD5SumFmtRename ( KMD5SumFmt *self, const char *oldpath, const char *newpath )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcFS, rcFile, rcUpdating, rcSelf, rcNull );
    else if ( newpath == NULL )
        rc = RC ( rcFS, rcFile, rcUpdating, rcString, rcNull );
    else if ( newpath [ 0 ] == 0 )
        rc = RC ( rcFS, rcFile, rcUpdating, rcString, rcEmpty );
    else if ( oldpath == NULL )
        rc = RC ( rcFS, rcFile, rcUpdating, rcString, rcNull );
    else if ( oldpath [ 0 ] == 0 )
        rc = RC ( rcFS, rcFile, rcUpdating, rcString, rcEmpty );
    else
    {
	uint8_t	digest [16];
	bool    bin;

	rc = KMD5SumFmtFind (self, oldpath, digest, &bin);

	if (rc == 0)
	    rc = KMD5SumFmtUpdate (self, newpath, digest, bin);

	if (rc == 0)
	    rc = KMD5SumFmtDelete (self, oldpath);
;
    }
    return rc;
}


/*--------------------------------------------------------------------------
 * MD5 KFile extensions
 */
typedef enum KMD5FileUnionType
{
    KMD5FileTypeRead,
    KMD5FileTypeWrite
} KMD5FileUnionType;
struct KMD5File
{
    KFile dad;

    /* this is position of next byte to read or write
       relative to the MD5 not actual read/write */
    uint64_t position;
    MD5State md5;

    KFile *file;
    KMD5SumFmt *fmt;
    KMD5FileUnionType type;
    union
    {
        struct
        {
            rc_t rc;
            uint8_t digest [ 16 ];
            bool eof;
        } rd;

        struct
        {
            /* maximum position written to "file" */
            uint64_t max_position;

            /* saved information for a transaction */
            uint64_t tposition;
            MD5State tmd5;

            /* is transaction open */
            bool topen;

            /* path to file for md5 fmt */
            char path [ 1 ];
        } wr;

    } u;
};

/* Whack
 */
static
rc_t CC KMD5FileWhackRead ( KMD5File *self )
{
    rc_t rc;

    atomic32_set ( & self -> dad . refcount, 1 );

    rc = KFileRelease ( self -> file );
    if ( rc == 0 )
        free ( self );

    return rc;
}
static
rc_t CC KMD5FileWhackCreate ( KMD5File *self )
{
    uint64_t	position;
    size_t	num_read;
    rc_t	rc;
    uint8_t	ignored [64 * 1024];

    atomic32_set ( & self -> dad . refcount, 1 );

    position = self->position;
    for (;;)
    {
	rc = KFileRead (&self->dad, position, ignored, sizeof ignored, &num_read);
	if (rc != 0)
	    break;

	if (num_read == 0)
	    break;

	position += num_read;
    }
    if (rc == 0)
    {
	rc = KFileRelease ( self -> file );
	if ( rc == 0 )
	    free ( self );
    }
    return rc;
}

static
rc_t CC KMD5FileWhackWrite ( KMD5File *self )
{
    rc_t rc;
    uint8_t digest [ 16 ];

    atomic32_set ( & self -> dad . refcount, 1 );

    /* if destination file has been written farther
       than our concept of eof, truncate */
    if ( self -> u . wr . max_position > self -> position )
    {
        rc = KFileSetSize ( self -> file, self -> position );
        if ( rc != 0 )
            return rc;
        self -> u . wr . max_position = self -> position;
    }

    rc = KFileRelease ( self -> file );
    if ( rc == 0 )
    {
        MD5StateFinish ( & self -> md5, digest );
        KMD5SumFmtUpdate ( self -> fmt, self -> u . wr . path, digest, true );
        KMD5SumFmtRelease ( self -> fmt );
        free ( self );
    }

    return rc;
}

static const char MD5TAG [8] = "MD5CNTXT";
static const char ENDIANTAG [4] =
{ 
    '0' + (char)((__BYTE_ORDER/1000)%10),
    '0' + (char)((__BYTE_ORDER/100)%10),
    '0' + (char)((__BYTE_ORDER/10)%10),
    '0' + (char)((__BYTE_ORDER/1)%10)
};
static const char ENDIANSWAPTAG [4] =
{ 
    '0' + (char)((__BYTE_ORDER/1)%10),
    '0' + (char)((__BYTE_ORDER/10)%10),
    '0' + (char)((__BYTE_ORDER/100)%10),
    '0' + (char)((__BYTE_ORDER/1000)%10)
};

static
rc_t CC KMD5FileWrite ( KMD5File *self, uint64_t pos,
    const void *buffer, size_t size, size_t *num_writ );

static
rc_t CC KMD5FileWhackAppend ( KMD5File *self )
{
    rc_t rc;

    /* if the file is empty, just whack it */
    if ( self -> position == 0 )
        return KMD5FileWhackWrite ( self );

    /* begin transaction will save current state */
    rc = KMD5FileBeginTransaction ( self );
    if ( rc == 0 )
    {
        size_t num_writ;

        /* create a closing tag with current MD5State */
        uint8_t closer [ sizeof MD5TAG + sizeof ENDIANTAG + sizeof self -> md5 ];
        memcpy ( closer, MD5TAG, sizeof MD5TAG );
        memcpy ( & closer [ sizeof MD5TAG ], ENDIANTAG, sizeof ENDIANTAG );
        memcpy ( & closer [ sizeof MD5TAG + sizeof ENDIANTAG ], & self -> md5, sizeof self -> md5 );
            
        /* append this to the file */
        rc = KMD5FileWrite ( self, self -> position, closer, sizeof closer, & num_writ );
        if ( rc == 0 )
        {
            /* test transfer */
            if ( num_writ != sizeof closer )
                rc = RC ( rcFS, rcFile, rcDestroying, rcTransfer, rcIncomplete );
            else
            {
                /* complete operation */
                rc = KMD5FileWhackWrite ( self );
                if ( rc == 0 )
                    return rc;
            }
        }

        /* restore entry state */
        KMD5FileRevert ( self );
    }

    /* bail on errors */
    atomic32_set ( & self -> dad . refcount, 1 );
    return rc;
}


/* GetSysFile
 */
static
struct KSysFile *CC KMD5FileGetSysFile ( const KMD5File *self, uint64_t *offset )
{
    if ( offset != NULL )
        * offset = 0;
    return NULL;
}


/* RandomAccess
 */
static
rc_t CC KMD5FileRandomAccessRead ( const KMD5File *self )
{
    return KFileRandomAccess ( self -> file );
}

static
rc_t CC KMD5FileRandomAccessWrite ( const KMD5File *self )
{
    return RC ( rcFS, rcFile, rcAccessing, rcFunction, rcUnsupported );
}


/* Type
 */
static
uint32_t CC KMD5FileType ( const KMD5File *self )
{
    return KFileType ( self -> file );
}


/* Size
 */
static
rc_t CC KMD5FileSize ( const KMD5File *self, uint64_t *size )
{
    return KFileSize ( self -> file, size );
}

static
rc_t CC KMD5FileSizeAppend ( const KMD5File *self, uint64_t *size )
{
    assert (self);
    assert (size);
    *size = self->position;
    return 0;
}


/* SetSize
 *  the read-only version should never be called, because
 *  the message dispatch checks for write enable
 *
 *  the writable version disallows all but a total reset
 *  and setting size to current size
 */
static
rc_t CC KMD5FileSetSizeRead ( KMD5File *self, uint64_t size )
{
    return RC ( rcFS, rcFile, rcUpdating, rcFunction, rcUnsupported );
}

static
rc_t CC KMD5FileSetSizeWrite ( KMD5File *self, uint64_t size )
{
    rc_t rc;

    /* allow a noop */
    if ( size == self -> position )
        return 0;

    /* disallow if not a reset */
    if ( size != 0 )
        return RC ( rcFS, rcFile, rcUpdating, rcFunction, rcUnsupported );

    /* ensure we're not in a transaction */
    if ( self -> u . wr . topen )
        return RC ( rcFS, rcFile, rcUpdating, rcFile, rcBusy );

    /* go ahead and try to truncate destination */
    rc = KFileSetSize ( self -> file, 0 );
    if ( rc == 0 )
    {
        /* reset our internal state */
        self -> position = self -> u . wr . max_position = 0;
        MD5StateInit ( & self -> md5 );
    }

    return rc;
}


/* Finish
 *  finishes an MD5State if necessary
 *  returns the results of digest comparison
 */
static
rc_t KMD5FileFinish ( KMD5File *self )
{
    /* finish MD5State on first time */
    if ( ! self -> u . rd . eof )
    {
        uint8_t digest [ 16 ];
        MD5StateFinish ( & self -> md5, digest );

        if ( memcmp ( digest, self -> u . rd . digest, sizeof digest ) != 0 )
            self -> u . rd . rc = RC ( rcFS, rcFile, rcReading, rcFile, rcCorrupt );

        self -> u . rd . eof = true;
    }

    return self -> u . rd . rc;
}


/* Seek
 *  used to scan ahead in a read-only file
 *  "self" is modifiable, but file is read-only
 */
static
rc_t KMD5FileSeek ( KMD5File *self, uint64_t pos )
{
    rc_t rc;
    size_t num_read;
    uint8_t buff [ 32 * 1024 ];

    /* should never be called by writable file */
    assert ( ! self -> dad . write_enabled );

    /* seek to "pos" */
    for ( rc = 0; self -> position < pos; self -> position += num_read )
    {
        /* maximum to read in this buffer */
        size_t to_read = sizeof buff;
        if ( self -> position + sizeof buff > pos )
            to_read = ( size_t ) ( pos - self -> position );

        /* read bytes */
        rc = KFileRead ( self -> file, self -> position, buff, to_read, & num_read );
        if ( rc != 0 )
            break;

        /* detect EOF */
        if ( num_read == 0 )
        {
	    switch (self->type)
	    {
	    case KMD5FileTypeRead:
		rc = KMD5FileFinish ( self );
		break;
	    case KMD5FileTypeWrite:
		if ( self -> fmt != NULL)
		{
		    uint8_t digest [ 16 ];

		    MD5StateFinish ( & self -> md5, digest );
		    KMD5SumFmtUpdate ( self -> fmt, self -> u . wr . path, digest, true );
		    KMD5SumFmtRelease ( self -> fmt );
		    self -> fmt = NULL;
		}
		break;
	    }
	    break;
        }

        /* append bytes */
        MD5StateAppend ( & self -> md5, buff, num_read );
    }

    return rc;
}

/* Read
 *  read-only version allows random access reads
 */
static
rc_t CC KMD5FileRead ( const KMD5File *cself,
    uint64_t pos, void *buffer, size_t bsize, size_t *num_read )
{
    rc_t rc;
    size_t total = 0;
    uint64_t end = pos + bsize;
    KMD5File *self = ( KMD5File* ) cself;

    * num_read = 0;

    /* no buffer means no bytes */
    if ( bsize == 0 )
        return 0;

    /* test for starting a read before the current marker
       meaning that it does not figure in to the md5sum */
    if ( self -> position  > pos )
    {
        /* test for entire buffer being before marker */
        if ( end <= self -> position )
            return KFileRead ( self -> file, pos, buffer, bsize, num_read );

        /* perform a partial read without affecting md5 */
        rc = KFileRead ( self -> file, pos, buffer, ( size_t ) ( self -> position - pos ), num_read );
        if ( rc != 0 )
            return rc;

        /* adjust as if a new read request, remembering number read so far */
        pos += * num_read;
        total = * num_read;
        buffer = & ( ( uint8_t* ) buffer ) [ * num_read ];

        /* in case num_read != all bytes up to marker */
        if ( pos != self -> position )
            return 0;
    }

    /* test for hole between current marker and desired start */
    else if ( self -> position < pos )
    {
        /* scan ahead, discarding */
        rc = KMD5FileSeek ( self, pos );
        if ( rc != 0 || self -> position < pos )
            return rc;
    }

    /* no hole - and still some space in buffer */
    assert ( self -> position == pos );
    assert ( pos < end );

    /* read bytes */
    rc = KFileRead ( self -> file, pos, buffer, ( size_t ) ( end - pos ), num_read );
    if ( rc == 0 )
    {
        /* detect bytes read */
        if ( * num_read != 0 )
        {
            /* accumulate into MD5 */
            self -> position += * num_read;
            MD5StateAppend ( & self -> md5, buffer, * num_read );

            /* add in any previously read bytes and return success */
            * num_read += total;
            return 0;
        }
	switch (self->type)
	{
	case KMD5FileTypeRead:
	    /* EOF - get MD5 digest results */
	    rc = KMD5FileFinish ( self );
	    break;
	case KMD5FileTypeWrite:
	    if ( self -> fmt != NULL)
	    {
		uint8_t digest [ 16 ];

		MD5StateFinish ( & self -> md5, digest );
		KMD5SumFmtUpdate ( self -> fmt, self -> u . wr . path, digest, true );
		KMD5SumFmtRelease ( self -> fmt );
		self -> fmt = NULL;
	    }
	    break;
	}

    }

    /* always return 0 if some bytes were read */
    if ( total != 0 )
    {
        * num_read = total;
        return 0;
    }

    return rc;
}

/* Read
 *  writeable version isn't keeping track of MD5 on read,
 *  so just forward to destination file
 */
static
rc_t CC KMD5FileFwdRead ( const KMD5File *self,
    uint64_t pos, void *buffer, size_t bsize, size_t *num_read )
{
    return KFileRead ( self -> file, pos, buffer, bsize, num_read );
}


/* Write
 */
static
rc_t CC KMD5FileBadWrite ( KMD5File *self, uint64_t pos,
    const void *buffer, size_t size, size_t *num_writ )
{
    * num_writ = 0;
    if ( size == 0 )
        return 0;
    if ( buffer == NULL )
        return RC ( rcFS, rcFile, rcWriting, rcBuffer, rcNull );

    return RC ( rcFS, rcFile, rcReading, rcFile, rcReadonly );
}

static
rc_t CC KMD5FileWrite ( KMD5File *self, uint64_t pos,
    const void *buffer, size_t size, size_t *num_writ )
{
    rc_t rc;

    if ( pos != self -> position )
        return RC ( rcFS, rcFile, rcWriting, rcParam, rcInvalid );

    rc = KFileWrite ( self -> file, pos, buffer, size, num_writ );
    if ( rc == 0 && * num_writ != 0 )
    {
        self -> position += * num_writ;
        if ( self -> position > self -> u . wr . max_position )
            self -> u . wr . max_position = self -> position;
        MD5StateAppend ( & self -> md5, buffer, * num_writ );
    }

    return rc;
}


/* ToKFile
 *  an MD5 file is a subclass of KFile
 *  this is an explicit cast operator
 */
LIB_EXPORT KFile* CC KMD5FileToKFile ( KMD5File *self )
{
    if ( self != NULL )
        return & self -> dad;
    return NULL;
}

LIB_EXPORT const KFile* CC KMD5FileToKFileConst ( const KMD5File *self )
{
    if ( self != NULL )
        return & self -> dad;
    return NULL;
}


/* BeginTransaction
 *  preserve current MD5 state and file marker
 *  in preparation for rollback
 */
LIB_EXPORT rc_t CC KMD5FileBeginTransaction ( KMD5File *self )
{
    if ( self == NULL )
        return RC ( rcFS, rcFile, rcFreezing, rcSelf, rcNull );
    if ( ! self -> dad . write_enabled )
        return RC ( rcFS, rcFile, rcFreezing, rcFile, rcReadonly );
    if ( self -> u . wr . topen )
        return RC ( rcFS, rcFile, rcFreezing, rcFile, rcBusy );

    self -> u . wr . topen = true;
    self -> u . wr . tposition = self -> position;
    self -> u . wr . tmd5 = self -> md5;

    return 0;
}


/* Commit
 *  accept current MD5 state as correct
 */
LIB_EXPORT rc_t CC KMD5FileCommit ( KMD5File *self )
{
    if ( self == NULL )
        return RC ( rcFS, rcFile, rcCommitting, rcSelf, rcNull );
    if ( ! self -> dad . write_enabled )
        return RC ( rcFS, rcFile, rcCommitting, rcFile, rcReadonly );

    self -> u . wr . topen = false;

    return 0;
}


/* Revert
 *  revert state to previous transaction boundary conditions
 */
LIB_EXPORT rc_t CC KMD5FileRevert ( KMD5File *self )
{
    if ( self == NULL )
        return RC ( rcFS, rcFile, rcReverting, rcSelf, rcNull );
    if ( ! self -> dad . write_enabled )
        return RC ( rcFS, rcFile, rcReverting, rcFile, rcReadonly );
    if ( ! self -> u . wr . topen )
        return RC ( rcFS, rcFile, rcReverting, rcFile, rcInconsistent );

    self -> position = self -> u . wr . tposition;
    self -> md5 = self -> u . wr . tmd5;
    self -> u . wr . topen = false;

    return 0;
}

/* Reset
 *  reset state to that of an empty file
 *  aborts any open transaction
 */
LIB_EXPORT rc_t CC KMD5FileReset ( KMD5File *self )
{
    if ( self == NULL )
        return RC ( rcFS, rcFile, rcResetting, rcSelf, rcNull );
    if ( ! self -> dad . write_enabled )
        return RC ( rcFS, rcFile, rcResetting, rcFile, rcReadonly );
#if 1
    self -> u . wr . topen = false;
#else
    if ( self -> u . wr . topen )
        return RC ( rcFS, rcFile, rcResetting, rcFile, rcBusy );
#endif

    self -> position = 0;
    MD5StateInit ( & self -> md5 );

    return 0;
}

/*--------------------------------------------------------------------------
 * MD5File factories
 */


/* MakeMD5Read
 *  creates an adapter to check MD5 checksum for a source file
 *
 *  "f" [ OUT ] - return parameter for file with md5 check
 *
 *  "src" [ IN ] - source file with read permission
 *
 *  "digest" [ IN ] - MD5 checksum to match
 *
 * NB - creates a read-only file that does NOT support random access
 *  i.e. must be consumed serially starting from offset 0
 */
static KFile_vt_v1 KMD5FileRead_vt =
{
    1, 1,

    /* 1.0 */
    KMD5FileWhackRead,
    KMD5FileGetSysFile,
    KMD5FileRandomAccessRead,
    KMD5FileSize,
    KMD5FileSetSizeRead,
    KMD5FileRead,
    KMD5FileBadWrite,

    /* 1.1 */
    KMD5FileType
};

LIB_EXPORT rc_t CC KFileMakeMD5Read ( const KFile **fp,
    const KFile *src, const uint8_t digest [ 16 ] )
{
    rc_t rc;

    if ( fp == NULL )
        rc = RC ( rcFS, rcFile, rcConstructing, rcParam, rcNull );
    else
    {
        if ( src == NULL )
            rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcNull );
        else if ( ! src -> read_enabled )
            rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcNoPerm );
        else if ( digest == NULL )
            rc = RC ( rcFS, rcFile, rcConstructing, rcParam, rcNull );
        else
        {
            KMD5File *f = malloc ( sizeof * f - sizeof f -> u + sizeof f -> u . rd );
            if ( f == NULL )
                rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
            else
            {
                rc = KFileInit ( & f -> dad,
				     ( const KFile_vt* ) & KMD5FileRead_vt, "KMD5File", "no-name", true, false );
                if ( rc == 0 )
                {
                    f -> position = 0;
                    MD5StateInit ( & f -> md5 );
                    f -> file = ( KFile* ) src;
                    f -> fmt = NULL;
                    f -> type = KMD5FileTypeRead;
                    f -> u . rd . rc = 0;
                    f -> u . rd . eof = 0;
                    memcpy ( f -> u . rd . digest, digest, sizeof f -> u . rd . digest );

                    * fp = & f -> dad;
                    return 0;
                }
            }

            free ( f );
        }

        * fp = NULL;
    }

    return rc;
}


/* MakeWrite
 *  creates an adapter to create MD5 checksum for a file
 *
 *  "f" [ OUT ] - return parameter for file with md5 check
 *
 *  "out" [ IN ] - output file with write permission
 *  since this file is being wrapped by "f", ownership of
 *  the provided reference is transferred to "f"
 *
 *  "md5" [ IN ] - md5 checksum output formatter.
 *  since this may be used for multiple checksums, "f" will
 *  attach its own reference to "md5".
 *
 *  "path" [ IN ] - md5 checksum output file path, used to
 *  create standard md5sum file entry.
 *
 * NB - creates a write-only file that does NOT suppport random access
 *  i.e. must be written serially from offset 0
 */
static KFile_vt_v1 sKMD5FileWrite_vt =
{
    1, 0,

    KMD5FileWhackWrite,
    KMD5FileGetSysFile,
    KMD5FileRandomAccessWrite,
    KMD5FileSize,
    KMD5FileSetSizeWrite,
    KMD5FileFwdRead,
    KMD5FileWrite
};

LIB_EXPORT rc_t CC KMD5FileMakeWrite ( KMD5File **fp,
    KFile *out, KMD5SumFmt *md5, const char *path )
{
    rc_t rc;

    if ( fp == NULL )
        rc = RC ( rcFS, rcFile, rcConstructing, rcParam, rcNull );
    else
    {
        if ( out == NULL )
            rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcNull );
        else if ( ! out -> write_enabled )
        {
            if ( out -> read_enabled )
                rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcReadonly );
            else
                rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcNoPerm );
        }
        else if ( md5 == NULL )
            rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcNull );
        else if ( md5 -> read_only )
            rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcReadonly );
        else if ( path == NULL )
            rc = RC ( rcFS, rcFile, rcConstructing, rcPath, rcNull );
        else if ( path [ 0 ] == 0 )
            rc = RC ( rcFS, rcFile, rcConstructing, rcPath, rcEmpty );
        else
        {
            KMD5File *f = malloc ( sizeof * f - sizeof f -> u +
                sizeof f -> u . wr + strlen ( path ) );
            if ( f == NULL )
                rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
            else
            {
                rc = KFileInit ( & f -> dad,
                    ( const KFile_vt* ) & sKMD5FileWrite_vt, "KMD5File", path, out -> read_enabled, true );
                if ( rc == 0 )
                {
                    f -> position = 0;
                    MD5StateInit ( & f -> md5 );
                    f -> file = out;
                    f -> fmt = md5;
		    f -> type = KMD5FileTypeWrite;

                    memset ( & f -> u . wr, 0, sizeof f -> u . wr );
                    strcpy ( f -> u . wr . path, path );

                    rc = KFileSize ( out, & f -> u . wr . max_position );
                    if (rc)
                    {
                        if (GetRCState (rc) == rcUnsupported)
                        {
                            f -> u . wr . max_position  = 0;
                            rc = 0;
                        }
                    }    
                    if ( rc == 0 )
                        rc = KMD5SumFmtAddRef ( md5 );
                    if ( rc == 0 )
                    {
                        * fp = f;
                        return 0;
                    }
                }

                free ( f );
            }
        }

        * fp = NULL;
    }

    return rc;
}

/* MakeAppend
 *  creates an adapter to create MD5 checksum for a file
 *
 *  "f" [ OUT ] - return parameter for file with md5 check
 *
 *  "out" [ IN ] - output file with write permission
 *  since this file is being wrapped by "f", ownership of
 *  the provided reference is transferred to "f"
 *
 *  "md5" [ IN ] - md5 checksum output formatter.
 *  since this may be used for multiple checksums, "f" will
 *  attach its own reference to "md5".
 *
 *  "path" [ IN ] - md5 checksum output file path, used to
 *  create standard md5sum file entry.
 *
 * NB - creates a write-only file that does NOT suppport random access
 *  i.e. must be written serially from current EOF (minus the stored MD5 context)
 */
static KFile_vt_v1 sKMD5FileAppend_vt =
{
    1, 0,

    KMD5FileWhackAppend,
    KMD5FileGetSysFile,
    KMD5FileRandomAccessWrite,
    KMD5FileSizeAppend,
    KMD5FileSetSizeWrite,
    KMD5FileFwdRead,
    KMD5FileWrite
};

LIB_EXPORT rc_t CC KMD5FileMakeAppend ( KMD5File **fp, KFile *out, KMD5SumFmt *md5, const char *path )
{
    rc_t rc;
    KLogLevel lvl = klogInt;
    
    if ( fp == NULL )
        rc = RC ( rcFS, rcFile, rcConstructing, rcParam, rcNull );
    else
    {
        if ( out == NULL )
            rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcNull );
        else if ( ! out -> write_enabled )
        {
            if ( out -> read_enabled )
                rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcReadonly );
            else
                rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcNoPerm );
        }
        else if ( md5 == NULL )
            rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcNull );
        else if ( md5 -> read_only )
            rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcReadonly );
        else if ( path == NULL )
            rc = RC ( rcFS, rcFile, rcConstructing, rcPath, rcNull );
        else if ( path [ 0 ] == 0 )
            rc = RC ( rcFS, rcFile, rcConstructing, rcPath, rcEmpty );
        else
        {
            KMD5File *f = malloc ( sizeof * f - sizeof f -> u +
                sizeof f -> u . wr + strlen ( path ) );
            if ( f == NULL )
            {
                rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
                lvl = klogSys;
            }
            else
            {
                rc = KFileInit ( & f -> dad,
                    ( const KFile_vt* ) & sKMD5FileAppend_vt, "KMD5File", path, out -> read_enabled, true );
                if ( rc == 0 )
                {
                    MD5StateInit ( & f -> md5 );
                    f -> file = out;
                    f -> fmt = md5;
		    f -> type = KMD5FileTypeWrite;

                    rc = KMD5SumFmtAddRef ( md5 );
                    if ( rc != 0 )
                    {
                        free ( f );
                        * fp = NULL;
                        return rc;
                    }

                    memset ( & f -> u . wr, 0, sizeof f -> u . wr );
                    strcpy ( f -> u . wr . path, path );

                    lvl = klogSys;
                    rc = KFileSize ( out, & f -> position );
                    if ( rc == 0 )
                    {
                        uint8_t buffer [ sizeof MD5TAG + sizeof ENDIANTAG + sizeof f -> md5 ];

                        /* an empty file is fine */
                        if ( f -> position == 0 )
                        {
                            * fp = f;
                            return 0;
                        }

                        /* set max */
                        f -> u . wr . max_position = f -> position;

                        /* a file without a trailing MD5 state is incorrect or corrupt */
                        if ( f -> position < sizeof buffer )
                        {
                            rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcIncorrect );
                            lvl = klogErr;
                        }
                        else
                        {
                            /* try to read MD5 state */
                            size_t num_read;
                            rc = KFileRead ( out, f -> position - sizeof buffer, buffer, sizeof buffer, & num_read );
                            if ( rc == 0 )
                            {
                                if ( num_read != sizeof buffer )
                                    rc = RC ( rcFS, rcFile, rcConstructing, rcTransfer, rcIncomplete );
                                else
                                {
                                    /* errors from here are usage errors */
                                    lvl = klogErr;
                                    
                                    if ( memcmp ( buffer, MD5TAG, sizeof MD5TAG ) != 0 )
                                        rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcIncorrect );
                                    else
                                    {
                                        /* forget about the MD5 state */
                                        f -> position -= sizeof buffer;
                                        memcpy ( & f -> md5, & buffer [ sizeof MD5TAG + sizeof ENDIANTAG ], sizeof f -> md5 );
                                        
                                        /* check for proper byte order */
                                        if ( memcmp ( & buffer [ sizeof MD5TAG ], ENDIANTAG, sizeof ENDIANTAG ) == 0 )
                                        {
                                            * fp = f;
                                            return 0;
                                        }

                                        /* probably swapped, but confirm */
                                        if ( memcmp ( & buffer [ sizeof MD5TAG ], ENDIANSWAPTAG, sizeof ENDIANSWAPTAG ) == 0 )
                                        {
                                            int i;

                                            for ( i = 0; i < sizeof f -> md5 . count / sizeof f -> md5 . count [ 0 ]; ++ i )
                                                f -> md5 . count [ i ] = bswap_32 ( f -> md5 . count [ i ] );
                                            for ( i = 0; i < sizeof f -> md5 . abcd / sizeof f -> md5 . abcd [ 0 ]; ++ i )
                                                f -> md5 . abcd [ i ] = bswap_32 ( f -> md5 . abcd [ i ] );

                                            * fp = f;
                                            return 0;
                                        }
                                        
                                        /* garbage */
                                        rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcCorrupt );
                                    }
                                }
                            }
                        }
                    }
                    
                    KMD5SumFmtRelease ( md5 );
                }
                
                free ( f );
            }
        }
        
        * fp = NULL;
    }
    
    LOGERR ( lvl, rc, "append-mode MD5 file" );
    
    return rc;
}

static KFile_vt_v1 sKMD5FileReadCreate_vt =
{
    1, 1,

    KMD5FileWhackCreate,
    KMD5FileGetSysFile,
    KMD5FileRandomAccessRead,
    KMD5FileSize,
    KMD5FileSetSizeRead,
    KMD5FileRead,
    KMD5FileBadWrite,

    /* 1.1 */
    KMD5FileType
};

/* MakeReadNew
 *  creates an adapter to create MD5 checksum for a file
 *
 *  "in" [ IN ] - input file 
 *  since this file is being wrapped by "f", ownership of
 *  the provided reference is transferred to "f"
 */
LIB_EXPORT rc_t CC KFileMakeNewMD5Read ( const KFile **fp,
    const struct KFile *in, KMD5SumFmt *md5, const char *path )
{
    rc_t rc;

    if ( fp == NULL )
        rc = RC ( rcFS, rcFile, rcConstructing, rcParam, rcNull );
    else
    {
        if ( in == NULL )
            rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcNull );
        else if ( ! in -> read_enabled )
	    rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcNoPerm );
        else if ( md5 == NULL )
            rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcNull );
        else if ( md5 -> read_only )
            rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcReadonly );
        else if ( path == NULL )
            rc = RC ( rcFS, rcFile, rcConstructing, rcPath, rcNull );
        else if ( path [ 0 ] == 0 )
            rc = RC ( rcFS, rcFile, rcConstructing, rcPath, rcEmpty );
        else
        {
            size_t path_size = string_size ( path );
            KMD5File *f = malloc ( sizeof * f + path_size );
            if ( f == NULL )
                rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
            else
            {
                rc = KFileInit ( & f -> dad,
                    ( const KFile_vt* ) & sKMD5FileReadCreate_vt, "KMD5File", path, in -> read_enabled, false );
                if ( rc == 0 )
                {
                    f -> position = 0;
                    MD5StateInit ( & f -> md5 );
                    f -> file = (KFile*)in;
                    f -> fmt = md5;
		    f -> type = KMD5FileTypeWrite;
                    memset ( & f -> u . wr, 0, sizeof f -> u . wr );
                    string_copy ( f -> u . wr . path, path_size + 1, path, path_size );
#if 0
/* KFileSize can't always be used */
                    rc = KFileSize ( in, & f -> u . wr . max_position );
#else
		    f->u.wr.max_position = 0;
#endif
                    if ( rc == 0 )
                        rc = KMD5SumFmtAddRef ( md5 );
                    if ( rc == 0 )
                    {
                        * fp = &f->dad;
                        return 0;
                    }
                }

                free ( f );
            }
        }

        * fp = NULL;
    }

    return rc;
}


/* end of file */
