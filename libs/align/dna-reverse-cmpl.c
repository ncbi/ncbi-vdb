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
#include <align/extern.h>

#include <klib/rc.h>
#include <insdc/insdc.h>
#include <sysalloc.h>

#include "debug.h"
#include <align/dna-reverse-cmpl.h>
#include <string.h>

LIB_EXPORT rc_t CC DNAReverseCompliment(const INSDC_dna_text* seq, INSDC_dna_text* cmpl, uint32_t len)
{
    rc_t rc = 0;
    uint32_t i;
    static INSDC_dna_text compl[256] = "~";

    if( seq == NULL || compl == NULL ) {
        rc = RC( rcAlign, rcType, rcConverting, rcParam, rcNull);
    } else {
        if( compl[0] == '~' ) {
            INSDC_dna_text* x = (INSDC_dna_text*)&compl[0];
            memset(x, 0, sizeof(compl));
            x['A'] = x['a'] = 'T';
            x['T'] = x['t'] = 'A';
            x['U'] = x['u'] = 'A';
            x['G'] = x['g'] = 'C';
            x['C'] = x['c'] = 'G';
            x['Y'] = x['y'] = 'R';
            x['R'] = x['r'] = 'Y';
            x['S'] = x['s'] = 'S';
            x['W'] = x['w'] = 'W';
            x['K'] = x['k'] = 'M';
            x['M'] = x['m'] = 'K';
            x['B'] = x['b'] = 'V';
            x['D'] = x['d'] = 'H';
            x['H'] = x['h'] = 'D';
            x['V'] = x['v'] = 'B';
            x['N'] = x['n'] = 'N';
            x['0'] = '0';
            x['1'] = '1';
            x['2'] = '2';
            x['3'] = '3';
        }
        cmpl += len;
        for(i = 0; i < len; i++) {
            *--cmpl= compl[(int)seq[i]];
            if( *cmpl == '\0' ) {
                rc = RC(rcAlign, rcFormatter, rcWriting, rcData, rcInvalid);
                break;
            }
        }
    }
    ALIGN_DBGERR(rc);
    return rc;
}
