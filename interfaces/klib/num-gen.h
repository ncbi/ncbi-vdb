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

#ifndef _h_klib_num_gen_
#define _h_klib_num_gen_

#ifndef _h_klib_extern_
#include <klib/extern.h>
#endif

#ifndef _h_klib_rc_
#include <klib/rc.h>
#endif

#ifndef _h_klib_text_
#include <klib/text.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------
 * A NUMBER GENERATOR
 * 
 *  input : string, for instance "3,6,8,12,44-49"
 *  ouptut: sequence of integers, for instance 3,6,8,12,44,45,46,47,48,49
 */


/*--------------------------------------------------------------------------
 * opaque number-generator and it's iterator
 */
struct num_gen;
struct num_gen_iter;


/*--------------------------------------------------------------------------
 * num_gen_make
 *
 *  creates a empty number-generator
 *  or creates a number-generator and parses the string
 *  or creates and presets it with a range
 */
KLIB_EXTERN rc_t CC num_gen_make( struct num_gen ** self );
KLIB_EXTERN rc_t CC num_gen_make_sorted( struct num_gen ** self, bool sorted );
KLIB_EXTERN rc_t CC num_gen_make_from_str( struct num_gen ** self, const char * src );
KLIB_EXTERN rc_t CC num_gen_make_from_str_sorted( struct num_gen ** self, const char * src, bool sorted );
KLIB_EXTERN rc_t CC num_gen_make_from_range( struct num_gen ** self, int64_t first, uint64_t count );
KLIB_EXTERN rc_t CC num_gen_copy( const struct num_gen * self, struct num_gen ** dest );

/*--------------------------------------------------------------------------
 * num_gen_destroy
 *
 *  destroys a number-generator
 */
KLIB_EXTERN rc_t CC num_gen_destroy( struct num_gen * self );


/*--------------------------------------------------------------------------
 * num_gen_clear
 *
 *  resets a number-generator, to be empty just like after num_gen_make()
 */
KLIB_EXTERN rc_t CC num_gen_clear( struct num_gen * self );


/*--------------------------------------------------------------------------
 * num_gen_parse
 *
 *  parses a given string in this form: "3,6,8,12,44-49"
 *  does not clear the number-generator before parsing
 *  eventual overlaps with the previous content are consolidated
 */
KLIB_EXTERN rc_t CC num_gen_parse( struct num_gen * self, const char * src );
KLIB_EXTERN rc_t CC num_gen_parse_S( struct num_gen * self, const String * src );

/*--------------------------------------------------------------------------
 * num_gen_add
 *
 *  inserts the given interval into the number-generator
 *
 *  num_gen_add( *g, 10, 30 )
 *  is equivalent to:
 *  num_gen_parse( *g, "10-39" );
 *
 *  eventual overlaps with the previous content are consolidated 
 */
KLIB_EXTERN rc_t CC num_gen_add( struct num_gen * self, const int64_t first, const uint64_t count );


/*--------------------------------------------------------------------------
 * num_gen_trim
 *
 *  checks if the content of the number-generator is inside the given interval
 *  removes or shortens internal nodes if necessary
 */
KLIB_EXTERN rc_t CC num_gen_trim( struct num_gen * self, const int64_t first, const uint64_t count );


/*--------------------------------------------------------------------------
 * num_gen_empty
 *
 *  checks if the generator has no ranges defined
 */
KLIB_EXTERN bool CC num_gen_empty( const struct num_gen * self );


/*--------------------------------------------------------------------------
 * num_gen_as_string
 *
 *  prints the content of the num_gen into the buffer
 *  *s = "1-5,20,24-25"
 */
KLIB_EXTERN rc_t CC num_gen_as_string( const struct num_gen * self, char * buffer, size_t buffsize,
                                        size_t * written, bool full_info );


/*--------------------------------------------------------------------------
 * num_gen_contains_value
 *
 *  checks if the generator contains the given value
 */
KLIB_EXTERN rc_t CC num_gen_contains_value( const struct num_gen * self, const int64_t value );


/*--------------------------------------------------------------------------
 * num_gen_range_check
 *
 *  if the generator is empty --> set it to the given range
 *  if it is not empty ---------> trim it to the given range
 */
KLIB_EXTERN rc_t CC num_gen_range_check( struct num_gen * self, const int64_t first, const uint64_t count );

/*--------------------------------------------------------------------------
 * num_gen_iterator_make
 *
 *  creates a iterator from the number-generator
 *  the iterator contains a constant copy of the number-ranges
 *  after this call it is safe to destroy or change the number-generator
 *  returns an error-code if the number-generator was empty,
 *  and *iter will be NULL
 */
KLIB_EXTERN rc_t CC num_gen_iterator_make( const struct num_gen * self, const struct num_gen_iter ** iter );


/*--------------------------------------------------------------------------
 * num_gen_iterator_destroy
 *
 *  destroys the iterator
 */
KLIB_EXTERN rc_t CC num_gen_iterator_destroy( const struct num_gen_iter * self );


/*--------------------------------------------------------------------------
 * num_gen_iterator_count
 *
 *  returns how many values the iterator contains
 */
KLIB_EXTERN rc_t CC num_gen_iterator_count( const struct num_gen_iter * self, uint64_t * count );


/*--------------------------------------------------------------------------
 * num_gen_iterator_next
 *
 *  pulls the next value out of the iterator...
 *  returns an error-code if the iterator has no more values
 */
KLIB_EXTERN bool CC num_gen_iterator_next( const struct num_gen_iter * self, int64_t * value, rc_t * rc );

/*--------------------------------------------------------------------------
 * num_gen_iterator_min
 *
 *  pulls the lowest value out of the iterator...
 *  returns an error-code if the iterator has no more values
 */
KLIB_EXTERN rc_t CC num_gen_iterator_min( const struct num_gen_iter * self, int64_t * value );


/*--------------------------------------------------------------------------
 * num_gen_iterator_max
 *
 *  pulls the highest value out of the iterator...
 *  returns an error-code if the iterator has no more values
 */
KLIB_EXTERN rc_t CC num_gen_iterator_max( const struct num_gen_iter * self, int64_t * value );


/*--------------------------------------------------------------------------
 * num_gen_iterator_percent
 *
 *  return in value the percentage of the iterator...
 *  depending on fract-digits the percentage will be:
 *      fract_digits = 0 ... full percent's
 *      fract_digits = 1 ... 1/10-th of a percent
 *      fract_digits = 2 ... 1/100-th of a percent
 */
KLIB_EXTERN rc_t CC num_gen_iterator_percent( const struct num_gen_iter * self, uint8_t fract_digits, uint32_t * value );


#ifdef __cplusplus
}
#endif

#endif /* _h_klib_num_gen_ */
