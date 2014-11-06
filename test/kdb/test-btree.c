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

/* broken test - this added just to make it compile */

#include <kapp/main.h>
#include <kapp/args.h>
#include <kfs/directory.h>
#include <kfs/pagefile.h>
#include <kfs/file.h>
#include <kfs/mmap.h>
#include <kdb/btree.h>
#include <klib/token.h>
#include <klib/log.h>
#include <klib/out.h>
#include <klib/debug.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define USE_EXTERN_TEXT_CMP 0


typedef struct test_btree_stat test_btree_stat;
struct test_btree_stat
{
    uint32_t type;
    uint32_t first_seen;
    uint32_t count;
};

static
void test_btree_stat_report ( const void *key, size_t key_size, KBTreeValue *val, void *ignore )
{
    size_t bytes;
    const test_btree_stat *stat;
    rc_t rc = KBTreeValueAccessRead ( val, ( const void** ) & stat, & bytes );
    if ( rc != 0 )
        LOGERR ( klogInt, rc, "KBTreeValueAccessRead failed" );
    else if ( bytes != sizeof * stat )
        LOGMSG ( klogInt, "KBTreeValueAccessRead returned wrong size" );
    else
    {
        printf ( "key: '%.*s'\n", ( int ) key_size, ( const char* ) key );
        printf ( "  type %u\n", stat -> type );
        printf ( "  first seen on line %u\n", stat -> first_seen );
        printf ( "  occurs %u times.\n\n", stat -> count );
    }
}

#if USE_EXTERN_TEXT_CMP
static
int bt_text_cmp ( const void *a, size_t asize, const void *b, size_t bsize )
{
    if ( asize < bsize )
        return -1;
    if ( asize > bsize )
        return 1;
    return memcmp ( a, b, asize );
}
#else
#define bt_text_cmp NULL
#endif

static
rc_t test_readonly1 ( const KDirectory *dir, const KFile *in )
{
    const KFile *backing;
    rc_t rc = KDirectoryOpenFileRead ( dir, & backing, "test-btree.out" );
    if ( rc == 0 )
    {
        const KBTree *bt;
        rc = KBTreeMakeRead ( & bt, backing, 256 * 1024 * 1024, bt_text_cmp );
        if ( rc != 0 )
            LOGERR ( klogInt, rc, "KBTreeMakeRead failed" );
        else
        {
            rc = KBTreeForEach ( bt, false, test_btree_stat_report, NULL );
            if ( rc != 0 )
                LOGERR ( klogInt, rc, "KBTreeForEach failed" );

            KBTreeRelease ( bt );
        }

        KFileRelease ( backing );
    }
    return rc;
}

static
rc_t test_readonly ( const KDirectory *dir, const KFile *in )
{
    rc_t rc;

    rc = test_readonly1 ( dir, in );

    return rc;
}

static
rc_t make_text_entry ( KBTree *bt, const char *key, size_t key_size, int id, uint32_t lineno )
{
    static int entry = 0;

    KBTreeValue val;
    bool was_inserted;
    test_btree_stat *stat;
    rc_t rc = KBTreeEntry ( bt, & val, & was_inserted, sizeof * stat, key, key_size );
    ++ entry;
    if ( rc != 0 )
    {
        LOGERR ( klogInt, rc, "KBTreeEntry failed" );
        printf ( "failed on entry %d\n", entry );
        fflush ( stdout );
    }
    else
    {
        size_t bytes;
        rc = KBTreeValueAccessUpdate ( & val, ( void** ) & stat, & bytes );
        if ( rc != 0 )
            LOGERR ( klogInt, rc, "KBTreeValueAccessUpdate failed" );
        else
        {
            if ( bytes != sizeof * stat )
                rc = -1;
            else if ( was_inserted )
            {
                stat -> type = id;
                stat -> first_seen = lineno;
                stat -> count = 1;
            }
            else
            {
                ++ stat -> count;
            }
        }

        KBTreeValueWhack ( & val );
    }
    return rc;
}

static
rc_t test_update1 ( KDirectory *dir, const KFile *in, const char *path )
{
    KFile *out;
    rc_t rc = KDirectoryCreateFile ( dir, & out, true, 0666, kcmInit, "test-btree.out" );
    if ( rc == 0 )
    {
        size_t pgsize = KPageConstSize ();
        size_t key_max = ( pgsize > 1024 ) ? 255 : ( pgsize - 8 - 24 ) / 2;

        KBTree *bt;
        rc = KBTreeMakeUpdate ( & bt, out,
            256 * 1024 * 1024, false, kbtOpaqueKey,
            8, sizeof ( test_btree_stat ),
            1, key_max,
            sizeof ( test_btree_stat ), sizeof ( test_btree_stat ),
            bt_text_cmp );
        if ( rc != 0 )
            LOGERR ( klogInt, rc, "KBTreeMakeUpdate failed" );
        else
        {
            const KMMap *mm;
            rc = KMMapMakeRead ( & mm, in );
            if ( rc == 0 )
            {
                const void *addr;
                rc = KMMapAddrRead ( mm, &addr );
                if ( rc == 0 )
                {
                    size_t size;
                    rc = KMMapSize ( mm, & size );
                    if ( rc == 0 )
                    {
                        char buffer [ 1024 ];

                        /* have a memory mapped input file, and it's text
                           ( er, at least that's the idea *. */
                        KToken t;
                        KTokenText ttxt;
                        KTokenSource src;
                        String sstr, pstr;

                        /* test also assumes ASCII - sorry */
                        StringInitCString ( & pstr, path );
                        StringInit ( & sstr, addr, size, ( uint32_t ) size );
                        KTokenTextInit ( & ttxt, & sstr, & pstr );
                        KTokenSourceInit ( & src, & ttxt );

                        /* okay, now parse the file */
                        while ( KTokenizerNext ( kDefaultTokenizer, & src, & t ) -> id != eEndOfInput )
                        {
                            switch  ( t . id )
                            {
                            case eUntermString:
                            case eString:
                            case eUntermEscapedString:
                            case eEscapedString:
                                rc = KTokenToString ( & t, buffer, sizeof buffer, & size );
                                if ( rc != 0 )
                                    rc = 0;
                                else
                                    rc = make_text_entry ( bt, buffer, size, t . id, t . lineno );
                                break;

                            case eIdent:
                            case eName:
                                rc = make_text_entry ( bt, t . str . addr, t . str . size, t . id, t . lineno );
                                break;
                            }
                        }
                    }
                }

                KMMapRelease ( mm );
            }

            KBTreeRelease ( bt );
        }

        KFileRelease ( out );
    }
    return rc;
}

static
rc_t test_update ( KDirectory *dir, const KFile *in, const char *path )
{
    rc_t rc;

    rc = test_update1 ( dir, in, path );

    return rc;
}

ver_t CC KAppVersion ( void )
{
    return 0;
}


const char UsageDefaultName[] = "vdb-unlock";

rc_t CC UsageSummary ( const char *progname )
{
    return KOutMsg ( "\n"
                     "Usage:\n"
                     "  %s [Options] <target>\n"
                     "\n"
                     "Summary:\n"
                     "  test the btree.\n"
                     , progname
        );
}

rc_t CC Usage ( const Args *args )
{
    const char * progname = UsageDefaultName;
    const char * fullpath = UsageDefaultName;
    rc_t rc;

    if (args == NULL)
        rc = RC (rcApp, rcArgv, rcAccessing, rcSelf, rcNull);
    else
        rc = ArgsProgram (args, &fullpath, &progname);
    if (rc)
        progname = fullpath = UsageDefaultName;

    UsageSummary (progname);

    KOutMsg ("Options:\n");

    HelpOptionsStandard ();

    HelpVersion (fullpath, KAppVersion());

    return rc;
}
rc_t CC KMain ( int argc, char *argv [] )
{
    Args *args;
    rc_t rc = ArgsMakeAndHandle ( & args, argc, argv, 0 );
    if ( rc != 0 )
        LogErr ( klogErr, rc, "failed to parse arguments" );
    else
    {
        KDirectory *wd;

        const char *path = "test-btree.c";
        if ( argc == 2 )
            path = argv [ 1 ];

        rc = KDirectoryNativeDir ( & wd );
        if ( rc == 0 )
        {
            const KFile *in;
            rc = KDirectoryOpenFileRead ( wd, & in, path );
            if ( rc == 0 )
            {
                rc = test_update ( wd, in, path );
                if ( rc == 0 )
                    rc = test_readonly ( wd, in );

                KFileRelease ( in );
            }

            KDirectoryRelease ( wd );
        }
    }
    return rc;
}
