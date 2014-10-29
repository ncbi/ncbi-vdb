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
 
%{  
    #include "kfg-parse.h"
    #include <sysalloc.h>
    #include <klib/rc.h>
    #include <klib/namelist.h>

    #define YYSTYPE_IS_DECLARED
    #define YYSTYPE KFGSymbol
    #include "config-tokens.h"
        
    #define KFG_lex KFGScan_yylex
    
    /* required parameter to VNamelistMake */
    #define NAMELIST_ALLOC_BLKSIZE 10
    
    static void ReportRc(KFGParseBlock* pb, KFGScanBlock* sb, rc_t rc);
    static void AppendName(KFGScanBlock* sb, VNamelist*, const KFGToken*);
    static void KFG_error(KFGParseBlock* pb, KFGScanBlock* sb, const char* msg);
%}

%pure-parser
%parse-param {KFGParseBlock* pb }
%lex-param {KFGToken* sb}
%parse-param {KFGScanBlock* sb }

%name-prefix="KFG_"

 /* tokens without textual representation or internal for the scanner */
%token kfgEND_INPUT 0 
%token kfgEND_LINE
%token kfgUNTERM_STRING
%token kfgUNTERM_ESCAPED_STRING
%token kfgUNRECOGNIZED
%token kfgVAR_REF
%token kfgUNTERM_VAR_REF
%token kfgASSIGN

 /* tokens with a textual representation in pb */
%token <pb> kfgSTRING
%token <pb> kfgESCAPED_STRING
%token <pb> kfgABS_PATH
%token <pb> kfgREL_PATH

%union {
    KFGToken                pb;
    const struct VNamelist* namelist;
}
%type <pb>          pathname  
%type <namelist>    value

%destructor
{
    VNamelistRelease($$);
} <namelist>

%%
 
config
    : name_value_pairs 
    | kfgEND_INPUT      
    ;
    
name_value_pairs
    : name_value_pair
    | name_value_pairs name_value_pair 
    ;
    
name_value_pair
    : pathname assign_op value line_end
        { 
            rc_t rc=pb->write_nvp(sb->self, $1.tokenText, $1.tokenLength, $3);
            if (rc != 0)
            {
                ReportRc(pb, sb, rc);
            }
            VNamelistRelease($3);
        } 
    | kfgEND_LINE       
    | error line_end    
    ;
    
pathname
    : kfgABS_PATH 
    | kfgREL_PATH
    ;
    
assign_op
    : kfgASSIGN
    ;   

value
    : kfgSTRING                 { VNamelistMake(&$$, NAMELIST_ALLOC_BLKSIZE); AppendName(sb, $$, &$1); }
    | kfgESCAPED_STRING         { VNamelistMake(&$$, NAMELIST_ALLOC_BLKSIZE); AppendName(sb, $$, &$1); }
    | value kfgSTRING           { AppendName(sb, $1, &$2); $$=$1; }
    | value kfgESCAPED_STRING   { AppendName(sb, $1, &$2); $$=$1; }
    ;
    
line_end
    : kfgEND_LINE
    | kfgEND_INPUT
    ;

%%

#include <assert.h>
#include <klib/token.h>
#include <klib/writer.h>

void KFG_error(KFGParseBlock* pb, KFGScanBlock* sb, const char* msg)
{
    sb->report_error(sb, msg);
}

void ReportRc(KFGParseBlock* pb, KFGScanBlock* sb, rc_t rc)
{
    char buf[1025];
    size_t num_writ;
    RCExplain(rc, buf, 1024, &num_writ);
    buf[1024]=0;
    yyerror(0, sb, buf);
}

void AppendName(KFGScanBlock* sb, VNamelist* nl, const KFGToken* pb)
{   /* pb represents either a kfgSTRING or a kfgESCAPED_STRING with opening and closed quotes clipped */
    rc_t rc;
    KToken t;
    size_t value_size;
    char* buf;

    if (pb->tokenLength == 0)
    {
        return;
    }
            
    t.id= pb->tokenId == kfgESCAPED_STRING ? eEscapedString : eString;
    StringInit(&t.str, pb->tokenText-1, pb->tokenLength+2, (uint32_t) (pb->tokenLength + 2) ); /* compensate for clipped quotes in order to use KTokenToString */
    buf=(char*)malloc(t.str.size);

    /* KTokenToString removes opening and closing quotes and handles escapes if present */
    rc = KTokenToString (&t, buf, t.str.size, &value_size);
    if (rc != 0)
    {
        ReportRc(0, sb, rc);
    }
    else
    {   
        assert(value_size < t.str.size);
        buf[value_size]=0;
        rc = VNamelistAppend(nl, buf);
        if (rc != 0)
        {
            ReportRc(0, sb, rc);
        }
    }       
    free(buf);
}


    
