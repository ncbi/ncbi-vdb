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

#include <kfg/extern.h>

#include <kfg/ngc.h> /* KNgcObjRelease */

#include <klib/rc.h>
#include <klib/text.h>
#include <klib/printf.h>
#include <klib/data-buffer.h>
#include <klib/refcount.h>

#include <kfg/ngc.h>
#include <kfs/file.h>
#include <kfs/subfile.h>
#include <kfs/gzip.h>

#include <strtol.h>

#include "ngc-priv.h"

#include <string.h>
#include <sysalloc.h>


#define MIN_ENC_KEY_LEN 1
#define MAX_ENC_KEY_LEN 256
#define MIN_DNLD_TICKET_LEN 1
#define MAX_DNLD_TICKET_LEN 256
#define MIN_DESCRIPTION_LEN 1
#define MAX_DESCRIPTION_LEN 256

static rc_t KNgcObjWhack ( KNgcObj * self )
{
    KDataBufferWhack ( & self-> buffer );
    free( self );
    return 0;
}


static rc_t KNgcObjParseIdKeyTicketDesc ( KNgcObj * self, uint32_t offset )
{
    rc_t rc = 0;
    uint64_t i;
    uint64_t l = ( self -> buffer . elem_count ) - offset;
    uint8_t state;
    const char * ptr = ( const char * ) self -> buffer . base;
    String projectId;
    String * dst = & projectId;

    memset ( & projectId, 0, sizeof projectId );

    ptr += offset;
    dst -> addr = ptr;
    for ( i = 0, state = 0; i < l && state < 4; i++ )
    {
        if ( ptr[ i ] == '|' )
        {
            dst -> size = dst -> len;
            switch( state )
            {
                case 0 : dst = & self -> encryptionKey; break;
                case 1 : dst = & self -> downloadTicket; break;
                case 2 : dst = & self -> description; break;
            }
            state ++;
            if ( state < 4 )
            {
                if ( i < ( l - 1 ) )
                    dst -> addr = &( ptr[ i + 1 ] );
            }
        }
        else
        {
            ( dst -> len )++;
        }
    }
    if ( projectId . addr == NULL ||
         self -> encryptionKey . addr == NULL ||
         self -> downloadTicket . addr == NULL ||
         self -> description . addr == NULL )
    {
        rc = RC ( rcKFG, rcFile, rcParsing, rcParam, rcInvalid );
    }
    else if ( projectId . len < 1 ||
               self -> encryptionKey . len < 1 ||
               self -> downloadTicket . len < 1 ||
               self -> description . len < 1 )
    {
        rc = RC ( rcKFG, rcFile, rcParsing, rcParam, rcInvalid );
    }

    /* the following tests may be version depended */

    /* test to verify that the project ID is a valid Integer */
    if ( rc == 0 )
    {
        char * end;
        self -> projectId = strtou32 ( projectId . addr, & end, 10 );
        if ( ( end - ( char* ) projectId . addr ) != projectId . size )
            rc = RC ( rcKFG, rcFile, rcParsing, rcParam, rcInvalid );
    }

    /* test that the download ticket and the encKey do have a minimum and maximum length */
    if ( rc == 0 )
    {
        if ( ( self -> downloadTicket . len < MIN_DNLD_TICKET_LEN ) ||
             ( self -> downloadTicket . len > MAX_DNLD_TICKET_LEN ) )
            rc = RC ( rcKFG, rcFile, rcParsing, rcParam, rcInvalid );
    }

    if ( rc == 0 )
    {
        if ( ( self -> encryptionKey . len < MIN_ENC_KEY_LEN ) ||
             ( self -> encryptionKey . len > MAX_ENC_KEY_LEN ) )
            rc = RC ( rcKFG, rcFile, rcParsing, rcParam, rcInvalid );
    }

    if ( rc == 0 )
    {
        if ( ( self -> description . len < MIN_DESCRIPTION_LEN ) ||
             ( self -> description . len > MAX_DESCRIPTION_LEN ) )
            rc = RC ( rcKFG, rcFile, rcParsing, rcParam, rcInvalid );
    }

    return rc;
}


static bool KNgcParseUntilfound ( const char * src, uint32_t l, const char * term, uint32_t term_count, String * dst )
{
    bool res = false;
    uint32_t i;

    dst -> addr = src;
    for ( i = 0; i < l && !res; ++i )
    {
        char * found = string_chr ( term, term_count, src[ i ] );
        res = ( found != NULL );
        if ( !res )
            ( dst -> len )++;
    }
    if ( res )
        dst -> size = dst -> len;
    else
        dst -> size = dst -> len = 0;
    return res;
}


static rc_t KNgcObjParseBuffer ( KNgcObj * self )
{
    rc_t rc = 0;
    uint32_t l = ( uint32_t ) self -> buffer . elem_count;
    const char * ptr = ( const char * ) self -> buffer . base;

    const char s_version[] = "version ";
    const char s_v1_0[] = "1.0";

    if ( ( size_t ) l != self -> buffer . elem_count )
        l = INT32_MAX;

    if ( string_cmp( s_version, sizeof s_version - 1, ptr, l, sizeof s_version - 1 ) != 0 )
        rc = RC( rcKFG, rcFile, rcParsing, rcFormat, rcUnrecognized );
    else if ( !KNgcParseUntilfound ( &( ptr[ 8 ] ), l - 8, "\n\r", 2, & self -> version ) )
        rc = RC( rcKFG, rcFile, rcParsing, rcFormat, rcUnrecognized );

    if ( rc == 0 )
    {
        if ( self -> version . len != 3 )
            rc = RC( rcKFG, rcFile, rcParsing, rcFormat, rcUnrecognized );
        else if ( string_cmp( s_v1_0, sizeof s_v1_0 - 1, self -> version . addr,  sizeof s_v1_0 - 1, sizeof s_v1_0 - 1 ) != 0 )
            rc = RC( rcKFG, rcFile, rcParsing, rcFormat, rcUnrecognized );
    }

    if ( rc == 0 )
    {
        uint32_t offset = sizeof s_version - 1 + ( self -> version . len ) + 1;
        rc = KNgcObjParseIdKeyTicketDesc ( self, offset );
    }
    return rc;
}


static rc_t KNgcObjInitFromString ( KNgcObj * self, const char * line )
{
    rc_t rc = 0;
    uint32_t len = string_measure( line, NULL );
    rc = KDataBufferResize ( &self -> buffer, len + 20 );
    if ( rc == 0 )
    {
        size_t written;
        rc = string_printf( self -> buffer . base, len + 20, &written, "version 1.0\n%s", line );
        if ( rc == 0 )
        {
            self -> buffer . elem_count = written;
            rc = KNgcObjParseBuffer ( self );
        }
    }
    return rc;
}


static rc_t KNgcObjInitFromFile ( KNgcObj * self, const struct KFile * src )
{
    char hdr [ 8 ];
    size_t num_read;
    rc_t rc = KFileReadAll ( src, 0, hdr, sizeof hdr, & num_read );
    if ( rc == 0 )
    {
        if ( num_read != sizeof hdr )
            rc = RC( rcKFG, rcFile, rcReading, rcFile, rcWrongType );
        else if ( memcmp( hdr, "ncbi_gap", sizeof hdr ) != 0 )
            rc = RC( rcKFG, rcFile, rcReading, rcFile, rcWrongType );
        else
        {
            uint64_t src_size;
            rc = KFileSize ( src, & src_size );
            if ( rc == 0 )
            {
                const struct KFile * sub;
                rc = KFileMakeSubRead ( &sub, src, sizeof hdr, src_size - sizeof hdr );
                if ( rc == 0 )
                {
                    const struct KFile * gzip;
                    rc = KFileMakeGzipForRead ( & gzip, sub );
                    if ( rc == 0 )
                    {
                        size_t to_read = ( src_size * 10 );
                        /* guessing that the unzip version will not be bigger that 10 x the zipped one */
                        rc = KDataBufferResize ( & self -> buffer, to_read );
                        if ( rc == 0 )
                        {
                            size_t num_read;
                            rc = KFileReadAll ( gzip, 0,  self -> buffer . base, to_read, & num_read );
                            if ( rc == 0 )
                            {
                                self -> buffer . elem_count = num_read;
                                rc = KNgcObjParseBuffer ( self );
                            }
                        }
                        KFileRelease ( gzip );
                    }
                    KFileRelease ( sub );
                }
            }
        }
    }
    return rc;
}


LIB_EXPORT rc_t CC KNgcObjMakeFromString ( const KNgcObj **ngc, const char * line )
{
    rc_t rc;
    if ( ngc == NULL || line == NULL )
        rc = RC ( rcKFG, rcMgr, rcAllocating, rcParam, rcNull );
    else
    {
        struct KNgcObj * f = calloc ( 1, sizeof * f );
        if ( f == NULL )
            rc = RC ( rcKFG, rcMgr, rcAllocating, rcMemory, rcExhausted );
        else
        {
            KRefcountInit ( & f -> refcount, 1, "KNgcObj", "init", "kfg" );
            memset ( & f -> buffer, 0, sizeof f -> buffer ); 
            rc = KDataBufferMakeBytes ( & f -> buffer, 0 );
            if ( rc == 0 )
            {
                rc = KNgcObjInitFromString( f, line );
                if ( rc == 0 )
                {
                    * ngc = f;
                    return rc;
                }
            }
            KNgcObjWhack ( f );
        }
        * ngc = NULL;
    }
    return rc;
}


LIB_EXPORT rc_t CC KNgcObjMakeFromFile ( const KNgcObj **ngc, const struct KFile * src )
{
    rc_t rc;
    if ( ngc == NULL || src == NULL )
        rc = RC ( rcKFG, rcFile, rcAllocating, rcParam, rcNull );
    else
    {
        struct KNgcObj * f = calloc ( 1, sizeof * f );
        if ( f == NULL )
            rc = RC ( rcKFG, rcFile, rcAllocating, rcMemory, rcExhausted );
        else
        {
            KRefcountInit ( & f -> refcount, 1, "KNgcObj", "init", "kfg" );
            memset ( & f -> buffer, 0, sizeof f -> buffer ); 
            rc = KDataBufferMakeBytes ( & f -> buffer, 0 );
            if ( rc == 0 )
            {
                rc = KNgcObjInitFromFile( f, src );
                if ( rc == 0 )
                {
                    * ngc = f;
                    return rc;
                }
            }
            KNgcObjWhack ( f );
        }
        * ngc = NULL;
    }
    return rc;
}


LIB_EXPORT rc_t CC KNgcObjAddRef ( const KNgcObj *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAdd( &self->refcount, "KNgcObj" ) )
        {
        case krefLimit:
            return RC ( rcKFG, rcFile, rcAttaching, rcRefcount, rcExcessive );
        case krefNegative:
            return RC ( rcKFG, rcFile, rcAttaching, rcRefcount, rcInvalid );
        }
    }
    return 0;
}


LIB_EXPORT rc_t CC KNgcObjRelease ( const KNgcObj *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDrop ( & self -> refcount, "KNgcObj" ) )
        {
        case krefWhack:
            return KNgcObjWhack ( ( KNgcObj * ) self );
        case krefNegative:
            return RC ( rcKFG, rcFile, rcReleasing, rcRefcount, rcInvalid );
        }
    }
    return 0;
}


LIB_EXPORT rc_t CC KNgcObjPrint ( const KNgcObj *self, char * buffer, size_t buffer_size, size_t * written )
{
    rc_t rc = 0;
    if ( self == NULL )
        rc = RC ( rcKFG, rcFile, rcFormatting, rcSelf, rcNull );
    else if ( buffer == NULL )
        rc = RC ( rcKFG, rcFile, rcFormatting, rcParam, rcNull );
    else
        rc = string_printf( buffer, buffer_size, written,
                            "Vers: '%S', ID:'%u', Key:'%S', Ticket:'%S', Desc:'%S'",
                            &self -> version, self -> projectId, &self -> encryptionKey,
                            &self -> downloadTicket, &self -> description );
    return rc;
}


LIB_EXPORT rc_t CC KNgcObjWriteToFile ( const KNgcObj *self, struct KFile * dst )
{
    rc_t rc = 0;
    if ( self == NULL )
        rc = RC ( rcKFG, rcFile, rcWriting, rcSelf, rcNull );
    else if ( dst == NULL )
        rc = RC ( rcKFG, rcFile, rcWriting, rcParam, rcNull );
    else
    {
        size_t written_to_hdr;
        char hdr [ 10 ];
        rc = string_printf( hdr, sizeof hdr, &written_to_hdr, "ncbi_gap" );
        if ( rc == 0 )
        {
            size_t written_to_file;
            rc = KFileWriteAll ( dst, 0, hdr, written_to_hdr, &written_to_file );
            if ( rc == 0 && written_to_hdr == written_to_file )
            {
                struct KFile * sub;
                rc = KFileMakeSubUpdate ( &sub, dst, written_to_file, 4096 );
                if ( rc == 0 )
                {
                    struct KFile * gzip;
                    rc = KFileMakeGzipForWrite ( &gzip, sub );
                    if ( rc == 0 )
                    {
                        size_t written_to_buffer;
                        char buffer[ 1024 ];
                        rc = string_printf( buffer, sizeof buffer, &written_to_buffer,
                                            "version %S\n%u|%S|%S|%S",
                                            &self -> version, self -> projectId, &self -> encryptionKey,
                                            &self -> downloadTicket, &self -> description );
                        if ( rc == 0 )
                        {
                            size_t written_to_gzip;
                            rc = KFileWriteAll ( gzip, 0, buffer, written_to_buffer, &written_to_gzip );
                        }
                        KFileRelease ( gzip );
                    }
                    KFileRelease ( sub );
                }
            }
        }
    }
    return rc;
}


LIB_EXPORT rc_t CC KNgcObjWriteKeyToFile ( const KNgcObj *self, struct KFile * dst )
{
    rc_t rc = 0;
    if ( self == NULL )
        rc = RC ( rcKFG, rcFile, rcWriting, rcSelf, rcNull );
    else if ( dst == NULL )
        rc = RC ( rcKFG, rcFile, rcWriting, rcParam, rcNull );
    else if ( self -> encryptionKey . addr == NULL || self -> encryptionKey . len < 1 )
        rc = RC ( rcKFG, rcFile, rcWriting, rcParam, rcInvalid );
    else
    {
        size_t written_to_file;
        rc = KFileWriteAll ( dst, 0, self -> encryptionKey . addr, self -> encryptionKey . len, &written_to_file );
    }
    return rc;
}


LIB_EXPORT rc_t CC KNgcObjGetProjectId ( const KNgcObj *self, uint32_t * id )
{
    rc_t rc;
    if ( id == NULL )
        rc = RC ( rcKFG, rcFile, rcFormatting, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcKFG, rcFile, rcFormatting, rcSelf, rcNull );
        else
        {
            * id = self -> projectId;
            return 0;
        }

        * id = 0;
    }
    return rc;
}

LIB_EXPORT rc_t CC KNgcObjGetProjectName ( const KNgcObj *self, char * buffer, size_t buffer_size, size_t * written )
{
    rc_t rc = 0;
    if ( self == NULL )
        rc = RC ( rcKFG, rcFile, rcFormatting, rcSelf, rcNull );
    else if ( buffer == NULL )
        rc = RC ( rcKFG, rcFile, rcFormatting, rcParam, rcNull );
    else
        rc = string_printf( buffer, buffer_size, written, "dbGaP-%u", self -> projectId );
    return rc;
}

LIB_EXPORT rc_t CC KNgcObjGetTicket(const KNgcObj *self,
    char * buffer, size_t buffer_size, size_t * written)
{
    rc_t rc = 0;

    if (self == NULL)
        rc = RC(rcKFG, rcFile, rcFormatting, rcSelf, rcNull);
    else if (buffer == NULL)
        rc = RC(rcKFG, rcFile, rcFormatting, rcParam, rcNull);
    else
        rc = string_printf(buffer, buffer_size, written, "%S",
            &self->downloadTicket);

    return rc;
}

rc_t KNgcObjGetEncryptionKey(const KNgcObj *self,
    char * buffer, size_t buffer_size, size_t * written)
{
    rc_t rc = 0;

    if (self == NULL)
        rc = RC(rcKFG, rcFile, rcFormatting, rcSelf, rcNull);
    else if (buffer == NULL)
        rc = RC(rcKFG, rcFile, rcFormatting, rcParam, rcNull);
    else
        rc = string_printf(buffer, buffer_size, written, "%S",
            &self->encryptionKey);

    return rc;
}
