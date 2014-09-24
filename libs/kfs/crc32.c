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

#include <kfs/extern.h>

#define CRC_DBG_REF(msg) DBGMSG(DBG_LEGREF,DBG_FLAG(DBG_LEGREF_CRC), msg)

struct KCRC32File;
#define KFILE_IMPL struct KCRC32File

#include <kfs/crc.h>
#include <kfs/impl.h>
#include <klib/checksum.h>
#include <klib/container.h>
#include <klib/text.h>
#include <klib/log.h>
#include <klib/debug.h>
#include <klib/rc.h>
#include <os-native.h>
#include <sysalloc.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <endian.h>
#include <byteswap.h>


/*--------------------------------------------------------------------------
 * KCRC32SumEntry
 *  describes an entry into a CRC32 checksum file
 */
typedef struct KCRC32SumEntry KCRC32SumEntry;
struct KCRC32SumEntry
{
    SLNode n;
    uint32_t crc32;
    bool bin;
    char path [ 1 ];
};

static
void CC KCRC32SumEntryWhack ( SLNode *n, void *ignore )
{
    free ( n );
}

typedef struct KCRC32SumEntryFlushData KCRC32SumEntryFlushData;
struct KCRC32SumEntryFlushData
{
    uint64_t pos;
    KFile *f;
    rc_t rc;
};

static
bool CC KCRC32SumEntryFlush ( SLNode *n, void *data )
{
    KCRC32SumEntryFlushData *pb = data;
    const KCRC32SumEntry *self = ( const KCRC32SumEntry* ) n;

    int len;
    size_t num_writ;
    char line [ 4096 + 16 ];

    len = snprintf ( line, sizeof line, "%08x %c%s\n",
        self -> crc32, self -> bin ? '*' : ' ', self -> path );
    if ( len < 0 || len >= sizeof line )
    {
        pb -> rc = RC ( rcFS, rcFile, rcFormatting, rcString, rcExcessive );
        return true;
    }

    pb -> rc = KFileWrite ( pb -> f, pb -> pos, line, len, & num_writ );
    if ( pb -> rc != 0 )
        return true;

    pb -> pos += num_writ;

    if ( num_writ == ( size_t ) len )
        return false;

    pb -> rc = RC ( rcFS, rcFile, rcFormatting, rcTransfer, rcIncomplete );
    return true;
}

static
rc_t KCRC32SumEntryMake ( KCRC32SumEntry **ep,
    const char *path, uint32_t crc32, bool bin )
{
    KCRC32SumEntry *entry = malloc ( sizeof * entry + strlen ( path ) );
    if ( entry == NULL )
        return RC ( rcFS, rcFile, rcUpdating, rcMemory, rcExhausted );

    entry -> crc32 = crc32;
    entry -> bin = bin;
    strcpy ( entry -> path, path );

    * ep = entry;
    return 0;
}


/*--------------------------------------------------------------------------
 * KCRC32SumFmt
 *  a formatter for reading or writing an crc32sum-style checksum file
 */
struct KCRC32SumFmt
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
LIB_EXPORT rc_t CC KCRC32SumFmtFlush ( KCRC32SumFmt *self )
{
    if ( self != NULL && self -> dirty )
    {
        KCRC32SumEntryFlushData pb;
        pb . pos = 0;
        pb . f = self -> f;
        pb . rc = 0;

        SLListDoUntil ( & self -> entries, KCRC32SumEntryFlush, & pb );

        if ( pb . rc != 0 )
            return pb . rc;

        self -> dirty = false;
    }
    return 0;
}


/* Whack
 */
static
rc_t KCRC32SumFmtWhack ( KCRC32SumFmt *self )
{
    rc_t rc;

    atomic32_set ( & self -> refcount, 1 );

    rc = KCRC32SumFmtFlush ( self );
    if ( rc == 0 )
        rc = KFileRelease ( self -> f );
    if ( rc != 0 )
        return rc;

    SLListWhack ( & self -> entries, KCRC32SumEntryWhack, NULL );
    free ( self );
    return 0;
}


/* AddRef
 * Release
 */
LIB_EXPORT rc_t CC KCRC32SumFmtAddRef ( const KCRC32SumFmt *cself )
{
    if ( cself != NULL )
    {
        CRC_DBG_REF (("%s self %p prior count %u\n", __func__, cself, atomic32_read ( & cself -> refcount ) ));
        atomic32_inc ( & ( ( KCRC32SumFmt* ) cself ) -> refcount );
    }
    else
    {
        CRC_DBG_REF(("%s self %p\n", __func__, cself));
    }
    return 0;
}

LIB_EXPORT rc_t CC KCRC32SumFmtRelease ( const KCRC32SumFmt *cself )
{
    KCRC32SumFmt *self = ( KCRC32SumFmt* ) cself;
    if ( cself != NULL )
    {
        CRC_DBG_REF (("%s self %p prior count %u\n", __func__, cself, atomic32_read ( & cself -> refcount ) ));
        if ( atomic32_dec_and_test ( & self -> refcount ) )
            return KCRC32SumFmtWhack ( self );
    }
    else
    {
        CRC_DBG_REF(("%s self %p\n", __func__, cself));
    }
    return 0;
}


/* Extract
 *  extract a single line
 */
static
rc_t KCRC32SumFmtExtract ( KCRC32SumFmt *self, const char *line, size_t size )
{
    rc_t rc = 0;
    KCRC32SumEntry *entry;

    /* check for a DOS condition not caught by parse */
    if ( line [ size - 1 ] == '\r' )
    {
        /* drop CR from line */
        size -= 1;
    }

    /* quick validity check */
    if ( size < 11 || line [ 8 ] != ' ' )
        return RC ( rcFS, rcFile, rcConverting, rcFormat, rcInvalid );

    /* create entry */
    entry = malloc ( sizeof * entry + size - ( 8 /* cs */ + 2 /* sp + mode */ ) );
    if ( entry == NULL )
        rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
    else
    {
        /* read checksum */
        char *p;
        entry -> crc32 = strtoul ( line, & p, 16 );
        if ( ( ( const char* ) p - line ) != 8 )
            rc = RC ( rcFS, rcFile, rcConverting, rcFormat, rcInvalid );

        /* parse mode */
        else if ( line [ 9 ] == ' ' )
            entry -> bin = false;
        else if ( line [ 9 ] == '*' )
            entry -> bin = true;
        else
            rc = RC ( rcFS, rcFile, rcConverting, rcFormat, rcInvalid );

        /* the rest is path */
        if ( rc == 0 )
        {
            string_copy ( entry -> path, size, & line [ 10 ], size - 10 );
            SLListPushTail ( & self -> entries, & entry -> n );
            ++ self -> count;
            return 0;
        }

        free ( entry );
    }

    return rc;
}


/* Parse
 *  parse *.crc32 file
 */
static
rc_t KCRC32SumFmtParse ( KCRC32SumFmt *self, const char *start, const char *stop )
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
        if ( ( end - start ) < ( 8 /* cs */ + 2 /* sp + mode */ + 1 /* path */ ) )
        {
            /* check for empty line ending in CR-LF */
            if ( end [ -1 ] == '\r' && ( end - start ) == 1 )
                continue;

            /* other stuff is garbage */
            rc = RC ( rcFS, rcFile, rcParsing, rcData, rcInsufficient );
            break;
        }

        /* extract entry */
        rc = KCRC32SumFmtExtract ( self, start, end - start );
        if ( rc != 0 )
            break;
    }

    return rc;
}


/* Populate
 *  reads all lines of input from *.crc32 file
 */
static
rc_t KCRC32SumFmtPopulate ( KCRC32SumFmt *self, const KFile *in )
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
                rc = KCRC32SumFmtParse ( self, buff, buff + total );

            free ( buff );
        }
    }

    return rc;
}


/* Make
 *  creates a formatted accessor to textual checksum file
 */
LIB_EXPORT rc_t CC KCRC32SumFmtMakeRead ( const KCRC32SumFmt **fp, const KFile *in )
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
            KCRC32SumFmt *f = malloc ( sizeof * f );
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

                rc = KCRC32SumFmtPopulate ( f, in );
                if ( rc == 0 )
                {
                    f -> f = ( KFile* ) in;
                    * fp = f;
                    return 0;
                }
                
                KCRC32SumFmtWhack ( f );
            }
        }

        * fp = NULL;
    }

    return rc;
}

LIB_EXPORT rc_t CC KCRC32SumFmtMakeUpdate ( KCRC32SumFmt **fp, KFile *out )
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
            KCRC32SumFmt *f = malloc ( sizeof * f );
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
                    rc = KCRC32SumFmtPopulate ( f, out );
                if ( rc == 0 )
                {
                    f -> f = out;
                    * fp = f;
                    return 0;
                }
                
                KCRC32SumFmtWhack ( f );
            }
        }

        * fp = NULL;
    }

    return rc;
}


/* Count
 *  return the number of checksums in file
 */
LIB_EXPORT rc_t CC KCRC32SumFmtCount ( const KCRC32SumFmt *self, uint32_t *count )
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
 *  "crc32" [ OUT ] - return parameter for the checksum
 *
 *  "bin" [ OUT, NULL OKAY ] - true if fopen mode used to
 *  calculate "digest" was binary, false if text. pertinent if
 *  digest was calculated using DOS text mode.
 */
LIB_EXPORT rc_t CC KCRC32SumFmtGet ( const KCRC32SumFmt *self, uint32_t idx,
    char *path, size_t size, uint32_t *crc32, bool *bin )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcFS, rcFile, rcAccessing, rcSelf, rcNull );
    else if ( idx >= self -> count )
        rc = RC ( rcFS, rcFile, rcAccessing, rcId, rcNotFound );
    else
    {
        uint32_t i;
        const KCRC32SumEntry *entry = ( const KCRC32SumEntry* )
            SLListHead ( & self -> entries );
        for ( i = 0; entry != NULL && i < idx; ++ i )
        {
            entry = ( const KCRC32SumEntry* )
                SLNodeNext ( & entry -> n );
        }

        if ( entry == NULL )
            rc = RC ( rcFS, rcFile, rcAccessing, rcId, rcNotFound );
        else if ( path == NULL )
            rc = RC ( rcFS, rcFile, rcAccessing, rcBuffer, rcNull );
        else if ( crc32 == NULL )
            rc = RC ( rcFS, rcFile, rcAccessing, rcParam, rcNull );
        else
        {
            /* copy everything out */
            * crc32 = entry -> crc32;
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

    if ( crc32 != NULL )
        * crc32 = 0;

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
 *  "crc32" [ OUT ] - return parameter for the checksum
 *
 *  "bin" [ OUT, NULL OKAY ] - true if fopen mode used to
 *  calculate "digest" was binary, false if text. pertinent if
 *  digest was calculated using DOS text mode.
 */
LIB_EXPORT rc_t CC KCRC32SumFmtFind ( const KCRC32SumFmt *self,
    const char *path, uint32_t *crc32, bool *bin )
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
        const KCRC32SumEntry *entry = ( const KCRC32SumEntry* )
            SLListHead ( & self -> entries );
        while ( entry != NULL )
        {
            if ( strcmp ( path, entry -> path ) == 0 )
                break;

            entry = ( const KCRC32SumEntry* )
                SLNodeNext ( & entry -> n );
        }

        if ( entry == NULL )
            rc = RC ( rcFS, rcFile, rcAccessing, rcPath, rcNotFound );
        else if ( crc32 == NULL )
            rc = RC ( rcFS, rcFile, rcAccessing, rcParam, rcNull );
        else
        {
            /* copy everything out */
            *crc32 = entry -> crc32;
            if ( bin != NULL )
                * bin = entry -> bin;

            return 0;
        }
    }

    if ( crc32 != NULL )
        * crc32 = 0;

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
LIB_EXPORT rc_t CC KCRC32SumFmtDelete ( KCRC32SumFmt *self, const char *path )
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
        KCRC32SumEntry *entry = ( KCRC32SumEntry* )
            SLListHead ( & self -> entries );
        while ( entry != NULL )
        {
            if ( strcmp ( path, entry -> path ) == 0 )
                break;

            entry = ( KCRC32SumEntry* )
                SLNodeNext ( & entry -> n );
        }

        if ( entry == NULL )
            rc = RC ( rcFS, rcFile, rcRemoving, rcPath, rcNotFound );
        else
        {
            /* pop entry from list */
            SLListUnlink ( & self -> entries, & entry -> n );

            /* delete entry */
            KCRC32SumEntryWhack ( & entry -> n, NULL );

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
 *  "crc32" [ IN ] - calculated checksum
 *
 *  "bin" [ IN ] - true if checksum was calculated on all bytes
 *  in file, false if fopen mode used to read file was text.
 */
LIB_EXPORT rc_t CC KCRC32SumFmtUpdate ( KCRC32SumFmt *self, const char *path, uint32_t crc32, bool bin )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcFS, rcFile, rcUpdating, rcSelf, rcNull );
    else if ( path == NULL )
        rc = RC ( rcFS, rcFile, rcUpdating, rcString, rcNull );
    else if ( path [ 0 ] == 0 )
        rc = RC ( rcFS, rcFile, rcUpdating, rcString, rcEmpty );
    else
    {
        /* create an entry */
        KCRC32SumEntry *entry;
        rc = KCRC32SumEntryMake ( & entry, path, crc32, bin );
        if ( rc == 0 )
        {
            /* scan for existing entry */
            KCRC32SumEntry *exist = ( KCRC32SumEntry* )
                SLListHead ( & self -> entries );
            while ( exist != NULL )
            {
                if ( strcmp ( exist -> path, entry -> path ) == 0 )
                    break;
                exist = ( KCRC32SumEntry* )
                    SLNodeNext ( & exist -> n );
            }

            /* if new entry, insert */
            if ( exist == NULL )
            {
                SLListPushTail ( & self -> entries, & entry -> n );
                self -> dirty = true;
                ++ self -> count;
            }

            /* if existing, perform update */
            else
            {
                if ( exist -> crc32 != entry -> crc32 || exist -> bin != entry -> bin )
                {
                    exist -> crc32 = entry -> crc32;
                    exist -> bin = entry -> bin;
                    self -> dirty = true;
                }
                KCRC32SumEntryWhack ( & entry -> n, NULL );
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
LIB_EXPORT rc_t CC KCRC32SumFmtRename ( KCRC32SumFmt *self, const char *oldpath, const char *newpath )
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
        bool bin;
        uint32_t crc32;

        rc = KCRC32SumFmtFind ( self, oldpath, & crc32, & bin );
        if ( rc == 0 )
            rc = KCRC32SumFmtUpdate ( self, newpath, crc32, bin );
        if ( rc == 0 )
            rc = KCRC32SumFmtDelete ( self, oldpath );
    }

    return rc;
}


/*--------------------------------------------------------------------------
 * CRC32 KFile extensions
 */
typedef enum KCRC32FileUnionType
{
    KCRC32FileTypeRead,
    KCRC32FileTypeWrite
} KCRC32FileUnionType;

struct KCRC32File
{
    KFile dad;

    /* this is position of next byte to read or write
       relative to the CRC not actual read/write */
    uint64_t position;
    KFile *file;
    KCRC32SumFmt *fmt;
    uint32_t crc32;
    KCRC32FileUnionType type;
    union
    {
        struct
        {
            rc_t rc;
            uint32_t crc32;
            bool eof;
        } rd;

        struct
        {
            /* maximum position written to "file" */
            uint64_t max_position;

            /* saved information for a transaction */
            uint64_t tposition;
            uint32_t crc32;

            /* is transaction open */
            bool topen;

            /* path to file for crc32 fmt */
            char path [ 1 ];
        } wr;

    } u;
};

/* Whack
 */
static
rc_t CC KCRC32FileWhackRead ( KCRC32File *self )
{
    rc_t rc;

    atomic32_set ( & self -> dad . refcount, 1 );

    rc = KFileRelease ( self -> file );
    if ( rc == 0 )
        free ( self );

    return rc;
}

static
rc_t CC KCRC32FileWhackWrite ( KCRC32File *self )
{
    rc_t rc;

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
        KCRC32SumFmtUpdate ( self -> fmt, self -> u . wr . path, self -> crc32, true );
        KCRC32SumFmtRelease ( self -> fmt );
        free ( self );
    }

    return rc;
}

static const char CRCTAG [8] = "CRC32CTX";
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
rc_t CC KCRC32FileWrite ( KCRC32File *self, uint64_t pos,
    const void *buffer, size_t size, size_t *num_writ );

static
rc_t CC KCRC32FileWhackAppend ( KCRC32File *self )
{
    rc_t rc;

    /* if the file is empty, just whack it */
    if ( self -> position == 0 )
        return KCRC32FileWhackWrite ( self );

    /* begin transaction will save current state */
    rc = KCRC32FileBeginTransaction ( self );
    if ( rc == 0 )
    {
        size_t num_writ;

        /* create a closing tag with current checksum */
        uint8_t closer [ sizeof CRCTAG + sizeof ENDIANTAG + sizeof self -> crc32 ];
        memcpy ( closer, CRCTAG, sizeof CRCTAG );
        memcpy ( & closer [ sizeof CRCTAG ], ENDIANTAG, sizeof ENDIANTAG );
        memcpy ( & closer [ sizeof CRCTAG + sizeof ENDIANTAG ], & self -> crc32, sizeof self -> crc32 );
            
        /* append this to the file */
        rc = KCRC32FileWrite ( self, self -> position, closer, sizeof closer, & num_writ );
        if ( rc == 0 )
        {
            /* test transfer */
            if ( num_writ != sizeof closer )
                rc = RC ( rcFS, rcFile, rcDestroying, rcTransfer, rcIncomplete );
            else
            {
                /* complete operation */
                rc = KCRC32FileWhackWrite ( self );
                if ( rc == 0 )
                    return rc;
            }
        }

        /* restore entry state */
        KCRC32FileRevert ( self );
    }

    /* bail on errors */
    atomic32_set ( & self -> dad . refcount, 1 );
    return rc;
}


/* GetSysFile
 */
static
struct KSysFile *CC KCRC32FileGetSysFile ( const KCRC32File *self, uint64_t *offset )
{
    if ( offset != NULL )
        * offset = 0;
    return NULL;
}


/* RandomAccess
 */
static
rc_t CC KCRC32FileRandomAccessRead ( const KCRC32File *self )
{
    return KFileRandomAccess ( self -> file );
}

static
rc_t CC KCRC32FileRandomAccessWrite ( const KCRC32File *self )
{
    return RC ( rcFS, rcFile, rcAccessing, rcFunction, rcUnsupported );
}


/* Type
 */
static
uint32_t CC KCRC32FileType ( const KCRC32File *self )
{
    return KFileType ( self -> file );
}


/* Size
 */
static
rc_t CC KCRC32FileSize ( const KCRC32File *self, uint64_t *size )
{
    return KFileSize ( self -> file, size );
}

static
rc_t CC KCRC32FileSizeAppend ( const KCRC32File *self, uint64_t *size )
{
    *size = self -> position;
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
rc_t CC KCRC32FileSetSizeRead ( KCRC32File *self, uint64_t size )
{
    return RC ( rcFS, rcFile, rcUpdating, rcFunction, rcUnsupported );
}

static
rc_t CC KCRC32FileSetSizeWrite ( KCRC32File *self, uint64_t size )
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
        self -> crc32 = 0;
    }

    return rc;
}


/* Finish
 *  returns the results of crc comparison
 */
static
rc_t KCRC32FileFinish ( KCRC32File *self )
{
    if ( ! self -> u . rd . eof )
    {
        if ( self -> crc32 != self -> u. rd . crc32 )
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
rc_t KCRC32FileSeek ( KCRC32File *self, uint64_t pos )
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
            case KCRC32FileTypeRead:
                rc = KCRC32FileFinish ( self );
                break;
            case KCRC32FileTypeWrite:
                if ( self -> fmt != NULL)
                {
                    KCRC32SumFmtUpdate ( self -> fmt, self -> u . wr . path, self -> crc32, true );
                    KCRC32SumFmtRelease ( self -> fmt );
                    self -> fmt = NULL;
                }
                break;
            }
            break;
        }
        
        /* append bytes */
        self -> crc32 = CRC32 ( self -> crc32, buff, num_read );
    }
    
    return rc;
}

/* Read
 *  read-only version allows random access reads
 */
static
rc_t CC KCRC32FileRead ( const KCRC32File *cself,
    uint64_t pos, void *buffer, size_t bsize, size_t *num_read )
{
    rc_t rc;
    size_t total = 0;
    uint64_t end = pos + bsize;
    KCRC32File *self = ( KCRC32File* ) cself;

    * num_read = 0;

    /* no buffer means no bytes */
    if ( bsize == 0 )
        return 0;

    /* test for starting a read before the current marker
       meaning that it does not figure in to the checksum */
    if ( self -> position > pos )
    {
        /* test for entire buffer being before marker */
        if ( end <= self -> position )
            return KFileRead ( self -> file, pos, buffer, bsize, num_read );

        /* perform a partial read without affecting checksum */
        rc = KFileRead ( self -> file, pos, buffer, ( size_t )( self -> position - pos ), num_read );
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
        rc = KCRC32FileSeek ( self, pos );
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
            /* accumulate into checksum */
            self -> position += * num_read;
            self -> crc32 = CRC32 ( self -> crc32, buffer, * num_read );

            /* add in any previously read bytes and return success */
            * num_read += total;
            return 0;
        }

        switch (self->type)
        {
        case KCRC32FileTypeRead:
            /* EOF - get results */
            rc = KCRC32FileFinish ( self );
            break;
        case KCRC32FileTypeWrite:
            if ( self -> fmt != NULL)
            {
                KCRC32SumFmtUpdate ( self -> fmt, self -> u . wr . path, self -> crc32, true );
                KCRC32SumFmtRelease ( self -> fmt );
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
 *  writeable version isn't keeping track of checksum on read,
 *  so just forward to destination file
 */
static
rc_t CC KCRC32FileFwdRead ( const KCRC32File *self,
    uint64_t pos, void *buffer, size_t bsize, size_t *num_read )
{
    return KFileRead ( self -> file, pos, buffer, bsize, num_read );
}


/* Write
 */
static
rc_t CC KCRC32FileBadWrite ( KCRC32File *self, uint64_t pos,
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
rc_t CC KCRC32FileWrite ( KCRC32File *self, uint64_t pos,
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
        self -> crc32 = CRC32 ( self -> crc32, buffer, * num_writ );
    }

    return rc;
}


/* ToKFile
 *  a CRC file is a subclass of KFile
 *  this is an explicit cast operator
 */
LIB_EXPORT KFile* CC KCRC32FileToKFile ( KCRC32File *self )
{
    if ( self != NULL )
        return & self -> dad;
    return NULL;
}

LIB_EXPORT const KFile* CC KCRC32FileToKFileConst ( const KCRC32File *self )
{
    if ( self != NULL )
        return & self -> dad;
    return NULL;
}


/* BeginTransaction
 *  preserve current state and file marker
 *  in preparation for rollback
 */
LIB_EXPORT rc_t CC KCRC32FileBeginTransaction ( KCRC32File *self )
{
    if ( self == NULL )
        return RC ( rcFS, rcFile, rcFreezing, rcSelf, rcNull );
    if ( ! self -> dad . write_enabled )
        return RC ( rcFS, rcFile, rcFreezing, rcFile, rcReadonly );
    if ( self -> u . wr . topen )
        return RC ( rcFS, rcFile, rcFreezing, rcFile, rcBusy );

    self -> u . wr . topen = true;
    self -> u . wr . tposition = self -> position;
    self -> u . wr . crc32 = self -> crc32;

    return 0;
}


/* Commit
 *  accept current state as correct
 */
LIB_EXPORT rc_t CC KCRC32FileCommit ( KCRC32File *self )
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
LIB_EXPORT rc_t CC KCRC32FileRevert ( KCRC32File *self )
{
    if ( self == NULL )
        return RC ( rcFS, rcFile, rcReverting, rcSelf, rcNull );
    if ( ! self -> dad . write_enabled )
        return RC ( rcFS, rcFile, rcReverting, rcFile, rcReadonly );
    if ( ! self -> u . wr . topen )
        return RC ( rcFS, rcFile, rcReverting, rcFile, rcInconsistent );

    self -> position = self -> u . wr . tposition;
    self -> crc32 = self -> u . wr . crc32;
    self -> u . wr . topen = false;

    return 0;
}

/* Reset
 *  reset state to that of an empty file
 *  aborts any open transaction
 */
LIB_EXPORT rc_t CC KCRC32FileReset ( KCRC32File *self )
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
    self -> crc32 = 0;

    return 0;
}

/*--------------------------------------------------------------------------
 * CRC32File factories
 */


/* MakeCRC32Read
 *  creates an adapter to check CRC32 checksum for a source file
 *
 *  "f" [ OUT ] - return parameter for file with crc32 check
 *
 *  "src" [ IN ] - source file with read permission
 *
 *  "crc32" [ IN ] - checksum to match
 *
 * NB - creates a read-only file that does NOT support random access
 *  i.e. must be consumed serially starting from offset 0
 */
static KFile_vt_v1 KCRC32FileRead_vt =
{
    1, 1,

    /* 1.0 */
    KCRC32FileWhackRead,
    KCRC32FileGetSysFile,
    KCRC32FileRandomAccessRead,
    KCRC32FileSize,
    KCRC32FileSetSizeRead,
    KCRC32FileRead,
    KCRC32FileBadWrite,

    /* 1.1 */
    KCRC32FileType
};

LIB_EXPORT rc_t CC KFileMakeCRC32Read ( const KFile **fp,
    const KFile *src, uint32_t crc32 )
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
        else
        {
            KCRC32File *f = malloc ( sizeof * f - sizeof f -> u + sizeof f -> u . rd );
            if ( f == NULL )
                rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
            else
            {
                rc = KFileInit ( & f -> dad,
				     ( const KFile_vt* ) & KCRC32FileRead_vt, "KCRC32File", "no-name", true, false );
                if ( rc == 0 )
                {
                    f -> position = 0;
                    f -> crc32 = 0;
                    f -> file = ( KFile* ) src;
                    f -> fmt = NULL;
                    f -> type = KCRC32FileTypeRead;
                    f -> u . rd . rc = 0;
                    f -> u . rd . eof = 0;
                    f -> u . rd . crc32 = crc32;

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
 *  creates an adapter to create checksum for a file
 *
 *  "f" [ OUT ] - return parameter for file with crc32 check
 *
 *  "out" [ IN ] - output file with write permission
 *  since this file is being wrapped by "f", ownership of
 *  the provided reference is transferred to "f"
 *
 *  "fmt" [ IN ] - checksum output formatter.
 *  since this may be used for multiple checksums, "f" will
 *  attach its own reference to "fmt".
 *
 *  "path" [ IN ] - checksum output file path, used to
 *  create standard crc32sum file entry.
 *
 * NB - creates a write-only file that does NOT suppport random access
 *  i.e. must be written serially from offset 0
 */
static KFile_vt_v1 sKCRC32FileWrite_vt =
{
    1, 0,

    KCRC32FileWhackWrite,
    KCRC32FileGetSysFile,
    KCRC32FileRandomAccessWrite,
    KCRC32FileSize,
    KCRC32FileSetSizeWrite,
    KCRC32FileFwdRead,
    KCRC32FileWrite
};

LIB_EXPORT rc_t CC KCRC32FileMakeWrite ( KCRC32File **fp,
    KFile *out, KCRC32SumFmt *fmt, const char *path )
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
        else if ( fmt == NULL )
            rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcNull );
        else if ( fmt -> read_only )
            rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcReadonly );
        else if ( path == NULL )
            rc = RC ( rcFS, rcFile, rcConstructing, rcPath, rcNull );
        else if ( path [ 0 ] == 0 )
            rc = RC ( rcFS, rcFile, rcConstructing, rcPath, rcEmpty );
        else
        {
            KCRC32File *f = malloc ( sizeof * f - sizeof f -> u +
                sizeof f -> u . wr + strlen ( path ) );
            if ( f == NULL )
                rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
            else
            {
                rc = KFileInit ( & f -> dad,
                    ( const KFile_vt* ) & sKCRC32FileWrite_vt, "KCRC32File", path, out -> read_enabled, true );
                if ( rc == 0 )
                {
                    f -> position = 0;
                    f -> crc32 = 0;
                    f -> file = out;
                    f -> fmt = fmt;
                    f -> type = KCRC32FileTypeWrite;

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
                        rc = KCRC32SumFmtAddRef ( fmt );
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
 *  creates an adapter to create CRC32 checksum for a file
 *
 *  "f" [ OUT ] - return parameter for file with crc32 check
 *
 *  "out" [ IN ] - output file with write permission
 *  since this file is being wrapped by "f", ownership of
 *  the provided reference is transferred to "f"
 *
 *  "fmt" [ IN ] - checksum output formatter.
 *  since this may be used for multiple checksums, "f" will
 *  attach its own reference to "fmt".
 *
 *  "path" [ IN ] - checksum output file path, used to
 *  create standard crc32sum file entry.
 *
 * NB - creates a write-only file that does NOT suppport random access
 *  i.e. must be written serially from current EOF (minus the stored crc context)
 */
static KFile_vt_v1 sKCRC32FileAppend_vt =
{
    1, 0,

    KCRC32FileWhackAppend,
    KCRC32FileGetSysFile,
    KCRC32FileRandomAccessWrite,
    KCRC32FileSizeAppend,
    KCRC32FileSetSizeWrite,
    KCRC32FileFwdRead,
    KCRC32FileWrite
};

LIB_EXPORT rc_t CC KCRC32FileMakeAppend ( KCRC32File **fp, KFile *out, KCRC32SumFmt *fmt, const char *path )
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
        else if ( fmt == NULL )
            rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcNull );
        else if ( fmt -> read_only )
            rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcReadonly );
        else if ( path == NULL )
            rc = RC ( rcFS, rcFile, rcConstructing, rcPath, rcNull );
        else if ( path [ 0 ] == 0 )
            rc = RC ( rcFS, rcFile, rcConstructing, rcPath, rcEmpty );
        else
        {
            KCRC32File *f = malloc ( sizeof * f - sizeof f -> u +
                sizeof f -> u . wr + strlen ( path ) );
            if ( f == NULL )
            {
                rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
                lvl = klogSys;
            }
            else
            {
                rc = KFileInit ( & f -> dad,
                    ( const KFile_vt* ) & sKCRC32FileAppend_vt, "KCRC32File", path, out -> read_enabled, true );
                if ( rc == 0 )
                {
                    f -> crc32 = 0;
                    f -> file = out;
                    f -> fmt = fmt;
                    f -> type = KCRC32FileTypeWrite;

                    rc = KCRC32SumFmtAddRef ( fmt );
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
                        uint8_t buffer [ sizeof CRCTAG + sizeof ENDIANTAG + sizeof f -> crc32 ];

                        /* an empty file is fine */
                        if ( f -> position == 0 )
                        {
                            * fp = f;
                            return 0;
                        }

                        /* set max */
                        f -> u . wr . max_position = f -> position;

                        /* a file without a trailing crc state is incorrect or corrupt */
                        if ( f -> position < sizeof buffer )
                        {
                            rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcIncorrect );
                            lvl = klogErr;
                        }
                        else
                        {
                            /* try to read crc state */
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
                                    
                                    if ( memcmp ( buffer, CRCTAG, sizeof CRCTAG ) != 0 )
                                        rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcIncorrect );
                                    else
                                    {
                                        /* forget about the CRC state */
                                        f -> position -= sizeof buffer;
                                        memcpy ( & f -> crc32, & buffer [ sizeof CRCTAG + sizeof ENDIANTAG ], sizeof f -> crc32 );
                                        
                                        /* check for proper byte order */
                                        if ( memcmp ( & buffer [ sizeof CRCTAG ], ENDIANTAG, sizeof ENDIANTAG ) == 0 )
                                        {
                                            * fp = f;
                                            return 0;
                                        }

                                        /* probably swapped, but confirm */
                                        if ( memcmp ( & buffer [ sizeof CRCTAG ], ENDIANSWAPTAG, sizeof ENDIANSWAPTAG ) == 0 )
                                        {
                                            f -> crc32 = bswap_32 ( f -> crc32 );
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
                    
                    KCRC32SumFmtRelease ( fmt );
                }
                
                free ( f );
            }
        }
        
        * fp = NULL;
    }
    
    LOGERR ( lvl, rc, "append-mode CRC32 file" );
    
    return rc;
}

static KFile_vt_v1 sKCRC32FileReadCreate_vt =
{
    1, 1,

    KCRC32FileWhackRead,
    KCRC32FileGetSysFile,
    KCRC32FileRandomAccessRead,
    KCRC32FileSize,
    KCRC32FileSetSizeRead,
    KCRC32FileRead,
    KCRC32FileBadWrite,

    /* 1.1 */
    KCRC32FileType
};

LIB_EXPORT rc_t CC KFileMakeNewCRC32Read ( const KFile **fp,
    const KFile *in, KCRC32SumFmt *fmt, const char *path )
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
        else if ( fmt == NULL )
            rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcNull );
        else if ( fmt -> read_only )
            rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcReadonly );
        else if ( path == NULL )
            rc = RC ( rcFS, rcFile, rcConstructing, rcPath, rcNull );
        else if ( path [ 0 ] == 0 )
            rc = RC ( rcFS, rcFile, rcConstructing, rcPath, rcEmpty );
        else
        {
            KCRC32File *f = malloc ( sizeof * f - sizeof f -> u +
                sizeof f -> u . wr + strlen ( path ) );
            if ( f == NULL )
                rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
            else
            {
                rc = KFileInit ( & f -> dad,
                    ( const KFile_vt* ) & sKCRC32FileReadCreate_vt, "KCRC32File", path, in -> read_enabled, false );
                if ( rc == 0 )
                {
                    f -> position = 0;
                    f -> crc32 = 0;
                    f -> file = ( KFile* ) in;
                    f -> fmt = fmt;
                    f -> type = KCRC32FileTypeWrite;
                    memset ( & f -> u . wr, 0, sizeof f -> u . wr );
                    strcpy ( f -> u . wr . path, path );
#if 0
                    /* KFileSize can't always be used */
                    rc = KFileSize ( in, & f -> u . wr . max_position );
#endif
                    if ( rc == 0 )
                        rc = KCRC32SumFmtAddRef ( fmt );
                    if ( rc == 0 )
                    {
                        * fp = & f -> dad;
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
