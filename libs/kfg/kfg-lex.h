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

#ifndef _h_kfg_lex_
#define _h_kfg_lex_

#include <string.h>
#include <stdbool.h>

typedef struct KFGToken
{
    const char* tokenText;
    size_t tokenLength;
    int tokenId;
    size_t line_no;
    size_t column_no;
} KFGToken;

typedef struct KFGScanBlock
{
    void* scanner;
    void* buffer;
    void* self; 
    const char* file;
    KFGToken* lastToken; /* used for error reporting */
    bool (*look_up_var)(void * self, struct KFGToken*);
    void (*report_error)(struct KFGScanBlock* sb, const char* msg);
} KFGScanBlock;

struct VNamelist;
typedef union 
{
	KFGToken            pb;
	struct VNamelist*   namelist;
} KFGSymbol;

extern bool KFGScan_yylex_init(KFGScanBlock* sb, const char *str); /* false = out of memory */
extern void KFGScan_yylex_destroy(KFGScanBlock* sb);

extern void KFGScan_set_debug(bool on, KFGScanBlock* scannerContext);

extern int KFGScan_yylex(KFGSymbol* pb, KFGScanBlock* scannerContext);

#endif /* _h_kfg_lex_ */
