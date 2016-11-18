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


#include <klib/time.h> /* KSleep */


LIB_EXPORT const KTime* CC KTimeIso8601 ( KTime *kt, const char * s )
{
    int i = 0;
    int tmp = 0;
    char c = 0;

    if ( kt == NULL || s == NULL )
        return NULL;

    memset ( kt, 0, sizeof * kt );

    for ( i = 0, tmp = 0; i < 4; ++ i ) {
        char c = s [ i ];
        if ( ! isdigit ( c ) )
            return NULL;
        tmp = tmp * 10 + c - '0';
    }
    if ( tmp < 1900 )
        return NULL;
    kt -> year = tmp - 1900;

    if ( s [ i ] != '-' )
        return NULL;

    c = s [ ++ i ];
    if ( ! isdigit ( c ) )
        return NULL;
    tmp = c - '0';
    c = s [ ++ i ];
    if ( ! isdigit ( c ) )
        return NULL;
    tmp = tmp * 10 + c - '0';
    if ( tmp == 0 || tmp > 12 )
        return NULL;
    kt -> month = tmp - 1;

    c = s [ ++ i ];
    if ( c != '-' )
        return NULL;

    c = s [ ++ i ];
    if ( ! isdigit ( c ) )
        return NULL;
    tmp = c - '0';
    c = s [ ++ i ];
    if ( ! isdigit ( c ) )
        return NULL;
    tmp = tmp * 10 + c - '0';
    if ( tmp == 0 || tmp > 31 )
        return NULL;
    kt -> mday = tmp;

    c = s [ ++ i ];
    if ( c != 'T' )
        return NULL;

    c = s [ ++ i ];
    if ( ! isdigit ( c ) )
        return NULL;
    tmp = c - '0';
    c = s [ ++ i ];
    if ( ! isdigit ( c ) )
        return NULL;
    tmp = tmp * 10 + c - '0';
    if ( tmp > 23 )
        return NULL;
    kt -> hour = tmp;

    c = s [ ++ i ];
    if ( c != ':' )
        return NULL;

    c = s [ ++ i ];
    if ( ! isdigit ( c ) )
        return NULL;
    tmp = c - '0';
    c = s [ ++ i ];
    if ( ! isdigit ( c ) )
        return NULL;
    tmp = tmp * 10 + c - '0';
    if ( tmp > 59 )
        return NULL;
    kt -> minute = tmp;

    c = s [ ++ i ];
    if ( c != ':' )
        return NULL;

    c = s [ ++ i ];
    if ( ! isdigit ( c ) )
        return NULL;
    tmp = c - '0';
    c = s [ ++ i ];
    if ( ! isdigit ( c ) )
        return NULL;
    tmp = tmp * 10 + c - '0';
    if ( tmp > 59 )
        return NULL;
    kt -> second = tmp;

    c = s [ ++ i ];
    if ( c != 'Z' )
        return NULL;

    return kt;
}


LIB_EXPORT rc_t CC KSleep(uint32_t seconds) { return KSleepMs(seconds * 1000); }
