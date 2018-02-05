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
#include <klib/container.h>
#include <klib/hashtable.h>
#include <klib/refcount.h>
#include <klib/printf.h>
#include <klib/log.h>
#include <kproc/lock.h>

#include "schwarzschraube.h"
#include "zehr.h"
#include "mehr.h"
#include "hdict.h"

#include <sysalloc.h>

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/


/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
 * place, where hashgict is living. No loitering :Lol:
 *
 *_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
 * JIPPOTAM
 *_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
static
uint64_t CC
_HashOfNullTerminatedStringLol ( const char * Key )
{
        /*))    We all know that Key is null terminated, right?
         ((*/
    if ( Key != NULL ) {
        return KHash ( Key, strlen ( Key ) );
    }
    return 0;
}   /* _HashOfNullTerminatedStringLol () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
 * _HashKeyVault - we need storage for keys, to kill them at once
 * So, that is ugly, but will work till Mike will implement good
 * ways to delete keys and values ...
 * So sorry for that :D
 *_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
static
rc_t CC
_HashKeyVaultMake ( struct KHashTable ** Vault )
{
    return KHashTableInit (
                        Vault,
                        sizeof ( char * ), /* key_size ??? */
                        sizeof ( char * ), /* the stored key */
                        0,      /* capacity, def value */
                        0,      /* max_load_factor, def value */
                        cstr    /* key_cstr */
                        );
}   /* _HashKeyVaultMake () */

static
rc_t CC
_HashKeyVaultDispose ( struct KHashTable * self )
{
    char * Key, * Val;

    if ( self != NULL ) {
        KHashTableIteratorMake ( self );
        while ( KHashTableIteratorNext ( self, & Key, & Val ) ) {
            free ( Val );
        }

        KHashTableWhack ( self, NULL, NULL, NULL );
    }

    return 0;
}   /* _HashKeyVaultDispose () */

static
rc_t CC
_HashKeyVaultGetKey (
                    struct KHashTable * self,
                    const char ** FoundKey,
                    uint64_t * KeyHash,
                    const char * Key
)
{
    rc_t RCt;
    char * KeyDuplicate;

    RCt = 0;
    KeyDuplicate = NULL;

    XFS_CSAN ( FoundKey )
    XFS_CSA ( KeyHash, 0 )
    XFS_CAN ( self )
    XFS_CAN ( Key )
    XFS_CAN ( KeyHash )
    XFS_CAN ( FoundKey )

    * KeyHash = _HashOfNullTerminatedStringLol ( Key );

    if ( KHashTableFind ( self , Key, * KeyHash, & KeyDuplicate ) ) {
        * FoundKey = KeyDuplicate;
    }
    else {
        RCt = XFS_StrDup ( Key, ( const char ** ) & ( KeyDuplicate ) );
        if ( RCt == 0 ) {
            RCt = KHashTableAdd (
                                self,
                                Key,
                                * KeyHash,
                                & KeyDuplicate
                                );
            if ( RCt == 0 ) {
                * FoundKey = KeyDuplicate;
            }
            else {
                * FoundKey = NULL;
                free ( KeyDuplicate );
            }
        }
    }

    return RCt;
}   /* _HashKeyVaultGetKey () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
 * XFSHashDict
 * Tapioca ... that structure contains data for resolved item
 * I do not know where I will use it, but, seems refcounts arent
 * necessary here.
 *_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

struct XFSHashDict {
    struct KHashTable * hash_table;
    struct KHashTable * keys;

    XFSHashDictBanana banana;
};

LIB_EXPORT
rc_t CC
XFSHashDictMake (
                const struct XFSHashDict ** Dict,
                XFSHashDictBanana Banana
)
{
    rc_t RCt;
    struct XFSHashDict * Ret;

    RCt = 0;
    Ret = NULL;

    XFS_CSAN ( Dict )
    XFS_CAN ( Dict )

    Ret = calloc ( 1, sizeof ( struct XFSHashDict ) );
    if ( Ret == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
        RCt = KHashTableInit (
                            & ( Ret -> hash_table ),
                            sizeof ( char * ), /* key_size ??? */
                            sizeof ( void * ), /* value_size */
                            0,      /* capacity, def value */
                            0,      /* max_load_factor, def value */
                            cstr    /* key_cstr */
                            );
        if ( RCt == 0 ) {
            RCt = _HashKeyVaultMake ( & ( Ret -> keys ) );
            if ( RCt == 0 ) {
                Ret -> banana = Banana;
                * Dict = Ret;
            }
        }
    }

    if ( RCt != 0 ) {
        * Dict = NULL;

        if ( Ret != NULL ) {
            XFSHashDictDispose ( Ret );
        }
    }

    return RCt;
}   /* XFSHashDictMake () */

LIB_EXPORT
rc_t CC
XFSHashDictDispose ( const struct XFSHashDict * self )
{
    void * Key, * Val;
    struct XFSHashDict * Dict;

    Key = NULL;
    Val = NULL;
    Dict = ( struct XFSHashDict * ) self;

        /*))    I do believe that will happen on the end of the days,
         //     so no any kind of locking.
        ((*/
    if ( Dict != NULL ) {
        if ( Dict -> banana != NULL ) {
            KHashTableIteratorMake ( Dict -> hash_table );
            while (
                KHashTableIteratorNext (
                                        Dict -> hash_table,
                                        & Key,
                                        & Val
                                        )
            ) {
                Dict -> banana ( Val );
            }
        }

        _HashKeyVaultDispose ( Dict -> keys );

        Dict -> banana = NULL;

        KHashTableWhack ( Dict -> hash_table, NULL, NULL, NULL );

        free ( Dict );
    }

    return 0;
}   /* XFSHashDictDispose () */

LIB_EXPORT
bool CC
XFSHashDictHas ( const struct XFSHashDict * self, const char * Key )
{
    const void * Value = NULL;

    return XFSHashDictGet ( self, & Value, Key ) == 0;
}   /* XFSHashDictHas () */

LIB_EXPORT
rc_t CC
XFSHashDictGet (
                const struct XFSHashDict * self,
                const void ** Value,
                const char * Key
)
{
    rc_t RCt;
    void * Ret;
    uint64_t KeyHash;

    RCt = 0;
    Ret = NULL;
    KeyHash = 0;

    XFS_CSAN ( Value )
    XFS_CAN ( self )
    XFS_CAN ( Value )
    XFS_CAN ( Key )

    KeyHash = _HashOfNullTerminatedStringLol ( Key );

    if ( KHashTableFind ( self -> hash_table, Key, KeyHash, & Ret ) ) {
        * Value = Ret;
    }
    else {
        RCt = XFS_RC ( rcNotFound );
    }

    return RCt;
}   /* XFSHashDictGet () */

LIB_EXPORT
rc_t CC
XFSHashDictDel ( const struct XFSHashDict * self, const char * Key )
{
    rc_t RCt;
    uint64_t KeyHash;
    void * KeyVal;

    RCt = 0;
    KeyHash = 0;
    KeyVal = NULL;

    XFS_CAN ( self )
    XFS_CAN ( Key )

    KeyHash = _HashOfNullTerminatedStringLol ( Key );

        /*)) First we should retrieve value for key and free it if
         //  banana for dictionary is set
        ((*/
    if ( self -> banana != NULL ) {
        if ( KHashTableFind (
                            self -> hash_table,
                            Key,
                            KeyHash,
                            & KeyVal )
        ) {
            self -> banana ( KeyVal );
        }
    }

        /*)) Second, we call secret procedure
         ((*/
    if ( ! KHashTableDelete ( self -> hash_table, Key, KeyHash ) ) {
        RCt = XFS_RC ( rcNotFound );
    }

    return RCt;
}   /* XFSHashDictDel () */

LIB_EXPORT
rc_t CC
XFSHashDictAdd (
                const struct XFSHashDict * self,
                const void * Value,
                const char * Key
)
{
    rc_t RCt;
    uint64_t KeyHash;
    const char * KeyToUse;

    RCt = 0;
    KeyHash = 0;
    KeyToUse = NULL;

    XFS_CAN ( self )
    XFS_CAN ( Value )
    XFS_CAN ( Key )

        /*) First we should delete previous value if it exists
         /  we don't care about RC code here :LOL:
        (*/
    XFSHashDictDel ( self, Key );

        /*) Second, storeing reusable key to vault
         (*/
    RCt = _HashKeyVaultGetKey (
                            self -> keys,
                            & KeyToUse,
                            & KeyHash,
                            Key
                            );

    if ( RCt == 0 ) {
        RCt = KHashTableAdd (
                            self -> hash_table,
                            KeyToUse,
                            KeyHash,
                            & Value
                            );
    }

    return RCt;
}   /* XFSHashDictAdd () */

LIB_EXPORT
rc_t CC
XFSHashDictReserve ( const struct XFSHashDict * self, size_t NewSize )
{
    rc_t RCt = 0;

    XFS_CAN ( self )
    XFS_CAN ( self -> hash_table )

    if ( 0 < NewSize ) {
        RCt = KHashTableReserve ( self -> hash_table, NewSize );
    }

    return RCt;
}   /* XFSHashDictReserve () */

LIB_EXPORT
rc_t CC
XFSHashDictForEach (
                    const struct XFSHashDict * self,
                    XFSHashDictEacher Eacher,
                    const void * Data
)
{
    void * Key, * Val;

    Key = Val = NULL;

    XFS_CAN ( self )
    XFS_CAN ( Eacher )
    XFS_CAN ( Data )
    XFS_CAN ( self -> hash_table )

    KHashTableIteratorMake ( self -> hash_table );
    while (
        KHashTableIteratorNext ( self -> hash_table, & Key, & Val )
    ) {
        Eacher ( Key, Val, Data );
    }

    return 0;
}   /* XFSHashDictForEach () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/


/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
 * place, where receptacleH is living. No loitering :Lol:
 *
 *_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

    /**** Really, I coud use XFSHashDict here, but copy/paste ****/
struct XFSIHashDict {
    struct KHashTable * hash_table;

    XFSHashDictBanana banana;
};

LIB_EXPORT
rc_t CC
XFSIHashDictMake (
                const struct XFSIHashDict ** IDict,
                XFSHashDictBanana Banana
)
{
    rc_t RCt;
    struct XFSIHashDict * Ret;

    RCt = 0;
    Ret = NULL;

    XFS_CSAN ( IDict )
    XFS_CAN ( IDict )

    Ret = calloc ( 1, sizeof ( struct XFSIHashDict ) );
    if ( Ret == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
        RCt = KHashTableInit (
                            & ( Ret -> hash_table ),
                            sizeof ( ihashD_t ), /* key_size ??? */
                            sizeof ( void * ), /* value_size */
                            0,      /* capacity, def value */
                            0,      /* max_load_factor, def value */
                            cstr    /* key_cstr */
                            );
        if ( RCt == 0 ) {
            Ret -> banana = Banana;
            * IDict = Ret;
        }
    }

    if ( RCt != 0 ) {
        * IDict = NULL;

        if ( Ret != NULL ) {
            XFSIHashDictDispose ( Ret );
        }
    }

    return RCt;
}   /* XFSIHashDictMake () */

LIB_EXPORT
rc_t CC
XFSIHashDictDispose ( const struct XFSIHashDict * self )
{
    void * Key, * Val;
    struct XFSIHashDict * Dict;

    Key = NULL;
    Val = NULL;
    Dict = ( struct XFSIHashDict * ) self;

        /*))    I do believe that will happen on the end of the days,
         //     so no any kind of locking.
        ((*/
    if ( Dict != NULL ) {
        if ( Dict -> banana != NULL ) {
            KHashTableIteratorMake ( Dict -> hash_table );
            while (
                KHashTableIteratorNext (
                                        Dict -> hash_table,
                                        & Key,
                                        & Val
                                        )
            ) {
                Dict -> banana ( Val );
            }
        }

        Dict -> banana = NULL;

        KHashTableWhack ( Dict -> hash_table, NULL, NULL, NULL );

        free ( Dict );
    }

    return 0;
}   /* XFSIHashDictDispose () */

LIB_EXPORT
bool CC
XFSIHashDictHas ( const struct XFSIHashDict * self, ihashD_t Key )
{
    const void * Value = NULL;

    return XFSIHashDictGet ( self, & Value, Key ) == 0;
}   /* XFSIHashDictHas () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
 * JIPPOTAM
 *_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
static
uint64_t CC
_HashOfUint64TLol ( ihashD_t Key )
{
        /*))    We all know that Key is null terminated, right?
         ((*/
    return KHash ( ( void * ) & Key, sizeof ( ihashD_t ) );
}   /* _HashOfUint64TLol () */

LIB_EXPORT
rc_t CC
XFSIHashDictGet (
                const struct XFSIHashDict * self,
                const void ** Value,
                ihashD_t Key
)
{
    rc_t RCt;
    void * Ret;
    uint64_t KeyHash;

    RCt = 0;
    Ret = NULL;
    KeyHash = 0;

    XFS_CSAN ( Value )
    XFS_CAN ( self )
    XFS_CAN ( Value )

    KeyHash = _HashOfUint64TLol ( Key );

    if ( KHashTableFind ( self -> hash_table, & Key, KeyHash, & Ret ) ) {
        * Value = Ret;
    }
    else {
        RCt = XFS_RC ( rcNotFound );
    }

    return RCt;
}   /* XFSIHashDictGet () */

LIB_EXPORT
rc_t CC
XFSIHashDictDel ( const struct XFSIHashDict * self, ihashD_t Key )
{
    rc_t RCt;
    uint64_t KeyHash;
    void * KeyVal;

    RCt = 0;
    KeyHash = 0;
    KeyVal = NULL;

    XFS_CAN ( self )

    KeyHash = _HashOfUint64TLol ( Key );

        /*)) First we should retrieve value for key and free it if
         //  banana for dictionary is set
        ((*/
    if ( self -> banana != NULL ) {
        if ( KHashTableFind (
                            self -> hash_table,
                            & Key,
                            KeyHash,
                            & KeyVal )
        ) {
            self -> banana ( KeyVal );
        }
    }

        /*)) Second, we call secret procedure
         ((*/
    if ( ! KHashTableDelete ( self -> hash_table, & Key, KeyHash ) ) {
        RCt = XFS_RC ( rcNotFound );
    }

    return RCt;
}   /* XFSIHashDictDel () */

LIB_EXPORT
rc_t CC
XFSIHashDictAdd (
                const struct XFSIHashDict * self,
                const void * Value,
                ihashD_t Key
)
{
    XFS_CAN ( self )
    XFS_CAN ( Value )

        /*) First we should delete previous value if it exists
         /  we don't care about RC code here :LOL:
        (*/
    XFSIHashDictDel ( self, Key );

    return KHashTableAdd (
                        self -> hash_table,
                        & Key,
                        _HashOfUint64TLol ( Key ),
                        & Value
                        );
}   /* XFSIHashDictAdd () */

LIB_EXPORT
rc_t CC
XFSIHashDictReserve ( const struct XFSIHashDict * self, size_t NewSize )
{
    rc_t RCt = 0;

    XFS_CAN ( self )
    XFS_CAN ( self -> hash_table )

    if ( 0 < NewSize ) {
        RCt = KHashTableReserve ( self -> hash_table, NewSize );
    }

    return RCt;
}   /* XFSIHashDictReserve () */

LIB_EXPORT
rc_t CC
XFSIHashDictForEach (
                    const struct XFSIHashDict * self,
                    XFSIHashDictEacher Eacher,
                    const void * Data
)
{
    ihashD_t Key;
    void * Val;

    Key = 0;
    Val = NULL;

    XFS_CAN ( self )
    XFS_CAN ( Eacher )
    XFS_CAN ( Data )
    XFS_CAN ( self -> hash_table )

    KHashTableIteratorMake ( self -> hash_table );
    while (
        KHashTableIteratorNext ( self -> hash_table, & Key, & Val )
    ) {
        Eacher ( Key, Val, Data );
    }

    return 0;
}   /* XFSIHashDictForEach () */

LIB_EXPORT
size_t CC
XFSHashDictCount ( const struct XFSHashDict * self )
{
    return self == NULL ? 0 : KHashTableCount ( self -> hash_table );
}   /* XFSHashDictCount () */
