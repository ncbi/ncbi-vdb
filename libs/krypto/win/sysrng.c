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

/* -----
 * this is a windows thing...  MSDN said required so it's out of our 
 * include normal order
 *
 * doing this define and include in this way brings in rand_s which is based on
 * the same RNG used in the fussy _CryptGenRandom.  But without creating the
 * files and other stuff that is suitable for full Windows based cryptography
 * but not our virtual environment to get a simple seed for our OpenSSL based 
 * cryptography.
 */
#define _CRT_RAND_S
#include <stdlib.h>

#include <os-native.h>

#include <krypto/extern.h>
#include <krypto/rng.h>
#include "rng-priv.h"

#include <klib/rc.h>
#include <klib/text.h>
#include <klib/printf.h>
#include <klib/checksum.h>

#include <stdio.h>
#include <string.h>

#include <time.h>


rc_t KRngSysEntropy (KRng * self, uint8_t * buffer, size_t buff_size)
{
    MD5State state;
    uint64_t ul;
    size_t buffix;
    int len;
    char buff [4096];
    char obuff [4096];
    char digest [16];

    if (self == NULL)
        return RC (rcKrypto, rcRng, rcWriting, rcSelf, rcNull);
    if (buffer == NULL)
        return RC (rcKrypto, rcRng, rcWriting, rcParam, rcNull);

    while (buff_size > 0)
    {
        for (buffix = 0; buffix < 256 ; ++buffix)
        {
            unsigned int ui;
            if (rand_s (&ui))
                break;
            /* still paranoid about the old axiom of not using lowest 
             * order bits in a random number */
            buff[buffix] = (char)(ui>>4);
        }

        len = gethostname (buff + buffix, sizeof (buff) - buffix);
        if (len == 0)
            buffix += strlen (buff + buffix);
        if (buffix >= sizeof (buff))
            break;


        _time64(&ul);

        string_printf (buff + buffix, sizeof (buff) - buffix,
                            &len, "%ld", ul);
        if (len > 0)
            buffix += len;
        if (buffix >= sizeof (buff))
            break;

        ul = clock ();

        string_printf (buff + buffix, sizeof (buff) - buffix,
                        &len, "%ld", ul);
        if (len > 0)
            buffix += len;
        if (buffix >= sizeof (buff))
            break;

        if (tmpnam_s(obuff, sizeof (obuff)))
        {
            string_printf (buff + buffix, sizeof (buff) - buffix,
                           &len, "%s", obuff);
            if (len > 0)
                buffix += len;
            if (buffix >= sizeof (buff))
                break;
        }
    

        MD5StateInit (&state);
        MD5StateAppend (&state, buff, sizeof buff);
        MD5StateFinish (&state, digest);

        len = sizeof digest > buff_size ? buff_size : sizeof digest;
        memcpy (buffer, digest, len);

        buffer += len;
        buff_size -= len;
    }
    
    return 0;
}
