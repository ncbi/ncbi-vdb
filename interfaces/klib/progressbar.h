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

#ifndef _h_progressbar_
#define _h_progressbar_

#ifndef _h_klib_extern_
#include <klib/extern.h>
#endif

#ifndef _h_klib_rc_
#include <klib/rc.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif

struct progressbar;
typedef uint32_t percent_t;

/*--------------------------------------------------------------------------
 * make_progressbar
 *
 *  creates a progressbar with zero-values inside
 *  does not output anything
 *  digits: 0  ... update_progressbar( pb, 7 ) 		---> 7%
 *          1  ... update_progressbar( pb, 71 )		---> 7.1%
 *          2  ... update_progressbar( pb, 715 )	---> 7.15%
 *  digits > 2 are internally stored as 2
 */
 
 /*
    writes to stdout, but only if stdout is a terminal ( not redirected to a file/pipe )
 */
KLIB_EXTERN rc_t CC make_progressbar( struct progressbar ** pb, const uint8_t digits );


 /*
    writes to stderr, but only if stderr is a terminal ( not redirected to a file/pipe )
 */
KLIB_EXTERN rc_t CC make_progressbar_stderr( struct progressbar ** pb, const uint8_t digits );

/*--------------------------------------------------------------------------
 * destroy_progressbar
 *
 *  destroy's the progressbar
 *  does not output anything
 */
KLIB_EXTERN rc_t CC destroy_progressbar( struct progressbar * pb );


/*--------------------------------------------------------------------------
 * update_progressbar
 *
 *  sets the progressbar to a specific percentage
 *  outputs only if the percentage has changed from the last call
 *  the meaning of the percent-value depends of the given didits value to make_progressbar()
 *		digits = 0		percent in full percent				 7 ---> 7%
 *      digits = 1		precent in 1/10-th of a percent 	71 ---> 7,1%
 *      digits = 2		precent in 1/100-th of a percent   715 ---> 7,15%
 *  expects the percents in increasing order ( does not jump back )
 *  writes a growing bar made from '-'-chars with the value at the end
 */
KLIB_EXTERN rc_t CC update_progressbar( struct progressbar * pb, const percent_t percent );

#ifdef __cplusplus
}
#endif

#endif /* _h_progressbar_ */
