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

#include <kdb/extern.h>

#include "index-priv.h"

#include <kfs/directory.h>
#include <kfs/file.h>
#include <kfs/md5.h>
#include <kfs/mmap.h>
#include <klib/pbstree.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <string.h>
#include <byteswap.h>

/******************************************************
 *  Actually, hash file build that way 
 *
 *  Header ....
 *  uint64_t  qty
 *  uint64_t  max_val
 *  uint64_t  max_key_len
 *  <value><length><string>[0]
 *  <value><length><string>[0]
 *  <value><length><string>[0]
 *  <value><length><string>[0]
 ******************************************************/
rc_t CC
KHashIndexWhack_v5 ( KHashIndex_v5 * self )
{
    if ( self != NULL ) {
        if ( self -> key_to_val != NULL ) {
            KHashTableDispose ( self -> key_to_val, NULL, NULL, NULL );
            self -> key_to_val = NULL;
        }

        if ( self -> val_to_key != NULL ) {
            KHashTableDispose ( self -> val_to_key, NULL, NULL, NULL );
            self -> val_to_key = NULL;
        }

        if ( self -> map != NULL ) {
            KMMapRelease ( self -> map );
            self -> map = NULL;
        }

        self -> qty = 0;
        self -> max_val = 0;
        self -> max_key_len = 0;
        self -> val_wid = 0;
        self -> key_len_wid = 0;
    }

    return 0;
}   /* KHashIndexWhack_v5 () */

static rc_t CC skhiIndexLoad (
                                KHashIndex_v5 * self,  
                                const char * Addr,
                                size_t Size,
                                bool ByteSwap
                                );

rc_t CC
KHashIndexOpen_v5 (
                    KHashIndex_v5 * self,
                    const struct KMMap * Map,
                    bool ByteSwap
)
{
    rc_t RCt;
    size_t Size;
    const char * Addr;

    RCt = 0;
    Size = 0;
    Addr = NULL;

        /* When opened for create, there will be no existing index */
    if ( Map == NULL ) {
        return 0;
    }

    if ( self == NULL ) {
        return RC ( rcDB, rcIndex, rcCreating, rcSelf, rcNull );
    }

    RCt = KMMapAddRef ( Map );
    if ( RCt == 0 ) {
        self -> map = Map;

        RCt = KMMapSize ( Map, & Size );
        if ( Size < sizeof ( KIndexFileHeader_v5 ) ) {
            return RC ( rcDB, rcIndex, rcConstructing, rcIndex, rcCorrupt );
        }

        if ( RCt == 0 ) {
            RCt = KMMapAddrRead ( Map, ( const void ** ) & Addr );
            if ( RCt == 0 ) {
                RCt = skhiIndexLoad (
                                self,  
                                Addr + sizeof ( KIndexFileHeader_v5 ),
                                Size - sizeof ( KIndexFileHeader_v5 ),
                                ByteSwap
                                );
            }
        }
    }

    if ( RCt != 0 ) {
        KHashIndexWhack_v5 ( self );
    } 

    return RCt;
}   /* KHashIndexOpen_v5 () */

static
rc_t CC
skhiReadSome (
                char * Dst,
                size_t DstSize,
                const char * Addr,
                size_t Size,
                const char ** NewAddr,
                size_t * NewSize
)
{
    if ( Size < DstSize ) {
        return RC ( rcDB, rcIndex, rcCreating, rcIndex, rcCorrupt );
    }

    memmove ( Dst, Addr, DstSize );

    * NewAddr = Addr + DstSize;
    * NewSize = Size - DstSize;

    return 0;
}   /* skhiReadUint64 () */

static
rc_t CC
skhiReadUint16 (
                uint16_t * Ret,
                const char * Addr,
                size_t Size,
                bool ByteSwap,
                const char ** NewAddr,
                size_t * NewSize
)
{
    rc_t RCt;
    uint16_t Val;

    RCt = 0;
    Val = 0;

    * Ret = 0;

    RCt = skhiReadSome (
                        ( char * ) & Val,
                        sizeof ( uint16_t ),
                        Addr,
                        Size,
                        NewAddr,
                        NewSize
                        );
    if ( RCt == 0 ) {
        * Ret = ByteSwap ? bswap_16 ( Val ) : Val;
    }

    return RCt;
}   /* skhiReadUint16 () */

static
rc_t CC
skhiReadUint32 (
                uint32_t * Ret,
                const char * Addr,
                size_t Size,
                bool ByteSwap,
                const char ** NewAddr,
                size_t * NewSize
)
{
    rc_t RCt;
    uint32_t Val;

    RCt = 0;
    Val = 0;

    * Ret = 0;

    RCt = skhiReadSome (
                        ( char * ) & Val,
                        sizeof ( uint32_t ),
                        Addr,
                        Size,
                        NewAddr,
                        NewSize
                        );
    if ( RCt == 0 ) {
        * Ret = ByteSwap ? bswap_32 ( Val ) : Val;
    }

    return RCt;
}   /* skhiReadUint32 () */

static
rc_t CC
skhiReadUint64 (
                uint64_t * Ret,
                const char * Addr,
                size_t Size,
                bool ByteSwap,
                const char ** NewAddr,
                size_t * NewSize
)
{
    rc_t RCt;
    uint64_t Val;

    RCt = 0;
    Val = 0;

    * Ret = 0;

    RCt = skhiReadSome (
                        ( char * ) & Val,
                        sizeof ( uint64_t ),
                        Addr,
                        Size,
                        NewAddr,
                        NewSize
                        );
    if ( RCt == 0 ) {
        * Ret = ByteSwap ? bswap_64 ( Val ) : Val;
    }

    return RCt;
}   /* skhiReadUint64 () */

static
rc_t CC
skhiReadUint (
                uint64_t * Ret,
                uint16_t RetWid,
                const char * Ad,
                size_t Sz,
                bool Bs,
                const char ** NAd,
                size_t * NSz
)
{
    rc_t RCt = 0;

    switch ( RetWid ) {
        case 2 :
            {
                uint16_t Val;
                RCt = skhiReadUint16 ( & Val, Ad, Sz, Bs, NAd, NSz );
                if ( RCt == 0 ) {
                    * Ret = ( uint64_t ) Val;
                }
            }
            break;
        case 4 :
            {
                uint32_t Val;
                RCt = skhiReadUint32 ( & Val, Ad, Sz, Bs, NAd, NSz );
                if ( RCt == 0 ) {
                    * Ret = ( uint64_t ) Val;
                }
            }
            break;
        case 8 :
            {
                uint64_t Val;
                RCt = skhiReadUint64 ( & Val, Ad, Sz, Bs, NAd, NSz );
                if ( RCt == 0 ) {
                    * Ret = ( uint64_t ) Val;
                }
            }
            break;
        default :
            RCt = RC ( rcDB, rcIndex, rcCreating, rcParam, rcInvalid );
            break;
    }

    return RCt;
}   /* skhiReadUing () */

/*  Returns NULL terminated "const char *"
 *  So, the string is < uint16 | uint32 | uint64 > < nullterminated >
 */
static
rc_t CC
skhiReadString (
                const char ** Ret,
                uint16_t StringWid, /* size of string len */
                const char * Addr,
                size_t Size,
                bool ByteSwap,
                const char ** NewAddr,
                size_t * NewSize
)
{
    rc_t RCt;
    uint64_t Len;

    RCt = 0;
    Len = 0;

    RCt = skhiReadUint (
                        & Len,
                        StringWid,
                        Addr,
                        Size,
                        ByteSwap,
                        NewAddr,
                        NewSize
                        );
    if ( RCt == 0 ) {
        if ( * NewSize < Len ) {
            return RC ( rcDB, rcIndex, rcCreating, rcIndex, rcCorrupt );
        }

        if ( ( * NewAddr ) [ Len ] != 0 ) {
            return RC ( rcDB, rcIndex, rcCreating, rcIndex, rcCorrupt );
        }

        * Ret = * NewAddr;
        * NewAddr += Len + 1;
        * NewSize -= Len + 1;
    }

    return RCt;
}   /* skhiReadString () */

/*  Calculates a hash value ... did it 10000 errors :D
 */
static
uint64_t CC
skhiHashStr ( const char * Str )
{
    return Str == NULL ? 0 : KHash ( Str, strlen ( Str ) );
}   /* skhiHashStr () */

static
uint64_t CC
skhiHashU64 ( uint64_t Val )
{
    return KHash ( ( const char * ) & Val, sizeof ( uint64_t ) );
}   /* skhiHashU64 () */

static
uint16_t CC
skhiFieldWidth ( uint64_t MaxValue )
{
    return ( MaxValue < 65536 ) ? 2 : ( MaxValue < 4294967296) ? 4 : 8;
}   /* skhiFieldWidht () */

static
rc_t CC
skhiReadStat (
                KHashIndex_v5 * self,  
                const char * Addr,
                size_t Size,
                bool ByteSwap,
                const char ** NewAddr,
                size_t * NewSize
)
{
    rc_t RCt = 0;

    if ( NewAddr != NULL ) {
        * NewAddr = NULL;
    }

    if ( NewSize != NULL ) {
        * NewSize = 0;
    }

    if ( self == NULL ) {
        return RC ( rcDB, rcIndex, rcCreating, rcSelf, rcNull );
    }

    if ( Addr == NULL || NewAddr == NULL || NewSize == NULL ) {
        return RC ( rcDB, rcIndex, rcCreating, rcParam, rcNull );
    }

    RCt = skhiReadUint64 (
                        & ( self -> qty ),
                        Addr,
                        Size,
                        ByteSwap,
                        & Addr,
                        & Size
                        );
    if ( RCt == 0 ) {
        RCt = skhiReadUint64 (
                            & ( self -> max_val ),
                            Addr,
                            Size,
                            ByteSwap,
                            & Addr,
                            & Size
                            );
        if ( RCt == 0 ) {
            self -> val_wid = skhiFieldWidth ( self -> max_val );

            RCt = skhiReadUint64 (
                                & ( self -> max_key_len ),
                                Addr,
                                Size,
                                ByteSwap,
                                & Addr,
                                & Size
                                );
            if ( RCt == 0 ) {
                self -> key_len_wid =
                                skhiFieldWidth ( self -> max_key_len );
                * NewAddr = Addr;
                * NewSize = Size;

            }
        }
    }

    return RCt;
}   /* skhiReadStat () */

rc_t CC
skhiIndexPrepareTables ( KHashIndex_v5 * self )
{
    rc_t RCt = 0;

    if ( self == NULL ) {
        return RC ( rcDB, rcIndex, rcCreating, rcSelf, rcNull );
    }

        /* Key -> Value */
    RCt = KHashTableMake (
                        & ( self -> key_to_val ),   /* Table */
                        sizeof ( char * ),          /* Key(str) */
                        self -> val_wid,            /* Val wid */
                        self -> qty,                /* Capacity */
                        0,                          /* Load fact */
                        cstr                        /* Key type */
                        );
    if ( RCt == 0 ) {
            /* Key -> Value */
        RCt = KHashTableMake (
                            & ( self -> val_to_key ),   /* Table */
                            self -> val_wid,            /* Key wid */
                            sizeof ( char * ),          /* Val(str) */
                            self -> qty,                /* Capacity */
                            0,                          /* Load fact */
                            raw                         /* Key type */
                            );
        if ( RCt == 0 ) {
            RCt = KHashTableReserve (
                                    self -> key_to_val,
                                    self -> qty
                                    );
            if ( RCt == 0 ) {
                RCt = KHashTableReserve (
                                        self -> val_to_key,
                                        self -> qty
                                        );
            }
        }
    }

    return RCt;
}   /* skhiIndexPrepareTables () */

static
rc_t CC
skhiReadKeyVal (
                KHashIndex_v5 * self,  
                const char * Addr,
                size_t Size,
                bool ByteSwap,
                const char ** NewAddr,
                size_t * NewSize
)
{
    rc_t RCt;
    const char * Key;
    uint16_t V16;
    uint32_t V32;
    uint64_t V64;
    uint64_t Hsh;

    RCt = 0;
    Key = NULL;
    V16 = 0;
    V32 = 0;
    V64 = 0;
    Hsh = 0;

    switch ( self -> val_wid ) {
        case 2 :
            RCt = skhiReadUint16 (
                                & V16,
                                Addr,
                                Size,
                                ByteSwap,
                                NewAddr,
                                NewSize
                                );
            break;
        case 4 :
            RCt = skhiReadUint32 (
                                & V32,
                                Addr,
                                Size,
                                ByteSwap,
                                NewAddr,
                                NewSize
                                );
            break;
        case 8 :
            RCt = skhiReadUint64 (
                                & V64,
                                Addr,
                                Size,
                                ByteSwap,
                                NewAddr,
                                NewSize
                                );
            break;
        default :
            RCt = RC ( rcDB, rcIndex, rcConstructing, rcIndex, rcCorrupt );
            break;
    }

    if ( RCt != 0 ) {
        return RCt;
    }

    RCt = skhiReadString (
                        & Key,
                        self -> key_len_wid,
                        * NewAddr,
                        * NewSize,
                        ByteSwap,
                        NewAddr,
                        NewSize
                        );
    if ( RCt != 0 ) {
        return RCt;
    }

    Hsh = skhiHashStr ( Key );

    switch ( self -> val_wid ) {
        case 2 :
            RCt = KHashTableAdd ( self -> key_to_val, Key, Hsh, & V16 );
            if ( RCt == 0 ) {
                RCt = KHashTableAdd (
                                    self -> val_to_key,
                                    & V16,
                                    skhiHashU64 ( ( uint64_t ) V16 ),
                                    & Key
                                    );
            }
            break;
        case 4 :
            RCt = KHashTableAdd ( self -> key_to_val, Key, Hsh, & V32 );
            if ( RCt == 0 ) {
                RCt = KHashTableAdd (
                                    self -> val_to_key,
                                    & V32,
                                    skhiHashU64 ( ( uint64_t ) V32 ),
                                    & Key
                                    );
            }
            break;
        case 8 :
            RCt = KHashTableAdd ( self -> key_to_val, Key, Hsh, & V64 );
            if ( RCt == 0 ) {
                RCt = KHashTableAdd (
                                    self -> val_to_key,
                                    & V64,
                                    skhiHashU64 ( V64 ),
                                    & Key
                                    );
            }
            break;
        default :
            RCt = RC ( rcDB, rcIndex, rcConstructing, rcIndex, rcCorrupt );
            break;
    }

    return RCt;
}   /* skhiReadKeyVal () */

static
rc_t CC
skhiReadData (
                KHashIndex_v5 * self,  
                const char * Addr,
                size_t Size,
                bool ByteSwap,
                const char ** NewAddr,
                size_t * NewSize
)
{
    rc_t RCt;
    uint64_t llp;
    const char * CAddr;
    size_t CSize;

    RCt = 0;
    llp = 0;
    CAddr = Addr;
    CSize = Size;

    for ( llp = 0; llp < self -> qty; llp ++ ) {
        RCt = skhiReadKeyVal (
                            self,
                            CAddr,
                            CSize,
                            ByteSwap,
                            & CAddr,
                            & CSize
                            );
        if ( RCt != 0 ) {
            break;
        }
    }

    if ( RCt == 0 ) {
        * NewAddr = CAddr;
        * NewSize = CSize;
    }

    return RCt;
}   /* skhiReadData () */

rc_t CC
skhiIndexLoad (
                KHashIndex_v5 * self,  
                const char * Addr,
                size_t Size,
                bool ByteSwap
)
{
    rc_t RCt;
    const char * HashAddr;
    size_t HashSize;

    RCt = 0;
    HashAddr = NULL;
    HashSize = 0;

        /* We read some stats first */
    RCt = skhiReadStat ( 
                        self,  
                        Addr,
                        Size,
                        ByteSwap,
                        & HashAddr,
                        & HashSize
                        );
    if ( RCt == 0 ) {
            /* Here we are preparing hash tables */
        RCt = skhiIndexPrepareTables ( self );
        if ( RCt == 0 ) {
            RCt = skhiReadData (
                                self,
                                HashAddr,
                                HashSize,
                                ByteSwap,
                                & HashAddr,
                                & HashSize
                                );
            if ( RCt == 0 ) {
                /* May be something apologestic should be here :LOL: */
            }
        }
    }

    return RCt;
}   /* skhiIndexLoad () */

rc_t CC
KHashIndexFind_v5 (
                    const KHashIndex_v5 * self,
                    const char * key,
                    int64_t * id
)
{
    rc_t RCt = 0;
    uint64_t Hsh;
    bool Found;

    RCt = 0;
    Hsh = 0;
    Found = false;

    if ( id != NULL ) {
        * id = 0; /* JOJOBA: is it good value ??? */
    }

    if ( self == NULL ) {
        return RC ( rcDB, rcIndex, rcSelecting, rcSelf, rcNull );
    }

    if ( key == NULL || id == NULL ) {
        return RC ( rcDB, rcIndex, rcSelecting, rcParam, rcNull );
    }

    if ( self -> qty == 0 ) {
        RCt = RC ( rcDB, rcIndex, rcSelecting, rcString, rcNotFound );
    }
    else {
        Hsh = skhiHashStr ( key );
        switch ( self -> val_wid ) {
            case 2 :
                {
                    uint16_t Val;
                    Found = KHashTableFind (
                                        self -> key_to_val,
                                        key,
                                        Hsh,
                                        & Val
                                        );
                    if ( Found ) {
                        * id = ( int64_t ) Val;
                    }
                }
                break;
            case 4 :
                {
                    uint32_t Val;
                    Found = KHashTableFind (
                                        self -> key_to_val,
                                        key,
                                        Hsh,
                                        & Val
                                        );
                    if ( Found ) {
                        * id = ( int64_t ) Val;
                    }
                }
                break;
            case 8 :
                {
                    uint64_t Val;
                    Found = KHashTableFind (
                                        self -> key_to_val,
                                        key,
                                        Hsh,
                                        & Val
                                        );
                    if ( Found ) {
                        * id = ( int64_t ) Val;
                    }
                }
                break;
            default :
                break;
        }
        if ( Found != true ) {
            RCt = RC ( rcDB, rcIndex, rcSelecting, rcString, rcNotFound );
        }
    }

    return RCt;
}   /* KHashIndexFind_v5 () */

rc_t CC
KHashIndexProject_v5 (
                    const KHashIndex_v5 *self,
                    uint32_t id,
                    char *key_buff,
                    size_t buff_size,
                    size_t *actsize
)
{
    rc_t RCt;
    bool Found;
    uint64_t Hash;
    const char * Key;
    size_t KeyLen;

    RCt = 0;
    Found = false;
    Hash = 0;
    Key = NULL;
    KeyLen = 0;

    if ( actsize != NULL ) {
        * actsize = 0; /* JOJOBA: is it good value ??? */
    }

    if ( key_buff != NULL ) {
        * key_buff = 0;
    }

    if ( self == NULL ) {
        return RC ( rcDB, rcIndex, rcSelecting, rcSelf, rcNull );
    }

    if ( key_buff == NULL || actsize == NULL || buff_size == 0 ) {
        return RC ( rcDB, rcIndex, rcSelecting, rcParam, rcNull );
    }

    if ( self -> qty == 0 ) {
        RCt = RC ( rcDB, rcIndex, rcSelecting, rcString, rcNotFound );
    }
    else {
        Hash = skhiHashU64 ( ( uint64_t ) id );
        switch ( self -> val_wid ) {
            case 2 :
                {
                    uint16_t Val = ( uint16_t ) id;
                    Found = KHashTableFind (
                                    self -> val_to_key,
                                    & Val, 
                                    Hash,
                                    & Key
                                    );
                }
                break;
            case 4 :
                {
                    uint32_t Val = ( uint32_t ) id;
                    Found = KHashTableFind (
                                    self -> val_to_key,
                                    & Val, 
                                    Hash,
                                    & Key
                                    );
                }
                break;
            case 8 :
                {
                    uint64_t Val = ( uint64_t ) id;
                    Found = KHashTableFind (
                                    self -> val_to_key,
                                    & Val, 
                                    Hash,
                                    & Key
                                    );
                }
                break;
            default :
                break;
        }
        if ( Found != true ) {
            RCt = RC ( rcDB, rcIndex, rcSelecting, rcString, rcNotFound );
        }
        else {
            KeyLen = strlen ( Key );
            if ( buff_size < KeyLen + 1 ) {
                RCt = RC ( rcDB, rcIndex, rcSelecting, rcString, rcInsufficient );
            }
            else {
                strcpy ( key_buff, Key );
                * actsize = KeyLen;
            }
        }
    }

    return RCt;
}   /* KHashIndexProject_v5 () */

/* conspiracy check not sure if I need it here */
rc_t CC
KHashIndexCheckConsistency_v5 (
                            const KHashIndex_v5 *self,
                            int64_t *start_id,
                            uint64_t *id_range,
                            uint64_t *num_keys,
                            uint64_t *num_rows,
                            uint64_t *num_holes
)
{
    /* JOJOBA : We should do it ... lol
     */
    return 0;
}   /* KHashIndexCheckConsistency_v5 () */
