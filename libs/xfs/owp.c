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

#include <kfg/config.h>

#include "hdict.h"
#include "owp.h"
#include "schwarzschraube.h"

#include <sysalloc.h>

#include <ctype.h>

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*))
 //   it is a struct, don't know why ... prolly will add refcount later
((*/
struct XFSOwpEntry {
    const char * Key;
    const char * Property;
};

static
rc_t CC
_OWPEntryDispose ( const struct XFSOwpEntry * self )
{
    struct XFSOwpEntry * Entry = ( struct XFSOwpEntry * ) self;

    if ( Entry != NULL ) {
        if ( Entry -> Key != NULL ) {
            free ( ( char * ) Entry -> Key );
            Entry -> Key = NULL;
        }

        if ( Entry -> Property != NULL ) {
            free ( ( char * ) Entry -> Property );
            Entry -> Property = NULL;
        }

        free ( Entry );
    }

    return 0;
}   /* _OWPEntryDispose () */

static
rc_t CC
_OWPEntryMake (
                const struct XFSOwpEntry ** Entry,
                const char * Key,
                const char * Property
)
{
    rc_t RCt;
    struct XFSOwpEntry * TheEntry;

    RCt = 0;
    TheEntry = NULL;

    XFS_CSAN ( Entry )
    XFS_CAN ( Entry )
    XFS_CAN ( Key )
    XFS_CAN ( Property )

    TheEntry = calloc ( 1, sizeof ( struct XFSOwpEntry ) );
    if ( TheEntry == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
        RCt = XFS_StrDup ( Key, & ( TheEntry -> Key ) );
        if ( RCt == 0 ) {

            if ( Property != NULL ) {
                RCt = XFS_StrDup ( Property, & ( TheEntry -> Property ) );
            }

            if ( RCt == 0 ) {
                * Entry = TheEntry;
            }
        }
    }

    if ( RCt != 0 ) {
        * Entry = NULL;

        if ( TheEntry != NULL ) {
            _OWPEntryDispose ( TheEntry );
        }
    }

    return RCt;
}   /* _OWPEntryMake () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

struct XFSOwp {
    const struct XFSHashDict * hash_dict;
};

static
void CC
_OwpEntryWhacker ( const void * Data )
{
    if ( Data != NULL ) {
        _OWPEntryDispose ( ( struct XFSOwpEntry * ) Data );
    }
}   /* _OwpEntryWhacker () */

LIB_EXPORT
rc_t CC
XFSOwpMake ( struct XFSOwp ** Owp )
{
    rc_t RCt;
    struct XFSOwp * tmpOwp;

    RCt = 0;
    tmpOwp = NULL;

    XFS_CSAN ( Owp )
    XFS_CAN ( Owp )

    tmpOwp = calloc ( 1, sizeof ( struct XFSOwp ) );
    if ( tmpOwp == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
        RCt = XFSOwpInit ( tmpOwp );
        if ( RCt == 0 ) {
            * Owp = tmpOwp;
        }
    }

    if ( RCt != 0 ) {
        * Owp = NULL; /* I know it was initialized in that way, but */

        if ( tmpOwp != NULL ) {
            XFSOwpDispose ( tmpOwp );
        }
    }

    return RCt;
}   /* XFSOwpMake () */

LIB_EXPORT
rc_t CC
XFSOwpInit ( struct XFSOwp * self )
{
    XFS_CAN ( self )

    return XFSHashDictMake ( & self -> hash_dict, _OwpEntryWhacker );
}   /* XFSOwpInit () */

LIB_EXPORT
rc_t CC
XFSOwpWhack ( struct XFSOwp * self )
{
    XFS_CAN ( self )

    return XFSHashDictDispose ( self -> hash_dict );
}   /* XFSOwpWhack () */

LIB_EXPORT
rc_t CC
XFSOwpDispose ( struct XFSOwp * self )
{
    if ( self != NULL ) {
        XFSOwpWhack ( self );

        free ( self );
    }

    return 0;
}   /* XFSOwpDispose () */

LIB_EXPORT
rc_t CC
XFSOwpClear ( struct XFSOwp * self )
{
    XFSOwpWhack ( self );

    return XFSOwpInit ( self );
}   /* XFSOwpClear () */

static
const struct XFSOwpEntry * CC
_OWPEntryFind ( const struct XFSOwp * self, const char * Key )
{
    struct XFSOwpEntry * Entry = NULL;

    if ( self != NULL && Key != NULL ) {
        XFSHashDictGet (
                        self -> hash_dict,
                        ( const void ** ) & Entry,
                        Key
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

LIB_EXPORT
rc_t CC
XFSOwpSet (
            const struct XFSOwp * self,
            const char * Key,
            const char * Property
)
{
    rc_t RCt;
    const struct XFSOwpEntry * Entry;

    RCt = 0;
    Entry = NULL;

        /*  I suppose, that Property could be NULL value
         */
    XFS_CAN ( self );
    XFS_CAN ( Key );

        /* All property values are unique */
    if ( XFSOwpHas ( self, Key ) == true ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = _OWPEntryMake ( & Entry, Key, Property );
    if ( RCt == 0 ) {
        RCt = XFSHashDictAdd ( self -> hash_dict, Entry, Key );
    }

    return RCt;
}   /* XFSOwpSet () */

static
void
_ListKeysEacher (
                const char * Key,
                const void * Value,
                const void * Data
)
{
    struct VNamelist * List;
    struct XFSOwpEntry * TheEntry;

    List = ( struct VNamelist * ) Data;
    TheEntry = ( struct XFSOwpEntry * ) Value;

    if ( List != NULL && TheEntry != NULL ) {
        VNamelistAppend ( List, TheEntry -> Key );
    }
}   /* _ListKeysEacher () */

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
    List = NULL;

    XFS_CSAN ( Keys )
    XFS_CAN ( self )
    XFS_CAN ( Keys )

    RCt = VNamelistMake ( & List, 16 /* ?? */ );
    if ( RCt == 0 ) {
        RCt = XFSHashDictForEach (
                                self -> hash_dict,
                                _ListKeysEacher,
                                List
                                );
        if ( RCt == 0 ) {
            RCt = VNamelistToConstNamelist ( List, Keys );
        }

        VNamelistRelease ( List );
    }

    return RCt;
}   /* XFSOwpListKeys () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
