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

/* %{
   Prologue
   %}
   Declarations
   %%
   Grammar rules
   %%
   Epilogue
   */

%{
    #include "samextract.h"
    #include "samextract-pool.h"
    #include <align/samextract-lib.h>
    #include <klib/rc.h>
    int SAMlex(SAMExtractor *);
%}

/* Bison Declarations */
%union {
 char * strval;
}

%name-prefix "SAM"
%param { SAMExtractor * state}
%require "3.0"
%define parse.error verbose

%token <strval> HEADER
%token <strval> SEQUENCE
%token <strval> READGROUP
%token <strval> PROGRAM
%token <strval> COMMENT

%token <strval> VALUE

%token <strval> QNAME
%token <strval> FLAG
%token <strval> RNAME
%token <strval> POS
%token <strval> MAPQ
%token <strval> CIGAR
%token <strval> RNEXT
%token <strval> PNEXT
%token <strval> TLEN
%token <strval> SEQ
%token <strval> QUAL

%token <strval> OPTTAG
%token <strval> OPTITAG
%token <strval> OPTZTAG
%token <strval> OPTBTAG

%token <strval> OPTATYPE
%token <strval> OPTITYPE
%token <strval> OPTFTYPE
%token <strval> OPTZTYPE
%token <strval> OPTHTYPE
%token <strval> OPTBTYPE

%token <strval> OPTAVALUE
%token <strval> OPTIVALUE
%token <strval> OPTFVALUE
%token <strval> OPTZVALUE
%token <strval> OPTHVALUE
%token <strval> OPTBVALUE

%token HDVN
%token HDSO
%token HDGO

%token <strval> RGID
%token <strval> RGCN
%token <strval> RGDS
%token <strval> RGDT
%token <strval> RGFO
%token <strval> RGKS
%token <strval> RGLB
%token <strval> RGPG
%token <strval> RGPI
%token <strval> RGPL
%token <strval> RGPM
%token <strval> RGPU
%token <strval> RGSM

%token <strval> PGID
%token <strval> PGPN
%token <strval> PGCL
%token <strval> PGPP
%token <strval> PGDS
%token <strval> PGVN

%token <strval> SQSN
%token <strval> SQLN
%token <strval> SQAS
%token <strval> SQM5
%token <strval> SQSP
%token <strval> SQUR

%token TAB
%token <strval> CONTROLCHAR
%token EOL
%token END 0 "end of file"

%expect 0
%%

 /* Bison grammar rules */
sam: /* beginning of input */
   %empty
   | sam line
   ;

line:
    EOL { DBG("empty line"); } /* Spec is unclear about empty lines, accept for now */
   | CONTROLCHAR { ERR("CONTROLCHAR %d", $1[0]);
                   rc_t rc=RC(rcAlign,rcRow,rcParsing,rcData,rcInvalid);
                   state->rc=rc;
                   return END;
   }
   | comment { DBG("comment"); }
   | header { DBG("header done"); }
   | sequence { DBG("sequence"); }
   | program { DBG("program"); }
   | readgroup { DBG("readgroup"); }
   | alignment { DBG("alignment"); }
   ;

comment:
       COMMENT { mark_headers(state,"CO"); }
    ;

header:
      HEADER headerlist EOL
    {
        DBG("header list");
        if (!state->hashdvn)
        {
            ERR("VN tag not seen in header");
            rc_t rc=RC(rcAlign,rcRow,rcParsing,rcData,rcInvalid);
            state->rc=rc;
        }
        if (state->hashdso && state->hashdgo)
           WARN("Both SO and GO tags present");
        if (!state->hashdso && !state->hashdgo)
           WARN("neither SO or GO tags present");

        mark_headers(state,"HD"); }
    /* TODO: Duplicate header */
    ;

headerlist:   hdr
            | headerlist hdr
  ;

hdr: HDVN VALUE {
        state->hashdvn=true;
        if (process_header(state,"HD","VN",$2)) return END;
        DBG("HDVN VALUE");
        }
   | HDSO VALUE {
        state->hashdso=true;
        if (process_header(state,"HD","SO",$2)) return END;
        }
   | HDGO VALUE {
        state->hashdgo=true;
        if (process_header(state,"HD","GO",$2)) return END;
        }
        /* TODO: Handle >2 tabs in a row */
        /*
  | TAB TAB {
        ERR("two tabs");
        rc_t rc=RC(rcAlign,rcRow,rcParsing,rcData,rcInvalid);
        state->rc=rc; }
        */
  | TAB { ERR("empty HD tags");
        rc_t rc=RC(rcAlign,rcRow,rcParsing,rcData,rcInvalid);
        state->rc=rc;
        return END;}
  ;



sequence:
     SEQUENCE sequencelist EOL
    {
        DBG("sequence");
        if (!state->hassqsn)
        {
            ERR("SN tag not seen in header");
            rc_t rc=RC(rcAlign,rcRow,rcParsing,rcData,rcInvalid);
            state->rc=rc;
        }
        if (!state->hassqln)
        {
            ERR("LN tag not seen in header");
            rc_t rc=RC(rcAlign,rcRow,rcParsing,rcData,rcInvalid);
            state->rc=rc;
        }
        mark_headers(state,"SQ"); }
    ;

sequencelist: sq
    | sequencelist sq
    ;

sq:
      SQSN VALUE {
        state->hassqsn=true;
        if (process_header(state,"SQ",$1,$2)) return END;
        }
    | SQLN VALUE {
        if (!inrange($2,1,INT32_MAX))
        {
            ERR("SQ LN field not in range %s",$2);
            rc_t rc=RC(rcAlign,rcRow,rcParsing,rcData,rcInvalid);
            state->rc=rc;
        }
        state->hassqln=true;
        if (process_header(state,"SQ",$1,$2)) return END;
        }
    | SQAS VALUE {
        if (process_header(state,"SQ",$1,$2)) return END;
        ; }
    | SQM5 VALUE {
        if (!ismd5($2))
            WARN("M5 value not followed by MD5");
        if (process_header(state,"SQ",$1,$2)) return END;
        ; }
    | SQSP VALUE {
        if (process_header(state,"SQ",$1,$2)) return END;
        ; }
    | SQUR VALUE {
        if (process_header(state,"SQ",$1,$2)) return END;
        ; }
    | TAB { ERR("Unexpected tab in sequence");
        rc_t rc=RC(rcAlign,rcRow,rcParsing,rcData,rcInvalid);
        state->rc=rc;
        return END;}
    ;

program:
      PROGRAM programlist EOL
     {
        DBG("program");
        if (!state->haspgid)
        {
            ERR("ID tag not seen in header");
            rc_t rc=RC(rcAlign,rcRow,rcParsing,rcData,rcInvalid);
            state->rc=rc;
        }
        mark_headers(state,"PG"); }
     ;

programlist: pg
    | programlist pg
    ;

pg:
      PGID VALUE {
        state->haspgid=true;
        if (process_header(state,"PG",$1,$2)) return END;
        ; }
    | PGPN VALUE {
        if (process_header(state,"PG",$1,$2)) return END;
        ; }
    | PGCL VALUE {
        if (process_header(state,"PG",$1,$2)) return END;
        ; }
    | PGPP VALUE {
        if (process_header(state,"PG",$1,$2)) return END;
        ; }
    | PGDS VALUE {
        if (process_header(state,"PG",$1,$2)) return END;
        ; }
    | PGVN VALUE {
        if (process_header(state,"PG",$1,$2)) return END;
        ; }
    | VALUE {
        WARN("Bogus value in PG:%s",$1);
         }
    ;


readgroup:
      READGROUP readgrouplist EOL
    {
        DBG("readgroup ");
        if (!state->hasrgid)
        {
            ERR("ID tag not seen in header");
            rc_t rc=RC(rcAlign,rcRow,rcParsing,rcData,rcInvalid);
            state->rc=rc;
        }

        mark_headers(state,"RG"); }
    ;

readgrouplist:   rg
            | readgrouplist rg
  ;

rg:  RGID VALUE {
        state->hasrgid=true;
        if (process_header(state,"RG",$1,$2)) return END;
        ; }
    /* Can't match platforms in lexer, they often collide with CN */
   | RGPL VALUE {
        if (
            strcasecmp($2,"illumina") && /* most frequent */
            strcmp($2,"CAPILLARY") &&
            strcmp($2,"LS454") &&
            strcasecmp($2,"solid") &&
            strcmp($2,"HELICOS") &&
            strcmp($2,"IONTORRENT") &&
            strcmp($2,"ONT") &&
            strcasecmp($2,"pacbio") &&
            strcmp($2,"\"Complete Genomics\"") && /* not compliant */
            strcmp($2,"illumina Hiseq") && /* most frequent */
            strcmp($2,"COMPLETEGENOMICS") && /* not compliant */
            strcmp($2,"PacBio_SMRT") && /* not compliant */
            strcmp($2,"PacBio_RS")  /* not compliant */
        )
            WARN("Invalid Platform %s", $2);
        if (process_header(state,"RG",$1,$2)) return END;
        ; }
   | RGCN VALUE {
        if (process_header(state,"RG",$1,$2)) return END;
        ; }
   | RGDS VALUE {
        if (process_header(state,"RG",$1,$2)) return END;
        ; }
   | RGDT VALUE {
        if (process_header(state,"RG",$1,$2)) return END;
        ; }
   | RGFO VALUE {
        if (!isfloworder($2))
            WARN("Flow order incorrec");
        if (process_header(state,"RG",$1,$2)) return END;
        ; }
   | RGKS VALUE {
        if (process_header(state,"RG",$1,$2)) return END;
        ; }
   | RGLB VALUE {
        if (process_header(state,"RG",$1,$2)) return END;
        ; }
   | RGPG VALUE {
        if (process_header(state,"RG",$1,$2)) return END;
        ; }
   | RGPI VALUE {
        if (process_header(state,"RG",$1,$2)) return END;
        ; }
   | RGPM VALUE {
        if (process_header(state,"RG",$1,$2)) return END;
        ; }
   | RGPU VALUE {
        if (process_header(state,"RG",$1,$2)) return END;
        ; }
   | RGSM VALUE {
        if (process_header(state,"RG",$1,$2)) return END;
        ; }
   | VALUE VALUE {
        WARN("Unknown readgroup (RG) tag:%s", $1);
        
        ;
        }
   | TAB TAB EOL {
        ERR("empty RG tags");
        rc_t rc=RC(rcAlign,rcRow,rcParsing,rcData,rcInvalid);
        state->rc=rc; }
   | TAB EOL { WARN("empty tags"); }
   ;



alignment:
     QNAME FLAG RNAME POS MAPQ CIGAR RNEXT PNEXT TLEN SEQ QUAL EOL
     {
        DBG("alignment record %s",$1);
        process_alignment(state,$1,$2,$3,$4,$5,$6,$7,$8,$9,$10,$11);
     }
     | QNAME FLAG RNAME POS MAPQ CIGAR RNEXT PNEXT TLEN SEQ QUAL optlist EOL
     {
        DBG("alignment record with optional tags");
        process_alignment(state,$1,$2,$3,$4,$5,$6,$7,$8,$9,$10,$11);
     }
    ;

optlist: opt { DBG("opt"); }
       | optlist opt { DBG(" opts"); }
    ;

opt:
    OPTTAG OPTATYPE OPTAVALUE
    {
        DBG("?AA");
    }
    | OPTTAG OPTITYPE OPTIVALUE
    {
        DBG("?II");
    }
    | OPTTAG OPTFTYPE OPTFVALUE
    {
        DBG("?FF");
    }
    | OPTTAG OPTZTYPE OPTZVALUE
    {
        DBG("?ZZ");
    }
    | OPTTAG OPTHTYPE OPTHVALUE
    {
        DBG("?HH");
    }
    | OPTTAG OPTBTYPE OPTBVALUE
    {
        DBG("?BB");
    }
    | OPTITAG OPTITYPE OPTIVALUE
    {
        DBG("III");
    }
    | OPTZTAG OPTZTYPE OPTZVALUE
    {
        DBG("ZZZ");
    }
    | OPTBTAG OPTBTYPE OPTBVALUE
    {
        DBG("BBB");
    }
    ;

%%

