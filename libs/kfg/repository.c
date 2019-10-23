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

#include <kfg/extern.h>

#include <kfg/repository.h>
#include <kfg/kfg-priv.h> /* KConfigMakeLocal */
#include <kfg/ngc.h>

#include <kfs/file.h>
#include <kfs/directory.h>
#include <kfs/impl.h>
#include <klib/refcount.h>
#include <klib/text.h>
#include <klib/printf.h>
#include <klib/vector.h>
#include <klib/namelist.h>
#include <klib/rc.h>

#include "kfg-priv.h" /* KConfigGetNgcFile */
#include "ngc-priv.h"

#include <sysalloc.h>

#include <assert.h>
#include <ctype.h> /* isdigit */
#include <os-native.h>
#include <stdlib.h>
#include <string.h>

#include <limits.h> /* PATH_MAX */
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

/*--------------------------------------------------------------------------
 * KRepository
 *  presents structured access to a storage repository
 *  as modeled in KConfig.
 *
 *  all objects are obtained via KRepositoryMgr ( see below )
 */
struct KRepository
{
    const KConfigNode *node;
    String name;
    KRefcount refcount;
    KRepCategory category;
    KRepSubCategory subcategory;
};


/* Whack
 */
static
rc_t KRepositoryWhack ( KRepository *self )
{
    KConfigNodeRelease ( self -> node );
    free ( self );
    return 0;
}

/* Sort
 */
static
int64_t CC KRepositorySort ( const void **a, const void **b, void *ignore )
{
    const KRepository *left = * a;
    const KRepository *right = * b;

    if ( left -> category != right -> category )
        return (int64_t)left -> category - (int64_t)right -> category;

    if ( left -> subcategory != right -> subcategory )
        return (int64_t)left -> subcategory - (int64_t)right -> subcategory;

    return StringCompare ( & left -> name, & right -> name );
}

/* Make
 */
static
rc_t KRepositoryMake ( KRepository **rp, const KConfigNode *node,
    const char *name, KRepCategory category, KRepSubCategory subcategory )
{
    rc_t rc;
    KRepository *r;
    String name_str;

    /* measure string */
    StringInitCString ( & name_str, name );

    /* create object */
    r = malloc ( sizeof * r + name_str . size + 1 );
    if ( r == NULL )
        return RC ( rcKFG, rcNode, rcConstructing, rcMemory, rcExhausted );

    rc = KConfigNodeAddRef ( node );
    if ( rc != 0 )
    {
        free ( r );
        return rc;
    }

    r -> node = node;
    r -> name = name_str;
    r -> name . addr = ( char* ) ( r + 1 );
    KRefcountInit ( & r -> refcount, 1, "KRepository", "make", name );
    r -> category = category;
    r -> subcategory = subcategory;
    memmove ( r + 1, name, name_str . size );
    ( ( char* ) ( r + 1 ) ) [ name_str . size ] = 0;

    * rp = r;

    return 0;
}


/* AddRef
 * Release
 */
LIB_EXPORT rc_t CC KRepositoryAddRef ( const KRepository *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAdd ( & self -> refcount, "KRepository" ) )
        {
        case krefLimit:
            return RC ( rcKFG, rcNode, rcAttaching, rcRange, rcExcessive );
        }
    }
    return 0;
}

LIB_EXPORT rc_t CC KRepositoryRelease ( const KRepository *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDrop ( & self -> refcount, "KRepository" ) )
        {
        case krefWhack:
            return KRepositoryWhack ( ( KRepository* ) self );
        case krefNegative:
            return RC ( rcKFG, rcNode, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
}


/* WhackEntry
 *  for cleaning up these vectors
 */
static
void CC KRepositoryWhackEntry ( void *item, void *ignore )
{
    KRepository *self = item;
    KRepositoryRelease ( self );
}


/* Category
 * SubCategory
 *  tells what the repository category or sub-category are
 *  or returns "bad" if the repository object is not usable.
 */
LIB_EXPORT KRepCategory CC KRepositoryCategory ( const KRepository *self )
{
    if ( self != NULL )
        return self -> category;
    return krepBadCategory;
}

LIB_EXPORT KRepSubCategory CC KRepositorySubCategory ( const KRepository *self )
{
    if ( self != NULL )
        return self -> subcategory;
    return krepBadSubCategory;
}


/* Name
 *  get the repository name
 *  attempts to copy NUL-terminated name into provided buffer
 *
 *  "buffer" [ OUT ] and "bsize" [ IN ] - name output parameter
 *
 *  "name_size" [ OUT, NULL OKAY ] - returns the name size in
 *  bytes, excluding any NUL termination.
 */
LIB_EXPORT rc_t CC KRepositoryName ( const KRepository *self,
    char *buffer, size_t bsize, size_t *name_size )
{
    if ( self == NULL )
        return RC ( rcKFG, rcNode, rcAccessing, rcSelf, rcNull );

    if ( name_size != NULL )
        * name_size = self -> name . size;

    if ( bsize < self -> name . size )
        return RC ( rcKFG, rcNode, rcAccessing, rcBuffer, rcInsufficient );

    if ( buffer == NULL )
        return RC ( rcKFG, rcNode, rcAccessing, rcBuffer, rcNull );

    memmove ( buffer, self -> name . addr, self -> name . size );

    if ( bsize > self -> name . size )
        buffer [ self -> name . size ] = 0;

    return 0;
}


/* DisplayName
 *  get the repository display name,
 *  if different from its actual name
 *
 *  attempts to copy NUL-terminated name into provided buffer
 *
 *  "buffer" [ OUT ] and "bsize" [ IN ] - name output parameter
 *
 *  "name_size" [ OUT, NULL OKAY ] - returns the name size in
 *  bytes, excluding any NUL termination.
 */
LIB_EXPORT rc_t CC KRepositoryDisplayName ( const KRepository *self,
    char *buffer, size_t bsize, size_t *name_size )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcKFG, rcNode, rcAccessing, rcSelf, rcNull );
    else
    {
        const KConfigNode *node;

        if ( name_size != NULL )
            * name_size = 0;

        rc = KConfigNodeOpenNodeRead ( self -> node, & node, "display-name" );
        if ( rc != 0 )
            rc = KRepositoryName ( self, buffer, bsize, name_size );
        else
        {
            size_t num_read, remaining;
            rc = KConfigNodeRead ( node, 0, buffer, bsize, & num_read, & remaining );
            KConfigNodeRelease ( node );

            if ( rc == 0 )
            {
                if ( name_size != NULL )
                    * name_size = num_read + remaining;

                if ( remaining != 0 )
                    rc = RC ( rcKFG, rcNode, rcAccessing, rcBuffer, rcInsufficient );
                else if ( num_read < bsize )
                    buffer [ num_read ] = 0;
            }
        }
    }

    return rc;
}


/* Root
 *  read the root path as a POSIX path or URL
 *
 *  attempts to copy NUL-terminated path into provided buffer
 *
 *  "buffer" [ OUT ] and "bsize" [ IN ] - path output parameter
 *
 *  "root_size" [ OUT, NULL OKAY ] - returns the path size in
 *  bytes, excluding any NUL termination.
 */
LIB_EXPORT rc_t CC KRepositoryRoot ( const KRepository *self,
    char *buffer, size_t bsize, size_t *root_size )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcKFG, rcNode, rcAccessing, rcSelf, rcNull );
    else
    {
        const KConfigNode *node;

        if ( root_size != NULL )
            * root_size = 0;

        rc = KConfigNodeOpenNodeRead ( self -> node, & node, "root" );
        if ( rc == 0 )
        {
            size_t num_read, remaining;
            rc = KConfigNodeRead ( node, 0, buffer, bsize, & num_read, & remaining );
            KConfigNodeRelease ( node );

            if ( rc == 0 )
            {
                if ( root_size != NULL )
                    * root_size = num_read + remaining;

                if ( remaining != 0 )
                    rc = RC ( rcKFG, rcNode, rcAccessing, rcBuffer, rcInsufficient );
                else if ( num_read < bsize )
                    buffer [ num_read ] = 0;
            }
        }
    }

    return rc;
}

LIB_EXPORT rc_t CC KRepositorySetRoot(KRepository *self,
    const char *root, size_t root_size)
{
    rc_t rc = 0;

    if (self == NULL) {
        return RC(rcKFG, rcNode, rcUpdating, rcSelf, rcNull);
    }
    else if (root == NULL) {
        return RC(rcKFG, rcNode, rcUpdating, rcParam, rcNull);
    }
    else {
        KConfigNode *self_node = (KConfigNode*)self->node;
        KConfigNode *node = NULL;

        rc = KConfigNodeOpenNodeUpdate(self_node, &node, "root");
        if (rc == 0) {
            rc = KConfigNodeWrite(node, root, root_size);
            KConfigNodeRelease(node);
        }
    }

    return rc;
}


const char root_history_key[] = "root/history";

/* RootHistory
 *  read the root-history as a semicolon separated list of POSIX paths
 *
 *  attempts to copy NUL-terminated path into provided buffer
 *
 *  "buffer" [ OUT ] and "bsize" [ IN ] - path output parameter
 *
 *  "roothistory_size" [ OUT, NULL OKAY ] - returns the root-history
 *  size in bytes, excluding any NUL termination.
 */
LIB_EXPORT rc_t CC KRepositoryRootHistory ( const KRepository *self,
    char *buffer, size_t bsize, size_t *roothistory_size )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcKFG, rcNode, rcAccessing, rcSelf, rcNull );
    else
    {
        const KConfigNode *node;

        if ( roothistory_size != NULL )
            * roothistory_size = 0;

        rc = KConfigNodeOpenNodeRead ( self -> node, & node, root_history_key );
        if ( rc == 0 )
        {
            size_t num_read, remaining;
            rc = KConfigNodeRead ( node, 0, buffer, bsize, & num_read, & remaining );
            KConfigNodeRelease ( node );

            if ( rc == 0 )
            {
                if ( roothistory_size != NULL )
                    * roothistory_size = num_read + remaining;

                if ( remaining != 0 )
                    rc = RC ( rcKFG, rcNode, rcAccessing, rcBuffer, rcInsufficient );
                else if ( num_read < bsize )
                    buffer [ num_read ] = 0;
            }
        }
    }

    return rc;

}


/* SetRootHistory
 *  set the root-history list of paths
 *
 *  "roothistory" [ IN ] and "roothistory_size" [ IN ] - path input parameter
 */
LIB_EXPORT rc_t CC KRepositorySetRootHistory( KRepository *self,
    const char *roothistory, size_t roothistory_size )
{
    rc_t rc = 0;
    if ( self == NULL )
        return RC(rcKFG, rcNode, rcUpdating, rcSelf, rcNull);
    else if ( roothistory == NULL )
        return RC(rcKFG, rcNode, rcUpdating, rcParam, rcNull);
    else
    {
        KConfigNode *self_node = ( KConfigNode* )self->node;
        KConfigNode *node = NULL;

        rc = KConfigNodeOpenNodeUpdate( self_node, &node, root_history_key );
        if ( rc == 0 )
        {
            rc = KConfigNodeWrite( node, roothistory, roothistory_size );
            KConfigNodeRelease( node );
        }
    }
    return rc;
}


static rc_t append_to_root_history( KRepository *self, const char *item )
{
    size_t required;
    rc_t rc = KRepositoryRootHistory( self, NULL, 0, &required );
    if ( GetRCState( rc ) == rcNotFound && GetRCObject( rc ) == ( enum RCObject ) rcPath )
    {
        /* we do not have a root-history yet */
        rc = KRepositorySetRootHistory( self, item, string_size( item ) );
    }
    else if ( GetRCState( rc ) == rcInsufficient && GetRCObject( rc ) == ( enum RCObject ) rcBuffer )
    {
        /* we expect the buffer to be insufficient, because we tested for size by giving a NULL as buffer */
        char * temp = malloc( required + 1 );
        if ( temp == NULL )
            rc = RC( rcKFG, rcNode, rcUpdating, rcMemory, rcExhausted );
        else
        {
            rc = KRepositoryRootHistory( self, temp, required, NULL );
            if ( rc == 0 )
            {
                VNamelist * list;
                temp[ required ] = 0;
                rc = VNamelistFromStr ( &list, temp, ':' );
                if ( rc == 0 )
                {
                    int32_t idx;
                    rc = VNamelistContainsStr( list, item, &idx );
                    if ( rc == 0 && idx < 0 )
                    {
                        rc = VNamelistAppend ( list, item );
                        if ( rc == 0 )
                        {
                            const String * new_value;
                            rc = VNamelistJoin( list, ':', &new_value );
                            if ( rc == 0 )
                            {
                                rc = KRepositorySetRootHistory( self, new_value -> addr, new_value -> size );
                                StringWhack( new_value );
                            }
                        }
                    }
                    VNamelistRelease ( list );
                }
            }
            free( temp );
        }
    }
    return rc;
}


/* AppendToRootHistory
 *  append to the root-history
 *
 *  "roothistory" [ IN ] and "roothistory_size" [ IN ] - path input parameter
 *  if item == NULL, add the current root to the root-history 
 */
LIB_EXPORT rc_t CC KRepositoryAppendToRootHistory( KRepository *self, const char *item )
{
    rc_t rc = 0;
    if ( self == NULL )
        return RC( rcKFG, rcNode, rcUpdating, rcSelf, rcNull );
    else if ( item == NULL )
    {
        size_t curr_root_size;
        rc = KRepositoryRoot( self, NULL, 0, &curr_root_size );
        if ( GetRCState( rc ) == rcInsufficient && GetRCObject( rc ) == ( enum RCObject ) rcBuffer )
        {
            char * root = malloc( curr_root_size + 1 );
            if ( root == NULL )
                rc = RC( rcKFG, rcNode, rcUpdating, rcMemory, rcExhausted );
            else
            {
                rc = KRepositoryRoot( self, root, curr_root_size, NULL );
                if ( rc == 0 )
                {
                    root[ curr_root_size ] = 0;
                    rc = append_to_root_history( self, root );
                }
                else
                {
                    /* we cannot determine the current root, because it is not stored */
                    rc = 0;
                }
                free( root );
            }
        }
        else
        {
            /* we cannot determine the current root, because it is not stored */
            rc = 0;
        }
    }
    else
        rc = append_to_root_history( self, item );
    return rc;
}

    
/* Resolver
 *  read the url of the CGI-resolver
 *
 *  attempts to copy NUL-terminated path into provided buffer
 *
 *  "buffer" [ OUT ] and "bsize" [ IN ] - path output parameter
 *
 *  "written" [ OUT, NULL OKAY ] - returns the url size in
 *  bytes, excluding any NUL termination.
 */
LIB_EXPORT rc_t CC KRepositoryResolver ( const KRepository *self,
    char *buffer, size_t bsize, size_t *written )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcKFG, rcNode, rcAccessing, rcSelf, rcNull );
    else
    {
        const KConfigNode *node;

        if ( written != NULL )
            * written = 0;

        rc = KConfigNodeOpenNodeRead ( self -> node, & node, "resolver-cgi" );
        if ( rc == 0 )
        {
            size_t num_read, remaining;
            rc = KConfigNodeRead ( node, 0, buffer, bsize, & num_read, & remaining );
            KConfigNodeRelease ( node );

            if ( rc == 0 )
            {
                if ( written != NULL )
                    * written = num_read + remaining;

                if ( remaining != 0 )
                    rc = RC ( rcKFG, rcNode, rcAccessing, rcBuffer, rcInsufficient );
                else if ( num_read < bsize )
                    buffer [ num_read ] = 0;
            }
        }
    }

    return rc;
}


/* Disabled
 *  discover whether the repository is enabled
 */
LIB_EXPORT bool CC KRepositoryDisabled ( const KRepository *self )
{
    if ( self != NULL )
    {
        const KConfigNode *node;
        rc_t rc = KConfigNodeOpenNodeRead ( self -> node, & node, "disabled" );
        if ( rc == 0 )
        {
            bool disabled = false;
            rc = KConfigNodeReadBool ( node, & disabled );
            KConfigNodeRelease ( node );
            if ( rc == 0 )
                return disabled;
        }
    }

    return false;
}


static const char * STR_TRUE  = "true";
static const char * STR_FALSE = "false";

LIB_EXPORT rc_t CC KRepositorySetDisabled ( const KRepository *self, bool disabled )
{
    rc_t rc = RC ( rcKFG, rcNode, rcAccessing, rcSelf, rcNull );
    if ( self != NULL )
    {
        KConfigNode * self_node = ( KConfigNode * )self->node;  /* casting away const-ness */
        KConfigNode * node;
        rc = KConfigNodeOpenNodeUpdate ( self_node, &node, "disabled" );
        if ( rc == 0 )
        {
            const char * value = disabled ? STR_TRUE : STR_FALSE;
            rc = KConfigNodeWrite ( node, value, string_size( value ) );
            if ( rc == 0 )
            {
                KConfig * cfg;
                rc = KConfigNodeGetMgr( self->node, &cfg );
                if ( rc == 0 )
                {
                    rc = KConfigCommit ( cfg );
                    KConfigRelease ( cfg );
                }
            }
            KConfigNodeRelease ( node );
        }
    }
    return rc;
}


/* CacheEnabled
 *  discover whether the repository supports caching
 */
LIB_EXPORT bool CC KRepositoryCacheEnabled ( const KRepository *self )
{
    if ( self != NULL )
    {
        const KConfigNode *node;
        rc_t rc = KConfigNodeOpenNodeRead ( self -> node, & node, "cache-enabled" );
        if ( rc == 0 )
        {
            bool enabled = false;
            rc = KConfigNodeReadBool ( node, & enabled );
            KConfigNodeRelease ( node );
            if ( rc == 0 )
                return enabled;
        }
    }

    return false;
}


/* DownloadTicket
 *  return any associated download ticket
 *
 *  attempts to copy NUL-terminated ticket into provided buffer
 *
 *  "buffer" [ OUT ] and "bsize" [ IN ] - ticket output parameter
 *
 *  "ticket_size" [ OUT, NULL OKAY ] - returns the ticket size in
 *  bytes, excluding any NUL termination.
 */
LIB_EXPORT rc_t CC KRepositoryDownloadTicket ( const KRepository *self,
    char *buffer, size_t bsize, size_t *ticket_size )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcKFG, rcNode, rcAccessing, rcSelf, rcNull );
    else
    {
        const KConfigNode *node;

        if ( ticket_size != NULL )
            * ticket_size = 0;

        rc = KConfigNodeOpenNodeRead ( self -> node, & node, "download-ticket" );
        if ( rc == 0 )
        {
            size_t num_read, remaining;
            rc = KConfigNodeRead ( node, 0, buffer, bsize, & num_read, & remaining );
            KConfigNodeRelease ( node );

            if ( rc == 0 )
            {
                if ( ticket_size != NULL )
                    * ticket_size = num_read + remaining;

                if ( remaining != 0 )
                    rc = RC ( rcKFG, rcNode, rcAccessing, rcBuffer, rcInsufficient );
                else if ( num_read < bsize )
                    buffer [ num_read ] = 0;
            }
        }
    }

    return rc;
}


/* EncryptionKey
 *  return any associated encryption key
 *
 *  attempts to copy NUL-terminated key into provided buffer
 *
 *  "buffer" [ OUT ] and "bsize" [ IN ] - encryption key output parameter
 *
 *  "key_size" [ OUT, NULL OKAY ] - returns the key size in
 *  bytes, excluding any NUL termination.
 */
LIB_EXPORT rc_t CC KRepositoryEncryptionKey ( const KRepository *self,
    char *buffer, size_t bsize, size_t *key_size )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcKFG, rcNode, rcAccessing, rcSelf, rcNull );
    else
    {
        const KConfigNode *node;

        if ( key_size != NULL )
            * key_size = 0;

        rc = KConfigNodeOpenNodeRead ( self -> node, & node, "encryption-key" );
        if ( rc == 0 )
        {
            size_t num_read, remaining;
            rc = KConfigNodeRead ( node, 0, buffer, bsize, & num_read, & remaining );
            KConfigNodeRelease ( node );

            if ( rc == 0 )
            {
                if ( key_size != NULL )
                    * key_size = num_read + remaining;

                if ( remaining != 0 )
                    rc = RC ( rcKFG, rcNode, rcAccessing, rcBuffer, rcInsufficient );
                else if ( num_read < bsize )
                    memset ( & buffer [ num_read ], 0, bsize - num_read );
            }
        }
        else if ( GetRCState ( rc ) == rcNotFound )
        {
            char path [ 4096 ];
            rc_t rc2 = KRepositoryEncryptionKeyFile ( self, path, sizeof path, NULL );
            if ( rc2 == 0 )
            {
                KDirectory *wd;
                rc2 = KDirectoryNativeDir ( & wd );
                if ( rc2 == 0 )
                {
                    const KFile *keyFile;
                    rc2 = KDirectoryOpenFileRead ( wd, & keyFile, "%s", path );
                    KDirectoryRelease ( wd );
                    if ( rc2 == 0 )
                    {
                        size_t num_read;
                        rc = KFileReadAll ( keyFile, 0, buffer, bsize, & num_read );
                        if ( rc == 0 )
                        {
                            if ( num_read == bsize )
                            {
                                uint64_t eof;
                                rc = KFileSize ( keyFile, & eof );
                                if ( rc == 0 )
                                    num_read = ( size_t ) eof;
                                else
                                    num_read = 0;

                                rc = RC ( rcKFG, rcFile, rcReading, rcBuffer, rcInsufficient );
                                memset ( buffer, 0, bsize );
                            }
                            else if ( num_read == 0 )
                            {
                                rc = RC ( rcKFG, rcFile, rcReading, rcFile, rcEmpty );
                                memset ( buffer, 0, bsize );
                            }
                            else
                            {
                                char *eoln = string_chr ( buffer, num_read, '\n' );
                                if ( eoln != NULL )
                                {
                                    if ( eoln == buffer )
                                        num_read = 0;
                                    else if ( eoln [ -1 ] == '\r' )
                                        num_read = eoln - buffer - 1;
                                    else
                                        num_read = eoln - buffer;
                                }

                                if ( key_size != NULL )
                                    * key_size = num_read;

                                memset ( & buffer [ num_read ], 0, bsize - num_read );
                            }
                        }

                        KFileRelease ( keyFile );
                    }
                }
            }
        }
    }

    return rc;
}


/* EncryptionKeyFile
 *  return path to any associated encryption key file
 *
 *  attempts to copy NUL-terminated path into provided buffer
 *
 *  "buffer" [ OUT ] and "bsize" [ IN ] - key file path output parameter
 *
 *  "path_size" [ OUT, NULL OKAY ] - returns the path size in
 *  bytes, excluding any NUL termination.
 */
LIB_EXPORT rc_t CC KRepositoryEncryptionKeyFile ( const KRepository *self,
    char *buffer, size_t bsize, size_t *path_size )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcKFG, rcNode, rcAccessing, rcSelf, rcNull );
    else
    {
        const KConfigNode *node;

        if ( path_size != NULL )
            * path_size = 0;

        rc = KConfigNodeOpenNodeRead ( self -> node, & node, "encryption-key-path" );
        if ( rc == 0 )
        {
            size_t num_read, remaining;
            rc = KConfigNodeRead ( node, 0, buffer, bsize, & num_read, & remaining );
            KConfigNodeRelease ( node );

            if ( rc == 0 )
            {
                if ( path_size != NULL )
                    * path_size = num_read + remaining;

                if ( remaining != 0 )
                    rc = RC ( rcKFG, rcNode, rcAccessing, rcBuffer, rcInsufficient );
                else if ( num_read < bsize )
                    buffer [ num_read ] = 0;
            }
        }
    }

    return rc;
}


/* Description
 *  return any associated descriptive text
 *
 *  attempts to copy NUL-terminated description into provided buffer
 *
 *  "buffer" [ OUT ] and "bsize" [ IN ] - description text output parameter
 *
 *  "desc_size" [ OUT, NULL OKAY ] - returns the text size in
 *  bytes, excluding any NUL termination.
 */
LIB_EXPORT rc_t CC KRepositoryDescription ( const KRepository *self,
    char *buffer, size_t bsize, size_t *desc_size )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcKFG, rcNode, rcAccessing, rcSelf, rcNull );
    else
    {
        const KConfigNode *node;

        if ( desc_size != NULL )
            * desc_size = 0;

        rc = KConfigNodeOpenNodeRead ( self -> node, & node, "description" );
        if ( rc == 0 )
        {
            size_t num_read, remaining;
            rc = KConfigNodeRead ( node, 0, buffer, bsize, & num_read, & remaining );
            KConfigNodeRelease ( node );

            if ( rc == 0 )
            {
                if ( desc_size != NULL )
                    * desc_size = num_read + remaining;

                if ( remaining != 0 )
                    rc = RC ( rcKFG, rcNode, rcAccessing, rcBuffer, rcInsufficient );
                else if ( num_read < bsize )
                    buffer [ num_read ] = 0;
            }
        }
    }

    return rc;
}


/* ProjectId
 *  return project id for protected user repository
 *  return RC when repository is not user protected
 *
 *  "projectId" [ OUT ] - returns the project id
 */
LIB_EXPORT rc_t CC KRepositoryProjectId
    ( const KRepository * self, uint32_t * projectId )
{
    rc_t rc = 0;

    if ( projectId == NULL )
        rc = RC ( rcKFG, rcMgr, rcAccessing, rcParam, rcNull );
    else if ( self == NULL )
        rc = SILENT_RC ( rcKFG, rcMgr, rcAccessing, rcSelf, rcNull );
    else if ( self -> category != krepUserCategory
           || self -> subcategory != krepProtectedSubCategory )
        rc = RC ( rcKFG, rcMgr, rcAccessing, rcSelf, rcWrongType );
    else {
        uint32_t id = 0;
        const char prefix [] = "dbGaP-";
        char localName [512] = "";
        size_t localNumWrit = 0;

        * projectId = 0;

        KRepositoryName (self, localName, sizeof ( localName ), & localNumWrit);
        assert ( localNumWrit < sizeof localName );

        if ( strcase_cmp ( localName, localNumWrit,
            prefix, sizeof prefix - 1, sizeof prefix - 1) == 0)
        {
            int i = sizeof prefix - 1;
            for ( i = sizeof prefix - 1; i < localNumWrit; ++ i ) {
                if ( ! isdigit ( localName [ i ] ) ) {
                    rc = RC (rcKFG, rcMgr, rcAccessing, rcSelf, rcUnrecognized);
                    break;
                }
                id = id * 10 + localName [ i ] - '0';
            }

            if ( rc == 0 ) {
                * projectId = id;
                return 0;
            }
        }

        rc = RC (rcKFG, rcMgr, rcAccessing, rcSelf, rcUnrecognized);
    }
        
    return rc;
}


/*--------------------------------------------------------------------------
 * KRepositoryVector
 *  uses Vector API
 *  holds zero or more KRepository objects
 */


/* Whack
 *  destroy your vector
 */
LIB_EXPORT rc_t CC KRepositoryVectorWhack ( KRepositoryVector *self )
{
    if ( self == NULL )
        return RC ( rcKFG, rcVector, rcDestroying, rcSelf, rcNull );

    VectorWhack ( self, KRepositoryWhackEntry, NULL );
    return 0;
}


/*--------------------------------------------------------------------------
 * KRepositoryMgr
 *  manages structured access to repositories
 */
struct KRepositoryMgr
{
    const KConfig *ro_cfg;
    KConfig *rw_cfg;
    KRefcount refcount;
};


/* Whack
 */
static
rc_t KRepositoryMgrWhack ( KRepositoryMgr *self )
{
    if ( self -> ro_cfg )
        KConfigRelease ( self -> ro_cfg );
    if ( self -> rw_cfg )
        KConfigRelease ( self -> rw_cfg );
    free ( self );
    return 0;
}


/* Make
 *  create a repository manager
 *  uses values from "self"
 */
LIB_EXPORT rc_t CC KConfigMakeRepositoryMgrRead ( const KConfig *self, const KRepositoryMgr **mgrp )
{
    rc_t rc;

    if ( mgrp == NULL )
        rc = RC ( rcKFG, rcMgr, rcConstructing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcKFG, rcMgr, rcConstructing, rcSelf, rcNull );
        else
        {
            KRepositoryMgr *mgr = calloc ( 1, sizeof * mgr );
            if ( mgr == NULL )
                rc = RC ( rcKFG, rcMgr, rcConstructing, rcMemory, rcExhausted );
            else
            {
                rc = KConfigAddRef ( self );
                if ( rc == 0 )
                {
                    mgr -> ro_cfg = self;
                    KRefcountInit ( & mgr -> refcount, 1, "KRepositoryMgr", "make-read", "mgr" );
                    * mgrp = mgr;
                    return 0;
                }

                free ( mgr );
            }
        }

        * mgrp = NULL;
    }

    return rc;
}

LIB_EXPORT rc_t CC KConfigMakeRepositoryMgrUpdate ( KConfig *self, KRepositoryMgr **mgrp )
{
    rc_t rc;

    if ( mgrp == NULL )
        rc = RC ( rcKFG, rcMgr, rcConstructing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcKFG, rcMgr, rcConstructing, rcSelf, rcNull );
        else
        {
            KRepositoryMgr *mgr = calloc ( 1, sizeof * mgr );
            if ( mgr == NULL )
                rc = RC ( rcKFG, rcMgr, rcConstructing, rcMemory, rcExhausted );
            else
            {
                rc = KConfigAddRef ( self );
                if ( rc == 0 )
                {
                    mgr -> rw_cfg = self;
                    KRefcountInit ( & mgr -> refcount, 1, "KRepositoryMgr", "make-update", "mgr" );
                    * mgrp = mgr;
                    return 0;
                }

                free ( mgr );
            }
        }

        * mgrp = NULL;
    }

    return rc;
}


/* AddRef
 * Release
 */
LIB_EXPORT rc_t CC KRepositoryMgrAddRef ( const KRepositoryMgr *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAdd ( & self -> refcount, "KRepositoryMgr" ) )
        {
        case krefLimit:
            return RC ( rcKFG, rcMgr, rcAttaching, rcRange, rcExcessive );
        }
    }
    return 0;
}

LIB_EXPORT rc_t CC KRepositoryMgrRelease ( const KRepositoryMgr *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDrop ( & self -> refcount, "KRepositoryMgr" ) )
        {
        case krefWhack:
            return KRepositoryMgrWhack ( ( KRepositoryMgr* ) self );
        case krefNegative:
            return RC ( rcKFG, rcMgr, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
}


static
const KConfig *KRepositoryMgrGetROKConfig ( const KRepositoryMgr *self )
{
    return self -> ro_cfg ? self -> ro_cfg : self -> rw_cfg;
}

static
rc_t KRepositoryMgrSubCategoryRepositories ( const KConfigNode *sub,
    KRepCategory category, KRepSubCategory subcategory, KRepositoryVector *repositories )
{
    KNamelist *repo_names;
    rc_t rc = KConfigNodeListChildren ( sub, & repo_names );
    if ( rc == 0 )
    {
        uint32_t i, count;
        rc = KNamelistCount ( repo_names, & count );
        for ( i = 0; i < count && rc == 0; ++ i )
        {
            const char *repo_name;
            rc = KNamelistGet ( repo_names, i, & repo_name );
            if ( rc == 0 )
            {
                const KConfigNode *node;
                rc = KConfigNodeOpenNodeRead ( sub, & node, "%s", repo_name );
                if ( rc == 0 )
                {
                    KRepository *repo;
                    rc = KRepositoryMake ( & repo, node, repo_name, category, subcategory );
                    if ( rc == 0 )
                    {
                        rc = VectorAppend ( repositories, NULL, repo );
                        if ( rc != 0 )
                            KRepositoryWhack ( repo );
                    }

                    KConfigNodeRelease ( node );
                }
            }
        }

        KNamelistRelease ( repo_names );
    }

    return rc;
}

static
rc_t KRepositoryMgrCategoryRepositories ( const KConfigNode *cat,
    KRepCategory category, KRepositoryVector *repositories )
{
    KNamelist *sub_names;
    rc_t rc = KConfigNodeListChildren ( cat, & sub_names );
    if ( rc == 0 )
    {
        uint32_t i, count;
        rc = KNamelistCount ( sub_names, & count );
        for ( i = 0; i < count && rc == 0; ++ i )
        {
            const char *sub_name;
            rc = KNamelistGet ( sub_names, i, & sub_name );
            if ( rc == 0 )
            {
                KRepSubCategory subcategory = krepBadSubCategory;
                if ( strcmp ( "main", sub_name ) == 0 )
                    subcategory = krepMainSubCategory;
                else if ( strcmp ( "aux", sub_name ) == 0 )
                    subcategory = krepAuxSubCategory;
                else if ( strcmp ( "protected", sub_name ) == 0 )
                    subcategory = krepProtectedSubCategory;

                if ( subcategory != krepBadSubCategory )
                {
                    const KConfigNode *sub;
                    rc = KConfigNodeOpenNodeRead ( cat, & sub, "%s", sub_name );
                    if ( rc == 0 )
                    {
                        rc = KRepositoryMgrSubCategoryRepositories ( sub, category, subcategory, repositories );
                        KConfigNodeRelease ( sub );
                    }
                }
            }
        }

        KNamelistRelease ( sub_names );
    }

    return rc;
}


/* UserRepositories
 *  retrieve all user repositories in a Vector
 */
LIB_EXPORT rc_t CC KRepositoryMgrGetRepositories ( const KRepositoryMgr * self, KRepCategory category,
    KRepositoryVector * repositories )
{
    rc_t rc;

    if ( repositories == NULL )
        rc = RC ( rcKFG, rcMgr, rcAccessing, rcParam, rcNull );
    else
    {
        VectorInit ( repositories, 0, 8 );

        if ( self == NULL )
            rc = RC ( rcKFG, rcMgr, rcAccessing, rcSelf, rcNull );
        else
        {
            const KConfig * kfg = KRepositoryMgrGetROKConfig ( self );

            const KConfigNode * node;
            switch( category )
            {
                case krepUserCategory   : rc = KConfigOpenNodeRead ( kfg, & node, "/repository/user" ); break;
                case krepSiteCategory   : rc = KConfigOpenNodeRead ( kfg, & node, "/repository/site" ); break;
                case krepRemoteCategory : rc = KConfigOpenNodeRead ( kfg, & node, "/repository/remote" ); break;
                default : rc = RC ( rcKFG, rcMgr, rcAccessing, rcParam, rcInvalid );
            }
            if ( rc == 0 )
            {
                rc = KRepositoryMgrCategoryRepositories ( node, category, repositories );
                KConfigNodeRelease ( node );
                if ( rc == 0 )
                    VectorReorder ( repositories, KRepositorySort, NULL );
            }

            if ( rc != 0 )
                KRepositoryVectorWhack ( repositories );
        }
    }
    return rc;
}


LIB_EXPORT bool CC KRepositoryMgrCategoryDisabled ( const KRepositoryMgr *self, KRepCategory category )
{
    bool res = false;

    if ( self != NULL )
    {
        const KConfig * kfg = KRepositoryMgrGetROKConfig ( self );
        if ( kfg != NULL )
        {
            switch( category )
            {
                case krepUserCategory   : KConfigReadBool ( kfg, "/repository/user/disabled", &res ); break;
                case krepSiteCategory   : KConfigReadBool ( kfg, "/repository/site/disabled", &res ); break;
                case krepRemoteCategory : KConfigReadBool ( kfg, "/repository/remote/disabled", &res ); break;
            }
        }
    }
    return res;
}


LIB_EXPORT rc_t CC KRepositoryMgrCategorySetDisabled ( const KRepositoryMgr *self, KRepCategory category, bool disabled )
{
    rc_t rc = 0;
    if ( self == NULL )
        rc = RC ( rcKFG, rcMgr, rcAccessing, rcSelf, rcNull );
    else
    {
        KConfig * kfg = ( KConfig * ) KRepositoryMgrGetROKConfig ( self );
        if ( kfg == NULL )
            rc = RC ( rcKFG, rcMgr, rcAccessing, rcParam, rcNull );
        else
        {
            switch( category )
            {
                case krepUserCategory   : rc = KConfigWriteBool ( kfg, "/repository/user/disabled", disabled ); break;
                case krepSiteCategory   : rc = KConfigWriteBool ( kfg, "/repository/site/disabled", disabled ); break;
                case krepRemoteCategory : rc = KConfigWriteBool ( kfg, "/repository/remote/disabled", disabled ); break;
                default : rc = RC ( rcKFG, rcMgr, rcAccessing, rcParam, rcInvalid );
            }
        }
    }
    return rc;
}


LIB_EXPORT rc_t CC KRepositoryMgrUserRepositories ( const KRepositoryMgr * self,
    KRepositoryVector * user_repositories )
{
    return KRepositoryMgrGetRepositories ( self, krepUserCategory, user_repositories );
}


/* SiteRepositories
 *  retrieve all site repositories in a Vector
 */
LIB_EXPORT rc_t CC KRepositoryMgrSiteRepositories ( const KRepositoryMgr *self,
    KRepositoryVector *site_repositories )
{
    return KRepositoryMgrGetRepositories ( self, krepSiteCategory, site_repositories );
}


/* RemoteRepositories
 *  retrieve all remote repositories in a Vector
 */
LIB_EXPORT rc_t CC KRepositoryMgrRemoteRepositories ( const KRepositoryMgr *self,
    KRepositoryVector *remote_repositories )
{
    return KRepositoryMgrGetRepositories ( self, krepRemoteCategory, remote_repositories );
}


#define RELEASE(type, obj) do { rc_t rc2 = type##Release(obj); \
    if (rc2 && !rc) { rc = rc2; } obj = NULL; } while (false)

static rc_t KRepositoryCurrentProtectedRepositoryForNgc(
    const KRepository ** self)
{
    rc_t rc = 0;

    const char * ngc_file = KConfigGetNgcFile();

    if (ngc_file == NULL)
        return SILENT_RC(rcKFG, rcMgr, rcAccessing,
            rcNode, rcNotFound);
    else {
        KDirectory * dir = NULL;
        const KFile * f = NULL;
        const KNgcObj * ngc = NULL;
        KConfig * kfg = NULL;
        const KRepositoryMgr * mgr = NULL;
        KRepositoryVector vc;
        uint32_t id = 0;
        char name[512] = "";
        size_t nameLen = 0;
        char n[512] = "";
        char v[512] = "";

        rc = KDirectoryNativeDir(&dir);
        if (rc == 0)
            rc = KDirectoryOpenFileRead(dir, &f, "%s", ngc_file);
        if (rc == 0)
            rc = KNgcObjMakeFromFile(&ngc, f);
        if (rc == 0)
            rc = KNgcObjGetProjectId(ngc, &id);
        if (rc == 0)
            rc = KConfigMakeLocal(&kfg, NULL);

        if (rc == 0)
            rc = string_printf(name, sizeof name, &nameLen, "dbGaP-%d", id);

        if (rc == 0)
            rc = KNgcObjGetEncryptionKey(ngc, v, sizeof v, NULL);
        if (rc == 0)
            rc = string_printf(n, sizeof n, NULL,
                "/repository/user/protected/%s/encryption-key", name);
        if (rc == 0)
            rc = KConfigWriteString(kfg, n, v);

        if (rc == 0)
            rc = KNgcObjGetTicket(ngc, v, sizeof v, NULL);
        if (rc == 0)
            rc = string_printf(n, sizeof n, NULL,
                "/repository/user/protected/%s/download-ticket", name);
        if (rc == 0)
            rc = KConfigWriteString(kfg, n, v);

        if (rc == 0)
            rc = KConfigMakeRepositoryMgrRead(kfg, &mgr);
        if (rc == 0)
            rc = KRepositoryMgrUserRepositories(mgr, &vc);

        assert(self);
        *self = NULL;

        if (rc == 0) {
            uint32_t i = 0;
            uint32_t count = VectorLength(&vc);
            for (i = 0; i < count; ++i) {
                bool found = false;
                const KRepository * r = (const void*)VectorGet(&vc, i);
                if (r->subcategory == krepProtectedSubCategory) {
                    char lclName[512] = "";
                    size_t lNumWrit = 0;
                    rc = KRepositoryName(r, lclName, sizeof lclName, &lNumWrit);
                    if (rc == 0) {
                        assert(lNumWrit < sizeof lclName);
                        if (strcase_cmp(lclName, lNumWrit,
                            name, nameLen, sizeof name) == 0)
                        {
                            found = true;
                        }
                    }
                }
                if (found) {
                    rc = KRepositoryAddRef(r);
                    if (rc == 0) {
                        *self = r;
                        break;
                    }
                }
            }
            KRepositoryVectorWhack(&vc);
        }

        if (rc == 0 && * self == NULL)
            rc = RC(rcKFG, rcMgr, rcAccessing, rcNode, rcNotFound);

        RELEASE(KRepositoryMgr, mgr);
        RELEASE(KConfig, kfg);
        RELEASE(KNgcObj, ngc);
        RELEASE(KFile, f);
        RELEASE(KDirectory, dir);
    }

    return rc;
}

/* CurrentProtectedRepository
 *  returns the currently active user protected repository
 */
LIB_EXPORT rc_t CC KRepositoryMgrCurrentProtectedRepository ( const KRepositoryMgr *self, const KRepository **protected )
{
    rc_t rc = 0;

    if ( protected == NULL )
        rc = RC ( rcKFG, rcMgr, rcAccessing, rcParam, rcNull );
    else
    {
        * protected = NULL;

        if ( self == NULL )
            rc = RC ( rcKFG, rcMgr, rcAccessing, rcSelf, rcNull );
        else
        {
            KRepositoryVector v;
            rc = KRepositoryMgrUserRepositories ( self, & v );
            if ( rc == 0 )
            {
                KDirectory *wd;
                rc = KDirectoryNativeDir ( & wd );
                if ( rc == 0 )
                {
                    /* we need services of system directory */
                    struct KSysDir *sysDir = KDirectoryGetSysDir ( wd );

                    /* allocate buffer space for 3 paths */
                    const size_t path_size = 4096;
                    char *wd_path = malloc ( path_size * 3 );
                    if ( wd_path == NULL )
                        rc = RC ( rcKFG, rcMgr, rcAccessing, rcMemory, rcExhausted );
                    else
                    {
                        /* the working directory already has a canonical path */
                        rc = KDirectoryResolvePath ( wd, true, wd_path, path_size, "." );
                        if ( rc == 0 )
                        {
                            size_t wd_size = string_size ( wd_path );

                            /* look for all protected user repositories */
                            uint32_t i, count = VectorLength ( & v );
                            for ( i = 0; i < count; ++ i )
                            {
                                const KRepository *r = ( const void* ) VectorGet ( & v, i );
                                if ( r -> subcategory == krepProtectedSubCategory )
                                {
                                    rc_t rc2 = 0;
                                    size_t resolved_size;
                                    char *resolved = wd_path + path_size;

                                    /* get stated root path to repository */
                                    char *root = resolved + path_size;
                                    rc2 = KRepositoryRoot ( r,
                                        root, path_size, NULL );
                                    if ( rc2 != 0 ) {
                                        /* VDB-1096:
                                        We cannot get repository root:
                                        is it a bad repository configuration?
                                        Then ignore this repository node
                                        and try another repository */
                                        continue;
                                    }

                                    /* get its canonical path */
                                    rc2 = KSysDirRealPath ( sysDir,
                                                            resolved, path_size, "%s", root );
                                    if ( rc2 != 0 ) {
                                        /* VDB-1096:
                      Invalid cannot get repository root? Ignore and continue */
                                        continue;
                                    }

                                    /* we know the current directory's canonical path size
                                       and we know the repository's canonical path size.
                                       to be "within" the repository, the current directory's
                                       size must be >= repository path size, and must match
                                       exactly the repository path itself over those bytes. */
                                    resolved_size = string_size ( resolved );
                                    if ( resolved_size <= wd_size && memcmp ( wd_path, resolved, resolved_size ) == 0 )
                                    {
                                        /* still have a little more to check */
                                        if ( resolved_size == wd_size ||
                                             wd_path [ resolved_size ] == '/' )
                                        {
                                            /* we are in the repository */
                                            rc = KRepositoryAddRef ( r );
                                            if ( rc == 0 )
                                                * protected = r;
                                            break;
                                        }
                                    }
                                }
                            }
                        }

                        free ( wd_path );
                    }

                    KDirectoryRelease ( wd );
                }

                KRepositoryVectorWhack ( & v );
            }
            else if (rc ==
                SILENT_RC(rcKFG, rcNode, rcOpening, rcPath, rcNotFound))
            {
                return SILENT_RC ( rcKFG, rcMgr, rcAccessing, rcNode, rcNotFound );
            }

            if (rc == 0 && * protected == NULL)
                rc = KRepositoryCurrentProtectedRepositoryForNgc(protected);
        }
    }

    return rc;
}


/* GetProtectedRepository
 *  retrieves a user protected repository by its associated project-id
 */
LIB_EXPORT rc_t CC KRepositoryMgrGetProtectedRepository ( const KRepositoryMgr *self, 
    uint32_t projectId, 
    const KRepository **protected )
{
    rc_t rc;

    if ( protected == NULL )
        rc = RC ( rcKFG, rcMgr, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcKFG, rcMgr, rcAccessing, rcSelf, rcNull );
        else
        {
            char repNodeName[512] = "";
            size_t numWrit = 0;
            KRepositoryVector v;
            rc = string_printf(repNodeName, sizeof repNodeName, &numWrit, "dbgap-%u", projectId); /* go case-insensitive */
            assert(numWrit < sizeof(repNodeName));
            
            rc = KRepositoryMgrUserRepositories ( self, & v );
            if ( rc == 0 )
            {  /* look for all protected user repositories */
                uint32_t i, count = VectorLength ( & v );
                for ( i = 0; i < count; ++ i )
                {
                    const KRepository *r = ( const void* ) VectorGet ( & v, i );
                    if ( r -> subcategory == krepProtectedSubCategory )
                    {
                        char localName[512] = "";
                        size_t localNumWrit = 0;
                        KRepositoryName(r, localName, sizeof(localName), &localNumWrit);
                        assert(localNumWrit < sizeof(localName));
                        if (strcase_cmp(repNodeName, numWrit, localName, localNumWrit, sizeof(localName)) == 0)
                        {
                            rc = KRepositoryAddRef ( r );
                            if ( rc == 0 )
                            {
                                * protected = r;
                                KRepositoryVectorWhack(&v);
                                return 0;
                            }
                        }
                    }
                }
                KRepositoryVectorWhack(&v);
                rc = RC ( rcKFG, rcMgr, rcAccessing, rcNode, rcNotFound );
            }
        }
    }

    return rc;
}    


static rc_t find_repository_in_vector( KRepositoryVector * v, KRepository **repository, const char * name, uint32_t name_len )
{
    rc_t rc = 0;
    KRepository * found = NULL;
    uint32_t count = VectorLength ( v );
    uint32_t i;
    for ( i = 0; i < count && found == NULL; ++ i )
    {
        char r_name[ 512 ];
        size_t written = 0;
        KRepository * r = ( KRepository * ) VectorGet ( v, i );
        rc = KRepositoryName( r, r_name, sizeof r_name , &written );
        if ( strcase_cmp( name, name_len, r_name, written, sizeof r_name ) == 0 )
        {
            found = r;
        }
    }
    if ( found == NULL )
        rc = RC ( rcKFG, rcMgr, rcSearching, rcName, rcNotFound );
    else
        *repository = found;
    return rc;
}


static rc_t create_child_node( const KConfigNode * self, const char * child_name, const char * child_val, uint32_t len )
{
    KConfigNode * child_node;
    rc_t rc = KConfigNodeOpenNodeUpdate( ( KConfigNode * )self, &child_node, "%s", child_name );
    if ( rc == 0 )
    {
        rc_t rc_2;
        rc = KConfigNodeWrite( child_node, child_val, len );
        rc_2 = KConfigNodeRelease( child_node );
        if ( rc == 0 ) rc = rc_2;
    }
    return rc;
}


static rc_t make_writable_file( struct KFile ** dst, const char * path )
{
    KDirectory * dir;
    rc_t rc = KDirectoryNativeDir( &dir );
    if ( rc == 0 )
    {
        rc = KDirectoryCreateFile ( dir, dst, false, 0600, ( kcmInit | kcmParents ), "%s", path );
        KDirectoryRelease( dir );
    }
    return rc;
}


static rc_t make_key_file( KRepositoryMgr * self, const struct KNgcObj * ngc, char * buffer, size_t buffer_size, size_t * written )
{
    struct String * home;
    rc_t rc = KConfigReadString ( self->rw_cfg, "HOME", &home );
    if ( rc == 0 )
    {
        rc = string_printf( buffer, buffer_size, written, "%S", home );
        if ( rc == 0 )
        {
            size_t written2;
            rc = string_printf( &buffer[ *written ], buffer_size - *written, &written2, "/.ncbi/dbGap-%u.enc_key", ngc->projectId );
            if ( rc == 0 )
                *written += written2;
        }
        StringWhack ( home );
    }
    if ( rc == 0 )
    {
        struct KFile * key_file;
        rc = make_writable_file( &key_file, buffer );
        if ( rc == 0 )
        {
            rc = KNgcObjWriteKeyToFile ( ngc, key_file );
            KFileRelease( key_file );
        }
    }
    return rc;
}

/******************************************************************************/

static rc_t _KRepositoryAppsNodeFix(KConfigNode *self,
    const char *path, const char *val, size_t len, uint32_t *modifications)
{
    rc_t rc = 0;

    KConfigNode *node = NULL;

    assert(self && modifications);

    rc = KConfigNodeOpenNodeUpdate(self, &node, path);
    if (rc == 0) {
        bool update = false;
        char buffer[8] = "";
        size_t num_read = 0;
        size_t remaining = 0;
        rc_t rc = KConfigNodeRead(node, 0,
            buffer, sizeof buffer, &num_read, &remaining);
        if ((rc != 0) || (string_cmp(buffer, num_read, val, len, len) != 0)) {
            update = true;
        }

        if (update) {
            rc = KConfigNodeWrite(node, val, len);
            if (rc == 0) {
                *modifications = INP_UPDATE_APPS;
            }
        }
    }

    RELEASE(KConfigNode, node);

    return rc;
}

static rc_t _KRepositoryFixApps(KRepository *self, uint32_t *modifications) {
    rc_t rf = 0;
    rc_t rs = 0;

    KConfigNode *self_node = NULL;

    assert(self);

    self_node = (KConfigNode*)self->node;

    rf = _KRepositoryAppsNodeFix(self_node, "apps/file/volumes/flat",
        "files", 5, modifications);

    rs = _KRepositoryAppsNodeFix(self_node, "apps/sra/volumes/sraFlat",
        "sra"  , 3, modifications);

    return rf != 0 ? rf : rs;
}

/* we have not found a repository named repo_name, let us create a new one... */
static rc_t create_new_protected_repository( KRepositoryMgr * self,
    const struct KNgcObj * ngc, const char * location, uint32_t location_len, const char * repo_name, uint32_t name_len )
{
    KConfigNode * new_repository;
    rc_t rc = KConfigOpenNodeUpdate( self->rw_cfg, &new_repository, "/repository/user/protected/dbGaP-%u", ngc->projectId );
    if ( rc == 0 )
    {
        rc = create_child_node( new_repository, "download-ticket", ngc->downloadTicket.addr, ngc->downloadTicket.len );

        if ( rc == 0 )
            rc = create_child_node( new_repository, "description", ngc->description.addr, ngc->description.len );

        if ( rc == 0 )
            rc = create_child_node( new_repository, "apps/file/volumes/flat", "files", 5 );

        if ( rc == 0 )
            rc = create_child_node( new_repository, "apps/sra/volumes/sraFlat", "sra", 3 );

        if ( rc == 0 )
            rc = create_child_node( new_repository, "cache-enabled", "true", 4 );

        if ( rc == 0 )
            rc = create_child_node( new_repository, "root", location, location_len );

        if ( rc == 0 )
        {
            size_t written;
            char key_file_path[ 4096 ];
            rc = make_key_file( self, ngc, key_file_path, sizeof key_file_path, &written );
            if ( rc == 0 )
                rc = create_child_node(new_repository,
                    "encryption-key-path", key_file_path, (uint32_t)written);
        }
        KConfigNodeRelease( new_repository );
    }
    return rc;
}


static rc_t check_for_modifications(
    const KRepository *repository, const KNgcObj *ngc,
    uint32_t *modifications, uint32_t *notFoundNodes)
{
    rc_t rc = 0;
    size_t written;
    char buffer[ 1024 ];
    assert(modifications && notFoundNodes);

    if (rc == 0) {
        rc = KRepositoryDownloadTicket(repository,
            buffer, sizeof buffer, &written);
        if (rc == 0) {
            if (strcase_cmp(buffer, written, ngc->downloadTicket.addr,
                ngc->downloadTicket.len, sizeof buffer) != 0)
            {
                *modifications |= INP_UPDATE_DNLD_TICKET;
            }
        }
        else if (rc == SILENT_RC(rcKFG, rcNode, rcOpening, rcPath, rcNotFound)) 
        {
            rc = 0;
            *notFoundNodes |= INP_UPDATE_DNLD_TICKET;
        }
    }

    if (rc == 0) {
        rc = KRepositoryEncryptionKey ( repository, buffer, sizeof buffer, &written );
        if (rc == 0) {
            if (strcase_cmp(buffer, written, ngc->encryptionKey.addr,
                ngc->encryptionKey.len, sizeof buffer) != 0)
            {
                *modifications |= INP_UPDATE_ENC_KEY;
            }
        }
        else if (rc == SILENT_RC(rcKFG, rcNode, rcOpening, rcPath, rcNotFound))
        {
            rc = 0;
            *notFoundNodes |= INP_UPDATE_ENC_KEY;
        }
    }

    if (rc == 0) {
        rc = KRepositoryDescription ( repository, buffer, sizeof buffer, &written );
        if (rc == 0) {
            if (strcase_cmp(buffer, written, ngc->description.addr,
                ngc->description.len, sizeof buffer) != 0)
            {
                *modifications |= INP_UPDATE_DESC;
            }
        }
        else if (rc == SILENT_RC(rcKFG, rcNode,
            rcOpening, rcPath, rcNotFound))
        {
            rc = 0;
            *notFoundNodes |= INP_UPDATE_DESC;
        }
    }

    return rc;
}


static rc_t check_for_root_modification(const KRepository *repository,
    const char *root, uint32_t *modifications)
{
    rc_t rc = 0;
    size_t root_size = 0;
    char buffer[PATH_MAX] = "";
    size_t bsize = 0;

    if (root == NULL || modifications == NULL) {
        return RC(rcKFG, rcMgr, rcAccessing, rcSelf, rcNull);
    }

    rc = KRepositoryRoot(repository, buffer, sizeof buffer, &root_size);
    if (rc != 0) {
        return rc;
    }

    bsize = string_measure(root, NULL);
    if (strcase_cmp(buffer, root_size, root, bsize, sizeof buffer) != 0) {
        *modifications |= INP_UPDATE_ROOT;
    }

    return rc;
}

/* we have an existing repository that corresponds with the given ngc-object, see if we have to / or can update its values */
static rc_t update_existing_protected_repository( KRepositoryMgr * self,
    KRepository * repository, const struct KNgcObj * ngc, uint32_t modifications )
{
    rc_t rc = 0;
    /* make the changes... */
    if ( modifications & INP_UPDATE_DNLD_TICKET )
    {
        rc = create_child_node( repository->node, "download-ticket", ngc->downloadTicket.addr, ngc->downloadTicket.len );
    }

    if ( rc == 0 && ( modifications & INP_UPDATE_ENC_KEY ) )
    {
        size_t written;
        char key_file_path[ 4096 ];
        rc = make_key_file( self, ngc, key_file_path, sizeof key_file_path, &written );
        if ( rc == 0 )
            rc = create_child_node(repository->node,
                "encryption-key-path", key_file_path, (uint32_t)written);
    }

    if ( rc == 0 && ( modifications & INP_UPDATE_DESC ) )
    {
        rc = create_child_node( repository->node, "description", ngc->description.addr, ngc->description.len );
    }
    return rc;
}


LIB_EXPORT rc_t CC KRepositoryMgrImportNgcObj( KRepositoryMgr * self,
    const struct KNgcObj * ngc, const char * location, uint32_t permissions, uint32_t * result_flags )
{
    rc_t rc = 0;
    if ( self == NULL )
        rc = RC ( rcKFG, rcMgr, rcUpdating, rcSelf, rcNull );
    if ( ngc == NULL || location == NULL || result_flags == NULL )
        rc = RC ( rcKFG, rcMgr, rcUpdating, rcParam, rcNull );
    else
    {
        KRepositoryVector user_repositories;
        size_t written;
        char ngc_repo_name[ 512 ];
        *result_flags = 0;
        memset(&user_repositories, 0, sizeof user_repositories);
        rc = string_printf( ngc_repo_name, sizeof ngc_repo_name, &written, "dbGaP-%u", ngc->projectId );
        if ( rc == 0 )
        {
            KRepository *repository = NULL;
            bool exists = false;

            rc = KRepositoryMgrUserRepositories ( self, &user_repositories );
            if ( rc == 0 )
            {
                rc = find_repository_in_vector(&user_repositories,
                    &repository, ngc_repo_name, (uint32_t)written);
                if ( rc == 0 )
                {
                    uint32_t modifications = 0;
                    uint32_t notFoundNodes = 0;

                    exists = true;

                    /* test for any changes:
                       1. download ticket
                       2. enc. key
                       3. description

                       mark each bit that differs
                    */
                    rc = check_for_modifications(repository,
                        ngc, &modifications, &notFoundNodes);
                    if ( rc == 0 )
                    {
                        if (notFoundNodes != 0) {
                            permissions |= notFoundNodes;
                            modifications |= notFoundNodes;
                        }

                        /* reject if any modification was not authorized */
                        if ( ( modifications & ( modifications ^ permissions ) ) != 0 )
                        {
                            /* tell what was wrong, set rc */
                            * result_flags = modifications & ( modifications ^ permissions );
                            rc = RC(rcKFG, rcMgr, rcUpdating,
                                rcConstraint, rcViolated);
                        }
                        else if ( modifications != 0 )
                        {
                            /* apply changes - all are authorized */

                            rc = update_existing_protected_repository( self, repository, ngc, modifications );
                            if ( rc == 0 )
                                *result_flags |= modifications;
                        }
                    }
                }
            }

            if (! exists) {
                if (permissions & INP_CREATE_REPOSITORY) {
                    uint32_t location_len = string_measure (location, NULL);
                    rc = create_new_protected_repository(
                        self, ngc, location, location_len,
                        ngc_repo_name, (uint32_t)written);
                    if (rc == 0) {
                        *result_flags |= INP_CREATE_REPOSITORY;
                    }
                }
                else {
                    *result_flags |= INP_CREATE_REPOSITORY;
                    rc = RC(rcKFG, rcMgr, rcUpdating, rcConstraint, rcViolated);
                }
            }
            else {
                if (rc == 0 && permissions & INP_UPDATE_ROOT) {
                    uint32_t modifications = 0;
                    rc = check_for_root_modification(
                        repository, location, &modifications);
                    if (rc == 0) {
                        if (modifications & INP_UPDATE_ROOT) {
                            uint32_t location_len
                                = string_measure(location, NULL);
                            rc = KRepositorySetRoot(repository,
                                location, location_len);
                            if (rc == 0) {
                                *result_flags |= INP_UPDATE_ROOT;
                            }
                        }
                        else if (modifications != 0) {
                            *result_flags |= INP_UPDATE_ROOT;
                            rc = RC(rcKFG, rcMgr, rcCreating,
                                rcConstraint, rcViolated);
                        }
                    }
                }
                if (rc == 0) {
                    uint32_t modifications = 0;
                    rc = _KRepositoryFixApps(repository, &modifications);
                    if (rc == 0 && modifications != 0) {
                        *result_flags |= INP_UPDATE_APPS;
                    }
                }
            }
        }

        KRepositoryVectorWhack ( &user_repositories );
    }

    return rc;
}


LIB_EXPORT
bool CC KRepositoryMgrHasRemoteAccess(const KRepositoryMgr *self)
{
    bool has = false;

    rc_t rc = 0, r2 = 0;

    uint32_t len = 0;

    KRepositoryVector remote_repositories;
    memset(&remote_repositories, 0, sizeof remote_repositories);

    rc = KRepositoryMgrRemoteRepositories(self, &remote_repositories);

    if (rc == 0) {
        len = VectorLength(&remote_repositories);
    }

    if (rc == 0 && len > 0) {
        uint32_t i = 0;
        if (! KRepositoryMgrCategoryDisabled(self, krepRemoteCategory)) {
            for (i = 0; i < len; ++ i) {
                const KRepository *r = VectorGet(&remote_repositories, i);
                if (r != NULL) {
                    if (KRepositoryDisabled(r)) {
                        continue;
                    }

                    if (KRepositorySubCategory(r)
                        != krepProtectedSubCategory)
                    {
                        has = true;
                    }
                }
            }
        }
    }

    r2 = KRepositoryVectorWhack(&remote_repositories);
    if (r2 != 0 && rc == 0) {
        rc = r2;
    }

    if (rc != 0) {
        return false;
    }

    return has;
}
