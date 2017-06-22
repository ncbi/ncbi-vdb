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

#include <klib/extern.h>

/*typedef struct VNamelist VNamelist;*/
#define KNAMELIST_IMPL VNamelist

#include <klib/rc.h>
#include <klib/text.h>
#include <klib/vector.h>
#include <klib/impl.h>
#include <os-native.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

/*--------------------------------------------------------------------------
 * KVectorNamelist ... a generic Namelist based on a Vector
 */
struct VNamelist
{
    KNamelist dad;
    Vector name_vector;
};


static
void CC VNameListDestroyNode( void* node, void* data )
{
    if ( node != NULL ) free( node );
}

static
rc_t CC VNameListDestroy ( VNamelist *self )
{
    VectorWhack ( &(self->name_vector), VNameListDestroyNode, NULL );
    free ( self );
    return 0;
}

LIB_EXPORT rc_t CC VNameListCount ( const VNamelist *self, uint32_t *count )
{
    * count = VectorLength ( &(self->name_vector) );
    return 0;
}

LIB_EXPORT rc_t CC VNameListGet ( const VNamelist *self, uint32_t idx, const char **name )
{
    if ( idx >= VectorLength ( &(self->name_vector) ) )
        return RC ( rcCont, rcNamelist, rcAccessing, rcParam, rcExcessive );
    * name = VectorGet ( &(self->name_vector), idx );
    return 0;
}

static KNamelist_vt_v1 sVNameList_vt =
{
    1, 0,
    VNameListDestroy,
    VNameListCount,
    VNameListGet
};

LIB_EXPORT rc_t CC VNamelistMake( VNamelist **names, const uint32_t alloc_blocksize  )
{
    rc_t rc;

    if ( names == NULL )
        rc = RC ( rcCont, rcNamelist, rcConstructing, rcParam, rcNull );
    else
    {
        (*names)=malloc( sizeof(**names) );
        if (*names != NULL )
        {
            rc = KNamelistInit( &(*names)->dad, (const KNamelist_vt*) &sVNameList_vt );
            if ( rc == 0 )
            {
                VectorInit ( & (*names)->name_vector, 0, alloc_blocksize );
            }
            else
            {
                free( *names );
                *names = NULL;
            }
        }
        else
        {
            rc = RC( rcCont, rcNamelist, rcListing, rcParam, rcNull );
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC VNamelistRelease( const VNamelist *self )
{
    rc_t rc = 0;
    if ( self != NULL )
    {
        rc = KNamelistRelease( &(self->dad) );
    }
    return rc;
}

LIB_EXPORT rc_t CC VNamelistToNamelist( VNamelist *self, KNamelist **cast )
{
    rc_t rc;
    if ( cast == NULL )
        rc = RC ( rcCont, rcNamelist, rcCasting, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcCont, rcNamelist, rcCasting, rcSelf, rcNull );
        else
        {
            rc = KNamelistAddRef ( & self -> dad );
            if ( rc == 0 )
            {
                * cast = & self -> dad;
                return 0;
            }
        }

        * cast = NULL;
    }

    return rc;
}

LIB_EXPORT rc_t CC VNamelistToConstNamelist( const VNamelist *self, const KNamelist **cast )
{
    return VNamelistToNamelist ( ( VNamelist* ) self, ( KNamelist** ) cast );
}

LIB_EXPORT rc_t CC VNamelistAppend( VNamelist *self, const char* src )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcCont, rcNamelist, rcInserting, rcSelf, rcNull );
    else
    {
        if ( src == NULL )
            rc = RC( rcCont, rcNamelist, rcInserting, rcString, rcNull );
        else
        {
            char* my_copy = string_dup( src, string_size( src ) );
            if ( my_copy == NULL )
                rc = RC( rcCont, rcNamelist, rcInserting, rcMemory, rcExhausted );
            else
            {
                rc = VectorAppend( &(self->name_vector), NULL, my_copy );
                if ( rc != 0 )
                    free ( my_copy );
            }
        }
    }
    return rc;
}


LIB_EXPORT rc_t CC VNamelistAppendString( VNamelist *self, const String * src )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcCont, rcNamelist, rcInserting, rcSelf, rcNull );
    else
    {
        if ( src == NULL )
            rc = RC( rcCont, rcNamelist, rcInserting, rcString, rcNull );
        else if ( src->addr == NULL )
            rc = RC( rcCont, rcNamelist, rcInserting, rcString, rcEmpty );
        else
        {
            char* my_copy = string_dup ( src->addr, src->len );
            if ( my_copy == NULL )
                rc = RC( rcCont, rcNamelist, rcInserting, rcMemory, rcExhausted );
            else
            {
                rc = VectorAppend( &(self->name_vector), NULL, my_copy );
                if ( rc != 0 )
                    free ( my_copy );
            }
        }
    }
    return rc;
}


LIB_EXPORT rc_t CC VNamelistIndexOf( VNamelist *self, const char* s, uint32_t *found )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcCont, rcNamelist, rcSearching, rcSelf, rcNull );
    else
    {
        if ( s == NULL )
            rc = RC( rcCont, rcNamelist, rcSearching, rcString, rcNull );
        else if ( s[0] == 0 )
            rc = RC( rcCont, rcNamelist, rcSearching, rcString, rcEmpty );
        else if ( found == NULL )
            rc = RC( rcCont, rcNamelist, rcSearching, rcParam, rcNull );
        else
        {
            uint32_t count;
            rc = VNameListCount ( self, &count );
            if ( rc == 0 )
            {
                uint32_t idx;
                size_t n1 = string_size ( s );
                for ( idx = 0; idx < count; ++idx )
                {
                    const char * name;
                    rc = VNameListGet ( self, idx, &name );
                    if ( rc == 0 )
                    {
                        size_t n2 = string_size ( name );
                        if ( string_cmp ( s, n1, name, n2, (uint32_t) ( ( n1 < n2 ) ? n2 : n1 ) ) == 0 )
                        {
                            *found = idx;
                            return 0;
                        }
                    }
                }
                rc = RC( rcCont, rcNamelist, rcSearching, rcString, rcNotFound );
            }
        }
    }
    return rc;
}


LIB_EXPORT rc_t CC VNamelistRemoveIdx( VNamelist *self, uint32_t idx )
{
    rc_t rc;
    if ( self == NULL )
        rc = RC ( rcCont, rcNamelist, rcSearching, rcSelf, rcNull );
    else
    {
        char *removed;
        rc = VectorRemove ( &(self->name_vector), idx, (void **)&removed );
        if ( rc == 0 )
            free( removed );
    }
    return rc;
}


LIB_EXPORT rc_t CC VNamelistRemove( VNamelist *self, const char* s )
{
    uint32_t idx;

    rc_t rc = VNamelistIndexOf( self, s, &idx );
    if ( rc == 0 )
        rc = VNamelistRemoveIdx( self, idx );
    return rc;
}


LIB_EXPORT rc_t CC VNamelistRemoveAll( VNamelist *self )
{
    rc_t rc = 0;
    uint32_t count = 1;
    while ( count > 0 && rc == 0 )
    {
        rc = VNameListCount ( self, &count );
        if ( rc == 0 && count > 0 )
            rc = VNamelistRemoveIdx( self, count - 1 );
    }
    return rc;
}


/* Reorder
 *  sort the names according to case sensitivity
 *  and UNICODE character code ordering
 *
 *  "case_insensitive" [ IN ] - when true, perform "tolower" on
 *   each character before compare
 */
static
int64_t CC vect_string_cmp ( const void **a, const void **b, void *ignore )
{
    return strcmp ( * a, * b );
}

static
int64_t CC vect_string_cmp_case ( const void **a, const void **b, void *ignore )
{
    uint32_t i;

    const char *ap = * a;
    const char *bp = * b;

    if ( ap == NULL )
        return bp != NULL;
    if ( bp == NULL )
        return -1;

    for ( i = 0; ; ++ i )
    {
        if ( ap [ i ] != bp [ i ] )
        {
            /* TBD - this should perform UTF-8 to UNICODE conversion
               but for that, create a function in text module */
            int64_t diff = (int64_t)tolower ( ap [ i ] ) - (int64_t)tolower ( bp [ i ] );
            if ( diff == 0 )
                continue;

            return diff;
        }

        if ( ap [ i ] == 0 )
            break;
    }

    return 0;
}

LIB_EXPORT void CC VNamelistReorder ( VNamelist *self, bool case_insensitive )
{
    if ( self != NULL )
    {
        VectorReorder ( & self -> name_vector, case_insensitive ? vect_string_cmp_case : vect_string_cmp, NULL );
    }
}


/* ---- split and join of strings ---------------------------------------------------------------- */

LIB_EXPORT rc_t CC foreach_String_part( const String * src, const uint32_t delim,
                                     rc_t ( CC * f ) ( const String * part, void *data ), void * data )
{
    rc_t rc = 0;
    if ( src == NULL || f == NULL )
        rc = RC ( rcCont, rcNamelist, rcParsing, rcParam, rcNull );
    else
    {
        String tmp, part;
        char * cptr;
        char last_cptr_char = 0;
        
        StringInit( &tmp, src -> addr, src -> size, src -> len );
        cptr = string_chr( tmp.addr, tmp.size, delim );
        if ( cptr != NULL ) last_cptr_char = cptr[ 0 ];
        while ( rc == 0 && cptr != NULL && tmp.len > 0 )
        {
            uint32_t l = ( cptr - tmp.addr );
            StringInit( &part, tmp.addr, l, l );
            rc = f( &part, data );
            if ( rc == 0 )
            {
                tmp.addr = cptr + 1;
                tmp.size -= l + 1;
                tmp.len -= l + 1;
                cptr = string_chr( tmp.addr, tmp.size, delim );
                if ( cptr != NULL ) last_cptr_char = cptr[ 0 ];
            }
        }
        if ( rc == 0 )
        {
            if ( tmp.len > 0 )
                rc = f( &tmp, data );
            else if ( last_cptr_char == delim )
            {
                part.addr = &last_cptr_char;
                part.size = 0;
                part.len = 0;
                rc = f( &part, data );
            }
        }
    }
    return rc;
}


LIB_EXPORT rc_t CC foreach_Str_part( const char * src, const uint32_t delim,
                                  rc_t ( CC * f ) ( const String * part, void *data ), void * data )
{
    rc_t rc;
    if ( src == NULL || f == NULL )
        rc = RC ( rcCont, rcNamelist, rcParsing, rcParam, rcNull );
    else
    {
        String S;
        StringInitCString( &S, src );
        rc = foreach_String_part( &S, delim, f, data );
    }
    return rc;
}


static rc_t CC add_to_nl( const String * part, void * data )
{
    return VNamelistAppendString ( data, part );
}


LIB_EXPORT rc_t CC VNamelistSplitString ( VNamelist * list, const String * str, const uint32_t delim )
{
    rc_t rc = 0;
    if ( list == NULL )
        rc = RC ( rcCont, rcNamelist, rcParsing, rcSelf, rcNull );
    else if ( str == NULL )
        rc = RC ( rcCont, rcNamelist, rcParsing, rcParam, rcNull );
    else
        rc = foreach_String_part( str, delim, add_to_nl, list );
    return rc;
}


LIB_EXPORT rc_t CC VNamelistSplitStr ( VNamelist * list, const char * str, const uint32_t delim )
{
    String S;
    StringInitCString( &S, str );
    return VNamelistSplitString ( list, &S, delim );
}


LIB_EXPORT rc_t CC VNamelistFromKNamelist ( VNamelist ** list, const KNamelist * src )
{
    rc_t rc = 0;
    if ( list == NULL )
        rc = RC ( rcCont, rcNamelist, rcParsing, rcSelf, rcNull );
    else
    {
        *list = NULL;
        if ( src == NULL )
            rc = RC ( rcCont, rcNamelist, rcParsing, rcParam, rcNull );
        else
        {
            uint32_t count;
            rc = KNamelistCount ( src, &count );
            if ( rc == 0 )
            {
                if ( count == 0 )
                    rc = RC ( rcCont, rcNamelist, rcParsing, rcParam, rcEmpty );    
                else
                {
                    rc = VNamelistMake( list, count );
                    if ( rc == 0 )
                    {
                        uint32_t idx;
                        for ( idx = 0; rc == 0 && idx < count; ++idx )
                        {
                            const char * s = NULL;
                            rc = KNamelistGet ( src, idx, &s );
                            if ( rc == 0 && s != NULL )
                            {
                                rc = VNamelistAppend ( *list, s );
                            }
                        }
                    }
                }
            }
        }
    }
    return rc;
}


LIB_EXPORT rc_t CC CopyVNamelist ( VNamelist ** list, const VNamelist * src )
{
    rc_t rc = 0;
    if ( list == NULL )
        rc = RC ( rcCont, rcNamelist, rcParsing, rcSelf, rcNull );
    else
    {
        const KNamelist * casted;
        *list = NULL;
        rc = VNamelistToConstNamelist ( src, &casted );
        if ( rc == 0 )
            rc = VNamelistFromKNamelist ( list, casted );
    }
    return rc;
}


LIB_EXPORT rc_t CC VNamelistFromString ( VNamelist ** list, const String * str, const uint32_t delim )
{
    rc_t rc = VNamelistMake( list, 10 );
    if ( rc == 0 )
    {
        rc = VNamelistSplitString ( *list, str, delim );
        if ( rc != 0 )
        {
            VNamelistRelease( *list );
            *list = NULL;
        }
    }
    return rc;
}


LIB_EXPORT rc_t CC VNamelistFromStr ( VNamelist ** list, const char * str, const uint32_t delim )
{
    String S;
    StringInitCString( &S, str );
    return VNamelistFromString ( list, &S, delim );
}


static rc_t join_size( const VNamelist * list, uint32_t count, size_t * size )
{
    rc_t rc = 0;
    uint32_t idx;
    *size = 0;
    for ( idx = 0; rc == 0 && idx < count; ++idx )
    {
        const char * item;
        rc = VNameListGet ( list, idx, &item );
        if ( rc == 0 )
            *size += string_size ( item );
    }
    *size += ( count - 1 );
    return rc;
}


LIB_EXPORT rc_t CC VNamelistJoin( const VNamelist * list, const uint32_t delim, const String ** rslt )
{
    rc_t rc = 0;
    if ( rslt == NULL )
        rc = RC ( rcCont, rcNamelist, rcRetrieving, rcParam, rcNull );
    else
    {
        *rslt = NULL;
        if ( list == NULL )
            rc = RC ( rcCont, rcNamelist, rcRetrieving, rcSelf, rcNull );
        else
        {
            uint32_t count;
            char empty[ 1 ];
            
            rc = VNameListCount ( list, &count );
            if ( rc == 0 )
            {
                String j;
                char * buffer = NULL;
                if ( count < 1 )
                {
                    j.addr = empty;
                    empty[ 0 ] = 0;
                    j.len = 0;
                    j.size = 1;
                }
                else if ( count == 1 )
                {
                    const char * item;
                    rc = VNameListGet ( list, 0, &item );
                    if ( rc == 0 )
                        StringInitCString( &j, item );
                }
                else
                {
                    size_t js;
                    rc = join_size( list, count, &js );
                    if ( rc == 0 )
                    {
                        buffer = malloc( js + 1 );
                        if ( buffer == NULL )
                            rc = RC ( rcCont, rcNamelist, rcRetrieving, rcMemory, rcExhausted );
                        else
                        {
                            uint32_t idx, dst = 0;
                            size_t dst_size = js;
                            for ( idx = 0; rc == 0 && idx < count; ++ idx )
                            {
                                const char * item;
                                rc = VNameListGet ( list, idx, &item );
                                if ( rc == 0 )
                                {
                                    size_t item_size = string_size ( item );
                                    string_copy ( &buffer[ dst ], dst_size, item, item_size );
                                    dst += item_size;
                                    if ( idx < ( count - 1 ) )
                                    {
                                        buffer[ dst++ ] = delim;
                                        dst_size -= ( item_size + 1 );
                                    }
                                }
                            }
                            if ( rc == 0 )
                            {
                                buffer[ dst ] = 0;
                                StringInitCString( &j, buffer );
                                /*
                                j.addr = buffer;
                                j.len  = ( uint32_t )( dst & 0xFFFFFFFF );
                                j.size = dst;
                                */
                            }
                        }
                    }
                }
                if ( rc == 0 )
                    rc = StringCopy ( rslt, &j );
                if ( buffer != NULL )
                    free( ( void * ) buffer );
            }
        }
    }
    return rc;
}


LIB_EXPORT rc_t CC VNamelistContainsString( const VNamelist * list,
    const String * item, int32_t * idx )
{
    rc_t rc = 0;
    if ( idx == NULL )
        rc = RC ( rcCont, rcNamelist, rcRetrieving, rcParam, rcNull );
    else
    {
        *idx = -1;
        if ( item == NULL )
            rc = RC ( rcCont, rcNamelist, rcRetrieving, rcParam, rcNull );
        else if ( list == NULL )
            rc = RC ( rcCont, rcNamelist, rcRetrieving, rcSelf, rcNull );
        else
        {
            uint32_t count, i;
            rc = VNameListCount ( list, &count );
            for ( i = 0; rc == 0 && *idx < 0 && i < count; ++i )
            {
                const char * s;
                rc = VNameListGet ( list, i, &s );
                if ( rc == 0 )
                {
                    int cmp;
                    String S;
                    StringInitCString( &S, s );
                    cmp = StringCompare( item, &S );
                    if ( cmp == 0 )
                        *idx = i;
                }
            }
        }
    }
    return rc;
}


LIB_EXPORT rc_t CC VNamelistContainsStr( const VNamelist * list,
        const char * item, int32_t * idx )
{
    rc_t rc = 0;
    if ( idx == NULL )
        rc = RC ( rcCont, rcNamelist, rcRetrieving, rcParam, rcNull );
    else
    {
        *idx = -1;
        if ( item == NULL )
            rc = RC ( rcCont, rcNamelist, rcRetrieving, rcParam, rcNull );
        else if ( list == NULL )
            rc = RC ( rcCont, rcNamelist, rcRetrieving, rcSelf, rcNull );
        else
        {
            String S;
            StringInitCString( &S, item );
            return VNamelistContainsString( list, &S, idx );
        }
    }
    return rc;
}
