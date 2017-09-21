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
#include <kfs/file.h>
#include <kfs/directory.h>

#include <krypto/encfile-priv.h>

#include <assert.h>
#include <string.h>

#include "test-cmn.hpp"

/**
 * Common functions to tests krypto lib
 */

rc_t CC
TOpenPtFile ( struct KDirectory * current_dir, const char * path, TFileOpenMode mode, struct KFile ** out_file )
{
    rc_t rc;
    struct KFile * file;
    
    rc = 0;
    file = NULL;
    
    assert ( path );
    assert ( out_file );
    
    * out_file = NULL;
    
    switch (mode)
    {
        case TFileOpenMode_Read:
            rc = KDirectoryOpenFileRead ( current_dir, (const struct KFile **)& file, path );
            break;
            
        case TFileOpenMode_Write:
            rc = KDirectoryOpenFileWrite ( current_dir, & file, false, path );
            break;
            
        case TFileOpenMode_ReadWrite:
            rc = KDirectoryOpenFileWrite ( current_dir, & file, true, path );
            break;
            
        default:
            assert(false);
    }
        
    if (rc == 0)
    {
        * out_file = file;
    }
    
    return rc;
}   /* TOpenPtFile () */

rc_t CC
TCreatePtFile ( struct KDirectory * current_dir, const char * path, TFileOpenMode mode, struct KFile ** out_file )
{
    rc_t rc;
    struct KFile * file;
    
    rc = 0;
    file = NULL;
    
    assert ( path );
    assert ( out_file );
    
    * out_file = NULL;
    
    switch (mode)
    {
        case TFileOpenMode_Write:
            rc = KDirectoryCreateFile ( current_dir, & file, false, 0644, kcmParents|kcmOpen, path );
            break;
                
        case TFileOpenMode_ReadWrite:
            rc = KDirectoryCreateFile ( current_dir, & file, true, 0644, kcmParents|kcmOpen, path );
            break;
                
        case TFileOpenMode_Read:
        default:
            assert(false);
    }
        
    if (rc == 0)
    {
        * out_file = file;
    }
    
    return rc;
}   /* TCreatePtFile () */

rc_t CC
TOpenEncFile ( struct KDirectory * current_dir, const char * path, TFileOpenMode mode, const KKey * enc_key, struct KFile ** out_file )
{
    rc_t rc;
    struct KFile * pt_file, * enc_file;
    
    rc = 0;
    pt_file = NULL;
    enc_file = NULL;
    
    assert ( out_file );
    
    * out_file = NULL;
    
    rc = TOpenPtFile (current_dir, path, mode, &pt_file);
    if (rc == 0)
    {
        switch (mode)
        {
            case TFileOpenMode_Read:
                rc = KEncFileMakeRead ( (const struct KFile **)&enc_file, (const struct KFile *)pt_file, enc_key );
                break;
                
            case TFileOpenMode_Write:
                rc = KEncFileMakeWrite ( &enc_file, pt_file, enc_key );
                break;
                
            case TFileOpenMode_ReadWrite:
                rc = KEncFileMakeUpdate ( &enc_file, pt_file, enc_key );
                break;
                
            default:
                assert(false);
        }
        
        if (rc == 0)
        {
            *out_file = enc_file;
        }
        KFileRelease ( pt_file );
    }
    
    return rc;
}   /* TOpenEncFile () */

rc_t CC
TCreateEncFile ( struct KDirectory * current_dir, const char * path, TFileOpenMode mode, const KKey * enc_key, struct KFile ** out_file )
{
    rc_t rc;
    struct KFile * pt_file, * enc_file;
    
    rc = 0;
    pt_file = NULL;
    enc_file = NULL;
    
    assert ( out_file );
    
    * out_file = NULL;
    
    rc = TCreatePtFile (current_dir, path, mode, &pt_file);
    if (rc == 0)
    {
        switch (mode)
        {
            case TFileOpenMode_Write:
                rc = KEncFileMakeWrite ( &enc_file, pt_file, enc_key );
                break;
                
            case TFileOpenMode_ReadWrite:
                rc = KEncFileMakeUpdate ( &enc_file, pt_file, enc_key );
                break;
                
            case TFileOpenMode_Read:
            default:
                assert(false);
        }
        
        if (rc == 0)
        {
            *out_file = enc_file;
        }
        KFileRelease ( pt_file );
    }
    
    return rc;
}   /* TCreateEncFile () */

rc_t CC
TFillFile ( struct KFile * file, const uint8_t* filler, size_t filler_size, uint64_t content_size )
{
    rc_t rc = 0;
    
    if (content_size == 0)
    {
        // do nothing
        return rc;
    }
    
    const size_t buffer_size = 32*1024;
    
    uint8_t buffer[buffer_size];
    size_t filler_sizes_in_buffer = buffer_size / filler_size;
    
    assert(buffer_size % filler_size == 0);
    assert(buffer_size / filler_size  > 0);

    for (size_t i = 0; i < filler_sizes_in_buffer; ++i)
    {
        memmove(buffer + i*filler_size, filler, filler_size);
    }

    uint64_t num_written_total = 0;
    size_t num_written;
    while (num_written_total < content_size)
    {
        size_t to_write = content_size - num_written_total;
        if (to_write > buffer_size)
        {
            to_write = buffer_size;
        }
        rc = KFileWriteAll ( file, num_written_total, buffer, to_write, &num_written );
        if (rc != 0)
        {
            break;
        }
        
        assert(num_written == to_write);
        num_written_total += num_written;
    }
    
    return rc;
}

rc_t CC
TCheckFileContent ( struct KFile * file, const uint8_t* filler, size_t filler_size )
{
    rc_t rc = 0;
    
    uint64_t content_size;
    rc = KFileSize ( file, &content_size );
    if (rc != 0)
    {
        return rc;
    }
    
    if (content_size == 0)
    {
        // do nothing
        return rc;
    }
    
    const size_t buffer_size = 32*1024;
    
    uint8_t buffer[buffer_size], reference_buffer[buffer_size];
    size_t filler_sizes_in_buffer = buffer_size / filler_size;
    
    assert(buffer_size % filler_size == 0);
    assert(buffer_size / filler_size  > 0);
    
    for (size_t i = 0; i < filler_sizes_in_buffer; ++i)
    {
        memmove(reference_buffer + i*filler_size, filler, filler_size);
    }
    
    uint64_t num_read_total = 0;
    size_t num_read;
    while (num_read_total < content_size)
    {
        size_t to_read = content_size - num_read_total;
        if (to_read > buffer_size)
        {
            to_read = buffer_size;
        }
        rc = KFileReadAll ( file, num_read_total, buffer, to_read, &num_read );
        if (rc != 0)
        {
            break;
        }
        
        assert(num_read == to_read);
        
        if (memcmp(reference_buffer, buffer, num_read) != 0)
        {
            return RC( rcKrypto, rcFile, rcValidating, rcEncryption, rcCorrupt );
        }
        
        num_read_total += num_read;
    }
    
    return rc;
}

rc_t CC
TCopyFile ( struct KFile * file_dst, struct KFile * file_src )
{
    rc_t rc = 0;
    
    uint64_t size_src;
    rc = KFileSize ( file_src, &size_src );
    if (rc != 0)
    {
        return rc;
    }
    
    if (size_src == 0)
    {
        // do nothing
        return rc;
    }
    
    const size_t buffer_size = 32*1024;
    
    uint8_t buffer[buffer_size];
   
    uint64_t num_copied_total = 0;
    size_t num_read, num_written;
    while (num_copied_total < size_src)
    {
        size_t to_copy = size_src - num_copied_total;
        if (to_copy > buffer_size)
        {
            to_copy = buffer_size;
        }
        rc = KFileReadAll ( file_src, num_copied_total, buffer, to_copy, &num_read );
        if (rc != 0)
        {
            break;
        }
        assert(num_read == to_copy);
        rc = KFileWriteAll ( file_dst, num_copied_total, buffer, to_copy, &num_written );
        if (rc != 0)
        {
            break;
        }
        assert(num_written == to_copy);
        
        num_copied_total += num_written;
    }
    
    return rc;
}
