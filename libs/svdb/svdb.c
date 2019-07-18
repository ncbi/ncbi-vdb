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

#include <svdb/extern.h>
#include <svdb/svdb.h>

#include <klib/rc.h>
#include <klib/log.h>
#include <klib/out.h>
#include <klib/debug.h>
#include <klib/text.h>
#include <klib/vector.h>
#include <klib/pack.h>
#include <klib/printf.h>

#include <kfs/directory.h>

#include <kdb/meta.h>
#include <kdb/namelist.h>

#include <vdb/manager.h>
#include <vdb/schema.h>
#include <vdb/database.h>
#include <vdb/table.h>
#include <vdb/cursor.h>

#include <sra/sraschema.h>
#include <sra/sradb.h>
#include <sra/pacbio.h>

#include <vfs/manager.h>
#include <vfs/resolver.h>
#include <vfs/path.h>

#include <sysalloc.h>

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <os-native.h>
#include <string.h>
#include <bitstr.h>

#define NTABS 8
#define NCOLUMNS 8
#define NMETACHILDS 4
#define NTYPES 8
#define DEF_ELEM_SEPARATOR ","
#define DEF_DIM_SEPARATOR "[]"
#define INVALID_COL 0xFFFFFFFF
#define INVALID_ROW 0xFFFFFFFFFFFFFFFF


bool print_err = false;
char last_err[ 1024 ];

/* forward decl's */
struct svdb_conn;
typedef struct svdb_conn* p_svdb_conn;

struct svdb_tab;
typedef struct svdb_tab* p_svdb_tab;

struct svdb_col;
typedef struct svdb_col* p_svdb_col;

struct svdb_type;
typedef struct svdb_type* p_svdb_type;

struct svdb_meta;
typedef struct svdb_meta* p_svdb_meta;


typedef const char * (*value_trans_func_t)( const uint32_t id );
typedef char * (*dim_trans_func_t)( const uint8_t *src );


typedef struct print_ctx
{
    char * buf;
    int buf_len;
    int needed;
    int printed;
} print_ctx;
typedef print_ctx* p_print_ctx;


typedef struct data_src
{
    uint32_t elem_bits;
    uint32_t boff;
    uint32_t row_len;
    uint32_t elem_idx;
    const void * base;
} data_src;
typedef data_src* p_data_src;


typedef struct svdb_type
{
    const char * name;
    VTypedecl typedecl;
    VTypedesc typedesc;
    p_svdb_col col;
} svdb_type;


typedef struct svdb_col
{
    const char * name;
    const char * cast;
    const char * elem_separator;
    const char * dim_separator;
    const char * cur_typedecl;
    p_svdb_tab tab;
    uint32_t cur_idx;
    int enabled;
    int open;
    int visible;
    Vector types;
    uint32_t dflt_type_idx;
    VTypedecl cursor_typedecl;
    VTypedesc cursor_typedesc;
    value_trans_func_t value_trans;
    dim_trans_func_t dim_trans;
} svdb_col;


typedef struct svdb_meta
{
    Vector childs;
    int id;
    const char * name;
    const void * value;
    int value_len;
} svdb_meta;


typedef struct svdb_tab
{
    const VTable * tab;
    const VCursor * cursor;
    const char * name;
    p_svdb_meta meta;
    uint64_t range;
    int64_t first;
    int meta_id;
    p_svdb_conn conn;
    Vector all_columns;
    Vector static_columns;
    Vector non_static_columns;
    Vector visible_columns;
    Vector non_visible_columns;
} svdb_tab;


typedef struct svdb_conn
{
    char * path;
    KDirectory * dir;
    const VDBManager * mgr;
    VSchema * schema;
    const VDatabase * db;
    int is_db;
    Vector tables;
} svdb_conn;


/* special translations of SRA-types into clear text */
const char SRA_PB_HS_0[] = { "SRA_PACBIO_HOLE_SEQUENCING" };
const char SRA_PB_HS_1[] = { "SRA_PACBIO_HOLE_ANTIHOLE" };
const char SRA_PB_HS_2[] = { "SRA_PACBIO_HOLE_FIDUCIAL" };
const char SRA_PB_HS_3[] = { "SRA_PACBIO_HOLE_SUSPECT" };
const char SRA_PB_HS_4[] = { "SRA_PACBIO_HOLE_ANTIMIRROR" };
const char SRA_PB_HS_5[] = { "SRA_PACBIO_HOLE_FDZMW" };
const char SRA_PB_HS_6[] = { "SRA_PACBIO_HOLE_FBZMW" };
const char SRA_PB_HS_7[] = { "SRA_PACBIO_HOLE_ANTIBEAMLET" };
const char SRA_PB_HS_8[] = { "SRA_PACBIO_HOLE_OUTSIDEFOV" };
const char SRA_PB_HS_9[] = { "unknown hole-status" };

static const char *sra_trans_hole_status( const uint32_t id )
{
    switch( id )
    {
        case SRA_PACBIO_HOLE_SEQUENCING     : return( SRA_PB_HS_0 ); break;
        case SRA_PACBIO_HOLE_ANTIHOLE       : return( SRA_PB_HS_1 ); break;
        case SRA_PACBIO_HOLE_FIDUCIAL       : return( SRA_PB_HS_2 ); break;
        case SRA_PACBIO_HOLE_SUSPECT        : return( SRA_PB_HS_3 ); break;
        case SRA_PACBIO_HOLE_ANTIMIRROR     : return( SRA_PB_HS_4 ); break;
        case SRA_PACBIO_HOLE_FDZMW          : return( SRA_PB_HS_5 ); break;
        case SRA_PACBIO_HOLE_FBZMW          : return( SRA_PB_HS_6 ); break;
        case SRA_PACBIO_HOLE_ANTIBEAMLET    : return( SRA_PB_HS_7 ); break;
        case SRA_PACBIO_HOLE_OUTSIDEFOV     : return( SRA_PB_HS_8 ); break;
    }
    return( SRA_PB_HS_9 );
}

const char SRA_PF_0[] = { "SRA_PLATFORM_UNDEFINED" };
const char SRA_PF_1[] = { "SRA_PLATFORM_454" };
const char SRA_PF_2[] = { "SRA_PLATFORM_ILLUMINA" };
const char SRA_PF_3[] = { "SRA_PLATFORM_ABSOLID" };
const char SRA_PF_4[] = { "SRA_PLATFORM_COMPLETE_GENOMICS" };
const char SRA_PF_5[] = { "SRA_PLATFORM_HELICOS" };
const char SRA_PF_6[] = { "SRA_PLATFORM_PACBIO_SMRT" };
const char SRA_PF_7[] = { "SRA_PLATFORM_ION_TORRENT" };
const char SRA_PF_8[] = { "SRA_PLATFORM_CAPILLARY" };
const char SRA_PF_9[] = { "SRA_PLATFORM_OXFORD_NANOPORE" };
const char SRA_PF_N[] = { "unknown platform" };

static const char *sra_trans_platform( const uint32_t id )
{
    switch( id )
    {
        case 0  : return( SRA_PF_0 ); break;
        case 1  : return( SRA_PF_1 ); break;
        case 2  : return( SRA_PF_2 ); break;
        case 3  : return( SRA_PF_3 ); break;
        case 4  : return( SRA_PF_4 ); break;
        case 5  : return( SRA_PF_5 ); break;
        case 6  : return( SRA_PF_6 ); break;
        case 7  : return( SRA_PF_7 ); break;
        case 8  : return( SRA_PF_8 ); break;
        case 9  : return( SRA_PF_9 ); break;		
    }
    return( SRA_PF_N );
}


const char SRA_RT_0[] = { "SRA_READ_TYPE_TECHNICAL" };
const char SRA_RT_1[] = { "SRA_READ_TYPE_BIOLOGICAL" };
const char SRA_RT_2[] = { "SRA_READ_TYPE_TECHNICAL|SRA_READ_TYPE_FORWARD" };
const char SRA_RT_3[] = { "SRA_READ_TYPE_BIOLOGICAL|SRA_READ_TYPE_FORWARD" };
const char SRA_RT_4[] = { "SRA_READ_TYPE_TECHNICAL|SRA_READ_TYPE_REVERSE" };
const char SRA_RT_5[] = { "SRA_READ_TYPE_BIOLOGICAL|SRA_READ_TYPE_REVERSE" };
const char SRA_RT_6[] = { "unknown read-type" };

static const char *sra_trans_read_type( const uint32_t id )
{
    switch( id )
    {
        case 0 : return( SRA_RT_0 ); break;
        case 1 : return( SRA_RT_1 ); break;
        case 2 : return( SRA_RT_2 ); break;
        case 3 : return( SRA_RT_3 ); break;
        case 4 : return( SRA_RT_4 ); break;
        case 5 : return( SRA_RT_5 ); break;
    }
    return( SRA_RT_6 );
}


const char SRA_FT_0[] = { "SRA_READ_FILTER_PASS" };
const char SRA_FT_1[] = { "SRA_READ_FILTER_REJECT" };
const char SRA_FT_2[] = { "SRA_READ_FILTER_CRITERIA" };
const char SRA_FT_3[] = { "SRA_READ_FILTER_REDACTED" };
const char SRA_FT_4[] = { "unknown read-filter" };

static const char *sra_trans_read_filter( const uint32_t id )
{
    switch( id )
    {
        case 0 : return( SRA_FT_0 ); break;
        case 1 : return( SRA_FT_1 ); break;
        case 2 : return( SRA_FT_2 ); break;
        case 3 : return( SRA_FT_3 ); break;
    }
    return( SRA_FT_4 );
}


/* hardcoded values taken from asm-trace/interface/sra/sradb.h */
#define SRA_KEY_PLATFORM_ID "INSDC:SRA:platform_id"
#define SRA_KEY_XREAD_TYPE "INSDC:SRA:xread_type"
#define SRA_KEY_READ_TYPE "INSDC:SRA:read_type"
#define SRA_KEY_READ_FILTER "INSDC:SRA:read_filter"
#define SRA_PACBIO_HOLE_STATUS "PacBio:hole:status"

static bool vdcd_type_cmp( const VSchema *my_schema, VTypedecl * typedecl, const char * to_check )
{
    VTypedecl type_to_check;
    rc_t rc = VSchemaResolveTypedecl ( my_schema, &type_to_check, "%s", to_check );
    if ( rc == 0 )
    {
        return VTypedeclToTypedecl ( typedecl, my_schema, &type_to_check, NULL, NULL );
    }
    return false;
}

static value_trans_func_t sra_get_value_trans_func( const VSchema *my_schema, VTypedecl * typedecl )
{
    value_trans_func_t res = NULL;

    if ( my_schema == NULL ) return res;
    if ( typedecl == NULL ) return res;

    if ( vdcd_type_cmp( my_schema, typedecl, SRA_KEY_PLATFORM_ID ) )
    {
        res = sra_trans_platform;
    }
    else if ( vdcd_type_cmp( my_schema, typedecl, SRA_KEY_XREAD_TYPE ) )
    {
        res = sra_trans_read_type;
    }
    else if ( vdcd_type_cmp( my_schema, typedecl, SRA_KEY_READ_TYPE ) )
    {
        res = sra_trans_read_type;
    }
    else if ( vdcd_type_cmp( my_schema, typedecl, SRA_KEY_READ_FILTER ) )
    {
        res = sra_trans_read_filter;
    }
    else if ( vdcd_type_cmp( my_schema, typedecl, SRA_PACBIO_HOLE_STATUS ) )
    {
        res = sra_trans_hole_status;
    }
    return res;
}


/* implementation of the dimension-translation-functions */
static char * sra_read_desc( const uint8_t * src )
{
    char *res = calloc( 1, 120 );
    SRAReadDesc desc;
    memmove( &desc, src, sizeof( desc ) );
    string_printf ( res, 119, NULL,
              "seg.start=%u, seg.len=%u, type=%u, cs_key=%u, label=%s",
              desc.seg.start, desc.seg.len, desc.type,
              desc.cs_key, desc.label );
    return res;
}


static char * sra_spot_desc( const uint8_t *src )
{
    char *res = calloc( 1, 120 );
    SRASpotDesc desc;
    memmove( &desc, src, sizeof( desc ) );
    string_printf ( res, 119, NULL,
              "spot_len=%u, fixed_len=%u, signal_len=%u, clip_qual_right=%u, num_reads=%u",
              desc.spot_len, desc.fixed_len, desc.signal_len,
              desc.clip_qual_right, desc.num_reads );
    return res;
}

/* hardcoded values taken from asm-trace/interface/sra/sradb.h */
#define SRA_KEY_READ_DESC "NCBI:SRA:ReadDesc"
#define SRA_KEY_SPOT_DESC "NCBI:SRA:SpotDesc"

static dim_trans_func_t sra_get_dim_trans_func( const VSchema *my_schema, VTypedecl * typedecl )
{
    dim_trans_func_t res = NULL;

    if ( my_schema == NULL ) return res;
    if ( typedecl == NULL ) return res;

    if ( vdcd_type_cmp( my_schema, typedecl, SRA_KEY_READ_DESC ) )
    {
        res = sra_read_desc;
    }
    else if ( vdcd_type_cmp( my_schema, typedecl, SRA_KEY_SPOT_DESC ) )
    {
        res = sra_spot_desc;
    }
    return res;
}


static void svdb_init_print_ctx( p_print_ctx ctx, char * buf, int buf_len )
{
    ctx->buf = buf;
    ctx->buf_len = buf_len;
    ctx->needed = 0;
    ctx->printed = 0;
}


KLIB_EXTERN int CC string_cmp ( const char *a, size_t asize,
    const char *b, size_t bsize, uint32_t max_chars );

static int svdb_strcmp( const char *a, const char *b )
{
    size_t sa = string_size ( a );
    size_t sb = string_size ( b );
    uint32_t max_chars = ( sa > sb ) ? (uint32_t)sa : (uint32_t)sb;
    return string_cmp ( a, sa, b, sb, max_chars );
}


static void svdb_set_last_err( const char * s )
{
    string_copy_measure ( last_err, sizeof last_err, s );
}


const char ACC_NOT_FOUND[] = { "accession not found" };
char accession_2_path_buffer[ 1024 ];


static void log_and_err( rc_t rc, const char * s )
{
    if ( print_err ) LOGERR( klogInt, rc, s );
    svdb_set_last_err( s );
}


MOD_EXPORT const char * CC svdb_accession_2_path( const char * accession )
{
    const char * res = NULL;
    if ( accession != NULL && accession[0] != 0 )
    {
        if ( strchr ( accession, '/' ) == NULL )
        {
            VFSManager * vfs_mgr;
            rc_t rc = VFSManagerMake ( &vfs_mgr );
            if ( rc != 0 )
                log_and_err( rc, "VFSManagerMake() failed in svdb_accession_2_path()" );
            else
            {
                VResolver * resolver;

                rc = VFSManagerGetResolver ( vfs_mgr, &resolver );
                if ( rc != 0 )
                    log_and_err( rc, "VFSManagerGetResolver() failed in svdb_accession_2_path()" );
                else
                {
                    VPath * vpath;
                    rc = VFSManagerMakeSysPath ( vfs_mgr, &vpath, accession );
                    if ( rc != 0 )
                        log_and_err( rc, "VFSManagerMakeSysPath() failed in svdb_accession_2_path()" );
                    else
                    {
                        const VPath * local;
                        rc = VResolverQuery ( resolver, 0, vpath, &local, NULL, NULL );
                        if ( rc != 0 )
                            log_and_err( rc, "VResolverQuery() failed in svdb_accession_2_path()" );
                        else
                        {
                            const String * str;
                            rc = VPathMakeString ( local, &str );
                            if ( rc != 0 )
                                log_and_err( rc, "VPathMakeString() failed in svdb_accession_2_path()" );
                            else
                            {
                                res = string_dup ( str->addr, str->size );
                                StringWhack ( str );
                            }

                            VPathRelease ( local );
                        }
                        VPathRelease ( vpath );
                    }
                    VResolverRelease ( resolver );
                }
                VFSManagerRelease ( vfs_mgr );
            }

        }
    }
    return res;
}


static p_svdb_type svdb_make_type( p_svdb_col col, const char * name )
{
    rc_t rc;
    p_svdb_type res = calloc( 1, sizeof( svdb_type ) );
    if ( res == NULL )
    {
        rc = RC( rcApp, rcNoTarg, rcConstructing, rcMemory, rcExhausted );
        LOGERR( klogInt, rc, "calloc() failed in svdb_make_type()" );
    }
    else
    {
        res->col = col;
        res->name = string_dup_measure ( name, NULL );
        rc = VSchemaResolveTypedecl ( col->tab->conn->schema, &res->typedecl, "%s", name );
        if ( rc != 0 )
        {
            LOGERR( klogInt, rc, "VSchemaResolveTypedecl() failed in svdb_make_type()" );
        }
        else
        {
            rc = VSchemaDescribeTypedecl ( col->tab->conn->schema, &res->typedesc, &res->typedecl );
            if ( rc != 0 )
            {
                LOGERR( klogInt, rc, "VSchemaDescribeTypedecl() failed in svdb_make_type()" );
            }
        }
    }
    return res;
}


static void CC svdb_free_type( void * item, void * data )
{
    p_svdb_type t = (p_svdb_type)item;
    if ( t != NULL )
    {
        if ( t->name != NULL )
        {
            free( ( void * )t->name );
        }
        free( item );
    }
}


static rc_t svdb_discover_types( p_svdb_col col )
{
    KNamelist *type_names;
    uint32_t dflt_idx;
    rc_t rc = VTableColumnDatatypes ( col->tab->tab, col->name, &dflt_idx, &type_names );
    if ( rc == 0 )
    {
        uint32_t n;
        col->dflt_type_idx = dflt_idx;
        rc = KNamelistCount( type_names, &n );
        if ( rc == 0 )
        {
            uint32_t i;
            for ( i = 0; i < n && rc == 0; ++i )
            {
                const char *type_name;
                rc = KNamelistGet( type_names, i, &type_name );
                if ( rc == 0 )
                {
                    p_svdb_type type = svdb_make_type( col, type_name );
                    if ( type == NULL )
                    {
                        rc = RC( rcApp, rcNoTarg, rcConstructing, rcMemory, rcExhausted );
                    }
                    else
                    {
                        rc = VectorAppend ( &col->types, NULL, type );
                        if ( rc != 0 )
                        {
                            LOGERR( klogInt, rc, "VectorAppend() failed in svdb_discover_types()" );
                            svdb_free_type( type, NULL );
                        }
                    }
                }
            }
        }
        KNamelistRelease( type_names );
    }
    return rc;
}


static p_svdb_col svdb_make_col( p_svdb_tab tab, const char * name )
{
    rc_t rc;
    p_svdb_col res = calloc( 1, sizeof( svdb_col ) );
    if ( res == NULL )
    {
        rc = RC( rcApp, rcNoTarg, rcConstructing, rcMemory, rcExhausted );
        LOGERR( klogInt, rc, "calloc() failed in svdb_make_col()" );
    }
    else
    {
        
        VectorInit ( &res->types, 0, NTYPES );
        res->tab = tab;
        res->name = string_dup_measure ( name, NULL );
        res->elem_separator = string_dup_measure ( DEF_ELEM_SEPARATOR, NULL );
        res->dim_separator = string_dup_measure ( DEF_DIM_SEPARATOR, NULL );
        res->visible = 1;
        rc = svdb_discover_types( res );
        if ( rc != 0 )
        {
            VectorWhack ( &res->types, svdb_free_type, NULL );
            free( ( void* )res->name );
            res = NULL;
        }
    }
    return res;
}


static void free_if_not_null( const char * s )
{
    if ( s != NULL )
    {
        free( ( void * ) s );
    }
}


static void CC svdb_free_col( void * item, void * data )
{
    p_svdb_col c = (p_svdb_col)item;
    if ( c != NULL )
    {
        VectorWhack ( &c->types, svdb_free_type, NULL );
        free_if_not_null( c->elem_separator );
        free_if_not_null( c->dim_separator );
        free_if_not_null( c->cur_typedecl );
        free_if_not_null( c->cast );
        free_if_not_null( c->name );
        free( item );
    }
}


static rc_t svdb_discover_columns( p_svdb_tab table )
{
    KNamelist *col_names;
    rc_t rc = VTableListReadableColumns ( table->tab, &col_names );
    if ( rc == 0 )
    {
        uint32_t n;
        rc = KNamelistCount( col_names, &n );
        if ( rc == 0 )
        {
            uint32_t i;
            for ( i = 0; i < n && rc == 0; ++i )
            {
                const char *col_name;
                rc = KNamelistGet( col_names, i, &col_name );
                if ( rc == 0 )
                {
                    p_svdb_col col = svdb_make_col( table, col_name );
                    if ( col == NULL )
                    {
                        rc = RC( rcApp, rcNoTarg, rcConstructing, rcMemory, rcExhausted );
                    }
                    else
                    {
                        rc = VectorAppend ( &table->all_columns, NULL, col );
                        if ( rc != 0 )
                        {
                            LOGERR( klogInt, rc, "VectorAppend() failed in svdb_discover_columns()" );
                            svdb_free_col( col, NULL );
                        }
                        else
                        {
                            VectorAppend ( &table->visible_columns, NULL, col );
                        }
                    }
                }
            }
        }
        KNamelistRelease( col_names );
    }
    return rc;
}


static void svdb_whack_meta( p_svdb_meta meta )
{
    if ( meta != NULL )
    {
        int i, n = VectorLength( &meta->childs );
        for ( i = 0; i < n; ++i )
        {
            p_svdb_meta child = VectorGet ( &meta->childs, i );
            if ( child != NULL )
            {
                svdb_whack_meta( child ); /* recursion here !!! */
            }
        }
        VectorWhack( &meta->childs, NULL, NULL );
        free_if_not_null( meta->name );
        free_if_not_null( meta->value );
        free( meta );
    }
}


static void svdb_read_meta_value( p_svdb_meta m, const KMDataNode *node )
{
    char buffer[ 8 ];
    size_t num_read, remaining;

    /* first try to detect how big the node-value is */
    rc_t rc = KMDataNodeRead ( node, 0, buffer, sizeof buffer, &num_read, &remaining );
    if ( rc == 0 )
    {
        size_t count = num_read + remaining;
        if ( count > 0 )
        {
            m->value = malloc( count );
            if ( m->value != NULL )
            {
                /* 2nd try to read it really now */
                rc = KMDataNodeRead ( node, 0, (void *)m->value, count, &num_read, &remaining );
                if ( rc == 0 )
                {
                    m->value_len = (uint32_t)count;
                }
                else
                {
                    free( (void *)m->value );
                    m->value = NULL;
                }
            }
        }
    }
}


static p_svdb_meta svdb_init_meta( p_svdb_tab tab, const KMDataNode *node, const char * name )
{
    p_svdb_meta res = calloc( 1, sizeof( svdb_meta )  );
    if ( res != NULL )
    {
        VectorInit ( &res->childs, 0, NMETACHILDS );
        res->id = ( tab->meta_id )++;
        res->name = string_dup_measure ( name, NULL );
        svdb_read_meta_value( res, node );

        if ( node != NULL )
        {
            KNamelist *names;
            rc_t rc = KMDataNodeListChild ( node, &names );
            if ( rc == 0 )
            {
                uint32_t i, count;
                rc = KNamelistCount ( names, & count );
                for ( i = 0; rc == 0 && i < count; ++ i )
                {
                    const char *node_path;
                    rc = KNamelistGet ( names, i, & node_path );
                    if ( rc == 0 )
                    {
                        const KMDataNode *child_node;
                        rc = KMDataNodeOpenNodeRead ( node, &child_node, "%s", node_path );
                        if ( rc == 0 )
                        {
                            p_svdb_meta child = svdb_init_meta( tab, child_node, node_path );
                            if ( child != NULL )
                            {
                                rc = VectorAppend ( &res->childs, NULL, child );
                                if ( rc != 0 )
                                {
                                    svdb_whack_meta( child );
                                }
                            }
                            KMDataNodeRelease ( child_node );
                        }
                    }
                }
                KNamelistRelease( names );
            }
        }
    }
    return res;
}


static p_svdb_meta svdb_find_meta( p_svdb_meta meta, const int id )
{
    p_svdb_meta res = NULL;
    if ( meta->id == id )
    {
        res = meta;
    }
    else
    {
        int i, n = VectorLength( &meta->childs );
        for ( i = 0; i < n && res == NULL; ++i )
        {
            p_svdb_meta child = VectorGet ( &meta->childs, i );
            if ( child != NULL )
            {
                res = svdb_find_meta( child, id );
            }
        }
    }
    return res;
}


static rc_t svdb_discover_meta( p_svdb_tab tab )
{
    const KMetadata *src_meta;
    rc_t rc = VTableOpenMetadataRead ( tab->tab, &src_meta );
    if ( rc == 0 )
    {
        const KMDataNode *root;
        rc = KMetadataOpenNodeRead ( src_meta, &root, NULL );
        if ( rc == 0 )
        {
            tab->meta = svdb_init_meta( tab, root, "/" );
            KMDataNodeRelease ( root );
        }
        KMetadataRelease ( src_meta );
    }
    return rc;
}


static p_svdb_tab svdb_make_tab( p_svdb_conn pself, const VTable * tab,
            const char * name )
{
    p_svdb_tab res = calloc( 1, sizeof( svdb_tab ) );
    if ( res == NULL )
    {
        rc_t rc = RC( rcApp, rcNoTarg, rcConstructing, rcMemory, rcExhausted );
        LOGERR( klogInt, rc, "calloc() failed in svdb_make_tab()" );
    }
    else
    {
        rc_t rc;

        res->conn = pself;
        res->tab = tab;
        VectorInit ( &res->all_columns, 0, NCOLUMNS );
        VectorInit ( &res->static_columns, 0, NCOLUMNS );
        VectorInit ( &res->non_static_columns, 0, NCOLUMNS );
        VectorInit ( &res->visible_columns, 0, NCOLUMNS );
        VectorInit ( &res->non_visible_columns, 0, NCOLUMNS );

        rc = svdb_discover_columns( res );
        if ( rc == 0 )
        {
            rc = svdb_discover_meta( res );
            if ( rc == 0 )
            {
                res->name = string_dup_measure ( name, NULL );
            }
        }

        if ( rc != 0 )
        {
            VectorWhack ( &res->static_columns, NULL, NULL );
            VectorWhack ( &res->non_static_columns, NULL, NULL );
            VectorWhack ( &res->visible_columns, NULL, NULL );
            VectorWhack ( &res->non_visible_columns, NULL, NULL );
            VectorWhack ( &res->all_columns, svdb_free_col, NULL );
            svdb_whack_meta( res->meta );
            free( res );
            res = NULL;
        }
    }
    return res;
}


static void CC svdb_free_table( void * item, void * data )
{
    p_svdb_tab t = (p_svdb_tab)item;
    if ( t != NULL )
    {
        rc_t rc;
        if ( t->cursor != NULL )
        {
            rc = VCursorRelease ( t->cursor );
            if ( rc != 0 )
            {
                LOGERR( klogInt, rc, "VCursorRelease() failed in svdb_close_table()" );
            }
        }
        if ( t->tab != NULL ) 
        {
            rc = VTableRelease( t->tab );
            if ( rc != 0 )
            {
                LOGERR( klogInt, rc, "VTableRelease() failed in svdb_close_table()" );
            }
        }
        VectorWhack ( &t->static_columns, NULL, NULL );
        VectorWhack ( &t->non_static_columns, NULL, NULL );
        VectorWhack ( &t->visible_columns, NULL, NULL );
        VectorWhack ( &t->non_visible_columns, NULL, NULL );
        VectorWhack ( &t->all_columns, svdb_free_col, NULL );
        svdb_whack_meta( t->meta );

        free_if_not_null( t->name );
        free( item );
    }
}


static void svdb_clear_vector( Vector * v )
{
    while ( VectorLength( v ) > 0 )
    {
        void * removed;
        VectorRemove ( v, 0, &removed );
    }
}


static Vector * svdb_get_column_vector( p_svdb_tab table, const int selection )
{
    Vector * res = &table->all_columns;
    switch( selection )
    {
        case STATIC_COLUMNS      : res = &table->static_columns; break;
        case NON_STATIC_COLUMNS  : res = &table->non_static_columns; break;
        case VISIBLE_COLUMNS     : res = &table->visible_columns; break;
        case NON_VISIBLE_COLUMNS : res = &table->non_visible_columns; break;
    }
    return res;
}


MOD_EXPORT void CC svdb_close( void * self )
{
    p_svdb_conn pself = (p_svdb_conn)self;
    if ( pself != NULL )
    {
        rc_t rc;
        VectorWhack ( &pself->tables, svdb_free_table, NULL );
        rc = VDatabaseRelease( pself->db );
        if ( rc != 0 )
        {
            LOGERR( klogInt, rc, "VDatabaseRelease() failed in svdb_close()" );
        }
        rc = VSchemaRelease( pself->schema );
        if ( rc != 0 )
        {
            LOGERR( klogInt, rc, "VSchemaRelease() failed in svdb_close()" );
        }
        rc = VDBManagerRelease( pself->mgr );
        if ( rc != 0 )
        {
            LOGERR( klogInt, rc, "VDBManagerRelease() failed in svdb_close()" );
        }
        rc = KDirectoryRelease( pself->dir );
        if ( rc != 0 )
        {
            LOGERR( klogInt, rc, "KDirectoryRelease() failed in svdb_close()" );
        }
        free_if_not_null( pself->path );
        free( self );
    }
}


static rc_t svdb_append_tab( p_svdb_conn pself, const VTable * tab,
            const char * name )
{
    rc_t rc = 0;
    p_svdb_tab t = svdb_make_tab( pself, tab, name );
    if ( t == NULL )
    {
        rc = VTableRelease( tab );
        if ( rc != 0 )
        {
            LOGERR( klogInt, rc, "VTableRelease() failed in svdb_open()" );
        }
        rc = RC( rcApp, rcNoTarg, rcConstructing, rcMemory, rcExhausted );
    }
    else
    {
        rc = VectorAppend ( &pself->tables, NULL, t );
        if ( rc != 0 )
        {
            LOGERR( klogInt, rc, "VectorAppend() failed in svdb_open()" );
            svdb_free_table( t, NULL );
        }
    }
    return rc;
}


static const char * svdb_filename( const char * path )
{
    const char * res = NULL;
    if ( path != NULL && path[0] != 0 )
    {
        res = strrchr( path, '/' );
        if ( res == NULL )
        {
            res = strrchr( path, '\\' );
        }
        if ( res == NULL )
        {
            res = path;
        }
        else
        {
            res++;
        }
    }
    return res;
}


static rc_t svdb_open_subtables( p_svdb_conn pself )
{
    KNamelist *tbl_names;
    rc_t rc = VDatabaseListTbl( pself->db, &tbl_names );
    if ( rc == 0 )
    {
        uint32_t n;
        rc = KNamelistCount( tbl_names, &n );
        if ( rc == 0 )
        {
            uint32_t i;
            for ( i = 0; i < n && rc == 0; ++i )
            {
                const char *tbl_name;
                rc = KNamelistGet( tbl_names, i, &tbl_name );
                if ( rc == 0 )
                {
                    const VTable * tab;
                    rc = VDatabaseOpenTableRead( pself->db, &tab, "%s", tbl_name );
                    if ( rc == 0 )
                    {
                        rc = svdb_append_tab( pself, tab, tbl_name );
                    }
                }
            }
        }
        KNamelistRelease( tbl_names );
    }
    return rc;
}


MOD_EXPORT void * CC svdb_open_path( const char * path  )
{
    p_svdb_conn pself = NULL;
   if ( path != NULL && path[0] != 0 )
    {
        rc_t rc;
        pself = calloc( 1, sizeof( svdb_conn ) );
        if ( pself == NULL )
        {
            rc = RC( rcApp, rcNoTarg, rcConstructing, rcMemory, rcExhausted );
            LOGERR( klogInt, rc, "calloc() failed in svdb_open()" );
            svdb_set_last_err( "calloc() failed in svdb_open()" );
        }
        else
        {
            VectorInit ( &pself->tables, 0, NTABS );
            rc = KDirectoryNativeDir( &pself->dir );
            if ( rc != 0 )
            {
                LOGERR( klogInt, rc, "KDirectoryNativeDir() failed in svdb_open()" );
                svdb_set_last_err( "KDirectoryNativeDir() failed in svdb_open()" );
            }
            else
            {
                rc = VDBManagerMakeRead ( &pself->mgr, pself->dir );
                if ( rc != 0 )
                {
                    LOGERR( klogInt, rc, "VDBManagerMakeRead() failed in svdb_open()" );
                    svdb_set_last_err( "VDBManagerMakeRead() failed in svdb_open()" );
                }
            }
            if ( rc == 0 )
            {
                rc = VDBManagerMakeSRASchema( pself->mgr, &pself->schema );
                if ( rc != 0 )
                {
                    LOGERR( klogInt, rc, "VDBManagerMakeSRASchema() failed in svdb_open()" );
                    svdb_set_last_err( "VDBManagerMakeSRASchema() failed in svdb_open()" );
                }
                else
                {
                    /* it is OK if these fail, we have no valid path then... */
                    rc = VDBManagerOpenDBRead( pself->mgr, &pself->db, pself->schema, "%s", path );
                    if ( rc == 0 )
                    {
                        pself->is_db = 1;
                        rc = svdb_open_subtables( pself );
                    }
                    else
                    {
                        const VTable * tab;
                        rc = VDBManagerOpenTableRead( pself->mgr, &tab, pself->schema, "%s", path );
                        if ( rc == 0 )
                        {
                            rc = svdb_append_tab( pself, tab, svdb_filename( path ) );
                            if ( rc != 0 )
                            {
                                svdb_set_last_err( "svdb_append_tab() failed in svdb_open()" );
                            }
                        }
                        else
                        {
                            string_printf ( last_err, sizeof last_err, NULL,
                                            "VDBManagerOpenTableRead('%s')->'%R' failed",
                                            path, rc );
/*                            svdb_set_last_err( "VDBManagerOpenTableRead() failed in svdb_open()" ); */
                        }
                    }
                }
            }

            if ( rc == 0 )
            {
                pself->path = string_dup_measure ( path, NULL );
                svdb_set_last_err( "OK" );
            }
            else
            {
                svdb_close( pself );
                pself = NULL;
            }
        }
    }
    else
    {
        svdb_set_last_err( "path empty in svdb_open()" );
    }
    return pself;
}


MOD_EXPORT const char * CC svdb_last_err( void )
{
    return last_err;
}


MOD_EXPORT int CC svdb_is_db( void * self )
{
    int res = 0;
    if ( self != NULL )
    {
        p_svdb_conn pself = self;
        res = pself->is_db;
    }
    return res;
}


MOD_EXPORT int CC svdb_count_tabs( void * self )
{
    int res = 0;
    if ( self != NULL )
    {
        p_svdb_conn pself = self;
        res = VectorLength( &pself->tables );
    }
    return res;
}


static p_svdb_tab svdb_get_tab( void * self, const int tab_id )
{
    p_svdb_tab res = NULL;
    if ( self != NULL )
    {
        p_svdb_conn pself = self;
        res = VectorGet ( &pself->tables, tab_id );
    }
    return res;
}


MOD_EXPORT int CC svdb_tab_meta_root( void * self, const int tab_id )
{
    int res = -1;
    p_svdb_tab t = svdb_get_tab( self, tab_id );
    if ( t != NULL )
    {
        if ( t->meta != NULL )
        {
            res = t->meta->id;
        }
    }
    return res;
}


static p_svdb_meta svdb_get_meta_node( void * self, const int tab_id, const int meta_id )
{
    p_svdb_meta m = NULL;
    p_svdb_tab t = svdb_get_tab( self, tab_id );
    if ( t != NULL )
    {
        if ( t->meta != NULL )
        {
            m = svdb_find_meta( t->meta, meta_id );
        }
    }
    return m;
}


static int svdb_check_printable( const void * ptr, const int len )
{
    int res = 0;
    if ( ptr != NULL && len > 0 )
    {
        int i, j = 0;
        const char * cp = ptr;
        for ( i = 0; i < len; ++i )
        {
            if ( !isprint ( cp[ i ] ) )
                j++;
        }
        if ( j == 0 )
            res = 1;
        else
            res = 0;
    }
    return res;
}


MOD_EXPORT int CC svdb_tab_meta_value_printable( void * self, const int tab_id, const int meta_id )
{
    int res = -1;
    p_svdb_meta m = svdb_get_meta_node( self, tab_id, meta_id );
    if ( m != NULL )
    {
        res = svdb_check_printable( m->value, m->value_len );
    }
    return res;

}


MOD_EXPORT int CC svdb_tab_meta_value_len( void * self, const int tab_id, const int meta_id )
{
    int res = -1;
    p_svdb_meta m = svdb_get_meta_node( self, tab_id, meta_id );
    if ( m != NULL && m->value != NULL && m->value_len )
    {
        res = m->value_len;
    }
    return res;
}


MOD_EXPORT const char * CC svdb_tab_meta_value_ptr( void * self, const int tab_id,
                                              const int meta_id )
{
    const char * res = NULL;
    p_svdb_meta m = svdb_get_meta_node( self, tab_id, meta_id );
    if ( m != NULL && m->value != NULL && m->value_len )
    {
        res = ( char * )m->value;
    }
    return res;
}


static rc_t svdb_print_hex( char * dst, size_t dstlen, char * src, size_t srclen,
                            size_t trim, size_t * num_writ )
{
    rc_t rc = 0;
    bool periods = true;
    size_t writ, i, n = ( dstlen / 3 );

    *num_writ = 0;
    if ( n >= srclen )
    {
        n = srclen;
        periods = false;
    }
    if ( periods )
        n--;
    for ( i = 0; i < n && rc == 0; ++i )
    {
        unsigned char x = src[i];
        if ( i > 0 )
            rc = string_printf ( dst, dstlen, &writ, "-%02X", x );
        else
            rc = string_printf ( dst, dstlen, &writ, "%02X", x );
        if ( rc == 0 )
        {
            dst += writ;
            *num_writ += writ;
            dstlen -= writ;
        }
    }
    if ( rc == 0 && periods )
    {
        rc = string_printf ( dst, dstlen, &writ, "..." );
        *num_writ += writ;
    }
    return rc;
}


MOD_EXPORT int CC svdb_tab_meta_value( void * self, const int tab_id, const int meta_id,
                                       char * buf, int buflen, int trim )
{
    int res = -1;
    p_svdb_meta m = svdb_get_meta_node( self, tab_id, meta_id );
    if ( m != NULL && m->value != NULL && m->value_len )
    {
        rc_t rc;
        size_t num_writ;
        if ( svdb_check_printable( m->value, m->value_len ) )
        {
            if ( trim == 0 || trim >= m->value_len )
                rc = string_printf ( buf, buflen, &num_writ, "%.*s", m->value_len, m->value );
            else
                rc = string_printf ( buf, buflen, &num_writ, "%.*s...", trim, m->value );
        }
        else
        {
            rc = svdb_print_hex( buf, buflen, (char *)m->value, m->value_len, trim, &num_writ );
        }

        if ( rc == 0 )
            res = ( int )num_writ;
    }
    return res;
}


MOD_EXPORT int CC svdb_tab_meta_name( void * self, const int tab_id, const int meta_id,
                                      char * buf, int buflen )
{
    int res = -1;
    p_svdb_meta m = svdb_get_meta_node( self, tab_id, meta_id );
    if ( m != NULL )
    {
        size_t num_writ;
        rc_t rc = string_printf ( buf, buflen, &num_writ, "%s", m->name );
        if ( rc == 0 )
        {
            res = ( int )num_writ;
        }
    }
    return res;
}


MOD_EXPORT int CC svdb_tab_meta_child_count( void * self, const int tab_id,
        const int meta_id )
{
    int res = -1;
    p_svdb_meta m = svdb_get_meta_node( self, tab_id, meta_id );
    if ( m != NULL )
    {
        res = VectorLength( &m->childs );
    }
    return res;
}


MOD_EXPORT int CC svdb_tab_meta_child_id( void * self, const int tab_id,
        const int meta_id, const int child_idx )
{
    int res = -1;
    p_svdb_meta m = svdb_get_meta_node( self, tab_id, meta_id );
    if ( m != NULL )
    {
        p_svdb_meta c = VectorGet( &m->childs, child_idx );
        if ( c != NULL )
        {
            res = c->id;
        }
    }
    return res;
}


MOD_EXPORT int CC svdb_tab_idx( void * self, const char * name )
{
    int res = -1;
    if ( self != NULL )
    {
        p_svdb_conn pself = self;
        int i, n = VectorLength( &pself->tables );
        for ( i = 0; i < n && res < 0; ++i )
        {
            p_svdb_tab tab = VectorGet ( &pself->tables, i );
            if ( tab != NULL )
            {
                if ( svdb_strcmp( tab->name, name ) == 0 )
                {
                    res = i;
                }
            }
        }   
    }
    return res;
}


static p_svdb_col svdb_get_col( void * self, const int tab_id,
            const int selection, const int col_id )
{
    p_svdb_col res = NULL;
    p_svdb_tab tab = svdb_get_tab( self, tab_id );
    if ( tab != NULL )
    {
        Vector * v = svdb_get_column_vector( tab, selection );
        res = VectorGet ( v, col_id );
    }
    return res;
}


MOD_EXPORT int CC svdb_set_column_visibility( void * self, const int tab_id,
            const int selection, const int col_id, const int visible )
{
    int res = 0;
    p_svdb_col col = svdb_get_col ( self, tab_id, selection, col_id );
    if ( col != NULL )
    {
        int vis = visible;
        if ( vis != 0 )
        {
            vis = 1;
        }
        if ( col->visible != vis )
        {
            int i, n;
            p_svdb_tab tab = svdb_get_tab( self, tab_id );
            col->visible = vis;
            /* clear the visible and invisible vector */
            VectorWhack ( &tab->visible_columns, NULL, NULL );
            VectorWhack ( &tab->non_visible_columns, NULL, NULL );

            VectorInit ( &tab->visible_columns, 0, NCOLUMNS );
            VectorInit ( &tab->non_visible_columns, 0, NCOLUMNS );

            /* loop through all columns an redistibute them new
               ( to preserve the original order ) */
            n = VectorLength( &tab->all_columns );
            for ( i = 0; i < n; ++i )
            {
                col = VectorGet ( &tab->all_columns, i );
                if ( col != NULL )
                {
                    if ( col->visible != 0 )
                    {
                        VectorAppend( &tab->visible_columns, NULL, col );
                    }
                    else
                    {
                        VectorAppend( &tab->non_visible_columns, NULL, col );
                    }
                }
            }   
            res = 1;
        }
    }
    return res;
}


MOD_EXPORT int CC svdb_col_idx( void * self, const int tab_id,
            const int selection, const char * name )
{
    int res = -1;
    p_svdb_tab tab = svdb_get_tab( self, tab_id );
    if ( tab != NULL )
    {
        Vector * v = svdb_get_column_vector( tab, selection );
        int i, n = VectorLength( v );
        for ( i = 0; i < n && res < 0; ++i )
        {
            p_svdb_col col = VectorGet ( v, i );
            if ( col != NULL )
            {
                if ( svdb_strcmp( col->name, name ) == 0 )
                {
                    res = i;
                }
            }
        }   
    }
    return res;
}


MOD_EXPORT const char * CC svdb_tabname( void * self, const int tab_id )
{
    const char * res = NULL;
    p_svdb_tab tab = svdb_get_tab( self, tab_id );
    if ( tab != NULL )
    {
        res = tab->name;
    }
    return res;
}


MOD_EXPORT int CC svdb_count_cols( void * self, const int tab_id, const int selection )
{
    int res = 0;
    p_svdb_tab tab = svdb_get_tab( self, tab_id );
    if ( tab != NULL )
    {
        Vector * v = svdb_get_column_vector( tab, selection );
        res = VectorLength( v );
    }
    return res;
}


MOD_EXPORT const char * CC svdb_colname( void * self, const int tab_id,
            const int selection, const int col_id )
{
    const char * res = NULL;
    p_svdb_col col = svdb_get_col( self, tab_id, selection, col_id );
    if ( col != NULL )
    {
        res = col->name;
    }
    return res;
}


MOD_EXPORT int CC svdb_dflt_type_idx( void * self, const int tab_id,
            const int selection, const int col_id )
{
    int res = -1;
    p_svdb_col col = svdb_get_col( self, tab_id, selection, col_id );
    if ( col != NULL )
    {
        res = col->dflt_type_idx;
    }
    return res;
}


MOD_EXPORT int CC svdb_count_types( void * self, const int tab_id,
            const int selection, const int col_id )
{
    int res = 0;
    p_svdb_col col = svdb_get_col( self, tab_id, selection, col_id );
    if ( col != NULL )
    {
        res = VectorLength( &col->types );
    }
    return res;
}


static p_svdb_type svdb_get_type( void * self, const int tab_id,
            const int selection, const int col_id, const int type_id )
{
    p_svdb_type res = NULL;
    p_svdb_col col = svdb_get_col( self, tab_id, selection, col_id );
    if ( col != NULL )
    {
        res = VectorGet ( &col->types, type_id );
    }
    return res;
}


MOD_EXPORT const char * CC svdb_typename( void * self, const int tab_id,
            const int selection, const int col_id, const int type_id )
{
    const char * res = NULL;
    p_svdb_type type = svdb_get_type( self, tab_id, selection, col_id, type_id );
    if ( type != NULL )
    {
        res = type->name;
    }
    return res;
}


MOD_EXPORT int CC svdb_type_idx( void * self, const int tab_id,
            const int selection, const int col_id, const char * name )
{
    int res = -1;
    p_svdb_col col = svdb_get_col( self, tab_id, selection, col_id );
    if ( col != NULL )
    {
        int i, n = VectorLength( &col->types );
        for ( i = 0; i < n && res < 0; ++i )
        {
            p_svdb_type type = VectorGet ( &col->types, i );
            if ( type != NULL )
            {
                if ( svdb_strcmp( type->name, name ) == 0 )
                {
                    res = i;
                }
            }
        }   
    }
    return res;
}


MOD_EXPORT int CC svdb_typedomain( void * self, const int tab_id,
            const int selection, const int col_id, const int type_id )
{
    int res = -1;
    p_svdb_type type = svdb_get_type( self, tab_id, selection, col_id, type_id );
    if ( type != NULL )
    {
        res = type->typedesc.domain;
    }
    return res;
}


MOD_EXPORT int CC svdb_typebits( void * self, const int tab_id,
            const int selection, const int col_id, const int type_id )
{
    int res = -1;
    p_svdb_type type = svdb_get_type( self, tab_id, selection, col_id, type_id );
    if ( type != NULL )
    {
        res = type->typedesc.intrinsic_bits;
    }
    return res;
}


MOD_EXPORT int CC svdb_typedim( void * self, const int tab_id,
            const int selection, const int col_id, const int type_id )
{
    int res = -1;
    p_svdb_type type = svdb_get_type( self, tab_id, selection, col_id, type_id );
    if ( type != NULL )
    {
        res = type->typedesc.intrinsic_dim;
    }
    return res;
}


static void svdb_replace_string( const char **dst, const char * s )
{
    free_if_not_null( *dst );
    *dst = string_dup_measure ( s, NULL );
}


static void svdb_set_tab_elem_separator( p_svdb_tab tab,
            const int selection, const int col_id, const char * s )
{
    Vector * v = svdb_get_column_vector( tab, selection );
    if ( col_id >= 0 )
    {
        p_svdb_col col = VectorGet ( v, col_id );
        if ( col != NULL )
        {
            svdb_replace_string( &col->elem_separator, s );
        }
    }
    else
    {
        int i, n = VectorLength( v );
        for ( i = 0; i < n; ++ i )
        {
            p_svdb_col col = VectorGet ( v, i );
            if ( col != NULL )
            {
                svdb_replace_string( &col->elem_separator, s );
            }
        }
    }
}


MOD_EXPORT void CC svdb_set_elem_separator( void * self,
            const int tab_id, const int selection, const int col_id,
            const char * s )
{
    if ( self != NULL && s != NULL && s[0] != 0 )
    {
        p_svdb_conn pself = self;
        if ( tab_id >= 0 )
        {
            p_svdb_tab tab = VectorGet ( &pself->tables, tab_id );
            if ( tab != NULL )
            {
                svdb_set_tab_elem_separator( tab, selection, col_id, s );
            }
        }
        else
        {
            int i, n = VectorLength( &pself->tables );
            for ( i = 0; i < n; ++ i )
            {
                p_svdb_tab tab = VectorGet ( &pself->tables, i );
                if ( tab != NULL )
                {
                    svdb_set_tab_elem_separator( tab, selection, col_id, s );
                }
            }
        }
    }
}


static void svdb_set_tab_dim_separator( p_svdb_tab tab,
            const int selection, const int col_id, const char * s )
{
    Vector * v = svdb_get_column_vector( tab, selection );
    if ( col_id >= 0 )
    {
        p_svdb_col col = VectorGet ( v, col_id );
        if ( col != NULL )
        {
            svdb_replace_string( &col->dim_separator, s );
        }
    }
    else
    {
        int i, n = VectorLength( v );
        for ( i = 0; i < n; ++ i )
        {
            p_svdb_col col = VectorGet ( v, i );
            if ( col != NULL )
            {
                svdb_replace_string( &col->dim_separator, s );
            }
        }
    }
}


MOD_EXPORT void CC svdb_set_dim_separator( void * self,
            const int tab_id, const int selection, const int col_id,
            const char * s )
{
    if ( self != NULL && s != NULL && s[0] != 0 )
    {
        p_svdb_conn pself = self;
        if ( tab_id >= 0 )
        {
            p_svdb_tab tab = VectorGet ( &pself->tables, tab_id );
            if ( tab != NULL )
            {
                svdb_set_tab_dim_separator( tab, selection, col_id, s );
            }
        }
        else
        {
            int i, n = VectorLength( &pself->tables );
            for ( i = 0; i < n; ++ i )
            {
                p_svdb_tab tab = VectorGet ( &pself->tables, i );
                if ( tab != NULL )
                {
                    svdb_set_tab_dim_separator( tab, selection, col_id, s );
                }
            }
        }
    }
}


MOD_EXPORT int CC svdb_is_enabled( void * self, const int tab_id,
            const int selection, const int col_id )
{
    int res = 0;
    p_svdb_col col = svdb_get_col( self, tab_id, selection, col_id );
    if ( col != NULL )
    {
        res = col->enabled;
    }
    return res;
}


static void svdb_set_columns_usage( p_svdb_tab tab, int enabled )
{
    Vector * v = svdb_get_column_vector( tab, 0 );
    uint32_t i, n = VectorLength( v );
    for ( i = 0; i < n; ++i )
    {
        p_svdb_col col = VectorGet ( v, i );
        if ( col != NULL )
        {
            col->enabled = enabled;
            col->open = 0;
            free_if_not_null( col->cast );
            col->cast = NULL;
        }
    }
}


static p_svdb_col svdb_find_col( p_svdb_tab tab, const char *name )
{
    p_svdb_col res = NULL;
    Vector * v = svdb_get_column_vector( tab, 0 );
    uint32_t i, n = VectorLength( v );
    for ( i = 0; i < n && res == NULL; ++i )
    {
        p_svdb_col col = VectorGet ( v, i );
        if ( col != NULL )
        {
            if ( svdb_strcmp( col->name, name ) == 0 )
            {
                res = col;
            }
        }
    }
    return res;
}


static void svdb_trim( char * s )
{
    int i, l = string_measure( s, NULL );
    for ( i = 0; i < l; ++i )
    {
        if ( s[ i ] == ' ' )
        {
            s[ i ] = 0;
        }
    }
    while( s[ 0 ] == 0 )
    {
        memmove( s, &s[1], l );
    }
}

static void svdb_enable_col( p_svdb_tab tab, const char *defline,
                             int start_cast, int end_cast, int start_name, int end_name )
{
    char * cast = NULL;
    char * name = NULL;
    int l = ( end_cast - start_cast );
    if ( l > 1 )
    {
        cast = calloc( 1, l );
        if ( cast != NULL )
        {
            memmove( cast, &defline[ start_cast + 1 ], l-1 );
            svdb_trim( cast );
        }
    }

    l = ( end_name - start_name );
    if ( l > 0 )
    {
        name = calloc( 1, l + 1 );
        if ( name != NULL )
        {
            memmove( name, &defline[ start_name ], l );
            svdb_trim( name );
        }
    }

    if ( name != NULL )
    {
        p_svdb_col col = svdb_find_col( tab, name );
        if ( col != NULL )
        {
            col->enabled = true;
            free_if_not_null( col->cast );
            if ( cast != NULL )
            {
                col->cast = string_dup_measure ( cast, NULL );
            }
        }
    }

    free_if_not_null( cast );
    free_if_not_null( name );
}


static void svdb_defline( p_svdb_tab tab, const char *defline )
{
    if ( defline == NULL || defline[0] == 0 )
    {
        /* if defline is empty, enable all columns without a cast */
        svdb_set_columns_usage( tab, 1 );
    }
    else
    {
        int start_cast = -1;
        int end_cast = -1;
        int start_name = 0;
        int i, l = string_measure ( defline, NULL );

        /* disable all columns first, clear all casts */
        svdb_set_columns_usage( tab, 0 );
        /* enable only what is in the defline "(CAST1)NAME1,(CAST2)NAME2,NAME3"*/
        for ( i = 0; i < l; ++i )
        {
            switch ( defline[ i ] )
            {
            case '(' : start_cast = i; break;
            case ')' : end_cast = i; start_name = i+1; break;
            case ',' : svdb_enable_col( tab, defline, start_cast, end_cast, start_name, i );
                       start_cast = end_cast = -1;
                       start_name = i+1;
                       break;
            }
        }
        svdb_enable_col( tab, defline, start_cast, end_cast, start_name, l );
    }
}


static int svdb_add_to_cursor( p_svdb_tab tab )
{
    int res = 0;
    rc_t rc = 0;
    Vector * v = svdb_get_column_vector( tab, 0 );
    uint32_t i, n = VectorLength( v );
    for ( i = 0; i < n && rc == 0; ++i )
    {
        p_svdb_col col = VectorGet ( v, i );
        if ( col != NULL && col->enabled )
        {
            if ( col->cast != NULL )
            {
                rc = VCursorAddColumn ( tab->cursor, &col->cur_idx, "(%s)%s", col->cast, col->name );
            }
            else
            {
                rc = VCursorAddColumn ( tab->cursor, &col->cur_idx, "%s", col->name );
            }
            if ( rc != 0 )
            {
                LOGERR( klogInt, rc, "VCursorAddColumn() failed in svdb_add_to_cursor()" );
            }
            else
            {
                col->open = 1;
                ++res;
            }
        }
    }
    return res;
}


static void svdb_read_cursor_data_types( p_svdb_tab tab )
{
    Vector * v = svdb_get_column_vector( tab, 0 );
    uint32_t i, n = VectorLength( v );
    for ( i = 0; i < n; ++i )
    {
        p_svdb_col col = VectorGet ( v, i );
        if ( col != NULL && col->open )
        {
            rc_t rc = VCursorDatatype ( tab->cursor, col->cur_idx,
                              &col->cursor_typedecl, &col->cursor_typedesc );
            free_if_not_null( col->cur_typedecl );
            if ( rc == 0 )
            {
                char buf[ 64 ];
                rc = VTypedeclToText( &col->cursor_typedecl, tab->conn->schema,
                                      buf, sizeof( buf ) );
                if ( rc == 0 )
                {
                    col->cur_typedecl = string_dup_measure ( buf, NULL );
                    col->value_trans = sra_get_value_trans_func( tab->conn->schema, &col->cursor_typedecl );
                    col->dim_trans = sra_get_dim_trans_func( tab->conn->schema, &col->cursor_typedecl );
                }
            }
        }
    }
}


static void svdb_discover_static_columns( p_svdb_tab tab )
{
    Vector * v = svdb_get_column_vector( tab, ALL_COLUMNS );
    uint32_t idx, i, n = VectorLength( v );
    int64_t first;
    uint64_t range;

    tab->first = 0;
    tab->range = 0;
    for ( i = 0; i < n; ++i )
    {
        p_svdb_col col = VectorGet ( v, i );
        if ( col != NULL && col->open )
        {
            rc_t rc = VCursorIdRange ( tab->cursor, col->cur_idx, &first, &range );
            if ( rc == 0 )
            {
                if ( range == 0 )
                {
                    VectorAppend ( &tab->static_columns, &idx, col );
                }
                else
                {
                    VectorAppend ( &tab->non_static_columns, &idx, col );
                    if ( tab->range == 0 )
                    {
                        tab->first = first;
                        tab->range = range;
                    }
                }
            }
        }
    }
}


MOD_EXPORT int CC svdb_open_table( void * self, const int tab_id,
            const char *defline )
{
    int res = -1;
    p_svdb_tab tab = svdb_get_tab( self, tab_id );
    if ( tab != NULL )
    {
        rc_t rc;
        svdb_clear_vector( &tab->static_columns );
        svdb_clear_vector( &tab->non_static_columns );
        svdb_defline( tab, defline );
        if ( tab->cursor != NULL )
        {
            rc = VCursorRelease( tab->cursor );
            if ( rc != 0 )
            {
                LOGERR( klogInt, rc, "VCursorRelease() failed in svdb_open_table()" );
            }
        }
        rc = VTableCreateCursorRead ( tab->tab, &tab->cursor );
        if ( rc != 0 )
        {
            LOGERR( klogInt, rc, "VTableCreateCursorRead() failed in svdb_open_table()" );
        }
        else
        {
            res = svdb_add_to_cursor( tab );
            if ( res > 0 )
            {
                rc = VCursorOpen ( tab->cursor );
                if ( rc == 0 )
                {
                    svdb_read_cursor_data_types( tab );
                    svdb_discover_static_columns( tab );
                }
                else
                {
                    LOGERR( klogInt, rc, "VCursorOpen() failed in svdb_open_table()" );
                    res = -1;
                }
            }
        }
    }
    return res;
}


MOD_EXPORT int CC svdb_max_colname_length( void * self,
            const int tab_id, const int selection )
{
    int res = 0;
    p_svdb_tab tab = svdb_get_tab( self, tab_id );
    if ( tab != NULL )
    {
        Vector * v = svdb_get_column_vector( tab, selection );
        uint32_t i, n = VectorLength( v );
        for ( i = 0; i < n; ++i )
        {
            p_svdb_col col = VectorGet ( v, i );
            if ( col != NULL && col->enabled )
            {
                int l = string_measure ( col->name, NULL );
                if ( l > res )
                {
                    res = l;
                }
            }
        }
    }
    return res;
}


MOD_EXPORT unsigned long long int CC svdb_row_range( void * self,
            const int tab_id )
{
    unsigned long long int res = 0;
    p_svdb_tab tab = svdb_get_tab( self, tab_id );
    if ( tab != NULL )
    {
        res = tab->range;
    }
    return res;
}


MOD_EXPORT signed long long int CC svdb_first_row( void * self,
            const int tab_id )
{
    signed long long int res = 0;
    p_svdb_tab tab = svdb_get_tab( self, tab_id );
    if ( tab != NULL )
    {
        res = tab->first;
    }
    return res;
}

static void svdb_print_char( p_print_ctx dst, const char c )
{
    if ( dst->printed < dst->buf_len )
    {
        dst->buf[ ( dst->printed )++ ] = c;
    }
    ( dst->needed )++;
}


static void svdb_print_str( p_print_ctx dst, const char * s )
{
    while( *s > 0 )
    {
        svdb_print_char( dst, *(s++) );
    }
}


#define BYTE_OFFSET(VALUE)  ( (VALUE) >> 3 )
#define BIT_OFFSET(VALUE)   ( (VALUE) & 0x7 )

uint8_t BitLength2Bytes[65] =
{
         /* 0  1  2  3  4  5  6  7  8  9*/
   /* 0 */  0, 1, 1, 1, 1, 1, 1, 1, 1, 2,
   /* 1 */  2, 2, 2, 2, 2, 2, 2, 3, 3, 3,
   /* 2 */  3, 3, 3, 3, 3, 4, 4, 4, 4, 4,
   /* 3 */  4, 4, 4, 5, 5, 5, 5, 5, 5, 5,
   /* 4 */  5, 6, 6, 6, 6, 6, 6, 6, 6, 7,
   /* 5 */  7, 7, 7, 7, 7, 7, 7, 8, 8, 8,
   /* 6 */  8, 8, 8, 8, 8
};

/*************************************************************************************
n_bits   [IN] ... number of bits

calculates the number of bytes that have to be copied to contain the given
number of bits
*************************************************************************************/
static uint16_t bitlength_2_bytes( const size_t n_bits )
{
    if ( n_bits > 64 )
        return 8;
    else
        return BitLength2Bytes[ n_bits ];
}

uint64_t BitLength2Mask[33] =
{
   /* 0 */ 0x00,
   /* 1 ..  4 */  0x1,                0x3,                0x7,                0xF,
   /* 5 ..  8 */  0x1F,               0x3F,               0x7F,               0xFF,
   /* 9 .. 12 */  0x1FF,              0x3FF,              0x7FF,              0xFFF,
   /*13 .. 16 */  0x1FFF,             0x3FFF,             0x7FFF,             0xFFFF,
   /*17 .. 20 */  0x1FFFF,            0x3FFFF,            0x7FFFF,            0xFFFFF,
   /*21 .. 24 */  0x1FFFFF,           0x3FFFFF,           0x7FFFFF,           0xFFFFFF,
   /*25 .. 28 */  0x1FFFFFF,          0x3FFFFFF,          0x7FFFFFF,          0xFFFFFFF,
   /*29 .. 32 */  0x1FFFFFFF,         0x3FFFFFFF,         0x7FFFFFFF,         0xFFFFFFFF
 };

/*************************************************************************************
n_bits   [IN] ... number of bits

creates a bitmask to mask exactly the given number of bits from a longer value
*************************************************************************************/
static uint64_t bitlength_2_mask( const size_t n_bits )
{
    uint64_t res;
    if ( n_bits < 33 )
        res = BitLength2Mask[ n_bits ];
    else
    {
        if ( n_bits < 65 )
            res = BitLength2Mask[ n_bits-32 ];
        else
            res = 0xFFFFFFFF;
        res <<= 32;
        res |= 0xFFFFFFFF;
    }
    return res;
}


static void move_to_value( void * dst, p_data_src src )
{
    int ofs = ( src->boff + ( src->elem_bits * src->elem_idx ) );
    char *src_ptr = ( char* )src->base + BYTE_OFFSET( ofs );
    if ( BIT_OFFSET( ofs ) == 0 )
    {
        memmove( dst, src_ptr, bitlength_2_bytes( src->elem_bits ) );
    }
    else
    {
        bitcpy ( dst, 0, src_ptr, BIT_OFFSET( ofs ), src->elem_bits );
    }
}


static uint64_t move_to_uint64( p_data_src src )
{
    uint64_t value = 0;
    move_to_value( &value, src );
    if ( src->elem_bits & 7 )
    {
        size_t unpacked = 0;
        Unpack( src->elem_bits, sizeof( value ), &value, 0, src->elem_bits,
                NULL, &value, sizeof(value), &unpacked );
    }
    value &= bitlength_2_mask( src->elem_bits );
    return value;
}


static void print_boolean_element( p_svdb_col col, p_print_ctx dst, p_data_src src )
{
    uint64_t value = move_to_uint64( src );
    if ( value == 0 )
        svdb_print_char( dst, '0' );
    else
        svdb_print_char( dst, '1' );
}


static void print_uint_element( p_svdb_col col, p_print_ctx dst, p_data_src src )
{
    uint64_t value = move_to_uint64( src );
    if ( src->elem_idx > 0 )
    {
        svdb_print_str( dst, col->elem_separator );
    }
    if ( col->value_trans != NULL )
    {

        const char * s = col->value_trans( (uint32_t)value );
        if ( s != NULL )
        {
            svdb_print_str( dst, s );
        }
    }
    else
    {
        char buf[ 32 ];
        size_t num_writ;
        if ( string_printf ( buf, sizeof( buf ), &num_writ, "%u", value ) == 0 )
        {
            svdb_print_str( dst, buf );
        }
    }
}


static void print_int_element( p_svdb_col col, p_print_ctx dst, p_data_src src )
{
    int64_t value = (int64_t)move_to_uint64( src );
    if ( src->elem_idx > 0 )
    {
        svdb_print_str( dst, col->elem_separator );
    }
    if ( col->value_trans != NULL )
    {
        const char * s = col->value_trans( (uint32_t)value );
        if ( s != NULL )
        {
            svdb_print_str( dst, s );
        }
    }
    else
    {
        rc_t rc;
        char buf[ 32 ];
        size_t num_writ;
        switch ( src->elem_bits )
        {
            case  8 : {
                        int8_t temp = (int8_t)value;
                        rc = string_printf ( buf, sizeof( buf ), &num_writ, "%d", temp );
                      }
                      break;
            case 16 : {
                        int16_t temp = (int16_t)value;
                        rc = string_printf ( buf, sizeof( buf ), &num_writ, "%d", temp );
                      }
                      break;
            case 32 : { 
                        int32_t temp = (int32_t)value;
                        rc = string_printf ( buf, sizeof( buf ), &num_writ, "%d", temp );
                      }
                      break;
            case 64 : { 
                        rc = string_printf ( buf, sizeof( buf ), &num_writ, "%ld", value );
                      }
                      break;
            default : {
                        rc = string_printf ( buf, sizeof( buf ), &num_writ, "?" );
                      }
        }
        if ( rc == 0 )
        {
            svdb_print_str( dst, buf );
        }
    }
}


#define MAX_CHARS_FOR_DOUBLE 26
#define BITSIZE_OF_FLOAT ( sizeof(float) * 8 )
#define BITSIZE_OF_DOUBLE ( sizeof(double) * 8 )
static void print_float_element( p_svdb_col col, p_print_ctx dst, p_data_src src )
{
    rc_t rc;
    char buf[ 32 ];
    size_t num_writ;
    if ( src->elem_bits == BITSIZE_OF_FLOAT )
    {
        float value;
        move_to_value( &value, src );
        rc = string_printf ( buf, sizeof( buf ), &num_writ, "%e", value );
    }
    else if ( src->elem_bits == BITSIZE_OF_DOUBLE )
    {
        double value;
        move_to_value( &value, src );
        rc = string_printf ( buf, sizeof( buf ), &num_writ, "%e", value );
    }
    else
    {
        rc = string_printf ( buf, sizeof( buf ), &num_writ, "unknown float-type" );
    }
    if ( rc == 0 )
    {
        if ( src->elem_idx > 0 )
        {
            svdb_print_str( dst, col->elem_separator );
        }
        svdb_print_str( dst, buf );
    }
}


static void print_ascii_element( p_svdb_col col, p_print_ctx dst, p_data_src src )
{
    int ofs = ( src->elem_bits * src->elem_idx );
    char * src_ptr = (char*)src->base + BYTE_OFFSET( src->boff + ofs );
    svdb_print_char( dst, *src_ptr );
}


static void print_unicode_element( p_svdb_col col, p_print_ctx dst, p_data_src src )
{
    print_ascii_element( col, dst, src );
}


static void svdb_print_domain( p_svdb_col col, p_print_ctx dst, p_data_src src )
{
    switch ( col->cursor_typedesc.domain )
    {
        case vtdBool    : print_boolean_element( col, dst, src ); break;
        case vtdUint    : print_uint_element( col, dst, src ); break;
        case vtdInt     : print_int_element( col, dst, src ); break;
        case vtdFloat   : print_float_element( col, dst, src ); break;
        case vtdAscii   : print_ascii_element( col, dst, src ); break;
        case vtdUnicode : print_unicode_element( col, dst, src ); break;
    }
}


static void svdb_print_elem( p_svdb_col col, p_print_ctx dst, p_data_src src )
{
    uint32_t dim = col->cursor_typedesc.intrinsic_dim;

    if ( dim == 1 )
    {
        for ( src->elem_idx = 0; src->elem_idx < src->row_len; ++(src->elem_idx) )
        {
            svdb_print_domain( col, dst, src );
        }
    }
    else
    {
        data_src sub_src;
        sub_src.base = src->base;
        sub_src.boff = src->boff;
        sub_src.elem_bits = ( src->elem_bits / dim );
        for ( src->elem_idx = 0; src->elem_idx < src->row_len; ++(src->elem_idx ) )
        {
            char sep = col->dim_separator[0];
            svdb_print_char( dst, sep );

            if ( col->dim_trans != NULL )
            {
                char * s;
                const uint8_t *src_ptr = sub_src.base;
                src_ptr += BYTE_OFFSET( sub_src.boff );
                s = col->dim_trans( src_ptr );
                if ( s != NULL )
                {
                    svdb_print_str( dst, s );
                }
            }
            else
            {
                for ( sub_src.elem_idx = 0; sub_src.elem_idx < dim; ++sub_src.elem_idx )
                {
                    svdb_print_domain( col, dst, &sub_src );
                }
            }

            sep = col->dim_separator[1];
            if ( sep == 0 )
            {
                sep = col->dim_separator[0];
            }
            svdb_print_char( dst, sep );
            sub_src.boff += src->elem_bits;
        }
    }
}


MOD_EXPORT int CC svdb_element_count( void * self,
            const int tab_id, const int selection, const int col_id,
            const unsigned long long int row )
{
    int res = 0;
    p_svdb_col col = svdb_get_col( self, tab_id, selection, col_id );
    if ( col != NULL && row < col->tab->range && col->open )
    {
        data_src data;
        int64_t row_id = col->tab->first + row;
        rc_t rc = VCursorCellDataDirect ( col->tab->cursor, row_id, col->cur_idx,
                     &data.elem_bits, &data.base, &data.boff, &data.row_len );
        if ( rc != 0 )
        {
            LOGERR( klogInt, rc, "VCursorCellDataDirect() failed in element_count()" );
        }
        else
        {
            if ( data.row_len > 0 )
            {
                res = data.row_len;
            }
        }
    }
    return res;
}


MOD_EXPORT int CC svdb_element( void * self, char * buf, int buflen,
            const int tab_id, const int selection, const int col_id, const int elem_id,
            const unsigned long long int row )
{
    int res = 0;
    p_svdb_col col;
    buf[ 0 ] = 0;
    col = svdb_get_col( self, tab_id, selection, col_id );
    if ( col != NULL && row < col->tab->range && col->open )
    {
        rc_t rc;
        data_src data;
        print_ctx dst;
        int64_t row_id = col->tab->first + row;

        svdb_init_print_ctx( &dst, buf, buflen - 1 );
        rc = VCursorCellDataDirect ( col->tab->cursor, row_id, col->cur_idx,
                &data.elem_bits, &data.base, &data.boff, &data.row_len );
        if ( rc != 0 )
        {
            LOGERR( klogInt, rc, "VCursorCellDataDirect() failed in svdb_element()" );
        }
        else
        {
/*
            if ( data.row_len > 0 && elem_id < data.row_len )
            {

            }
*/
        }
    }
    return res;
}


MOD_EXPORT int CC svdb_cell( void * self, char * buf, int buflen,
           const int tab_id, const int selection, const int col_id,
           const unsigned long long int row )
{
    int res = 0;
    p_svdb_col col;
    buf[ 0 ] = 0;
    col = svdb_get_col( self, tab_id, selection, col_id );
    if ( col != NULL && row < col->tab->range && col->open )
    {
        data_src data;
        int64_t row_id = col->tab->first + row;
        rc_t rc = VCursorCellDataDirect ( col->tab->cursor, row_id, col->cur_idx,
                         &data.elem_bits, &data.base, &data.boff, &data.row_len );
        if ( rc != 0 )
        {
            LOGERR( klogInt, rc, "VCursorCellDataDirect() failed in svdb_cell()" );
        }
        else
        {
            if ( data.row_len > 0 )
            {
                print_ctx dst;

                svdb_init_print_ctx( &dst, buf, buflen - 1 );
                svdb_print_elem( col, &dst, &data ); /* <--- */
                dst.buf[ dst.printed ] = 0;
                res = dst.needed;
            }
        }
    }
    return res;
}


static int svdb_find_bufsize( p_svdb_col col, data_src *data )
{
    char buf[ 8 ];
    print_ctx dst;

    svdb_init_print_ctx( &dst, buf, sizeof buf );
    svdb_print_elem( col, &dst, data ); /* <--- */
    return dst.needed + 1;
}


MOD_EXPORT unsigned long long int CC svdb_find_fwd( void * self, const int tab_id,
            const int selection, const int col_id, const unsigned long long int row,
            const int chunksize, const char * pattern )
{
    unsigned long long int res = INVALID_ROW;
    p_svdb_col col = svdb_get_col( self, tab_id, selection, col_id );
    if ( col != NULL && row < col->tab->range && col->open && pattern != NULL )
    {
        int64_t search_row = row;
        rc_t rc = 0;
        int buf_size = 0;
        int chunk = 0;
        char * buf = NULL;

        while( ( rc == 0 ) && 
                ( res == INVALID_ROW ) &&
                ( (uint64_t)search_row < col->tab->range ) &&
                ( chunk < chunksize ) )
        {
            data_src data;
            int64_t row_id = col->tab->first + search_row;
            rc = VCursorCellDataDirect ( col->tab->cursor, row_id, col->cur_idx,
                         &data.elem_bits, &data.base, &data.boff, &data.row_len );
            if ( rc == 0 )
            {
                int this_size = svdb_find_bufsize( col, &data );
                ++chunk;
                if ( this_size > buf_size )
                {
                    buf_size = this_size;
                    if ( buf != NULL )
                    {
                        char * newbuf = realloc( buf, buf_size );
                        if ( newbuf != NULL )
                            buf = newbuf;
                        else
                        {
                            free( buf );
                            buf = NULL;
                        }
                    }
                    else
                        buf = malloc( buf_size );
                }
                if ( buf != NULL )
                {
                    print_ctx dst;
                    svdb_init_print_ctx( &dst, buf, buf_size - 1 );
                    svdb_print_elem( col, &dst, &data ); /* <--- */
                    dst.buf[ dst.printed ] = 0;
                    if ( strstr ( buf, pattern ) != NULL )
                    {
                        res = search_row;
                    }
                }
                search_row++;
            }
        }
        if ( ( chunk == chunksize )&&( res == INVALID_ROW ) )
        {
            res = 0xFFFFFFFFFFFFFFFE;
        }
        if ( buf != NULL )
            free( buf );
    }
    return res;
}


MOD_EXPORT unsigned long long int CC svdb_find_bwd( void * self, const int tab_id,
            const int selection, const int col_id, const unsigned long long int row,
            const int chunksize, const char * pattern )
{
    unsigned long long int res = INVALID_ROW;
    p_svdb_col col;
    col = svdb_get_col( self, tab_id, selection, col_id );
    if ( col != NULL && row < col->tab->range && col->open && pattern != NULL )
    {
        int64_t search_row = row;
        rc_t rc = 0;
        int buf_size = 0;
        int chunk = 0;
        char * buf = NULL;

        while( ( rc == 0 ) && ( res == INVALID_ROW ) &&
               ( search_row >= 0 ) && ( chunk < chunksize ) )
        {
            data_src data;
            int64_t row_id = col->tab->first + search_row;
            rc = VCursorCellDataDirect ( col->tab->cursor, row_id, col->cur_idx,
                         &data.elem_bits, &data.base, &data.boff, &data.row_len );
            if ( rc == 0 )
            {
                int this_size = svdb_find_bufsize( col, &data );
                ++chunk;
                if ( this_size > buf_size )
                {
                    buf_size = this_size;
                    if ( buf != NULL )
                    {
                        char * newbuf = realloc( buf, buf_size );
                        if ( newbuf != NULL )
                            buf = newbuf;
                        else
                        {
                            free( buf );
                            buf = NULL;
                        }
                    }
                    else
                        buf = malloc( buf_size );
                }
                if ( buf != NULL )
                {
                    print_ctx dst;
                    svdb_init_print_ctx( &dst, buf, buf_size - 1 );
                    svdb_print_elem( col, &dst, &data ); /* <--- */
                    dst.buf[ dst.printed ] = 0;
                    if ( strstr ( buf, pattern ) != NULL )
                    {
                        res = search_row;
                    }
                }
                search_row--;
            }
        }
        if ( ( chunk == chunksize )&&( res == INVALID_ROW ) )
        {
            res = 0xFFFFFFFFFFFFFFFE;
        }
        if ( buf != NULL )
            free( buf );
    }
    return res;
}


/* **************************************************************************************************************** */

struct svdb_fastq;
typedef struct svdb_fastq* p_svdb_fastq;

typedef struct svdb_fastq
{
    char * path;
    KDirectory * dir;
    const VDBManager * mgr;
    VSchema * schema;
    const VDatabase * db;
    const VTable * sequence;
    const VCursor * cursor;
    int is_db;

    uint32_t read_idx;
    uint32_t qual_idx;
    uint32_t start_idx;
    uint32_t len_idx;
    uint32_t name_idx;
    uint32_t rd_type_idx;
    uint32_t rd_filter_idx;

} svdb_fastq;


MOD_EXPORT void CC svdb_close_fastq( void * self )
{
    p_svdb_fastq pself = ( p_svdb_fastq )self;
    if ( pself != NULL )
    {
        rc_t rc = VCursorRelease( pself->cursor );
        if ( rc != 0 )
        {
            LOGERR( klogInt, rc, "VCursorRelease() failed in svdb_close_fastq()" );
        }
        rc = VTableRelease( pself->sequence );
        if ( rc != 0 )
        {
            LOGERR( klogInt, rc, "VTableRelease() failed in svdb_close_fastq()" );
        }
        rc = VDatabaseRelease( pself->db );
        if ( rc != 0 )
        {
            LOGERR( klogInt, rc, "VDatabaseRelease() failed in svdb_close_fastq()" );
        }
        rc = VSchemaRelease( pself->schema );
        if ( rc != 0 )
        {
            LOGERR( klogInt, rc, "VSchemaRelease() failed in svdb_close_fastq()" );
        }
        rc = VDBManagerRelease( pself->mgr );
        if ( rc != 0 )
        {
            LOGERR( klogInt, rc, "VDBManagerRelease() failed in svdb_close_fastq()" );
        }
        rc = KDirectoryRelease( pself->dir );
        if ( rc != 0 )
        {
            LOGERR( klogInt, rc, "KDirectoryRelease() failed in svdb_close_fastq()" );
        }
        free_if_not_null( pself->path );
        free( self );
    }
}


static bool svdb_KNamelist_contains( const KNamelist * nl, const char * to_find )
{
    bool res = false;
    uint32_t n;
    rc_t rc = KNamelistCount( nl, &n );
    if ( rc == 0 && n > 0 )
    {
        uint32_t i;
        for ( i = 0; i < n && !res && rc == 0; ++i )
        {
            const char * name;
            rc = KNamelistGet ( nl, i, &name );
            if ( rc == 0 && name != NULL )
                res = ( svdb_strcmp( name, to_find ) == 0 );
        }
    }
    return res;
}


static rc_t svdb_discover_fastq_columns( p_svdb_fastq pself )
{
    KNamelist *col_names;
    rc_t rc = VTableListReadableColumns ( pself->sequence, &col_names );
    if ( rc == 0 )
    {
        pself->read_idx     = INVALID_COL;
        pself->qual_idx     = INVALID_COL;
        pself->start_idx    = INVALID_COL;
        pself->len_idx      = INVALID_COL;
        pself->name_idx     = INVALID_COL;
        pself->rd_type_idx  = INVALID_COL;
        pself->rd_filter_idx= INVALID_COL;

        if ( svdb_KNamelist_contains( col_names, "READ" ) )
        {
            rc = VCursorAddColumn ( pself->cursor, &pself->read_idx, "(INSDC:dna:text)READ" );
            if ( rc != 0 )
                log_and_err( rc, "VCursorAddColumn( READ ) failed" );
        }

        if ( rc == 0 && svdb_KNamelist_contains( col_names, "QUALITY" ) )
        {
            rc = VCursorAddColumn ( pself->cursor, &pself->qual_idx, "(INSDC:quality:text:phred_33)QUALITY" );
            if ( rc != 0 )
                log_and_err( rc, "VCursorAddColumn( QUALITY ) failed" );
        }

        if ( rc == 0 && svdb_KNamelist_contains( col_names, "READ_START" ) )
        {
            rc = VCursorAddColumn ( pself->cursor, &pself->start_idx, "(INSDC:coord:zero)READ_START" );
            if ( rc != 0 )
                log_and_err( rc, "VCursorAddColumn( READ_START ) failed" );
        }

        if ( rc == 0 && svdb_KNamelist_contains( col_names, "READ_LEN" ) )
        {
            rc = VCursorAddColumn ( pself->cursor, &pself->len_idx, "(INSDC:coord:len)READ_LEN" );
            if ( rc != 0 )
                log_and_err( rc, "VCursorAddColumn( READ_LEN ) failed" );
        }

        if ( rc == 0 && svdb_KNamelist_contains( col_names, "NAME" ) )
        {
            rc = VCursorAddColumn ( pself->cursor, &pself->name_idx, "(ascii)NAME" );
            if ( rc != 0 )
                log_and_err( rc, "VCursorAddColumn( NAME ) failed" );
        }

        if ( rc == 0 && svdb_KNamelist_contains( col_names, "READ_TYPE" ) )
        {
            rc = VCursorAddColumn ( pself->cursor, &pself->rd_type_idx, "(INSDC:SRA:xread_type)READ_TYPE" );
            if ( rc != 0 )
                log_and_err( rc, "VCursorAddColumn( READ_TYPE ) failed" );
        }

        if ( rc == 0 && svdb_KNamelist_contains( col_names, "READ_FILTER" ) )
        {
            rc = VCursorAddColumn ( pself->cursor, &pself->rd_filter_idx, "(INSDC:SRA:read_filter)READ_FILTER" );
            if ( rc != 0 )
                log_and_err( rc, "VCursorAddColumn( READ_FILTER ) failed" );
        }

        KNamelistRelease( col_names );
    }
    return rc;
}


MOD_EXPORT void * CC svdb_open_fastq( const char * path )
{
    p_svdb_fastq pself = NULL;
   if ( path != NULL && path[0] != 0 )
    {
        rc_t rc;
        pself = calloc( 1, sizeof( svdb_fastq ) );
        if ( pself == NULL )
        {
            rc = RC( rcApp, rcNoTarg, rcConstructing, rcMemory, rcExhausted );
            log_and_err( rc, "calloc() failed in svdb_open_fastq()" );
        }
        else
        {
            rc = KDirectoryNativeDir( &pself->dir );
            if ( rc != 0 )
                log_and_err( rc, "KDirectoryNativeDir() failed in svdb_open_fastq()" );

            if ( rc == 0 )
            {
                rc = VDBManagerMakeRead ( &pself->mgr, pself->dir );
                if ( rc != 0 )
                    log_and_err( rc, "VDBManagerMakeRead() failed in svdb_open_fastq()" );
            }

            if ( rc == 0 )
            {
                rc = VDBManagerMakeSRASchema( pself->mgr, &pself->schema );
                if ( rc != 0 )
                    log_and_err( rc, "VDBManagerMakeSRASchema() failed in svdb_open_fastq()" );
            }

            if ( rc == 0 )
            {
                rc = VDBManagerOpenDBRead( pself->mgr, &pself->db, pself->schema, "%s", path );
                if ( rc == 0 )
                {
                    pself->is_db = 1;
                    rc = VDatabaseOpenTableRead( pself->db, &pself->sequence, "SEQUENCE" );
                    if ( rc != 0 )
                    {
                        string_printf ( last_err, sizeof last_err, NULL,
                                        "VDatabaseOpenTableRead('%s')->'%R' failed in svdb_open_fastq()",
                                        path, rc );
                        svdb_set_last_err( "VDatabaseOpenTableRead() failed in svdb_open_fastq()" );

                    }
                }
                else
                {
                    rc = VDBManagerOpenTableRead( pself->mgr, &pself->sequence, pself->schema, "%s", path );
                    if ( rc != 0 )
                    {
                        string_printf ( last_err, sizeof last_err, NULL,
                                        "VDBManagerOpenTableRead('%s')->'%R' failed in svdb_open_fastq()",
                                        path, rc );
                        svdb_set_last_err( "VDBManagerOpenTableRead() failed in svdb_open_fastq()" );
                    }
                }
            }

            if ( rc == 0 )
            {
                rc = VTableCreateCursorRead ( pself->sequence, &pself->cursor );
                if ( rc != 0 )
                    log_and_err( rc, "VTableCreateCursorRead() failed in svdb_open_fastq()" );
            }

            if ( rc == 0 )
                rc = svdb_discover_fastq_columns( pself );

            if ( rc == 0 )
            {
                rc = VCursorOpen( pself->cursor );
                if ( rc != 0 )
                    log_and_err( rc, "VCursorOpen() failed in svdb_open_fastq()" );
            }
    
            if ( rc == 0 )
            {
                pself->path = string_dup_measure ( path, NULL );
                svdb_set_last_err( "OK" );
            }
            else
            {
                svdb_close_fastq( pself );
                pself = NULL;
            }
        }
    }
    else
    {
        svdb_set_last_err( "path empty in svdb_open_fastq()" );
    }
    return pself;

}


/*
    returns what the fastq-obj can produce
    0 ... nothing, handle invalid
    1 ... only READ ( that means fasta )
    2 ... READ and QUALITY ( but not spot splitting )
    3 ... READ, QUALITY and READ_START/READ_LEN ( splitted spots )
*/
MOD_EXPORT int CC svdb_fastq_scope( void * self )
{
    int res = 0;
    if ( self != NULL )
    {
        p_svdb_fastq pself = self;
        if ( pself->read_idx != INVALID_COL )
        {
            if ( pself->qual_idx != INVALID_COL )
            {
                if ( pself->start_idx != INVALID_COL &&
                     pself->len_idx != INVALID_COL )
                    res = 3;
                else
                    res = 2;
            }
            else res = 1;
        }
    }
    return res;
}


static int svdb_fastq_without_name_col( p_svdb_fastq pself, char * buf, int buflen,
                                        int seq, const unsigned long long int row )
{
    int res = 0;
    uint32_t elem_bits, boff, data_len;
    const char * data = NULL;

    rc_t rc = VCursorCellDataDirect( pself->cursor, row, pself->read_idx, &elem_bits,
                            (const void**)&data, &boff, &data_len );
    if ( rc == 0 )
    {
        size_t num_writ;
        if ( seq > 0 )
            rc = string_printf ( buf, buflen, &num_writ, "%s.%li length=%u/%u",
                                 pself->path, row, data_len, seq );
        else
            rc = string_printf ( buf, buflen, &num_writ, "%s.%li length=%u",
                                 pself->path, row, data_len );
        if ( rc == 0 )
            res = (int)num_writ;
    }

    return res;
}


static int svdb_fastq_with_name_col( p_svdb_fastq pself, char * buf, int buflen,
                                     int seq, const unsigned long long int row )
{
    int res = 0;
    uint32_t elem_bits, boff, name_len;
    const char * name = NULL;

    rc_t rc = VCursorCellDataDirect( pself->cursor, row, pself->name_idx, &elem_bits,
                                (const void**)&name, &boff, &name_len );
    if ( rc == 0 )
    {
        uint32_t data_len;
        size_t num_writ;
        if ( seq > 0 )
        {
            uint32_t * data = NULL;
            rc = VCursorCellDataDirect( pself->cursor, row, pself->len_idx, &elem_bits,
                                    (const void**)&data, &boff, &data_len );
            if ( rc == 0 )
                rc = string_printf ( buf, buflen, &num_writ, "%s.%li %.*s length=%u/%u",
                                     pself->path, row, name_len, name, data[ seq - 1 ], seq );
        }
        else
        {
            const char * data = NULL;
            rc = VCursorCellDataDirect( pself->cursor, row, pself->read_idx, &elem_bits,
                                    (const void**)&data, &boff, &data_len );
            if ( rc == 0 )
                rc = string_printf ( buf, buflen, &num_writ, "%s.%li %.*s length=%u",
                                     pself->path, row, name_len, name, data_len );

        }
        if ( rc == 0 ) res = (int)num_writ;
    }
    return res;
}


MOD_EXPORT int CC svdb_fastq_name( void * self, char * buf, int buflen,
                                   int seq, const unsigned long long int row )
{
    int res = 0;
    if ( self != NULL )
    {
        p_svdb_fastq pself = self;
        if ( pself->name_idx != INVALID_COL )
            res = svdb_fastq_with_name_col( pself, buf, buflen, seq, row );
        else
            res = svdb_fastq_without_name_col( pself, buf, buflen, seq, row );
    }
    return res;
}


static int svdb_fastq_data_ptr( p_svdb_fastq pself, const char ** buf, uint32_t src_idx,
                                int seq, const unsigned long long int row )
{ 
    int res = 0;
    uint32_t elem_bits, boff, data_len;
    const char * data = NULL;
    rc_t rc = VCursorCellDataDirect( pself->cursor, row, src_idx, &elem_bits,
                            (const void**)&data, &boff, &data_len );
    if ( rc == 0 )
    {
        if ( seq > 0 )
        {
            uint32_t read_start_len, read_len_len;
            uint32_t * read_start = NULL;
            uint32_t * read_len = NULL;
            rc = VCursorCellDataDirect( pself->cursor, row, pself->start_idx, &elem_bits,
                                        (const void**)&read_start, &boff, &read_start_len );
            if ( rc == 0 )
                rc = VCursorCellDataDirect( pself->cursor, row, pself->len_idx, &elem_bits,
                                            (const void**)&read_len, &boff, &read_len_len );
            if ( rc == 0 && seq <= (int)read_start_len && seq <= (int)read_len_len )
            {
                uint32_t start  = read_start[ seq - 1 ];
                res = read_len[ seq - 1 ];
                *buf = &data[ start ];
            }
        }
        else
        {
            *buf = data;
            res = data_len;
        }
    }
    return res;
}


static int svdb_fastq_data( p_svdb_fastq pself, char * buf, int buflen, uint32_t src_idx,
                            int seq, const unsigned long long int row )
{ 
    const char * src = NULL;
    int res = svdb_fastq_data_ptr( pself, &src, src_idx, seq, row );
    if ( res > 0 && src != NULL )
    {
        size_t num_writ;
        rc_t rc = string_printf ( buf, buflen, &num_writ, "%.*s", res, src );
        if ( rc == 0 ) res = (int)num_writ;
    }
    return res;
}


MOD_EXPORT int CC svdb_fastq_readcount( void * self, const unsigned long long int row )
{
    int res = 0;
    if ( self != NULL )
    {
        uint32_t elem_bits, boff, data_len;
        const char * data = NULL;
        p_svdb_fastq pself = self;
        rc_t rc = VCursorCellDataDirect( pself->cursor, row, pself->start_idx, &elem_bits,
                                         (const void**)&data, &boff, &data_len );
        if ( rc == 0 )
            res = data_len;
    }
    return res;
}


MOD_EXPORT int CC svdb_fastq_sequence( void * self, char * buf, int buflen,
                                       int seq, const unsigned long long int row )
{
    int res = 0;
    if ( self != NULL )
    {
        p_svdb_fastq pself = self;
        res = svdb_fastq_data( pself, buf, buflen, pself->read_idx, seq, row );
    }
    return res;
}


MOD_EXPORT int CC svdb_fastq_quality( void * self, char * buf, int buflen,
                                      int seq, const unsigned long long int row )
{
    int res = 0;
    if ( self != NULL )
    {
        p_svdb_fastq pself = self;
        res = svdb_fastq_data( pself, buf, buflen, pself->qual_idx, seq, row );
    }
    return res;
}


MOD_EXPORT int CC svdb_fastq_rd_type_available( void * self )
{
    int res = 0;
    if ( self != NULL )
    {
        p_svdb_fastq pself = self;
        if ( pself->rd_type_idx != INVALID_COL ) res = 1;
    }
    return res;
}


MOD_EXPORT int CC svdb_fastq_rd_filter_available( void * self )
{
    int res = 0;
    if ( self != NULL )
    {
        p_svdb_fastq pself = self;
        if ( pself->rd_filter_idx != INVALID_COL ) res = 1;
    }
    return res;
}


MOD_EXPORT int CC svdb_fastq_read_type_is_bio( void * self, int seq, const unsigned long long int row )
{
    int res = 0;
    if ( self != NULL && seq > 0 )
    {
        p_svdb_fastq pself = self;
        if ( pself->rd_type_idx != INVALID_COL )
        {
            uint32_t elem_bits, boff, data_len;
            const uint8_t * data = NULL;
            rc_t rc = VCursorCellDataDirect( pself->cursor, row, pself->rd_type_idx, &elem_bits,
                                             (const void**)&data, &boff, &data_len );
            if ( rc == 0 && seq <= (int)data_len )
            {
                if ( data[ seq - 1 ] & 0x01 ) res = 1;
            }
        }
    }
    return res;
}


MOD_EXPORT int CC svdb_fastq_read_filter_is_pass( void * self, int seq, const unsigned long long int row )
{
    int res = 0;
    if ( self != NULL && seq > 0 )
    {
        p_svdb_fastq pself = self;
        if ( pself->rd_filter_idx != INVALID_COL )
        {
            uint32_t elem_bits, boff, data_len;
            const uint8_t * data = NULL;
            rc_t rc = VCursorCellDataDirect( pself->cursor, row, pself->rd_filter_idx, &elem_bits,
                                             (const void**)&data, &boff, &data_len );
            if ( rc == 0 && seq <= (int)data_len )
            {
                if ( data[ seq - 1 ] == 0 ) res = 1;
            }
        }
    }
    return res;
}


MOD_EXPORT unsigned long long int CC svdb_fastq_row_count( void * self )
{
    unsigned long long int res = 0;
    if ( self != NULL )
    {
        int64_t first;
        uint64_t range;
        p_svdb_fastq pself = self;
        rc_t rc = VCursorIdRange ( pself->cursor, pself->read_idx, &first, &range );
        if ( rc == 0 )
            res = range;
    }
    return res;
}
