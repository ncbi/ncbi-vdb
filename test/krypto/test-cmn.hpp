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

/**
 * Common functions/types to tests krypto lib
 */

#include <kfs/directory.h>
#include <kfs/file.h>
#include <krypto/encfile-priv.h>

#define BLOCK_32K_SIZE (32*1024)

#if defined(__APPLE__)
#define TMP_FOLDER "temp_mac"
#else
#define TMP_FOLDER "temp_linux"
#endif

enum TFileOpenMode
{
    TFileOpenMode_Read,
    TFileOpenMode_Write,
    TFileOpenMode_ReadWrite
};

struct KFile;

/**
 * Create/open plain text file
 */
rc_t CC
TOpenPtFile ( struct KDirectory * current_dir, const char * path, TFileOpenMode mode, struct KFile ** out_file );
rc_t CC
TCreatePtFile ( struct KDirectory * current_dir, const char * path, TFileOpenMode mode, struct KFile ** out_file );

/**
 * Create/open encrypted file
 */
rc_t CC
TOpenEncFile ( struct KDirectory * current_dir, const char * path, TFileOpenMode mode, const KKey * enc_key, struct KFile ** out_file );
rc_t CC
TCreateEncFile ( struct KDirectory * current_dir, const char * path, TFileOpenMode mode, const KKey * enc_key, struct KFile ** out_file );

/**
 * Fill "file" from pos 0 to "content_size"-1 with content defined by "filler" pattern
 */
rc_t CC
TFillFile ( struct KFile * file, const uint8_t* filler, size_t filler_size, uint64_t content_size );

/**
 * Read "file" from beginning to the end and make sure that its content matches "filler" pattern
 */
rc_t CC
TCheckFileContent ( struct KFile * file, const uint8_t* filler, size_t filler_size );

/**
 * Copies content from "file_src" to "file_dst"
 */
rc_t CC
TCopyFile ( struct KFile * file_dst, struct KFile * file_src );

inline uint64_t CC
TEncSizeFromPtSize ( uint64_t pt_size )
{
    return ((pt_size + BLOCK_32K_SIZE - 1 ) / BLOCK_32K_SIZE) * sizeof(KEncFileBlock) + sizeof(KEncFileHeader) + sizeof(KEncFileFooter);
}
