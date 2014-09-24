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

#ifndef _h_compiler_
#define _h_compiler_

#include <math.h>
#include <float.h>

#ifdef __cplusplus
extern "C" {
#endif

static __inline
float roundf( float value_in )
{
    return ( value_in < 0 ) ? ceilf ( value_in - 0.5F ) : floorf ( value_in + 0.5F );
}

#define lroundf( x ) (long int)roundf( x )

static __inline
double round( double value_in )
{
    return ( value_in < 0 ) ? ceil ( value_in - 0.5 ) : floor ( value_in + 0.5 );
}

typedef double double_t;

#define isnan( x ) _isnan ( x )

static __inline
float truncf( float value_in )
{
	return ( value_in < 0 ) ? ceilf( value_in ) : floorf( value_in );
}

static __inline
double trunc( double value_in )
{
	return ( value_in < 0 ) ? ceil( value_in ) : floor( value_in );
}

#ifdef __cplusplus
}
#endif

#endif /* _h_compiler_ */

