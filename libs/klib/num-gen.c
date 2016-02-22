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
#include <klib/rc.h>
#include <klib/printf.h>
#include <klib/text.h>
#include <klib/vector.h>
#include <klib/num-gen.h>
#include <sysalloc.h>

#include <strtol.h>
#include <stdlib.h>
#include <string.h>


typedef struct num_gen_node
{
    int64_t start;
    uint64_t count; /* 0 ... skip, > 0 ... valid */
} num_gen_node;


/* **************************************************************************
{ start:5,count: 0 } ---> [ ]
{ start:5,count: 1 } ---> [ 5 ]
{ start:5,count: 2 } ---> [ 5, 6 ]
 ************************************************************************** */

struct num_gen
{
    Vector nodes;
    bool sorted;
};


struct num_gen_iter
{
    Vector nodes;
    uint32_t curr_node;
    uint64_t curr_node_sub_pos;
    uint64_t total;
    uint64_t progress;
    int64_t min_value;
    int64_t max_value;
};

/* forward decl. for fixing-function */
static rc_t num_gen_fix_overlaps( struct num_gen * self, uint32_t * count );


/* helper function to destroy a node*/
static void CC num_gen_node_destroy( void *item, void *data )
{
    free( item );
}


/* helper function to create a node from start/count */
static struct num_gen_node * num_gen_make_node( const int64_t start, const uint64_t count )
{
    num_gen_node * p = malloc( sizeof( * p ) );
    if ( p != NULL )
    {
        p -> start = start;
        p -> count = count;
    }
    return p;
}


/* helper callback to compare 2 nodes, lets VectorInsert create a sorted vector */
static int64_t CC num_gen_insert_helper( const void* item1, const void* item2 )
{
    int64_t res = 0;
    const num_gen_node * node1 = item1;
    const num_gen_node * node2 = item2;
    if ( node1 != NULL && node2 != NULL )
    {
        if ( node1 -> start < node2 -> start )
            res = -1;
        else if ( node1 -> start > node2 -> start )
            res = 1;
        else if ( node1 -> count < node2 -> count )
            res = -1;
        else if ( node1 -> count > node2 -> count )
            res = 1;
    }
    return res;
}


/* helper callback to create a deep and conditional copy of a node-vector */
static void CC num_gen_copy_cb_sorted( void *item, void *data )
{
    num_gen_node * node = item;
    if ( node != NULL && node -> count > 0 )
    {
        num_gen_node * new_node = num_gen_make_node( node->start, node->count );
        if ( new_node != NULL )
        {
            Vector * dst = data;
            if ( dst != NULL )
                VectorInsert( dst, new_node, NULL, num_gen_insert_helper );
        }
    }
}

static void CC num_gen_copy_cb_unsorted( void *item, void *data )
{
    num_gen_node * node = item;
    if ( node != NULL && node -> count > 0 )
    {
        num_gen_node * new_node = num_gen_make_node( node->start, node->count );
        if ( new_node != NULL )
        {
            Vector * dst = data;
            if ( dst != NULL )
                VectorAppend( dst, NULL, new_node );
        }
    }
}


/* helper function that creates a deep and conditional copy of a node-vector */
static void num_gen_copy_vector( const Vector * src, Vector * dst, bool sorted )
{
    if ( src != NULL && dst != NULL )
    {
        if ( sorted )
            VectorForEach ( src, false, num_gen_copy_cb_sorted, dst );
        else
            VectorForEach ( src, false, num_gen_copy_cb_unsorted, dst );
    }
}


/* helper callback to add up all count values in the vector*/
static void CC num_gen_total_count_cb( void *item, void *data )
{
    num_gen_node * node = item;
    if ( node != NULL )
    {
        uint64_t * total = ( uint64_t *)data;
        if ( total != NULL )
            *total += node -> count;
    }
}


/* helper function that adds up all count values in the vector*/
static uint64_t num_gen_total_count( const Vector * src )
{
    uint64_t res = 0;
    if ( src != NULL )
        VectorForEach ( src, false, num_gen_total_count_cb, &res );
    return res;
}


/* helper function for the parse-function */
static rc_t num_gen_add_node( struct num_gen * self, const int64_t from, const int64_t to )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC( rcVDB, rcNoTarg, rcInserting, rcSelf, rcNull );
    else
    {
        num_gen_node * node;
        if ( from < to )
            node = num_gen_make_node( from, ( to - from ) + 1 );
        else
            node = num_gen_make_node( to, ( from - to ) + 1 );

        if ( node == NULL )
            rc = RC( rcVDB, rcNoTarg, rcInserting, rcMemory, rcExhausted );
        else
        {
            if ( self->sorted )
                rc = VectorInsert( &( self -> nodes ), node, NULL, num_gen_insert_helper );
            else
                rc = VectorAppend( &( self -> nodes ), NULL, node );
        }
    }
    return rc;
}


#define MAX_NUM_STR 12
/* helper-structure for num_gen_parse() */
typedef struct num_gen_parse_ctx
{
    uint32_t num_str_idx;
    bool this_is_the_first_number;
    int64_t num1;
    int64_t num2;
    char num_str[ MAX_NUM_STR + 1 ];
} num_gen_parse_ctx;


/* helper for num_gen_parse() */
static void num_gen_convert_ctx( num_gen_parse_ctx * parse_ctx )
{
    if ( parse_ctx != NULL )
    {
        char * endp;
        
        parse_ctx -> num_str[ parse_ctx -> num_str_idx ] = 0;
        parse_ctx -> num1 = strtoi64( parse_ctx -> num_str, &endp, 10 );
        parse_ctx -> this_is_the_first_number = false;
        parse_ctx -> num_str_idx = 0;
    }
}


/* helper for num_gen_parse() */
static rc_t num_gen_convert_and_add_ctx( struct num_gen * self, num_gen_parse_ctx * parse_ctx )
{
    rc_t rc;
    if ( self == NULL )
        rc = RC( rcVDB, rcNoTarg, rcInserting, rcSelf, rcNull );
    else if ( parse_ctx == NULL )
        rc = RC( rcVDB, rcNoTarg, rcInserting, rcParam, rcNull );
    else if ( parse_ctx -> num_str_idx == 0 )
        rc = RC( rcVDB, rcNoTarg, rcInserting, rcParam, rcEmpty );
    else
    {
        char *endp;

        /* terminate the source-string */
        parse_ctx -> num_str[ parse_ctx -> num_str_idx ] = 0;

        /* convert the string into a uint64_t */
        if ( parse_ctx -> this_is_the_first_number )
        {
            parse_ctx -> num1 = strtoi64( parse_ctx -> num_str, &endp, 10 );
            parse_ctx -> num2 = parse_ctx -> num1;
        }
        else
        {
            parse_ctx -> num2 = strtoi64( parse_ctx -> num_str, &endp, 10 );
        }

        /* empty the source-string to be reused */
        parse_ctx -> num_str_idx = 0;
        
        parse_ctx -> this_is_the_first_number = true;
        rc = num_gen_add_node( self, parse_ctx -> num1, parse_ctx -> num2 );
    }
    return rc;
}


static rc_t num_gen_parse_len( struct num_gen * self, const char * src, size_t len )
{
    rc_t rc = 0;

    if ( len == 0 )
        rc = RC( rcVDB, rcNoTarg, rcParsing, rcParam, rcEmpty );
    else
    {
        size_t i;
        num_gen_parse_ctx parse_ctx;

        parse_ctx . num_str_idx = 0;
        parse_ctx . this_is_the_first_number = true;

        for ( i = 0; i < len && rc == 0; ++i )
        {
            switch ( src[ i ] )
            {
            /* a dash switches from N1-mode into N2-mode */
            case '-' :
                num_gen_convert_ctx( &parse_ctx );
                break;

            /* a comma ends a single number or a range */
            case ',' :
                rc = num_gen_convert_and_add_ctx( self, &parse_ctx );
                break;

            /* in both mode add the char to the temp string */
            default:
                if ( ( src[ i ] >= '0' ) && ( src[ i ] <= '9' )&&
                     ( parse_ctx . num_str_idx < MAX_NUM_STR ) )
                    parse_ctx . num_str[ parse_ctx . num_str_idx ++ ] = src[ i ];
                break;
            }
        }

        /* dont forget to add what is left in ctx.num_str ... */
        if ( parse_ctx . num_str_idx > 0 )
            rc = num_gen_convert_and_add_ctx( self, &parse_ctx );

        if ( rc == 0 && self->sorted )
            rc = num_gen_fix_overlaps( self, NULL );
    }
    return rc;
}


/* parse the given string and insert the found ranges 
   into the number-generator, fixes eventual overlaps */
LIB_EXPORT rc_t CC num_gen_parse( struct num_gen * self, const char * src )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC( rcVDB, rcNoTarg, rcParsing, rcSelf, rcNull );
    else if ( src == NULL )
        rc = RC( rcVDB, rcNoTarg, rcParsing, rcParam, rcNull );
    else if ( src[ 0 ] == 0 )
        rc = RC( rcVDB, rcNoTarg, rcParsing, rcParam, rcEmpty );
    else
        rc = num_gen_parse_len( self, src, string_measure ( src, NULL ) );

    return rc;
}


LIB_EXPORT rc_t CC num_gen_parse_S( struct num_gen * self, const String * src )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC( rcVDB, rcNoTarg, rcParsing, rcSelf, rcNull );
    else if ( src == NULL )
        rc = RC( rcVDB, rcNoTarg, rcParsing, rcParam, rcNull );
    else if ( src->len == 0 )
        rc = RC( rcVDB, rcNoTarg, rcParsing, rcParam, rcEmpty );
    else
        rc = num_gen_parse_len( self, src->addr, src->len );

    return rc;
}


/* inserts the given ranges into the number-generator,
   fixes eventual overlaps */
LIB_EXPORT rc_t CC num_gen_add( struct num_gen * self, const int64_t first, const uint64_t count )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC( rcVDB, rcNoTarg, rcInserting, rcSelf, rcNull );
    if ( count == 0 )
        rc = RC( rcVDB, rcNoTarg, rcInserting, rcParam, rcEmpty );
    else
    {
        rc = num_gen_add_node( self, first, ( first + count ) - 1 );
        if ( rc == 0 && self->sorted )
            rc = num_gen_fix_overlaps( self, NULL );
    }
    return rc;
}


/* helper function for range-check */
static bool CC num_gen_check_range_start( num_gen_node * node, const int64_t range_start )
{
    bool res = ( node != NULL );
    if ( res )
    {
        int64_t last = ( node -> start + node -> count ) - 1;
        
        if ( node -> start < range_start )
        {
            node -> start = range_start;
            if ( node -> start <= last )
                node -> count = ( last - ( node -> start ) ) + 1;
            else
            {
                /* the node becomes invalid ... */
                node -> start = 0;
                node -> count = 0;
                res = false;
            }
        }
    }
    return res;
}


/* helper function for range-check */
static void CC num_gen_check_range_end( num_gen_node * node, const int64_t last )
{
    if ( node != NULL )
    {
        int64_t node_last = ( node -> start + node -> count ) - 1;

        if ( node_last > last )
        {
            node_last = last;
            if ( node -> start <= node_last )
            {
                node->count = ( node_last - ( node -> start ) ) + 1;
            }
            else
            {
                /* the node becomes invalid ... */
                node -> start = 0;
                node -> count = 0;
            }
        }
    }
}


/* helper function for range-check */
static void CC num_gen_check_range_callback( void * item, void * data )
{
    num_gen_node * node   = item;
    num_gen_node * range  = data;

    if ( node != NULL && range != NULL )
    {
        /* ignore invalid nodes... */
        if ( node -> start != 0 && node -> count != 0 )
        {
            /* check if the start value is not out of range... */
            if ( num_gen_check_range_start( node, range -> start ) )
            {
                uint64_t last = ( ( range -> start ) + ( range -> count ) ) - 1;
                num_gen_check_range_end( node, last );
            }
        }
    }
}


/* helper function for range-check */
static void CC num_gen_count_invalid_nodes( void * item, void * data )
{
    num_gen_node * node = item;
    
    if ( ( node != NULL ) && ( node -> start == 0 ) && ( node -> count == 0 ) )
    {
        uint32_t * invalid_count = data;
        if ( invalid_count != NULL )
        {
            ( *invalid_count )++;
        }
    }
}


/* helper function for range-check */
static void CC num_gen_copy_valid_nodes( void * item, void * data )
{
    num_gen_node * node = item;

    if ( node != NULL )
    {
        if ( ( node -> start != 0 )&&( node -> count != 0 ) )
        {
            Vector * dest = data;
            if ( dest != NULL )
                VectorInsert ( dest, node, NULL, num_gen_insert_helper );
        }
        else
            free ( node );
    }
}


/* helper function for range-check */
static void num_gen_remove_invalid_nodes( struct num_gen * self )
{
    if ( self != NULL )
    {
        uint32_t count = VectorLength( &( self -> nodes ) );
        if ( count > 0 )
        {
            Vector temp_nodes;
            /* create a temp. vector */
            VectorInit( &temp_nodes, 0, count );

            /* copy all valid nodes into the temp. vector */
            VectorForEach ( &( self -> nodes ), false,
                            num_gen_copy_valid_nodes, &temp_nodes );

            /* clear all nodes so far...,
               DO NOT PASS num_gen_node_destroy into it */
            VectorWhack( &( self -> nodes ), NULL, NULL );

            /* initialize and copy (shallow) the valid nodes back
               into the generator */
            VectorCopy ( &temp_nodes, &( self -> nodes ) );

            /* destroy the temp-vector,
               DO NOT PASS num_gen_node_destroy into it */
            VectorWhack ( &temp_nodes, NULL, NULL );
        }
    }
}


/* helper function for trim */
LIB_EXPORT rc_t CC num_gen_trim( struct num_gen * self, const int64_t first, const uint64_t count )
{
    rc_t rc = 0;

    if ( self == NULL )
        rc = RC( rcVDB, rcNoTarg, rcValidating, rcSelf, rcNull );
    else
    {
        num_gen_node trim_range;
        uint32_t invalid_nodes = 0;

        /* walk all nodes to check for boundaries... */
        trim_range.start = first;
        trim_range.count = count;

        VectorForEach ( &( self -> nodes), false,
                        num_gen_check_range_callback, &trim_range );

        VectorForEach ( &( self -> nodes ), false,
                        num_gen_count_invalid_nodes, &invalid_nodes );

        if ( invalid_nodes > 0 )
            num_gen_remove_invalid_nodes( self );
    }

    return rc;
}


LIB_EXPORT rc_t CC num_gen_make( struct num_gen ** self )
{
    rc_t rc = 0;

    if ( self == NULL )
        rc = RC( rcVDB, rcNoTarg, rcConstructing, rcSelf, rcNull );
    else
    {
        struct num_gen * ng = calloc( 1, sizeof( * ng ) );
        if ( ng == NULL )
            rc = RC( rcVDB, rcNoTarg, rcConstructing, rcMemory, rcExhausted );
        else
        {
            VectorInit( &( ng -> nodes ), 0, 5 );
            ng->sorted = false;
            *self = ng;
        }
    }
    return rc;
}


LIB_EXPORT rc_t CC num_gen_make_sorted( struct num_gen ** self, bool sorted )
{
    rc_t rc = 0;

    if ( self == NULL )
        rc = RC( rcVDB, rcNoTarg, rcConstructing, rcSelf, rcNull );
    else
    {
        struct num_gen * ng = calloc( 1, sizeof( * ng ) );
        if ( ng == NULL )
            rc = RC( rcVDB, rcNoTarg, rcConstructing, rcMemory, rcExhausted );
        else
        {
            VectorInit( &( ng -> nodes ), 0, 5 );
            ng->sorted = sorted;
            *self = ng;
        }
    }
    return rc;
}


LIB_EXPORT rc_t CC num_gen_make_from_str( struct num_gen ** self, const char * src )
{
    rc_t rc = 0;
    if ( self == NULL )
        rc = RC( rcVDB, rcNoTarg, rcConstructing, rcSelf, rcNull );
    else if ( src == NULL || src[ 0 ] == 0 )
    {
        *self = NULL;
        rc = RC( rcVDB, rcNoTarg, rcConstructing, rcSelf, rcNull );
    }
    else
    {
        struct num_gen * temp;
        rc = num_gen_make( &temp );
        if ( rc == 0 )
        {
            rc = num_gen_parse( temp, src );
        }
        if ( rc == 0 )
            *self = temp;
        else
        {
            *self = NULL;
            num_gen_destroy( temp );
        }
    }
    return rc;
}


LIB_EXPORT rc_t CC num_gen_make_from_str_sorted( struct num_gen ** self, const char * src, bool sorted )
{
    rc_t rc = 0;
    if ( self == NULL )
        rc = RC( rcVDB, rcNoTarg, rcConstructing, rcSelf, rcNull );
    else if ( src == NULL || src[ 0 ] == 0 )
    {
        *self = NULL;
        rc = RC( rcVDB, rcNoTarg, rcConstructing, rcSelf, rcNull );
    }
    else
    {
        struct num_gen * temp;
        rc = num_gen_make_sorted( &temp, sorted );
        if ( rc == 0 )
        {
            rc = num_gen_parse( temp, src );
        }
        if ( rc == 0 )
            *self = temp;
        else
        {
            *self = NULL;
            num_gen_destroy( temp );
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC num_gen_make_from_range( struct num_gen ** self, int64_t first, uint64_t count )
{
    rc_t rc = 0;
    if ( self == NULL )
        rc = RC( rcVDB, rcNoTarg, rcConstructing, rcSelf, rcNull );
    else if ( count == 0 )
    {
        *self = NULL;
        rc = RC( rcVDB, rcNoTarg, rcConstructing, rcParam, rcEmpty );
    }
    else
    {
        struct num_gen * temp;
        rc = num_gen_make( &temp );
        if ( rc == 0 )
            rc = num_gen_add( temp, first, count );
        if ( rc == 0 )
            *self = temp;
        else
        {
            *self = NULL;
            num_gen_destroy( temp );
        }
    }
    return rc;
}


LIB_EXPORT rc_t CC num_gen_clear( struct num_gen * self )
{
    rc_t rc = 0;
    if ( self == NULL )
        rc = RC( rcVDB, rcNoTarg, rcClearing, rcSelf, rcNull );
    else if ( VectorLength( &( self -> nodes ) ) > 0 )
    {
        /* clear all nodes so far... */
        VectorWhack( &( self -> nodes ), num_gen_node_destroy, NULL );

        /* re-init the vector */
        VectorInit( &( self->nodes ), 0, 5 );
    }
    return rc;
}


LIB_EXPORT rc_t CC num_gen_destroy( struct num_gen * self )
{
    rc_t rc = 0;
    if ( self == NULL )
        rc = RC( rcVDB, rcNoTarg, rcDestroying, rcSelf, rcNull );
    else
    {
        VectorWhack( &( self -> nodes ), num_gen_node_destroy, NULL );
        free( self );
    }
    return rc;
}


LIB_EXPORT bool CC num_gen_empty( const struct num_gen * self )
{
    bool res = ( self != NULL );
    if ( res )
        res = ( VectorLength( &( self -> nodes ) ) < 1 );
    return res;
}


typedef struct overlap_ctx
{
    num_gen_node * prev;
    uint32_t overlaps;
} overlap_ctx;


/* static bool CC num_gen_overlap_fix_cb( void *item, void *data ) */
static bool CC num_gen_overlap_fix_cb( void *item, void *data )
{
    bool res = false;
    num_gen_node * node = item;
    if ( node != NULL )
    {
        /* skip invalid nodes */
        if ( node -> count > 0 && node -> start > 0 )
        {
            overlap_ctx * ov_ctx = data;
            if ( ov_ctx != NULL )
            {
                /* if we do not have a previous node, take this one... */
                if ( ov_ctx -> prev == NULL )
                {
                    ov_ctx -> prev = node;
                }
                else
                {
                    int64_t prev_last = ( ov_ctx -> prev -> start + ov_ctx -> prev -> count ) - 1;

                    /* if we do not have an overlap, take this node as prev-node and continue */
                    if ( prev_last < node -> start )
                    {
                        ov_ctx -> prev = node;
                    }
                    else
                    {
                        /* we have a overlap, the end of the prev-node is inside
                           the current-node, we fix it by expanding the prev-node
                           to the end of this node, and later declaring this
                           node as invalid */
                        int64_t this_last = ( node -> start + node -> count ) - 1;
                        if ( prev_last < this_last )
                        {
                            ov_ctx -> prev -> count = ( prev_last - ( ov_ctx -> prev -> start ) ) + 1;
                        }
                        else
                        {
                            /* if the prev-node ends after this node, all we have to
                               do is declaring this node as invalid */
                            node->count = 0;
                            node->start = 0;
                        }
                        res = true;
                    }
                }
            }
        }
    }
    return res;
}


static rc_t num_gen_fix_overlaps( struct num_gen * self, uint32_t * count )
{
    rc_t rc = 0;

    if ( self == NULL )
        rc = RC( rcVDB, rcNoTarg, rcReading, rcSelf, rcNull );
    else
    {
        overlap_ctx ov_ctx;
        bool fix_executed = false;
        
        ov_ctx . overlaps = 0;
        do
        {
            ov_ctx . prev = NULL;
            fix_executed = VectorDoUntil ( &( self -> nodes ), false,
                                           num_gen_overlap_fix_cb, &ov_ctx );
        } while ( fix_executed );

        if ( count )
            *count = ov_ctx . overlaps;
    }
    return rc;
}


typedef struct string_ctx
{
    char * buffer;
    size_t buffsize, written;
} string_ctx;


static void CC num_gen_as_string_cb( void * item, void * data )
{
    num_gen_node * node = item;
    if ( node != NULL )
    {
        string_ctx * str_ctx = data;
        if ( str_ctx != NULL && node -> count > 0 )
        {
            size_t bsize = str_ctx -> buffsize - str_ctx -> written;
            if ( bsize > 0 )
            {
                rc_t rc;
                char * buf = str_ctx -> buffer + str_ctx -> written;
                size_t written;
                if ( node -> count == 1 )
                {
                    rc = string_printf ( buf, bsize, &written, "%ld,", node -> start );
                }
                else
                {
                    uint64_t last = ( ( node -> start ) + ( node -> count ) ) - 1;
                    rc = string_printf ( buf, bsize, &written, "%ld-%ld,", node -> start, last );
                }
                if ( rc == 0 )
                    str_ctx -> written += written;
            }
        }
    }
}


static void CC num_gen_as_string_full_info_cb( void * item, void * data )
{
    num_gen_node * node = item;
    if ( node != NULL )
    {
        string_ctx * str_ctx = data;
        if ( str_ctx != NULL )
        {
            size_t bsize = str_ctx -> buffsize - str_ctx -> written;
            char * buf = str_ctx -> buffer + str_ctx -> written;
            size_t written;
            rc_t rc = string_printf ( buf, bsize, &written, "[s:%lu c:%lu]", node -> start, node -> count );
            if ( rc == 0 )
                str_ctx -> written += written;
        }
    }
}


LIB_EXPORT rc_t CC num_gen_as_string( const struct num_gen * self, char * buffer, size_t buffsize, size_t * written, bool full_info )
{
    rc_t rc = 0;

    if ( self == NULL )
        rc = RC( rcVDB, rcNoTarg, rcReading, rcSelf, rcNull );
    else if ( buffer == NULL || buffsize == 0 )
        rc = RC( rcVDB, rcNoTarg, rcReading, rcParam, rcNull );
    else
    {
        string_ctx str_ctx;
    
        str_ctx . buffer = buffer;
        str_ctx . buffsize = buffsize;
        str_ctx . written = 0;

        if ( full_info )
            VectorForEach ( &( self -> nodes ), false, num_gen_as_string_full_info_cb, &str_ctx );
        else
            VectorForEach ( &( self -> nodes ), false, num_gen_as_string_cb, &str_ctx );

        if ( written != NULL )
            *written = str_ctx . written;
    }
    return rc;
}


static bool CC num_gen_contains_cb( void * item, void * data )
{
    bool res = false;
    num_gen_node * node = item;
    if ( node != NULL && node->count > 0 )
    {
        int64_t * value = data;
        if ( value != NULL )
        {
            int64_t end = ( node -> start + node -> count ) - 1;
            res = ( node -> start <= *value && *value <= end );
        }
    }
    return res;
}


LIB_EXPORT rc_t CC num_gen_contains_value( const struct num_gen * self, const int64_t value )
{
    rc_t rc = 0;

    if ( self == NULL )
        rc = RC( rcVDB, rcNoTarg, rcReading, rcSelf, rcNull );
    else
    {
        int64_t temp = value;
        if ( !VectorDoUntil ( &( self -> nodes ), false, num_gen_contains_cb, &temp ) )
            rc = RC( rcVDB, rcNoTarg, rcReading, rcData, rcEmpty );
    }
    return rc;
}


LIB_EXPORT rc_t CC num_gen_range_check( struct num_gen * self, const int64_t first, const uint64_t count )
{
    rc_t rc;
    /* if the user did not specify a range, take all */
    if ( num_gen_empty( self ) )
        rc = num_gen_add( self, first, count );
    /* if the user did specify a range, check the boundaries */
    else
        rc = num_gen_trim( self, first, count );
    return rc;
}


LIB_EXPORT rc_t CC num_gen_copy( const struct num_gen * self, struct num_gen ** dest )
{
    rc_t rc = num_gen_make( dest );
    if ( rc == 0 )
    {
        num_gen_copy_vector( &( self -> nodes ), &( ( *dest ) -> nodes ), self->sorted );
    }
    return rc;
}


static int64_t min_vector_value( Vector * v )
{
    int64_t res = 0;
    num_gen_node * node = VectorGet( v, 0 );
    if ( node != NULL )
        res = node -> start;
    return res;
}


static int64_t max_vector_value( Vector * v )
{
    int64_t res = 0;
    uint32_t count = VectorLength( v );
    if ( count > 0 )
    {
        num_gen_node * node = VectorGet( v, count - 1 );
        if ( node != NULL )
            res = ( node -> start + node -> count ) - 1;
    }
    return res;
}


LIB_EXPORT rc_t CC num_gen_iterator_make( const struct num_gen * self, const struct num_gen_iter ** iter )
{
    rc_t rc = 0;

    if ( self == NULL )
        rc = RC( rcVDB, rcNoTarg, rcReading, rcSelf, rcNull );
    else if ( iter == NULL )
        rc = RC( rcVDB, rcNoTarg, rcReading, rcParam, rcNull );
    else
    {
        struct num_gen_iter * temp = calloc( 1, sizeof( * temp ) );
        *iter = NULL;
        if ( temp == NULL )
            rc = RC( rcVDB, rcNoTarg, rcConstructing, rcMemory, rcExhausted );
        else
        {
            uint32_t count = VectorLength( &( self -> nodes ) );
            if ( count == 0 )
            {
                VectorInit( &( temp -> nodes ), 0, 5 );
                /* everything else is set to zero, because of calloc() above */
            }
            else
            {
                VectorInit( &( temp -> nodes ), 0, count );
                num_gen_copy_vector( &( self -> nodes ), &( temp -> nodes ), self->sorted );
                temp -> total = num_gen_total_count( &( temp -> nodes ) );
                temp -> min_value = min_vector_value( &( temp -> nodes ) );
                temp -> max_value = max_vector_value( &( temp -> nodes ) );
            }
            *iter = temp;
        }
    }
    return rc;
}


LIB_EXPORT rc_t CC num_gen_iterator_destroy( const struct num_gen_iter *self )
{
    rc_t rc = 0;

    if ( self == NULL )
        rc = RC( rcVDB, rcNoTarg, rcDestroying, rcSelf, rcNull );
    else
    {
        struct num_gen_iter * temp = ( struct num_gen_iter * )self;
        VectorWhack( &( temp -> nodes ), num_gen_node_destroy, NULL );
        free( temp );
    }
    return rc;
}


LIB_EXPORT bool CC num_gen_iterator_next( const struct num_gen_iter * self, int64_t * value, rc_t * rc )
{
    bool res = false;
    if ( self == NULL )
    {
        if ( rc != NULL )
            *rc = RC( rcVDB, rcNoTarg, rcReading, rcSelf, rcNull );
    }
    else if ( value == NULL )
    {
        if ( rc != NULL )
            *rc = RC( rcVDB, rcNoTarg, rcReading, rcParam, rcNull );
    }
    else if ( self -> curr_node < VectorLength( &( self -> nodes ) ) )
    {
        *value = 0;
        {
            struct num_gen_iter * temp = ( struct num_gen_iter * )self;
            num_gen_node * node = VectorGet( &( temp -> nodes ), temp -> curr_node );
            if ( node == NULL )
            {
                if ( rc != NULL )
                    *rc = RC( rcVDB, rcNoTarg, rcReading, rcItem, rcInvalid );
            }
            else
            {
                if ( node -> count < 2 )
                {
                    /* the node is a single-number-node, next node for next time */
                    *value = node -> start;
                    ( temp -> curr_node )++;
                }
                else
                {
                    /* the node is a number range, add the sub-position */
                    *value = node -> start + temp -> curr_node_sub_pos;
                    ( temp -> curr_node_sub_pos )++;
                    /* if the sub-positions are used up, switch to next node */
                    if ( temp -> curr_node_sub_pos >= node -> count )
                    {
                        ( temp -> curr_node )++;
                        temp -> curr_node_sub_pos = 0;
                    }
                }
                ( temp -> progress )++;
                res = true;
            }
        }
    }
    return res;
}


LIB_EXPORT rc_t CC num_gen_iterator_count( const struct num_gen_iter * self, uint64_t * count )
{
    rc_t rc = 0;
    if ( self == NULL )
        rc = RC( rcVDB, rcNoTarg, rcReading, rcSelf, rcNull );
    else if ( count == NULL )
        rc = RC( rcVDB, rcNoTarg, rcReading, rcParam, rcNull );
    else
        *count = self -> total;
    return rc;
}


LIB_EXPORT rc_t CC num_gen_iterator_min( const struct num_gen_iter * self, int64_t * value )
{
    rc_t rc = 0;
    if ( self == NULL )
        rc = RC( rcVDB, rcNoTarg, rcReading, rcSelf, rcNull );
    else if ( value == NULL )
        rc = RC( rcVDB, rcNoTarg, rcReading, rcParam, rcNull );
    else
        *value = self -> min_value;
    return rc;
}


LIB_EXPORT rc_t CC num_gen_iterator_max( const struct num_gen_iter * self, int64_t * value )
{
    rc_t rc = 0;
    if ( self == NULL )
        rc = RC( rcVDB, rcNoTarg, rcReading, rcSelf, rcNull );
    else if ( value == NULL )
        rc = RC( rcVDB, rcNoTarg, rcReading, rcParam, rcNull );
    else
        *value = self -> max_value;
    return rc;
}


LIB_EXPORT rc_t CC num_gen_iterator_percent( const struct num_gen_iter * self, uint8_t fract_digits, uint32_t * value )
{
    rc_t rc = 0;

    if ( self == NULL )
        rc = RC( rcVDB, rcNoTarg, rcReading, rcSelf, rcNull );
    else if ( value == NULL )
        rc = RC( rcVDB, rcNoTarg, rcReading, rcParam, rcNull );
    else
    {
        uint32_t factor = 100;
        if ( fract_digits > 0 )
        {
            if ( fract_digits > 1 )
                factor = 10000;
            else
                factor = 1000;
        }
            
        if ( self -> total > 0 )
        {
            if ( self -> progress >= self -> total )
                *value = factor;
            else
            {
                uint64_t temp = self -> progress;
                temp *= factor;
                temp /= self -> total;
                *value = ( uint16_t ) temp;
            }
        }
        else
            *value = 0;
    }
    return rc;
}
