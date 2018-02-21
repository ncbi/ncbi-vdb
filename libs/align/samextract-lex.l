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

   /*
      lex file is three sections:
      definitions: name definition
      %%
      rules: pattern action
      %%
      user code
   */

%top{
#include <string.h>
#include "samextract.h"
#include "samextract-pool.h"
#include "samextract-tokens.h"

/* #define LEXDEBUG 1 */
#define YY_INPUT(buf,numbytes,maxbytes) moredata(buf,&numbytes,maxbytes)
 }
 /* %top */

 /* definitions */
digits [[:digit:]]+
integer [-+]?[0-9]+
posinteger [+]?[0-9]+
float  [-+]?[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?
eol    (\r\n|\r|\n)
 /* Exclude tabs, newlines, carrige returns */

 /* controlchar [\x00-\x08\x0b-\x0c\x0e-\x1f\x80-\xff] */
controlchar [\x00-\x08\x0b-\x0c\x0e-\x1f]


 /* qname=No @ in first, to distinguish from headers */
qname [!-?A-~][!-~]{0,253}
flag {posinteger}
 /* rname=No * or = in first */
rname \*|[!-)+-<>-~][!-~]*
pos {posinteger}
mapq {posinteger}
cigar \*|([0-9]+[MIDNSHPX=])+
rnext \*|=|[!-)+-<>-~][!-~]*
pnext {posinteger}
tlen {integer}
seq \*|[A-Za-z=.]+
qual [!-~]+
opttag [A-Za-z][A-Za-z0-9]

 /* See bison's lex-param/parse-param */
%option prefix="SAM"
 /* stop after first EOF */
%option noyywrap
%option nodefault
%option warn
%option nodebug
 /* %option nodebug */
 /* Don't use these functions */
%option nounput
%option noinput
 /* Not a TTY */
%option never-interactive
 /* %option yylineno */
%option nounistd
%option extra-type="SAMExtractor *"
 /*%option bison-bridge */

 /* Start states */
%s INHD
%s INSQ
%s INRG
%s INPG
%x ALIGNQNAME
%x ALIGNFLAG
%x ALIGNRNAME
%x ALIGNPOS
%x ALIGNMAPQ
%x ALIGNCIGAR
%x ALIGNRNEXT
%x ALIGNPNEXT
%x ALIGNTLEN
%x ALIGNSEQ
%s ALIGNOPT
%x ALIGNOPTTYPE
%x ALIGNOPTAVALUE
%x ALIGNOPTIVALUE
%x ALIGNOPTFVALUE
%x ALIGNOPTZVALUE
%x ALIGNOPTHVALUE
%x ALIGNOPTBVALUE

%%
 /* Generic rules */
{controlchar} {
    SAMlval.strval=pool_memdup(SAMtext,SAMleng+1);
    DBG("CONTROLCHAR %d",yytext[0]);
    return CONTROLCHAR; }
\t { DBG(" tab"); return TAB; }
{eol} {
    BEGIN INITIAL;
    DBG("EOL %d",yytext[0]);
    if (yytext[0]==0)
    {
        ERR("NULL");
        return END;
    }
    return EOL; }

 /* Header */
<INITIAL>^@HD   {
    BEGIN INHD;
    DBG( "HEADER");
    /* TODO Header should be in first line only */
    return HEADER; }
<INHD>"\tVN:" { DBG("header VN"); return HDVN; }
<INHD>"\tSO:" { DBG("header SO"); return HDSO; }
<INHD>"\tGO:" return HDGO;
<INHD>\t[A-Z][A-Z]: { ERR("Undefined HD tag %s",yytext+1); yyterminate();}
<INHD>[0-9]+\.[0-9]+ {
    SAMlval.strval=pool_memdup(SAMtext,SAMleng+1);
    DBG(" header version");
    return VALUE; }
<INHD>unknown|unsorted|queryname|coordinate {
    SAMlval.strval=pool_memdup(SAMtext,SAMleng+1);
    DBG(" header SO");
    return VALUE; }
<INHD>none|query|reference {
    SAMlval.strval=pool_memdup(SAMtext,SAMleng+1);
    DBG(" header GO");
    return VALUE; }


 /* Sequences */
<INITIAL>^@SQ {
    BEGIN INSQ;
    DBG("SEQUENCE");
    return SEQUENCE;
}
<INSQ>"\tSN:" { SAMlval.strval="SN"; return SQSN; }
<INSQ>"\tLN:" { SAMlval.strval="LN"; return SQLN; }
<INSQ>"\tAS:" { SAMlval.strval="AS"; return SQAS; }
<INSQ>"\tM5:" { SAMlval.strval="M5"; return SQM5; }
<INSQ>"\tMD5:" { WARN("SQ MD5 should be M5"); SAMlval.strval="M5"; return SQM5; }
<INSQ>"\tSP:" { SAMlval.strval="SP"; return SQSP; }
<INSQ>"\tUR:" { SAMlval.strval="UR"; return SQUR; }
<INSQ>\t[A-Z][A-Z]: { ERR("Undefined SQ tag %s",yytext+1); yyterminate();}
<INSQ>[^\t\r\n]+ {
    SAMlval.strval=pool_memdup(SAMtext,SAMleng+1);
    DBG("SEQ VALUE");
    return VALUE;
}

 /* Read Groups */
<INITIAL>^@RG {
    BEGIN INRG;
    DBG("READGROUP");
    return READGROUP;
}
<INRG>"\tID:" { SAMlval.strval="ID"; return RGID; }
<INRG>"\tCN:" { SAMlval.strval="CN"; return RGCN; }
<INRG>"\tDS:" { SAMlval.strval="DS"; return RGDS; }
<INRG>"\tDT:" { SAMlval.strval="DT"; return RGDT; }
<INRG>"\tFO:" { SAMlval.strval="FO"; return RGFO; }
<INRG>"\tKS:" { SAMlval.strval="KS"; return RGKS; }
<INRG>"\tLB:" { SAMlval.strval="LB"; return RGLB; }
<INRG>"\tPG:" { SAMlval.strval="PG"; return RGPG; }
<INRG>"\tPI:" { SAMlval.strval="PI"; return RGPI; }
<INRG>"\tPL:" { SAMlval.strval="PL"; return RGPL; }
<INRG>"\tPM:" { SAMlval.strval="PM"; return RGPM; }
<INRG>"\tPU:" { SAMlval.strval="PU"; return RGPU; }
<INRG>"\tSM:" { SAMlval.strval="SM"; return RGSM; }
<INRG>\t[A-Z][A-Z]: { ERR("Undefined RG tag:%s",yytext+1); yyterminate();}
<INRG>"\tLNID:" { WARN("LNID is improper RG tag");}
<INRG>\t[^: -~]+: { ERR("Undefined long RG tag: %s",yytext+1); yyterminate();}
<INRG>[^\t\r\n]+ {
    SAMlval.strval=pool_memdup(SAMtext,SAMleng+1);
    DBG("readgroup value:%s",yytext);
    return VALUE;
 }


 /* Programs */
<INITIAL>^@PG {
    BEGIN INPG;
    DBG("PROGRAM");
    return PROGRAM;
}
<INPG>"\tID:" { SAMlval.strval="ID"; return PGID; }
<INPG>"\tPN:" { SAMlval.strval="PN"; return PGPN; }
<INPG>"\tCL:" { SAMlval.strval="CL"; return PGCL; }
<INPG>"\tPP:" { SAMlval.strval="PP"; return PGPP; }
<INPG>"\tDS:" { SAMlval.strval="DS"; return PGDS; }
<INPG>"\tVN:" { SAMlval.strval="VN"; return PGVN; }
<INPG>\t[A-Z][A-Z]: { ERR("Undefined PG tag %s",yytext+1); yyterminate();}
<INPG>\t[a-z_]{3,} {
    WARN("Not a PG tag:%s",yytext+1);
    SAMlval.strval=pool_memdup(SAMtext+1, SAMleng);
    return VALUE; }
<INPG>[^\t\r\n]+ {
    SAMlval.strval=pool_memdup(SAMtext,SAMleng+1);
    DBG("program value:%s",yytext);
    return VALUE;
 }


 /* Comments */
<INITIAL>^@CO.*{eol} { DBG( " Comment"); return COMMENT; }


 /* Alignments */
<INITIAL>^{qname} {
    BEGIN ALIGNQNAME;
    SAMlval.strval=pool_memdup(SAMtext, SAMleng+1);
    DBG(" alignment qname '%s' %d '%s'", SAMtext, SAMleng, SAMlval.strval);
    return QNAME; }
<ALIGNQNAME>\t{flag} {
    BEGIN ALIGNFLAG;
    SAMlval.strval=pool_memdup(SAMtext+1, SAMleng);
    DBG(" alignment flag");
    return FLAG; }
<ALIGNFLAG>\t{rname} {
    BEGIN ALIGNRNAME;
    SAMlval.strval=pool_memdup(SAMtext+1, SAMleng);
    DBG(" alignment rname");
    return RNAME; }
<ALIGNRNAME>\t{pos} {
    BEGIN ALIGNPOS;
    SAMlval.strval=pool_memdup(SAMtext+1, SAMleng);
    DBG(" alignment pos");
    return POS; }
<ALIGNPOS>\t{mapq} {
    BEGIN ALIGNMAPQ;
    SAMlval.strval=pool_memdup(SAMtext+1, SAMleng);
    DBG(" alignment mapq");
    return MAPQ; }
<ALIGNMAPQ>\t{cigar} {
    BEGIN ALIGNCIGAR;
    SAMlval.strval=pool_memdup(SAMtext+1, SAMleng);
    DBG(" alignment cigar");
    return CIGAR; }
<ALIGNCIGAR>\t{rnext} {
    BEGIN ALIGNRNEXT;
    SAMlval.strval=pool_memdup(SAMtext+1, SAMleng);
    DBG(" alignment rnext");
    return RNEXT; }
<ALIGNRNEXT>\t{pnext} {
    BEGIN ALIGNPNEXT;
    SAMlval.strval=pool_memdup(SAMtext+1, SAMleng);
    DBG(" alignment pnext ");
    return PNEXT; }
<ALIGNPNEXT>\t{tlen} {
    BEGIN ALIGNTLEN;
    SAMlval.strval=pool_memdup(SAMtext+1, SAMleng);
    DBG(" alignment tlen");
    return TLEN; }
<ALIGNTLEN>\t{seq} {
    BEGIN ALIGNSEQ;
    SAMlval.strval=pool_memdup(SAMtext+1, SAMleng);
    DBG(" alignment seq");
    return SEQ; }
<ALIGNSEQ>\t{qual} {
    BEGIN ALIGNOPT;
    SAMlval.strval=pool_memdup(SAMtext+1, SAMleng);
    DBG(" alignment qual");
    return QUAL; }

 /* Optional alignment fields */
 /* Reserved and Locally defined tags */
<ALIGNOPT>\t(GC|GQ|GS|MF|SQ|X[A-Za-z0-9]|Y[A-Za-z0-9]|Z[A-Za-z0-9]) {
    BEGIN ALIGNOPTTYPE;
    SAMlval.strval=pool_memdup(SAMtext+1, 3);
    DBG(" alignment opt reserved");
    return OPTTAG; }

 /* Only types iZB have optioanl standardized tags */
<ALIGNOPT>\t(AM|AS|CM|CP|FI|H0|H1|H2|HI|IH|MQ|NH|NM|OP|PQ|SM|TC|UQ) {
    BEGIN ALIGNOPTTYPE;
    SAMlval.strval=pool_memdup(SAMtext+1, 3);
    DBG(" alignment opt '%s'", SAMlval.strval);
    return OPTITAG; }

<ALIGNOPT>\t(BC|BQ|CC|CO|CQ|CS|CT|E2|FS|LB|MC|MD|OC|OQ|PG|PT|PU|QT|Q2|R2|RG|RT|SZ|U2) {
    BEGIN ALIGNOPTTYPE;
    DBG(" alignment opt %d '%s'", SAMleng+1, SAMtext);
    SAMlval.strval=pool_memdup(SAMtext+1, 3);
    DBG(" alignment opt '%s'", SAMlval.strval);
    return OPTZTAG; }

<ALIGNOPT>\t(FZ) {
    BEGIN ALIGNOPTTYPE;
    SAMlval.strval=pool_memdup(SAMtext+1, 3);
    DBG(" alignment opt '%s'", SAMlval.strval);
    return OPTBTAG; }

<ALIGNOPT>\t{opttag} {
    BEGIN ALIGNOPTTYPE;
    SAMlval.strval=pool_memdup(SAMtext+1, 3);
    DBG(" alignment opt '%s'", SAMlval.strval);
    return OPTTAG; }

<ALIGNOPTTYPE>":A:" {
    BEGIN ALIGNOPTAVALUE;
    SAMlval.strval="A";
    return OPTATYPE; }
<ALIGNOPTTYPE>":i:" {
    BEGIN ALIGNOPTIVALUE;
    SAMlval.strval="i";
    return OPTITYPE; }
<ALIGNOPTTYPE>":f:" {
    BEGIN ALIGNOPTFVALUE;
    SAMlval.strval="f";
    return OPTFTYPE; }
<ALIGNOPTTYPE>":Z:" {
    BEGIN ALIGNOPTZVALUE;
    SAMlval.strval="Z";
    return OPTZTYPE; }
<ALIGNOPTTYPE>":H:" {
    BEGIN ALIGNOPTHVALUE;
    SAMlval.strval="H";
    return OPTHTYPE; }
<ALIGNOPTTYPE>":B:" {
    BEGIN ALIGNOPTBVALUE;
    SAMlval.strval="B";
    return OPTBTYPE; }

<ALIGNOPTAVALUE>[!-~] {
    BEGIN ALIGNOPT;
    SAMlval.strval=pool_memdup(SAMtext,2);
    return OPTAVALUE; }
<ALIGNOPTIVALUE>{integer} {
    BEGIN ALIGNOPT;
    SAMlval.strval=pool_memdup(SAMtext,SAMleng+1);
    return OPTIVALUE; }
<ALIGNOPTFVALUE>{float} {
    BEGIN ALIGNOPT;
    SAMlval.strval=pool_memdup(SAMtext,SAMleng+1);
    return OPTFVALUE; }
<ALIGNOPTZVALUE>[ -~]* {
    BEGIN ALIGNOPT;
    SAMlval.strval=pool_memdup(SAMtext,SAMleng+1);
    return OPTZVALUE; }
<ALIGNOPTHVALUE>([0-9A-Fa-f][0-9A-Fa-f])* {
    BEGIN ALIGNOPT;
    SAMlval.strval=pool_memdup(SAMtext,SAMleng+1);
    return OPTHVALUE; }
<ALIGNOPTBVALUE>cCsSiIf](,[-+]?[0-9]*\\.?[0-9]+(eE][-+]?[0-9]+)?)+ {
    BEGIN ALIGNOPT;
    SAMlval.strval=pool_memdup(SAMtext,SAMleng+1);
    return OPTBVALUE; }


.|\n    {
    if (yytext[0] < ' ' || yytext[0] > '~')
    {
        DBG("CONTROLCHAR %d",yytext[0]);
        return CONTROLCHAR;
    }
    ERR("DEFAULT '%c' ", *yytext);
    return VALUE;}
<<EOF>> { DBG("EOF"); yyterminate(); }
%%

#if YY_FLEX_MAJOR_VERSION == 2 && YY_FLEX_MINOR_VERSION < 6
#error "Flex version should be >= 2.6.X"
#endif

