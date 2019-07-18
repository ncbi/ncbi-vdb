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

#include <klib/extern.h> /* MUST BE FIRST */

#include <kapp/args.h> /* OPTION_REPORT */

#include <kfs/directory.h> /* KDirectoryNativeDir */
#include <kfg/kfg-priv.h> /* ENV_KRYPTO_PWFILE */

#include <klib/klib-priv.h>
#include <klib/log.h> /* LOGERR */
#include <klib/misc.h> /* is_iser_an_admin */
#include <klib/namelist.h> /* KNamelistRelease */
#include <klib/out.h> /* OUTMSG */
#include <klib/printf.h> /* string_printf */
#include <klib/rc.h>
#include <klib/report.h> /* ReportInit */
#include <klib/time.h> /* KTime */
#include <klib/text.h> /* string_printf */

#include <atomic.h> /* atomic_test_and_set_ptr */

#include <stdarg.h> /* va_start */
#include <stdio.h> /* sprintf */
#include <stdlib.h> /* malloc */
#include <string.h> /* memset */
#include <limits.h> /* PATH_MAX */
#include <assert.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define RELEASE(type, obj) do { rc_t rc2 = type##Release(obj); \
    if (rc2 && !rc) { rc = rc2; } obj = NULL; } while (false)

/*
 * An unrecoverable error happened.
 * We can help to solve it
 * by reporting information about known application execution environment.
 */

static
rc_t XMLLogger_Encode(const char* src, char *dst,
    size_t dst_sz, size_t *num_writ)
{
    rc_t rc = 0;
    if( src == NULL || dst == NULL || num_writ == NULL ) {
        rc = RC(rcApp, rcArgv, rcParsing, rcParam, rcNull);
    } else {
        *num_writ = 0;
        do {
            size_t bytes;
            const char* p;
            switch(*src) {
                case '\'':
                    bytes = 6;
                    p = "&apos;";
                    break;
                case '"':
                    bytes = 6;
                    p = "&quot;";
                    break;
                case '&':
                    bytes = 5;
                    p = "&amp;";
                    break;
                case '<':
                    bytes = 4;
                    p = "&lt;";
                    break;
                case '>':
                    bytes = 4;
                    p = "&gt;";
                    break;
                default:
                    bytes = 1;
                    p = src;
                    break;
            }
            if( (*num_writ + bytes) > dst_sz ) {
                rc = RC(rcApp, rcLog, rcEncoding, rcBuffer, rcInsufficient);
                break;
            } else {
                memmove(dst, p, bytes);
                *num_writ = *num_writ + bytes;
                dst += bytes;
            }
        } while(*src++ != '\0');
        *num_writ = *num_writ - 1;
    }
    return rc;
}

typedef rc_t (CC ReportObj)(const ReportFuncs *f, uint32_t indent,
                              const char *path, bool *wasDbOrTableSet);
typedef rc_t (CC ReportSoftware)(const ReportFuncs *f, uint32_t indent,
                          const char *argv_0, const char *date, ver_t tool_ver);
typedef rc_t (CC Whack )(void);
typedef struct Report {
    rc_t ( CC * report_redirect ) ( KWrtHandler* handler,
        const char* filename, bool* to_file, bool finalize );
    rc_t ( CC * report_cwd ) ( const ReportFuncs *f, uint32_t indent );
    rc_t ( CC * report_config ) ( const ReportFuncs *f,
        uint32_t indent, uint32_t configNodesSkipCount, va_list args );
    ReportObj *report_vdb;
    ReportSoftware *report_software;
    Whack *whack_vdb;
    atomic_ptr_t object; /* path: to free */
    const char* date;
    char** argv;
    int argc;
    KTime_t start;
    ver_t tool_ver;
    bool hasZombies;
    bool silence;
} Report;

static rc_t ReportReplaceObjectPtr(Report* self, const char* path) {
    char *copy;
    void *prev_ptr, *cur_ptr;

    assert(self);

    copy = string_dup_measure(path, NULL);
    if (copy == NULL)
        return RC(rcExe, rcMemory, rcAllocating, rcMemory, rcExhausted);

    cur_ptr = self -> object . ptr;
    do
    {
        prev_ptr = cur_ptr;
        cur_ptr = atomic_test_and_set_ptr ( & self -> object, copy, prev_ptr );
    }
    while ( cur_ptr != prev_ptr );

    if ( cur_ptr != NULL )
        free ( cur_ptr );

    return 0;
}

static rc_t ReportFreeObject(Report* self) {
    rc_t rc = 0;
    void *prev_ptr, *cur_ptr;

    assert(self);

    cur_ptr = self -> object . ptr;
    do
    {
        prev_ptr = cur_ptr;
        cur_ptr = atomic_test_and_set_ptr ( & self -> object, NULL, prev_ptr );
    }
    while ( cur_ptr != prev_ptr );

    if ( cur_ptr != NULL )
        free(cur_ptr);

    return rc;
}

static atomic_ptr_t report_singleton;
/*static Report * volatile report_singleton;*/

static rc_t ReportGet(Report** self)
{
    if ( report_singleton . ptr == NULL )
        return RC ( rcRuntime, rcStorage, rcAccessing, rcSelf, rcNull );

    * self = report_singleton . ptr;
    return 0;
}
static rc_t ReportRelease(void)
{
    rc_t rc = 0;
    Report *prev_report, *cur_report;

    cur_report = report_singleton . ptr;
    do
    {
        prev_report = cur_report;
        cur_report = atomic_test_and_set_ptr ( & report_singleton, NULL, prev_report );
    }
    while ( cur_report != prev_report );

    if ( cur_report != NULL )
    {

        /* cleanup */
        ReportFreeObject( cur_report );
        if ( cur_report -> whack_vdb != NULL )
            rc = ( * cur_report -> whack_vdb ) ();

        if ( cur_report -> argv != NULL )
        {
            int i;
            for ( i = 0; i < cur_report -> argc; ++ i )
                free ( cur_report -> argv [ i ] );
            free ( cur_report -> argv );
            cur_report -> argv = NULL;
        }

        memset ( cur_report, 0, sizeof * cur_report );
    }

    return rc;
}

static void reportOpen1(uint32_t indent, const char* name) {
    OUTMSG(("%*c%s>", indent + 1, '<', name));
}

static rc_t reportData1(const char* data) {
    rc_t rc = 0;

    char dummy[PATH_MAX * 2 + 1] = "";
    const char* p = data;

    assert(data);

    if (data[0] == 0) {
        return rc;
    }

    if (strpbrk(data, "'\"&<>")) {
        size_t num_writ = 0;
        rc = XMLLogger_Encode(data, dummy, sizeof dummy, &num_writ);
        if (rc == 0)
        {   p = dummy; }
    }

    OUTMSG(("%s", p));

    return rc;
}

static void reportClose1(const char* name) { OUTMSG(("</%s>\n", name)); }

static rc_t reportImpl(int indent, bool open, bool close, bool eol,
    const char* name, const char* data, int count, va_list args)
{
    rc_t rc = 0;
    int i = 0;

    if (count < 0) {
        eol = false;
        count = -count;
    }

    OUTMSG(("%*c", eol ? ( indent + 1 ) : 1, '<'));
    OUTMSG(("%s%s", ( ! open && close ) ? "/" : "", name));

    for (i = 0; i < count; ++i) {
        const char* name = va_arg(args, const char*);
        int format = va_arg(args, int);
        union {
            const char* s;
            int i;
            int64_t i64;
            KTime* t;
            rc_t rc;
            const uint8_t* digest;
            uint64_t u64;
            ver_t vers;
        } u;
        OUTMSG((" %s=\"", name));
        u.i64 = 0;
        switch(format) {
            case 'M':
                u.digest = va_arg(args, const uint8_t*);
                {
                    int i = 0;
                    for (i = 0; i < 16; ++i)
                    {  OUTMSG(("%02x", *(u.digest + i))); }
                }
                break;
            case 'l':
                u.i64 = va_arg(args, int64_t);
                OUTMSG(("%ld", u.i64));
                break;
            case 'u':
                u.u64 = va_arg(args, uint64_t);
                OUTMSG(("%lu", u.u64));
                break;
            case 'R':
                u.rc = va_arg(args, rc_t);
                OUTMSG(("%R", u.rc));
                break;
            case 'T':
                u.t = va_arg(args, KTime*);
                OUTMSG(("%lT", u.t));
                break;
            case 's':
                u.s = va_arg(args, const char*);
                OUTMSG(("%s", u.s));
                break;
            case 'V':
                u.vers = va_arg(args, ver_t);
                OUTMSG(("%V", u.vers));
                break;
            case 'd':
            default:
                u.i = va_arg(args, int);
                OUTMSG(("%d", u.i));
                break;
        }
        OUTMSG(("\""));
    }

    if (data) {
        OUTMSG((">"));
        rc = reportData1(data);
        if (close)
        {   OUTMSG(("</%s", name)); }
    }
    else if (open && close)
    {   OUTMSG(("/")); }

    OUTMSG((">"));
    if (eol)
    {   OUTMSG(("\n")); }

    return rc;
}

static void CC report(uint32_t indent, const char* name, uint32_t count, ...) {
     va_list args;
     va_start(args, count);
     reportImpl(indent, true, true, true, name, NULL, count, args);
     va_end(args);
}

static
void CC reportData(uint32_t indent, const char* name, const char* data, uint32_t count, ...)
{
    va_list args;
    va_start(args, count);
    reportImpl(indent, true, true, true, name, data, count, args);
    va_end(args);
}

static void CC reportOpen(uint32_t indent, const char* name, uint32_t count, ...) {
    va_list args;
    va_start(args, count);
    reportImpl(indent, true, false, true, name, NULL, count, args);
    va_end(args);
}

static void CC reportClose(uint32_t indent, const char* name)
{   reportImpl(indent, false, true, true, name, NULL, 0, NULL); }

static void CC reportError(uint32_t indent, rc_t rc, const char* function) {
    report(indent, "Error", 2,
         "rc"      , 'R', rc,
         "function", 's', function);
}

static void CC reportErrorStrImpl(uint32_t indent, rc_t rc,
    const char* function, const char* name, const char* val, bool eol)
{
    int sign = eol ? 1 : -1;
    if (rc || function) {
        if (name != NULL || val != NULL) {
            report(indent, "Error", sign * 3,
                "rc", 'R', rc,
                "function", 's', function,
                name, 's', val);
        }
        else {
            report(indent, "Error", sign * 2,
                "rc", 'R', rc,
                "function", 's', function);
        }
    }
    else {
        report(indent, "Error", sign,
            name, 's', val);
    }
}

static void CC reportErrorStr(uint32_t indent, rc_t rc, const char* function,
    const char* name, const char* val)
{   reportErrorStrImpl(indent, rc, function, name, val, true); }

static void CC reportErrorStrInt(uint32_t indent, rc_t rc, const char* function,
    const char* names, const char* vals, const char* namei, uint32_t vali)
{
    report(indent, "Error", 4,
        "rc"      , 'd', rc,
        "function", 's', function,
        names     , 's', vals,
        namei     , 'd', vali);
}

static void CC reportError3Str(uint32_t indent, rc_t rc, const char* function,
    const char* name, const char* v1, const char* v2, const char* v3,
    bool eol)
{
    char* buffer = malloc(strlen(v1) + strlen(v2) + strlen(v3) + 1);
    if (buffer) {
        sprintf(buffer, "%s%s%s", v1, v2, v3);
        reportErrorStrImpl(indent, rc, function, name, buffer, eol);
        free(buffer);
    }
}

static ReportFuncs report_funcs =
{
    report,
    reportData,
    reportData1,
    reportOpen,
    reportOpen1,
    reportClose,
    reportClose1,
    reportError,
    reportErrorStr,
    reportErrorStrImpl,
    reportErrorStrInt,
    reportError3Str
};

static rc_t ReportRun(int indent, rc_t rc_in) {
    rc_t rc = 0;

    const char tag[] = "Run";

    Report* self = NULL;
    ReportGet(&self);
    if (self == NULL) {
        return rc;
    }

    reportOpen(indent, tag, 0);

    {
        KTime kt;
        const char tag[] = "Date";
        reportOpen(indent + 1, tag, 0);
        KTimeLocal(&kt, self->start);
        report(indent + 2, "Start", 1, "value", 'T', &kt);
        KTimeLocal(&kt, KTimeStamp());
        report(indent + 2, "End"  , 1, "value", 'T', &kt);
        reportClose(indent + 1, tag);
    }

    {
        const char* name = "HOME";
        const char* val = getenv(name);
        if (val == NULL) {
            name = "USERPROFILE";
            val = getenv(name);
        }
        if (val == NULL)
        {   name = val = "not found"; }
        report(indent + 1, "Home", 2, "name", 's', name, "value", 's', val);
    }

    if ( self -> report_cwd != NULL )
        rc = ( * self -> report_cwd ) ( & report_funcs, indent + 1 );

    {
        const char tag[] = "CommandLine";
        int i = 0;
        reportOpen(indent + 1, tag, 1, "argc", 'd', self->argc);
        for (i = 0; i < self->argc; ++i) {
            if (self->argv && self->argv[i]) {
                report(indent + 2, "Arg", 2, "index", 'd', i,
                    "value", 's', self->argv[i]);
            }
        }
        reportClose(indent + 1, tag);
    }

    report(indent + 1, "Result", 1, "rc", 'R', rc_in);

    report(indent + 1, "User", 1,
        "admin", 's', is_iser_an_admin() ? "true" : "false");

    reportClose(indent, tag);

    return rc;
}

static rc_t ReportEnv(int indent) {
    rc_t rc = 0;

    const char tag[] = "Env";

    const char * env_list [] = {
        "all_proxy",
        "ALL_PROXY",
        "http_proxy",
        "HTTP_PROXY",
        "KLIB_CONFIG",
        "VDBCONFIG",
        "VDB_CONFIG",
        ENV_KRYPTO_PWFILE,
    };

    int i = 0;

    reportOpen(indent, tag, 0);

    for (i = 0; i < sizeof env_list / sizeof env_list[0]; ++i) {
        const char *val = getenv(env_list[i]);
        if (val != NULL) {
            report(indent + 1, env_list[i], 1, "value", 's', val);
        }
    }

    reportClose(indent, tag);

    return rc;
}

/* Silence
 *  tell report to be silent at exit
 *  useful especially in response to ^C
 */
LIB_EXPORT void CC ReportSilence ( void )
{
    Report* self = NULL;
    ReportGet(&self);
    if ( self != NULL )
        self -> silence = true;
}

static rc_t reportToFileSet(const Report* self,
    KWrtHandler* old_handler, const char* name, bool* to_file)
{
    rc_t rc = 0;
    assert(self && to_file);
    if (self->report_redirect) {
        rc = (*self->report_redirect)(old_handler, name, to_file, false);
    }
    return rc;
}

static rc_t reportToStdErrSet(const Report* self, KWrtHandler* old_handler) {
    rc_t rc = 0;
    assert(self);
    if (self->report_redirect) {
        rc = (*self->report_redirect)(old_handler, NULL, NULL, false);
    }
    return rc;
}

/* Finalize:
 * If (rc_in != 0) then report environment information.
 * Then clean itself.
 *
 * When "--" OPTION_REPORT "always" command line argument is passed
 * then the report [usually] goes to strerr.
 *
 * When aForce == true then the report goes to strout:
 * it is done to insert it into test-sra output.
 */
static rc_t _ReportFinalize
    (rc_t rc_in, bool aForce, uint32_t configNodesSkipCount, ...)
{
    rc_t rc = 0;

    va_list args;

    const char* report_arg = NULL;

    bool force = rc_in != 0;

    Report* self = NULL;
    ReportGet(&self);

    if (self == NULL) {
        return rc;
    }

    va_start(args, configNodesSkipCount);

    if (GetRCTarget(rc_in) == rcArgv) {
        force = false;
    }
    else {
        bool wasDbOrTableSet = true;
        if (self->report_vdb != NULL) {
            (*self->report_vdb)(&report_funcs, 0, NULL, &wasDbOrTableSet);
            if (!wasDbOrTableSet) {
                force = false;
            }
        }
    }

    if (!force && aForce) {
        force = true;
        self -> silence = false;
    }

    if (self->argv) {
        int i = 0;
        for (i = 1; i < self->argc; ++i) {
            if (strcmp("--" OPTION_REPORT, self->argv[i]) == 0) {
                if (i + 1 < self->argc) {
                    report_arg = self->argv[i + 1];
                }
                break;
            }
        }
    }

    if (report_arg && strcmp("always", report_arg) == 0) {
        force = true;
        self -> silence = false;
    }

    if (!self -> silence) {
        if (force) { 
            if (report_arg && strcmp("never", report_arg) == 0) {
                force = false;
            }
        }
/*                              PLOGERR(klogErr, (klogErr,
                               RC(rcApp, rcArgv, rcParsing, rcRange, rcInvalid),
                               "report type '$(type)' is unrecognized",
                               "type=%s", self->argv[i + 1]));*/

        if (force) {
            if (self->hasZombies && !aForce) {
                KOutHandlerSetStdErr();
                if (self->object.ptr != NULL) {
                    OUTMSG(("\nThe archive '%s' may be truncated: "
                        "Please download it again.\n", self->object.ptr));
                }
                else {
                    OUTMSG(("\nThe archive may be truncated: "
                        "Please download it again.\n"));
                }
            }
            else {
                int indent = 0;
                const char tag[] = "Report";

                char path[PATH_MAX + 1] = "";
                bool to_file = false;
                KWrtHandler old_handler;
                if (rc_in == 0) {
                    if (!aForce) {
                        reportToStdErrSet(self, &old_handler);
                    }
                }
                else {
                    rc_t rc2 = 0;
                    const char name[] = "ncbi_error_report.txt";
                    const char* home = getenv("HOME");
                    if (home == NULL) {
                        home = getenv("USERPROFILE");
                    }
                    if (home) {
                        size_t num_writ = 0;
                        rc2 = string_printf
                            (path, sizeof path, &num_writ, "%s/%s", home, name);
                        assert(num_writ < sizeof path);
                    }
                    if (rc2 != 0 || home == NULL) {
                        size_t num_writ = 0;
                        rc2 = string_printf
                            (path, sizeof path, &num_writ, "%s", name);
                        assert(num_writ < sizeof path);
                        assert(rc2 == 0);
                    }
                    reportToFileSet(self, &old_handler, path, &to_file);
                }
                if (!to_file && rc_in) {
                    OUTMSG((
"\n"
"=============================================================\n"
"An error occurred during processing.\n"
"If the problem persists, you may consider sending the following XML document\n"
"to 'sra-tools@ncbi.nlm.nih.gov' for assistance.\n"
"=============================================================\n\n"));
                }

                reportOpen(indent, tag, 0);

                {
                    rc_t rc2 = ReportRun(indent + 1, rc_in);
                    if (rc == 0 && rc2 != 0)
                    {   rc = rc2; }
                }

                if ( self -> report_config != NULL )
                {
                    rc_t rc2 = ( * self -> report_config ) ( & report_funcs,
                        indent + 1, configNodesSkipCount, args );
                    if (rc == 0 && rc2 != 0)
                    {   rc = rc2; }
                }

                if (self -> report_vdb != NULL) {
                    rc_t rc2 = (*self->report_vdb)
                        (&report_funcs, indent + 1, self -> object . ptr, NULL);
                    if (rc == 0 && rc2 != 0) {
                        rc = rc2;
                    }
                }

                if ( self -> report_software != NULL )
                {
                    const char *argv_0 = self -> argv ? self -> argv [ 0 ] : "";
                    rc_t rc2 = ( * self -> report_software )
                        ( & report_funcs, indent + 1,
                            argv_0, self -> date, self -> tool_ver );
                    if (rc == 0 && rc2 != 0)
                    {   rc = rc2; }
                }

                {
                    rc_t rc2 = ReportEnv(indent + 1);
                    if (rc == 0 && rc2 != 0)
                    {   rc = rc2; }
                }

                if (self->hasZombies) {
                    OUTMSG(("\nOne of archives may be truncated: "
                        "It should be redownloaded.\n"));
                }

                reportClose(indent, tag);
                reportData1("\n");

                if (to_file) {
                    KOutHandlerSetStdErr();
                    OUTMSG((
"\n"
"=============================================================\n"
"An error occurred during processing.\n"
"A report was generated into the file '%s'.\n"
"If the problem persists, you may consider sending the file\n"
"to 'sra-tools@ncbi.nlm.nih.gov' for assistance.\n"
"=============================================================\n\n", path));
                }
            }
        }
    }

    ReportRelease();

    va_end(args);

    return rc;
}

/* Finalize:
 * If (rc_in != 0) then report environment information.
 * Then clean itself.
 */
LIB_EXPORT rc_t CC ReportFinalize(rc_t rc_in) {
    return _ReportFinalize(rc_in, false, 0);
}

LIB_EXPORT rc_t CC ReportForceFinalize(void) {
    return _ReportFinalize(0, true, 1, "VDBCOPY");
}

static
char **copy_argv ( int argc, char **argv )
{
    char **argv2 = calloc ( argc, sizeof * argv2 );
    if (argv2 != NULL) {
        int i = 0;
        for (i = 0; i < argc; ++i) {
            argv2[i] = string_dup_measure(argv[i], NULL);
        }
    }
    return argv2;
}

/* Init: initialize with common information */
LIB_EXPORT void CC ReportInit(int argc, char* argv[], ver_t tool_version)
{
    static bool latch;
    if ( ! latch )
    {
        /* comes pre-zeroed by runtime */
        static Report self;

        /* initialize with non-zero parameters */
        self.start = KTimeStamp();
        self.argc = argc;
        self.argv = copy_argv ( argc, argv );
        self.date = __DATE__;
        self.tool_ver = tool_version;
        self.silence = false;

        /* export to the remainder of the source */
        report_singleton . ptr = & self;

        latch = true;
    }
}


/* BuildDate
 *  set the build date of the tool
 *
 *  "date" [ IN ] - pre-processor __DATE__
 */
LIB_EXPORT void CC ReportBuildDate ( const char *date )
{
    Report* self = NULL;
    ReportGet(&self);
    if ( self != NULL )
        self -> date = date;
}


/* InitCWD
 */
LIB_EXPORT void CC ReportInitKFS (
    rc_t ( CC * report_cwd ) ( const ReportFuncs *f, uint32_t indent ),
    rc_t ( CC * report_redirect ) ( KWrtHandler* handler,
        const char* filename, bool* to_file, bool finalize ) )
{
    Report* self = NULL;
    ReportGet(&self);
    if ( self != NULL ) {
        self -> report_redirect = report_redirect;
        self -> report_cwd = report_cwd;
    }
}


/* InitConfig
 */
LIB_EXPORT const char* CC ReportInitConfig ( rc_t ( CC * report )
    ( const ReportFuncs *f, uint32_t indent,
      uint32_t configNodesSkipCount, va_list args ) )
{
    Report* self = NULL;
    ReportGet(&self);
    if ( self != NULL )
    {
        self -> report_config = report;
        return self -> argv [ 0 ];
    }
    return NULL;
}

/* InitVDB
 */
LIB_EXPORT rc_t CC ReportInitVDB(
    ReportObj *report_obj, ReportSoftware *report_software, Whack *whack)
{
    Report* self = NULL;
    rc_t rc = ReportGet(&self);
    if ( rc == 0 && self)
    {
        self -> report_vdb = report_obj;
        self -> report_software = report_software;
        self -> whack_vdb = whack;
    }
    return rc;
}

LIB_EXPORT rc_t CC ReportResetObject(const char* path)
{
    Report* self = NULL;

    if ( path == NULL )
        path = "NULL";
    else if ( path [ 0 ] == 0 )
        path = "EMPTY";

    ReportGet(&self);
    if (self == NULL) {
        return 0;
    }

    return ReportReplaceObjectPtr(self, path);
}

LIB_EXPORT void CC ReportRecordZombieFile(void)
{
    Report* self = NULL;
    ReportGet(&self);
    if (self == NULL)
    {   return; }
    self->hasZombies = true;
}
