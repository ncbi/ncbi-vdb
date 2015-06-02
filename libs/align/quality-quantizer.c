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
#include <klib/defs.h>
#include <align/quality-quantizer.h>

#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include <os-native.h>


LIB_EXPORT
bool CC QualityQuantizerInitMatrix(uint8_t dst[256], char const quant[])
{
    unsigned i = 0;
    unsigned limit = 0;
    unsigned value = 0;
    unsigned cur;
    int ws = 1;
    int st = 0;
    
    memset(dst, 0, 256);
    for (cur = 0; quant[cur] != 0; ++cur) {
        int const ch = quant[cur];
        
        if (ws) {
            if (isspace(ch))
                continue;
            ws = false;
        }
        switch (st) {
        case 0:
            if (isdigit(ch)) {
                value = (value * 10) + ch - '0';
                break;
            }
            else if (isspace(ch)) {
                ++st;
                ws = true;
                break;
            }
            ++st;
            /* no break */
        case 1:
            if (ch != ':')
                return false;
            ws = true;
            ++st;
            break;
        case 2:
            if (isdigit(ch)) {
                limit  = (limit * 10) + ch - '0';
                break;
            }
            else if (isspace(ch)) {
                ++st;
                ws = true;
                break;
            }
            else if (ch == '-' && limit == 0) {
                memset(dst + i, value, 256 - i);
                return true;
            }
            ++st;
            /* no break */
        case 3:
            if (ch != ',')
                return false;
            ws = true;
            st = 0;
            if (i > limit)
                return false;
            /* set from dst[i..(limit - 1)] = value
             * not inclusive of the endpoint
             */
            memset(dst + i, value, limit - i);
            i = limit;
            limit = value = 0;
            break;
        }
    }
    return false;
}
