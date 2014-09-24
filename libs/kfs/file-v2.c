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

struct KFile_v2;
#define KFILE_IMPL struct KFile_v2
#define KFILE_VERS 2

#include <kfs/extern.h>

#include <kfc/ctx.h>
#include <kfc/except.h>
#include <kfc/xc.h>

#include <kfs/file-impl.h>
#include <klib/rc.h>
#include <kproc/timeout.h>
#include <os-native.h>
#include <sysalloc.h>

#include <assert.h>

/*--------------------------------------------------------------------------
 * KFile
 *  a file
 */

KITFTOK_DEF ( KFile_v2 );

/* GetSysFile
 *  returns an underlying system file object
 *  and starting offset to contiguous region
 *  suitable for memory mapping, or NULL if
 *  no such file is available.
 */
LIB_EXPORT struct KSysFile_v2 * CC KFileGetSysFile_v2 ( const KFile_v2 *self, ctx_t ctx, uint64_t *offset )
{
    FUNC_ENTRY ( ctx, rcFS, rcFile, rcRetrieving );

    if ( offset == NULL )
        INTERNAL_ERROR ( xcParamNull, "bad offset parameter" );

    else
    {
        * offset = 0;

        if ( self == NULL )
            INTERNAL_ERROR ( xcSelfNull, "failed to retrieve file" );
        else
        {
            const KFile_v2_vt * vt = KVTABLE_CAST ( TO_REFCOUNT_V1 ( self ) -> vt, ctx, KFile_v2 );
            if ( vt == NULL )
                INTERNAL_ERROR ( xcInterfaceIncorrect, "this object does not support the KFile_v2 interface" );

            else
            {
                return ( * vt -> get_sysfile ) ( self, ctx, offset );
            }
        }
    }

    return NULL;
}

/* RandomAccess
 *  ALMOST by definition, the file is random access
 *  certain file types ( notably compressors ) will refuse random access
 *
 *  returns 0 if random access, error code otherwise
 */
LIB_EXPORT bool CC KFileRandomAccess_v2 ( const KFile_v2 *self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcFS, rcFile, rcAccessing );

    if ( self == NULL )
        INTERNAL_ERROR ( xcSelfNull, "failed to access file" );

    else 
    {
        const KFile_v2_vt *vt = KVTABLE_CAST ( TO_REFCOUNT_V1 ( self ) -> vt, ctx, KFile_v2 );
        if ( vt == NULL )
            INTERNAL_ERROR ( xcInterfaceIncorrect, "this object does not support the KFile_v2 interface" );
        else
            return ( *vt -> random_access ) ( self, ctx );
    }

    return false;
}

/* Type
 *  returns a KFileDesc
 *  not intended to be a content type,
 *  but rather an implementation class
 */
LIB_EXPORT uint32_t CC KFileType_v2 ( const KFile_v2 *self, ctx_t ctx )
{
    if ( self == NULL )
        return kfdNull;
    else
    {
        FUNC_ENTRY ( ctx, rcFS, rcFile, rcAccessing );
        const KFile_v2_vt *vt = KVTABLE_CAST ( TO_REFCOUNT_V1 ( self ) -> vt, ctx, KFile_v2 );
        if ( vt == NULL )
            INTERNAL_ERROR ( xcInterfaceIncorrect, "this object does not support the KFile_v2 interface" );
        else
            return ( *vt -> get_type ) ( self, ctx );

    }

    return kfdInvalid;
}

/* Size
 *  returns size in bytes of file
 *
 *  "size" [ OUT ] - return parameter for file size
 */
LIB_EXPORT uint64_t CC KFileSize_v2 ( const KFile_v2 *self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcFS, rcFile, rcAccessing );

    if ( self == NULL )
        INTERNAL_ERROR ( xcSelfNull, "failed to access file" );
    else
    {
        const KFile_v2_vt *vt = KVTABLE_CAST ( TO_REFCOUNT_V1 ( self ) -> vt, ctx, KFile_v2 );
        if ( vt == NULL )
            INTERNAL_ERROR ( xcInterfaceIncorrect, "this object does not support the KFile_v2 interface" );
        else
            return ( *vt -> get_size ) ( self, ctx );

    }

    return 0;
}

/* SetSize
 *  sets size in bytes of file
 *
 *  "size" [ IN ] - new file size
 */
LIB_EXPORT void CC KFileSetSize_v2 ( KFile_v2 *self, ctx_t ctx, uint64_t size )
{
    FUNC_ENTRY ( ctx, rcFS, rcFile, rcResizing );

    if ( self == NULL )
        INTERNAL_ERROR ( xcSelfNull, "failed to access file" );

    else if ( ! self -> write_enabled )
        INTERNAL_ERROR ( xcFileReadOnly, "file has no write permissions" );

    else
    {
        const KFile_v2_vt *vt = KVTABLE_CAST ( TO_REFCOUNT_V1 ( self ) -> vt, ctx, KFile_v2 );
        if ( vt == NULL )
            INTERNAL_ERROR ( xcInterfaceIncorrect, "this object does not support the KFile_v2 interface" );
        else
            ( * vt -> set_size ) ( self, ctx, size );
    }
}

/* Read
 *  read file from known position
 *
 *  "pos" [ IN ] - starting position within file
 *
 *  "buffer" [ OUT ] and "bsize" [ IN ] - return buffer for read
 *
 *  "num_read" [ OUT ] - number of bytes actually read
 */
LIB_EXPORT size_t CC KFileRead_v2 ( const KFile_v2 *self, ctx_t ctx, uint64_t pos,
    void *buffer, size_t bsize )
{
    FUNC_ENTRY ( ctx, rcFS, rcFile, rcReading );

    if ( self == NULL )
        INTERNAL_ERROR ( xcSelfNull, "failed to access file" );

    else if ( ! self -> read_enabled )
        INTERNAL_ERROR ( xcFileWriteOnly, "file has no read permissions" );

    else if ( buffer == NULL )
        INTERNAL_ERROR ( xcParamNull, "buffer is NULL" );
    else if ( bsize == 0 )
        INTERNAL_ERROR ( xcBufferInsufficient, "size of buffer is insufficient" );

    else
    {
        const KFile_v2_vt *vt = KVTABLE_CAST ( TO_REFCOUNT_V1 ( self ) -> vt, ctx, KFile_v2 );
        if ( vt == NULL )
            INTERNAL_ERROR ( xcInterfaceIncorrect, "this object does not support the KFile_v2 interface" );
        else
            return ( *vt -> read ) ( self, ctx, pos, buffer, bsize );
    }
    
    return 0;
}

LIB_EXPORT size_t CC KFileTimedRead_v2 ( const KFile_v2 *self, ctx_t ctx, uint64_t pos,
    void *buffer, size_t bsize, struct timeout_t *tm )
{
    FUNC_ENTRY ( ctx, rcFS, rcFile, rcReading );

    if ( self == NULL )
        INTERNAL_ERROR ( xcSelfNull, "failed to read file" );

    else if ( ! self -> read_enabled )
        INTERNAL_ERROR ( xcFileWriteOnly, "file has no read permissions" );

    else if ( buffer == NULL )
        INTERNAL_ERROR ( xcParamNull, "buffer is NULL" );
    else if ( bsize == 0 )
        INTERNAL_ERROR ( xcBufferInsufficient, "size of buffer is insufficient" );

    else 
    {
        const KFile_v2_vt *vt = KVTABLE_CAST ( TO_REFCOUNT_V1 ( self ) -> vt, ctx, KFile_v2 );
        if ( vt == NULL )
            INTERNAL_ERROR ( xcInterfaceIncorrect, "this object does not support the KFile_v2 interface" );
        else
            return ( *vt -> timed_read ) ( self, ctx, pos, buffer, bsize, tm );
    }

    return 0;
}

/* ReadAll
 *  read from file until "bsize" bytes have been retrieved
 *  or until end-of-input
 *
 *  "pos" [ IN ] - starting position within file
 *
 *  "buffer" [ OUT ] and "bsize" [ IN ] - return buffer for read
 *
 *  "num_read" [ OUT ] - return parameter giving number of bytes
 *  actually read. when returned value is zero and return code is
 *  also zero, interpreted as end of file.
 */
LIB_EXPORT size_t CC KFileReadAll_v2 ( const KFile_v2 *self, ctx_t ctx, uint64_t pos,
    void *buffer, size_t bsize )
{
    FUNC_ENTRY ( ctx, rcFS, rcFile, rcReading );

    size_t total = 0;

    if ( self == NULL )
        INTERNAL_ERROR ( xcSelfNull, "failed to read file" );

    else if ( ! self -> read_enabled )
        INTERNAL_ERROR ( xcFileWriteOnly, "file has no read permissions" );

    else if ( buffer == NULL )
        INTERNAL_ERROR ( xcParamNull, "buffer is NULL" );
    else if ( bsize == 0 )
        INTERNAL_ERROR ( xcBufferInsufficient, "size of buffer is insufficient" );

    else 
    {
        const KFile_v2_vt *vt = KVTABLE_CAST ( TO_REFCOUNT_V1 ( self ) -> vt, ctx, KFile_v2 );
        if ( vt == NULL )
            INTERNAL_ERROR ( xcInterfaceIncorrect, "this object does not support the KFile_v2 interface" );
        else
        {
            TRY ( total = ( *vt -> read ) ( self, ctx, pos, buffer, bsize ) )
            {
                if ( total != 0 && total < bsize )
                {
                    uint8_t *b;
                    size_t count;

                    timeout_t no_block;
                    TimeoutInit ( & no_block, 0 );
                
                    for ( b = buffer; total < bsize; total += count )
                    {
                        TRY ( count = ( *vt -> timed_read ) ( self, ctx, pos + total, b + total, bsize - total, & no_block ) )
                        {
                            if ( count == 0 )
                                break;
                        }
                        CATCH_ALL ()
                        {
                            break;
                        }
                    }
                }
            }
        }
    }

    return total;
}

LIB_EXPORT size_t CC KFileTimedReadAll_v2 ( const KFile_v2 *self, ctx_t ctx, uint64_t pos,
    void *buffer, size_t bsize, struct timeout_t *tm )
{
    FUNC_ENTRY ( ctx, rcFS, rcFile, rcReading );

    size_t total = 0;

    if ( self == NULL )
        INTERNAL_ERROR ( xcSelfNull, "failed to read file" );

    else if ( ! self -> read_enabled )
        INTERNAL_ERROR ( xcFileWriteOnly, "file has no read permissions" );

    else if ( buffer == NULL )
        INTERNAL_ERROR ( xcParamNull, "buffer is NULL" );
    else if ( bsize == 0 )
        INTERNAL_ERROR ( xcBufferInsufficient, "size of buffer is insufficient" );

    else
    {
        const KFile_v2_vt *vt = KVTABLE_CAST ( TO_REFCOUNT_V1 ( self ) -> vt, ctx, KFile_v2 );
        if ( vt == NULL )
            INTERNAL_ERROR ( xcInterfaceIncorrect, "this object does not support the KFile_v2 interface" );
        else
        {
            TRY ( total = ( *vt -> timed_read ) ( self, ctx, pos, buffer, bsize, tm ) )
            {
                if ( total != 0 && total < bsize )
                {
                    uint8_t *b;
                    size_t count;

                    timeout_t no_block;
                    TimeoutInit ( & no_block, 0 );
                    
                    for ( b = buffer; total < bsize; total += count )
                    {
                        TRY ( count = ( *vt -> timed_read ) ( self, ctx, pos + total, b + total, bsize - total, & no_block ) )
                        {
                            if ( count == 0 )
                                break;
                        }
                        CATCH_ALL ()
                        {
                            break;
                        }
                    }
                }
            }
        }
    }

    return total;
}

/* ReadExactly
 * TimedReadExactly
 *  read from file until "bytes" have been retrieved
 *  or return incomplete transfer error
 *
 *  "pos" [ IN ] - starting position within file
 *
 *  "buffer" [ OUT ] and "bytes" [ IN ] - return buffer for read
 *
 *  "tm" [ IN/OUT, NULL OKAY ] - an optional indicator of
 *  blocking behavior. not all implementations will support
 *  timed reads. a NULL timeout will block indefinitely,
 *  a value of "tm->mS == 0" will have non-blocking behavior
 *  if supported by implementation, and "tm->mS > 0" will indicate
 *  a maximum wait timeout.
 */
LIB_EXPORT void CC KFileReadExactly_v2 ( const KFile_v2 *self, ctx_t ctx,
    uint64_t pos, void *buffer, size_t bytes )
{
    FUNC_ENTRY ( ctx, rcFS, rcFile, rcReading );

    if ( self == NULL )
        INTERNAL_ERROR ( xcSelfNull, "failed to read file" );

    else if ( ! self -> read_enabled )
        INTERNAL_ERROR ( xcFileWriteOnly, "file has no read permissions" );

    else if ( bytes == 0 )
        return;
    else if ( buffer == NULL )
        INTERNAL_ERROR ( xcParamNull, "buffer is NULL" );

    else
    {
        const KFile_v2_vt *vt = KVTABLE_CAST ( TO_REFCOUNT_V1 ( self ) -> vt, ctx, KFile_v2 );
        if ( vt == NULL )
            INTERNAL_ERROR ( xcInterfaceIncorrect, "this object does not support the KFile_v2 interface" );
        else
        {
            size_t total = 0;

            TRY ( total = ( *vt -> read ) ( self, ctx, pos, buffer, bytes ) )
            {
                uint8_t *b;
                size_t count;

                for ( b = buffer ; total < bytes; total += count )
                {
                    TRY ( count = ( *vt -> read ) ( self, ctx, pos + total, b + total, bytes - total ) )
                    {
                        if ( count == 0 )
                        {
                            SYSTEM_ERROR ( xcTransferIncomplete, "failed to read complete file" );
                            break;
                        }
                    }
#if 0
                    CATCH ( xcTimeoutExhausted )
                    {
                        /* ignore and try again */
                        CLEAR ();
                        count = 0;
                    }
#endif
                    CATCH_ALL ()
                    {
                        rc_t rc = ctx -> rc;
                        if ( GetRCObject ( rc ) != ( enum RCObject ) rcTimeout || GetRCState ( rc ) != rcExhausted )
                            break;

                        CLEAR ();
                        count = 0;
                    }
                }
            }
        }
    }
}

LIB_EXPORT void CC KFileTimedReadExactly_v2 ( const KFile_v2 *self, ctx_t ctx,
    uint64_t pos, void *buffer, size_t bytes, struct timeout_t *tm )
{
    FUNC_ENTRY ( ctx, rcFS, rcFile, rcReading );

    if ( self == NULL )
        INTERNAL_ERROR ( xcSelfNull, "failed to read file" );

    else if ( ! self -> read_enabled )
        INTERNAL_ERROR ( xcFileWriteOnly, "file has no read permissions" );

    else if ( bytes == 0 )
        return;
    else if ( buffer == NULL )
        INTERNAL_ERROR ( xcParamNull, "buffer is NULL" );

    else
    {
        const KFile_v2_vt *vt = KVTABLE_CAST ( TO_REFCOUNT_V1 ( self ) -> vt, ctx, KFile_v2 );
        if ( vt == NULL )
            INTERNAL_ERROR ( xcInterfaceIncorrect, "this object does not support the KFile_v2 interface" );
        else
        {
            size_t total;

            TRY ( total = ( *vt -> timed_read ) ( self, ctx, pos, buffer, bytes, tm ) )
            {
                uint8_t *b;
                size_t count;

                for ( b = buffer, total = 0; total < bytes; total += count )
                {
                    TRY ( count = ( *vt -> timed_read ) ( self, ctx, pos + total, b + total, bytes - total, tm ) )
                    {
                        if ( count == 0 )
                        {
                            INTERNAL_ERROR ( xcTransferIncomplete, "failed to read complete file" );
                            break;
                        }
                    }
#if 0
                    CATCH ( xcTimeoutExhausted )
                    {
                        /* ignore and try again */
                        CLEAR ();
                        count = 0;
                    }
#endif
                    CATCH_ALL ()
                    {
                        rc_t rc = ctx -> rc;
                        if ( GetRCObject ( rc ) != ( enum RCObject ) rcTimeout || GetRCState ( rc ) != rcExhausted )
                            break;

                        CLEAR ();
                        count = 0;
                    }
                }
            }
        }
    }
}

/* Write
 *  write file at known position
 *
 *  "pos" [ IN ] - starting position within file
 *
 *  "buffer" [ IN ] and "size" [ IN ] - data to be written
 *
 *  "num_writ" [ OUT ] - number of bytes actually written
 */
LIB_EXPORT size_t CC KFileWrite_2 ( KFile_v2 *self, ctx_t ctx, uint64_t pos,
    const void *buffer, size_t size )
{
    FUNC_ENTRY ( ctx, rcFS, rcFile, rcWriting );

    if ( self == NULL )
        INTERNAL_ERROR ( xcSelfNull, "failed to read file" );

    if ( ! self -> write_enabled )
        INTERNAL_ERROR ( xcFileReadOnly, "file has no write permissions" );

    else if ( buffer == NULL )
        INTERNAL_ERROR ( xcParamNull, "buffer is NULL" );

    else 
    {
        const KFile_v2_vt *vt = KVTABLE_CAST ( TO_REFCOUNT_V1 ( self ) -> vt, ctx, KFile_v2 );
        if ( vt == NULL )
            INTERNAL_ERROR ( xcInterfaceIncorrect, "this object does not support the KFile_v2 interface" );
        else
            return ( *vt -> write ) ( self, ctx, pos, buffer, size );
    }
    return 0;
}

LIB_EXPORT size_t CC KFileTimedWrite_v2 ( KFile_v2 *self, ctx_t ctx, uint64_t pos,
    const void *buffer, size_t size, struct timeout_t *tm )
{
    FUNC_ENTRY ( ctx, rcFS, rcFile, rcWriting );

    if ( self == NULL )
        INTERNAL_ERROR ( xcSelfNull, "failed to read file" );

    if ( ! self -> write_enabled )
        INTERNAL_ERROR ( xcFileReadOnly, "file has no write permissions" );

    else if ( buffer == NULL )
        INTERNAL_ERROR ( xcParamNull, "buffer is NULL" );

    else
    {
        const KFile_v2_vt *vt = KVTABLE_CAST ( TO_REFCOUNT_V1 ( self ) -> vt, ctx, KFile_v2 );
        if ( vt == NULL )
            INTERNAL_ERROR ( xcInterfaceIncorrect, "this object does not support the KFile_v2 interface" );
        else
            return ( *vt -> timed_write ) ( self, ctx, pos, buffer, size, tm );
    }
    return 0;
}

/* WriteAll
 *  write from file until "size" bytes have been transferred
 *  or until no further progress can be made
 *
 *  "pos" [ IN ] - starting position within file
 *
 *  "buffer" [ IN ] and "size" [ IN ] - data to be written
 *
 *  "num_writ" [ OUT, NULL OKAY ] - optional return parameter
 *  giving number of bytes actually written
 */
LIB_EXPORT size_t CC KFileWriteAll_v2 ( KFile_v2 *self, ctx_t ctx, uint64_t pos,
    const void *buffer, size_t size )
{
    size_t total = 0;

    FUNC_ENTRY ( ctx, rcFS, rcFile, rcWriting );

    if ( self == NULL )
        INTERNAL_ERROR ( xcSelfNull, "failed to read file" );

    if ( ! self -> write_enabled )
        INTERNAL_ERROR ( xcFileReadOnly, "file has no write permissions" );

    else if ( buffer == NULL )
        INTERNAL_ERROR ( xcParamNull, "buffer is NULL" );

    else
    {
        const KFile_v2_vt *vt = KVTABLE_CAST ( TO_REFCOUNT_V1 ( self ) -> vt, ctx, KFile_v2 );
        if ( vt == NULL )
            INTERNAL_ERROR ( xcInterfaceIncorrect, "this object does not support the KFile_v2 interface" );
        else
        {
            size_t count;

            TRY ( total = count = ( *vt -> write ) ( self, ctx, pos, buffer, size ) )
            {
                if ( count != 0 && count < size )
                {
                    const uint8_t *b;
                    timeout_t no_block;
                    TimeoutInit ( & no_block, 0 );
                
                    for ( b = buffer; total < size; total += count )
                    {
                        TRY ( count = ( *vt -> timed_write ) 
                              ( self, ctx, pos + total, b + total, size - total, & no_block ) )
                        {
                            if ( count == 0 )
                                break;
                        }
                        CATCH_ALL ()
                        {
                            break;
                        }
                    }
                }
            }
        }
    }

    return total;
}

LIB_EXPORT size_t CC KFileTimedWriteAll_v2 ( KFile_v2 *self, ctx_t ctx, uint64_t pos,
    const void *buffer, size_t size, struct timeout_t *tm )
{
    size_t total = 0;

    FUNC_ENTRY ( ctx, rcFS, rcFile, rcWriting );

    if ( self == NULL )
        INTERNAL_ERROR ( xcSelfNull, "failed to read file" );

    if ( ! self -> write_enabled )
        INTERNAL_ERROR ( xcFileReadOnly, "file has no write permissions" );

    else if ( buffer == NULL )
        INTERNAL_ERROR ( xcParamNull, "buffer is NULL" );

    else
    {
        const KFile_v2_vt *vt = KVTABLE_CAST ( TO_REFCOUNT_V1 ( self ) -> vt, ctx, KFile_v2 );
        if ( vt == NULL )
            INTERNAL_ERROR ( xcInterfaceIncorrect, "this object does not support the KFile_v2 interface" );
        else
        {
            TRY ( total = ( *vt -> timed_write ) ( self, ctx, pos, buffer, size, tm ) )
            {
                if ( total != 0 && total < size )
                {
                    const uint8_t *b;
                    size_t count;

                    for ( b = buffer, total = 0; total < size; total += count )
                    {
                        TRY ( count = ( *vt -> timed_write )
                              ( self, ctx, pos + total, b + total, size - total, tm ) )
                        {
                            if ( count == 0 )
                                break;
                        }
                        CATCH_ALL ()
                        {
                            break;
                        }
                    }
                }
            }
        }
    }
    return total;
}

/* Init
 *  initialize a newly allocated file object
 */
LIB_EXPORT void CC KFileInit_v2 ( KFile_v2 *self, ctx_t ctx, const KVTable *kvt,
    const char *fname, bool read_enabled, bool write_enabled )
{
    FUNC_ENTRY ( ctx, rcFS, rcFile, rcConstructing );

    TRY ( KRefcountInit_v1 ( & self -> dad, ctx, kvt, fname ) )
    {
        const KFile_v2_vt *vt = KVTABLE_CAST ( kvt, ctx, KFile_v2 );
        if ( vt == NULL )
            INTERNAL_ERROR ( xcInterfaceIncorrect, "vtable does not appear to implement KFile_v2" );
        else switch ( vt -> dad . min )
        {
        case 0:
#if _DEBUGGING
            if ( vt -> write         == NULL ||
                 vt -> read          == NULL ||
                 vt -> set_size      == NULL ||
                 vt -> get_size      == NULL ||
                 vt -> random_access == NULL ||
                 vt -> get_sysfile   == NULL ||
                 vt -> timed_write   == NULL ||
                 vt -> timed_read    == NULL ||
                 vt -> get_type      == NULL )
                
                INTERNAL_ERROR ( xcInterfaceInvalid, "null method pointer(s)" );
#endif
            break;
        default:
            INTERNAL_ERROR ( xcInterfaceInvalid, "file has an invalid version" );
        }

        self -> read_enabled = ( uint8_t ) ( read_enabled != 0 );
        self -> write_enabled = ( uint8_t ) ( write_enabled != 0 );
    }
}
