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

#ifndef _h_kfg_scan_
#define _h_kfg_scan_

#include <kfg/extern.h>
#include <klib/rc.h>
#include <klib/namelist.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct KFGParseBlock
{
    const char* tokenText;
    size_t tokenLength;
    int tokenId;
    size_t line_no;
    size_t column_no;
} KFGParseBlock;

typedef struct KFGScanBlock
{
    void* scanner;
    void* buffer;
    void* self; 
    const char* file;
    KFGParseBlock* lastToken; /* used for error reporting */
    rc_t (*write_nvp)(void * self, const char* name, size_t nameLen, VNamelist*);
    bool (*look_up_var)(void * self, struct KFGParseBlock*);
    void (*report_error)(struct KFGScanBlock* sb, const char* msg);
} KFGScanBlock;

#define YYSTYPE_IS_DECLARED
typedef union 
{
	KFGParseBlock   pb;
	VNamelist*	    namelist;
} KFGSymbol;

/* Exposed for the sake of wb-test-kfg. */
KFG_EXTERN rc_t CC KFGScan_yylex_init(KFGScanBlock* sb, const char *str);
KFG_EXTERN void CC KFGScan_yylex_destroy(KFGScanBlock* sb);

KFG_EXTERN void CC KFGScan_set_debug(bool on, KFGScanBlock* scannerContext);

KFG_EXTERN int CC KFGScan_yylex(KFGSymbol* pb, KFGScanBlock* scannerContext);

KFG_EXTERN int CC KFG_parse(KFGParseBlock* pb, KFGScanBlock* scannerContext);
void CC KFG_error(KFGParseBlock* pb, KFGScanBlock* sb, const char* msg);

#ifdef __cplusplus
}
#endif

#endif /* _h_kfg_scan_ */
