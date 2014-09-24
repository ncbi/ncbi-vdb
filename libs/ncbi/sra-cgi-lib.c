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
 *
 */

#include <ncbi/kimpl-sra-cgi.h>
#include <ncbi/ksra-cgi.h>
#include <ncbi/ksra-cgi-lib.h>

#include <stdbool.h> /* true */

#include <sysalloc.h> /* required for free */

#include <assert.h>
#include <stdio.h> /* fopen */
#include <stdlib.h> /* free */
#include <string.h> /* memset */

/******************************************************************************/

typedef enum {
    eCmdBad,
    eCmdLib,
    eCmdTst,
} ECmd;
typedef enum {
    eDMUnknown,
    eDM32,
    eDM64,
} EDM;
typedef struct Debug {
    char *req;
    bool dbg;
    bool printFName;
    bool printReq;
    bool trace;
} Debug;
typedef struct Request {
    EDM bits;
    ECmd cmd;
    const char *os_arch;
    const char *os_name;
    const char *os_version;
    const char *version;
    Debug dbg;
} Request;
static void RequestFini(Request *self) {
    assert(self);
    free(self->dbg.req);
    memset(self, 0, sizeof *self);
}
static bool startsWith(const char *haystack, const char *needle) {
 if (haystack == NULL || haystack[0] == '\0') {
  return false;
 }
 size_t n = strlen(haystack);
 size_t s = strlen(needle);
 if (n >= s) {
        if (strncmp(haystack, needle, s) == 0) {
            return true;
        }
        else {
            return false;
        }
 }
 else {
  return false;
 }
}
static const char *RequestPath(const Request *self) {
 assert(self);
#define T "/panfs/traces01.be-md.ncbi.nlm.nih.gov/trace_software/"
 static const char linux64[]
                  = T "toolkit-versions/test/ncbi-ngs/libncbi-ngs-jni-64.so";
 if (self->os_name == NULL || startsWith(self->os_name, "Linux")) {
  if (self->bits == eDM32) {
   return           T "toolkit-versions/test/ncbi-ngs/libncbi-ngs-jni-32.so";
  }
  else {
   return linux64;
  }
 }
 else if (startsWith(self->os_name, "Mac")) {
  if (self->bits == eDM32) {
   return           T "toolkit-versions/test/ncbi-ngs/libncbi-ngs-jni-32.dylib";
  }
  else {
   return           T "toolkit-versions/test/ncbi-ngs/libncbi-ngs-jni-64.dylib";
  }
 }
 else if (startsWith(self->os_name, "Win")) {
  if (self->bits == eDM32) {
   return           T "toolkit-versions/test/ncbi-ngs/libncbi-ngs-jni-32.dll";
  }
  else {
   return           T "toolkit-versions/test/ncbi-ngs/libncbi-ngs-jni-64.dll";
  }
 }
 else {
  return linux64;
 }
}
static
int _SraCgiResponseLib(SraCgiResponse *self, const Request *request)
{
    if (request->dbg.dbg) {
        if (request->dbg.printReq) {
            SraCgiResponseWrite(self,
                request->dbg.req, strlen(request->dbg.req));
            SraCgiResponseWrite(self, "\n", 1);
        }
        return 0;
    }

    const char *path = RequestPath(request);
    if (request->dbg.trace) {
        const char buffer[] = "fopening ";
        SraCgiResponseWrite(self, buffer, sizeof buffer - 1);
        SraCgiResponseWrite(self, path, strlen(path));
        SraCgiResponseWrite(self, "\n", 1);
    }
    FILE *f = fopen(path, "r");
    if (request->dbg.trace) {
        const char buffer[] = "fopen = ";
        SraCgiResponseWrite(self, buffer, sizeof buffer - 1);
        SraCgiResponseWrite(self, f == NULL ? "KO\n" : "OK\n", 3);
    }
    if (f == NULL) {
        SraCgiResponseStatus(self, 500);
        return 500;
    }

    size_t total = 0;
    while (true) {
        char buffer[8192] = "";
        size_t s = fread(buffer, 1, sizeof buffer, f);
        total += s;

        if (s > 0) {
            ssize_t i = SraCgiResponseWrite(self, buffer, s);
            if (i < 0) {
                return self->err;
            }
        }

        if (s != sizeof buffer) {
            int eof = feof(f);
            int error = ferror(f);

            fclose(f);

            if (eof) {
                return 0;
            }
            else {
                return error;
            }
        }
    }
}

static int _SraCgiResponseTst(SraCgiResponse *self, const Request *request) {
    SraCgiResponseStatus(self, 500);
    const char header[] = "text/plain"; // application/octet-stream";
    int error = SraCgiResponseHeader(self, "Content-type",
        header, sizeof header - 1);
    if (error < 0) {
        return error;
    }
    {
        const char header[] = "Thu, 23 Jan 2014 23:59:59 GMT";
        error = SraCgiResponseHeader(self, "Expires",
            header, sizeof header - 1);
        if (error < 0) {
            return error;
        }
    }
    SraCgiResponseWrite(self, "\n", 1);
    return 0;
}

static bool equals(const char *start, size_t asize,
    const char *b, size_t bsize)
{
    size_t i = 0;

    if (asize < bsize) {
        return false;
    }

    for (i = 0 ; i < bsize; ++i) {
        if (start[i] != b[i]) {
            return false;
        }
    }

    return true;
}

static void RequestInit(Request *self, char *start, size_t size) {
    char *end = NULL;
    char *a = NULL;
    char *e = NULL;
    const char nBits  [] = "bits";
    const char nCmd   [] = "cmd";
    const char nOsArch[] = "os_arch";
    const char nOsName[] = "os_name";
    const char nOsVers[] = "os_version";
    const char nVers  [] = "version";
    const char nX     [] = "x";
    assert(self && start);
    end = start + size;
    memset(self, 0, sizeof *self);
    self->dbg.req = strdup(start);
    while (true) {
        size_t d = 0;
        a = strchr(start, '&');
        if (a == NULL) {
            a = end;
        }
        e = strchr(start, '=');
        if (e == NULL) {
            return;
        }
        if (e > a) {
            return;
        }
        *e = '\0';
        *a = '\0';
        ++e;
        if (equals(start, sizeof nBits - 1, nBits, sizeof nBits - 1)) {
            if (e < end) {
                const char b32[] = "32";
                const char b64[] = "64";
                if (equals(e, sizeof b32 - 1, b32, sizeof b32 - 1)) {
                   self->bits = eDM32;
                }
                else if (equals(e, sizeof b64 - 1, b64, sizeof b64 - 1)) {
                   self->bits = eDM64;
                }
            }
        }
        else if (equals(start, sizeof nCmd - 1, nCmd, sizeof nCmd - 1)) {
            if (e < end) {
                const char l[] = "lib";
                const char t[] = "tst";
                if (equals(e, sizeof l - 1, l, sizeof l - 1)) {
                   self->cmd = eCmdLib;
                }
                else if (equals(e, sizeof t - 1, t, sizeof t - 1)) {
                   self->cmd = eCmdTst;
                }
            }
        }
        else if (equals(start,
            sizeof nOsArch - 1, nOsArch, sizeof nOsArch - 1))
        {
            if (e < end) {
                self->os_arch = e;
            }
        }
        else if (equals(start,
            sizeof nOsName - 1, nOsName, sizeof nOsName - 1))
        {
            if (e < end) {
                self->os_name = e;
            }
        }
        else if (equals(start,
            sizeof nOsVers - 1, nOsVers, sizeof nOsVers - 1))
        {
            if (e < end) {
                self->os_version = e;
            }
        }
        else if (equals(start,
            sizeof nVers - 1, nVers, sizeof nVers - 1))
        {
            if (e < end) {
                self->version = e;
            }
        }
        else if (equals(start, sizeof nX - 1, nX, sizeof nX - 1)) {
            if (e < end) {
                const char *c = e;
                for (c = e; c < end && c < a; ++c) {
                    switch (*c) {
                        case 'n':
                            self->dbg.printFName = self->dbg.dbg = true;
                            break;
                        case 'P':
                            self->dbg.printReq = self->dbg.dbg = true;
                            break;
                        case 'T':
                            self->dbg.trace = true;
                            break;
                    }
                }
            }
        }
        d = ++a - start;
        if (size >= d) {
            size -= d;
            if (size == 0) {
                break;
            }
        }
        else {
            break;
        }
        start = a;
    }
}

/* TODO http://en.wikipedia.org/wiki/List_of_HTTP_status_codes#5xx_Server_Error
http://intranet.ncbi.nlm.nih.gov/ieb/ToolBox/CPP_DOC/lxr/source/include/cgi/ncbicgir.hpp#L52
add CCgiResponse::SetStatus call
make sure headers are written even without SraCgiResponseHeader call */

int VdbCgiFunction(SraCgiRequest *req, SraCgiResponse *resp) {
    Request request;
    if (req == NULL && resp == NULL) {
        return 1000;
    }

    TVdbCgiFunction *f = VdbCgiFunction;
    int error = f(NULL, NULL);
    if (error != 1000) {
        return 1001;
    }

    char in[4096];
    ssize_t sz = SraCgiRequestRead(req, in, sizeof in);
    if (sz < 0) {
        return req->err;
    }

    if (sz >= sizeof in) {
        return 1002;
    }
    in[sz] = '\0';

    RequestInit(&request, in, sz);

/*  const char header[] = "text/plain"; // application/octet-stream";
    error = SraCgiResponseHeader(resp, "Content-type",
        header, sizeof header - 1);
    if (error < 0) {
        return error;
    }*/

    int r = 0;
    switch (request.cmd) {
        case eCmdLib:
            r = _SraCgiResponseLib(resp, &request);
            break;
        case eCmdTst:
            r = _SraCgiResponseTst(resp, &request);
            break;
        default: {
            const char buffer[] = "1\n";
            ssize_t i = SraCgiResponseWrite(resp, buffer, sizeof buffer - 1);
            if (i < 0) {
                r = resp->err;
            }
            SraCgiResponseStatus(resp, 501);
            break;
        }
    }
    RequestFini(&request);
    return r;
}
