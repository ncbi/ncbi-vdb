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

#include <klib/namelist.h>
#include <klib/printf.h>
#include <klib/strings.h> /* KFG_USER_ACCEPT_GCP_CHARGES etc */
#include <klib/rc.h>
#include <klib/text.h>
#include <klib/vector.h>

#include <kfs/directory.h>
#include <kfs/file.h>

#include <kfg/config.h>
#include <kfg/repository.h>
#include <kfg/ngc.h>
#include "ngc-priv.h"

#include <va_copy.h>

/* ---------------------------------------------------------------------------------------------------- */

static rc_t KConfig_Get_Repository_State( const KConfig *self,
    bool * state, bool negate, bool dflt, const char * path, ... )
{
    rc_t rc = 0;
    if ( self == NULL )
        rc = RC ( rcKFG, rcNode, rcReading, rcSelf, rcNull );
    else if ( state == NULL || path == NULL )
        rc = RC ( rcKFG, rcNode, rcReading, rcParam, rcNull );
    else {
        va_list args;
        char tmp[ 4096 ];
        size_t num_writ;

        *state = dflt;
        va_start ( args, path );
        rc = string_vprintf ( tmp, sizeof tmp, & num_writ, path, args );
        va_end ( args );

        if ( rc == 0 )
        {
            bool rd_state;
            rc = KConfigReadBool ( self, tmp, &rd_state );
            if ( rc == 0 )
            {
                if ( negate )
                    *state = !rd_state;
                else
                    *state = rd_state;
            }
            else
                rc = 0;     /* it is OK to not find the node, return the default then... */
        }
    }
    return rc;
}


static rc_t KConfig_Set_Repository_State( KConfig *self,
    bool state, bool negate, const char * path, ... )
{
    rc_t rc = 0;
    if ( self == NULL )
        rc = RC ( rcKFG, rcNode, rcWriting, rcSelf, rcNull );
    else if ( path == NULL )
        rc = RC ( rcKFG, rcNode, rcWriting, rcParam, rcNull );
    else
    {
        va_list args;
        char tmp[ 4096 ];
        size_t num_writ;

        va_start ( args, path );
        rc = string_vprintf ( tmp, sizeof tmp, & num_writ, path, args );
        va_end ( args );

        if ( rc == 0 )
        {
            if ( negate )
                rc = KConfigWriteBool( self, tmp, !state );
            else
                rc = KConfigWriteBool( self, tmp, state );
        }
    }
    return rc;
}


/* ---------------------------------------------------------------------------------------------------- */

static rc_t KConfig_Get_Repository_String( const KConfig *self,
    char * buffer, size_t buffer_size, size_t * written, const char * path, ... )
{
    rc_t rc = 0;
    if ( self == NULL )
        rc = RC ( rcKFG, rcNode, rcReading, rcSelf, rcNull );
    else if ( buffer == NULL || path == NULL )
        rc = RC ( rcKFG, rcNode, rcReading, rcParam, rcNull );
    {
        va_list args;
        char tmp[ 4096 ];
        size_t num_writ;

        va_start ( args, path );
        rc = string_vprintf ( tmp, sizeof tmp, & num_writ, path, args );
        va_end ( args );

        if ( rc == 0 )
        {
            struct String * res;
            rc = KConfigReadString ( self, tmp, &res );
            if ( rc == 0 )
            {
                rc = string_printf( buffer, buffer_size, written, "%S", res );
                StringWhack ( res );
            }
            else
            {
                * buffer = 0;
                if ( written != NULL )
                {
                    * written = 0;
                }
            }
        }
    }
    return rc;
}


static rc_t KConfig_Set_Repository_String( KConfig *self,
    const char * value, const char * path, ... )
{
    rc_t rc = 0;
    if ( self == NULL )
        rc = RC ( rcKFG, rcNode, rcWriting, rcSelf, rcNull );
    else if ( path == NULL )
        rc = RC ( rcKFG, rcNode, rcWriting, rcParam, rcNull );
    else
    {
        va_list args;
        char tmp[ 4096 ];
        size_t num_writ;

        va_start ( args, path );
        rc = string_vprintf ( tmp, sizeof tmp, & num_writ, path, args );
        va_end ( args );

        if ( rc == 0 )
            rc = KConfigWriteString( self, tmp, value );
    }
    return rc;
}

/* -------------------------------------------------------------------------- */

/* get/set HTTP proxy path */
LIB_EXPORT rc_t CC KConfig_Get_Http_Proxy_Path( const KConfig *self,
    char *buffer, size_t buffer_size, size_t *written )
{
    return KConfig_Get_Repository_String
        (self, buffer, buffer_size, written, "http/proxy/path");
}

LIB_EXPORT rc_t CC KConfig_Set_Http_Proxy_Path
    ( KConfig *self, const char *value )
{
    return KConfig_Set_Repository_String(self, value, "http/proxy/path");
}

/* get/set enabled-state for HTTP proxy */
LIB_EXPORT rc_t CC KConfig_Get_Http_Proxy_Enabled
    ( const KConfig *self, bool *enabled, bool dflt )
{
    rc_t rc = 0;

    if ( self == NULL ) {
        rc = RC ( rcKFG, rcNode, rcReading, rcSelf, rcNull );
    }
    else if ( enabled == NULL ) {
        rc = RC ( rcKFG, rcNode, rcReading, rcParam, rcNull );
    }
    else {
        *enabled = dflt;
        KConfigReadBool ( self, "http/proxy/enabled", enabled );
        /* errors are ignored - then default value is returned */
    }

    return rc;
}

LIB_EXPORT rc_t CC KConfig_Set_Http_Proxy_Enabled
    ( KConfig *self, bool enabled )
{
    rc_t rc = 0;

    if ( self == NULL ) {
        rc = RC ( rcKFG, rcNode, rcWriting, rcSelf, rcNull );
    }
    else {
        rc = KConfigWriteBool( self, "http/proxy/enabled", enabled );
    }

    return rc;
}

/* -------------------------------------------------------------------------- */
/* get/set priority of environmnet vs. configuration for HTTP proxy */
LIB_EXPORT rc_t CC KConfig_Has_Http_Proxy_Env_Higher_Priority
    ( const KConfig *self, bool *enabled )
{
    rc_t rc = 0;

    if ( self == NULL ) {
        rc = RC ( rcKFG, rcNode, rcReading, rcSelf, rcNull );
    }
    else if ( enabled == NULL ) {
        rc = RC ( rcKFG, rcNode, rcReading, rcParam, rcNull );
    }
    else {
        String * res = NULL;
        * enabled = false;
        rc = KConfigReadString ( self, "/http/proxy/use", &res );
        if ( rc == 0 ) {
            String v;
            CONST_STRING ( & v, "env,kfg" );
            if ( StringEqual ( res, & v ) ) {
                * enabled = true;
            }
        } else {
            rc = 0;
        }
        free ( res );
    }

    return rc;
}
LIB_EXPORT rc_t CC KConfig_Set_Http_Proxy_Env_Higher_Priority
    ( KConfig *self, bool enabled )
{
    rc_t rc = 0;

    if ( self == NULL ) {
        rc = RC ( rcKFG, rcNode, rcWriting, rcSelf, rcNull );
    }
    else {
        rc = KConfigWriteString
            ( self, "/http/proxy/use", enabled ? "env,kfg" : "kfg,env" );
    }

    return rc;
}

/* -------------------------------------------------------------------------- */

LIB_EXPORT rc_t CC KConfig_Get_Home( const KConfig *self, char * buffer, size_t buffer_size, size_t * written )
{   return KConfig_Get_Repository_String( self, buffer, buffer_size, written, "HOME" ); }

#define USER_DEFAULT_PATH "/repository/user/default-path"
LIB_EXPORT rc_t CC KConfig_Get_Default_User_Path( const KConfig *self, char * buffer, size_t buffer_size, size_t * written )
{   return KConfig_Get_Repository_String( self, buffer, buffer_size, written, USER_DEFAULT_PATH ); }
LIB_EXPORT rc_t CC KConfig_Set_Default_User_Path( KConfig *self, const char * value )
{   return KConfig_Set_Repository_String( self, value, USER_DEFAULT_PATH ); }

/* -------------------------------------------------------------------------- */

#define PATH_REPOSITORY_REMOTE_DISABLED "/repository/remote/disabled"
LIB_EXPORT rc_t CC KConfig_Get_Remote_Access_Enabled
    ( const KConfig *self, bool * enabled )
{
    return KConfig_Get_Repository_State( self, enabled,
        true, true, PATH_REPOSITORY_REMOTE_DISABLED );
}
LIB_EXPORT rc_t CC KConfig_Set_Remote_Access_Enabled
    ( KConfig *self, bool enabled )
{
    return KConfig_Set_Repository_State( self, enabled,
        true, PATH_REPOSITORY_REMOTE_DISABLED );
}

#define PATH_REPOSITORY_REMOTE_MAIN_CGI_DISABLED \
    "/repository/remote/main/CGI/disabled"
LIB_EXPORT rc_t CC KConfig_Get_Remote_Main_Cgi_Access_Enabled
    ( const KConfig *self, bool * enabled )
{
    return KConfig_Get_Repository_State( self, enabled,
        true, true, PATH_REPOSITORY_REMOTE_MAIN_CGI_DISABLED );
}

#define PATH_REPOSITORY_REMOTE_AUX_NCBI_DISABLED \
    "/repository/remote/aux/NCBI/disabled"
LIB_EXPORT rc_t CC KConfig_Get_Remote_Aux_Ncbi_Access_Enabled
    ( const KConfig *self, bool * enabled )
{
    return KConfig_Get_Repository_State( self, enabled,
        true, true, PATH_REPOSITORY_REMOTE_DISABLED );
}

#define PATH_REPOSITORY_SITE_DISABLED "/repository/site/disabled"
#define PATH_REPOSITORY_USER_DISABLED "/repository/user/disabled"
LIB_EXPORT rc_t CC KConfig_Get_Site_Access_Enabled( const KConfig *self, bool * enabled )
{   return KConfig_Get_Repository_State( self, enabled, true, true, PATH_REPOSITORY_SITE_DISABLED ); }
LIB_EXPORT rc_t CC KConfig_Set_Site_Access_Enabled( KConfig *self, bool enabled )
{   return KConfig_Set_Repository_State( self, enabled, true, PATH_REPOSITORY_SITE_DISABLED ); }

LIB_EXPORT rc_t CC KConfig_Get_User_Access_Enabled( const KConfig *self, bool * enabled )
{   return KConfig_Get_Repository_State( self, enabled, true, true, PATH_REPOSITORY_USER_DISABLED ); }
LIB_EXPORT rc_t CC KConfig_Set_User_Access_Enabled( KConfig *self, bool enabled )
{   return KConfig_Set_Repository_State( self, enabled, true, PATH_REPOSITORY_USER_DISABLED ); }

#define PATH_ALLOW_ALL_CERTS "/tls/allow-all-certs"
LIB_EXPORT rc_t CC KConfig_Get_Allow_All_Certs( const KConfig *self, bool * enabled )
{   return KConfig_Get_Repository_State( self, enabled, false, false, PATH_ALLOW_ALL_CERTS ); }
LIB_EXPORT rc_t CC KConfig_Set_Allow_All_Certs( KConfig *self, bool enabled )
{   return KConfig_Set_Repository_State( self, enabled, false, PATH_ALLOW_ALL_CERTS ); }

#define PATH_REPOSITORY_USER_PUBLIC_DISABLED "/repository/user/main/public/disabled"
LIB_EXPORT rc_t CC KConfig_Get_User_Public_Enabled( const KConfig *self, bool * enabled )
{   return KConfig_Get_Repository_State( self, enabled, true, true, PATH_REPOSITORY_USER_PUBLIC_DISABLED ); }
LIB_EXPORT rc_t CC KConfig_Set_User_Public_Enabled( KConfig *self, bool enabled )
{   return KConfig_Set_Repository_State( self, enabled, true, PATH_REPOSITORY_USER_PUBLIC_DISABLED ); }

#define PATH_REPOSITORY_USER_PUBLIC_CACHE_DISABLED "/repository/user/main/public/cache-disabled"
LIB_EXPORT rc_t CC KConfig_Get_User_Public_Cached( const KConfig *self, bool * enabled )
{   return KConfig_Get_Repository_State( self, enabled, true, true, PATH_REPOSITORY_USER_PUBLIC_CACHE_DISABLED ); }
LIB_EXPORT rc_t CC KConfig_Set_User_Public_Cached( KConfig *self, bool enabled )
{   return KConfig_Set_Repository_State( self, enabled, true, PATH_REPOSITORY_USER_PUBLIC_CACHE_DISABLED ); }

#define PATH_REPOSITORY_USER_PROTECTED_CACHE_ENABLED "/repository/user/protected/%s/cache-enabled"
LIB_EXPORT rc_t CC KConfig_Get_User_Protected_Cached( const KConfig *self, bool * enabled, const char * name )
{   return KConfig_Get_Repository_State( self, enabled, false, false, PATH_REPOSITORY_USER_PROTECTED_CACHE_ENABLED, name ); }
LIB_EXPORT rc_t CC KConfig_Set_User_Protected_Cached( KConfig *self, bool enabled, const char * name )
{   return KConfig_Set_Repository_State( self, enabled, false, PATH_REPOSITORY_USER_PROTECTED_CACHE_ENABLED, name ); }

#define PATH_REPOSITORY_USER_PUBLIC_CACHE_LOCATION "/repository/user/main/public/root"
LIB_EXPORT rc_t CC KConfig_Get_User_Public_Cache_Location( const KConfig *self,
    char * value, size_t value_size, size_t * written )
{   return KConfig_Get_Repository_String( self, value, value_size, written, PATH_REPOSITORY_USER_PUBLIC_CACHE_LOCATION ); }
LIB_EXPORT rc_t CC KConfig_Set_User_Public_Cache_Location( KConfig *self, const char * value )
{   return KConfig_Set_Repository_String( self, value, PATH_REPOSITORY_USER_PUBLIC_CACHE_LOCATION ); }

/* ---------------------------------------------------------------------------------------------------- */

LIB_EXPORT rc_t CC KConfigGetProtectedRepositoryCount( const KConfig *self, uint32_t * count )
{
    rc_t rc = 0;
    if ( self == NULL )
        rc = RC ( rcKFG, rcNode, rcReading, rcSelf, rcNull );
    else if ( count == NULL )
        rc = RC ( rcKFG, rcNode, rcReading, rcParam, rcNull );
    else
    {
        const struct KConfigNode * node;
        rc = KConfigOpenNodeRead ( self, &node, "/repository/user/protected" );
        if ( rc == 0 )
        {
            struct KNamelist * names;
            rc = KConfigNodeListChildren ( node, &names );
            if ( rc == 0 )
            {
                rc = KNamelistCount ( names, count );
                KNamelistRelease ( names );
            }
            KConfigNodeRelease ( node );
        }
    }
    return rc;
}


LIB_EXPORT rc_t CC KConfigGetProtectedRepositoryName( const KConfig *self,
    uint32_t id, char * buffer, size_t buffer_size, size_t * written )
{
    rc_t rc = 0;
    if ( self == NULL )
        rc = RC ( rcKFG, rcNode, rcReading, rcSelf, rcNull );
    else if ( buffer == NULL )
        rc = RC ( rcKFG, rcNode, rcReading, rcParam, rcNull );
    else
    {
        const struct KConfigNode * node;
        rc = KConfigOpenNodeRead ( self, &node, "/repository/user/protected" );
        if ( rc == 0 )
        {
            struct KNamelist * names;
            rc = KConfigNodeListChildren ( node, &names );
            if ( rc == 0 )
            {
                const char * name;
                rc = KNamelistGet ( names, id, &name );
                if ( rc == 0 )
                    rc = string_printf( buffer, buffer_size, written, "%s", name );
                KNamelistRelease ( names );
            }
            KConfigNodeRelease ( node );
        }
    }
    return rc;
}


static rc_t get_root_dir_of_repository( const struct KConfigNode * node,
    const char * name, char * buffer, size_t buffer_size, size_t * written )
{
    const struct KConfigNode * sub_node;
    rc_t rc = KConfigNodeOpenNodeRead ( node, &sub_node, "%s/root", name );
    if ( rc == 0 )
    {
        struct String * S;
        rc = KConfigNodeReadString ( sub_node, &S );
        if ( rc == 0 )
        {
            rc = string_printf( buffer, buffer_size, written, "%S", S );
            StringWhack ( S );
        }
        KConfigNodeRelease ( sub_node );
    }
    return rc;
}


static rc_t get_description_of_repository( const struct KConfigNode * node,
    const char * name, char * buffer, size_t buffer_size, size_t * written )
{
    const struct KConfigNode * sub_node;
    rc_t rc = KConfigNodeOpenNodeRead (node, &sub_node, "%s/description", name);
    if ( rc == 0 )
    {
        struct String * S;
        rc = KConfigNodeReadString ( sub_node, &S );
        if ( rc == 0 )
        {
            rc = string_printf( buffer, buffer_size, written, "%S", S );
            StringWhack ( S );
        }
        KConfigNodeRelease ( sub_node );
    }
    return rc;
}


LIB_EXPORT rc_t CC KConfigGetProtectedRepositoryPathById( const KConfig *self,
    uint32_t id, char * buffer, size_t buffer_size, size_t * written )
{
    rc_t rc = 0;
    if ( self == NULL )
        rc = RC ( rcKFG, rcNode, rcReading, rcSelf, rcNull );
    else if ( buffer == NULL )
        rc = RC ( rcKFG, rcNode, rcReading, rcParam, rcNull );
    else
    {
        const struct KConfigNode * node;
        rc = KConfigOpenNodeRead ( self, &node, "/repository/user/protected" );
        if ( rc == 0 )
        {
            struct KNamelist * names;
            rc = KConfigNodeListChildren ( node, &names );
            if ( rc == 0 )
            {
                const char * name;
                rc = KNamelistGet ( names, id, &name );
                if ( rc == 0 )
                    rc = get_root_dir_of_repository( node, name, buffer, buffer_size, written );
                KNamelistRelease ( names );
            }
            KConfigNodeRelease ( node );
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC KConfigSetProtectedRepositoryPathById( KConfig *self, uint32_t id, const char * value )
{
    char repo_name[ 1024 ];
    size_t written;
    rc_t rc = KConfigGetProtectedRepositoryName( self, id, repo_name, sizeof repo_name, &written );
    if ( rc == 0 )
        rc = KConfig_Set_Repository_String( self, value, "/repository/user/protected/%s/root", repo_name );
    return rc;
}


LIB_EXPORT rc_t CC KConfigGetProtectedRepositoryIdByName
    (const KConfig *self, const char *name, uint32_t *id)
{
    if (self == NULL)
        return RC(rcKFG, rcNode, rcReading, rcSelf, rcNull);
    else if (name == NULL || id == NULL)
        return RC(rcKFG, rcNode, rcReading, rcParam, rcNull);
    else {
        const struct KConfigNode *node = NULL;
        rc_t rc
            = KConfigOpenNodeRead(self, &node, "/repository/user/protected");
        if (rc == 0) {
            struct KNamelist *names = NULL;
            rc = KConfigNodeListChildren(node, &names);
            if (rc == 0) {
                uint32_t count = 0;
                rc = KNamelistCount(names, &count);
                if (rc == 0) {
                    if (count == 0)
                        rc = RC(rcKFG, rcNode, rcReading, rcName, rcNotFound);
                    else {        /* loop through the names to find the one */
                        uint32_t i = 0; /* which matches the name parameter */
                        bool found = false;
                        size_t name_size = string_size(name);
                        for (i = 0; i < count && rc == 0; ++i) {
                            const char *s = NULL;
                            rc = KNamelistGet(names, i, &s);
                            if ( rc == 0 && s != NULL ) {
                                size_t s_size = string_size(s);
                                if (name_size == s_size) {
                                    int cmp = string_cmp(name, name_size,
                                        s, s_size, (uint32_t)s_size);
                                    found = (cmp == 0);
                                    if (found) {
                                        *id = i;
                                        break;
                                    }
                                }
                            }
                        }
                        if (rc == 0 && !found) {
                            rc = RC
                                (rcKFG, rcNode, rcReading, rcName, rcNotFound);
                        }
                    }
                }
                KNamelistRelease(names);
            }
            KConfigNodeRelease(node);
        }
        return rc;
    }
}

LIB_EXPORT rc_t CC KConfigGetProtectedRepositoryPathByName( const KConfig *self,
    const char * name, char * buffer, size_t buffer_size, size_t * written )
{
    rc_t rc = 0;
    if ( self == NULL )
        rc = RC ( rcKFG, rcNode, rcReading, rcSelf, rcNull );
    else if ( buffer == NULL )
        rc = RC ( rcKFG, rcNode, rcReading, rcParam, rcNull );
    else
    {
        const struct KConfigNode * node;
        rc = KConfigOpenNodeRead ( self, &node, "/repository/user/protected" );
        if ( rc == 0 )
        {
            struct KNamelist * names;
            rc = KConfigNodeListChildren ( node, &names );
            if ( rc == 0 )
            {
                uint32_t count;
                rc = KNamelistCount ( names, &count );
                if ( rc == 0 )
                {
                    if ( count == 0 )
                        rc = RC ( rcKFG, rcNode, rcReading, rcName, rcNotFound );
                    else
                    {
                        /* loop through the names to find the one which matches the name parameter */
                        uint32_t i;
                        bool found = false;
                        size_t name_size = string_size( name );
                        for ( i = 0; !found && i < count && rc == 0; ++i )
                        {
                            const char * s = NULL;
                            rc = KNamelistGet ( names, i, &s );
                            if ( rc == 0 && s != NULL )
                            {
                                size_t s_size = string_size( s );
                                if ( name_size == s_size )
                                {
                                    int cmp = string_cmp ( name, name_size,
                                        s, s_size, (uint32_t)s_size);
                                    found = ( cmp == 0 );
                                    if ( found )
                                        rc = get_root_dir_of_repository( node, s, buffer, buffer_size, written );
                                }
                            }
                        }
                        if ( rc == 0 && !found )
                            rc = RC ( rcKFG, rcNode, rcReading, rcName, rcNotFound );
                    }
                }
                KNamelistRelease ( names );
            }
            KConfigNodeRelease ( node );
        }
    }
    return rc;
}


LIB_EXPORT rc_t CC KConfigGetProtectedRepositoryDescriptionByName(
    const KConfig *self,
    const char * name, char * buffer, size_t buffer_size, size_t * written )
{
    rc_t rc = 0;
    if ( self == NULL )
        rc = RC ( rcKFG, rcNode, rcReading, rcSelf, rcNull );
    else if ( buffer == NULL )
        rc = RC ( rcKFG, rcNode, rcReading, rcParam, rcNull );
    else
    {
        const struct KConfigNode * node;
        rc = KConfigOpenNodeRead ( self, &node, "/repository/user/protected" );
        if ( rc == 0 )
        {
            struct KNamelist * names;
            rc = KConfigNodeListChildren ( node, &names );
            if ( rc == 0 )
            {
                uint32_t count;
                rc = KNamelistCount ( names, &count );
                if ( rc == 0 )
                {
                    if ( count == 0 )
                        rc = RC( rcKFG, rcNode, rcReading, rcName, rcNotFound );
                    else
                    {
   /* loop through the names to find the one which matches the name parameter */
                        uint32_t i;
                        bool found = false;
                        size_t name_size = string_size( name );
                        for ( i = 0; !found && i < count && rc == 0; ++i )
                        {
                            const char * s = NULL;
                            rc = KNamelistGet ( names, i, &s );
                            if ( rc == 0 && s != NULL )
                            {
                                size_t s_size = string_size( s );
                                if ( name_size == s_size )
                                {
                                    int cmp = string_cmp ( name, name_size,
                                        s, s_size, (uint32_t)s_size);
                                    found = ( cmp == 0 );
                                    if ( found )
                                        rc = get_description_of_repository(node,
                                            s, buffer, buffer_size, written );
                                }
                            }
                        }
                        if ( rc == 0 && !found )
                            rc = RC
                                (rcKFG, rcNode, rcReading, rcName, rcNotFound);
                    }
                }
                KNamelistRelease ( names );
            }
            KConfigNodeRelease ( node );
        }
    }
    return rc;
}


LIB_EXPORT rc_t CC KConfigDoesProtectedRepositoryExist( const KConfig *self, const char * name, bool * res )
{
    rc_t rc = 0;
    if ( self == NULL )
        rc = RC ( rcKFG, rcNode, rcReading, rcSelf, rcNull );
    else if ( res == NULL )
        rc = RC ( rcKFG, rcNode, rcReading, rcParam, rcNull );
    else
    {
        const struct KConfigNode * node;
        *res = false;
        rc = KConfigOpenNodeRead ( self, &node, "/repository/user/protected" );
        if ( rc == 0 )
        {
            struct KNamelist * names;
            rc = KConfigNodeListChildren ( node, &names );
            if ( rc == 0 )
            {
                uint32_t count;
                rc = KNamelistCount ( names, &count );
                if ( rc == 0 )
                {
                    if ( count == 0 )
                        rc = RC ( rcKFG, rcNode, rcReading, rcName, rcNotFound );
                    else
                    {
                        /* loop through the names to find the one which matches the name parameter */
                        uint32_t i;
                        bool found = false;
                        size_t name_size = string_size( name );
                        for ( i = 0; !found && i < count && rc == 0; ++i )
                        {
                            const char * s = NULL;
                            rc = KNamelistGet ( names, i, &s );
                            if ( rc == 0 && s != NULL )
                            {
                                size_t s_size = string_size( s );
                                if ( name_size == s_size )
                                {
                                    int cmp = string_cmp ( name, name_size,
                                        s, s_size, (uint32_t)s_size);
                                    found = ( cmp == 0 );
                                }
                            }
                        }
                        if ( rc == 0 && found )
                            *res = true;
                    }
                }
                KNamelistRelease ( names );
            }
            KConfigNodeRelease ( node );
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC KConfigGetProtectedRepositoryEnabledById( const KConfig *self, uint32_t id, bool * enabled )
{
    char repo_name[ 1024 ];
    size_t written;
    rc_t rc = KConfigGetProtectedRepositoryName( self, id, repo_name, sizeof repo_name, &written );
    if ( rc == 0 )
        rc = KConfig_Get_Repository_State( self, enabled, true, true, "/repository/user/protected/%s/disabled", repo_name );
    return rc;
}


LIB_EXPORT rc_t CC KConfigSetProtectedRepositoryEnabledById( KConfig *self, uint32_t id, bool enabled )
{
    char repo_name[ 1024 ];
    size_t written;
    rc_t rc = KConfigGetProtectedRepositoryName( self, id, repo_name, sizeof repo_name, &written );
    if ( rc == 0 )
        rc = KConfig_Set_Repository_State( self, enabled, true, "/repository/user/protected/%s/disabled", repo_name );
    return rc;
}


LIB_EXPORT rc_t CC KConfigGetProtectedRepositoryCachedById( const KConfig *self, uint32_t id, bool * enabled )
{
    char repo_name[ 1024 ];
    size_t written;
    rc_t rc = KConfigGetProtectedRepositoryName( self, id, repo_name, sizeof repo_name, &written );
    if ( rc == 0 )
        rc = KConfig_Get_Repository_State( self, enabled, false, false, "/repository/user/protected/%s/cache-enabled", repo_name );
    return rc;
}


LIB_EXPORT rc_t CC KConfigSetProtectedRepositoryCachedById( KConfig *self, uint32_t id, bool enabled )
{
    char repo_name[ 1024 ];
    size_t written;
    rc_t rc = KConfigGetProtectedRepositoryName( self, id, repo_name, sizeof repo_name, &written );
    if ( rc == 0 )
        rc = KConfig_Set_Repository_State( self, enabled, false, "/repository/user/protected/%s/cache-enabled", repo_name );
    return rc;
}

/* ---------------------------------------------------------------------------------------------------- */

#define PREFETCH_DOWNLOAD_TO_CACHE "/tools/prefetch/download_to_cache"
LIB_EXPORT rc_t CC KConfig_Get_Prefetch_Download_To_Cache ( const KConfig *self, bool * value )
{
    rc_t rc = KConfigReadBool ( self, PREFETCH_DOWNLOAD_TO_CACHE, value );
    if ( GetRCState ( rc ) == rcNotFound)
    {
        * value = true;
        rc = 0;
    }
    return rc;
}
LIB_EXPORT rc_t CC KConfig_Set_Prefetch_Download_To_Cache ( KConfig *self, bool value )
{
    return KConfigWriteBool( self, PREFETCH_DOWNLOAD_TO_CACHE, value );
}

#define USER_ACCEPT_AWS_CHARGES "/libs/cloud/accept_aws_charges"
LIB_EXPORT rc_t CC KConfig_Get_User_Accept_Aws_Charges ( const KConfig *self, bool * value )
{
    rc_t rc = KConfigReadBool ( self, USER_ACCEPT_AWS_CHARGES, value );
    if ( GetRCState ( rc ) == rcNotFound)
    {
        * value = false;
        rc = 0;
    }
    return rc;
}
LIB_EXPORT rc_t CC KConfig_Set_User_Accept_Aws_Charges ( KConfig *self, bool value )
{
    return KConfigWriteBool( self, USER_ACCEPT_AWS_CHARGES, value );
}

#define USER_ACCEPT_GCP_CHARGES "/libs/cloud/accept_gcp_charges"
LIB_EXPORT rc_t CC KConfig_Get_User_Accept_Gcp_Charges ( const KConfig *self, bool * value )
{
    rc_t rc = KConfigReadBool ( self, USER_ACCEPT_GCP_CHARGES, value );
    if ( GetRCState ( rc ) == rcNotFound)
    {
        * value = false;
        rc = 0;
    }
    return rc;
}
LIB_EXPORT rc_t CC KConfig_Set_User_Accept_Gcp_Charges ( KConfig *self, bool value )
{
    return KConfigWriteBool( self, USER_ACCEPT_GCP_CHARGES, value );
}

#define REPORT_CLOUD_INSTANCE_IDENTITY "/libs/cloud/report_instance_identity"
LIB_EXPORT rc_t CC KConfig_Get_Report_Cloud_Instance_Identity ( const KConfig *self, bool * value )
{
    rc_t rc = KConfigReadBool ( self, REPORT_CLOUD_INSTANCE_IDENTITY, value );
    if ( GetRCState ( rc ) == rcNotFound)
    {
        * value = false;
        rc = 0;
    }
    return rc;
}
LIB_EXPORT rc_t CC KConfig_Set_Report_Cloud_Instance_Identity ( KConfig *self, bool value )
{
    return KConfigWriteBool( self, REPORT_CLOUD_INSTANCE_IDENTITY, value );
}

#define TEMP_CACHE "/libs/temp_cache"
LIB_EXPORT rc_t CC
KConfig_Get_Temp_Cache( const KConfig *self,
    char * value, size_t value_size, size_t * written )
{
    return KConfig_Get_Repository_String( self, value, value_size, written, TEMP_CACHE );
}
LIB_EXPORT rc_t CC
KConfig_Set_Temp_Cache( KConfig *self, const char * value )
{
    return KConfig_Set_Repository_String( self, value, TEMP_CACHE );
}

#define GCP_CREDENTIAL_FILE "/gcp/credential_file"
LIB_EXPORT rc_t CC
KConfig_Get_Gcp_Credential_File( const KConfig *self,
    char * value, size_t value_size, size_t * written )
{
    return KConfig_Get_Repository_String( self, value, value_size, written, GCP_CREDENTIAL_FILE );
}
LIB_EXPORT rc_t CC
KConfig_Set_Gcp_Credential_File( KConfig *self, const char * value )
{
    return KConfig_Set_Repository_String( self, value, GCP_CREDENTIAL_FILE );
}

#define AWS_CREDENTIAL_FILE "/aws/credential_file"
LIB_EXPORT rc_t CC
KConfig_Get_Aws_Credential_File( const KConfig *self,
    char * value, size_t value_size, size_t * written )
{
    return KConfig_Get_Repository_String( self, value, value_size, written, AWS_CREDENTIAL_FILE );
}
LIB_EXPORT rc_t CC
KConfig_Set_Aws_Credential_File( KConfig *self, const char * value )
{
    return KConfig_Set_Repository_String( self, value, AWS_CREDENTIAL_FILE );
}

#define AWS_PROFILE "/aws/profile"
LIB_EXPORT rc_t CC
KConfig_Get_Aws_Profile( const KConfig *self,
    char * value, size_t value_size, size_t * written )
{
    rc_t rc = 0;

    size_t dummy = 0;
    if (written == NULL)
        written = &dummy;

    rc = KConfig_Get_Repository_String( self, value, value_size, written,
        AWS_PROFILE );
    if ( GetRCState ( rc ) == rcNotFound || ( rc == 0 && * written == 0 ) )
    {
        * written = string_copy_measure ( value, value_size, "default" );
        rc = 0;
    }

    return rc;
}
LIB_EXPORT rc_t CC
KConfig_Set_Aws_Profile( KConfig *self, const char * value )
{
    return KConfig_Set_Repository_String( self, value, AWS_PROFILE );
}

#define CACHE_AMOUNT "/libs/cache_amount"
LIB_EXPORT rc_t CC KConfig_Get_Cache_Amount ( const KConfig *self, uint32_t * value )
{
    rc_t rc;
    if ( self == NULL )
        rc = RC ( rcKFG, rcNode, rcReading, rcSelf, rcNull );
    else if ( value == NULL )
        rc = RC ( rcKFG, rcNode, rcReading, rcParam, rcNull );
    else
    {
        uint64_t long_value = 0;
        rc = KConfigReadU64 ( self, CACHE_AMOUNT, &long_value );
        if ( rc == 0 || GetRCState ( rc ) == rcNotFound )
        {
            * value = long_value & 0xFFFFFFFF;
            rc = 0;
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC KConfig_Set_Cache_Amount( KConfig *self, uint32_t value )
{
    rc_t rc;
    if ( self == NULL )
        rc = RC ( rcKFG, rcNode, rcReading, rcSelf, rcNull );
    else
    {
        char buff[ 128 ];
        size_t num_writ;
        rc = string_printf ( buff, sizeof buff, &num_writ, "%u", value );
        if ( rc == 0 )
            rc = KConfigWriteString( self, CACHE_AMOUNT, buff );
    }
    return rc;
}

/* ------------------------------------------------------------------------ */
static rc_t get_uint32_t_value( const KConfig *self, const char * key, uint32_t * value, uint32_t dflt )
{
    rc_t rc;
    if ( self == NULL )
        rc = RC ( rcKFG, rcNode, rcReading, rcSelf, rcNull );
    else if ( value == NULL )
        rc = RC ( rcKFG, rcNode, rcReading, rcParam, rcNull );
    else
    {
        uint64_t long_value = dflt;
        rc = KConfigReadU64 ( self, key, &long_value );
        if ( rc == 0 )
            * value = ( long_value & 0xFFFFFFFF );
        rc = 0;
    }
    return rc;
}

static rc_t set_uint32_t_value( KConfig *self, const char * key, uint32_t value )
{
    rc_t rc;
    if ( self == NULL )
        rc = RC ( rcKFG, rcNode, rcReading, rcSelf, rcNull );
    else
    {
        char buff[ 128 ];
        size_t num_writ;
        rc = string_printf ( buff, sizeof buff, &num_writ, "%u", value );
        if ( rc == 0 )
            rc = KConfigWriteString( self, key, buff );
    }
    return rc;
}

static rc_t get_bool_value( const KConfig *self, const char * key, bool * value, bool dflt )
{
    rc_t rc;
    if ( self == NULL )
        rc = RC ( rcKFG, rcNode, rcReading, rcSelf, rcNull );
    else if ( value == NULL )
        rc = RC ( rcKFG, rcNode, rcReading, rcParam, rcNull );
    else
    {
        bool res = dflt;
        rc = KConfigReadBool ( self, key, &res );
        if ( rc == 0 )
            * value = res;
        rc = 0;
    }
    return rc;
}

static rc_t set_bool_value( KConfig *self, const char * key, bool value )
{
    rc_t rc;
    if ( self == NULL )
        rc = RC ( rcKFG, rcNode, rcReading, rcSelf, rcNull );
    else
        rc = KConfigWriteBool( self, key, value );
    return rc;
}

#define CACHE_TEE_VERSION "/CACHINGPARAMS/CACHETEEVER"
LIB_EXPORT rc_t CC KConfig_Get_CacheTeeVersion ( const KConfig *self, uint32_t * value, uint32_t dflt )
{ return get_uint32_t_value( self, CACHE_TEE_VERSION, value, dflt ); }
LIB_EXPORT rc_t CC KConfig_Set_CacheTeeVersion( KConfig *self, uint32_t value )
{ return set_uint32_t_value( self, CACHE_TEE_VERSION, value ); }

#define CACHE_TEE_CLUSTER_FACTOR "/CACHINGPARAMS/CACHETEECLUSTERFACTOR"
LIB_EXPORT rc_t CC KConfig_Get_CacheClusterFactorBits( const KConfig *self, uint32_t * value, uint32_t dflt )
{ return get_uint32_t_value( self, CACHE_TEE_CLUSTER_FACTOR, value, dflt ); }
LIB_EXPORT rc_t CC KConfig_Set_CacheClusterFactorBits( KConfig *self, uint32_t value )
{ return set_uint32_t_value( self, CACHE_TEE_CLUSTER_FACTOR, value ); }

#define CACHE_TEE_PAGE_SIZE "/CACHINGPARAMS/CACHETEEPAGESIZE"
LIB_EXPORT rc_t CC KConfig_Get_CachePageSizeBits( const KConfig *self, uint32_t * value, uint32_t dflt )
{ return get_uint32_t_value( self, CACHE_TEE_PAGE_SIZE, value, dflt ); }
LIB_EXPORT rc_t CC KConfig_Set_CachePageSizeBits( KConfig *self, uint32_t value )
{ return set_uint32_t_value( self, CACHE_TEE_PAGE_SIZE, value ); }

#define CACHE_BLOCKSIZE "/CACHINGPARAMS/BLOCKSIZE"
LIB_EXPORT rc_t CC KConfig_Get_CacheBlockSize ( const KConfig *self, size_t * value, size_t dflt )
{
    rc_t rc;
    if ( self == NULL )
        rc = RC ( rcKFG, rcNode, rcReading, rcSelf, rcNull );
    else if ( value == NULL )
        rc = RC ( rcKFG, rcNode, rcReading, rcParam, rcNull );
    else
    {
        uint64_t long_value = dflt;
        rc = KConfigReadU64 ( self, CACHE_BLOCKSIZE, &long_value );
        if ( rc == 0 )
            * value = long_value;
        rc = 0;
    }
    return rc;
}

LIB_EXPORT rc_t CC KConfig_Set_CacheBlockSize( KConfig *self, size_t value )
{
    rc_t rc;
    if ( self == NULL )
        rc = RC ( rcKFG, rcNode, rcReading, rcSelf, rcNull );
    else
    {
        char buff[ 128 ];
        size_t num_writ;
        rc = string_printf ( buff, sizeof buff, &num_writ, "%u", value );
        if ( rc == 0 )
            rc = KConfigWriteString( self, CACHE_BLOCKSIZE, buff );
    }
    return rc;
}

#define CACHE_PAGE_COUNT "/CACHINGPARAMS/PAGECOUNT"
LIB_EXPORT rc_t CC KConfig_Get_CachePageCount( const KConfig *self, uint32_t * value, uint32_t dflt )
{ return get_uint32_t_value( self, CACHE_PAGE_COUNT, value, dflt ); }
LIB_EXPORT rc_t CC KConfig_Set_CachePageCount( KConfig *self, uint32_t value )
{ return set_uint32_t_value( self, CACHE_PAGE_COUNT, value ); }

#define CACHE_LOG_USE_CWD "/CACHINGPARAMS/LOGUSECWD"
LIB_EXPORT rc_t CC KConfig_Get_CacheLogUseCWD( const KConfig *self, bool * value, bool dflt )
{ return get_bool_value( self, CACHE_LOG_USE_CWD, value, dflt ); }
LIB_EXPORT rc_t CC KConfig_Set_CacheLogUseCWD( KConfig *self, bool value )
{ return set_bool_value( self, CACHE_LOG_USE_CWD, value ); }

#define CACHE_LOG_APPEND "/CACHINGPARAMS/LOGAPPEND"
LIB_EXPORT rc_t CC KConfig_Get_CacheLogAppend( const KConfig *self, bool * value, bool dflt )
{ return get_bool_value( self, CACHE_LOG_APPEND, value, dflt ); }
LIB_EXPORT rc_t CC KConfig_Set_CacheLogAppend( KConfig *self, bool value )
{ return set_bool_value( self, CACHE_LOG_APPEND, value ); }

#define CACHE_LOG_TIMED "/CACHINGPARAMS/LOGTIMED"
LIB_EXPORT rc_t CC KConfig_Get_CacheLogTimed( const KConfig *self, bool * value, bool dflt )
{ return get_bool_value( self, CACHE_LOG_TIMED, value, dflt ); }
LIB_EXPORT rc_t CC KConfig_Set_CacheLogTimed( KConfig *self, bool value )
{ return set_bool_value( self, CACHE_LOG_TIMED, value ); }

#define CACHE_LOG_OUTER "/CACHINGPARAMS/LOGOUTER"
LIB_EXPORT rc_t CC KConfig_Get_CacheLogOuter( const KConfig *self, bool * value, bool dflt )
{ return get_bool_value( self, CACHE_LOG_OUTER, value, dflt ); }
LIB_EXPORT rc_t CC KConfig_Set_CacheLogOuter( KConfig *self, bool value )
{ return set_bool_value( self, CACHE_LOG_OUTER, value ); }

#define CACHE_LOG_INNER "/CACHINGPARAMS/LOGINNER"
LIB_EXPORT rc_t CC KConfig_Get_CacheLogInner( const KConfig *self, bool * value, bool dflt )
{ return get_bool_value( self, CACHE_LOG_INNER, value, dflt ); }
LIB_EXPORT rc_t CC KConfig_Set_CacheLogInner( KConfig *self, bool value )
{ return set_bool_value( self, CACHE_LOG_INNER, value ); }

#define DEBUG_CACHEING "/CACHINGPARAMS/DEBUG"
LIB_EXPORT rc_t CC KConfig_Get_CacheDebug( const KConfig *self, bool * value, bool dflt )
{ return get_bool_value( self, DEBUG_CACHEING, value, dflt ); }
LIB_EXPORT rc_t CC KConfig_Set_CacheDebug( KConfig *self, bool value )
{ return set_bool_value( self, DEBUG_CACHEING, value ); }

#define GUID_KEY "LIBS/GUID"
LIB_EXPORT rc_t CC KConfig_Get_GUID( const KConfig *self, char * value, size_t value_size, size_t * written )
{
    size_t local_written;
    rc_t rc = KConfig_Get_Repository_String( self, value, value_size, &local_written, GUID_KEY );
    if ( rc == 0 )
    {
        if ( written != NULL )
            *written = local_written;
    }
    return rc;

}

LIB_EXPORT rc_t CC KConfig_Set_GUID( KConfig *self, const char * value )
{
    return KConfig_Set_Repository_String( self, value, GUID_KEY );
}
