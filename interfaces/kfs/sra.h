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

#ifndef _h_kfs_sra_
#define _h_kfs_sra_

#ifndef _h_kfs_extern_
#include <kfs/extern.h>
#endif

#include <klib/defs.h>

#include <stdarg.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * This file provides documentation and symbols for the single file archive
 * format originally created for the SRA project.
 */

/*
 * filler bytes before data files can be inserted to make the start line
 * up on specified boundaries.  By using a single bit approach for the values
 * we get the bit-wise not of the mask that must be met for the start of the file.
 * Or one more than the maximum number of inserted bytes.  This is only used on
 * creation of the file and has no bearing at all on the parsing afterwards.
 * In it's only use so far any bit in a 32 bit bit number could be used not
 * just these enumerated values.  
 */
typedef uint32_t KSRAFileAlignment;
enum eKSRAFileAlignment
{
    /* align on byte count unit boundaries
     * set this to the number of bytes in the alignment unit
     */
    sraAlignInvalid = 0,
    sraAlign1Byte = 1,
    sraAlign2Byte = 2,
    sraAlign4Byte = 4,
    sraAlign8Byte = 8,
    sraAlign16Byte = 16,
    sraAlign32Byte = 32,
    sraAlign64Byte = 64
};

/* ======================================================================
 *
 * SRA File Format
 */
typedef uint32_t SraEndian_t;
/* enum fails to handle these due to integer overflow */
#define eSraByteOrderTag     0x05031988
#define eSraByteOrderReverse 0x88190305




typedef struct KSraHeader KSraHeader;
struct KSraHeader
{
    /* the first two fields are byte endian agnostic - they are low to high as shown */
    uint8_t	ncbi[4];                /* 'N', 'C', 'B', 'I' */
    uint8_t	sra[4];                 /* '.', 's', 'r', 'a' */
    /* this field lets you determine the byte order of the file */
    SraEndian_t	byte_order;
    /* not a ver_t - this is a 4 byte integer with no subfields */
    uint32_t	version;
    union 
    {
        struct
        {
            /* the only header field in the first version is the data offset */
            uint64_t	file_offset;
        } v1;
    } u;
};

/* ======================================================================
 * Header format structures
 *
 */



#define FS_SRA_CUR_VERSION 1
KFS_EXTERN size_t CC SraHeaderSize ( const KSraHeader * self );

KFS_EXTERN uint64_t CC SraHeaderGetFileOffset (const KSraHeader * self);

/*--------------------------------------------------------------------------
 * forwards
 */
struct KFile;
struct KDirectory;
struct KToc;

/*--------------------------------------------------------------------------
 * KDirectory
 *  SRA-specific extensions
 */

/* KArcParseSRA
 *	A parse function for sra files suitable for KDirectoryOpenArcDirRead
 *
 * [INOUT] KArcTOC*	self	The TOC (table of contents) to fill in with the parse
 * [IN]    const KFile*	kfile	The opened (as KFile) archive to parse.
 */
KFS_EXTERN rc_t CC KArcParseSRA ( struct KToc * self,
                                  const void * kvoid,
                                  bool (CC*ignored) (const struct KDirectory*,  const char*, void*),
                                  void *ignored_data);
KFS_EXTERN rc_t CC KArcParseSRAUnbounded ( struct KToc * self,
                                           const void * kvoid,
                                           bool (CC*ignored) (const struct KDirectory*,  const char*, void*),
                                           void *ignored_data);

KFS_EXTERN rc_t CC KArcParseKDir( struct KToc * self,
                                  const void * kdir,
                                  bool(CC*f)(const struct KDirectory*,const char *, void*),
                                  void *filterparam);


/* KDirectoryOpenSraArchive
 *  opens a named SRA archive
 *
 *  "sra_dir" [ OUT ] - return parameter for archive
 *
 *  "chroot" [ IN ] - if non-zero, the new directory becomes
 *  chroot'd and will interpret paths beginning with '/'
 *  relative to itself.
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting sraget archive
 */
KFS_EXTERN rc_t CC KDirectoryOpenSraArchiveRead ( struct KDirectory const *self,
    struct KDirectory const **sra_dir, int chroot, const char *path, ... );
KFS_EXTERN rc_t CC KDirectoryVOpenSraArchiveRead ( struct KDirectory const *self,
    struct KDirectory const **sra_dir, int chroot, const char *path, va_list args );
KFS_EXTERN rc_t CC KDirectoryOpenSraArchiveRead_silent ( struct KDirectory const *self,
    struct KDirectory const **sra_dir, int chroot, const char *path, ... );
KFS_EXTERN rc_t CC KDirectoryVOpenSraArchiveRead_silent ( struct KDirectory const *self,
    struct KDirectory const **sra_dir, int chroot, const char *path, va_list args );
KFS_EXTERN rc_t CC KDirectoryOpenSraArchiveRead_silent_preopened ( struct KDirectory const *self,
    struct KDirectory const **sra_dir, int chroot, const struct KFile * f, const char *path, ... );
KFS_EXTERN rc_t CC KDirectoryVOpenSraArchiveRead_silent_preopened ( struct KDirectory const *self,
    struct KDirectory const **sra_dir, int chroot, const struct KFile * f, const char *path, va_list args );
KFS_EXTERN rc_t CC KDirectoryOpenSraArchiveReadUnbounded_silent_preopened ( struct KDirectory const *self,
    struct KDirectory const **sra_dir, int chroot, const struct KFile * f, const char *path, ... );
KFS_EXTERN rc_t CC KDirectoryVOpenSraArchiveReadUnbounded_silent_preopened ( struct KDirectory const *self,
    struct KDirectory const **sra_dir, int chroot, const struct KFile * f, const char *path, va_list args );
/* copycat needs an unbounded version - that is filesize is not necessarily known
 * this is unsafe for most other uses */
KFS_EXTERN rc_t CC KDirectoryOpenSraArchiveReadUnbounded ( struct KDirectory const *self,
    struct KDirectory const **sra_dir, int chroot, const char *path, ... );
KFS_EXTERN rc_t CC KDirectoryVOpenSraArchiveReadUnbounded ( struct KDirectory const *self,
    struct KDirectory const **sra_dir, int chroot, const char *path, va_list args );

KFS_EXTERN rc_t CC KFileIsSRA (const char * b, size_t z);

#ifdef __cplusplus
}
#endif

#endif /* _h_kfs_sra_ */
