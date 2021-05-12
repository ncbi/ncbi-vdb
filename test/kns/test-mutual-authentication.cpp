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
* =========================================================================== */

#include <kapp/main.h> // KMain

#include <kfs/directory.h> /* KDirectoryRelease */
#include <kfs/file.h> /* KFileRelease */

#include <klib/out.h> // KOutMsg
#include <klib/rc.h> // RC

#include <kns/http.h> // KNSManagerMakeHttpFile
#include <kns/manager.h> // KNSManagerRelease

#include <iostream> // cerr

using std::cerr;

static rc_t KDirectory_Load(const KDirectory * self,
    const char * path1, const char * path2, char ** buffer)
{
    const KFile * file = NULL;
    rc_t rc = KDirectoryOpenFileRead(self, &file, path1, path2);

    uint64_t s = 0;
    if (rc == 0)
        rc = KFileSize(file, &s);

    assert(buffer);
    if (rc == 0)
        *buffer = (char*)calloc(1, s + 1);
    if (rc == 0 && *buffer == NULL)
        rc = 1;

    size_t num_read = 0;
    if (rc == 0)
        rc = KFileRead(file, 0, *buffer, s + 1, &num_read);

    rc_t r2 = KFileRelease(file);
    if (rc == 0 && r2 != 0)
        rc = r2;

    return rc;
}

static bool LoadOwnCert(const char * location,
    const char ** own_cert, const char ** pk_key)
{
    char ** cert = (char**)own_cert;
    char ** key = (char**)pk_key;
    assert(cert && key);

    if (location == NULL)
        return false;

    KDirectory * dir = NULL;
    rc_t rc = KDirectoryNativeDir(&dir);

    if (rc == 0)
        rc = KDirectory_Load(dir, "%s/own_cert", location, cert);

    if (rc == 0)
        rc = KDirectory_Load(dir, "%s/pk_key", location, key);

    KDirectoryRelease(dir);

    return rc == 0;
}

#define S_ALIAS  "n"
#define S_OPTION "no-singleton"
static
const char * S_USAGE[] = { "not to use KNSManager singleton", NULL };

static
OptDef Options[] = {
    /* name      alias    help_gen help max_count needs_value required*/
     { S_OPTION, S_ALIAS, NULL, S_USAGE,    1,       false,   false  },
};

ver_t CC KAppVersion(void) { return 0; }

const char UsageDefaultName[] = "test-mutual-authentication";

rc_t CC UsageSummary(const char * progname) {
    return KOutMsg(
     "Usage:\n"
     "  %s [options] <DIR> <URL> [<DIR> <URL> ...]\n"
     "  where\n"
     "    <DIR> is path to directory with files named 'own_cert' and 'pk_key'\n"
     "    use X to skip sending client certificate\n"
     "\n"
     "Summary:\n"
     "  Program to test mutual TLS Authentication\n",
        progname);
}

rc_t CC Usage(const struct Args * args) {
    rc_t rc = 0;
    unsigned i = 0;

    const char * progname = UsageDefaultName;
    const char * fullpath = UsageDefaultName;

    if (args == NULL)
        rc = RC(rcApp, rcArgv, rcAccessing, rcSelf, rcNull);
    else
        rc = ArgsProgram(args, &fullpath, &progname);
    if (rc)
        progname = fullpath = UsageDefaultName;

    UsageSummary(progname);

    OUTMSG(("\nOptions:\n"));
    for (i = 0; i < sizeof Options / sizeof Options[0]; i++) {
        const OptDef * o = &Options[i];
        assert(o);
        HelpOptionLine(o->aliases, o->name, NULL, o->help);
    }
    OUTMSG(("\n"));

    HelpOptionsStandard();

    HelpVersion(fullpath, KAppVersion());

    return rc;
}

rc_t MutualConnection(const char * own_cert, const char * pk_key,
    const char * url, const char * host, uint32_t port);

rc_t CC KMain(int argc, char *argv[]) {
    if (argc == 7 && argv[6][0] == '-') {
        char * own_cert = NULL;
        char * pk_key = NULL;

        rc_t rc = 0;
        if (argv[1][0] != 'X' || argv[1][1] != '\0')
            rc = KDirectory_Load(NULL, "%s", argv[1], &own_cert);

        if (rc == 0)
            rc = KDirectory_Load(NULL, "%s", argv[2], &pk_key);

        if (rc == 0)
            rc = MutualConnection(
                own_cert, pk_key, argv[3], argv[4], atoi(argv[5]));

        free(own_cert);
        free(pk_key);

        return rc;
    }

    bool singleton = true;

    Args * args = NULL;
    rc_t rc = ArgsMakeAndHandle(
        &args, argc, argv, 1, Options, sizeof Options / sizeof Options[0]);
    if (rc != 0)
        return rc;

    uint32_t pcount = 0;
    do {
        rc = ArgsOptionCount(args, S_OPTION, &pcount);
        if (rc != 0)
            break;
        if (pcount > 0)
            singleton = false;
    } while (false);

    KNSManager * mgr = NULL;
    if (rc == 0)
        rc = singleton ? KNSManagerMake(&mgr) : KNSManagerMakeLocal(&mgr, 0);

    rc = ArgsParamCount(args, &pcount);
    if (rc == 0 && pcount == 0) {
        UsageSummary(argv[0]);
        rc = 1;
    }

    for (uint32_t i = 0; rc == 0 && i < pcount; ++i) {
        if (i > 1 && !singleton) {
            rc_t r2 = KNSManagerRelease(mgr);
            mgr = NULL;
            if (r2 != 0 && rc == 0)
                rc = r2;
            if (rc == 0)
                rc = KNSManagerMakeLocal(&mgr, NULL);
        }
        const char * v = NULL;
        rc = ArgsParamValue(args, i, (const void **)&v);
        if (rc != 0)
            break;
        assert(v);
        if (v[0] != 'X' || v[1] != '\0') {
            const char *own_cert = NULL, *pk_key = NULL;
            if (LoadOwnCert(v, &own_cert, &pk_key)) {
                rc = KNSManagerSetOwnCert(mgr, own_cert, pk_key);
                // second call to KNSManagerSetOwnCert should fail
                if (i > 0 && singleton) {
                    if (rc == 0)
                        rc = 2;
                    else
                        rc = 0;
                }
                if (rc == 0) {
                    rc = KNSManagerSetOwnCert(mgr, own_cert, pk_key);
                    if (rc == 0)
                        rc = 3;
                    else
                        rc = 0;
                }
            }
            free(const_cast<void*>(static_cast<const void*>(own_cert)));
            free(const_cast<void*>(static_cast<const void*>(pk_key)));
        }

        if (rc == 0 && ++i < pcount) {
            rc = ArgsParamValue(args, i, (const void **)&v);
            if (rc == 0) {
                const KFile * file = NULL;
                rc = KNSManagerMakeHttpFile(
                    mgr, &file, NULL, 0x01010000, "%s", v);
                rc_t r2 = KFileRelease(file);
                if (r2 != 0 && rc == 0)
                    rc = r2;
            }
        }
    }

    rc_t r2 = KNSManagerRelease(mgr);
    if (r2 != 0 && rc == 0)
        rc = r2;

    r2 = ArgsWhack(args);
    if (r2 != 0 && rc == 0)
        rc = r2;

    return rc;
}
