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

#ifndef _h_scanner_lex_
#define _h_scanner_lex_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

typedef struct SchemaToken
{
    int type;
    const char* value; /* points into the flex's buffer, valid until the next call to lex() */
    size_t value_len;
    char* leading_ws; /* NUL-terminated; the consumer of the token is responsible for free()ing */
    void* subtree; /* used by the parser */
} SchemaToken;

typedef struct SchemaScanBlock
{
    void* scanner;
    void* buffer;
    char* whitespace;
} SchemaScanBlock;

extern void SchemaScan_yylex_init ( SchemaScanBlock* sb, const char *str );

extern void SchemaScan_yylex_destroy ( SchemaScanBlock* sb );

extern void SchemaScan_set_debug ( SchemaScanBlock* sb, int on );

#ifdef __cplusplus
}
#endif

#endif
