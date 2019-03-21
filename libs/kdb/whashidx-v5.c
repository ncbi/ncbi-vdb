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

#include "windex-priv.h"
#include "index-cmn.h"

#include <kdb/index.h>
#include <kfs/directory.h>
#include <kfs/file.h>
#include <kfs/md5.h>
#include <kfs/mmap.h>
#include <klib/pbstree.h>
#include <klib/rc.h>
#include <os-native.h>
#include <sysalloc.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <byteswap.h>

rc_t CC
KHashIndexWhack_v5 ( KHashIndex_v5 * self )
{
    char * Key;
    uint64_t Val;

    Key = NULL;
    Val = 0;

    if ( self != NULL ) {

        if ( self -> key_to_val != NULL ) {
                /* First we are removing all keys
                 */
            KHashTableIteratorMake ( self -> key_to_val );
            while ( KHashTableIteratorNext (
                                            self -> key_to_val,
                                            & Key,
                                            & Val
                                            )
            ) {
                free ( Key );
            }

            KHashTableDispose ( self -> key_to_val, NULL, NULL, NULL );
            self -> key_to_val = NULL;
        }

        if ( self -> val_to_key != NULL ) {
            KHashTableDispose ( self -> val_to_key, NULL, NULL, NULL );
            self -> val_to_key = NULL;
        }

        self -> qty = 0;
        self -> max_val = 0;
        self -> max_key_len = 0;
    }

    return 0;
}   /* KHashIndexWhack_v5 () */

static rc_t CC skhiIndexLoad (
                                KHashIndex_v5 * self,
                                const char * Addr,
                                size_t Size,
                                bool ByteSwap
                                );
static rc_t CC skhiIndexInit (  KHashIndex_v5 * self );

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
        return skhiIndexInit ( self );
    }

    if ( self == NULL ) {
        return RC ( rcDB, rcIndex, rcCreating, rcSelf, rcNull );
    }

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

    if ( RCt != 0 ) {
        KHashIndexWhack_v5 ( self );
    }

    return 0;
}   /* KHashIndexOpen_v5 () */

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

/*  copies null terminated string ... I need two of them :LOL:
 */
static
rc_t CC
skhiDupStr ( const char ** Dst, const char * Src )
{
    char * Str;
    size_t Len;

    Str = NULL;
    Len = 0;

    if ( Dst != NULL ) {
        * Dst = NULL;
    }

    if ( Dst == NULL || Src == NULL ) {
        return RC ( rcDB, rcIndex, rcCreating, rcParam, rcNull );
    }

    Len = strlen ( Src );
    if ( Len == 0 ) {
        return RC ( rcDB, rcIndex, rcCreating, rcParam, rcInvalid );
    }

    Str = calloc ( Len + 1, sizeof ( char ) );

    if ( Str == NULL ) {
        return RC ( rcDB, rcIndex, rcCreating, rcMemory, rcExhausted );
    }

    memmove ( Str, Src, Len );

    * Dst = Str;

    return 0;
}   /* skhiDupStr () */

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
 *  WARNING : it allocates new string, Your responcibility to free it
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

        RCt = skhiDupStr ( Ret, * NewAddr );
        if ( RCt == 0 ) {
            * NewAddr += Len + 1;
            * NewSize -= Len + 1;
        }
    }

    return RCt;
}   /* skhiReadString () */

static
uint16_t CC
skhiFieldWidth ( uint64_t MaxValue )
{
    return ( MaxValue < 65536 ) ? 2 : ( MaxValue < 4294967296) ? 4 : 8;
}   /* skhiFieldWidht () */

static
rc_t CC
skhiReadStat (
                const char * Addr,
                size_t Size,
                bool ByteSwap,
                const char ** NewAddr,
                size_t * NewSize,
                uint64_t * Qty,
                uint16_t * ValWid,
                uint16_t * KeyLenWid
)
{
    rc_t RCt;
    uint64_t KW;
    uint64_t VW;

    RCt = 0;
    KW = 0;
    VW = 0;

    if ( NewAddr != NULL ) {
        * NewAddr = NULL;
    }

    if ( NewSize != NULL ) {
        * NewSize = 0;
    }

    if ( Qty != NULL ) {
        * Qty = 0;
    }

    if ( ValWid != NULL ) {
        * ValWid = 0;
    }

    if ( KeyLenWid != NULL ) {
        * KeyLenWid = 0;
    }

    if ( Addr == NULL || NewAddr == NULL || NewSize == NULL ) {
        return RC ( rcDB, rcIndex, rcCreating, rcParam, rcNull );
    }

    if ( Qty == NULL || ValWid == NULL || KeyLenWid == NULL ) {
        return RC ( rcDB, rcIndex, rcCreating, rcParam, rcNull );
    }

    RCt = skhiReadUint64 ( Qty, Addr, Size, ByteSwap, & Addr, & Size );
    if ( RCt == 0 ) {
        RCt = skhiReadUint64 (
                            & VW,
                            Addr,
                            Size,
                            ByteSwap,
                            & Addr,
                            & Size
                            );
        if ( RCt == 0 ) {
            RCt = skhiReadUint64 (
                                & KW,
                                Addr,
                                Size,
                                ByteSwap,
                                & Addr,
                                & Size
                                );
            if ( RCt == 0 ) {
                * ValWid = skhiFieldWidth ( VW );
                * KeyLenWid = skhiFieldWidth ( KW );
                * NewAddr = Addr;
                * NewSize = Size;
            }
        }
    }

    return RCt;
}   /* skhiReadStat () */

static
rc_t CC
skhiIndexInit ( KHashIndex_v5 * self )
{
    rc_t RCt = 0;

    if ( self == NULL ) {
        return RC ( rcDB, rcIndex, rcCreating, rcSelf, rcNull );
    }

        /* Key -> Value */
    RCt = KHashTableMake (
                        & ( self -> key_to_val ),   /* Table */
                        sizeof ( char * ),          /* Key(str) */
                        sizeof ( uint64_t ),        /* Val wid */
                        0,                          /* Capacity */
                        0,                          /* Load fact */
                        cstr                        /* Key type */
                        );
    if ( RCt == 0 ) {
            /* Key -> Value */
        RCt = KHashTableMake (
                            & ( self -> val_to_key ),   /* Table */
                            sizeof ( uint64_t ),        /* Key wid */
                            sizeof ( char * ),          /* Val(str) */
                            0,                          /* Capacity */
                            0,                          /* Load fact */
                            raw                         /* Key type */
                            );

    }

    return RCt;
}   /* skhiIndexInit () */

static
rc_t CC
skhiIndexPrepareTables ( KHashIndex_v5 * self, uint64_t Qty )
{
    rc_t RCt = 0;

    if ( self == NULL ) {
        return RC ( rcDB, rcIndex, rcCreating, rcSelf, rcNull );
    }

    RCt = skhiIndexInit ( self );
    if ( RCt == 0 ) {
        RCt = KHashTableReserve ( self -> key_to_val, Qty );
        if ( RCt == 0 ) {
            RCt = KHashTableReserve ( self -> val_to_key, Qty );
        }
    }

    return RCt;
}   /* skhiIndexPrepareTables () */

static
rc_t CC
skhiReadKeyVal (
                KHashIndex_v5 * self,  
                uint16_t ValWid,
                uint16_t KeyLenWid,
                const char * Addr,
                size_t Size,
                bool ByteSwap,
                const char ** NewAddr,
                size_t * NewSize
)
{
    rc_t RCt;
    const char * Key;
    size_t KeyLen;
    uint64_t Val;

    RCt = 0;
    Key = NULL;
    KeyLen = 0;
    Val = 0;

    RCt = skhiReadUint (
                        & Val,
                        ValWid,
                        Addr,
                        Size,
                        ByteSwap,
                        NewAddr,
                        NewSize
                        );
    if ( RCt == 0 ) {

        RCt = skhiReadString (
                            & Key,
                            KeyLenWid,
                            * NewAddr,
                            * NewSize,
                            ByteSwap,
                            NewAddr,
                            NewSize
                            );
        if ( RCt == 0 ) {

            KeyLen = strlen ( Key );
            RCt = KHashTableAdd (
                                self -> key_to_val,
                                Key,
                                skhiHashStr ( Key ),
                                & Val
                                );
            if ( RCt == 0 ) {
                RCt = KHashTableAdd (
                                    self -> val_to_key,
                                    & Val,
                                    skhiHashU64 ( Val ),
                                    & Key
                                    );
                if ( RCt == 0 ) {
                    self -> qty ++;
                    if ( self -> max_val < Val ) {
                        self -> max_val = Val;
                    }
                    if ( self -> max_key_len < KeyLen ) {
                        self -> max_key_len = KeyLen;
                    }
                }
            }
        }
    }

    return RCt;
}   /* skhiReadKeyVal () */

static
rc_t CC
skhiReadData (
                KHashIndex_v5 * self,  
                uint64_t Qty,
                uint16_t ValWid,
                uint16_t KeyLenWid,
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

    for ( llp = 0; llp < Qty; llp ++ ) {
        RCt = skhiReadKeyVal (
                            self,
                            ValWid,
                            KeyLenWid,
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
    uint64_t Qty;
    uint16_t ValWid;
    uint16_t KeyLenWid;

    RCt = 0;
    HashAddr = NULL;
    HashSize = 0;
    Qty = 0;
    ValWid = 0;
    KeyLenWid = 0;

        /* We read some stats first */
    RCt = skhiReadStat ( 
                        Addr,
                        Size,
                        ByteSwap,
                        & HashAddr,
                        & HashSize,
                        & Qty,
                        & ValWid,
                        & KeyLenWid
                        );
    if ( RCt == 0 ) {
            /* Here we are preparing hash tables */
        RCt = skhiIndexPrepareTables ( self, Qty );
        if ( RCt == 0 ) {
            RCt = skhiReadData (
                                self,
                                Qty,
                                ValWid,
                                KeyLenWid,
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
    uint64_t Val;

    RCt = 0;
    Val = 0;

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
        if ( KHashTableFind (
                            self -> key_to_val,
                            key,
                            skhiHashStr ( key ),
                            & Val
                            )
        ) {
            * id = ( int64_t ) Val;
        }
        else {
            RCt = RC ( rcDB, rcIndex, rcSelecting, rcString, rcNotFound );
        }
    }

    return RCt;
}   /* KHashIndexFind_v5 () */

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

/*  insert string into hashtable, mapping to 64 bit id
 *  There is some flaw in KHashTableAdd method. Because it is
 *  just copying over a value, and a key. And when we are adding
 *  the value for the same key twice, previous index uses 'InsertUnique'
 *  to do that. So, we do a 'IndexFind' first
 */
rc_t CC
KHashIndexInsert_v5 (
                    KHashIndex_v5 * self,
                    const char * key,
                    int64_t Val
)
{
    rc_t RCt;
    const char * Key;
    size_t KeyLen;
    uint64_t Hash;

    RCt = 0;
    Key = NULL;
    KeyLen = 0;
    Hash = 0;

    if ( self == NULL ) {
        return RC ( rcDB, rcIndex, rcInserting, rcSelf, rcNull );
    }

    if ( key == NULL ) {
        return RC ( rcDB, rcIndex, rcInserting, rcParam, rcNull );
    }

        /*  Fist we should check if there is already value with the
         *  same key
         */
    Hash = skhiHashStr ( key );
    if ( KHashTableFind ( self -> key_to_val, key, Hash, & Val ) ) {
        RCt = RC ( rcDB, rcIndex, rcInserting, rcItem, rcExists );
    }
    else {
        RCt = skhiDupStr ( & Key, key );
        if ( RCt == 0 ) {
            RCt = KHashTableAdd (
                                self -> key_to_val,
                                Key,
                                Hash,
                                & Val
                                );    
            if ( RCt == 0 ) {
                RCt = KHashTableAdd (
                                    self -> val_to_key,
                                    & Val,
                                    skhiHashU64 ( Val ),
                                    & Key
                                    );
                if ( RCt == 0 ) {
                    self -> qty ++;
                    if ( self -> max_val < Val ) {
                        self -> max_val = Val;
                    }
                    KeyLen = strlen ( key );
                    if ( self -> max_key_len < KeyLen ) {
                        self -> max_key_len = KeyLen;
                    }
                }
            }
        }
    }

    return RCt;
}   /* KHashIndexInsert_v5 () */

/*  drop string from hashtable and all mappings
 *  So, we search Val by Key, and Key by Val, dropping both
 */
rc_t CC
KHashIndexDelete_v5 ( KHashIndex_v5 * self, const char * key )
{
    rc_t RCt;
    uint64_t Val;
    uint64_t Hash;
    const char * Key;

    RCt = 0;
    Val = 0;
    Hash = 0;
    Key = NULL;

    if ( self == NULL ) {
        return RC ( rcDB, rcIndex, rcInserting, rcSelf, rcNull );
    }

    if ( key == NULL ) {
        return RC ( rcDB, rcIndex, rcInserting, rcParam, rcNull );
    }

    Hash = skhiHashStr ( key );
        /*  Obtaining Val for Key
         */
    if ( KHashTableFind ( self -> key_to_val, key, Hash, & Val ) ) {
            /*  Deleting Key record from table
             */
        KHashTableDelete ( self -> key_to_val, key, Hash );

            /*  Obtaining Key for Val
             */
        Hash = skhiHashU64 ( Val );
        if ( KHashTableFind ( self -> val_to_key, & Val, Hash, & Key ) ) {
                /*  Deleting Val from table
                 */
            KHashTableDelete ( self -> val_to_key, & Val, Hash );

                /*  Freeing Key ...
                 */
            free ( ( char * ) Key );

            self -> qty --;
        }
        else {
            RCt = RC ( rcDB, rcIndex, rcRemoving, rcIndex, rcCorrupt );
        }
    }
    else {
        RCt = RC ( rcDB, rcIndex, rcRemoving, rcString, rcNotFound );
    }

    return RCt;
}   /* KHashIndexDelete_v5 () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
 * There we are perspiring
 *_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
struct HIP_data {
    uint64_t pos;       /*  Position if file */
    uint64_t buf_pos;   /*  Position in buffer */

    const char * file_path;
    const char * file_path_tmp;
    const char * md5_file_path;
    const char * md5_file_path_tmp;

    KFile * file;

    char * buf;
    size_t buf_size;

    rc_t RCt;
};

static
rc_t CC
skhiHIP_dataInit (
                struct HIP_data * HIP,
                struct KDirectory * Dir,
                const char * Path,
                bool UseMD5
)
{
    rc_t RCt;
    char Buf [ 1024 ];

    RCt = 0;
    * Buf = 0;

    if ( HIP == NULL ) {
        return RC ( rcDB, rcIndex, rcPersisting, rcParam, rcNull );
    }

    memset ( HIP, 0, sizeof ( struct HIP_data ) );

        /*  First we are allocating buffer
         */
    HIP -> buf_size = 128 * 1024;
    HIP -> buf = calloc ( HIP -> buf_size, sizeof ( char ) );
    if ( HIP -> buf == NULL ) {
        RCt = RC ( rcDB, rcIndex, rcPersisting, rcMemory, rcExhausted );
    }

        /*  Second is resolving paths
         */
    RCt = KDirectoryResolvePath (
                                Dir,
                                false,
                                Buf,
                                sizeof ( Buf ),
                                "%s",
                                Path
                                );
    if ( RCt == 0 ) {
        RCt = skhiDupStr ( & ( HIP -> file_path ), Buf );
        if ( RCt == 0 ) {
            sprintf ( Buf, "%s.tmp", HIP -> file_path );
            RCt = skhiDupStr ( & ( HIP -> file_path_tmp ), Buf );
        }
    }

    if ( RCt == 0 && UseMD5 ) {
        sprintf ( Buf, "%s.md5", HIP -> file_path );
        RCt = skhiDupStr ( & ( HIP -> md5_file_path ), Buf );
        if ( RCt == 0 ) {
            sprintf ( Buf, "%s.md5", HIP -> file_path_tmp );
            RCt = skhiDupStr ( & ( HIP -> md5_file_path_tmp ), Buf );
        }
    }

    return RCt;
}   /* skhiHIP_dataInit () */

static
rc_t CC
skhiHIP_dataWhack ( struct HIP_data * HIP )
{
    if ( HIP != NULL ) {
        if ( HIP -> file_path != NULL ) {
            free ( ( char * ) HIP -> file_path );
            HIP -> file_path = NULL;
        }

        if ( HIP -> file_path_tmp != NULL ) {
            free ( ( char * ) HIP -> file_path_tmp );
            HIP -> file_path_tmp = NULL;
        }
        
        if ( HIP -> md5_file_path != NULL ) {
            free ( ( char * ) HIP -> md5_file_path );
            HIP -> md5_file_path = NULL;
        }
        
        if ( HIP -> md5_file_path_tmp != NULL ) {
            free ( ( char * ) HIP -> md5_file_path_tmp );
            HIP -> md5_file_path_tmp = NULL;
        }
        
        if ( HIP -> file != NULL ) {
            KFileRelease ( HIP -> file );
            HIP -> file = NULL;
        }

        if ( HIP -> buf != NULL ) {
            free ( HIP -> buf );
            HIP -> buf = NULL;
        }
        
        HIP -> buf_size = 0;
        HIP -> pos = 0;
        HIP -> RCt = 0;
    }

    return 0;
}   /* skhiHIP_dataWhack () */

static
rc_t CC
skhiPrepareFiles (
                    const KHashIndex_v5 * self,
                    struct HIP_data * HIP,
                    struct KDirectory * Dir
)
{
    rc_t RCt;
    KMD5SumFmt * Fmt;
    KFile * md5F;
    KMD5File * md5FWR;

    RCt = 0;
    md5F = NULL;
    Fmt = NULL;
    md5FWR = NULL;

    if ( HIP == NULL ) {
        return RC ( rcDB, rcIndex, rcPersisting, rcParam, rcNull );
    }

    if ( HIP -> file_path == NULL ) {
        return RC ( rcDB, rcIndex, rcPersisting, rcParam, rcInvalid );
    }


        /*  First we are creating index temporary file
         */
    RCt = KDirectoryCreateFile (
                                Dir,
                                & ( HIP -> file ),
                                true,
                                0664,
                                kcmInit,
                                "%s",
                                HIP -> file_path_tmp
                                );
    if ( RCt == 0 ) {

        if ( HIP -> md5_file_path_tmp != NULL ) {
            RCt = KDirectoryCreateFile (
                                        Dir,
                                        & md5F,
                                        true,
                                        0664,
                                        kcmInit,
                                        "%s",
                                        HIP -> md5_file_path_tmp
                                        );
            if ( RCt == 0 ) {
                RCt = KMD5SumFmtMakeUpdate ( & Fmt, md5F );
                if ( RCt == 0 ) {
                    RCt = KMD5FileMakeWrite (
                                            & md5FWR,
                                            HIP -> file,
                                            Fmt,
                                            HIP -> file_path
                                            );
                    KMD5SumFmtRelease ( Fmt );

                    if ( RCt == 0 ) {
                        HIP -> file = KMD5FileToKFile ( md5FWR );
                    }
                }
            }
        }
    }

    if ( RCt != 0 ) {
        KFileRelease ( HIP -> file );
    }

    return RCt;
}   /* skhiPrepareFiles () */

static
rc_t CC
skhiWriteSome ( struct HIP_data * HIP, const char * Data, size_t Size )
{
    rc_t RCt;
    size_t NumWrit;
    const char * cData;
    size_t cSize;
    size_t toWR;

    RCt = 0;
    NumWrit = 0;
    cData = Data;
    cSize = Size;
    toWR = 0;

    if ( HIP == NULL ) {
        return RC ( rcDB, rcIndex, rcPersisting, rcParam, rcNull );
    }

    if ( HIP -> RCt != 0 ) {
        return HIP -> RCt;
    }

    if ( Data == NULL ) {
        HIP -> RCt = RC ( rcDB, rcIndex, rcPersisting, rcParam, rcNull );
        return HIP -> RCt;
    }

    if ( HIP -> buf == NULL || HIP -> file == NULL ) {
        HIP -> RCt =  RC ( rcDB, rcIndex, rcPersisting, rcParam, rcInvalid );
        return HIP -> RCt;
    }

        /*  Nothing to do
         */
    if ( Size == 0 ) {
        return 0;
    }

    if ( HIP -> buf_size < HIP -> buf_pos + Size ) {
        while ( HIP -> buf_size < HIP -> buf_pos + cSize ) {
            toWR = HIP -> buf_size - HIP -> buf_pos;

            memmove ( HIP -> buf + HIP -> buf_pos, cData, toWR );

            RCt = KFileWriteAll (
                            HIP -> file,
                            HIP -> pos,
                            HIP -> buf,
                            HIP -> buf_size,
                            & NumWrit
                            );
            if ( RCt != 0 ) {
                HIP -> RCt = RCt;
                break;
            }

            if ( NumWrit != HIP -> buf_size ) {
                RCt = RC ( rcDB, rcIndex, rcPersisting, rcTransfer, rcIncomplete );
                HIP -> RCt = RCt;
                break;
            }

            cData += toWR;
            cSize -= toWR;
            HIP -> buf_pos = 0;
            HIP -> pos += HIP -> buf_size;
        }

        if ( RCt == 0 ) {
            if ( cSize != 0 ) {
                memmove ( HIP -> buf, cData, cSize );
                HIP -> buf_pos += cSize;
            }
        }
    }
    else {
        memmove ( HIP -> buf + HIP -> buf_pos, Data, Size );
        HIP -> buf_pos += Size;
    }


    return RCt;
}   /* skhiWriteSome () */

static
rc_t CC
skhiFlushSome ( struct HIP_data * HIP )
{
    rc_t RCt;
    size_t NumWrit;

    RCt = 0;
    NumWrit = 0;

    if ( HIP == NULL ) {
        return RC ( rcDB, rcIndex, rcPersisting, rcParam, rcNull );
    }

    if ( HIP -> RCt != 0 ) {
        return HIP -> RCt;
    }

    if ( HIP -> buf == NULL || HIP -> file == NULL ) {
        HIP -> RCt = RC ( rcDB, rcIndex, rcPersisting, rcParam, rcInvalid );
        return HIP -> RCt;
    }

    if ( HIP -> buf_pos != 0 ) {
        RCt = KFileWriteAll (
                            HIP -> file,
                            HIP -> pos,
                            HIP -> buf,
                            HIP -> buf_pos,
                            & NumWrit
                            );
        if ( RCt == 0 ) {
            if ( NumWrit != HIP -> buf_pos ) {
                RCt = RC ( rcDB, rcIndex, rcPersisting, rcTransfer, rcIncomplete );
                HIP -> RCt = RCt;
            }
            else {
                HIP -> buf_pos = 0;
            }
        }
    }

    return RCt;
}   /* skhiFlushSome () */


static
rc_t CC
skhiPerspireHeader ( const KHashIndex_v5 * self, struct HIP_data * HIP )
{
    rc_t RCt;
    KIndexFileHeader_v5 Header;

    RCt = 0;
    memset ( & Header, 0, sizeof ( Header ) );

    if ( self == NULL ) {
        return RC ( rcDB, rcIndex, rcPersisting, rcSelf, rcNull );
    }

    if ( HIP == NULL ) {
        return RC ( rcDB, rcIndex, rcPersisting, rcParam, rcNull );
    }

    if ( HIP -> RCt != 0 ) {
        return HIP -> RCt;
    }

        /*  KDBHdr initialisation
         */
    KDBHdrInit ( & ( Header . h ), KDBINDEXVERS );
    Header . index_type = kitHash;

    RCt = skhiWriteSome ( HIP, ( const char * ) & Header, sizeof ( Header ) );
    if ( RCt == 0 ) {
        RCt = skhiWriteSome (
                        HIP,
                        ( const char * ) & self -> qty,
                        sizeof ( self -> qty )
                        );
        if ( RCt == 0 ) {
            RCt = skhiWriteSome (
                            HIP,
                            ( const char * ) & self -> max_val,
                            sizeof ( self -> max_val )
                            );
            if ( RCt == 0 ) {
                RCt = skhiWriteSome (
                                HIP,
                                ( const char * ) & self -> max_key_len,
                                sizeof ( self -> max_key_len )
                                );
            }
        }
    }

    return RCt;
}   /* skhiPerspireHeader () */

static
rc_t CC
skhiPerspireLine (
                    struct HIP_data * HIP,
                    const char * Key,
                    uint16_t KeyLenWid,
                    uint64_t Val,
                    uint16_t ValWid
)
{
    rc_t RCt;
    size_t KeyLen;

    RCt = 0;
    KeyLen = 0;

        /*  NO STANDARD CHECKS THEY SUPPOSE TO BE DONE EARLIER
         */
    if ( HIP -> RCt != 0 ) {
        return HIP -> RCt;
    }

        /*  First is value 
         */
    switch ( ValWid ) {
        case 2 :
            {
                uint16_t cVal = ( uint16_t ) Val;
                RCt = skhiWriteSome (
                                HIP, ( char * ) & cVal, sizeof ( cVal )
                                );
            }
            break;
        case 4 :
            {
                uint32_t cVal = ( uint32_t ) Val;
                RCt = skhiWriteSome (
                                HIP, ( char * ) & cVal, sizeof ( cVal )
                                );
            }
            break;
        case 8 :
            {
                uint64_t cVal = ( uint64_t ) Val;
                RCt = skhiWriteSome (
                                HIP, ( char * ) & cVal, sizeof ( cVal )
                                );
            }
            break;
        default :
            RCt = RC ( rcDB, rcIndex, rcPersisting, rcParam, rcInvalid );
            break;
    }

    if ( RCt != 0 ) {
        HIP -> RCt = RCt;

        return RCt;
    }

    KeyLen = strlen ( Key );

    switch ( KeyLenWid ) {
        case 2 :
            {
                uint16_t cVal = ( uint16_t ) KeyLen;
                RCt = skhiWriteSome (
                                HIP, ( char * ) & cVal, sizeof ( cVal )
                                );
            }
            break;
        case 4 :
            {
                uint32_t cVal = ( uint32_t ) KeyLen;
                RCt = skhiWriteSome (
                                HIP, ( char * ) & cVal, sizeof ( cVal )
                                );
            }
            break;
        case 8 :
            {
                uint64_t cVal = ( uint64_t ) KeyLen;
                RCt = skhiWriteSome (
                                HIP, ( char * ) & cVal, sizeof ( cVal )
                                );
            }
            break;
        default :
            RCt = RC ( rcDB, rcIndex, rcPersisting, rcParam, rcInvalid );
            break;
    }

    if ( RCt == 0 ) {
        RCt = skhiWriteSome ( HIP, Key, KeyLen + 1 );
    }

    if ( RCt != 0 ) {
        HIP -> RCt = RCt;
    }

    return RCt;
}   /* skhiPerspireLine () */

static
rc_t CC
skhiPerspireTable ( const KHashIndex_v5 * self, struct HIP_data * HIP )
{
    rc_t RCt;
    char * Key;
    uint64_t Val;
    uint64_t Qty;
    uint16_t ValWid;
    uint16_t KeyLenWid;

    RCt = 0;
    Key = NULL;
    Val = 0;
    Qty = 0;
    ValWid = 0;
    KeyLenWid = 0;

    if ( self == NULL ) {
        return RC ( rcDB, rcIndex, rcPersisting, rcSelf, rcNull );
    }

    if ( HIP == NULL ) {
        return RC ( rcDB, rcIndex, rcPersisting, rcParam, rcNull );
    }

    if ( HIP -> RCt != 0 ) {
        return HIP -> RCt;
    }

        /*  First we should define Val and KeyLen widhts
         */
    ValWid = skhiFieldWidth ( self -> max_val );
    KeyLenWid = skhiFieldWidth ( self -> max_key_len );

    KHashTableIteratorMake ( self -> key_to_val );
    while ( KHashTableIteratorNext (
                                    self -> key_to_val,
                                    & Key,
                                    & Val
                                    )
    ) {
        RCt = skhiPerspireLine ( HIP, Key, KeyLenWid, Val, ValWid );
        if ( RCt != 0 ) {
            break;
        }

        Qty ++;
    }

    if ( RCt == 0 && Qty != self -> qty ) {
        RCt = RC ( rcDB, rcIndex, rcConstructing, rcIndex, rcCorrupt );

        HIP -> RCt = RCt;
    }

    return RCt;
}   /* skhiPerspireTable () */

/*  Moving files all around
 */
static 
rc_t CC
skhiFinishPerspiration (
                        struct HIP_data * HIP,
                        struct KDirectory * Dir,
                        bool use_md5
)
{
    rc_t RCt = 0;

    if ( HIP == NULL || Dir == NULL ) {
        return RC ( rcDB, rcIndex, rcInserting, rcParam, rcNull );
    }

    RCt = KDirectoryRename (
                            Dir,
                            false,
                            HIP -> file_path_tmp,
                            HIP -> file_path
                            );
    if ( RCt == 0 ) {
        if ( use_md5 ) {
            RCt = KDirectoryRename (
                                    Dir,
                                    false,
                                    HIP -> md5_file_path_tmp,
                                    HIP -> md5_file_path
                                    );
        }
    }

    return RCt;
}   /* kshiFinishPerspiration () */

/*  persist index to file
 */
rc_t CC
KHashIndexPersist_v5 (
                    const KHashIndex_v5 * self,
                    struct KDirectory * dir,
                    const char * path,
                    bool use_md5
)
{
    rc_t RCt;
    struct HIP_data HIP;

    RCt = 0;

    if ( self == NULL ) {
        return RC ( rcDB, rcIndex, rcInserting, rcSelf, rcNull );
    }

    if ( self -> qty == 0 ) {
        return 0;
    }

    RCt = skhiHIP_dataInit ( & HIP, dir, path, use_md5 ); 
    if ( RCt == 0 ) {
        RCt = skhiPrepareFiles ( self, & HIP, dir );
        if ( RCt == 0 ) {
            RCt = skhiPerspireHeader ( self, & HIP );
            if ( RCt == 0 ) {
                RCt = skhiPerspireTable ( self, & HIP );
                if ( RCt == 0 ) {
                    RCt = skhiFlushSome ( & HIP );
                }
            }

                /*  We don't need that anymore
                 */
            KFileRelease ( HIP . file );
            HIP . file = NULL;
        }
    }

    if ( RCt == 0 ) {
        RCt = skhiFinishPerspiration ( & HIP, dir, use_md5 );
    }

    skhiHIP_dataWhack ( & HIP );

    return RCt;
}   /* KHashIndexPersist_v5 () */

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
    uint64_t Val;
    const char * Key;
    size_t KeyLen;

    RCt = 0;
    Found = false;
    Val = ( uint64_t ) id;
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
        Found = KHashTableFind (
                                self -> val_to_key,
                                & Val,
                                skhiHashU64 ( Val ),
                                & Key
                                );
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
