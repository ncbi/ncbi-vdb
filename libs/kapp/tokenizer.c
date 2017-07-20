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

#include <kapp/extern.h>
#include <sysalloc.h>

#include <kapp/args.h>

#include <klib/rc.h>
#include <klib/log.h>
#include <klib/text.h>

#include <vfs/path.h>
#include <vfs/manager.h>
#include <kfs/file.h>

#include <os-native.h>

#include <string.h>
#include <stdlib.h>


#define ARGV_INC 10
#define TOKEN_INC 512

/*
   strtok() not used, because we tokenize the content of a file!
   we do that in chunks of 4096 char's
   a token can span multiple chunks!
   we also support quoting and escaping ( a la bash )
*/

typedef struct tokenzr
{
    int * argc;
    char *** argv;
    char *token;
    size_t allocated;
    size_t used;
    uint32_t state;
    uint32_t nargs;
    char escape[ 3 ];
    char n_escape;
    char m_escape;
} tokenzr;


static rc_t make_tokenzr( tokenzr **t, int * argc, char *** argv )
{
    rc_t rc = 0;
    if ( t == NULL )
    {
        rc = RC( rcApp, rcNoTarg, rcConstructing, rcSelf, rcNull );
    }
    else
    {
        tokenzr *t1 = malloc( sizeof *t1 );
        if ( t1 == NULL )
        {
            rc = RC( rcApp, rcNoTarg, rcConstructing, rcMemory, rcExhausted );
        }
        else
        {
            t1->token = malloc( TOKEN_INC );
            if ( t1->token == NULL )
            {
                rc = RC( rcApp, rcNoTarg, rcConstructing, rcMemory, rcExhausted );
            }
            else
            {
                t1->allocated = TOKEN_INC;
                t1->used = 0;
                t1->state = 0;
                t1->nargs = 0;
                t1->argc = argc;
                t1->argv = argv;
                t1->n_escape = 0;
                t1->m_escape = 0;
            }
        }
        if ( rc == 0 )
        {
            *t = t1;
        }
    }
    return rc;
}


static void free_tokenzr( tokenzr *t )
{
    if ( t != NULL )
    {
        if ( t->token != NULL )
            free( t->token );
        free( t );
    }
}


static rc_t expand_argv( tokenzr *t )
{
    rc_t rc = 0;
    char ** pargv = *(t->argv);

    if ( pargv == NULL )
    {
        pargv = malloc( ARGV_INC * ( sizeof pargv[ 0 ] ) );
        if ( pargv != NULL )
        {
            t->nargs = ARGV_INC;
            ( *(t->argc) ) = 0;
        }
        else
            rc = RC( rcApp, rcNoTarg, rcConstructing, rcMemory, rcExhausted );
    }
    else
    {
        if ( (uint32_t) ( *(t->argc) ) >= t->nargs )
        {
            char ** pargv1 = realloc( pargv, ( ( t->nargs + ARGV_INC ) * ( sizeof pargv[ 0 ] ) ) );
            if ( pargv1 != NULL )
            {
                t->nargs += ARGV_INC;
                pargv = pargv1;
            }
            else
            {
                rc = RC( rcApp, rcNoTarg, rcConstructing, rcMemory, rcExhausted );
                free( pargv );
                pargv = NULL;
            }
        }
    }
    if ( rc == 0 )
    {
        *(t->argv) =  pargv;
    }
    return rc;
}


static rc_t add_string_to_argv( tokenzr *t, const char * str, size_t len )
{
    rc_t rc = expand_argv( t );
    if ( rc == 0 )
    {
        int argc = *(t->argc);
        char ** pargv = *(t->argv);

        pargv[ argc ] = string_dup ( str, len );
        if ( pargv[ argc ] == NULL )
            rc = RC( rcApp, rcNoTarg, rcConstructing, rcMemory, rcExhausted );
        else
        {
            ( *(t->argc) )++;
            *(t->argv) =  pargv;
            t->used = 0;
        }
    }
    return rc;
}


static rc_t add_token_to_argv( tokenzr *t )
{
    return add_string_to_argv( t, t->token, t->used );
}


static rc_t add_buffer_to_token( tokenzr *t, const char *buffer, size_t buflen )
{
    rc_t rc = 0;
    if ( t->used + buflen > t->allocated )
    {
        size_t new_size = t->used + buflen + TOKEN_INC;
        char * temp = realloc( t->token, new_size );
        if ( temp == NULL )
        {
            rc = RC( rcApp, rcNoTarg, rcConstructing, rcMemory, rcExhausted );
        }
        else
        {
            t->token = temp;
            t->allocated = new_size;
        }
    }
    if ( rc == 0 )
    {
        memmove( t->token + t->used, buffer, buflen );
        t->used += buflen;
    }
    return rc;
}


/* this is the 'normal' state, that the buffer is divided into tokens by white-space */
const static char delim0[] = " \r\n\f\t\v\\\"";
static rc_t tokenize_state0( tokenzr *t, bool *done, char ** ptr, const char *buffer, size_t buflen )
{
    rc_t rc;
    char * s = strpbrk ( *ptr, delim0 );
    if ( s == NULL )
    {
        /* delimiting char not found!
            ---> add everything to the tokenbuffer */
        size_t to_add = buflen - ( *ptr - buffer );
        rc = add_buffer_to_token( t, *ptr, to_add );
        *done = true;
    }
    else
    {
        /* delimiting char found!
            ---> add everything from ptr to s to the tokenbuffer
            ---> then add token to argv
            ---> then clear token */
        rc = add_buffer_to_token( t, *ptr, s - *ptr );
        if ( rc == 0 )
        {
            if ( t->used > 0 && *s != '\\' )
                rc = add_token_to_argv( t );
            *ptr = s + 1;
            *done = ( *ptr >= buffer + buflen );
            switch( *s )
            {
                case '"'  : t->state =  1; break;
                case '\\' : t->state =  2; break;
            }
        }
    }
    return rc;
}


/* this state, provides 'quoting' ( prevents tokenizing by putting quotes "" around strings
    which would be otherwise separated by whitespace ) */
const static char delim1[] = "\"";
static rc_t tokenize_state1( tokenzr *t, bool *done, char ** ptr, const char *buffer, size_t buflen )
{
    rc_t rc;
    char * s = strpbrk ( *ptr, delim1 );
    if ( s == NULL )
    {
        /* delimiting char not found!
            ---> add everything to the tokenbuffer */
        size_t to_add = buflen - ( *ptr - buffer );
        rc = add_buffer_to_token( t, *ptr, to_add );
        *done = true;
    }
    else
    {
        /* delimiting char found!
            ---> add everything from ptr to s to the tokenbuffer
            ---> then add token to argv
            ---> then clear token */
        rc = add_buffer_to_token( t, *ptr, s - *ptr );
        if ( rc == 0 )
        {
            if ( t->used > 0 )
                rc = add_token_to_argv( t );
            *ptr = s + 1;
            *done = ( *ptr >= buffer + buflen );
            if ( *s == '"' )
                t->state = 0;
        }
    }
    return rc;
}


/* this state, provides 'escaping' ( \nnn or \xHH or \' or \" ) */
static rc_t tokenize_state2( tokenzr *t, bool *done, char ** ptr, const char *buffer, size_t buflen )
{
    rc_t rc = 0;
    char c = **ptr;
    switch( c )
    {
        case '\\' : /* no break intended ! */
        case '\'' :
        case '\"' : rc = add_buffer_to_token( t, (*ptr)++, 1 );
                    break;

        case 't'  : rc = add_buffer_to_token( t, "\t", 1 );
                    (*ptr)++;
                    break;
        case 'n'  : rc = add_buffer_to_token( t, "\n", 1 );
                    (*ptr)++;
                    break;
        case 'r'  : rc = add_buffer_to_token( t, "\r", 1 );
                    (*ptr)++;
                    break;

    }
    t->state = 0;
    *done = ( *ptr >= buffer + buflen );
    return rc;
}


static rc_t tokenize_buffer( tokenzr *t, const char *buffer, size_t buflen )
{
    rc_t rc = 0;
    char * ptr = ( char * ) buffer;
    bool done = false;
    while ( rc == 0 && !done )
        switch( t->state )
        {
            case 0 : rc = tokenize_state0( t, &done, &ptr, buffer, buflen ); break;
            case 1 : rc = tokenize_state1( t, &done, &ptr, buffer, buflen ); break;
            case 2 : rc = tokenize_state2( t, &done, &ptr, buffer, buflen ); break;
        }
    return rc;
}


static rc_t tokenize_file_and_progname_into_argv( const char * filename, const char * progname,
                                                  int * argc, char *** argv )
{
    rc_t rc2, rc = 0;
    VFSManager *vfs_mgr;

    ( *argv ) = NULL;
    ( *argc ) = 0;
    rc = VFSManagerMake ( &vfs_mgr );
    if ( rc != 0 )
        LOGERR( klogInt, rc, "VFSManagerMake() failed" );
    else
    {
        VPath * vfs_path;
        rc = VFSManagerMakePath ( vfs_mgr, &vfs_path, "%s", filename );
        if ( rc != 0 )
            LOGERR( klogInt, rc, "VPathMake() failed" );
        else
        {
            struct KFile const *my_file;
            rc = VFSManagerOpenFileRead ( vfs_mgr, &my_file, vfs_path );
            if ( rc != 0 )
                LOGERR( klogInt, rc, "VFSManagerOpenFileRead() failed" );
            else
            {
                tokenzr *t;
                uint64_t pos = 0;
                char buffer[ 4096 + 1 ];
                size_t num_read;

                rc = make_tokenzr( &t, argc, argv );
                if ( rc != 0 )
                    LOGERR( klogInt, rc, "make_tokenzr() failed" );
                else
                {
                    if ( progname != NULL )
                        rc = add_string_to_argv( t, progname, string_size( progname ) );

                    if ( rc == 0 )
                    {
                        do
                        {
                            rc = KFileRead ( my_file, pos, buffer, ( sizeof buffer ) - 1, &num_read );
                            if ( rc != 0 )
                                LOGERR( klogInt, rc, "KFileRead() failed" );
                            else if ( num_read > 0 )
                            {
                                buffer[ num_read ]  = 0;
                                rc = tokenize_buffer( t, buffer, num_read );
                                if ( rc != 0 )
                                    LOGERR( klogInt, rc, "tokenize_buffer() failed" );
                                pos += num_read;
                            }
                        } while ( rc == 0 && num_read > 0 );
                    }

                    if ( rc == 0 && t->used > 0 )
                    {
                        rc = add_token_to_argv( t );
                        if ( rc != 0 )
                            LOGERR( klogInt, rc, "add_token_to_argv() failed" );
                    }
                    free_tokenzr( t );
                }
                rc2 = KFileRelease ( my_file );
                if ( rc2 != 0 )
                    LOGERR( klogInt, rc2, "KFileRelease() failed" );
            }
            rc2 = VPathRelease ( vfs_path );
            if ( rc2 != 0 )
                LOGERR( klogInt, rc2, "VPathRelease() failed" );
        }
        rc2 = VFSManagerRelease ( vfs_mgr );
        if ( rc2 != 0 )
            LOGERR( klogInt, rc2, "VFSManagerRelease() failed" );
    }
    return rc;
}


rc_t CC Args_tokenize_file_into_argv( const char * filename, int * argc, char *** argv )
{
    return tokenize_file_and_progname_into_argv( filename, NULL, argc, argv );
}


rc_t CC Args_tokenize_file_and_progname_into_argv( const char * filename, const char * progname,
                                                               int * argc, char *** argv )
{
    return tokenize_file_and_progname_into_argv( filename, progname, argc, argv );
}


void CC Args_free_token_argv( int argc, char * argv[] )
{
    if ( argv != NULL )
    {
        int i;
        for ( i = 0; i < argc; ++i )
            free( argv[ i ] );
        free( argv );
    }
}


static int sized_str_cmp( const char *a, const char *b )
{
    size_t asize = string_size ( a );
    size_t bsize = string_size ( b );
    return strcase_cmp ( a, asize, b, bsize, (uint32_t) ( ( asize > bsize ) ? asize : bsize ) );
}


/* get's the filename following a file_option - parameter into the file-name-buffer */
rc_t CC Args_find_option_in_argv( int argc, char * argv[],
                                              const char * option_name,
                                              char * option, size_t option_len )
{
    rc_t rc = 0;
    int i, opt_idx;

    if ( option == NULL )
        return RC( rcRuntime, rcArgv, rcConstructing, rcParam, rcNull );

    option[ 0 ] = 0;

    if ( argv == NULL || argc < 1 )
        return RC( rcRuntime, rcArgv, rcConstructing, rcSelf, rcNull );
    if ( option_name == NULL || option_len == 0 )
        return RC( rcRuntime, rcArgv, rcConstructing, rcParam, rcNull );

    opt_idx = -1;
    for ( i = 0; i < argc && ( option[ 0 ] == 0 ); ++i )
    {
        if ( sized_str_cmp( (const char *)argv[i], option_name ) == 0 )
        {
                opt_idx = ( i + 1 );
        }
        else if ( i == opt_idx )
        {
            string_copy( option, option_len, argv[i], string_size( argv[i] ) );
        }
    }
    if ( option[ 0 ] == 0 )
        rc = RC( rcRuntime, rcArgv, rcConstructing, rcParam, rcNotFound );
    return rc;
}


rc_t CC Args_parse_inf_file( Args * args, const char * file_option )
{
    uint32_t count;
    rc_t rc = ArgsOptionCount( args, file_option, &count );
    if ( rc != 0 )
        LOGERR( klogInt, rc, "ArgsOptionCount() failed" );
    else if ( count > 0 )
    {
        uint32_t count2 = 0;
        do
        {
            uint32_t idx;
            for ( idx = count2; idx < count && rc == 0; ++idx )
            {
                const char *filename;
                rc = ArgsOptionValue( args, file_option, idx, (const void **)&filename );
                if ( rc != 0 )
                    LOGERR( klogInt, rc, "ArgsOptionValue() failed" );
                else if ( filename != NULL )
                {
                    int argc;
                    char ** argv;
                    rc = Args_tokenize_file_into_argv( filename, &argc, &argv );
                    if ( rc == 0 && argv != NULL && argc > 0 )
                    {
                        rc = ArgsParse ( args, argc, argv );
                        Args_free_token_argv( argc, argv );
                    }
                }
            }
            count2 = count;
            rc = ArgsOptionCount( args, file_option, &count );
            if ( rc != 0 )
                LOGERR( klogInt, rc, "ArgsOptionCount() failed" );
        } while ( rc == 0 && count > count2 );
    }
    return rc;
}
