/*==============================================================================
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
* ==============================================================================
* Tests of cache of names resolve results / SDL responses.
*/

#include <kfg/kfg-priv.h> /* KConfigMakeLocal */
#include <kfg/properties.h> /* KConfig_Set_Sdl_Cache_Limit */
#include <klib/printf.h> /* string_printf */
#include <klib/strings.h> /* ENV_VAR_NO_CACHE_SDL_RESPONSE */
#include <klib/time.h> /* KTimeGlobal */
#include <vfs/manager.h> /* VFSManagerRelease */
#include <vfs/manager-priv.h> /* VFSManagerMakeFromKfg */
#include <vfs/path-priv.h> /* VPathGetDirectory */
#include "../../libs/vfs/manager-priv.h" // VFSManagerSdlCacheEmpty

#include <string>

#include <limits.h> /* PATH_MAX */
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#include <os-native.h> // setenv

enum {
    eCaching = 1,
    eEnv = 2,
    eKfg = 4,
};

class CachingFixture {
    char path[PATH_MAX];
protected:
    const VPath * remote;

    CachingFixture
    (int caching = eCaching | eEnv, int limiting = 0)
        : remote(nullptr)
        , cfg(nullptr)
        , mgr(nullptr)
        , query(nullptr)
        , resolver(nullptr)
    {
        path[0] = '\0';

        if (caching & eCaching)
            unsetenv(ENV_VAR_NO_CACHE_SDL_RESPONSE);
        else {
            if (caching & eEnv)
                setenv(ENV_VAR_NO_CACHE_SDL_RESPONSE, "1", 1);
            else if (caching & eKfg) {
                if (KConfigMakeLocal(&cfg, nullptr))
                    throw "cannot KConfigMakeLocal";
                if (KConfig_Set_Sdl_Caching_Disabled(cfg, true))
                    throw "cannot KConfigWriteBool";
            }
        }

        if (limiting) {
            if (limiting & eEnv)
                setenv(ENV_VAR_SDL_CACHE_LIMIT, "1", 1);
            else if (limiting & eKfg) {
                if (cfg == nullptr && KConfigMakeLocal(&cfg, nullptr))
                    throw "cannot KConfigMakeLocal";
                if (KConfig_Set_Sdl_Cache_Limit(cfg, 1))
                    throw "cannot KConfig_Set_Sdl_Cache_Limit";
            }
        }

        rc_t rc(VFSManagerMakeFromKfg(&mgr, cfg));
        if (rc != 0)
            throw rc;

        rc = VFSManagerGetResolver(mgr, &resolver);
        if (rc != 0)
            throw rc;
    }

    ~CachingFixture() {
        KConfigRelease(cfg);
        VFSManagerRelease(mgr);
        VPathRelease(remote);
        VPathRelease(query);
        VResolverRelease(resolver);
        unsetenv(ENV_VAR_NO_CACHE_SDL_RESPONSE);
        unsetenv("NCBI_VDB_NO_SDL_CACHE_LIMIT");
    }

    rc_t ResetQuery(const char * path_str) {
        if (strcmp(path, path_str) == 0)
            return 0;

        strcpy(path, path_str);
        rc_t rc(VPathRelease(query));
        query = NULL;

        if (rc == 0)
            rc = VFSManagerMakePath(mgr, &query, path_str);

        return rc;
    }

    rc_t QueryRemote(const char * path_str) {
        rc_t rc(ResetQuery(path_str));

        if (rc == 0)
            rc = VPathRelease(remote);
        remote = NULL;

        if (rc == 0)
            rc = VResolverRemote(resolver, 0, query, &remote);

        return rc;
    }

    rc_t RemoteEquals(const char * path_str) const {
        char p[4096]("");
        rc_t rc(VPathReadUri(remote, p, sizeof p, NULL));
        if (rc == 0)
            if (strcmp(p, path_str) != 0)
                rc = 115;

        return rc;
    }

    static std::string MkSdlJson
    (const std::string & acc, const std::string & url, int sec = -1)
    {
        time_t now(KTimeStamp()); // time(&now);
        now += sec;
        KTime kt;
        KTimeGlobal(&kt, now);
        //struct tm * ptr(gmtime(&now));
        char timeString[99]("");
        //strftime(timeString, sizeof timeString, "%Y-%m-%dT%H:%M:%SZ", ptr);
        string_printf(timeString, sizeof timeString, nullptr,
            "%04d-%02d-%02dT%02d:%02d:%02dZ",
            kt.year, kt.month + 1, kt.day + 1,
            kt.hour, kt.minute, kt.second);

        std::string json(acc + "="
            "{ \"result\": [ { \"files\": [ { \"locations\": [ {\n");
        if (sec >= 0) {
            json += "              \"expirationDate\": \"";
            json += timeString;
            json += "\",\n";
        }
        json += "              \"link\": \"" + url + "\" } ] } ] } ] }\n";

        return json;
    }

public:
    KConfig * cfg;
    VFSManager * mgr;
    VPath * query;
    VResolver * resolver;
};

class NotCachingFixture : protected CachingFixture {
protected:
    NotCachingFixture() : CachingFixture(eEnv) {}
};

class NotCachingByKfgFixture : protected CachingFixture {
protected:
    NotCachingByKfgFixture() : CachingFixture(eKfg) {}
};

class LimitCachingByEnvFixture : protected CachingFixture {
protected:
    LimitCachingByEnvFixture() : CachingFixture(eCaching | eEnv, eEnv) {}
};

class LimitCachingByKfgFixture : protected CachingFixture {
protected:
    LimitCachingByKfgFixture() : CachingFixture(eCaching | eEnv, eKfg) {}
};
