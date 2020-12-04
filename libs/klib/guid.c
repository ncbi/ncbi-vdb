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

#include <klib/guid.h>

#include <time.h>

const size_t GuidSize = 36;
const char * Uint4ToHex = "0123456789abcdef";

LIB_EXPORT rc_t CC KGUIDMake( char * buf, size_t bufSize )
{
    if ( buf == NULL )
    {
        return RC ( rcRuntime, rcString, rcCreating, rcParam, rcNull );
    }
    if ( bufSize < GuidSize + 1 ) /* includes the 0 terminator */
    {
        return RC ( rcRuntime, rcString, rcCreating, rcParam, rcTooShort );
    }

    srand ( time ( NULL ) );

    for (unsigned int i=0; i < GuidSize + 1; ++i)
    {
        switch ( i )
        {
        case 8:
        case 13:
        case 18:
        case 23:
            buf[ i ] = '-';
            break;
        case 14:
            buf[ i ] = '4';
            break;
        case 19:
            /* variant 10 in the 2 most significant bits*/
            buf[ i ] =  Uint4ToHex [ 8 | ( rand () % 4 ) ];
            break;
        default:
            buf[ i ] =  Uint4ToHex [ rand () % 16 ];
            break;
        }
    }
    buf[ GuidSize ] = 0;

    return 0;
}

