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

#include <klib/rc.h>
#include <klib/out.h>
#include <klib/namelist.h>
#include <klib/refcount.h>
#include <klib/printf.h>
#include <klib/container.h>

#include <kfg/config.h>

#include "owp.h"
#include "schwarzschraube.h"

#include <sysalloc.h>

#include <ctype.h>

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*))
 //   it is a struct, don't know why ... prolly will add refcount later
((*/
struct XFSOwp {
    BSTree tree;
};

struct XFSOwpEntry {
    struct BSTNode node;
    char * Key;
    char * Property;
};

LIB_EXPORT
rc_t CC
XFSOwpMake ( struct XFSOwp ** Owp )
{
    rc_t RCt;
    struct XFSOwp * tmpOwp;

    RCt = 0;
    tmpOwp = NULL;

    if ( Owp == NULL ) {
        return XFS_RC ( rcNull );
    }
    * Owp = NULL;

    tmpOwp = calloc ( 1, sizeof ( struct XFSOwp ) );
    if ( tmpOwp == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    RCt = XFSOwpInit ( tmpOwp );
    if ( RCt == 0 ) {
        * Owp = tmpOwp;
    }
    else {
        * Owp = NULL; /* I know it was initialized in that way, but */
        free ( tmpOwp );
    }

    return RCt;
}   /* XFSOwpMake () */

LIB_EXPORT
rc_t CC
XFSOwpInit ( struct XFSOwp * self )
{
    if ( self == NULL ) {
        return XFS_RC ( rcNull );
    }

    BSTreeInit ( ( BSTree * ) self );

    return 0;
}   /* XFSOwpInit () */

static
void CC
_OWPEntryWhack ( BSTNode * Node, void * UsuallyNullParam )
{
    struct XFSOwpEntry * Entry;

    Entry = ( struct XFSOwpEntry * ) Node;

    if ( Entry != NULL ) {
        if ( Entry -> Key != NULL ) {
            free ( Entry -> Key );
            Entry -> Key = NULL;
        }

        if ( Entry -> Property != NULL ) {
            free ( Entry -> Property );
            Entry -> Property = NULL;
        }

        free ( Entry );
        Entry = NULL; /* :lol: */
    }
}   /* _OWPEntryWhack () */

LIB_EXPORT
rc_t CC
XFSOwpWhack ( struct XFSOwp * self )
{
    if ( self == NULL ) {
        return XFS_RC ( rcNull );
    }

    BSTreeWhack ( ( BSTree * ) self, _OWPEntryWhack, NULL );

    return 0;
}   /* XFSOwpWhack () */

LIB_EXPORT
rc_t CC
XFSOwpDispose ( struct XFSOwp * self )
{
    rc_t RCt;

    RCt = 0;

    if ( self == NULL ) {
        return XFS_RC ( rcNull );
    }

    RCt = XFSOwpWhack ( self );
    if ( RCt == 0 ) {
        free ( self );
    }

    return RCt;
}   /* XFSOwpDispose () */

LIB_EXPORT
rc_t CC
XFSOwpClear ( struct XFSOwp * self )
{
    XFSOwpWhack ( self );
    XFSOwpInit ( self );

    return 0;
}   /* XFSOwpClear () */

static
int CC
_OWPStringCmp ( const char * Str1, const char * Str2 )
{
    if ( Str1 == NULL || Str2 == NULL ) {
        if ( Str1 != NULL ) {
            return 4096;
        }

        if ( Str2 != NULL ) {
            return - 4096;
        }

        return 0;
    }

    return strcmp ( Str1, Str2 );
}   /* _OWPStringCmp () */

static
int64_t CC
_OWPEntryCmp ( const void * Item, const BSTNode * Node )
{
    return _OWPStringCmp (
                ( const char * ) Item,
                ( ( Node == NULL )
                    ? NULL
                    : ( ( struct XFSOwpEntry * ) Node ) -> Key
                )
                );
}   /* _OWPEntryCmp () */

static
const struct XFSOwpEntry * CC
_OWPEntryFind ( const struct XFSOwp * self, const char * Key )
{
    struct XFSOwpEntry * Entry;

    Entry = NULL;

    if ( self != NULL && Key != NULL ) {
        Entry = ( struct XFSOwpEntry * ) BSTreeFind (
                                                    ( BSTree * )self,
                                                    Key,
                                                    _OWPEntryCmp
                                                    );
    }

    return Entry;
}   /* _OWPEntryFind () */

LIB_EXPORT
bool CC
XFSOwpHas ( const struct XFSOwp * self, const char * Key )
{
    return _OWPEntryFind ( self, Key ) != NULL ;
}   /* XFSOwpHas () */

LIB_EXPORT
const char * CC
XFSOwpGet ( const struct XFSOwp * self, const char * Key )
{
    const struct XFSOwpEntry * Entry;

    Entry = _OWPEntryFind ( self, Key );

    return Entry == NULL ? NULL : Entry -> Property;
}   /* XFSOwpGet () */

static
struct XFSOwpEntry * CC
_OWPEntryMake ( const char * Key, const char * Property )
{
    struct XFSOwpEntry * Entry;

    Entry = calloc ( 1, sizeof ( struct XFSOwpEntry ) );
    if ( Entry != NULL ) {
        if ( XFS_StrDup ( Key, ( const char ** ) & ( Entry -> Key ) ) == 0 ) {

            if ( Property == NULL ) {
                return Entry;
            }
            else {
                if ( XFS_StrDup ( Property, ( const char ** ) & ( Entry -> Property ) ) == 0 ) {
                    return Entry;
                }
            }

            free ( Entry -> Key );
        }

        free ( Entry );
    }

    return NULL;
}   /* _OWPEntryMake () */

static
int64_t CC
_OWPNodeCmp ( const BSTNode * Node1, const BSTNode * Node2 )
{
    return _OWPStringCmp (
                        ( ( struct XFSOwpEntry * ) Node1 ) -> Key,
                        ( ( struct XFSOwpEntry * ) Node2 ) -> Key
                        );

}   /* _OWPNodeCmp () */

LIB_EXPORT
rc_t CC
XFSOwpSet (
            const struct XFSOwp * self,
            const char * Key,
            const char * Property
)
{
    rc_t RCt;
    struct XFSOwpEntry * Entry;

    RCt = 0;
    Entry = NULL;

        /* I suppose, that Property could be NULL value */
    if ( self == NULL || Key == NULL ) {
        return XFS_RC ( rcNull );
    }

        /* All property values are unique */
    if ( XFSOwpHas ( self, Key ) == true ) {
        return XFS_RC ( rcInvalid );
    }

    Entry = _OWPEntryMake ( Key, Property );
    if ( Entry == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
        RCt = BSTreeInsert (
                        ( BSTree * ) self,
                        ( BSTNode * ) Entry,
                        _OWPNodeCmp
                        );
    }

    return RCt;
}   /* XFSOwpSet () */

static
void
_ListKeysCallback ( BSTNode * Node, void * Data )
{
    struct VNamelist * List;
    struct XFSOwpEntry * TheNode;

    List = NULL;
    TheNode = NULL;

    if ( Node != NULL && Data != NULL ) {
        List = ( struct VNamelist * ) Data;
        TheNode = ( struct XFSOwpEntry * ) Node;

        VNamelistAppend ( List, TheNode -> Key );

    }
}   /* _ListKeysCallback () */

LIB_EXPORT
rc_t CC
XFSOwpListKeys (
            const struct XFSOwp * self,
            const struct KNamelist ** Keys
)
{
    rc_t RCt;
    struct VNamelist * List;

    RCt = 0;

    if ( self == NULL || Keys == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Keys = NULL;

    RCt = VNamelistMake ( & List, 16 /* ?? */ );
    if ( RCt == 0 ) {
        BSTreeForEach (
                    ( BSTree * ) self,
                    false,
                    _ListKeysCallback,
                    List
                    );

        RCt = VNamelistToConstNamelist ( List, Keys );

        VNamelistRelease ( List );
    }

    return RCt;
}   /* XFSOwpListKeys () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
