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

#include <kfs/extern.h>
#include "mmap-priv.h"
#include "sysmmap-priv.h"
#include "sysfile-priv.h"
#include <klib/refcount.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <string.h>


#define USE_MALLOC_MMAP 1

#if USE_MALLOC_MMAP

#ifndef FORCE_MALLOC_MMAP
#define FORCE_MALLOC_MMAP 0
#endif

#endif


/*--------------------------------------------------------------------------
 * KMMap
 *  a memory mapped region
 */


/* Unmap
 */
static
rc_t KMMapUnmapRgn ( KMMap *self )
{
    rc_t rc;

    if ( self -> sys_mmap )
        rc = KMMapUnmap ( self );
    else
    {
        rc = 0;

        if ( self -> dirty )
        {
            size_t num_writ, total;
            for (  total = 0; total < self -> size; total += num_writ )
            {
                rc = KFileWrite ( self -> f, self -> pos + total,
                    & self -> addr [ total ], self -> size - total, & num_writ );
                if ( rc != 0 || num_writ == 0 )
                    break;
            }

            if ( rc == 0 && total < self -> size )
                rc = RC ( rcFS, rcMemMap, rcDestroying, rcTransfer, rcIncomplete );
        }

        if ( rc == 0 )
            free ( self -> addr );
    }

    if ( rc == 0 )
    {
        self -> off = self -> pos = 0;
        self -> addr = NULL;
        self -> size = 0;
        self -> sys_mmap = self -> dirty = false;
    }

    return rc;
}

/* Whack
 */
static
rc_t KMMapWhack ( KMMap *self )
{
    rc_t rc = KMMapUnmapRgn ( self );
    if ( rc != 0 )
    {
        KRefcountAdd ( & self -> refcount, "KMMap" );
        return rc;
    }

    /* now goes the file */
    KFileRelease ( self -> f );
    free ( self );

    return 0;
}


/* AddRef
 * Release
 *  ignores NULL references
 */
LIB_EXPORT rc_t CC KMMapAddRef ( const KMMap *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAdd ( & self -> refcount, "KMMap" ) )
        {
        case krefLimit:
            return RC ( rcFS, rcMemMap, rcAttaching, rcRange, rcExcessive );
        }
    }
    return 0;
}

LIB_EXPORT rc_t CC KMMapRelease ( const KMMap *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDrop ( & self -> refcount, "KMMap" ) )
        {
        case krefWhack:
            return KMMapWhack ( ( KMMap* ) self );
        case krefNegative:
            return RC ( rcFS, rcMemMap, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
}

/* Size
 *  returns size of memory region
 *
 *  "size" [ OUT ] - return parameter for region size
 */
LIB_EXPORT rc_t CC KMMapSize ( const KMMap *self, size_t *size )
{
    if ( size == NULL )
        return RC ( rcFS, rcMemMap, rcAccessing, rcParam, rcNull );

    if ( self != NULL )
    {
        * size = self -> size;
        return 0;
    }

    * size = 0;
    return RC ( rcFS, rcMemMap, rcAccessing, rcSelf, rcNull );
}


/* Position
 *  returns region starting offset into mapped file
 *
 *  "pos" [ OUT ] - return parameter for file offset
 */
LIB_EXPORT rc_t CC KMMapPosition ( const KMMap *self, uint64_t *pos )
{
    if ( pos == NULL )
        return RC ( rcFS, rcMemMap, rcAccessing, rcParam, rcNull );

    if ( self != NULL )
    {
        * pos = self -> pos;
        return 0;
    }

    * pos = 0;
    return RC ( rcFS, rcMemMap, rcAccessing, rcSelf, rcNull );
}


/* Addr
 *  returns starting address of memory region
 *
 *  "addr" [ OUT ] - return parameter for memory address
 */
LIB_EXPORT rc_t CC KMMapAddrRead ( const KMMap *self, const void **addr )
{
    if ( addr == NULL )
        return RC ( rcFS, rcMemMap, rcAccessing, rcParam, rcNull );

    if ( self != NULL )
    {
        if ( self -> addr == NULL )
            return RC ( rcFS, rcMemMap, rcAccessing, rcMemMap, rcInvalid );

        * addr = ( const void* ) self -> addr;
        return 0;
    }

    * addr = NULL;
    return RC ( rcFS, rcMemMap, rcAccessing, rcSelf, rcNull );
}

LIB_EXPORT rc_t CC KMMapAddrUpdate ( KMMap *self, void **addr )
{
    if ( addr == NULL )
        return RC ( rcFS, rcMemMap, rcAccessing, rcParam, rcNull );

    * addr = NULL;

    if ( self == NULL )
        return RC ( rcFS, rcMemMap, rcAccessing, rcSelf, rcNull );

    if ( self -> read_only )
        return RC ( rcFS, rcMemMap, rcAccessing, rcMemMap, rcReadonly );

    if (  self -> addr == NULL )
        return RC ( rcFS, rcMemMap, rcAccessing, rcMemMap, rcInvalid );

    if ( ! self -> sys_mmap )
        self -> dirty = true;

    * addr = self -> addr;
    return 0;
}


/* MallocRgn
 */
#if USE_MALLOC_MMAP
static
rc_t KMMapMallocRgn ( KMMap *self, uint64_t pos, size_t size, bool read_only )
{
    rc_t rc;
    uint64_t eof;

    self -> off = 0;
    self -> read_only = read_only;
    self -> sys_mmap = self -> dirty = false;

    /* try to crop size to actual file size */
    rc = KFileSize ( self -> f, & eof );
    if ( rc == 0 )
    {
        /* mapping beyond end of file */
        if ( pos >= eof )
        {
            if ( read_only )
            {
                self -> pos = pos;
                self -> addr = NULL;
                self -> size = 0;
                return 0;
            }

            /* allocate real memory */
            self -> addr = calloc ( 1, size );
            if ( self -> addr == NULL )
                return RC ( rcFS, rcMemMap, rcConstructing, rcMemory, rcExhausted );

            self -> pos = pos;
            self -> size = size;
            self -> dirty = true;
            return 0;
        }

        /* crop for readonly case */
        if ( read_only && (( size == 0 ) || ( pos + size > eof )))
            size = ( size_t ) ( eof - pos );
    }

    /* now allocate memory */
    self -> addr = malloc ( size );
    if ( self -> addr == NULL )
        rc = RC ( rcFS, rcMemMap, rcConstructing, rcMemory, rcExhausted );
    else
    {
        size_t num_read, total;
        for ( rc = 0, total = 0; total < size; total += num_read )
        {
            rc = KFileRead ( self -> f, pos + total,
                & self -> addr [ total ], size - total, & num_read );
            if ( rc != 0 || num_read == 0 )
                break;
        }

        if ( total != 0 )
            rc = 0;

        if ( total != size )
            memset ( & self -> addr [ total ], 0, size - total );

        self -> pos = pos;
        self -> size = total;
    }

    return rc;
}
#endif


/* RWRgn
 */
static
rc_t KMMapRWRgn ( KMMap *self, uint64_t pos, size_t size )
{
    rc_t rc;
    KSysFile *sf = KFileGetSysFile ( self -> f, & self -> off );

    if ( ! self -> f -> read_enabled || ! self -> f -> write_enabled )
        rc = RC ( rcFS, rcMemMap, rcConstructing, rcFile, rcNoPerm );

    else if ( sf == NULL )
    {
#if USE_MALLOC_MMAP
        rc = KMMapMallocRgn ( self, pos, size, false );
#else
        rc = RC ( rcFS, rcMemMap, rcConstructing, rcFile, rcIncorrect );
#endif
    }

    else if ( self -> off != 0 )
        rc = RC ( rcFS, rcMemMap, rcConstructing, rcFile, rcNoPerm );

    else
    {
        uint64_t eof;
        rc = KFileSize ( self -> f, & eof );
        if ( rc == 0 )
        {
            size_t bytes;
            uint64_t pg_mask, left, right, range;

            if ( size == 0 )
            {
                size = ( size_t ) eof;
                if ( ( uint64_t ) size != eof )
                    return RC ( rcFS, rcMemMap, rcConstructing, rcParam, rcExcessive );
            }

            pg_mask = self -> pg_size - 1;
            left = pos & ~ pg_mask;
            right = ( pos + size + pg_mask ) & ~ pg_mask;
            range = right - left;

            bytes = ( size_t ) range;
            if ( ( uint64_t ) bytes != range )
                return RC ( rcFS, rcMemMap, rcConstructing, rcParam, rcExcessive );

            /* extend right edge if necessary */
            if ( eof < right )
            {
                rc = KFileSetSize ( self -> f, right );
                if ( rc != 0 )
                    return ResetRCContext ( rc, rcFS, rcMemMap, rcConstructing );
            }

            if ( bytes == 0 )
            {
                self -> addr = NULL;
                self -> addr_adj = 0;
                self -> size_adj = 0;
                self -> size = 0;
            }
            else
            {
#if FORCE_MALLOC_MMAP
                rc = -1;
#else
                rc = KMMapRWSys ( self, left, bytes );
#endif
                if ( rc != 0 )
                {
                    self -> addr_adj = self -> size_adj = 0;
                    self -> size = 0;

                    if ( eof < right )
                        KFileSetSize ( self -> f, eof );
#if USE_MALLOC_MMAP
                    rc = KMMapMallocRgn ( self, pos, size, false );
#endif
                    return rc;
                }

                self -> addr += pos - left;
                self -> addr_adj = ( uint32_t ) ( pos - left );
                self -> size_adj = ( uint32_t ) ( bytes - size );
                self -> size = size;
                self -> sys_mmap = true;
            }

            self -> pos = pos;
            self -> read_only = false;
        }
    }

    return rc;
}


/* RORgn
 */
static
rc_t KMMapRORgn ( KMMap *self, uint64_t pos, size_t size )
{
    rc_t rc;

    if ( ! self -> f -> read_enabled )
        rc = RC ( rcFS, rcMemMap, rcConstructing, rcFile, rcNoPerm );
    else
    {
        uint64_t eof;

        /* pos is relative to a virtual file; s_pos is relative to a system file */
        uint64_t s_pos = pos;

        KSysFile *sf = KFileGetSysFile ( self -> f, & self -> off );
        if ( sf == NULL )
        {
#if USE_MALLOC_MMAP
            return KMMapMallocRgn ( self, pos, size, true );
#else
            return RC ( rcFS, rcMemMap, rcConstructing, rcFile, rcIncorrect );
#endif
        }

        rc = KFileSize ( self -> f, & eof );

        /* if file has known size */
        if ( rc == 0 )
        {
            size_t bytes;
            uint64_t pg_mask, left, right, range;

            if ( size == 0 )
            {
                size = ( size_t ) eof;
                if ( ( uint64_t ) size != eof )
                    return RC ( rcFS, rcMemMap, rcConstructing, rcParam, rcExcessive );
            }

            if ( pos + size > eof )
            {
                if ( pos >= eof )
                    return RC ( rcFS, rcMemMap, rcConstructing, rcParam, rcInvalid );
                size = ( size_t ) ( eof - pos );
            }

            if ( self -> off != 0 )
            {
                s_pos = pos + self -> off;
                rc = KFileSize ( & sf -> dad, & eof ); /* eof is now for the sysfile */
                if ( rc != 0 )
                    return ResetRCContext ( rc, rcFS, rcMemMap, rcConstructing );
            }

            pg_mask = self -> pg_size - 1;
            left = s_pos & ~ pg_mask;
            right = ( s_pos + size + pg_mask ) & ~ pg_mask;

            if ( eof < right )
                right = eof;

            range = right - left;
            bytes = ( size_t ) range;
            if ( ( uint64_t ) bytes != range )
                return RC ( rcFS, rcMemMap, rcConstructing, rcParam, rcExcessive );

            if ( bytes == 0 )
            {
                size = 0;
                self -> addr = NULL;
                self -> addr_adj = 0;
                self -> size_adj = 0;
            }
            else
            {
#if FORCE_MALLOC_MMAP
                rc = -1;
#else
                rc = KMMapROSys ( self, left, bytes );
#endif
                if ( rc != 0 )
                {
                    self -> addr_adj = self -> size_adj = 0;
                    self -> size = 0;
#if USE_MALLOC_MMAP
                    rc = KMMapMallocRgn ( self, pos, size, true );
#endif
                    return rc;
                }
            
                self -> addr += s_pos - left;
                self -> addr_adj = ( uint32_t ) ( s_pos - left );
                self -> size_adj = ( uint32_t ) ( bytes - size );
                self -> sys_mmap = true;
            }
        }

        self -> pos = pos;
        self -> size = size;
        self -> read_only = true;
    }

    return rc;
}


/* MakeRgn
 *  maps a portion of a file
 *
 *  "f" [ IN ] - file to map
 *  a new reference will be attached
 *
 *  "pos" [ IN ] - starting offset into file
 *
 *  "size" [ IN, DEFAULT ZERO ] - size of resulting region. if 0,
 *  the size will be interpreted as meaning size of "f" - "pos".
 */
static
rc_t KMMapMakeRgn ( KMMap **mmp, const KFile *f )
{
    KMMap *mm;
    rc_t rc = KMMapMake ( & mm );
    if ( rc == 0 )
    {
        rc = KFileAddRef ( f );
        if ( rc == 0 )
        {
            mm -> f = ( KFile* ) f;
            mm -> off = mm -> pos = 0;
            mm -> size = 0;
            mm -> addr = NULL;
            mm -> addr_adj = mm -> size_adj = 0;
            KRefcountInit ( & mm -> refcount, 1, "KMMap", "make", "mmap" );
            mm -> sys_mmap = false;
            mm -> read_only = false;
            mm -> dirty = false;
    
            * mmp = mm;
            return 0;
        }

        KMMapWhack ( mm );
    }

    * mmp = NULL;
    return rc;
}

LIB_EXPORT rc_t CC KMMapMakeRgnRead ( const KMMap **mmp,
    const KFile *f, uint64_t pos, size_t size )
{
    rc_t rc;
    if ( mmp == NULL )
        rc = RC ( rcFS, rcMemMap, rcConstructing, rcParam, rcNull );
    else
    {
        if ( f == NULL )
            rc = RC ( rcFS, rcMemMap, rcConstructing, rcFile, rcNull );
        else if ( ! f -> read_enabled )
            rc = RC ( rcFS, rcMemMap, rcConstructing, rcFile, rcNoPerm );
        else
        {
            KMMap *mm;
            rc = KMMapMakeRgn ( & mm, f );
            if ( rc == 0 )
            {
                rc = KMMapRORgn ( mm, pos, size );
                if ( rc == 0 )
                {
                    * mmp = mm;
                    return 0;
                }

                KMMapWhack ( mm );
            }
        }

        * mmp = NULL;
    }
    return rc;
}

LIB_EXPORT rc_t CC KMMapMakeRgnUpdate ( KMMap **mmp,
    KFile *f, uint64_t pos, size_t size )
{
    rc_t rc;
    if ( mmp == NULL )
        rc = RC ( rcFS, rcMemMap, rcConstructing, rcParam, rcNull );
    else
    {
        if ( f == NULL )
            rc = RC ( rcFS, rcMemMap, rcConstructing, rcFile, rcNull );
        else if ( ! f -> read_enabled || ! f -> write_enabled )
            rc = RC ( rcFS, rcMemMap, rcConstructing, rcFile, rcNoPerm );
        else
        {
            KMMap *mm;
            rc = KMMapMakeRgn ( & mm, f );
            if ( rc == 0 )
            {
                rc = KMMapRWRgn ( mm, pos, size );
                if ( rc == 0 )
                {
                    * mmp = mm;
                    return 0;
                }

                KMMapWhack ( mm );
            }
        }

        * mmp = NULL;
    }
    return rc;
}

/* Make
 *  maps entire file
 *
 *  "f" [ IN ] - file to map.
 *  a new reference will be attached
 */
LIB_EXPORT rc_t CC KMMapMakeRead ( const KMMap **mm, const KFile *f )
{
    return KMMapMakeRgnRead ( mm, f, 0, 0 );
}

LIB_EXPORT rc_t CC KMMapMakeUpdate ( KMMap **mm, KFile *f )
{
    return KMMapMakeRgnUpdate ( mm, f, 0, 0 );
}


/* MakeMax
 *  maps as much of a file as possible
 *
 *  "f" [ IN ] - file to map.
 *  a new reference will be added
 */
LIB_EXPORT rc_t CC KMMapMakeMaxRead ( const KMMap **mm, const KFile *f )
{
    /* 32-bit architecture */
    if ( sizeof mm == 4 )
        return KMMapMakeRgnRead ( mm, f, 0, 1500 * 1024 * 1024 );

    /* 64-bit or beyond */
    return KMMapMakeRgnRead ( mm, f, 0,  ( ( size_t ) 48 ) * 1024 * 1024 * 1024 );
}

LIB_EXPORT rc_t CC KMMapMakeMaxUpdate ( KMMap **mm, KFile *f )
{
    uint64_t eof;
    rc_t rc = KFileSize ( f, & eof );
    if ( rc == 0 )
    {
        /* 32-bit architecture */
        if ( sizeof mm == 4 && eof > 1500 * 1024 * 1024 )
            eof = 1500 * 1024 * 1024;

        /* 64-bit or beyond */
        if ( sizeof mm == 8 && eof > ( ( size_t ) 48 ) * 1024 * 1024 * 1024 )
            eof = ( ( size_t ) 48 ) * 1024 * 1024 * 1024;

        rc = KMMapMakeRgnUpdate ( mm, f, 0, ( size_t ) eof );
    }
    return rc;
}


/* Reposition
 *  shift region to a new position
 *  may change region size
 *
 *  "pos" [ IN ] - new starting position for region
 *
 *  "size" [ OUT, NULL OKAY ] - optional return parameter
 *  for resultant region size
 */
LIB_EXPORT rc_t CC KMMapReposition ( const KMMap *cself, uint64_t pos, size_t *size )
{
    KMMap *self;
    size_t ignore;
    if ( size == NULL )
        size = & ignore;

    * size = 0;

    self = ( KMMap* ) cself;
    if ( cself != NULL )
    {
        rc_t rc;
        size_t rgn_size = self -> size;
        uint64_t pg_mask, left, right;

        /* regardless of condition, if no change
           in position is requested, we're done */
        if ( pos == self -> pos )
        {
            * size = rgn_size;
            return 0;
        }

        /* detect multiple owners */
        if ( atomic32_read ( & self -> refcount ) > 1 )
            return RC ( rcFS, rcMemMap, rcPositioning, rcMemMap, rcBusy );

        pg_mask = self -> pg_size - 1;
        left = self -> pos & ~ pg_mask;
        right = left + rgn_size + self -> size_adj;

        /* we're modifiable, so see if position
           is within first page of current map */
        if ( ( pos & ~ pg_mask ) == left )
        {
            self -> addr -= self -> addr_adj;
            self -> addr_adj = self -> size_adj = ( uint32_t ) ( pos - left );
            self -> addr += pos - left;
            self -> pos = pos;
            * size = self -> size = ( size_t ) ( right - pos );
            return 0;
        }

        if ( rgn_size != 0 || self -> size_adj != 0 )
        {
            rc = KMMapUnmapRgn ( self );
            if ( rc != 0 )
                return rc;
        }

        self -> addr_adj = self -> size_adj = 0;

        rc = ( self -> read_only ? KMMapRORgn : KMMapRWRgn ) ( self, pos, rgn_size );

        if ( rc == 0 )
            * size = self -> size;
        return rc;
    }

    return RC ( rcFS, rcMemMap, rcPositioning, rcSelf, rcNull );
}
