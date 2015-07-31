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

#include <vdb/manager.h>
#include <vdb/schema.h>
#include <kfs/directory.h>
#include <kfs/file.h>
#include <kapp/main.h>
#include <kapp/args.h>
#include <klib/container.h>
#include <klib/vector.h>
#include <klib/log.h>
#include <klib/out.h>
#include <klib/rc.h>
#include <klib/namelist.h>
#include <os-native.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define WITH_DNA

#define DEPENDENCY_EXTENSION "d"

ver_t CC KAppVersion ( void )
{
    return 0x1000000;
}


/********************************************************************
helper function to display failure or success message
********************************************************************/
static void display_rescode( const rc_t rc, const char* failure, const char* success )
{
    if ( rc != 0 )
        LOGERR( klogInt, rc, failure );
    else
        if ( success ) LOGMSG( klogInfo, success );
}

typedef struct context
{
    const char *output_file;
    char *dependency_file;
    VNamelist *include_files;
    VNamelist *src_files;
} context;
typedef context* p_context;

static void context_set_str( char **dst, const char *src )
{
    size_t len;
    if ( dst == NULL ) return;
    if ( *dst != NULL ) free( *dst );
    if ( src == NULL ) return;
    len = strlen( src );
    if ( len == 0 ) return;
    *dst = (char*)malloc( len + 1 );
    strcpy( *dst, src );
}

static bool context_init( context **ctx )
{
    bool res = false;
    if ( ctx != NULL )
    {
        (*ctx) = malloc( sizeof( context ) );
        if ( *ctx )
        {
            (*ctx)->output_file = NULL;
            (*ctx)->dependency_file = NULL;
            VNamelistMake( &( (*ctx)->include_files ), 5 );
            VNamelistMake( &( (*ctx)->src_files ), 5 );
            res = true;
        }
    }
    return res;
}

static void context_destroy( p_context ctx )
{
    if ( ctx == NULL ) return;
    free( (char*)ctx->output_file );
    free( (char*)ctx->dependency_file );
    VNamelistRelease( ctx->include_files );
    VNamelistRelease( ctx->src_files );
    free( ctx );
}

const char UsageDefaultName [] = "sra-dflt-schema";


rc_t CC UsageSummary (const char * progname)
{
    return KOutMsg ("\n"
                    "Usage:\n"
                    "  %s src1 src2 -Iinclude1 -Iinclude2 -ooutput -Tdependency\n",
                    progname);
}

rc_t CC Usage ( const Args * args )
{
    rc_t rc;

    if ( args == NULL )
        rc = RC( rcApp, rcArgv, rcAccessing, rcSelf, rcNull );
    else
    {
        const char * progname = UsageDefaultName;
        const char * fullpath = UsageDefaultName;

        rc = ArgsProgram ( args, &fullpath, &progname );

        UsageSummary( progname );

        KOutMsg( "%s src1 src2 -Iinclude1 -Iinclude2 -ooutput -Tdependency\n"
                  "    src1, src2 ..........schema source-files  (mandatory))\n"
                  " -I include1, include2 ..schema include-files (optional))\n"
                  " -o output-file .........schema output (optional)) if none --> stdout\n"
                  " -T dependency-file .....list of includes in make-syntax (optional))\n",
                  progname );

        HelpVersion( fullpath, KAppVersion() );
    }
    return rc;
}


/* pass the new extension without a leading dot!
   assumes that the filename is not static, but made via malloc */
void change_extension( char **filename, const char *new_extension )
{
    char *temp, *buf;
    size_t src_len, ext_len;

    if ( filename == NULL ) return;
    if ( (*filename) == NULL ) return;
    if ( new_extension == NULL ) return;
    src_len = strlen( *filename );
    if ( src_len < 1 ) return;
    ext_len = strlen( new_extension );
    if ( ext_len < 1 ) return;
    buf = malloc( src_len + ext_len + 2 );
    temp = strrchr( *filename, '.' );
    if ( temp != NULL ) *temp = 0; /* terminate */
    sprintf( buf, "%s.%s", *filename, new_extension );
    temp = *filename;
    *filename = buf;
    free( temp );
}

#define OPTION_INCLUDE "include"
#define OPTION_DEPENDENCY "dependency"
#define OPTION_OUTPUT "output"
#define ALIAS_INCLUDE "I"
#define ALIAS_DEPENDENCY "T"
#define ALIAS_OUTPUT "o"

const char * include_usage[] = { "include schema", NULL };
const char * dependency_usage[] = { "dependency", NULL };
const char * output_usage[] = { "output", NULL };

OptDef Options[] =
{
/* is their a real reason for a limit of 10? */
    { OPTION_INCLUDE, ALIAS_INCLUDE, NULL, include_usage, 10, true, false },
    { OPTION_DEPENDENCY, ALIAS_DEPENDENCY, NULL, dependency_usage, 1, true, false },
    { OPTION_OUTPUT, ALIAS_OUTPUT, NULL, output_usage, 1, true, false }
};

static const char* get_str_option( const Args *my_args, const char *name )
{
    const char* res = NULL;
    uint32_t count;
    rc_t rc = ArgsOptionCount( my_args, name, &count );
    if ( ( rc == 0 )&&( count > 0 ) )
    {
        ArgsOptionValue( my_args, name, 0, (const void **)&res );
    }
    return res;
}

static void append_str_options( const Args *my_args, const char *name, VNamelist *dst )
{
    uint32_t count;
    rc_t rc = ArgsOptionCount( my_args, name, &count );
    if ( ( rc == 0 )&&( count > 0 ) )
    {
        uint32_t idx;
        for ( idx=0; idx<count; ++idx )
        {
            const char* s;
            if ( ArgsOptionValue( my_args, name, idx, (const void **)&s ) == 0 )
                VNamelistAppend( dst, s );
        }
    }
}

static void append_str_arguments( const Args *my_args, VNamelist *dst )
{
    uint32_t count;
    rc_t rc = ArgsParamCount( my_args, &count );
    if ( ( rc == 0 )&&( count > 0 ) )
    {
        uint32_t idx;
        for ( idx=0; idx<count; ++idx )
        {
            const char* s;
            if ( ArgsParamValue( my_args, idx, (const void **)&s ) == 0 )
                VNamelistAppend( dst, s );
        }
    }
}

static void evaluate_options( const Args *my_args, context *ctx )
{
    context_set_str( (char**)&(ctx->output_file),
                     get_str_option( my_args, OPTION_OUTPUT ) );
    context_set_str( (char**)&(ctx->dependency_file),
                     get_str_option( my_args, OPTION_DEPENDENCY ) );
    append_str_options( my_args, OPTION_INCLUDE, ctx->include_files );
    append_str_arguments( my_args, ctx->src_files );
}

static rc_t capture_arguments_and_options( Args * args, context *ctx )
{
    evaluate_options( args, ctx );
    change_extension( &(ctx->dependency_file), DEPENDENCY_EXTENSION );

    return 0;
}

typedef rc_t (*name_fkt)( void *data, const char *my_name );

static rc_t for_each_v_name( VNamelist *names, void *data, const name_fkt f )
{
    KNamelist *knames;
    uint32_t count, idx;
    rc_t rc = VNamelistToNamelist( names, (KNamelist **)&knames );
    display_rescode( rc, "VNamelistToNamelist failed", NULL );
    if ( rc == 0 )
    {
        rc = KNamelistCount( knames, &count );
        display_rescode( rc, "KNamelistCount failed", NULL );
        if ( rc == 0 )
        {
            for ( idx=0; idx<count; ++idx )
            {
                const char *my_path;
                rc = KNamelistGet( knames, idx, &my_path );
                display_rescode( rc, "KNamelistGet failed", NULL );
                if ( rc == 0 )
                    rc = f( data, my_path );
            }
        }
        KNamelistRelease( knames );
    }
    return rc;
}

static rc_t for_each_k_name( KNamelist *names, void *data, const name_fkt f )
{
    uint32_t count, idx;
    rc_t rc = KNamelistCount( names, &count );
    display_rescode( rc, "KNamelistCount failed", NULL );
    if ( rc == 0 )
    {
        for ( idx=0; idx<count; ++idx )
        {
            const char *my_path;
            rc = KNamelistGet( names, idx, &my_path );
            display_rescode( rc, "KNamelistGet failed", NULL );
            if ( rc == 0 )
                rc = f( data, my_path );
        }
    }
    return rc;
}

static uint32_t namelistcount( VNamelist *names )
{
    KNamelist *knames;
    uint32_t count = 0;

    if ( VNamelistToNamelist( names, (KNamelist **)&knames ) == 0 )
        KNamelistCount( knames, &count );
    return count;
}

static rc_t add_include( void *data, const char *my_name )
{
    VSchema *my_schema = (VSchema*)data;
    rc_t rc = VSchemaAddIncludePath( my_schema, "%s", my_name );
    display_rescode( rc, "VSchemaAddIncludePath failed", NULL );
    return rc;
}

static rc_t add_source( void *data, const char *my_name )
{
    VSchema *my_schema = (VSchema*)data;
    rc_t rc = VSchemaParseFile( my_schema, "%s", my_name );
    display_rescode( rc, "VSchemaParseFile failed", NULL );
    return rc;
}

struct schema_dumper_state_t {
    FILE *fp;
    unsigned line_pos;
    uint32_t out_size;
    char line_buf[255];
};

static void schema_dumper_state_write_line(struct schema_dumper_state_t *st) {
    st->line_buf[st->line_pos] = '\n';
    fwrite(st->line_buf, 1, st->line_pos + 1, st->fp);
    st->line_pos = 0;
}

/********************************************************************
helper function needed by schema-dump
********************************************************************/
static rc_t CC my_flush ( void *dst, const void *buffer, size_t bsize )
{
    struct schema_dumper_state_t *st = (struct schema_dumper_state_t *)dst;
    size_t i;
    
    st->out_size += bsize;
    for (i = 0; i != bsize; ++i) {
        int n;
        int c = ((const char *)buffer)[i];
        
        if (st->line_pos > 80)
            schema_dumper_state_write_line(st);

        if (c == '\'' || c == '\\')
            n = sprintf(st->line_buf + st->line_pos, " '\\%c',", c);
        else
            n = sprintf(st->line_buf + st->line_pos, " '%c',", c);
        st->line_pos += n;
    }
    return 0;
}

static rc_t Write1Dependency( void* data, const char* name )
{
    FILE *my_file = (FILE*)data;
    char prefix[] = { " \\\n  " };
    size_t to_write;

    if ( my_file == NULL ) return 0;
    if ( name == NULL ) return 0;
    to_write = strlen( name );
    if ( to_write == 0 ) return 0;
    fwrite( prefix, sizeof(char), strlen(prefix), my_file );
    fwrite( name, sizeof(char), to_write, my_file );
    return 0;
}

static rc_t WriteDependency( VSchema *my_schema, const context *ctx )
{
    rc_t rc = 0;
    FILE *my_file;
    const KNamelist *knames;

    if ( ctx->dependency_file == NULL ) return rc;
    if ( ctx->output_file == NULL ) return rc;
    rc = VSchemaIncludeFiles( my_schema, &knames );
    display_rescode( rc, "VSchemaIncludeFiles failed", NULL );
    if ( rc == 0 )
    {
        my_file = fopen( ctx->dependency_file, "w" );
        if ( my_file != NULL )
        {
            char buf[512];
            int len = snprintf( buf, sizeof(buf), "%s:", ctx->output_file );
            if ( ( len < 0 )||( len >= sizeof(buf) ) )
            {
                rc = -1;
            }
            else
            {
                fwrite( buf, sizeof(char), len, my_file );
                for_each_k_name( (void*)knames, my_file, Write1Dependency );
            }
            fclose( my_file );
        }
        KNamelistRelease( knames );
    }
    return rc;
}

static rc_t DumpSchema_to( VSchema *my_schema, const context *ctx )
{
    struct schema_dumper_state_t st;
    rc_t rc = 0;
    unsigned i;

    static const char *preamble[] = {
        "/*===========================================================================",
        "*",
        "*                             PUBLIC DOMAIN NOTICE",
        "*                National Center for Biotechnology Information",
        "*",
        "*   This software/database is a \"United States Government Work\" under the",
        "*   terms of the United States Copyright Act.  It was written as part of",
        "*   the author's official duties as a United States Government employee and",
        "*   thus cannot be copyrighted.  This software/database is freely available",
        "*   to the public for use. The National Library of Medicine and the U.S.",
        "*   Government have not placed any restriction on its use or reproduction.",
        "*",
        "*   Although all reasonable efforts have been taken to ensure the accuracy",
        "*   and reliability of the software and data, the NLM and the U.S.",
        "*   Government do not and cannot warrant the performance or results that",
        "*   may be obtained by using this software or data. The NLM and the U.S.",
        "*   Government disclaim all warranties, express or implied, including",
        "*   warranties of performance, merchantability or fitness for any particular",
        "*   purpose.",
        "*",
        "*   Please cite the author in any work or product based on this material.",
        "*",
        "* ===========================================================================",
        "*",
        "* THIS IS AN AUTO-GENERATED FILE - DO NOT EDIT",
        "*/",

        "#include \"sraschema-priv.h\"",
        "const char sra_schema_text[] = {"
    };
    static const char *postamble[] = {
        "};",
        "const size_t sra_schema_size =",
    };
    
    memset(&st, 0, sizeof(st));
    
    st.fp = ctx->output_file ? fopen(ctx->output_file, "w") : stdout;
    
    if (st.fp == NULL)
        return RC(rcSRA, rcSchema, rcFormatting, rcFile, rcNotOpen);

    for ( i = 0; i != sizeof(preamble) / sizeof(preamble[0]); ++i)
        fprintf(st.fp, "%s\n", preamble[i]);
    
    rc = VSchemaDump( my_schema, sdmCompact, NULL, my_flush, &st );
    if (rc == 0) {
        if (st.line_pos != 0)
            schema_dumper_state_write_line(&st);
        fprintf(st.fp, "%s\n", postamble[0]);
        fprintf(st.fp, "%s %u;\n", postamble[1], st.out_size);
    }
    if (st.fp != stdout)
        fclose(st.fp);
    return rc;
}

static rc_t DumpSchema( const context *ctx )
{
    KDirectory *my_dir;
    rc_t rc = KDirectoryNativeDir ( &my_dir );
    display_rescode( rc, "failed to open native dir", NULL );
    if ( rc == 0 )
    {
        const VDBManager *my_manager;
        rc = VDBManagerMakeRead ( &my_manager, my_dir );
        display_rescode( rc, "failed to make manager", NULL );
        if ( rc == 0 )
        {
            VSchema *my_schema;
            rc = VDBManagerMakeSchema( my_manager, &my_schema );
            display_rescode( rc, "failed to make schema", NULL );
            if ( rc == 0 )
            {
                rc = for_each_v_name( ctx->include_files, my_schema, add_include );
                if ( rc == 0 )
                {
                    rc = for_each_v_name( ctx->src_files, my_schema, add_source );
                    if ( rc == 0 )
                    {
                        rc = DumpSchema_to( my_schema, ctx );
                        if ( rc == 0 )
                            rc = WriteDependency( my_schema, ctx );
                    }
                }
                VSchemaRelease( my_schema );
            }
            VDBManagerRelease( my_manager );
        }
        KDirectoryRelease( my_dir );
    }
    return rc;
}

/****************************************************************************************
 ***************************************************************************************/
rc_t CC KMain ( int argc, char *argv [] )
{
    Args * my_args;
    rc_t rc;

    rc = ArgsMakeAndHandle (&my_args, argc, argv, 1, 
                            Options, sizeof (Options) / sizeof (OptDef));
    if (rc == 0)
    {
        context *ctx;

        context_init( &ctx );

        rc = capture_arguments_and_options( my_args, ctx );
        if ( rc == 0 )
        {
            if ( namelistcount( ctx->src_files ) > 0 )
            {
                rc = DumpSchema( ctx );
            }
            else
            {
                MiniUsage (my_args);
            }
        }
        context_destroy( ctx );

        ArgsWhack( my_args );
    }
    return rc;
}
