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

#include <vfs/extern.h>

#undef LIB_EXPORT
#define LIB_EXPORT /* ignore me */
#undef VFS_EXTERN
#define VFS_EXTERN /* ignore me */

#include "../libs/vfs/path.c"

#include <klib/out.h>
#include <klib/status.h> /* STSMSG */

#include <kfg/config.h>
#include <kapp/args.h>
#include <kapp/main.h>

static
const char *VPathGetAccTypeStr ( const VPath * self )
{
    switch ( self -> acc_code >> 8 )
    {
    case 0x000:
        return "";

    case 0x015:
    case 0x026:
    case 0x106:
    case 0x126:
        return " ( appREFSEQ )";

    case 0x109:
        if ( ( self -> acc_code & 0xF ) != 0 )
                return " ( appNANNOT )";
        if ( self -> path . addr [ 0 ] == 'G' &&
             self -> path . addr [ 1 ] == 'C' &&
             ( self -> path . addr [ 2 ] == 'A' || self -> path . addr [ 2 ] == 'F' ) &&
             self -> path . addr [ 3 ] == '_' )
        {
            return " ( appNANNOT )";
        }
        return " ( appREFSEQ )";

    case 0x036:
    case 0x037:
    case 0x038:
    case 0x039:
        if ( ( (self -> acc_code >> 4) & 0xF ) == 0 )
            return " ( appSRA )";
            
        {
            const char pileup_ext[] = ".pileup";
            size_t pileup_ext_size = sizeof( pileup_ext ) / sizeof( pileup_ext[0] ) - 1;
            size_t path_size = self -> path . size;
            
            
            if ( path_size > pileup_ext_size && memcmp(&self -> path . addr[path_size - pileup_ext_size], pileup_ext, pileup_ext_size) == 0)
            {
                return " ( appSRAPileup )";
            }
        }
            
        return " ( appAny )";

    case 0x042:
    case 0x048:
    case 0x049:
    case 0x142:
    case 0x148:
    case 0x149:
        return " ( appWGS )";

    case 0x029:
        if ( self -> acc_code == 0x02910 )
        {
            if ( self -> path . addr [ 0 ] == 'N' &&
                 self -> path . addr [ 1 ] == 'A' )
            {
                return " ( appNANNOT )";
            }
        }

        /* no break */
    default:
        return " ( unrecognized )";
    }
}


static
const char *VPathGetSchemeTypeStr ( const VPath * self )
{
    switch ( self -> scheme_type )
    {
#define CASE( x ) \
        case x: return # x

    CASE ( vpuri_none );
    CASE ( vpuri_not_supported );
    CASE ( vpuri_ncbi_file );
    CASE ( vpuri_file );
    CASE ( vpuri_ncbi_acc );
    CASE ( vpuri_http );
    CASE ( vpuri_https );
    CASE ( vpuri_ftp );
    CASE ( vpuri_ncbi_legrefseq );
    CASE ( vpuri_ncbi_obj );
    CASE ( vpuri_fasp );

#undef CASE
    }

    return "<INVALID>";
}


static
const char *VPathGetPathTypeStr ( const VPath * self )
{
    switch ( self -> path_type )
    {
#define CASE( x ) \
        case x: return # x

    CASE ( vpInvalid );
    CASE ( vpOID );
    CASE ( vpAccession );
    CASE ( vpNameOrOID );
    CASE ( vpNameOrAccession );
    CASE ( vpName );
    CASE ( vpRelPath );
    CASE ( vpFullPath );
    CASE ( vpUNCPath );
    CASE ( vpHostName );
    CASE ( vpEndpoint );

#undef CASE
    }

    return "<INVALID>";
}


static
const char *VPathGetHostTypeStr ( const VPath * self )
{
    switch ( self -> host_type )
    {
#define CASE( x ) \
        case x: return # x

    CASE ( vhDNSName );
    CASE ( vhIPv4 );
    CASE ( vhIPv6 );

#undef CASE
    }

    return "<INVALID>";
}

static
void VPathDisplay ( const VPath * self )
{
    if ( self == NULL ) {
        STSMSG(1, ( "<null>\n" ) );
        STSMSG(1, ( "\n" ) );
    }
    else
    {
        size_t num_read;
        char buffer [ 8192 ];
        rc_t rc = VPathReadUri ( self, buffer, sizeof buffer, & num_read );
        uint32_t size = ( rc == 0 ) ? ( uint32_t ) num_read
            : ( GetRCState ( rc ) == rcInsufficient ) ? sizeof buffer : 0;

        STSMSG(1, ("data        :  %lu bits, %lu bytes ( including NUL )\n"
            , self -> data . elem_bits, self -> data . elem_count));
        STSMSG(1, ("scheme      : '%S' ( size = %zu, len = %u )\n"
            , & self -> scheme, self -> scheme . size, self -> scheme . len));
        STSMSG(1, ("auth        : '%S' ( size = %zu, len = %u )\n"
            , & self -> auth, self -> auth . size, self -> auth . len));
        STSMSG(1, ("host        : '%S' ( size = %zu, len = %u )\n"
            , & self -> host, self -> host . size, self -> host . len));
        STSMSG(1, ("portname    : '%S' ( size = %zu, len = %u )\n"
        , & self -> portname, self -> portname . size, self -> portname . len));
        STSMSG(1, ("path        : '%S' ( size = %zu, len = %u )\n"
        , & self -> path, self -> path . size, self -> path . len));
        STSMSG(1, ("query       : '%S' ( size = %zu, len = %u )\n"
            , & self -> query, self -> query . size, self -> query . len));
        STSMSG(1, ("fragment    : '%S' ( size = %zu, len = %u )\n"
            , & self -> fragment, self -> fragment . size, self -> fragment . len));
        STSMSG(1, ("obj_id      :  %u\n", self -> obj_id));
        STSMSG(1, ("acc_code    :  0x%X%s\n"
            , self -> acc_code, VPathGetAccTypeStr ( self )));
        STSMSG(1, ("ipv4        :  %u.%u.%u.%u\n"
            , ( self -> ipv4 >> 24 ) & 0xFF, ( self -> ipv4 >> 16 ) & 0xFF,
                ( self -> ipv4 >> 8 ) & 0xFF, ( self -> ipv4 >> 0 ) & 0xFF));
        STSMSG(1, ("ipv6        :  %04X:%04X:%04X:%04X:%04X:%04X:%04X:%04X\n"
                   , self -> ipv6 [ 0 ], self -> ipv6 [ 1 ], self -> ipv6 [ 2 ], self -> ipv6 [ 3 ],
                     self -> ipv6 [ 4 ], self -> ipv6 [ 5 ], self -> ipv6 [ 6 ], self -> ipv6 [ 7 ]));
        STSMSG(1, ("portnum     :  %u\n"                   , self -> portnum));

        STSMSG(1, ("scheme_type :  %u ( %s )\n"
            , self -> scheme_type, VPathGetSchemeTypeStr ( self )));
        STSMSG(1, ("path_type   :  %u ( %s )\n"
            , self -> path_type, VPathGetPathTypeStr ( self )));
        STSMSG(1, ("host_type   :  %u ( %s )\n"
            , self -> host_type, VPathGetHostTypeStr ( self )));
        STSMSG(1, ("from_uri    :  %s\n"
            , self -> from_uri ? "true" : "false"));
        STSMSG(1, ("\n"));
        STSMSG(1, ("read back   : '%.*s' ( rc = %R, num_read = %zu )\n"
            , size, buffer, rc, num_read));
        STSMSG(1, ("\n"));
    }
}

static
rc_t ParseUrlTest ( const VFSManager * vfs )
{
    rc_t rc;
    size_t i;
    
    static const char *test_urls [] =
    {
        /* accessions */
        "SRR123456",
        "SRR1234567",
        "SRR12345678",
        "SRR123456789",
        "SRR123456.pileup",
        "SRR1234567.pileup",
        "SRR12345678.pileup",
        "SRR123456789.pileup",
        "J01415",
        "J01415.2",
        "CM000071",
        "CM000039.2",
        "NC_000012.10",
        "NW_003315935.1",
        "GPC_000000393.1",
        "AAAB01",
        "AAAA01000001",
        "NZ_AAEW01",
        "NZ_AAEW01000001",
        "NA000008777.1",

        "SRRR876",
        "SRR000123.sra",

        "ncbi-acc:SRR000123",
        "ncbi-acc:NZ_AAEW01000001",

        "ncbi-obj:12345",
        "ncbi-obj:00123",
        "123456",
        "0987655",

        "GCA_012345678.1",
        "GCA_012345678.1_S",

        /* <scheme>://<host>[:<port>]/<path>[?<query>][#<fragment>]*/
        /*0*/ "http://www.abc.com/library/index.html?x&y=123&z=test#ignore-me",
        /*1*/ "http://www.abc.com:123/library/index.html?x&y=123&z=test#ignore-me",
        /*2*/ "HTTP://www.abc.com:123/library/index.html?x&y=123&z=test#ignore-me",

        /* <scheme>://<host>[:<port>]/<path>[#<fragment>]*/
        /*3*/ "http://www.abc.com/library/index.html#ignore-me",
        /*4*/ "http://www.abc.com:123/library/index.html#ignore-me",

        /* <scheme>://<host>[:<port>]/<path>[?<query>]*/
        /*5*/ "http://www.abc.com/library/index.html?x&y=123&z=test",
        /*6*/ "http://www.abc.com:123/library/index.html?x&y=123&z=test",

        /* <scheme>://<host>[:<port>]/<path>*/
        /*7*/ "http://www.abc.com/library/index.html",
        /*8*/ "http://www.abc.com:80/library/index.html",

        /* <scheme>://<host>[:<port>]*/
        /*9*/ "http://www.abc.com",
        /*9.5*/ "https://www.abc.com",
        /*9.5*/ "Https://www.abc.com",
        /*10*/ "http://www.abc.com:80",

        /* /<path>[#fragment]*/
        /*11*/ "/library/index.html#ignore-me",

        /* /<path>[?query]*/
        /*12*/ "/library/index.html?x&y=123&z=test",
        /*13*/ "///library",

        /* having a login */
        "ftp://anonftp@ftp.ncbi.nlm.nih.gov/sra/sra-instant/reads/ByRun/sra/SRR/SRR053/SRR053325/SRR053325.sra",

        /* having the fasp weirdness */
        "fasp://anonftp@ftp.ncbi.nlm.nih.gov:/sra/sra-instant/reads/ByRun/sra/SRR/SRR053/SRR053325/SRR053325.sra",

        /* some unc paths */
        "//this/is/a/really/nice/UNC/path",
        "ncbi-file://here/is/another",
        "ncbi-file://here/is/yet/another#fraggy-the-bear",

        /* now what does this produce, and why? */
        "ncbi-file:///c/scanned-data/0001/file.sra?enc&pwd-file=/c/Users/JamesMcCoy/ncbi.pwd",

        /* modification to accept bad paths because the FS does */
        "/path/10315_3#63.bam"
    };
    const size_t num_urls = sizeof test_urls / sizeof test_urls [ 0 ];
    
    static const char *fail_url [] = 
    {
        /*<scheme>:/<path>*/
        /*0*/ "http:/library/index.html",
        /*1*/ "http//library/index.html",

        /*<scheme>:/[#fragment]*/
        /*2*/ "http:#ignore-me",

        /*<scheme>:/[#query]*/
        /*3*/ "http:?x&y=123&z=test",

        /*<scheme>*/
        /*4*/ "http",
        /*5*/ "ftp",

        /* <host>[:<port>]*/
        /*6*/ "www.abc.com",
        /*7*/ "www.abc.com:80",

        /* <scheme>://<host>[:<port>]*/
        /*8*/ "ftp://www.abc.com",

        /*9*/ "https:#bad",
        
            
    };
    const size_t num_fail_urls = sizeof fail_url / sizeof fail_url [ 0 ];
    
    static const char *sys_path [] = 
    {
#if WINDOWS
        "C:\\Program Files\\Wonderful\\Bob-o-matic",
#endif
        "//this/is/a/really/nice/UNC/path",
        "SRR000123"
    };
    const size_t num_sys_paths = sizeof sys_path / sizeof sys_path [ 0 ];
    
    for ( i = 0; i < num_urls; ++ i )
    {
        VPath *vpath;
        
        String url;
        StringInitCString ( & url, test_urls [ i ] );
        
        rc  = VFSManagerMakePath ( vfs, & vpath, "%S", & url );
        if ( rc == 0 )
        {
            STSMSG(1,  ( "%s - VFSManagerMakePath succeeded -- %zu: %S\n"
                       , __func__, i, & url ) );
            VPathDisplay ( vpath );
            VPathRelease ( vpath );
        }
        else
        {
            STSMSG(1, ( "%s: VFSManagerMakePath failed on iteration: '%zu' url: %S\n", __func__, i, & url));
            STSMSG(1, ( "with rc=%R\n", rc));
            STSMSG(1, ( "\n" ));
            STSMSG(1, ( "%s: VFSManagerMakePath failed on iteration: '%zu' url: %S\n", __func__, i, & url ));
            STSMSG(1, ( "with rc=%R\n", rc ));
            STSMSG(1, ( "\n" ));
            return rc;
        }
    }
    
    
    for ( i = 0; i < num_fail_urls; ++ i )
    {
        VPath *vpath;
        
        String url;
        StringInitCString ( & url, fail_url [ i ] );
        
        rc  = VFSManagerMakePath ( vfs, & vpath, "%S", & url );
        if ( rc != 0 )
        {
            STSMSG(1,  ( "%s - VFSManagerMakePath succeeded in catching error-- %zu: %S\n", __func__, i, & url));
            STSMSG(1,  ( "with rc=%R\n", rc ));
            STSMSG(1, ( "\n" ));
        }
        else
        {
            STSMSG(1, ( "%s: VFSManagerMakePath failed catching error on iteration: '%zu' url: %S\n"
                       , __func__, i, & url ) );
            VPathDisplay ( vpath );
            VPathRelease ( vpath );
        }
    }

    for ( i = 0; i < num_sys_paths; ++ i )
    {
        VPath *vpath;

        size_t num_read;
        char buffer [ 4096 ];
        
        String sys;
        StringInitCString ( & sys, sys_path [ i ] );
        
        rc  = VFSManagerMakeSysPath ( vfs, & vpath, sys_path [ i ] );
        if ( rc == 0 )
        {
            STSMSG(1,  ( "%s - VFSManagerMakeSysPath succeeded -- %zu: %S\n"
                       , __func__, i, & sys ) );
            VPathDisplay ( vpath );
            rc = VPathReadSysPath ( vpath, buffer, sizeof buffer, & num_read );
            STSMSG(1,  ( "read sys    : '%.*s' ( rc = %R, num_read = %zu )\n"
                       , ( uint32_t ) num_read, buffer
                       , rc
                       , num_read
                 ) );
            STSMSG(1,  ( "\n" ));

            VPathRelease ( vpath );
        }
        else
        {
            STSMSG(1, ( "%s: VFSManagerMakeSysPath failed on iteration: '%zu' path: %S\n", __func__, i, & sys ));
            STSMSG(1, ( "with rc=%R\n", rc ));
            STSMSG(1,  ( "\n" ));
            return rc;
        }
    }
    
    return 0;
}

static
rc_t ModifyPathTest ( const VFSManager * vfs )
{
    rc_t rc;
    size_t i;
    static const char *test_urls [] =
    {
        /* simple accessions */
        "SRR123456",
        "SRR123456.pileup",
        "NZ_AAEW01000001",
        "NA000008777.1",
        "GCA_012345678.1_S",

        /* oid */
        "ncbi-obj:123456",

        /* names */
        "SRR000123.sra",
        "123456",

        /* urls */
        "http://www.abc.com/library/index.html",
        "https://www.abc.com/library/index.html",
        "HTTPS://www.abc.com/library/index.html",
        "http://www.abc.com/library/index.html?x&y=123&z=test#ignore-me",

        /* having auth portion */
        "ftp://anonftp@ftp.ncbi.nlm.nih.gov/sra/sra-instant/reads/ByRun/sra/SRR/SRR053/SRR053325/SRR053325.sra",

        /* unc */
        "//this/is/a/really/nice/UNC/path",
        "ncbi-file://here/is/another",
        "ncbi-file://here/is/yet/another#fraggy-the-bear",

        /* degenerate fs paths */
        "/path/10315_3#63.bam"
    };
    const size_t num_urls = sizeof test_urls / sizeof test_urls [ 0 ];

    static const char *fail_url [] = 
    {
        /* host */
        "http://www.abc.com",
        "http://www.abc.com:80",
        "https://www.abc.com"
    };
    const size_t num_fail_urls = sizeof fail_url / sizeof fail_url [ 0 ];

    static const char *test_ext [] =
    {
        ".vdbcache",
        "_underscore_ext",
        "",
        NULL
    };
    const size_t num_ext = sizeof test_ext / sizeof test_ext [ 0 ];
    
    for ( i = 0; i < num_urls; ++ i )
    {
        VPath * orig;
        
        String url;
        StringInitCString ( & url, test_urls [ i ] );
        
        rc  = VFSManagerMakePath ( vfs, & orig, "%S", & url );
        if ( rc == 0 )
        {
            size_t j;
            for ( j = 0; j < num_ext; ++ j )
            {
                VPath * vpath;
                rc = VFSManagerMakePathWithExtension ( vfs, & vpath, orig, test_ext [ j ] );
                if ( rc == 0 )
                {
                    STSMSG(1,  ( "%s - VFSManagerMakePathWithExtension succeeded -- %zu:%zu: '%S' + '%s'\n"
                               , __func__, i, j, & url, test_ext [ j ] ) );
                    VPathDisplay ( vpath );
                    VPathRelease ( vpath );
                }
                else
                {
                    STSMSG(1, ( "%s: VFSManagerMakePathWithExtension failed on iteration: '%zu:%zu' url: '%S' + '%s'\n", __func__, i, j, & url, test_ext [ j ]));
                    STSMSG(1, ( "with rc=%R\n", rc ));
                    STSMSG(1, ( "\n" ));
                    return rc;
                }
            }

            VPathRelease ( orig );
        }
    }
    
    for ( i = 0; i < num_fail_urls; ++ i )
    {
        VPath * orig;
        
        String url;
        StringInitCString ( & url, fail_url [ i ] );
        
        rc  = VFSManagerMakePath ( vfs, & orig, "%S", & url );
        if ( rc == 0 )
        {
            size_t j;
            for ( j = 0; j < num_ext; ++ j )
            {
                VPath * vpath;
                rc = VFSManagerMakePathWithExtension ( vfs, & vpath, orig, test_ext [ j ] );
                if ( rc != 0 )
                {
                    STSMSG(1,  ( "%s - VFSManagerMakePathWithExtension properly caught error -- %zu:%zu: '%S' + '%s'\n", __func__, i, j, & url, test_ext [ j ]));
                    STSMSG(1,  ( "with rc=%R\n", rc ) );
                    STSMSG(1,  ( "\n" ) );
                    rc = 0;
                }
                else
                {
                    STSMSG(1, ( "%s: VFSManagerMakePathWithExtension failed to catch error on iteration: '%zu:%zu' url: '%S' + '%s'\n"
                              , __func__, i, j, & url, test_ext [ j ] ));
                    VPathDisplay ( vpath );
                    VPathRelease ( vpath );
                }
            }

            VPathRelease ( orig );
        }
    }

    return 0;
}

static
rc_t ExtractAccessionTest ( const VFSManager * vfs )
{
    rc_t rc;
    size_t i;
    static const char *test_urls [] =
    {
        /* simple accessions */
        "SRR123456",
        "SRR123456.pileup",
        "NZ_AAEW01000001",
        "NA000008777.1",
        "GCA_012345678.1_S",

        /* oid */
        "ncbi-obj:123456",

        /* names */
        "SRR000123.sra",
        "SRR000123.sra.ncbi_enc",
        "SRR000123.vdbcache",
        "SRR000123.pileup.sra",
        "SRR000123.pileup.sra.ncbi_enc",
        "SRR000123.pileup.vdbcache",

        /* unc */
        "//this/is/a/really/nice/UNC/path",
        "ncbi-file://here/is/another",
        "ncbi-file://here/is/yet/another#fraggy-the-bear",

        "ftp://anonftp@ftp.ncbi.nlm.nih.gov/sra/sra-instant/reads/ByRun/sra/SRR/SRR053/SRR053325/SRR053325.sra",
        "ncbi-file://here/is/yet/another/NA000008777.1.vdbcache#fraggy-the-bear"
    };
    const size_t num_urls = sizeof test_urls / sizeof test_urls [ 0 ];

    static const char *fail_url [] = 
    {
        /* urls */
        "http://www.abc.com/library/index.html",
        "http://www.abc.com/library/index.html?x&y=123&z=test#ignore-me",

        /* host */
        "http://www.abc.com",
        "http://www.abc.com:80",
        "https://www.abc.com:80",

        /* degenerate fs paths */
        "123456",
        "/path/10315_3#63.bam"
    };
    const size_t num_fail_urls = sizeof fail_url / sizeof fail_url [ 0 ];
    
    for ( i = 0; i < num_urls; ++ i )
    {
        VPath * orig;

        String url;
        StringInitCString ( & url, test_urls [ i ] );

        rc  = VFSManagerMakePath ( vfs, & orig, "%S", & url );
        if ( rc == 0 )
        {
            VPath * vpath;
            rc = VFSManagerExtractAccessionOrOID ( vfs, & vpath, orig );
            if ( rc == 0 )
            {
                STSMSG(1,  ( "%s - VFSManagerExtractAccessionOrOID succeeded -- %zu: '%S'\n"
                           , __func__, i, & url ) );
                VPathDisplay ( vpath );
                VPathRelease ( vpath );
            }
            else
            {
                STSMSG(1, ( "%s: VFSManagerExtractAccessionOrOID failed on iteration: '%zu' url: '%S'\n", __func__, i, & url));
                STSMSG(1, ( "with rc=%R\n", rc ));
                STSMSG(1, ( "\n" ));
                return rc;
            }

            VPathRelease ( orig );
        }
    }

    for ( i = 0; i < num_fail_urls; ++ i )
    {
        VPath * orig;

        String url;
        StringInitCString ( & url, fail_url [ i ] );

        rc  = VFSManagerMakePath ( vfs, & orig, "%S", & url );
        if ( rc == 0 )
        {
            VPath * vpath;
            rc = VFSManagerExtractAccessionOrOID ( vfs, & vpath, orig );
            if ( rc != 0 )
            {
                STSMSG(1,  ( "%s - VFSManagerExtractAccessionOrOID properly caught error -- %zu: '%S'\n", __func__, i, & url));
                STSMSG(1,  ( "with rc=%R\n", rc ) );
                STSMSG(1,  ( "\n" ) );
                rc = 0;
            }
            else
            {
                STSMSG(1, ( "%s: VFSManagerExtractAccessionOrOID failed to catch error on iteration: '%zu' url: '%S'\n"
                          , __func__, i, & url ));
                VPathDisplay ( vpath );
                VPathRelease ( vpath );
            }

            VPathRelease ( orig );
        }
    }

    return 0;
}


/* Version  EXTERN
 *  return 4-part version code: 0xMMmmrrrr, where
 *      MM = major release
 *      mm = minor release 
 *    rrrr = bug-fix release
 */
ver_t CC KAppVersion ( void )
{
    return 0;
}


/* Usage
 *  This function is called when the command line argument
 *  handling sees -? -h or --help
 */
rc_t CC UsageSummary ( const char *progname )
{
    return 0;
}

const char UsageDefaultName[] = "path-test";

rc_t CC Usage ( const Args *args )
{
    return 0;
}

    
static
rc_t run ( const char *progname )
{
    VFSManager * vfs;
    rc_t rc = VFSManagerMake ( & vfs );
    if ( rc == 0 )
    {
        rc = ParseUrlTest ( vfs );
        if ( rc == 0 )
            rc = ModifyPathTest ( vfs );
        if ( rc == 0 )
            rc = ExtractAccessionTest ( vfs );
        VFSManagerRelease ( vfs );
    }
    return rc;
}

/* KMain
 */
rc_t CC KMain ( int argc, char *argv [] )
{
    Args *args;
    rc_t rc = ArgsMakeAndHandle ( & args, argc, argv, 0 );
    if ( rc == 0 )
    {
        KConfigDisableUserSettings ();
        rc = run ( argv [ 0 ] );
        ArgsWhack ( args );
    }

    return rc;
}
