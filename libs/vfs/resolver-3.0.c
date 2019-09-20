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
 * =============================================================================
 */

#include "path-priv.h" /* VPathMakeFmt */
#include "resolver-priv.h" /* rcResolver */

#include <klib/log.h> /* klogInt */
#include <klib/rc.h> /* RC */
#include <klib/text.h> /* string_chr */

#include <vfs/path.h> /* VPathMakeFmt */

static rc_t _GetAny(const rc_t *rc, const char **start,
    const char *end, String *s, bool last)
{
    assert(rc && start);

    if (*rc != 0) {
        return *rc;
    }

    if (!last) {
        const char *sep = string_chr ( *start, end - *start, '|' );
        if ( sep == NULL ) {
            return RC ( rcVFS, rcResolver, rcResolving, rcName, rcNotFound );
        }

        StringInit(s, *start, sep - *start, ( uint32_t ) ( sep - *start ));

        *start = sep + 1;
    }
    else {
        const char *sep = NULL;
        for ( sep = end; sep > *start; -- sep ) {
            switch ( sep [ -1 ] ) {
                case '\n':
                case '\r':
                    continue;
                default:
                    break;
            }
            break;
        }

        StringInit ( s, *start, sep - *start, ( uint32_t ) ( sep - *start ) );
    }

    return 0;
}

static rc_t _Get(const rc_t *rc, const char **start,
    const char *end, String *s)
{
    return _GetAny(rc, start, end, s, false);
}

static rc_t _GetLast(const rc_t *rc, const char **start,
    const char *end, String *s)
{
    return _GetAny(rc, start, end, s, true);
}

typedef enum {
    eBadObjectType,
    eDbgap,
    eProvisional,
    eSrapub, /* eSra_run, */
    eSrapub_files,
    eSragap, /* eSra_run, */
    eSra_source,
    eSra_addon,
    eRefseq,
    eWgs,
    eNa,
} EObjectType;
EObjectType _StringToObjectType(const String *self) {
    if (self->size == 0) {
        return eBadObjectType;
    }
    else {
        String dbgap;
        String provisional;
        String srapub;
        String srapub_files;
        String sragap;
        String sra_source;
        String sra_addon;
        String refseq;
        String wgs;
        String na;
        CONST_STRING(&dbgap, "dbgap");
        CONST_STRING(&provisional, "provisional");
        CONST_STRING(&srapub, "srapub");
        CONST_STRING(&srapub_files, "srapub_files");
        CONST_STRING(&sragap, "sragap");
        CONST_STRING(&sra_source, "sra-source");
        CONST_STRING(&sra_addon, "sra-addon");
        CONST_STRING(&refseq, "refseq");
        CONST_STRING(&wgs, "wgs");
        CONST_STRING(&na, "na");
        if (StringEqual(self, &dbgap)) {
            return eDbgap;
        }
        else if (StringEqual(self, &provisional)) {
            return eProvisional;
        }
        else if (StringEqual(self, &srapub)) {
            return eSrapub;
        }
        else if (StringEqual(self, &srapub_files)) {
            return eSrapub_files;
        }
        else if (StringEqual(self, &sragap)) {
            return eSragap;
        }
        else if (StringEqual(self, &sra_source)) {
            return eSra_source;
        }
        else if (StringEqual(self, &sra_addon)) {
            return eSra_addon;
        }
        else if (StringEqual(self, &refseq)) {
            return eRefseq;
        }
        else if (StringEqual(self, &wgs)) {
            return eWgs;
        }
        else if (StringEqual(self, &na)) {
            return eNa;
        }
        else {
            return eBadObjectType;
        }
    }
}

static rc_t _ProcessCode(EObjectType objectType, const String *object_id,
    const String *code, const String *download_ticket,
    const String *url, const String *message, const VPath **path,
    const VPath **mapping, const String *acc, const String *ticket)
{
    rc_t rc = 0;
    KLogLevel lvl = 0;
    uint32_t result_code = 0;
    char *rslt_end = NULL;
    assert(objectType && object_id && code && download_ticket && url);
    if (code->size == 0)
        return RC ( rcVFS, rcResolver, rcResolving, rcMessage, rcCorrupt );
    result_code = strtoul ( code -> addr, & rslt_end, 10 );
    if ( ( const char* ) rslt_end - code -> addr != code -> size )
        return RC ( rcVFS, rcResolver, rcResolving, rcMessage, rcCorrupt );
    switch ( result_code / 100 ) {
        case 2: /* successful response but can only handle 200 */
            if ( result_code == 200 ) { /* normal public response */
                if (download_ticket->size == 0) {
                    rc = VPathMakeFmt ( ( VPath** ) path, "%S", url );
                }
                else { /* protected response */
                    rc = VPathMakeFmt
                        ((VPath**) path, "%S?tic=%S", url, download_ticket);
                }
                if ( rc == 0 ) {
                    rc = VPathCheckFromNamesCGI ( * path, ticket, -1, mapping );
                    if ( rc == 0 ) {
                        if ( mapping == NULL )
                            return 0;
                        if (download_ticket->size != 0) {
                            if (object_id->size != 0 && objectType == eSragap) {
                                rc = VPathMakeFmt ( ( VPath** ) mapping,
                                    "ncbi-acc:%S?tic=%S",
                                    object_id, download_ticket);
                            }
                            else {
                                if (object_id->size == 0)
                                    return 0;
                                else
                                    rc = VPathMakeFmt ( ( VPath** ) mapping,
                                        "ncbi-file:%S?tic=%S",
                                        object_id, download_ticket);
                            }
                        }
                        else if (object_id->size != 0 && objectType == eSrapub)
                        {
                            rc = VPathMakeFmt ( ( VPath** ) mapping,
                                "ncbi-acc:%S", object_id );
                        }
                        else {
                            if (object_id->size == 0)
                                return 0;
                            else
                                rc = VPathMakeFmt ( ( VPath** ) mapping,
                                    "ncbi-file:%S", object_id );
                        }
                        if ( rc == 0 )
                            return 0;
                    }
                    VPathRelease ( * path );
                    * path = NULL;
                }
                return rc;
            }
            lvl = klogInt;
            rc = RC ( rcVFS, rcResolver, rcResolving, rcError, rcUnexpected );
            break;
        case 4: /* client error */
            lvl = klogErr;
            switch (result_code) {
                case 400:
                    rc = RC(rcVFS, rcResolver, rcResolving,
                        rcMessage, rcInvalid);
                    break;
                case 401:
                case 403:
                    rc = RC( rcVFS, rcResolver, rcResolving,
                        rcQuery, rcUnauthorized);
                    break;
                case 404: /* 404|no data :
           If it is a real response then this assession is not found.
           What if it is a DB failure? Will be retried if configured to do so?*/
                case 410:
                    rc = RC( rcVFS, rcResolver, rcResolving,
                        rcName, rcNotFound );
                    break;
                default:
                    rc = RC(rcVFS, rcResolver, rcResolving,
                        rcError, rcUnexpected);
            }
            break;
        case 5: /* server error */
            lvl = klogSys;
            switch (result_code) {
                case 503:
                    rc = RC(rcVFS, rcResolver, rcResolving,
                        rcDatabase, rcNotAvailable);
                    break;
                case 504:
                    rc = RC(rcVFS, rcResolver, rcResolving,
                        rcTimeout, rcExhausted);
                    break;
                default:
                    rc = RC(rcVFS, rcResolver, rcResolving,
                        rcError, rcUnexpected);
            }
            break;
        case 1: /* informational response not much we can do here */
        case 3: /* redirection:
                        currently this is being handled by our request object */
        default:
            lvl = klogInt;
            rc = RC ( rcVFS, rcResolver, rcResolving, rcError, rcUnexpected );
            break;
    }
    /* log message to user */
    PLOGERR(lvl, (lvl, rc,
        "failed to resolve accession '$(acc)' - $(msg) ( $(code) )",
        "acc=%S,msg=%S,code=%u", acc, message, result_code));
    return rc;
}

rc_t VResolverAlgParseResolverCGIResponse_3_0(const char *astart,
    size_t asize, const VPath **path, const VPath **mapping,
    const String *acc, const String *ticket)
{
    const char *start = astart;
    const char *end = start + asize;
    String object_type, object_id, size, date,
        md5, download_ticket, url, code, message;
    EObjectType objectType = eBadObjectType;
    
    rc_t rc = 0;
    rc = _Get    (&rc, &start, end, &object_type);
    rc = _Get    (&rc, &start, end, &object_id);
    rc = _Get    (&rc, &start, end, &size);
    rc = _Get    (&rc, &start, end, &date);
    rc = _Get    (&rc, &start, end, &md5);
    rc = _Get    (&rc, &start, end, &download_ticket);
    rc = _Get    (&rc, &start, end, &url);
    rc = _Get    (&rc, &start, end, &code);
    rc = _GetLast(&rc, &start, end, &message);
    if (rc != 0) {
        return rc;
    }

    objectType = _StringToObjectType(&object_type);
    if (objectType == eBadObjectType) {
        return RC ( rcVFS, rcResolver, rcResolving, rcMessage, rcCorrupt );
    }

    /* compare acc to accession or obj_id */
    assert(acc);
    if ( ! StringEqual ( & object_id, acc ) && objectType != eDbgap )
        return RC ( rcVFS, rcResolver, rcResolving, rcMessage, rcCorrupt );

    /* compare ticket
       currently this makes sense with 1 request from a known workspace */
    if (download_ticket.size != 0) {
        if ( ticket == NULL || ! StringEqual ( & download_ticket, ticket ) )
            return RC ( rcVFS, rcResolver, rcResolving, rcMessage, rcCorrupt );
    }

    /* get the result code */
    return _ProcessCode(objectType, &object_id, &code,
        &download_ticket, &url, &message, path, mapping, acc, ticket);
}
