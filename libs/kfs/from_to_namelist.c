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

#include <klib/rc.h>
#include <klib/text.h>
#include <klib/printf.h>
#include <klib/namelist.h>

#include <kfs/file.h>
#include <kfs/directory.h>

#include <sysalloc.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


/* ****************************************************************************************** */


#define STATE_ALPHA 0
#define STATE_LF 1
#define STATE_NL 2


typedef struct buffer_range
{
    const char * start;
    uint32_t processed, count, state;
} buffer_range;


static const char empty_str[ 2 ] = { ' ', 0 };


static void LoadFromBuffer( VNamelist * nl, buffer_range * range )
{
    uint32_t idx;
    const char * p = range->start;
    String S;

    S.addr = p;
    S.len = S.size = range->processed;
    for ( idx = range->processed; idx < range->count; ++idx )
    {
        switch( p[ idx ] )
        {
            case 0x0A : switch( range->state )
                        {
                            case STATE_ALPHA : /* ALPHA --> LF */
                                                VNamelistAppendString ( nl, &S );
                                                range->state = STATE_LF;
                                                break;

                            case STATE_LF : /* LF --> LF */
                                             VNamelistAppend ( nl, empty_str );
                                             break;

                            case STATE_NL : /* NL --> LF */
                                             VNamelistAppend ( nl, empty_str );
                                             range->state = STATE_LF;
                                             break;
                        }
                        break;

            case 0x0D : switch( range->state )
                        {
                            case STATE_ALPHA : /* ALPHA --> NL */
                                                VNamelistAppendString ( nl, &S );
                                                range->state = STATE_NL;
                                                break;

                            case STATE_LF : /* LF --> NL */
                                             range->state = STATE_NL;
                                             break;

                            case STATE_NL : /* NL --> NL */
                                             VNamelistAppend ( nl, empty_str );
                                             break;
                        }
                        break;

            default   : switch( range->state )
                        {
                            case STATE_ALPHA : /* ALPHA --> ALPHA */
                                                S.len++; S.size++;
                                                break;

                            case STATE_LF : /* LF --> ALPHA */
                                             S.addr = &p[ idx ]; S.len = S.size = 1;
                                             range->state = STATE_ALPHA;
                                             break;

                            case STATE_NL : /* NL --> ALPHA */
                                             S.addr = &p[ idx ]; S.len = S.size = 1;
                                             range->state = STATE_ALPHA;
                                             break;
                        }
                        break;
        }
    }
    if ( range->state == STATE_ALPHA )
    {
        range->start = S.addr;
        range->count = S.len;
    }
    else
        range->count = 0;
}


static rc_t LoadFromFile( struct KFile const * f, VNamelist * nl )
{
    rc_t rc = 0;
    uint64_t pos = 0;
    char buffer[ 4096 ];
    buffer_range range;
    bool done = false;

    range.start = buffer;
    range.count = 0;
    range.processed = 0;
    range.state = STATE_ALPHA;

    do
    {
        size_t num_read;
        rc = KFileRead ( f, pos, ( char * )( range.start + range.processed ),
                        ( sizeof buffer ) - range.processed, &num_read );
        if ( rc == 0 )
        {
            done = ( num_read == 0 );
            if ( !done )
            {
                range.start = buffer;
                range.count = range.processed + num_read;

                LoadFromBuffer( nl, &range );
                if ( range.count > 0 )
                {
                    memmove ( buffer, range.start, range.count );
                }
                range.start = buffer;
                range.processed = range.count;

                pos += num_read;
            }
            else if ( range.state == STATE_ALPHA )
            {
                String S;
                S.addr = range.start;
                S.len = S.size = range.count;
                VNamelistAppendString ( nl, &S );
            }
        }
    } while ( rc == 0 && !done );

    return rc;
}



/* -----

 * loads the content of a KFile into a Namelist
 *
 */
LIB_EXPORT rc_t CC LoadKFileToNameList( struct KFile const * self, VNamelist * namelist )
{
    rc_t rc;
    if ( self == NULL )
        rc = RC( rcFS, rcFile, rcValidating, rcSelf, rcNull );
    else if ( namelist == NULL )
        rc = RC( rcFS, rcFile, rcValidating, rcParam, rcNull );
    else
        rc = LoadFromFile( self, namelist );
    return rc;
}


LIB_EXPORT rc_t CC LoadFileByNameToNameList( VNamelist * namelist, const char * filename )
{
    rc_t rc;
    if ( namelist == NULL || filename == NULL )
        rc = RC( rcFS, rcFile, rcValidating, rcParam, rcNull );
    else
    {
        KDirectory * dir;
        rc = KDirectoryNativeDir ( &dir );
        if ( rc == 0 )
        {
            KFile const * f;
            rc = KDirectoryOpenFileRead ( dir, &f, "%s", filename );
            if ( rc == 0 )
            {
                if ( rc == 0 )
                    rc = LoadFromFile( f, namelist );
                KFileRelease ( f );
            }
            KDirectoryRelease ( dir );
        }
    }
    return rc;

}


static rc_t SaveToFile( struct KFile * f, const VNamelist * nl, const char * delim )
{
    uint32_t count;
    rc_t rc = VNameListCount ( nl, &count );
    if ( rc == 0 && count > 0 )
    {
        uint32_t idx;
        uint64_t pos = 0;
        for ( idx = 0; idx < count && rc == 0; ++idx )
        {
            const char * s;
            rc = VNameListGet ( nl, idx, &s );
            if ( rc == 0 && s != NULL )
            {
                size_t num_writ;
                rc = KFileWriteAll ( f, pos, s, string_size ( s ), &num_writ );
                if ( rc == 0 )
                {
                    pos += num_writ;
                    rc = KFileWriteAll ( f, pos, delim, string_size ( delim ), &num_writ );
                    if ( rc == 0 )
                        pos += num_writ;
                }
            }
        }
        if ( rc == 0 )
            rc = KFileSetSize ( f, pos );
    }
    return rc;
}


static rc_t ProcessFromBuffer( buffer_range * range,
    rc_t ( CC * on_line )( const String * line, void * data ), void * data  )
{
    rc_t rc = 0;
    uint32_t idx;
    const char * p = range->start;
    String S;

    S.addr = p;
    S.len = S.size = range->processed;
    for ( idx = range->processed; idx < range->count && rc == 0; ++idx )
    {
        switch( p[ idx ] )
        {
            case 0x0A : switch( range->state )
                        {
                            case STATE_ALPHA : /* ALPHA --> LF */
                                                rc = on_line( &S, data );
                                                range->state = STATE_LF;
                                                break;

                            case STATE_LF : /* LF --> LF */
                                             break;

                            case STATE_NL : /* NL --> LF */
                                             range->state = STATE_LF;
                                             break;
                        }
                        break;

            case 0x0D : switch( range->state )
                        {
                            case STATE_ALPHA : /* ALPHA --> NL */
                                                rc = on_line( &S, data );
                                                range->state = STATE_NL;
                                                break;

                            case STATE_LF : /* LF --> NL */
                                             range->state = STATE_NL;
                                             break;

                            case STATE_NL : /* NL --> NL */
                                             break;
                        }
                        break;

            default   : switch( range->state )
                        {
                            case STATE_ALPHA : /* ALPHA --> ALPHA */
                                                S.len++; S.size++;
                                                break;

                            case STATE_LF : /* LF --> ALPHA */
                                             S.addr = &p[ idx ]; S.len = S.size = 1;
                                             range->state = STATE_ALPHA;
                                             break;

                            case STATE_NL : /* NL --> ALPHA */
                                             S.addr = &p[ idx ]; S.len = S.size = 1;
                                             range->state = STATE_ALPHA;
                                             break;
                        }
                        break;
        }
    }
    if ( range->state == STATE_ALPHA )
    {
        range->start = S.addr;
        range->count = S.len;
    }
    else
        range->count = 0;
    return rc;
}


static rc_t ProcessLineByLine( struct KFile const * f,
        rc_t ( CC * on_line )( const String * line, void * data ), void * data )
{
    rc_t rc = 0;
    uint64_t pos = 0;
    char buffer[ 4096 ];
    buffer_range range;
    bool done = false;

    range.start = buffer;
    range.count = 0;
    range.processed = 0;
    range.state = STATE_ALPHA;

    do
    {
        size_t num_read;
        rc = KFileRead ( f, pos, ( char * )( range.start + range.processed ),
                        ( sizeof buffer ) - range.processed, &num_read );
        if ( rc == 0 )
        {
            done = ( num_read == 0 );
            if ( !done )
            {
                range.start = buffer;
                range.count = range.processed + num_read;

                rc = ProcessFromBuffer( &range, on_line, data );
                if ( range.count > 0 )
                {
                    memmove ( buffer, range.start, range.count );
                }
                range.start = buffer;
                range.processed = range.count;

                pos += num_read;
            }
            else if ( range.state == STATE_ALPHA )
            {
                String S;
                S.addr = range.start;
                S.len = S.size = range.count;
                rc = on_line( &S, data );
            }
        }
    } while ( rc == 0 && !done );

    return rc;
}

/* -----

 * processes each line in a KFile by the callback
 *
 */
LIB_EXPORT rc_t CC ProcessFileLineByLine( struct KFile const * self,
    rc_t ( CC * on_line )( const String * line, void * data ), void * data )
{
    rc_t rc;
    if ( self == NULL )
        rc = RC( rcFS, rcFile, rcValidating, rcSelf, rcNull );
    else if ( on_line == NULL )
        rc = RC( rcFS, rcFile, rcValidating, rcParam, rcNull );
    else
        rc = ProcessLineByLine( self, on_line, data );
    return rc;
}

 
 
/* -----

 * writes content of a Namelist into a KFile
 *
 */

LIB_EXPORT rc_t CC WriteNameListToKFile( struct KFile * self, const VNamelist * namelist, 
                                         const char * delim )
{
    rc_t rc;
    if ( self == NULL )
        rc = RC( rcFS, rcFile, rcValidating, rcSelf, rcNull );
    else if ( namelist == NULL )
        rc = RC( rcFS, rcFile, rcValidating, rcParam, rcNull );
    else
        rc = SaveToFile( self, namelist, delim );
    return rc;
}


LIB_EXPORT rc_t CC WriteNamelistToFileByName( const VNamelist * namelist, const char * filename,
                                              const char * delim )
{
    rc_t rc;
    if ( namelist == NULL || filename == NULL || delim == NULL )
        rc = RC( rcFS, rcFile, rcValidating, rcParam, rcNull );
    else
    {
        KDirectory * dir;
        rc = KDirectoryNativeDir ( &dir );
        if ( rc == 0 )
        {
            KFile * f;
            rc = KDirectoryCreateFile( dir, &f, true, 0664, kcmInit, "%s", filename );
            if ( rc == 0 )
            {
                if ( rc == 0 )
                    rc = SaveToFile( f, namelist, delim );
                KFileRelease ( f );
            }
            KDirectoryRelease ( dir );
        }
    }
    return rc;
}


typedef struct dir_entry_ctx
{
    VNamelist * namelist;
    bool add_files;
    bool add_dirs;
} dir_entry_ctx;


static rc_t CC on_dir_entry( const KDirectory * dir, uint32_t type, const char * name, void * data )
{
    rc_t rc = 0;
    if ( name != NULL && data != NULL && name[ 0 ] != 0 && name[ 0 ] != '.' )
    {
        dir_entry_ctx * dec = data;
        bool is_dir = ( ( type & ~kptAlias ) == kptDir );
        bool is_file = ( ( type & ~kptAlias ) == kptFile );
        if ( ( dec->add_dirs && is_dir ) || ( dec->add_files && is_file ) )
            rc = VNamelistAppend( dec->namelist, name );
    }
    return rc;
}

LIB_EXPORT rc_t CC ReadDirEntriesIntoToNamelist( VNamelist ** namelist, const KDirectory * dir,
    bool perform_sort, bool add_files, bool add_dirs, const char * path )
{
    rc_t rc;
    if ( namelist == NULL || dir == NULL )
        rc = RC( rcFS, rcFile, rcValidating, rcParam, rcNull );
    else
    {
        dir_entry_ctx dec;
        
        *namelist = NULL;
        rc = VNamelistMake( &dec.namelist, 25 );
        if ( rc == 0 )
        {
            dec.add_files = add_files;
            dec.add_dirs = add_dirs;

            rc = KDirectoryVisit( dir, false, on_dir_entry, &dec, "%s", path );
            if ( rc == 0 && perform_sort )
                VNamelistReorder( dec.namelist, false );
                
            if ( rc == 0 )
                *namelist = dec.namelist;
            else
                VNamelistRelease( dec.namelist );
        }
    }
    return rc;
}
