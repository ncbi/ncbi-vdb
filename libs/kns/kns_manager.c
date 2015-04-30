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

#include <kns/extern.h>

#include <klib/refcount.h>
#include <klib/rc.h>
#include <kfs/dyload.h>
#include <kfs/directory.h>
#include <kfs/impl.h>

#include "kns_mgr_priv.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <sysalloc.h>

static const char knsmanager_classname [] = "KNSManager";
static struct KNSManager * kns_mgr_singleton = NULL;
static KDylib *lib_curl_handle = NULL;


LIB_EXPORT rc_t CC KNSManagerAddRef ( const struct KNSManager *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAdd( &self->refcount, knsmanager_classname ) )
        {
        case krefOkay:
            break;
        case krefZero:
            return RC( rcNS, rcMgr, rcAttaching, rcRefcount, rcIncorrect);
        case krefLimit:
            return RC( rcNS, rcMgr, rcAttaching, rcRefcount, rcExhausted);
        case krefNegative:
            return RC( rcNS, rcMgr, rcAttaching, rcRefcount, rcInvalid);
        default:
            return RC( rcNS, rcMgr, rcAttaching, rcRefcount, rcUnknown);
        }
    }
    return 0;
}


static rc_t KNSManager_Make_DlCurl( KDyld ** dl )
{
    rc_t rc = KDyldMake ( dl );
#if ! WINDOWS
    if ( rc == 0 )
    {
        const KDirectory * dir;
        rc = KDyldHomeDirectory ( *dl, &dir, ( fptr_t ) KNSManager_Make_DlCurl );
        if ( rc == 0 )
        {
            struct KSysDir const *sysdir = KDirectoryGetSysDir ( dir );
            if ( sysdir != NULL )
            {
                char int_path[ 4096 ];
                rc = KSysDirRealPath ( sysdir, int_path, sizeof int_path, "." );
                if ( rc == 0 )
                {
                    KDyldAddSearchPath ( *dl, int_path );
                }
            }
            KDirectoryRelease( dir );
        }
        rc = 0;
    }
#endif
    return rc;
}

static rc_t KNSManagerLoadLib( struct KNSManager *self )
{
#ifdef HAVE_LIBCURL /* just link directly */
    self->curl_easy_init_fkt      = &curl_easy_init;
    self->curl_easy_cleanup_fkt   = &curl_easy_cleanup;
    self->curl_easy_setopt_fkt    = &curl_easy_setopt;
    self->curl_easy_perform_fkt   = &curl_easy_perform;
    self->curl_easy_getinfo_fkt   = &curl_easy_getinfo;
    self->curl_slist_append_fkt   = &curl_slist_append;
    self->curl_slist_free_all_fkt = &curl_slist_free_all;

    return 0;
#else
    KDyld *dl;
    /* make a dynamic-library loader */
    rc_t rc = KDyldMake ( &dl );
    if ( rc == 0 )
    {
        /* load the curl-library */
        rc = KDyldLoadLib( dl, &lib_curl_handle, LPFX "curl" SHLX );
        if ( rc != 0 )
        {
            KDyldRelease ( dl );
            rc = KNSManager_Make_DlCurl( &dl );
            if ( rc == 0 )
            {
                rc = KDyldLoadLib( dl, &lib_curl_handle, LPFX "curl" SHLX );
            }
        }
        if ( rc == 0 )
        {
            KSymAddr *sym;

            /* resolve symbols */

            /* curl_easy_init() */
            rc = KDylibSymbol( lib_curl_handle, &sym, "curl_easy_init" );
            if ( rc == 0 )
            {
                KSymAddrAsFunc( sym, ( fptr_t* ) &(self->curl_easy_init_fkt) );
                KSymAddrRelease( sym );
            }

            /* curl_easy_cleanup() */
            if ( rc == 0 )
            {
                rc = KDylibSymbol( lib_curl_handle, &sym, "curl_easy_cleanup" );
                KSymAddrAsFunc( sym, ( fptr_t* ) &(self->curl_easy_cleanup_fkt) );
                KSymAddrRelease( sym );
            }

            /* curl_easy_setopt() */
            if ( rc == 0 )
            {
                rc = KDylibSymbol( lib_curl_handle, &sym, "curl_easy_setopt" );
                KSymAddrAsFunc( sym, ( fptr_t* ) &(self->curl_easy_setopt_fkt) );
                KSymAddrRelease( sym );
            }

            /* curl_easy_perform() */
            if ( rc == 0 )
            {
                rc = KDylibSymbol( lib_curl_handle, &sym, "curl_easy_perform" );
                KSymAddrAsFunc( sym, ( fptr_t* ) &(self->curl_easy_perform_fkt) );
                KSymAddrRelease( sym );
            }

            /* curl_easy_getinfo() */
            if ( rc == 0 )
            {
                rc = KDylibSymbol( lib_curl_handle, &sym, "curl_easy_getinfo" );
                KSymAddrAsFunc( sym, ( fptr_t* ) &(self->curl_easy_getinfo_fkt) );
                KSymAddrRelease( sym );
            }

            /* curl_slist_append() */
            if ( rc == 0 )
            {
                rc = KDylibSymbol( lib_curl_handle, &sym, "curl_slist_append" );
                KSymAddrAsFunc( sym, ( fptr_t* ) &(self->curl_slist_append_fkt) );
                KSymAddrRelease( sym );
            }

            /* curl_version() */
            if ( rc == 0 )
            {
                rc = KDylibSymbol( lib_curl_handle, &sym, "curl_version" );
                KSymAddrAsFunc( sym, ( fptr_t* ) &(self->curl_version_fkt) );
                KSymAddrRelease( sym );
            }

            /* curl_slist_free_all() */
            if ( rc == 0 )
            {
                rc = KDylibSymbol( lib_curl_handle, &sym, "curl_slist_free_all" );
                KSymAddrAsFunc( sym, ( fptr_t* ) &(self->curl_slist_free_all_fkt) );
                KSymAddrRelease( sym );
            }

            /* bail on error */
            if ( rc != 0 )
            {
                KDylibRelease ( lib_curl_handle );
                lib_curl_handle = NULL;
                self->curl_easy_init_fkt = NULL;
                self->curl_easy_cleanup_fkt = NULL;
                self->curl_easy_setopt_fkt = NULL;
                self->curl_easy_perform_fkt = NULL;
                self->curl_slist_append_fkt = NULL;
                self->curl_version_fkt = NULL;
                self->curl_easy_getinfo_fkt = NULL;
            }
        }
        KDyldRelease ( dl );
    }

    return rc;
#endif
}


LIB_EXPORT rc_t CC KNSManagerMake( struct KNSManager **self )
{
    rc_t rc = 0;

    if ( self == NULL )
        return RC( rcNS, rcMgr, rcConstructing, rcSelf, rcNull );

    *self = kns_mgr_singleton;
    if ( kns_mgr_singleton != NULL )
    {
/*      fprintf(stderr, "%p KNSManagerMake(KNSManagerAddRef)\n", *self); */
        rc = KNSManagerAddRef( kns_mgr_singleton );
        if ( rc != 0 )
            *self = NULL;
    }
    else
    {
        struct KNSManager *tmp;
        tmp = calloc( 1, sizeof * tmp );
        if ( tmp == NULL )
            rc = RC( rcNS, rcMgr, rcConstructing, rcMemory, rcExhausted );
        else
        {
            rc = KNSManagerInit ( tmp );
            if ( rc != 0 )
            {
                free ( tmp );
                tmp = NULL;
            }
            else
            {
                tmp->create_rc = KNSManagerLoadLib( tmp );
                KRefcountInit( &tmp->refcount, 1, "KNS", "make", knsmanager_classname );

/*              fprintf(stderr,
                    "%p KNSManagerLoadLib = %d\n", tmp, tmp->create_rc); */
            }
        }
        *self = tmp;
        kns_mgr_singleton = tmp;
    }

    return rc;
}


LIB_EXPORT rc_t CC KNSManagerAvail( const struct KNSManager *self )
{
    if ( self == NULL )
        return RC( rcNS, rcMgr, rcReading, rcSelf, rcNull );
    return self->create_rc;
}


LIB_EXPORT rc_t CC KNSManagerCurlVersion( const struct KNSManager *self, const char ** version_string )
{
    if ( self == NULL )
        return RC( rcNS, rcMgr, rcReading, rcSelf, rcNull );
    if ( version_string == NULL )
        return RC( rcNS, rcMgr, rcReading, rcParam, rcNull );
    if ( self->create_rc != 0 )
        return self->create_rc;
    *version_string = self->curl_version_fkt();
    return 0;
}


LIB_EXPORT void KNSManagerSetVerbose ( struct KNSManager *self, bool verbosity )
{
    if ( self != NULL )
        self->verbose = verbosity;
}


LIB_EXPORT bool KNSManagerIsVerbose ( struct KNSManager *self )
{
    if ( self != NULL )
        return self->verbose;
    else
        return false;
}


static rc_t KNSManagerDestroy( struct KNSManager *self )
{
    if ( self == NULL )
        return RC( rcNS, rcFile, rcDestroying, rcSelf, rcNull );

/*  fprintf(stderr, "%p KNSManagerDestroy\n", self); */

    KDylibRelease ( lib_curl_handle );
    lib_curl_handle = NULL;

    KNSManagerCleanup ( self );

    KRefcountWhack( &self->refcount, knsmanager_classname );

    memset(self, 0, sizeof *self);
    free( self );
    kns_mgr_singleton = NULL;
    
    return 0;
}


LIB_EXPORT rc_t CC KNSManagerRelease( const struct KNSManager *self )
{
    rc_t rc = 0;
    if ( self != NULL )
    {
        switch ( KRefcountDrop( &self->refcount, knsmanager_classname ) )
        {
        case krefOkay:
        case krefZero:
/*        fprintf(stderr, "%p KNSManagerRelease(!KNSManagerDestroy)\n", self);*/
            break;
        case krefWhack:
            rc = KNSManagerDestroy( ( struct KNSManager * )self );
            break;
        case krefNegative:
            return RC( rcNS, rcMgr, rcAttaching, rcRefcount, rcInvalid );
        default:
            rc = RC( rcNS, rcMgr, rcAttaching, rcRefcount, rcUnknown );
            break;            
        }
    }
    return rc;
}
