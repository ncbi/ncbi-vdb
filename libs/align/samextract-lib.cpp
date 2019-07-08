/* ===========================================================================
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

// C99 says that C++ has to enable this for stdint to apply. Various versions
// of g++ disagree if not set.
#define __STDC_LIMIT_MACROS
#include "samextract.h"
#include "samextract-pool.h"
#include "samextract-tokens.h"
#include <align/samextract-lib.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <kapp/args.h>
#include <kapp/main.h>
#include <kfs/file.h>
#include <klib/defs.h>
#include <klib/rc.h>
#include <klib/vector.h>
#include <kproc/queue.h>
#include <kproc/thread.hpp>
#include <kproc/timeout.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static char * fname_desc = NULL;
char          curline[READBUF_SZ + 1];
int           curline_len = 0;

void logmsg( const char * fname, int line, const char * func,
             const char * severity, const char * fmt, ... )
{
    const char * basename = strrchr( fname, '/' );
    if ( !basename )
        basename = strrchr( fname, '\\' );
    if ( basename )
        ++basename;
    if ( !basename )
        basename = fname;
    if ( fname_desc )
        fprintf( stderr, "`%s`:", fname_desc );
    fprintf( stderr, "\t[%s:%s():%d]\n", basename, func, line );
    va_list args;
    va_start( args, fmt );
    if ( vfprintf( stderr, fmt, args ) < 0 )
        fprintf( stderr, "bad log format: '%s'\n", fmt );
    va_end( args );
    fputc( '\n', stderr );
    fflush( stderr );
}

rc_t SAM_parseline( SAMExtractor * state )
{
    DBG( "Parsing line (%d bytes): '%s'", strlen( curline ), curline );
    SAMparse( state );
    return state->rc;
}

int moredata( char * buf, int * numbytes, size_t maxbytes )
{
    if ( !curline_len )
        DBG( "nomoredata" );
    else
        DBG( "  moredata %p %d\ncurline:'%s'", buf, maxbytes, curline );
    if ( curline_len > READBUF_SZ )
        ERR( "Bad length" );
    memmove( buf, curline, (size_t)curline_len );
    *numbytes   = curline_len;
    curline_len = 0;
    return 0;
}

inline rc_t readfile( SAMExtractor * state )
{
    if ( state->readbuf_pos == state->readbuf_sz )
    {
        state->readbuf_sz = READBUF_SZ;
        DBG( "reading in at %d", state->file_pos );
        size_t sz = state->readbuf_sz;
        rc_t rc = KFileReadAll( state->infile, state->file_pos, state->readbuf,
                                sz, &sz );
        state->readbuf_sz = (u32)sz;

        state->file_pos += state->readbuf_sz;
        if ( rc )
        {
            ERR( "readfile error" );
            state->rc = rc;
            return rc;
        }
        DBG( "Read in %d", state->readbuf_sz );
        state->readbuf_pos = 0;
        if ( !state->readbuf_sz )
        {
            DBG( "Buffer complete. EOF" );
        }
    }
    return 0;
}

void SAMerror( SAMExtractor * state, const char * s )
{
    ERR( " Parsing error: %s\nLine was:'%s'", s, curline );
    rc_t rc   = RC( rcAlign, rcRow, rcParsing, rcData, rcInvalid );
    state->rc = rc;
}

// No error checking, doesn't skip whitespace, produces bad data if non-digts
// present
i64 fast_strtoi64( const char * p )
{
    i64 val  = 0;
    i64 sign = 1;
    if ( *p == '-' )
    {
        ++p;
        sign = -1;
    }

    while ( *p )
    {
        val *= 10;
        val += ( *p - '0' );
        ++p;
    }

    return sign * val;
}

/* low<=str<=high */
bool inrange( const char * str, i64 low, i64 high )
{
    i64 i = fast_strtoi64( str );
    if ( errno )
        return false;
    if ( i < low || i > high )
        return false;
    return true;
}

bool ismd5( const char * str )
{
    size_t i;
    size_t len = strlen( str );

    if ( len != 32 )
        return false;

    for ( i = 0; i != len; ++i )
    {
        if ( !isalnum( str[i] ) && str[i] != '*' )
            return false;
    }

    return true;
}

/* Avoiding handling this as flex token because so many other ReadGroup values
 * could end up looking like flow orders.
 */
bool isfloworder( const char * str )
{
    size_t i;
    size_t len = strlen( str );

    if ( len == 1 && str[0] == '*' )
        return true;
    for ( i = 0; i != len; ++i )
    {
        switch ( str[i] )
        {
          case 'A':
          case 'C':
          case 'M':
          case 'G':
          case 'R':
          case 'S':
          case 'V':
          case 'T':
          case 'W':
          case 'Y':
          case 'H':
          case 'K':
          case 'D':
          case 'B':
          case 'N':
              continue;
          default:
              return false;
        }
    }
    return true;
}

bool check_cigar( const char * cigar, const char * seq )
{
    static unsigned char mult[256];
    size_t               cigarlen = 0;

    if ( cigar[0] == '*' && cigar[1] == '\0' )
        return true;

    size_t seqlen = strlen( seq );

    // faster to just set these rather than test if initialized
    mult[(u8)'='] = 1;
    mult[(u8)'I'] = 1;
    mult[(u8)'M'] = 1;
    mult[(u8)'S'] = 1;
    mult[(u8)'X'] = 1;

    char *       opcodestr = (char *)pool_alloc( 1 + strlen( cigar ) / 2 );
    char *       opcode    = opcodestr;
    const char * p         = cigar;
    while ( *p )
    {
        size_t val = 0;
        while ( *p && isdigit( *p ) )
        {
            val *= 10;
            val += ( *p - '0' );
            ++p;
        }
        const char op = *p++;
        *opcode++     = op;

        cigarlen += val * mult[(unsigned int)op];
    }
    *opcode = '\0';

    // "Sum of lengths of the M/I/S/=/X operations shall equal the length of
    // SEQ."
    if ( cigarlen != seqlen )
    {
        return false;
    }

    // TODO: "Recommended practice #2: Adjacent CIGAR operations should be
    // different

    // "H can only be present as the first and/or last operation."
    // "S may only have H operations between them and the ends of the CIGAR
    // string."
    // Actual valid rule is apparently H?S?[MIDNPX=]+S?H?, but H*S*...S*H*
    // would also be compliant.
    opcode = opcodestr;
    while ( *opcode == 'H' )
        ++opcode;
    while ( *opcode == 'S' )
        ++opcode;
    if ( !*opcode )
        return true;
    if ( *opcode == 'H' )
        return false;
    static bool valid[256];
    valid[(u8)'M'] = true;
    valid[(u8)'I'] = true;
    valid[(u8)'D'] = true;
    valid[(u8)'N'] = true;
    valid[(u8)'P'] = true;
    valid[(u8)'X'] = true;
    valid[(u8)'='] = true;
    while ( valid[(u8)*opcode] )
        ++opcode;  // main loop
    if ( !*opcode )
        return true;
    while ( *opcode == 'S' )
        ++opcode;
    if ( !*opcode )
        return true;
    while ( *opcode == 'H' )
        ++opcode;
    if ( !*opcode )
        return true;

    return false;
}

rc_t process_header( SAMExtractor * state, const char * type, const char * tag,
                     const char * value )
{
    DBG( "processing header type:%s tag:%s value:%s", type, tag, value );
    if ( strcmp( type, "HD" ) && strcmp( type, "SQ" ) && strcmp( type, "RG" )
         && strcmp( type, "PG" ) )
    {
        ERR( "record '%s' must be HD, SQ, RG or PG", type );
        rc_t rc   = RC( rcAlign, rcRow, rcParsing, rcData, rcInvalid );
        state->rc = rc;
        return rc;
    }

    if ( strlen( tag ) != 2 )
    {
        ERR( "tag '%s' must be 2 characters", tag );
        rc_t rc   = RC( rcAlign, rcRow, rcParsing, rcData, rcInvalid );
        state->rc = rc;
        return rc;
    }

    if ( islower( tag[0] && islower( tag[1] ) ) )
    {
        DBG( "optional tag" );
    }

    if ( value == NULL )
    {
        ERR( "Null value for type:%s tag:%s", type, tag );
        rc_t rc   = RC( rcAlign, rcRow, rcParsing, rcData, rcInvalid );
        state->rc = rc;
        return rc;
    }

    TagValue * tv = (TagValue *)pool_alloc( sizeof( *tv ) );
    tv->tag       = pool_strdup( tag );
    tv->value     = pool_strdup( value );
    VectorAppend( &state->tagvalues, NULL, tv );

    return 0;
}

rc_t mark_headers( SAMExtractor * state, const char * type )
{
    DBG( "mark_headers" );
    Header * hdr    = (Header *)pool_alloc( sizeof( *hdr ) );
    hdr->headercode = type;
    VectorCopy( &state->tagvalues, &hdr->tagvalues );
    VectorAppend( &state->headers, NULL, hdr );
    VectorWhack( &state->tagvalues, NULL, NULL );
    return 0;
}

// Returns true if we can skip record
bool filter( const SAMExtractor * state, String * srname, ssize_t pos )
{
    if ( state->filter_rname && StringEqual( state->filter_rname, srname ) )
        return true;

    if ( pos < 0 )
        return false;  // No filtering if pos uncertain

    if ( state->filter_pos != -1 )
    {
        if ( pos < state->filter_pos )  // Before pos
            return true;

        if ( state->filter_length != -1 )
            if ( pos > ( state->filter_pos
                         + state->filter_length ) )  // After pos+length
                return true;
    }

    return false;
}

rc_t process_alignment( SAMExtractor * state, const char * qname,
                        const char * flag, const char * rname, const char * pos,
                        const char * mapq, const char * cigar,
                        const char * rnext, const char * pnext,
                        const char * tlen, const char * seq, const char * qual )
{
    DBG( "Have %d alignments", VectorLength( &state->alignments ) );
    DBG( "process_alignment qname=%s flag=%d rname=%s", qname, flag, rname );

    i64 ipos = fast_strtoi64( pos );

    if ( ipos < 0 )
        ERR( "POS not in range %s", pos );

    if ( state->file_type == SAM )
    {
        if ( !inrange( flag, 0, UINT16_MAX ) )
            ERR( "Flag not in range %s", flag );

        if ( !inrange( pos, 0, INT32_MAX ) )
            ERR( "Pos not in range %s", flag );

        if ( !inrange( mapq, 0, UINT8_MAX ) )
            ERR( "MAPQ not in range %s", mapq );

        if ( !inrange( pnext, 0, INT32_MAX ) )
            ERR( "PNEXT not in range %s", pnext );

        if ( !inrange( tlen, INT32_MIN, INT32_MAX ) )
            ERR( "TLEN not in range %s", tlen );

        if ( !strcmp( rnext, "=" ) )
            rnext = rname;

        if ( qual && qual[0] != '*' )
            if ( strlen( qual ) != strlen( seq ) )
                WARN( "QUAL and SEQ length mismatch %d %d", strlen( qual ),
                      strlen( seq ) );
    }

    if ( cigar && strlen( cigar ) && !check_cigar( cigar, seq ) )
    {
        WARN( "CIGAR '%s' and sequence '%s' mismatch", cigar, seq );
    }

    String srname;
    // Faster to avoid string_measure()
    // StringInitCString(&srname, rname);
    size_t l = strlen( rname );
    StringInit( &srname, rname, l, (u32)l );

    if ( filter( state, &srname, ipos ) )
    {
        DBG( "Skipping" );
        return 0;
    }

    // TODO: ordered

    Alignment * align = (Alignment *)pool_alloc( sizeof( *align ) );

    align->qname = qname;
    align->flags = fast_strtoi64( flag );
    align->rname = rname;
    align->pos   = ipos;
    align->mapq  = fast_strtoi64( mapq );
    align->cigar = cigar;
    align->rnext = rnext;
    align->pnext = fast_strtoi64( pnext );
    align->tlen  = fast_strtoi64( tlen );
    align->read  = seq;
    align->qual  = qual;
    VectorAppend( &state->alignments, NULL, align );
    DBG( "Now Have %d alignments", VectorLength( &state->alignments ) );

    return 0;
}

// Reads next line into curline, returns false if file complete.
static bool readline( SAMExtractor * state )
{
    if ( readfile( state ) )
        return false;
    char * line = curline;
    line[0]     = '\0';
    curline_len = 0;
    // Is there a newline in current buffer?
    char * nl = (char *)memchr( ( state->readbuf + state->readbuf_pos ), '\n',
                                ( state->readbuf_sz - state->readbuf_pos ) );
    if ( nl )
    {
        nl += 1;
        u32 len = ( u32 )( nl - ( state->readbuf + state->readbuf_pos ) );
        if ( len > READBUF_SZ )
            ERR( "Bad length" );
        memmove( line, state->readbuf + state->readbuf_pos, len );
        curline_len += len;
        state->readbuf_pos += len;
        if ( memchr( line, 0, curline_len - 1 ) )
        {
            ERR( "NULLZ found in line: '%s'", line );
            rc_t rc   = RC( rcAlign, rcRow, rcParsing, rcData, rcInvalid );
            state->rc = rc;
            return false;
        }
        line[curline_len + 1] = '\0';
        return true;
    }

    // Nope, append and get more
    u32 len = ( u32 )( state->readbuf_sz - state->readbuf_pos );
    memmove( line, state->readbuf + state->readbuf_pos, len );
    line += len;
    curline_len += len;

    state->readbuf_pos = state->readbuf_sz;
    if ( readfile( state ) )
        return false;

    // Better be a newline now
    nl = (char *)memchr( state->readbuf, '\n', state->readbuf_sz );
    if ( !nl )
    {
        //        ERR("No newline present");
        return false;
    }
    nl += 1;
    len = ( u32 )( nl - state->readbuf );
    curline_len += len;
    if ( curline_len > READBUF_SZ )
    {
        ERR( "No newline present" );
        return false;
    }
    memmove( line, state->readbuf, len );
    state->readbuf_pos += len;
    curline[curline_len + 1] = '\0';

    if ( memchr( curline, 0, curline_len - 1 ) )
    {
        ERR( "NULLZ found in line: '%s' %d", curline, curline_len );
        rc_t rc   = RC( rcAlign, rcRow, rcParsing, rcData, rcInvalid );
        state->rc = rc;
        return false;
    }
    return true;
}

LIB_EXPORT rc_t CC SAMExtractorMake( SAMExtractor ** state, const KFile * fin,
                                     String * fname, int32_t num_threads = -1 )
{
    SAMExtractor * s = (SAMExtractor *)calloc( 1, sizeof( *s ) );
    *state           = s;

    pool_init();

    s->infile  = fin;
    fname_desc = strdup( fname->addr );
    //    s->fname = fname_desc;

    VectorInit( &s->headers, 0, 0 );
    VectorInit( &s->alignments, 0, 0 );
    VectorInit( &s->tagvalues, 0, 0 );
    VectorInit( &s->bam_references, 0, 0 );

    s->prev_headers = NULL;
    s->prev_aligns  = NULL;

    s->num_threads = num_threads;

    // Default number of threads to number of cores
    if ( s->num_threads <= -1 )
#if LINUX
        s->num_threads = (int)sysconf( _SC_NPROCESSORS_ONLN ) - 1;
#else
    s->num_threads = 8;
#endif

    DBG( "%d threads", s->num_threads );
    DBG( "fname is '%s'", fname_desc );

    VectorInit( &s->threads, 0, 0 );
    KQueueMake( &s->inflatequeue, 64 );
    KQueueMake( &s->parsequeue, 64 );

    s->pos      = 0;
    s->file_pos = 0;

    s->readbuf     = (char *)malloc( READBUF_SZ + 1 );
    s->readbuf_sz  = 0;
    s->readbuf_pos = 0;

    s->file_type = unknown;
    s->n_ref     = -1;

    s->rc = 0;

    s->filter_rname   = NULL;
    s->filter_pos     = -1;
    s->filter_length  = -1;
    s->filter_ordered = false;

    s->hashdvn = false;
    s->hashdso = false;
    s->hashdgo = false;
    s->hassqsn = false;
    s->hassqln = false;
    s->hasrgid = false;
    s->haspgid = false;

    return 0;
}

LIB_EXPORT rc_t CC SAMExtractorRelease( SAMExtractor * s )
{
    DBG( "complete release_Extractor" );
    SAMlex_destroy();

    SAMExtractorInvalidateHeaders( s );
    SAMExtractorInvalidateAlignments( s );

    releasethreads( s );

    pool_destroy();
    VectorWhack( &s->headers, NULL, NULL );
    for ( u32 i = 0; i != VectorLength( &s->bam_references ); ++i )
        free( (void *)VectorGet( &s->bam_references, i ) );
    VectorWhack( &s->bam_references, NULL, NULL );
    KQueueRelease( s->inflatequeue );
    KQueueRelease( s->parsequeue );
    VectorWhack( &s->threads, NULL, NULL );
    free( s->readbuf );
    free( s->filter_rname );
    free( fname_desc );
    fname_desc = NULL;
    memset( s, 0, sizeof( SAMExtractor ) );
    free( s );

    return 0;
}

LIB_EXPORT rc_t CC SAMExtractorAddFilterName( SAMExtractor * state,
                                              String * srname, bool ordered )
{
    return SAMExtractorAddFilterNamePosLength( state, srname, -1, -1, ordered );
}

LIB_EXPORT rc_t CC SAMExtractorAddFilterNamePos( SAMExtractor * state,
                                                 String * srname, ssize_t pos,
                                                 bool ordered )
{
    return SAMExtractorAddFilterNamePosLength( state, srname, pos, -1,
                                               ordered );
}

LIB_EXPORT rc_t CC SAMExtractorAddFilterPos( SAMExtractor * state, ssize_t pos,
                                             bool ordered )
{
    return SAMExtractorAddFilterNamePosLength( state, NULL, pos, -1, ordered );
}

LIB_EXPORT rc_t CC SAMExtractorAddFilterPosLength( SAMExtractor * state,
                                                   ssize_t pos, ssize_t length,
                                                   bool ordered )
{
    return SAMExtractorAddFilterNamePosLength( state, NULL, pos, length,
                                               ordered );
}

LIB_EXPORT rc_t CC SAMExtractorAddFilterNamePosLength( SAMExtractor * state,
                                                       String *       srname,
                                                       ssize_t        pos,
                                                       ssize_t        length,
                                                       bool           ordered )
{
    // TODO: Check if GetHeaders/GetAlignments already invoked

    state->filter_rname   = srname;
    state->filter_pos     = pos;
    state->filter_length  = length;
    state->filter_ordered = ordered;

    return 0;
}

LIB_EXPORT rc_t CC SAMExtractorGetHeaders( SAMExtractor * s, Vector * headers )
{
    rc_t rc = 0;
    DBG( "GetHeaders" );
    if ( s->file_type != unknown )
        ERR( "Recycled state?" );

    u64 sz = 0;
    rc     = KFileSize( s->infile, &sz );
    if ( rc )
        return rc;
    DBG( "File size=%u", sz );
    if ( sz < 12 )
    {
        ERR( "File too small" );
        return RC( rcAlign, rcRow, rcParsing, rcData, rcInvalid );
    }

    rc = readfile( s );
    if ( rc )
        return rc;

    if ( !memcmp( s->readbuf, "\x1f\x8b\x08", 3 ) )
    {
        DBG( "gzip file, BAM or SAM.gz" );
        s->file_type = BAM;
    }
    else if ( s->readbuf[0] == '@' )
    {
        DBG( "SAM file" );
        s->file_type = SAM;
    }
    else
    {
        ERR( "Unkown magic, not a SAM file." );
        return RC( rcAlign, rcFile, rcParsing, rcData, rcInvalid );
    }

    switch ( s->file_type )
    {
      case BAM:
          rc = threadinflate( s );
          if ( rc )
              return rc;
          rc = BAMGetHeaders( s );
          if ( rc )
              return rc;
          break;
      case SAM:
          while ( readline( s ) )
          {
              rc = SAM_parseline( s );
              if ( rc )
                  return rc;

              if ( curline[0] != '@' )
              {
                  // First line of alignments will be processed
                  DBG( "out of headers" );
                  break;
              }
          }
          if ( s->rc )
              return s->rc;
          break;
      case SAMGZUNSUPPORTED:
      case unknown:
          ERR( "Unknown file type" );
          return RC( rcAlign, rcFile, rcParsing, rcData, rcInvalid );
    }

    DBG( "Done parsing headers" );
    VectorInit( headers, 0, 0 );
    VectorCopy( &s->headers, headers );
    s->prev_headers = headers;
    return 0;
}

LIB_EXPORT rc_t CC SAMExtractorInvalidateHeaders( SAMExtractor * s )
{
    DBG( "invalidate_headers" );
    for ( u32 i = 0; i != VectorLength( &s->headers ); ++i )
    {
        Header * hdr = (Header *)VectorGet( &s->headers, i );

        hdr->headercode = NULL;

        Vector * tvs = &hdr->tagvalues;
        for ( u32 j = 0; j != VectorLength( tvs ); ++j )
        {
            TagValue * tv = (TagValue *)VectorGet( tvs, j );
            tv->tag       = NULL;
            tv->value     = NULL;
        }
        VectorWhack( &hdr->tagvalues, NULL, NULL );
        hdr = NULL;
    }
    // TODO: FIXME
    //    pool_destroy();
    //    pool_init();
    VectorWhack( &s->headers, NULL, NULL );
    VectorWhack( &s->tagvalues, NULL, NULL );
    VectorWhack( s->prev_headers, NULL, NULL );
    s->prev_headers = NULL;
    return 0;
}

LIB_EXPORT rc_t CC SAMExtractorGetAlignments( SAMExtractor * s,
                                              Vector *       alignments )
{
    rc_t rc = 0;
    DBG( "GetAlignments" );
    //    SAMExtractorInvalidateAlignments(s);
    //    VectorInit(&s->alignments, 0, 0);
    VectorInit( alignments, 0, 0 );

    if ( s->file_type == SAM )
    {
        while ( VectorLength( &s->alignments ) < 64 )
        {
            if ( !readline( s ) )
                break;

            if ( curline[0] == '@' )
            {
                ERR( "header restarted" );
                rc_t rc = RC( rcAlign, rcRow, rcParsing, rcData, rcInvalid );
                return rc;
            }

            rc = SAM_parseline( s );
            if ( rc )
                return rc;
        }

        DBG( "Done parsing %d alignments", VectorLength( &s->alignments ) );
    }
    else if ( s->file_type == BAM )
    {
        rc = BAMGetAlignments( s );
        DBG( "complete parsing %d alignments", VectorLength( &s->alignments ) );
        if ( rc )
        {
            ERR( "BAMGetAlignments failed" );
            return rc;
        }
    }
    else
    {
        ERR( "Unknown file type" );
    }

    VectorCopy( &s->alignments, alignments );
    VectorWhack( &s->alignments, NULL, NULL );
    s->prev_aligns = alignments;

    return 0;
}

LIB_EXPORT rc_t CC SAMExtractorInvalidateAlignments( SAMExtractor * s )
{
    size_t num = VectorLength( &s->alignments );
    DBG( "invalidate_alignments %d", num );
    pool_destroy();
    pool_init();
    VectorWhack( &s->alignments, NULL, NULL );
    VectorWhack( s->prev_aligns, NULL, NULL );
    s->prev_aligns = NULL;

    return 0;
}
