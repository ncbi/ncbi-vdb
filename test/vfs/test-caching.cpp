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

#include <kfg/kfg-priv.h> /* KConfigMakeLocal  */
#include <kfs/directory.h> /* KDirectory */
#include <klib/debug.h> /* KDbgSetString */
#include <ktst/unit_test.hpp> // TEST_SUITE
#include <sra/sraschema.h> // VDBManagerMakeSRASchema
#include <vdb/cursor.h> /* VCursor */
#include <vdb/manager.h> /* VDBManager */
#include <vdb/table.h> /* VTable */
#include <vdb/vdb-priv.h> /* VDBManagerMakeReadWithVFSManager */
#include <vfs/manager.h> /* VFSManager */
#include <vfs/manager-priv.h> /* VFSManagerMakeFromKfg */
#include <vfs/resolver.h> /* VResolverCacheEnable */
#include <vdb/database.h> /* VDatabase */
#include <vdb/schema.h> /* VSchema */

#define ALL

#define RELEASE(type, obj) do { rc_t rc2 = type##Release(obj); \
    if (rc2 != 0 && rc == 0) { rc = rc2; } obj = NULL; } while (false)

rc_t StringRelease ( String * self ) { free ( self ); return 0; }

TEST_SUITE(CachingSuite);

typedef enum {
/* SETTERS */
    eNone                            =      0,
    eAppsRefseq                      =      1,
    eAppsSra                         =      2,
    eAppsWgs                         =      4,
    
    eUserCacheDisabledSetFalse       =   0x20, //                 10b
    eUserCacheDisabledSetTrue        =   0x30, //                 11b

    eUserPublicCacheEnabledSetFalse  =   0x80, //               1000b
    eUserPublicCacheEnabledSetTrue   =   0xC0, //               1100b

    eUserPublicDisabledSetFalse      =  0x200, //            10 0000b
    eUserPublicDisabledSetTrue       =  0x300, //            11 0000b

    eAppRefseqDisabledSetFalse       =  0x800, //          1000 0000b
    eAppRefseqDisabledSetTrue        =  0xC00, //          1100 0000b

    eAppSraDisabledSetFalse          = 0x2000, //       10 0000 0000b
    eAppSraDisabledSetTrue           = 0x3000, //       11 0000 0000b

    eAppRefseqCacheEnabledSetFalse   = 0x8000, //     1000 0000 0000b
    eAppRefseqCacheEnabledSetTrue    = 0xC000, //     1100 0000 0000b

    eAppSraCacheEnabledSetFalse      =0x20000, //  10 0000 0000 0000b
    eAppSraCacheEnabledSetTrue       =0x30000, //  11 0000 0000 0000b

    eResolverAlwaysEnable            =0x40000, // 100 0000 0000 0000b
    eResolverAlwaysDisable           =0x80000, //1000 0000 0000 0000b

/* GETTERS */
    eUserCacheDisabled = eUserCacheDisabledSetFalse,
    eUserCacheDisabledTrue           =   0x10,  //                 1

    eUserPublicCacheEnabled = eUserPublicCacheEnabledSetFalse,
    eUserPublicCacheEnabledTrue      =   0x40,  //               100b

    eUserPublicDisabled = eUserPublicDisabledSetFalse,
    eUserPublicDisabledTrue          =  0x100,  //             1 000b

    eAppRefseqDisabled = eAppRefseqDisabledSetFalse,
    eAppRefseqDisabledTrue           =  0x400,  //           100 000b

    eAppSraDisabled = eAppSraDisabledSetFalse,
    eAppSraDisabledTrue              = 0x1000,  //      01 0000 0000b

    eAppRefseqCacheEnabled = eAppRefseqCacheEnabledSetFalse,
    eAppRefseqCacheEnabledTrue       = 0x4000,  //     100 0000 0000b

    eAppSraCacheEnabled = eAppSraCacheEnabledSetFalse,
    eAppSraCacheEnabledTrue          =0x10000,  //  1 0000 0000 0000b
} E;

class SET {
public:
    static rc_t set ( KConfig * self, int e, bool caching ) {
        rc_t rc = KConfigWriteString(self,
            "/repository/user/ad/public/cache-enabled",
            caching ? "true" : "false");

        if ( e == 0 ){
            return rc;
        }

        if ( e & eAppsRefseq ) {
            rc = KConfigWriteString ( self,
               "/repository/user/main/public/apps/refseq/volumes/refseq",
                "refseq" );
        }
        if ( e & eAppsSra ) {
            rc = KConfigWriteString ( self,
               "/repository/user/main/public/apps/sra/volumes/sraFlat", "sra" );
        }
        if ( e & eAppsWgs ) {
            rc = KConfigWriteString ( self,
               "/repository/user/main/public/apps/wgs/volumes/wgsFlat", "wgs" );
        }
        if ( rc == 0 && e & eUserCacheDisabled ) {
            const char * v = e & eUserCacheDisabledTrue ? "true" : "false";
            rc = KConfigWriteString ( self,
                "/repository/user/cache-disabled", v );
        }
        if ( rc == 0 && e & eUserPublicCacheEnabled ) {
            const char * v = e & eUserPublicCacheEnabledTrue ? "true" : "false";
            rc = KConfigWriteString ( self,
                "/repository/user/main/public/cache-enabled", v );
        }
        if ( rc == 0 && e & eUserPublicDisabled ) {
            const char * v = e & eUserPublicDisabledTrue ? "true" : "false";
            rc = KConfigWriteString ( self,
                "/repository/user/main/public/disabled", v );
        }
        if ( rc == 0 && e & eAppRefseqDisabled ) {
            const char * v = e & eAppRefseqDisabledTrue ? "true" : "false";
            rc = KConfigWriteString ( self,
                "/repository/user/main/public/apps/refseq/disabled", v );
        }
        if ( rc == 0 && e & eAppSraDisabled ) {
            const char * v = e & eAppSraDisabledTrue ? "true" : "false";
            rc = KConfigWriteString ( self,
                "/repository/user/main/public/apps/sra/disabled", v );
            if (rc == 0)
                rc = KConfigWriteString(self,
                    "/repository/user/ad/public/apps/sra/disabled", v);
        }
        if ( rc == 0 && e & eAppRefseqCacheEnabled ) {
            const char * v = e & eAppRefseqCacheEnabledTrue ? "true" : "false";
            rc = KConfigWriteString ( self,
                "/repository/user/main/public/apps/refseq/cache-enabled", v );
        }
        if ( rc == 0 && e & eAppSraCacheEnabled ) {
            const char * v = e & eAppSraCacheEnabledTrue ? "true" : "false";
            rc = KConfigWriteString ( self,
                "/repository/user/main/public/apps/sra/cache-enabled", v );
        }
        return rc;
    }

    static rc_t set ( const VFSManager * self, int e ) {
        if ( ( e & eResolverAlwaysEnable ) && ( e & eResolverAlwaysDisable ) ) {
            assert (0);
        }
        if ( e & eResolverAlwaysEnable ) {
            return enbl ( self, vrAlwaysEnable );
        }
        else if ( e & eResolverAlwaysDisable ) {
            return enbl ( self, vrAlwaysDisable );
        }
        else {
            return 0;
        }
    }

    static rc_t resetSingleton ( const VFSManager * self ) {
        return enbl ( self, vrUseConfig ); // VResolverEnableState IS STICKY !!!
    }

private:
    static rc_t enbl ( const VFSManager * self,  VResolverEnableState enable ) {
        VResolver * resolver = NULL;
        rc_t rc = VFSManagerGetResolver ( self, & resolver );
        if ( rc == 0 ) {
            VResolverCacheEnable ( resolver, enable );
        }
        RELEASE ( VResolver, resolver );
        return rc;
    }
};

typedef enum {
    eRefseq,
    eSra,
    eWgs,
} EType;

class Test : protected ncbi::NK::TestCase {
    TestCase * _dad;

public:
    Test ( TestCase * dad, bool caching, int v, EType type )
        : TestCase ( dad -> GetName () ), _dad ( dad )
    {
#define WGS "AFVF01"
        const char * acc = type == eRefseq ? "KC702199.1" :
                           type == eSra    ? "SRR003325" : WGS ".1"; 
//                         type == eSra    ? "SRR053325" : WGS ".1"; 
        rc_t rc = 0;
        KDirectory * native  = NULL;
        REQUIRE_RC ( KDirectoryNativeDir ( & native ) );
        const KDirectory * dir = NULL;

#if WINDOWS
#define KFG_OS "win"
#else
#define KFG_OS "unix"
#endif
        REQUIRE_RC ( KDirectoryOpenDirRead ( native, &dir, false, "caching-kfg/" KFG_OS) );
#undef  OS

        KConfig * cfg = NULL;
        REQUIRE_RC ( KConfigMakeLocal ( & cfg, dir ) );
        REQUIRE_RC ( SET :: set ( cfg, v, caching) );

        String * test_root = NULL;
        REQUIRE_RC ( KConfigReadString ( cfg, "/TEST_ROOT", & test_root ) );
        String * user_root = NULL;
        REQUIRE_RC ( KConfigReadString
            ( cfg, "/repository/user/main/public/root", & user_root ) );

        // KConfigReadString creates NULL-terminated strings
        REQUIRE ( user_root );
		std::cout << std::string(user_root->addr) << std::endl;
        REQUIRE_RC ( KDirectoryRemove ( native, true, "%s", user_root -> addr ) );

        VFSManager * vfs = NULL;

//KConfigPrint(cfg, 0);

        REQUIRE_RC ( VFSManagerMakeFromKfg ( & vfs, cfg ) );
        REQUIRE_RC ( SET :: resetSingleton ( vfs ) );

        REQUIRE_RC ( SET :: set ( vfs, v ) );
        const VDBManager * mgr = NULL;
        REQUIRE_RC ( VDBManagerMakeReadWithVFSManager ( & mgr, NULL, vfs ) );
        VSchema * schema = NULL;
        REQUIRE_RC ( VDBManagerMakeSRASchema ( mgr, & schema ) );

        // KConfigReadString creates NULL-terminated strings
        REQUIRE ( user_root ); 
        REQUIRE_EQ ( KDirectoryPathType ( native, user_root -> addr ),
                     ( KPathType ) kptNotFound );
        const VDatabase * db = NULL;
        const VTable * tbl = NULL;
        if ( type == eWgs ) {
            REQUIRE_RC ( VDBManagerOpenDBRead ( mgr, & db, NULL, acc ) );
        }
        else {
            REQUIRE_RC ( VDBManagerOpenTableRead ( mgr, & tbl, schema, acc ) );
        }

        // KConfigReadString creates NULL-terminated strings
        REQUIRE ( user_root ); 
        if ( caching ) {
            REQUIRE_EQ ( KDirectoryPathType ( native, user_root -> addr ),
                         ( KPathType ) kptDir );
            switch ( type ) {
            case eRefseq:
                REQUIRE_EQ ( KDirectoryPathType ( native, "%s/refseq/%s.cache",
                             user_root -> addr, acc ), ( KPathType ) kptFile );
                break;
            case eSra:
                REQUIRE_EQ ( KDirectoryPathType ( native, "%s/sra/%s.sra.cache",
                             user_root -> addr, acc ), ( KPathType ) kptFile );
                break;
            case eWgs:
                REQUIRE_EQ ( KDirectoryPathType ( native, "%s/wgs/%s.cache",
                             user_root -> addr, WGS ), ( KPathType ) kptFile );
                break;
            default:
                assert ( 0 ) ;
                break;
            }
        }
        else {
            REQUIRE_EQ ( KDirectoryPathType ( native, user_root -> addr ),
                         ( KPathType ) kptNotFound );
        }
        RELEASE ( VDatabase, db );
        RELEASE ( VSchema, schema );
        RELEASE ( VTable, tbl );

        REQUIRE_RC ( SET :: resetSingleton ( vfs ) );
        RELEASE ( VFSManager, vfs );

        RELEASE ( VDBManager, mgr );
        RELEASE ( KDirectory, dir );

        // KConfigReadString creates NULL-terminated strings
        REQUIRE ( user_root ); 
        REQUIRE_RC ( KDirectoryRemove ( native, true, user_root -> addr ) );
        REQUIRE_EQ ( KDirectoryPathType ( native, user_root -> addr ),
                     ( KPathType ) kptNotFound );

        RELEASE ( String, user_root );
        REQUIRE_RC ( KConfigReadString ( cfg, "/TEST_SUBROOT", & user_root ) );

        // KConfigReadString creates NULL-terminated strings
        REQUIRE ( user_root );
        REQUIRE_RC ( KDirectoryRemove ( native, false, user_root -> addr ) );
        RELEASE ( String, user_root );

        // KConfigReadString creates NULL-terminated strings
        REQUIRE ( test_root ); 
        KDirectoryRemove ( native, false, test_root -> addr );
        // might fail when there is another test build running parallelly

        RELEASE ( String, test_root );
        RELEASE ( KConfig, cfg );
        RELEASE ( KDirectory, native );
        REQUIRE_RC ( rc );
}

  ~Test ( void ) {
      assert( _dad );
      _dad->ErrorCounterAdd(GetErrorCounter());
    }
};

class NonCaching : private Test {
public:
    NonCaching ( TestCase * dad, int v = eNone, EType type = eSra )
        : Test ( dad, false, v, type )
    {}
};

class Caching : private Test {
public:
    Caching (TestCase * dad, int v, EType type = eSra )
        : Test ( dad, true, v, type )
    {}
};

#ifdef ALL
TEST_CASE ( SRR_INCOMPLETE_USER_REPO ) {
    NonCaching ( this );
}
TEST_CASE ( REFSEQ_INCOMPLETE_USER_REPO ) {
    NonCaching ( this, eNone, eRefseq );
}
TEST_CASE ( WGS_INCOMPLETE_USER_REPO ) {
    NonCaching ( this, eNone, eWgs );
}

TEST_CASE ( REFSEQ_BAD_APP_IN_USER_REPO ) {
    NonCaching ( this, eAppsSra , eRefseq );
}
TEST_CASE ( WGS_BAD_APP_IN_USER_REPO ) {
    NonCaching ( this, eAppsSra , eWgs );
}

TEST_CASE ( SRR_COMPLETE_USER_REPO ) {
    Caching ( this, eAppsSra );
}
TEST_CASE ( REFSEQ_COMPLETE_USER_REPO ) {
    Caching ( this, eAppsRefseq, eRefseq );
}
TEST_CASE ( WGS_COMPLETE_USER_REPO ) {
    Caching ( this, eAppsWgs, eWgs );
}

TEST_CASE ( SRR_CASHING_DISABLED_BY_VDB_CONFIG_GRF ) {
    NonCaching ( this, eAppsSra | eUserCacheDisabledSetTrue );
}
TEST_CASE ( REFSEQ_CASHING_DISABLED_BY_VDB_CONFIG_GRF ) {
    NonCaching ( this, eAppsRefseq | eUserCacheDisabledSetTrue, eRefseq );
}
TEST_CASE ( WGS_DISABLED_BY_VDB_CONFIG_GRF ) {
    NonCaching ( this, eAppsWgs | eUserCacheDisabledSetTrue, eWgs );
}

TEST_CASE ( SRR_CASHING_ENABLED_BY_VDB_CONFIG_GRF ) {
    Caching ( this, eAppsSra | eUserCacheDisabledSetFalse );
}
TEST_CASE ( REFSEQ_CASHING_ENABLED_BY_VDB_CONFIG_GRF ) {
    Caching ( this, eAppsRefseq | eUserCacheDisabledSetFalse, eRefseq );
}
TEST_CASE ( WGS_CASHING_ENABLED_BY_VDB_CONFIG_GRF ) {
    Caching ( this, eAppsWgs | eUserCacheDisabledSetFalse, eWgs );
}

TEST_CASE ( SRR_CASHING_ENABLED_BY_VDB_CONFIG_TXT ) {
    Caching ( this, eAppsSra | eUserPublicCacheEnabledSetTrue );
}
TEST_CASE ( REFSEQ_CASHING_ENABLED_BY_VDB_CONFIG_TXT ) {
    Caching ( this, eAppsRefseq | eUserPublicCacheEnabledSetTrue, eRefseq );
}
TEST_CASE ( WGS_CASHING_ENABLED_BY_VDB_CONFIG_TXT ) {
    Caching ( this, eAppsWgs | eUserPublicCacheEnabledSetTrue, eWgs );
}

TEST_CASE ( SRR_CASHING_DISABLED_BY_VDB_CONFIG_TXT ) {
    NonCaching ( this, eAppsSra | eUserPublicCacheEnabledSetFalse );
}
TEST_CASE ( REFSEQ_CASHING_DISABLED_BY_VDB_CONFIG_TXT ) {
    NonCaching ( this, eAppsRefseq | eUserPublicCacheEnabledSetFalse, eRefseq );
}
TEST_CASE ( WGS_CASHING_DISABLED_BY_VDB_CONFIG_TXT ) {
    NonCaching ( this, eAppsWgs | eUserPublicCacheEnabledSetFalse, eWgs );
}

TEST_CASE ( SRR_CASHING_ENABLED_BY_VDB_CONFIG_TXT_AND_GRF ) {
    Caching ( this, eAppsSra | eUserCacheDisabledSetFalse |
                               eUserPublicCacheEnabledSetTrue );
}
TEST_CASE ( REFSEQ_CASHING_ENABLED_BY_VDB_CONFIG_TXT_AND_GRF ) {
    Caching ( this, eAppsRefseq | eUserCacheDisabledSetFalse |
                                  eUserPublicCacheEnabledSetTrue, eRefseq );
}
TEST_CASE ( WGS_CASHING_ENABLED_BY_VDB_CONFIG_TXT_AND_GRF ) {
    Caching ( this, eAppsWgs | eUserCacheDisabledSetFalse |
                               eUserPublicCacheEnabledSetTrue, eWgs );
}

TEST_CASE ( SRR_USER_REPO_DISABLED ) {
    NonCaching ( this, eAppsSra | eUserCacheDisabledSetFalse |
        eUserPublicCacheEnabledSetTrue | eUserPublicDisabledSetTrue );
}
TEST_CASE ( REFSEQ_USER_REPO_DISABLED ) {
    NonCaching ( this, eAppsRefseq | eUserCacheDisabledSetFalse |
        eUserPublicCacheEnabledSetTrue | eUserPublicDisabledSetTrue, eRefseq );
}
TEST_CASE ( WGS_USER_REPO_DISABLED ) {
    NonCaching ( this, eAppsWgs | eUserCacheDisabledSetFalse |
        eUserPublicCacheEnabledSetTrue | eUserPublicDisabledSetTrue, eWgs );
}

TEST_CASE ( SRR_CASHING_AND_REPO_ENABLED ) {
    Caching ( this, eAppsSra | eUserCacheDisabledSetFalse |
        eUserPublicCacheEnabledSetTrue | eUserPublicDisabledSetFalse );
}
TEST_CASE ( REFSEQ_CASHING_AND_REPO_ENABLED ) {
    Caching ( this, eAppsRefseq | eUserCacheDisabledSetFalse |
        eUserPublicCacheEnabledSetTrue | eUserPublicDisabledSetFalse, eRefseq );
}
TEST_CASE ( WGS_CASHING_AND_REPO_ENABLED ) {
    Caching ( this, eAppsWgs | eUserCacheDisabledSetFalse |
        eUserPublicCacheEnabledSetTrue | eUserPublicDisabledSetFalse, eWgs );
}

TEST_CASE ( SRR_CASHING_AND_REPO_ENABLED_DISABLE_REFSEQ_APP ) {
    Caching ( this, eAppsSra | eUserCacheDisabledSetFalse |
        eUserPublicCacheEnabledSetTrue | eUserPublicDisabledSetFalse |
        eAppRefseqDisabledSetTrue );
}
TEST_CASE ( REFSEQ_CASHING_AND_REPO_ENABLED_DISABLE_REFSEQ_APP ) {
    NonCaching ( this, eAppsRefseq | eUserCacheDisabledSetFalse |
        eUserPublicCacheEnabledSetTrue | eUserPublicDisabledSetFalse |
        eAppRefseqDisabledSetTrue, eRefseq );
}
TEST_CASE ( WGS_CASHING_AND_REPO_ENABLED_DISABLE_REFSEQ_APP ) {
    Caching ( this, eAppsWgs | eUserCacheDisabledSetFalse |
        eUserPublicCacheEnabledSetTrue | eUserPublicDisabledSetFalse |
        eAppRefseqDisabledSetTrue, eWgs );
}

TEST_CASE ( SRR_CASHING_AND_REPO_ENABLED_DISABLE_SRA_APP ) {
    NonCaching ( this, eAppsSra | eUserCacheDisabledSetFalse |
        eUserPublicCacheEnabledSetTrue | eUserPublicDisabledSetFalse |
        eAppSraDisabledSetTrue );
}
TEST_CASE ( REFSEQ_CASHING_AND_REPO_ENABLED_DISABLE_SRA_APP ) {
    Caching ( this, eAppsRefseq | eUserCacheDisabledSetFalse |
        eUserPublicCacheEnabledSetTrue | eUserPublicDisabledSetFalse |
        eAppSraDisabledSetTrue, eRefseq );
}
TEST_CASE ( WGS_CASHING_AND_REPO_ENABLED_DISABLE_SRA_APP ) {
    Caching ( this, eAppsWgs | eUserCacheDisabledSetFalse |
        eUserPublicCacheEnabledSetTrue | eUserPublicDisabledSetFalse |
        eAppSraDisabledSetTrue, eWgs );
}

TEST_CASE ( SRR_CASHING_AND_REPO_ENABLED_DISABLE_SRA_APP_CACHE ) {
    NonCaching ( this, eAppsSra | eUserCacheDisabledSetFalse |
        eUserPublicCacheEnabledSetTrue | eUserPublicDisabledSetFalse |
        eAppSraCacheEnabledSetFalse );
}
TEST_CASE ( REFSEQ_CASHING_AND_REPO_ENABLED_DISABLE_SRA_APP_CACHE ) {
    Caching ( this, eAppsRefseq | eUserCacheDisabledSetFalse |
        eUserPublicCacheEnabledSetTrue | eUserPublicDisabledSetFalse |
        eAppSraCacheEnabledSetTrue | eAppSraCacheEnabledSetFalse, eRefseq );
}
TEST_CASE ( WGS_CASHING_AND_REPO_ENABLED_DISABLE_SRA_APP_CACHE ) {
    Caching ( this, eAppsWgs | eUserCacheDisabledSetFalse |
        eUserPublicCacheEnabledSetTrue | eUserPublicDisabledSetFalse |
        eAppSraCacheEnabledSetTrue | eAppSraCacheEnabledSetFalse, eWgs );
}

TEST_CASE ( SRR_CASHING_ENABLED_BY_VDB_CONFIG_GRF_DISABLED_BY_TXT ) {
    NonCaching ( this, eAppsSra | eUserCacheDisabledSetFalse |
                                  eUserPublicCacheEnabledSetFalse );
}
TEST_CASE ( REFSEQ_CASHING_ENABLED_BY_VDB_CONFIG_GRF_DISABLED_BY_TXT ) {
    NonCaching ( this, eAppsRefseq | eUserCacheDisabledSetFalse |
                                     eUserPublicCacheEnabledSetFalse, eRefseq );
}
TEST_CASE ( WGS_CASHING_ENABLED_BY_VDB_CONFIG_GRF_DISABLED_BY_TXT ) {
    NonCaching ( this, eAppsWgs | eUserCacheDisabledSetFalse |
                                  eUserPublicCacheEnabledSetFalse, eWgs );
}

TEST_CASE ( SRR_CASHING_DISABLED_BY_VDB_CONFIG_TXT_AND_GRF ) {
     NonCaching ( this, eAppsSra | eUserCacheDisabledSetTrue |
                                   eUserPublicCacheEnabledSetFalse );
}
TEST_CASE ( REFSEQ_CASHING_DISABLED_BY_VDB_CONFIG_TXT_AND_GRF ) {
    NonCaching ( this, eAppsRefseq | eUserCacheDisabledSetTrue |
                                     eUserPublicCacheEnabledSetFalse, eRefseq );
}
TEST_CASE ( WGS_CASHING_DISABLED_BY_VDB_CONFIG_TXT_AND_GRF ) {
    NonCaching ( this, eAppsWgs | eUserCacheDisabledSetTrue |
                                  eUserPublicCacheEnabledSetFalse, eWgs );
}

TEST_CASE ( SRR_CASHING_DISABLED_BY_VDB_CONFIG_GRF_ENABLED_BY_TXT ) {
     NonCaching ( this, eAppsSra | eUserCacheDisabledSetTrue |
                                   eUserPublicCacheEnabledSetTrue );
}
TEST_CASE ( REFSEQ_CASHING_DISABLED_BY_VDB_CONFIG_GRF_ENABLED_BY_TXT ) {
    NonCaching ( this, eAppsRefseq | eUserCacheDisabledSetTrue |
                                     eUserPublicCacheEnabledSetTrue, eRefseq );
}
TEST_CASE ( WGS_CASHING_DISABLED_BY_VDB_CONFIG_GRF_ENABLED_BY_TXT ) {
    NonCaching ( this, eAppsWgs | eUserCacheDisabledSetTrue |
                                  eUserPublicCacheEnabledSetTrue, eWgs );
}

TEST_CASE ( SRR_APP_DISABLED ) {
    NonCaching ( this, eAppsSra | eUserCacheDisabledSetFalse |
        eUserPublicCacheEnabledSetTrue | eUserPublicDisabledSetFalse |
        eAppRefseqDisabledSetFalse | eAppSraDisabledSetTrue |
        eAppRefseqCacheEnabledSetTrue | eAppSraCacheEnabledSetTrue );
}
TEST_CASE ( REFSEQ_SRR_APP_DISABLED ) {
    Caching ( this, eAppsRefseq | eUserCacheDisabledSetFalse |
        eUserPublicCacheEnabledSetTrue | eUserPublicDisabledSetFalse |
        eAppRefseqDisabledSetFalse | eAppSraDisabledSetTrue |
        eAppRefseqCacheEnabledSetTrue | eAppSraCacheEnabledSetTrue, eRefseq );
}
#endif

TEST_CASE ( SRR_APP_DISABLED_ALWAYS_ENABLE ) {
    NonCaching ( this, eAppsSra | eUserCacheDisabledSetFalse |
        eUserPublicCacheEnabledSetTrue | eUserPublicDisabledSetFalse |
        eAppRefseqDisabledSetFalse | eAppSraDisabledSetTrue |
        eAppRefseqCacheEnabledSetTrue | eAppSraCacheEnabledSetTrue |
        eResolverAlwaysEnable );
}

TEST_CASE ( SRR_CASHING_AND_REPO_AND_APP_ENABLED ) {
    Caching ( this, eAppsSra | eUserCacheDisabledSetFalse |
        eUserPublicCacheEnabledSetTrue | eUserPublicDisabledSetFalse |
        eAppRefseqDisabledSetFalse | eAppSraDisabledSetFalse |
        eAppRefseqCacheEnabledSetTrue | eAppSraCacheEnabledSetTrue );
}
TEST_CASE ( REFSEQ_CASHING_AND_REPO_AND_APP_ENABLED ) {
    Caching ( this, eAppsRefseq | eUserCacheDisabledSetFalse |
        eUserPublicCacheEnabledSetTrue | eUserPublicDisabledSetFalse |
        eAppRefseqDisabledSetFalse | eAppSraDisabledSetFalse |
        eAppRefseqCacheEnabledSetTrue | eAppSraCacheEnabledSetTrue, eRefseq );
}
TEST_CASE ( WGS_CASHING_AND_REPO_AND_APP_ENABLED ) {
    Caching ( this, eAppsWgs | eUserCacheDisabledSetFalse |
        eUserPublicCacheEnabledSetTrue | eUserPublicDisabledSetFalse |
        eAppRefseqDisabledSetFalse | eAppSraDisabledSetFalse |
        eAppRefseqCacheEnabledSetTrue | eAppSraCacheEnabledSetTrue, eWgs );
}

TEST_CASE ( SRR_CASHING_ALWAYS_DISABLE ) {
    NonCaching ( this, eAppsSra | eUserCacheDisabledSetFalse |
        eUserPublicCacheEnabledSetTrue | eUserPublicDisabledSetFalse |
        eAppRefseqDisabledSetFalse | eAppSraDisabledSetFalse |
        eAppRefseqCacheEnabledSetTrue | eAppSraCacheEnabledSetTrue |
        eResolverAlwaysDisable );
}
TEST_CASE ( REFSEQ_CASHING_ALWAYS_DISABLE ) {
    NonCaching ( this, eAppsRefseq | eUserCacheDisabledSetFalse |
        eUserPublicCacheEnabledSetTrue | eUserPublicDisabledSetFalse |
        eAppRefseqDisabledSetFalse | eAppSraDisabledSetFalse |
        eAppRefseqCacheEnabledSetTrue | eAppSraCacheEnabledSetTrue |
        eResolverAlwaysDisable, eRefseq );
}
TEST_CASE ( WGS_CASHING_ALWAYS_DISABLE ) {
    NonCaching ( this, eAppsWgs | eUserCacheDisabledSetFalse |
        eUserPublicCacheEnabledSetTrue | eUserPublicDisabledSetFalse |
        eAppRefseqDisabledSetFalse | eAppSraDisabledSetFalse |
        eAppRefseqCacheEnabledSetTrue | eAppSraCacheEnabledSetTrue |
        eResolverAlwaysDisable, eWgs );
}

TEST_CASE ( SRR_NON_CACHING ) {
    NonCaching ( this, eAppsSra | eUserCacheDisabledSetTrue |
        eUserPublicCacheEnabledSetFalse | eUserPublicDisabledSetFalse |
        eAppRefseqDisabledSetTrue | eAppSraDisabledSetFalse |
        eAppRefseqCacheEnabledSetFalse | eAppSraCacheEnabledSetFalse );
}
TEST_CASE ( SRR_ALWAYS_ENABLE ) {
    Caching ( this, eAppsSra | eUserCacheDisabledSetTrue |
        eUserPublicCacheEnabledSetFalse | eUserPublicDisabledSetFalse |
        eAppRefseqDisabledSetTrue | eAppSraDisabledSetFalse |
        eAppRefseqCacheEnabledSetFalse | eAppSraCacheEnabledSetFalse |
        eResolverAlwaysEnable );
}

TEST_CASE ( REFSEQ_APP_CACHE_DISABLE ) {
    NonCaching ( this, eAppsRefseq | eAppRefseqCacheEnabledSetFalse, eRefseq );
}

////////////////////////////////////////////////////////////////////////////////

extern "C" {
    ver_t CC KAppVersion ( void ) {
        return 0;
    }
    rc_t CC KMain ( int argc, char * argv [] ) {
        KConfigDisableUserSettings ();
if (
0 ) assert ( ! KDbgSetString ( "VFS" ) );
        return CachingSuite ( argc, argv );
    }
}
