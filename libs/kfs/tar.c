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
#define HANDLING_EXTENDED_HEADERS 0

#include <kfs/extern.h>
#include <klib/defs.h>
#include <klib/rc.h>
#include <kfs/file.h>
#include <kfs/mmap.h>
#include <kfs/arc.h>
#include <kfs/toc.h>
#include <kfs/tar.h>
#include <klib/log.h>
#include <klib/debug.h>

#include "toc-priv.h"
#include <os-native.h>
#include <sysalloc.h>
#include <strtol.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h> /* temporary for development */

#include <limits.h>
#include <sys/types.h>

#ifdef _DEBUGGING
#define TAR_FUNC_ENTRY() DBGMSG (DBG_KFS, DBG_FLAG(DBG_KFS_TARENTRY), ("Enter: %s\n", __func__))
#define TAR_DEBUG(msg) DBGMSG (DBG_KFS, DBG_FLAG(DBG_KFS_TAR), msg)
#else
#define TAR_FUNC_ENTRY()
#define TAR_DEBUG(msg)
#endif

/* -----
 * offset of is the count of bytes between the base of a structure and 
 * a particular member of that structure
 */
#ifndef OFFSET_OF
#define OFFSET_OF(structure,member) ((size_t)((&(((structure*)0)->member))-(0)))
#endif

/* -----
 * Hide any definition of sun that might have come from sun compilers
 * or the like
 */
#undef sun
#undef SUN


#define PASTE_2(a,b)   a##b
#define PASTE_3(a,b,c) a##b##c
#define STRINGIFY(a)   #a


/* ======================================================================
 * return an ASCII string that describes a header type
 *
 * Needs to match tar_header_type_enum.
 */
#if _DEBUGGING
static const char * get_bool_string (bool b)
{
    /* this curiosity comes from an embedded programmer who worried that
       while false is generally considered 0b00000000, true is usually
       considered 0b00000001 but sometimes 0b11111111, e.g MC68000. */
    switch ( ( int ) b )
    {
    case true:
    return "true";
    case false:
    return "false";
    default:
    return "not-false";
    }
}
#endif


/* ======================================================================
 * Header format structures
 *
 * Tar (tape archiver) started possibly as a unix utility in BSD rivalling
 * the cpio from the AT&T System III.  The exact derivation is unimportant
 * and some names might be slightly misleading based on historical 
 * inaccuracies but successful implmentation is not dependant on such 
 * accuracy in historical trivia but rather on accuracy in technical details.
 *
 * Posix attempted to standardize the growingly divergent variants of tar
 * but it has led to only slightly more standardized variants with vaguely
 * compatible extensions.
 *
 * For the purposes of this implmentation we will refer to the known to the
 * author versions of tar and include the tar like aspects of the posix pax 
 * replacement for tar.
 *
 * Cpio support could be added if desired with only some difficulty.
 *
 * Supported known variants for this implmentation will be:
 *  V7  - the oldest known common base defitions for a tar header
 *         block ( possibly from Unix V7?)
 *      POSIX   - Posix.1-1988 initial restandardization of a header block
 *        This versio introduced the ustar name for a tar header
 *        and includes that term as a "magic" constant.
 *  PAX - Posix.1.2001 headers for pax a tar derivative that us a
 *        peaceful attempt to unify the tar and cpio formats apparently.
 *        It isn't different than POSIX in the ustar block but instead
 *        is an introduction of two new values of a link field in the
 *        tar header that defines what comes next (see headers defined
 *        below).
 *  SUN - an extension to the POSIX tar header format from SunOS 5
 *  STAR 85 - pre-POSIX extensions to tar from Heorg Schilling (ask him its
 *        the bestest most greatest tar until STAR94)
 *  STAR 94 - A redo of star based on the POSIX ustar tar header.  A less
 *        broken than most implmentation of a POSIX/ustar tar header
 *        based tar. Schilling says its the only real implmentation of
 *        a ustar based tar but it isn't fully compliant by design.
 *  GNU 89  - a selected variant of tar from FSF/GNU that is a broken 
 *        implementation of a POSIX/ustar header based tar.
 *  GNU 01  - A slight redo of the FSF/GNU tar format.  There are actually
 *        evolving variants all of which are still somewhat broken
 *        implementations of a ustar based tar header format.
 *
 * Along with these variants of a tar/ustar semi-standard tar header there are
 * other header blocks and other significant blocks that are also tracked in
 * this implmentation of a tar reader.
 *  ZERO BLOCK - a block of 512 zero bytes that is supposed to be padding at
 *        the end of a tar file to meet some super blocksize.  Based on
 *        where it fits in it would be found when looking for a header
 *        for the next file included in a tar archive.
 *  RAW - a convention to mean a header block of a type that has not been 
 *        determined.
 */
#define TYPES() \
    type_(UNDEFINED) type_(ZERO_BLOCK) type_(CPIO) type_(V7) type_(POSIX) \
        type_(SUN) type_(STAR_85) type_(STAR_94)  type_(GNU_89) type_(SPARSE)


#define type_(e) PASTE_2(TAR_,e),

typedef enum tar_header_type
{
    TYPES()
    TAR_TYPE_COUNT
} tar_header_type;

#undef type_
#define type_(e) STRINGIFY(e),
static const char * get_type_string(tar_header_type t)
{
    static const char * type_error = "Error";
    static const char * type_strings[] = 
        {
            TYPES()
        };
    if ((t < 0) || (t >= TAR_TYPE_COUNT))
    return type_error;
    return type_strings[t];
}
#undef type_
#undef TYPES


/* =============================================================================
 * Tar headers are almost ASCII based but definitely byte/octet based so all
 * elements are best defined as arrays of char and use casts to signed and unsigned
 * where appropriate in interpretation..
 * 
 * All Tar files or streams are divided into blocks of 512 bytes
 * This is significant in the file data in that the last block 
 * of a file is supposed to be padded with NUL to fill out a block
 * and then be followed by two blocks of all NUL bytes.
 * headers are also 512 bytes with various but fairly consistent 
 * interpretations of what is where with in that block
 *
 * Most tar utilities further define super blocks consisting of a number of blocks
 * typically 10 of them for a length of 5120 bytes.  This is irrelevant for this
 * implmentation.  By definition a tar file ends with two "zero blocks" and enough
 * more after that to fill one of these super blocks.  We ignore all aspects of 
 * this.
 */
#define TAR_BLOCK_SIZE      (512)
typedef char tar_raw_block [TAR_BLOCK_SIZE];
#define BLOCKS_FOR_BYTES(byte_count)    ((byte_count+TAR_BLOCK_SIZE-1)/TAR_BLOCK_SIZE)

/* --------------------------------------------------------------------------------
 * lengths of various tar header fields
 */

/* --------------------
 * Tar file names are always 100 bytes long and include preceding 
 * path names.  The utilities do not precluded paths that put the 
 * files outside of the "base" where the tar file was created.
 *
 * To handle tar files made by older tar utilities if the last 
 * character is '/' then the file should be assumed to be a directory.
 *
 * This 'type' is used for both the name of the object being archived
 * and the link target if it is a hard or soft link.
 */
#define TAR_NAME_LEN        (100)
typedef char    tar_file_name   [TAR_NAME_LEN];

/* --------------------
 * Tar mode strings are always 8 bytes long.
 *
 * 9 file access permissions bits and three execution mode bits.
 *
 * Zero '0' not NUL pre-fill unused bytes.  a user permision of 0644 would be stored as
 * "0000644" with a NUL terminator.
 *
 * The format is 7 octal ASCII bytes with only the last 4 being 
 * significant.  That is the first three are always '0'.  The 8th 
 * byte is NUL.
 *
 * Older utilities might put preceding ' ' characters or
 * instead of pre-fill have post fill ' ' or NUL.
 */
#define TAR_MODE_LEN        (8)
typedef char    tar_file_mode   [TAR_MODE_LEN];
/* -----
 * These are the bits if the mode is in binary (octal defines for ease of interpretation)
 * they match the st_mode field from the stat()/fstat() struct stat.
 *
 * GNU tar puts the file type bits from the stat structure in the mode of the tar file.
 * Other tar implmentations might as well though no where is this dktefined as required or
 * recommended or even supported.
 */
#define TAR_SUID_BIT    (04000) /* set UID on execution */
#define TAR_GUID_BIT    (02000) /* set GID on execution */
#define TAR_STICKY_BIT  (01000) /* save text / sticky bit */
/* file permissions */
#define TAR_MODE_OREAD  (00400) /* read by owner */
#define TAR_MODE_OWRITE (00200) /* write by owner */
#define TAR_MODE_OEXEC  (00100) /* execute by owner */
#define TAR_MODE_GREAD  (00040) /* read by group */
#define TAR_MODE_GWRITE (00020) /* write by group */
#define TAR_MODE_GEXEC  (00010) /* execute by group */
#define TAR_MODE_WREAD  (00004) /* read by other */
#define TAR_MODE_WWRITE (00002) /* write by other */
#define TAR_MODE_WEXEC  (00001) /* execute by other */

/* -----
 * These are the bits once converted into ASCII
 * with in an ASCII byte these bits are actually usable so no conversion needed
 * '0' = 0x30 / 060
 * '1' = 0x31 / 061
 * '2' = 0x32 / 062
 * '3' = 0x33 / 063
 * '4' = 0x34 / 064
 * '5' = 0x35 / 065
 * '6' = 0x36 / 066
 * '7' = 0x37 / 067
 */
#define TAR_MODE_READ       (0x01)
#define TAR_MODE_WRITE      (0x02)
#define TAR_MODE_EXEC       (0x04)
#define TAR_MODE_STICKY     (0x01)
#define TAR_MODE_GUID       (0x02)
#define TAR_MODE_SUID       (0x04)
#define TAR_MODE_OWNER_BYTE (6)
#define TAR_MODE_GROUP_BYTE (5)
#define TAR_MODE_WORLD_BYTE (4)
#define TAR_MODE_USER_BYTE  (3)
#define TAR_MODE_EXEC_BYTE  (2)

/* --------------------
 * Tar user (and group) numeric IDs are put into 8 bytes.
 *
 * There is a '0' prefill and as terminating NUL.
 *
 * Older utilities might put preceding ' ' characters or
 * instead of pre-fill have post fill ' ' or NUL.
 */
#define TAR_ID_LEN      (8)
typedef char    tar_id      [TAR_ID_LEN];

/* --------------------
 * Tar file size elements are 12 bytes long with 11 used
 * for octal characters making the maximum size of a file
 * for pure classic or Posix tar limited to 8 GBytes.  
 * Various tar utilities handle longer files in different
 * ways if at all.
 *
 * A length of 100 bytes would be stored as "00000000144".
 *
 * Links and some other special values are archived with a
 * length of zero and thus no data blocks.
 *
 * There is a '0' prefill and as terminating NUL.
 *
 * Older utilities might put preceding ' ' characters or
 * instead of pre-fill have post fill ' ' or NUL.
 *
 * GNU tar implmentations use alternative intrpretatopms of
 * this and possibly other fields using mime base 64 or
 * base 256 (big endian nonstandard sized binary)
 */
#define TAR_SIZE_LEN        (12)
typedef char    tar_size    [TAR_SIZE_LEN];
#define MAX_TAR_FILE_SIZE   (077777777777)

/* --------------------
 * Tar file modification/access/creation times are 12 bytes
 * long.  This holds 11 octal ASCII digits representing the
 * number of seconds since 01/01/1970 00:00 UTC.
 *
 * There is a '0' prefill and as terminating NUL.
 *
 * Older utilities might put preceding ' ' characters or
 * instead of pre-fill have post fill ' ' or NUL.
 */
#define TAR_TIME_LEN        (12)
typedef char    tar_time    [TAR_TIME_LEN];

/* --------------------
 * Tar has a weak checksum protection of part of the tar header
 * that is 8 bytes long and again uses 7 ASCII octal digits.
 *
 * There is a '0' prefill and as terminating NUL.
 *
 * Older utilities might put preceding ' ' characters or
 * instead of pre-fill have post fill ' ' or NUL.
 */
#define TAR_CSUM_LEN        (8)
typedef char    tar_csum    [TAR_CSUM_LEN];
#define csum_blanks     ("       ")

/* --------------------
 * The tar link is a single byte that expresses the type of file
 * or special value represented by this entry.
 */
typedef char    tar_link;

#define LINKS()                   \
    link_('\0',OLDNORMAL_FILE)    \
    link_('0',NORMAL_FILE)        \
    link_('1',HARD_LINK)          \
    link_('2',SYMBOLIC_LINK)      \
    link_('3',CHARACTER_SPECIAL)  \
    link_('4',BLOCK_SPECIAL)      \
    link_('5',DIRECTORY)          \
    link_('6',FIFO)               \
    link_('7',CONTIGUOUS_FILE)    \
    link_('A',SOLARIS_ACL)        \
    link_('D',GNU_DUMPDIR)        \
    link_('E',SOLARIS_ACL_FILE)   \
    link_('I',INODE_METADATA)     \
    link_('K',NEXT_LONG_LINK)     \
    link_('L',NEXT_LONG_NAME)     \
    link_('M',MULTI_VOLUME)       \
    link_('N',GNU1989_LONG_NAMES) \
    link_('S',SPARSE)             \
    link_('V',VOLUME_NAME)        \
    link_('X',SUN_XHDR)           \
    link_('g',PAX_GLOBAL_XHDR)    \
    link_('x',PAX_XHDR)

#define link_(v,n)      PASTE_2(LINK_,n) = v,

/* LINK_COUNT is a count not a mac value */
enum e_tar_link
{
    LINKS()
    LINK_COUNT
};
#undef link_

#if _DEBUGGING
#define link_(v,n) {STRINGIFY(n), v},
struct nv_pair
{
    const char * name;
    tar_link  link;
};
static const char * get_link_string(tar_link l)
{
    static const struct nv_pair pairs[] = 
        {
            LINKS()
            {NULL, 0}
        };
    unsigned int ix;

    for (ix = 0; pairs[ix].name != NULL; ++ix)
        if (pairs[ix].link == l)
            return pairs[ix].name;
    return "UNDEFINED";
}
#undef link_
#endif
#undef LINKS


/* --------------------
 * The tar magic string is 6 bytes long.
 */
#define TAR_MAGIC_LEN       (6)
typedef char    tar_magic   [TAR_MAGIC_LEN];
#define POSIX_MAGIC_CONST   "ustar"     /* includes terminating NUL */

/* --------------------
 * The tar version string is two bytes long and uses both bytes
 */
#define TAR_VERSION_LEN     (2)
typedef char    tar_version [TAR_VERSION_LEN];
#define POSIX_VERSION_CONST "00"        /* does not include terminating NUL */

/* --------------------
 * The Posix strong user/group name is 32 bytes long
 */
#define TAR_STRNAME_LEN     (32)
typedef char    tar_strname [TAR_STRNAME_LEN];

/* --------------------
 * The dev? strings are 8 bytes long
 */
#define TAR_DEV_LEN     (8)
typedef char    tar_dev     [TAR_DEV_LEN];

/* --------------------
 * Posix prefix is 155 bytes that can be put before the name to give a path of
 * 255 bytes instad of the smaller limit of 99.
 */
#define TAR_PREFIX_LEN      (155)
typedef char    tar_prefix  [TAR_PREFIX_LEN];

/*
 * Sun extensions
 */

/* --------------------
 */
typedef char sun_extnum;
#define SUN_FULLSIZE_LEN    (10)
typedef char    sun_fullsize    [SUN_FULLSIZE_LEN];

/* --------------------
 * star extensions
 *
 * star85 is old star from 1985
 */
typedef char    star85_version;
#define STAR85_FILETYPE_LEN (8)
typedef char    star85_filetype [STAR85_FILETYPE_LEN];
#define STAR85_TYPE_LEN     (12)
typedef char    star85_type [STAR85_TYPE_LEN];
#define STAR85_RDEV_LEN     (12)
typedef char    star85_rdev [STAR85_RDEV_LEN];
/* ignoring the 11 byte rdev with minor bits */
#define STAR85_UNAME_LEN    (16)
typedef char    star85_uname    [STAR85_UNAME_LEN];
#define STAR85_GNAME_LEN    (15)
typedef char    star85_gname    [STAR85_GNAME_LEN];
#define STAR_XMAGIC_LEN     (4)
typedef char    star_magic  [STAR_XMAGIC_LEN];
#define STAR_MAGIC_CONST    ("tar")
#define NSTAR_PREFIX_LEN    (1)
typedef char    nstar_prefix    [NSTAR_PREFIX_LEN];

/* --------------------
 * gnu extensions
 */
#define GNU89_MAGIC_LEN     (8)
typedef char    gnu89_magic [GNU89_MAGIC_LEN];
#define GNU_89_MAGIC_CONST  "ustar  "       /* includes terminating NUL */
#define GNU_89_GNUMAGIC_CONST   "GNUtar "       /* includes terminating NUL */
#define GNU89_LONGNAMES_LEN (4)
typedef char    gnu89_longnames [GNU89_LONGNAMES_LEN];


/* --------------------
 * shared between the feuding star and gnu tar
 */
typedef char    tar_isextended ;
typedef struct  tar_sparse
{
    tar_size    offset;
    tar_size    num_bytes;
} tar_sparse;
#define GNU_SPARSES_IN_EXTRA_HEADER     (16)
#define GNU_SPARSES_IN_OLD_HEADER       (4)
#define GNU_SPARSES_IN_SPARSE_HEADER        (21)
#define STAR_SPARSES_IN_HEADER          (4)
#define STAR_SPARSES_IN_EXT_HEADER      (21)


/* ----------------------------------------
 * The various tar header formats
 *
 * Note that in all the tar, posix and pax formats the first 257 bytes are
 * the same.  In all posix ad almost compliant formats the first
 * 345 bytes are the same (except GNU 89).  It is abuse of the 155
 * bytes of the prefix that make star and gnu truly not posix compliant
 * Sun used the 12 bytes after the prefix so is still psox compliant.
 */
typedef struct  tar_v7_header
{
    /* type     member name          and offset */
    tar_file_name   name;               /*   0 */
    tar_file_mode   mode;               /* 100 */
    tar_id      uid;                /* 108 */
    tar_id      gid;                /* 116 */
    tar_size        size;               /* 124 */
    tar_time        mtime;              /* 136 */
    tar_csum        csum;               /* 148 */
    tar_link        link;               /* 156 */
    tar_file_name   linkname;           /* 157 */
    /* end of header                    // 257 */
} tar_v7_header;

typedef struct tar_posix_header
{
    /* type     member name          and offset */
    tar_file_name   name;               /*   0 */
    tar_file_mode   mode;               /* 100 */
    tar_id      uid;                /* 108 */
    tar_id      gid;                /* 116 */
    tar_size        size;               /* 124 */
    tar_time        mtime;              /* 136 */
    tar_csum        csum;               /* 148 */
    tar_link        link;               /* 156 */
    tar_file_name   linkname;           /* 157 */
    tar_magic       magic;              /* 257 */
    tar_version     version;            /* 263 */
    tar_strname     uname;              /* 265 */
    tar_strname     gname;              /* 297 */
    tar_dev     devmajor;           /* 329 */
    tar_dev     devminor;           /* 337 */
    tar_prefix      prefix;             /* 345 */
    /* end of header                    // 500 */
} tar_posix_header, tar_pax_header;

typedef struct tar_sun_header
{
    /* type     member name          and offset */
    tar_file_name   name;               /*   0 */
    tar_file_mode   mode;               /* 100 */
    tar_id      uid;                /* 108 */
    tar_id      gid;                /* 116 */
    tar_size        size;               /* 124 */
    tar_time        mtime;              /* 136 */
    tar_csum        csum;               /* 148 */
    tar_link        link;               /* 156 */
    tar_file_name   linkname;           /* 157 */
    tar_magic       magic;              /* 257 */
    tar_version     version;            /* 263 */
    tar_strname     uname;              /* 265 */
    tar_strname     gname;              /* 297 */
    tar_dev     devmajor;           /* 329 */
    tar_dev     devminor;           /* 337 */
    tar_prefix      prefix;             /* 345 */
    sun_extnum      extnum;             /* 500 non-conformant */
    sun_extnum      extcount;           /* 501 non-conformant */
    sun_fullsize    fullsize;           /* 502 non-conformant */
    /* end of header                    // 512 non-conformant */
} tar_sun_header;

typedef struct tar_star_85_header
{
    /* type     member name          and offset */
    tar_file_name   name;               /*   0 */
    tar_file_mode   mode;               /* 100 */
    tar_id      uid;                /* 108 */
    tar_id      gid;                /* 116 */
    tar_size        size;               /* 124 */
    tar_time        mtime;              /* 136 */
    tar_csum        csum;               /* 148 */
    tar_link        link;               /* 156 */
    tar_file_name   linkname;           /* 157 */
    star85_version  starversion;            /* 257 non-conformant */
    star85_filetype starfiletype;           /* 258 internal type of file? non-conformant */
    star85_type     startype;           /* 266 type of file (UNIX)? non-conformant */
    star85_rdev     rdev;               /* 278 non-conformant */
    tar_time        atime;              /* 290 non-conformant */
    tar_time        ctime;              /* 302 non-conformant */
    star85_uname    uname;              /* 314 non-conformant */
    star85_gname    gname;              /* 330 non-conformant */
    tar_prefix      prefix;             /* 345 non-conformant */
    char        ___fill0[8];            /* 500 non-conformant */
    star_magic      xmagic;             /* 508 non-conformant */
    /* end of header                    // 512 non-conformant */
} tar_star_85_header;
typedef struct tar_star_94_header
{
    /* type     member name          and offset */
    tar_file_name   name;               /*   0 */
    tar_file_mode   mode;               /* 100 */
    tar_id      uid;                /* 108 */
    tar_id      gid;                /* 116 */
    tar_size        size;               /* 124 */
    tar_time        mtime;              /* 136 */
    tar_csum        csum;               /* 148 */
    tar_link        link;               /* 156 */
    tar_file_name   linkname;           /* 157 */
    tar_magic       magic;              /* 257 */
    tar_version     version;            /* 263 */
    tar_strname     uname;              /* 265 */
    tar_strname     gname;              /* 297 */
    tar_dev     devmajor;           /* 329 */
    tar_dev     devminor;           /* 337 */
    nstar_prefix    prefix;             /* 345 */
    char        ___fill0;           /* 346 */
    char        ___fill1[8];            /* 347 */
    tar_isextended  isextended;         /* 355 non-conformant */
    tar_sparse      sparse[STAR_SPARSES_IN_HEADER]; /* 356 non-conformant */
    tar_size        realsize;           /* 452 non-conformant */
    tar_size        offset;             /* 464 non-conformant */
    tar_time        atime;              /* 476 non-conformant */
    tar_time        ctime;              /* 488 non-conformant */
    char        ___fill2[8];            /* 500 */
    star_magic      xmagic;             /* 508 non-conformant */
    /* end of header                    // 512 */
} tar_star_94_header;

typedef struct tar_gnu_89_header
{
    /* type     member name          and offset */
    tar_file_name   name;               /*   0 */
    tar_file_mode   mode;               /* 100 */
    tar_id      uid;                /* 108 */
    tar_id      gid;                /* 116 */
    tar_size        size;               /* 124 */
    tar_time        mtime;              /* 136 */
    tar_csum        csum;               /* 148 */
    tar_link        link;               /* 156 */
    tar_file_name   linkname;           /* 157 */
    gnu89_magic     magic;              /* 257 non-conformant */
    tar_strname     uname;              /* 265 */
    tar_strname     gname;              /* 297 */
    tar_dev     devmajor;           /* 329 */
    tar_dev     devminor;           /* 337 */
    tar_time        atime;              /* 345 non-conformant */
    tar_time        ctime;              /* 357 non-conformant */
    tar_size        offset;             /* 369 non-conformant */
    gnu89_longnames longnames;          /* 381 non-conformant */
    char        ___fill0[1];            /* 385 */
    tar_sparse      sparse  [GNU_SPARSES_IN_OLD_HEADER];/* 386 optional sparse */
    tar_isextended  isextended;         /* 482 non-conformant */
    tar_size        realsize;           /* 483 non-conformant */
    /* end of header                    // 495 */
} tar_gnu_89_header;

typedef struct tar_gnu_99_header
{
    /* type     member name          and offset */
    tar_file_name   name;               /*   0 */
    tar_file_mode   mode;               /* 100 */
    tar_id      uid;                /* 108 */
    tar_id      gid;                /* 116 */
    tar_size        size;               /* 124 */
    tar_time        mtime;              /* 136 */
    tar_csum        csum;               /* 148 */
    tar_link        link;               /* 156 */
    tar_file_name   linkname;           /* 157 */
    gnu89_magic     magic;              /* 257 non-conformant */
    tar_strname     uname;              /* 265 */
    tar_strname     gname;              /* 297 */
    tar_dev     devmajor;           /* 329 */
    tar_dev     devminor;           /* 337 */
    tar_time        atime;              /* 345 non-conformant */
    tar_time        ctime;              /* 357 non-conformant */
    tar_size        offset;             /* 369 non-conformant */
    gnu89_longnames longnames;          /* 381 non-conformant */
    char        ___fill0[1];            /* 385 */
    tar_sparse      sparse  [GNU_SPARSES_IN_OLD_HEADER];/* 386 non-conformant optional sparse */
    tar_isextended  isextended;         /* 482 non-conformant */
    tar_size        realsize;           /* 483 non-conformant */
    /* end of header                    // 495 */
} tar_gnu_99_header;

typedef struct tar_sparse_header
{
    /* type     member name          and offset */
    tar_sparse      sparse  [STAR_SPARSES_IN_EXT_HEADER];/* 0 */
    tar_isextended  isextended;         /* 504 */
} tar_sparse_header;                    /* 505 end of header */

typedef struct tar_cpio_header
{
    /* type     member name          and offset */
    char        magic       [6];        /*   0  must be "070707" */
#define TAR_CPIO_MAGIC_CONST ("070707")
    char        dev     [6];        /*   6  (dev,ino) is unique for each file in archive */
    char        ino     [6];        /*  12  see dev */
    char        mode        [6];        /*  18 */
    char        uid     [6];        /*  24 */
    char        gif     [6];        /*  30 */
    char        nlink       [6];        /*  36 */
    char        rdev        [6];        /*  42 */
    char        mtime       [11];       /*  48 */
    char        namesize    [6];        /*  59 */
    char        filesize    [6];        /*  65 */
    char        buff        [1];        /*  71  name and file data */
} tar_cpio_header;

typedef union tar_header
{
    /* ----------
     * The TAR header is to be zero filled by definition but some tar
     * programs use ' ' instead.
     *
     * accept zeros ('0'), spaces (' ') or NULs (0x00) as equivalent where reasonable
     *
     * This name refers to the entire header as a single undifferentiated
     * sequence of bytes.
     */
    tar_raw_block   raw;        /* tar block as an array of bytes: used for 0 blocks here */
    tar_cpio_header cpio;       /* cpio not really tar */
    tar_v7_header   tar;        /* classic header back to the beginnings of memory */
    tar_posix_header    posix;      /* ustar or Posix 1003.1 header */
    tar_sun_header  suntar;     /* Sun Microsystems tar header */
    tar_star_85_header  star_85;    /* star header from 1985 (pre-Posix) */
    tar_star_94_header  star_94;    /* star header from 1994 (post-Posix) */
    tar_gnu_89_header   gnu_89;     /* gnu header from circa 1989 (post-posix but broken) */
    tar_gnu_99_header   gnu_99;         /* gnu header from ???? */
    tar_sparse_header   sparse;     /* star / gnu extended sparses header */
} tar_header;


/* ======================================================================
 * local module-wide function like macros
 */
/* -----
 * offset of is the count of bytes between the base of a structure and 
 * a particular member of that structure
 */
#define OFFSET_OF(structure,member) ((size_t)((&(((structure*)0)->member))-(0)))


/* ======================================================================
 * compilation unit local functions
 */

/* ======================================================================
 * return the RFC 2045 base 64 value for a byte character 
 * -1 for any out of range
 *
 * base 64 uses A-Z as 0-25, a-z as 26-51, 0-9 as 52-61, + as 62 and / as 63
 */
static int64_t decode_base64char (uint8_t byte)
{
    /* 
     * trade off of space for table versus time to upsize the return
     * and reupsize where it is used
     */
    static const int8_t table[] = 
    {
        /*
         * 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F
         */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, /* 00 */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, /* 10 */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63, /* 20 */
        52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1, /* 30 */
        -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14, /* 40 */
        15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1, /* 50 */
        -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40, /* 60 */
        41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1, /* 70 */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, /* 80 */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, /* 90 */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, /* A0 */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, /* B0 */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, /* C0 */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, /* D0 */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, /* E0 */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1  /* F0 */
    };
    return table[byte];
}


/* ======================================================================
 * return the RFC 2045 base 64 value of a string at <str> of length <len>
 *
 * Interpreted the specification to mean that you simply ignore any out
 * of range characters.  They are not treated as bytes of 0 as that would
 * mean a shift of earlier bytes.
 *
 * Each character is 6 bits of the final value.
 */
static int64_t decode_base64string (const uint8_t* str, size_t len)
{
    int64_t     result = 0;
    int64_t     temp = 0;
    size_t  ix;

    for (ix = 0; ix < len; ++ix)
    {
    if ((temp = decode_base64char(str[ix])) >= 0)
    {
        result <<= 6; /* result *= 64 */
        result += temp;
    }
    }
    return result;
}


/* ======================================================================
 * tar_strtoll
 *
 * This function will convert a string in a tar header into a 64 signed
 * integer.
 *
 * The original tar header used just octal numbers in ascii in fixed length
 * fields.  As normal in the computer software world these "obviously plenty
 * big" fields became way too small.
 *
 * GNU tar invented two approaches to make these numeric fields "bigger" but
 * only prolonged the pain by squeezing the new numbers into the same fields.
 *
 * In the tar v7 and ustar based tar headers the fields are:
 *  Name    Size    Octal Range Interpretation
 *  mode    8   0-2097151   bit flags
 *  uid 8   0-2097151
 *  gid 8   0-2097151
 *  size    12  0-8589934591    up to 8 Giga-Byte files
 *  mtime   12  0-8589934591    1970/01/01 00:00:00 GMT-2242/03/16 12:56:31 GMT
 *  
 *  atime   12  0-8589934591    1970/01/01 00:00:00 GMT-2242/03/16 12:56:31 GMT
 *  ctime   12  0-8589934591    1970/01/01 00:00:00 GMT-2242/03/16 12:56:31 GMT
 *
 * GNU's first and already obsolete approach was to put Mime base-64 numbers
 * With these the first byte is '+' or '-' to signal it isn't octal ASCII.
 *
 * GNU's second approach is base-256 which is a big endian binary string of lengths other
 * than the 1, 2, 4 or 8 bytes of the standard integer types.  In this approach the first
 * byte is either 0x80 for a positive number or 0xFF for a negative number.
 */

static int64_t tar_strtoll ( const uint8_t * str, size_t len, bool silent )
{
    int64_t result = 0;
    bool negative = false;
    uint8_t temp_buff[24];  /* long enough to hold all octal bytes for 64 bit numbers */

    /* -----
     * force a NUL in case the source doesn't have one; this is for
     * strtoll() on ASCII Octal
     */
    if ( len >= sizeof temp_buff )
        len = sizeof temp_buff - 1;
    memmove (temp_buff, str, len);
    temp_buff[ len ] = 0x00;

    /* -----
     * Most are going to be simple ASCII octal using '0'-'7' with NUL terminator
     * leading 0 is not required but of course is accepted to match tar 
     * header specifications
     */
    if (((temp_buff[0] >= '0')&&(temp_buff[0] <= '7'))||(temp_buff[0] == ' '))
    {
        /* use stdlib strtoll - longest expected goes a few bits into the upper longword */
        result = strtoi64((char*)temp_buff,NULL,8);
    }
    /* -----
     * "base-256" well that is "binary" big endian of some length
     *
     * For fields longer than eight bytes upper bytes will shift out of 
     * significance into the bit bucket.
     */
    else if ((temp_buff[0] == 0x80)||(temp_buff[0] == 0xFF)) /* from GNU tar */
    {
        unsigned int ix;
        negative = (bool)((temp_buff[0] == 0xFF) ? true : false);
        result = temp_buff[0] & 0x7F; /* toss first flag bit */
        for (ix = 1; ix < len; ++ix)
        {
            result <<= 8; /* result *= 256; */
            result += temp_buff[ix];
        }
        if (negative)
            result = -result;
    }
    /* -----
     * "base-64" an already dumped idea from GNU tar
     */
    else if ((temp_buff[0] == '+')||(temp_buff[0] == '-')) /* from GNU tar */
    {
        int64_t temp;
        negative = (bool)((temp_buff[0] == '-') ? true : false);
        temp = decode_base64string(temp_buff+1,len-1);
        /* potential overflow */
        result = negative ? -temp : temp;
    }

    /* -----
     * look for an empty field of all NUL
     */
    else if (temp_buff[0] == 0x00)
    {
        unsigned int ix;
        for (ix = 1; ix < len; ++ix)
        {
            if (temp_buff[ix] != 0x00)
                goto fail;
        }
    }
    /* -----
     * no idea what it is then
     */
    else
    {
    fail:
        result = 0; /* as good a guess as any */
        TAR_DEBUG (("%s: unknown integer storage type %c%c%c%c%c%c%c%c\n",
                    temp_buff[0],temp_buff[1],temp_buff[2],temp_buff[3],
                    temp_buff[4],temp_buff[5],temp_buff[6],temp_buff[7]));
        if ( ! silent ) {
            PLOGMSG (klogErr, (klogErr, "unknown integer storage type "
                                     "$(B0)$(B1)$(B2)$(B3)$(B4)$(B5)$(B6)$(B7)",
                           "B0=%c,B1=%c,B2=%c,B3=%c,B4=%c,B5=%c,B6=%c,B7=%c",
                           temp_buff[0],temp_buff[1],temp_buff[2],temp_buff[3],
                           temp_buff[4],temp_buff[5],temp_buff[6],temp_buff[7]));
        }
    }
    return result;
}

/* ======================================================================
 * tar_header_type
 * determine most probable tar header block type
 */
static tar_header_type  what_header_type(const tar_header* header)
{
    /* -----
     * we'll assume its bad until we find a better guess
     */
    tar_header_type type = TAR_UNDEFINED;

    /* -----
     * look for a cpio header though we aren't expecting to support it quite yet
     */
    if (strncmp(header->cpio.magic, TAR_CPIO_MAGIC_CONST, sizeof(header->cpio.magic)) == 0)
    {
    type = TAR_CPIO;
    }
    /* -----
     * look for extended headers of some type
     *
     * look for posix based extensions as hopefully most likely
     */
    else if (strcmp(header->posix.magic, POSIX_MAGIC_CONST) == 0)
    {
    /* -----
     * we have a post posix standard tar header but we aren't done yet
     *
     * First we look for star extensions to the header
     */
    if (strcmp(header->star_94.xmagic, STAR_MAGIC_CONST) == 0)
    {

        /* -----
         * we have either an old or new star archive
         */
        if (strcmp(header->star_94.magic, POSIX_MAGIC_CONST) == 0)
        {
        /* -----
         * we have a new star type archive
         */
        type = TAR_STAR_94;
        }
        else
        {
        /* -----
         * we have a old star type archive
         */
        type = TAR_STAR_85;
        }
    }
    /* -----
     * Next we look for SunOS5 extensions to the header
     */
    else if (header->suntar.extnum &&
         header->suntar.extcount &&
         header->suntar.fullsize[0])
    {
        type = TAR_SUN;
    }
    /* -----
     * Else it seems to be the generic Posix tar header
     */
    else
    {
        type = TAR_POSIX;
    }
    }
    /* -----
     * Not a proper magic for a POSIX ustar header so look for the improper magic of GNU's tar
     */
    else if (strcmp(header->star_94.magic, GNU_89_MAGIC_CONST) == 0)
    {
    /* we have an old GNU not really posix compliant archive type */
    type = TAR_GNU_89;
    }
    /* -----
     * If there is anything else there we don't know what to do with it
     */
    else if (header->posix.magic[0] != 0)
    {
    /* -----
     * Log an anomaly showing what we found in the magic field
     */
    char temp_str   [9];
    memset(temp_str,0,sizeof(temp_str));
    string_copy(temp_str, sizeof(temp_str), header->posix.magic, 8);
        TAR_DEBUG(("%s: unknown header type magic [%s]\n",
                   __func__, temp_str));
    type = TAR_UNDEFINED;
    }
    /* -----
     * not knowing what we have lets see if it is a block of all zeroes
     * knowing we can ignore it
     */
    else
    {
    /* -----
     * If this were a legitimate V7 (well just old style)
     * tar header the first character would be non-NUL
     * and the link would be on eof the old types
     */
    if ((header->raw[0]>= ' ')&&(header->raw[0] <= '~'))
    {
        switch (header->tar.link)
        {
        case LINK_OLDNORMAL_FILE:
        case LINK_NORMAL_FILE:
        case LINK_HARD_LINK:
        case LINK_SYMBOLIC_LINK:
        case LINK_CHARACTER_SPECIAL:
        case LINK_BLOCK_SPECIAL:
        case LINK_DIRECTORY:
        case LINK_FIFO:
        case LINK_CONTIGUOUS_FILE:
        /* GNU TAR will do this to us */
        case LINK_NEXT_LONG_LINK:
        case LINK_NEXT_LONG_NAME:
        case LINK_GNU1989_LONG_NAMES:
        type = TAR_V7;
        break;
        default:
        break;
        }
    }
    else
    {
        /* -----
         * look for anything not zero
         */
        unsigned int ix;

        for (ix = 0; ix < sizeof(tar_header); ++ix)
        {
        if (header->raw[ix])
        {
            /* -----
             * non-zero so quit looking
             */
            break;
        }
        }
        /* -----
         * if we got to the end we know they are all zero so say so
         */
        if (ix == sizeof(tar_header))
        {
        type = TAR_ZERO_BLOCK;
        }
    }
    }
    TAR_DEBUG (("%s: %s(%d)\n", __func__, get_type_string(type), type));
    return type;
}


typedef struct sparse_data
{
    struct sparse_data* next;
    uint64_t        offset;
    uint64_t        size;
} sparse_data;
/* ======================================================================
 */

static rc_t sparse_data_make (sparse_data ** new_item, uint64_t offset, uint64_t size)
{
    sparse_data * p;

    p = malloc (sizeof (sparse_data));
    if (p != NULL)
    {
        p->offset = offset;
        p->size = size;
        p->next = NULL;
        *new_item = p;
        return 0;
    }
    return -1;
}

static rc_t sparse_data_push(sparse_data ** q, uint64_t offset, uint64_t size)
{
    if (q == NULL)
        return -1;
    if (*q == NULL)
        return sparse_data_make (q, offset, size);
    return sparse_data_push (&((*q)->next), offset, size);
}

static rc_t sparse_data_pop (sparse_data ** q, sparse_data **item)
{
    if ((q == NULL) || (item == NULL))
        return -1;

    *item = *q;
    *q = (*item)->next;
    (*item)->next = NULL;
    return 0;
}

static rc_t sparse_data_kill (sparse_data ** q)
{
    if (q == NULL)
        return -1;
    if (*q == NULL)
        return 0;
    if ((*q)->next != NULL)
        return sparse_data_kill(&(*q)->next);

    free (*q);
    *q = NULL;
    return 0;
}

/* ======================================================================
 * local module-wide variables (reduction in parameter pushing)
 *
 * Module shared variables; kinda like C++ class elements
 */
typedef struct KTarState
{
    /* TODO: optimize chunk/sparse lists by counting as pushed */
    const KFile *   kfile;      /* KFS reference to the specific file being parsed */
    const KMMap *   kmmap;      /* KFS memory mapping for a portion of that file */
    const void *    map;        /* where the tar file got put by mmap */
    sparse_data *   sparse_q;
    KTocChunk *     chunks;     /* table of chunks: logical_position, source_position, size */
    KToc *      toc;        /* the Table of Contents we are building */
    size_t      tar_length; /* how long is the tar file */
    size_t      buffer_length;  /* how long is the window into the buffer */
    uint64_t        buffer_start;   /* how far into the tar file is the buffer start */
    uint64_t        buffer_limit;   /* how far into the tar file is the buffer end */
    uint32_t        num_chunks;
    /* -----
     * zero blocks are only supposed to pad out the end of a tar file to
     * match a larger block size.  If we find something after a zero block
     * it is a file error of some type
     */
    bool        found_zero_block;
    bool        found_second_zero_block;
} KTarState;

static rc_t make_chunk_list(KTarState * self, uint64_t file_offset)
{
    uint64_t        count = self->num_chunks;
    uint64_t        source_position = file_offset;
    uint64_t        ix;
    sparse_data *   psd;
    rc_t        ret;

    if (self->chunks)
    free (self->chunks); /* shouldn't happen */
    self->chunks = malloc( (size_t)( count * sizeof(KTocChunk) ) );
    if (self->chunks == NULL)
    return -1;


    for (ix = 0; ix < count; ++ix)
    {
    ret = sparse_data_pop (&self->sparse_q, &psd);
    if (ret != 0)
        return ret;
    self->chunks[ix].logical_position = psd->offset;
    self->chunks[ix].source_position = source_position;
    self->chunks[ix].size = psd->size;
    source_position += psd->size;
    sparse_data_kill(&psd);
    }
    return 0;
}

static void whack_chunk_list(KTarState * self)
{
    free (self->chunks);
    self->chunks = NULL;
    self->num_chunks = 0;
}

#if HANDLING_EXTENDED_HEADERS
/* ======================================================================
 * mini class for handling pax/posix/ustar 
 * extended headers and global extended headers
 *
 * Many of the values are included to get past range limits imposed by the ustar format
 * particular for string length and charcter set or shortish integral values.  <ekyword>
 * and <value> below are UTF-8.
 *
 * values in the header are string values written as with a 'printf' using the form
 *  printf("%d %s=%s\n",<length>,<keyword>,<value>)
 *
 * <length> is described ambiguously as 
 *  "The <length> field shall be the decimal length of the extended header record in octets,
 *   including the trailing <newline>."
 * So does that include the length of <length>? or not?
 *
 * The field <keyword> is allowed in a pax Extended Header include but are not limited to
 *  atime       time_t but with fractional seconds maybe
 *  charset     enumeration list
 *  comment     human readable comments
 *  gid     integer gid allowing numbers greater than ustar limit of 2097151 (07777777)
 *  gname       over rides xhdr gid and ustar gname and gid
 *  linkpath
 *  mtime       time_t but with franctional seconds maybe
 *  path
 *  realtime.<ANY>
 *  security.<ANY>
 *  size
 *  uid     integer uid allowing numbers greater than ustar limit of 2097151 (07777777)
 *  uname
 * Any other keywords desired can be included but might not be meaningful to many applications.
 * Keyword can have pretty much any character in it except '='.
 *
 * <value> is a UTF-8 string that ends with the '\n'.
 * 
 *
 * charset is limited to (omit the quotation marks)
 *      <value>           Formal Standard
 * "ISO-IR 646 1990"        ISO/IEC 646:1990
 * "ISO-IR 8859 1 1998"     ISO/IEC 8859-1:1998
 * "ISO-IR 8859 2 1999"     ISO/IEC 8859-2:1999
 * "ISO-IR 8859 3 1999"     ISO/IEC 8859-3:1999
 * "ISO-IR 8859 4 1998"     ISO/IEC 8859-4:1998
 * "ISO-IR 8859 5 1999"     ISO/IEC 8859-5:1999
 * "ISO-IR 8859 6 1999"     ISO/IEC 8859-6:1999
 * "ISO-IR 8859 7 1987"     ISO/IEC 8859-7:1987
 * "ISO-IR 8859 8 1999"     ISO/IEC 8859-8:1999
 * "ISO-IR 8859 9 1999"     ISO/IEC 8859-9:1999
 * "ISO-IR 8859 10 1998"    ISO/IEC 8859-10:1998
 * "ISO-IR 8859 13 1998"    ISO/IEC 8859-13:1998
 * "ISO-IR 8859 14 1998"    ISO/IEC 8859-14:1998
 * "ISO-IR 8859 15 1999"    ISO/IEC 8859-15:1999
 * "ISO-IR 10646 2000"      ISO/IEC 10646:2000
 * "ISO-IR 10646 2000 UTF-8"    ISO/IEC 10646, UTF-8 encoding
 * "BINARY"         None.
 */
typedef enum pax_charset
{
    PAX_CS_NOT_SPECIFIED,
    PAX_CS_ISO_IR_646_1990,
    PAX_CS_ISO_IR_8859_1_1998,
    PAX_CS_ISO_IR_8859_2_1999,
    PAX_CS_ISO_IR_8859_3_1999,
    PAX_CS_ISO_IR_8859_4_1998,
    PAX_CS_ISO_IR_8859_5_1999,
    PAX_CS_ISO_IR_8859_6_1999,
    PAX_CS_PAX_CS_ISO_IR_8859_7_1997,
    PAX_CS_ISO_IR_8859_8_1999,
    PAX_CS_ISO_IR_8859_9_1999,
    PAX_CS_ISO_IR_8859_10_1998,
    PAX_CS_ISO_IR_8859_13_1998,
    PAX_CS_ISO_IR_8859_14_1998,
    PAX_CS_ISO_IR_8859_15_1998,
    PAX_CS_ISO_IR_1064_2000,
    PAX_CS_ISO_IR_1064_2000_UTF_8,
    PAX_CS_BINARY
} pax_charset;

static const char * pax_charset_strings[] = 
{
    "Not Specified",
    "ISO-IR 646 1990",
    "ISO-IR 8859 1 1998",
    "ISO-IR 8859 2 1999",
    "ISO-IR 8859 3 1999",
    "ISO-IR 8859 4 1998",
    "ISO-IR 8859 5 1999",
    "ISO-IR 8859 6 1999",
    "ISO-IR 8859 7 1987",
    "ISO-IR 8859 8 1999",
    "ISO-IR 8859 9 1999",
    "ISO-IR 8859 10 1998",
    "ISO-IR 8859 13 1998",
    "ISO-IR 8859 14 1998",
    "ISO-IR 8859 15 1999",
    "ISO-IR 10646 2000",
    "ISO-IR 10646 2000 UTF-8",
    "BINARY",
    NULL
};

static pax_charset pax_xhdr_parse_charset_string(char*string)
{
    int ix;     /* index */
    const char * ps;    /* pointer to string */

    for (ps = pax_charset_strings[ix= 0]; ps; ps = pax_charset_strings[++ix])
    if (strcmp(ps,string) == 0)
        return ix;
    return PAX_CS_NOT_SPECIFIED;    /* matched none so set it to not specified */
}

typedef struct pax_xheader pax_xheader;
struct pax_xheader
{
    char *  path;       /* supercedes .posix.name */
    char *  linkpath;   /* supercedes .posix.linkname */
    char *  uname;      /* user name supercedes .posix.uname uid and .posix.uid */
    char *  gname;      /* group name supercedes .posix.gname gid and .posix.gid */
    time_t  atime;      /* supercedes .posix.atime */
    time_t  mtime;      /* supercedes .posix.mtime */
    uid_t   uid;        /* supercedes .posix.uid */
    uint32_t    gid;        /* supercedes .posix.gname */
    uint64_t    size;       /* supercedes .posix.size */
    pax_charset charset;    /* not supported by us? */
};

static pax_xheader *    pax_xhdr_create (void)
{
    pax_xheader * pxv = malloc (sizeof(pax_xheader));
    memset (pxv,0,sizeof(pax_xheader));
    return pxv;
}

static void pax_xhdr_delete(pax_xheader * self)
{
    if (self->path)
    free (self->path);
    if (self->linkpath)
    free (self->linkpath);
    if (self->uname)
    free (self->uname);
    if (self->gname)
    free (self->gname);
    free (self);
}

static rc_t  pax_xhdr_set_general_string (char ** str, const char * val)
{
    size_t  len;    /* how much memory */
    char      * nl; /* point to any new line in source */
    rc_t    rc = 0; /* return code; assume success */

    if (*str)       /* if already set, free the old value */
    free (*str);
    nl = strchr (val, '\n');
    if (nl)
    {
    len = nl - val + 1; /* +1 for NUL */
    }
    else
    {
        size_t size;
        len = string_measure(val, &size) + 1;
    }
    *str = malloc (len);
    if (*str)
    {
        string_copy(*str, len, val, len-1);
        (*str)[len-1] = '\0'; /* if it was a '\n' terminated value this is needed not worth the check if needed */
    }
    else
    {
    rc = RC(rcFS/*?*/,rcAllocating,0/*?*/,rcNoObj/*?*/,rcNull); /* well its not 0 */
    }
    return rc;
}

/* ----------
 * new_path points to a string that ends with either a '\n' or a NUL
 */
LIB_EXPORT rc_t CC pax_xhdr_set_path(pax_xheader * self, char * new_path)
{
    return pax_xhdr_set_general_string(&(self->path),new_path);
}

/* ----------
 * new_path points to a string that ends with either a '\n' or a NUL
 */
LIB_EXPORT rc_t CC pax_xhdr_set_linkpath(pax_xheader * self, char * new_path)
{
    return pax_xhdr_set_general_string(&(self->linkpath),new_path);
}

/* ----------
 * new_name points to a string that ends with either a '\n' or a NUL
 */
LIB_EXPORT rc_t CC pax_xhdr_set_uname(pax_xheader * self, char * new_name)
{
    return pax_xhdr_set_general_string(&(self->uname),new_name);
}

/* ----------
 * new_name points to a string that ends with either a '\n' or a NUL
 */
LIB_EXPORT rc_t CC pax_xhdr_set_gname(pax_xheader * self, char * new_name)
{
    return pax_xhdr_set_general_string(&(self->gname),new_name);
}

LIB_EXPORT bool CC pax_xhdr_get_general_string (char**src, char**dst, size_t max)
{
    size_t size;
    if (string_measure(*src, &size) > max-1)    /* fail if too big for target */
    {
        return false;
    }
    string_copy(*dst, max, *src, size);
    return true;
}

LIB_EXPORT rc_t CC pax_xhdr_get_path(pax_xheader * self, char ** path, size_t max)
{
    return (pax_xhdr_get_general_string(&self->path,path,max))? 0 : ~0;
}

LIB_EXPORT rc_t CC pax_xhdr_get_linkpath(pax_xheader * self, char ** path, size_t max)
{
    return (pax_xhdr_get_general_string(&self->linkpath,path,max))? 0 : ~0;
}

LIB_EXPORT rc_t CC pax_xhdr_get_uname(pax_xheader * self, char ** name, size_t max)
{
    return (pax_xhdr_get_general_string(&self->uname,name,max))? 0 : ~0;
}

LIB_EXPORT rc_t CC pax_xhdr_get_gname(pax_xheader * self, char ** name, size_t max)
{
    return (pax_xhdr_get_general_string(&self->gname,name,max))? 0 : ~0;
}

/* ----------
 * The mtime and atime <value> is defined as
 *  The pax utility shall write an mtime record for each file in write
 *  or copy modes if the file's modification time cannot be represented
 *  exactly in the ustar header logical record described in ustar
 *  Interchange Format. This can occur if the time is out of ustar range,
 *  or if the file system of the underlying implementation supports
 *  non-integer time granularities and the time is not an integer. All of
 *  these time records shall be formatted as a decimal representation of
 *  the time in seconds since the Epoch. If a period ( '.' ) decimal
 *  point character is present, the digits to the right of the point shall
 *  represent the units of a subsecond timing granularity, where the first
 *  digit is tenths of a second and each subsequent digit is a tenth of
 *  the previous digit. In read or copy mode, the pax utility shall
 *  truncate the time of a file to the greatest value that is not greater
 *  than the input header file time. In write or copy mode, the pax
 *  utility shall output a time exactly if it can be represented exactly
 *  as a decimal number, and otherwise shall generate only enough digits
 *  so that the same time shall be recovered if the file is extracted on a
 *  system whose underlying implementation supports the same time
 *  granularity.
 *
 * So...
 * Looks like for our purposes we ignore anything after a possible decimal
 * point and just use the integer part of whatever is there and just not be
 * to worried about it since we aren't creating any of this, just reading ie.
 */
static rc_t pax_xhdr_set_general_time (const char * ts, time_t * tt)
{
    uint64_t temp = strtou64(ts,NULL,10); /* tosses decimal part for us */

    if (temp)
        *tt = (time_t)temp;
    return temp ? 0 : ~0;
}

/* ======================================================================
 * This is the data accumulated for each entry in a tar file.
 */
static struct   tar_entry_data
{
    /* -----
     * We are not using linux (or other O/S specific) type here because the ranges
     * for the system creating the archive might have larger types than the system
     * we are running on.  We'll leave it to the outside caller of this program to
     * "make it fit".
     */

    
    entry_type  type;

    char *  path;
    char *  link;

    uint64_t    size;
    uint64_t    offset;

    mode_t  mode;

    char *  uname;
    uid_t   uid;

    char *  gname;
    uint32_t    gid;

    time_t  mtime;
    time_t  atime;
    time_t  ctime;

} tar_entry_data;
#endif
/* ======================================================================
 * Mapping a memory region
 *
 * This will have two versions:
 *  initially for development it will make raw Linux O/S calls
 *  quickly it will be ported to use KFS structures instead
 */
/* -----
 * Use a map window size of a gigabyte
 * Life would be really bad if a single header was with in pagesize of that gigabyte
 * This assumes that a gigabyte is a multiple of system pagesize - a very safe bet.
 */
#define MAP_WINDOW_SIZE (1024*1024*1024)

/* ----------
 * map_tar_file
 *
 * This function uses existing members of the ktar state structure and a single parameter
 * to decide what part of a file to map.
 *
 * IN: offset:  an uint64_t type of where the starting point with in the file the memory mapped region
 *      should start
 * SIDE:    side effects are a freeing of any existing memory mapped region of a file and
 *      if successful mappinga region it will have mapped that region and put real
 *      parameters describing that region in the private memory block
 */
static
rc_t map_tar_file (KTarState * self, uint64_t requested_offset)
{
    rc_t  ret = 0;

    /* -----
     * If we are mapping for the first time
     */
    if (self->kmmap == NULL)
    {
    ret = KMMapMakeMaxRead(&self->kmmap,self->kfile);
    if (ret)
        return ret;
    ret = KMMapSize(self->kmmap, &self->buffer_length);
    if (ret)
        return ret;
    }
    /* -----
     * if we are remapping the region
     */
    else
    {
    ret = KMMapReposition(self->kmmap, requested_offset, &self->buffer_length);
    if (ret)
        return ret;
    }
    ret = KMMapAddrRead(self->kmmap, &self->map);
    if (ret)
    return ret;
    ret = KMMapPosition(self->kmmap, &self->buffer_start);
    if (ret)
    return ret;
    ret = KMMapSize(self->kmmap, &self->buffer_length);
    if (ret)
    return ret;
    self->buffer_limit = self->buffer_start + self->buffer_length;

    return ret;
}

static
rc_t release_map (KTarState * self)
{
    KMMapRelease (self->kmmap);
    self->kmmap = NULL;
    return 0;
}

/* ======================================================================
 *
 * offset is the byte position within the tar file
 * hard_limit is the byte position with in the tar file that is not mapped
 *
 * This is the ugliest function/method in the whole module.  The extensions
 * to the tar header are not done in a consistent manner so convolutions
 * have to be made to support all manner of extensions.
 */
static
uint64_t process_one_entry (KTarState * self, uint64_t offset, uint64_t hard_limit, bool silent)
{
    /* -----
     * full_path will store the full path of an element which can be longer than 
     * will fit in the standard tar header.  This will also usually be an output
     * to the consumer.
     */
    char full_path [ 4096 ];

    /* -----
     * full_path will store the full link (if any) of an element which can be longer than 
     * will fit in the standard tar header.  This will also usually be an output
     * to the consumer.
     */
    char full_link [ 4096 ];

    /* -----
     * data_offset will index into the tar file where the data portion for the current header
     * lies. (Usually at the address of the header + 512)  This will also usually be an output
     * to the consumer.
     */
    uint64_t data_offset = 0;

    /* -----
     * data_size will hold the size of the data portion for the current header.  This will also
     * usually be an output to the consumer.
     */
    uint64_t data_size = 0;

    /* -----
     * virtual_data_size will hold the virtual size of a sparse file.  This will also
     * usually be an output to the consumer.
     */
    uint64_t virtual_data_size = 0;
    /* -----
     * type is an enumerated type that described the format of the tar header.  Its initialized
     * to an invalid header type.
     */
    tar_header_type type = TAR_UNDEFINED;

    /* -----
     * link is an enumerated type that described the contents of this element.
     */
    tar_link link = LINK_OLDNORMAL_FILE;

#if _DEBUGGING && 0
/* We are not using these components of the tar header block at this point 
 * but with a debug build it doesn't hurt to verify we fully understand the
 * header.
 */
    uid_t  uid = 0;
    uint32_t gid = 0;
#endif
    time_t mtime = 0;
    mode_t mode = 0;

    /* -----
     * current_offset is the offset of the current header which might be a different header 
     * than the one we started with.
     */
    uint64_t current_offset = offset;

    /* -----
     * we can access the header either as a sequence of bytes or as a
     * header structure.  That header can further be accessed as one of
     * several more specific types of header.
     */
    union
    {
        const uint8_t *    b;
        const tar_header * h;
    } current_header;

    /* -----
     * done is a flag as to when we are finished processing a tar element
     * that might have multiple headers and other elements
     */
    bool done = false;

    /* -----
     * gnu_sparse is a flag that we are currently inside a sparse file
     * with in the TAR and have more sparse header elements to parse
     */
    bool gnu_sparse = false;

    TAR_FUNC_ENTRY();

    /* -----
     * check right away to make sure we are still in our memory mapped window
     */
    if (offset > self->buffer_limit)
    {
        /* -----
         * if not bail and say we couldn't consume any bytes
         */
        return 0;
    }

    /* -----
     * clear the full name and link name entries
     */
    memset (full_path, 0, sizeof(full_path));
    memset (full_link, 0, sizeof(full_link));

    /* -----
     * set the header at the current TAR block.
     * That is the map starts at offset buffer_start and we are at 
     * current_offset into the file so we take the map as a pointer
     * and add to it the difference between our current offset and the map's
     * initial offset (first header is at map + 0 - 0)
     */
    current_header.b = (const uint8_t *)self->map + current_offset - self->buffer_start;
    /* -----
     * start processing
     */
    do
    {
        TAR_DEBUG (( "Processing one block at (%lu), it is sparse? %s\n",
                     current_offset, get_bool_string(gnu_sparse)));

        /* -----
         * what we will do depends upon the type of this block
         */
        type = gnu_sparse ? TAR_SPARSE : what_header_type(current_header.h);
        if (self->found_zero_block)
        {
            if (self->found_second_zero_block == true)
            {
                type = TAR_ZERO_BLOCK; /* skip anyway */
            }
            else if (type == TAR_ZERO_BLOCK)
            {
                self->found_second_zero_block = true;
            }
            else
            {
                if ( ! silent )
                {
                    PLOGMSG(klogErr,(klogErr,
                        "Found Extra Header after a block of zeros $(O)",
                        PLOG_U64(O), offset));
                }
                return -1;
            }
        }
        switch (type)
        {
        case TAR_ZERO_BLOCK:
        {
            self->found_zero_block = true;
            done = true;
            break;
        }
        case TAR_SPARSE:
        {
            /* -----
             * If there is an extension header we'll have different work to do
             */
#if 0
            if (current_header.h->sparse.isextended)
            {
                LOGMSG (klogDebug3,"isextended true");
            }
            else
            {
                LOGMSG (klogDebug3,"isextended false");
            }
#endif
            {
                int64_t  ix;
                uint64_t of;
                uint64_t sz;

                for (ix = 0; ix< GNU_SPARSES_IN_EXTRA_HEADER; ++ix)
                {
                    rc_t ret;
                    of = tar_strtoll(
                     (const uint8_t*)current_header.h->sparse.sparse[ix].offset,
                                     TAR_SIZE_LEN, silent);
                    sz = tar_strtoll(
                  (const uint8_t*)current_header.h->sparse.sparse[ix].num_bytes,
                                     TAR_SIZE_LEN, silent);
                    if (sz == 0)
                    {
                        break;
                    }
#if 0
                    PLOGMSG ((klogDebug4, 
                              "SPARSE ext: $(count): $(offset) $(size)",
                              PLOG_3(PLOG_I64(count),PLOG_X64(offset),PLOG_X64(size)),
                              ix,
                              of,
                              sz));
#endif

                    ret = sparse_data_push (&self->sparse_q, of, sz);
                    if (ret)
                    {
                        sparse_data_kill(&self->sparse_q);
                        return -1;
                    }
                    ++self->num_chunks;
                }
            }
        }
        break;

#if 0
        /*Same as the default case */
        case TAR_CPIO:
            PLOGMSG ((klogErr,
                      "unsupported header type $(type) $(string)",
                      "type=%d,string=%s",
                      type, get_type_string(type)));
            done = true;
            break;
#endif

        default:
            if ( ! silent )
            {
                PLOGMSG (klogErr,(klogErr,
                                  "unsupported header type $(type) $(string)",
                                  "type=%d,string=%s",
                                  type, get_type_string(type)));
            }
            done = true;
            break;

        case TAR_GNU_89:
        case TAR_V7:
        case TAR_POSIX:
            /* -----
             * almost anything we do will depend upon the size of the data for this block
             *
             * this will be wrong if we ever support cpio...
             */
            data_size = (uint64_t) ( tar_strtoll
                ( (uint8_t*)current_header.h->tar.size,TAR_SIZE_LEN, silent ) );
#if _DEBUGGING && 0
            uid =  ( tar_strtoll
                ( (uint8_t*)current_header.h->tar.uid,TAR_ID_LEN, silent ) );
            gid =  ( tar_strtoll
                ( (uint8_t*)current_header.h->tar.gid,TAR_ID_LEN, silent ) );
#endif
            mtime = ( tar_strtoll
                ( (uint8_t*)current_header.h->tar.mtime,TAR_TIME_LEN, silent) );
            mode = (uint32_t) ( tar_strtoll
                ( (uint8_t*)current_header.h->tar.mode,TAR_MODE_LEN, silent) );
        }

        /* -----
         * Sometimes we are done just by identifying the header type.
         * If so we break the loop here.
         */
        if (done)
        {
            /* -----
             * point at the next header block
             * Add the size of the header itself plus enough block sizes of data to cover
             * any associated data.
             */
            size_t header_plus_data_block_size = (1+BLOCKS_FOR_BYTES(data_size))*TAR_BLOCK_SIZE;
            current_offset += (uint64_t)header_plus_data_block_size;
            current_header.b += header_plus_data_block_size;
            break;
        }

        /* -----
         * several extensions to USTAR/TAR format headers involve
         * prepending another header type to give a name longer 
         * than will fit in tthe header itself.  If we had one of
         * those use that name.  But if the full path has not been
         * set use the path from this header.
         */
        if (full_path[0] == 0) /* if full_path wasn't filled in by an 'L' long name */
        {
            size_t len, size;
            /* -----
             * if there is a prefix (POSIX style) use it
             * copy the prefix and then concatenate the name field
             */
            if (current_header.h->posix.prefix[0])
            {
#if 0
                PLOGMSG ((klogDebug1,
                          "used a posix prefix $(prefix)",
                          "prefix=%s",
                          current_header.h->posix.prefix));
#endif
                /* -----
                 * copy in the prefix, force a NUL just in case. then add a directory divider 
                 */
                string_copy(full_path, sizeof(full_path), current_header.h->posix.prefix, TAR_PREFIX_LEN);
                full_path[TAR_PREFIX_LEN] = 0x00;
                strcat(full_path,"/");
            }

            strncat(full_path,current_header.h->tar.name,TAR_NAME_LEN);
            len = string_measure(full_path, &size);
            while (len > 1)
            {
                if (full_path[len-1] == '/')
                    len--;
                else
                    break;
            }
            full_path[len] = '\0';
        }
        link = current_header.h->tar.link;
        TAR_DEBUG(("link = %s(%c)\n",get_link_string(link),link));
        switch (link)
        {
        case LINK_SPARSE:
            /* -----
             * If there is an extension header we'll have different work to do
             */
            if (current_header.h->gnu_89.isextended)
            {
                /* -----
                 * If we have an extended header following make sure there is room
                 */
                if ( (uint64_t)( offset + 2 * sizeof(tar_header) ) > hard_limit )
                    return 0;
                done = false;
                gnu_sparse = true;  /* next block will be part of the header and not data */
                data_offset = offset + 2 * sizeof(tar_header);
            }
            else
            {
                done = true;
                gnu_sparse = false;
                data_offset = offset + sizeof(tar_header);
            }

            done = (bool)! current_header.h->gnu_89.isextended;
            {
                int32_t ix;
                rc_t    ret;

                virtual_data_size = (uint64_t)(tar_strtoll(
                    (uint8_t*)current_header.h->gnu_89.realsize,TAR_SIZE_LEN,
                    silent));

                for (ix = 0; ix< GNU_SPARSES_IN_OLD_HEADER; ++ix)
                {
                    uint64_t soffset = tar_strtoll(
                     (const uint8_t*)current_header.h->gnu_89.sparse[ix].offset,
                                                   TAR_SIZE_LEN, silent);
                    uint64_t ssize = tar_strtoll(
                  (const uint8_t*)current_header.h->gnu_89.sparse[ix].num_bytes,
                                                 TAR_SIZE_LEN, silent);
                    if (ssize == 0)
                    {
                        break;
                    }

                    ret = sparse_data_push (&self->sparse_q, soffset, ssize);
                    if (ret) 
                    {
                        sparse_data_kill(&self->sparse_q);
                        return -1;
                    }
                    ++self->num_chunks;
                }
            }
            data_offset = current_offset + sizeof(tar_header);
            break;

        case LINK_OLDNORMAL_FILE:       /* deprecated normal file */
            /* -----
             * this should only happen with LINK_OLDNORMAL_FILE
             *
             * If the type is file but the last character in the path is "/"
             * treat it as a directory instead
             */
        {
            size_t size;
            if (full_path[string_measure(full_path, &size)-1] == '/')
            {
                link = LINK_DIRECTORY;
            }
        }
        /* fall through */
        case LINK_NORMAL_FILE:
        case LINK_CONTIGUOUS_FILE:
        case LINK_DIRECTORY:
            data_offset = current_offset + sizeof(tar_header);
            done = true;
            break;

            /* since we do not extract for tar files, a symlink and a hardlink
             * are the same to us */
        case LINK_HARD_LINK:
        case LINK_SYMBOLIC_LINK:
            if (full_link[0] == 0)
            {
                string_copy(full_link, sizeof(full_link), current_header.h->tar.linkname, TAR_NAME_LEN);
            }
            done = true;
            break;
            
            /* ----------
             * These types we ignore and they are defined to not have a data size
             */
        case LINK_CHARACTER_SPECIAL:
        case LINK_BLOCK_SPECIAL:
        case LINK_FIFO:
        case LINK_INODE_METADATA:
            /* -----
             * Nothing to be done
             */
            TAR_DEBUG (("%s: ignored block link type %s(%c) @ %lu\n",
                        __func__,
                        get_link_string(current_header.h->tar.link),
                        get_link_string(current_header.h->tar.link),
                        (uint64_t)(current_offset)+(uint64_t)(OFFSET_OF(tar_v7_header,link)))); 
            data_size = 0; /* data size is specifically to be ignored */
            done = true;
            break;
            
            /* ----------
             * These types we ignore as a final block or a block unto themselves and are
             * not part of a series of blocks and they are defined to have a data size
             */
        case LINK_PAX_GLOBAL_XHDR:
        case LINK_GNU_DUMPDIR: /* we are just gonna ignore this and not treat it like LINK_DIRECTORY */
        case LINK_SOLARIS_ACL_FILE:
        case LINK_VOLUME_NAME:
        case LINK_MULTI_VOLUME:
            TAR_DEBUG (("%s: ignored block link type %s(%c) %lu @ %lu\n",
                        __func__,
                        get_link_string(current_header.h->tar.link),
                        get_link_string(current_header.h->tar.link),
                        data_size,
                        (uint64_t)(current_offset)+(uint64_t)(OFFSET_OF(tar_v7_header,link)))); 
            done = true;
            break;
            
            /* ----------
             * These types we ignore as a block with in a series of blocks
             * and they are defined to have a data size
             *
             * Nothing to be done
             */
        case LINK_SOLARIS_ACL:
        case LINK_PAX_XHDR: /* posix extended */
            /* -----
             */
            TAR_DEBUG (("%s: ignored block link type %s(%c) @ %lu\n",
                        __func__,
                        get_link_string(current_header.h->tar.link),
                        get_link_string(current_header.h->tar.link),
                        (uint64_t)(current_offset)+(uint64_t)(OFFSET_OF(tar_v7_header,link)))); 
            break;
            
            /* some link types we ignore this block */
        default:
            TAR_DEBUG (("%s: Ignoring block with link %s(%c/%02.2x) @ %lu\n",
                        __func__, 
                        get_link_string(current_header.h->tar.link),
                        current_header.h->tar.link,
                        (unsigned)(current_header.h->tar.link),
                        (uint64_t)(current_offset)+(uint64_t)(OFFSET_OF(tar_v7_header,link))));
            break;
            
        case LINK_NEXT_LONG_LINK:   /* long link name */
            /* -----
             * Long link name needs access now to its full set of data blocks, request a window shift 
             * if it is not currently accessible
             */
            if ( (uint64_t)( offset + sizeof( tar_header ) + data_size ) > hard_limit )
                return 0;
            
            string_copy(full_link, sizeof(full_link), (char*)(current_header.b + sizeof(tar_header)), data_size);
            break;
        case LINK_NEXT_LONG_NAME:   /* long path name */
            /* -----
             * Long path name needs access now to its full set of data blocks, request a window shift 
             * if it is not currently accessible
             */
            if ( (uint64_t)( offset + sizeof( tar_header ) + data_size ) > hard_limit )
            {
                return 0;
            }
            
            string_copy(full_path, sizeof(full_path), (char*)(current_header.b + sizeof(tar_header)), data_size);
            break;
        }
        
        /* -----
         * move the current header offset to past the data blocks
         */
        if (link == LINK_SPARSE)
        {
            current_offset += sizeof (tar_header);
            current_header.b += sizeof(tar_header);
        }
        else
        {
            current_offset += sizeof (tar_header)+ ((data_size+TAR_BLOCK_SIZE-1)/TAR_BLOCK_SIZE)*TAR_BLOCK_SIZE;
            current_header.b += sizeof(tar_header) + ((data_size+TAR_BLOCK_SIZE-1)/TAR_BLOCK_SIZE)*TAR_BLOCK_SIZE;
        }
        /* -----
         * if that is past the currently available
         * quit the parse of this entry asn ask for a window shift, yeah, we'll redo work
         * but its far simpler code to just start over than track being in the middle
         */
        if ((!done) && (current_offset >= hard_limit))
        {
            return 0;
        }
        
    } while (! done);
    
    /* -----
     * generate output for this entry
     */
    switch (type)
    {
    default:
        if ( ! silent )
        {
            PLOGMSG (klogErr,(klogErr,"Unhandled Header Block Type $(type):$(typeint)","type=%c,typeint=%d",type ? type : '0',type));
        }
        return RC (rcFS, rcArc, rcParsing, rcData, rcUnsupported);
    case TAR_ZERO_BLOCK:
#if 0
        LOGMSG (klogDebug1, "Zero Block");
#endif
        /* ignored */
        break;

    case TAR_CPIO:
    case TAR_V7:
    case TAR_POSIX:
#if 0
    case TAR_PAX:
#endif
    case TAR_SUN:
    case TAR_STAR_85:
    case TAR_STAR_94:
    case TAR_GNU_89:
#if 0
    case TAR_GNU_01:
#endif
    case TAR_SPARSE:
        /* -----
         * TODO:
         *  implement a header checksum routine call it here, and return -1 if the check sum fails
         */
        if (gnu_sparse)
        {
#if 0
            PLOGMSG ((klogDebug3,
                      "Need to create a file but gnu_sparse is true $(l)",
                      PLOG_U8(l),
                      link
                         ));
#endif
            link = LINK_SPARSE; /* faking it for the next section */
        }
        switch (link)
        {
        case LINK_OLDNORMAL_FILE:
        case LINK_NORMAL_FILE:
        case LINK_CONTIGUOUS_FILE:
#if 0
            LOGMSG (klogDebug3, "KTocCreateFile");
#endif
            TAR_DEBUG (("%s call KTocCreateFile '%c':%hhd\n", __func__, link, link));
            KTocCreateFile (self->toc,
                            data_size?data_offset:0,
                            data_size,
                            mtime,
                            mode,
                            (KCreateMode)(kcmInit|kcmParents),
                            full_path);
            /* -----
             * TODO:
             *  print something if extraneous fields found
             */
            break;
        case LINK_HARD_LINK:
            TAR_DEBUG (("%s call KTocCreateHardLink\n", __func__));
            KTocCreateHardLink (self->toc, mtime, mode,
                                (KCreateMode)(kcmInit|kcmParents),
                                full_link, full_path);
            /* -----
             * TODO:
             *  print something if extraneous fields found
             */
            break;
        case LINK_SYMBOLIC_LINK:
            TAR_DEBUG (("%s call KTocCreateSoftLink\n", __func__));
            KTocCreateSoftLink (self->toc, mtime, mode,
                                (KCreateMode)(kcmInit|kcmParents),
                                full_link, full_path);
            /* -----
             * TODO:
             *  print something if extraneous fields found
             */
            break;

        case LINK_SPARSE:
            TAR_DEBUG (("%s LINK_SPARSE current_offset %jx: data_offset %jx: data_size %jx\n",
                        __func__, current_offset, data_offset, data_size));
            make_chunk_list(self, data_offset);
            KTocCreateChunkedFile (self->toc,
                                   virtual_data_size,
                                   mtime, mode,
                                   self->num_chunks,
                                   self->chunks,
                                   (KCreateMode)(kcmInit|kcmParents), 
                                   full_path);
            whack_chunk_list (self);
            break;
        case LINK_CHARACTER_SPECIAL:
        case LINK_BLOCK_SPECIAL:
        case LINK_FIFO:
        case LINK_VOLUME_NAME:
        case LINK_SOLARIS_ACL:
        case LINK_GNU_DUMPDIR:
        case LINK_SUN_XHDR:
        case LINK_INODE_METADATA:
            TAR_DEBUG ( ( "%s ignored entry type %s linktype name  %s\n",
                          __func__, get_link_string( link ), full_path ) );
            /* ignore */
            break;
        case LINK_DIRECTORY:
            TAR_DEBUG (("%s call KTocCreateDir\n", __func__));
            KTocCreateDir (self->toc, mtime, mode, (KCreateMode)(kcmOpen|kcmParents), full_path);
            /* -----
             * TODO:
             *  print something if extraneous fields found?
             */
            break;
        default:
            if ( ! silent )
            {
                PLOGMSG(klogErr,(klogErr,
                                 " type ($(type)) name ($(name() link ($(link)) size ($(size)) offset ($(offset))",
                                 "type=%s,name=%s,link=%s,size=%lld,offset=%lld",
                                 get_type_string(type),
                                 full_path,
                                 full_link,
                                 data_size,
                                 data_offset));
            }
            break;
        }
        break;
    }
    return current_offset - offset;
}


static
rc_t KArcParseTAR_intern ( KToc * self, 
                           const void * kvoid,
                           bool silent )
{
    KTarState state;
    /* -----
     * offset is the running index into the file of where the first tar header
     *          for the next element should start
     */
    uint64_t offset;
    /* -----
     * used is the count of bytes used for headers, storage and padding of the last
     *          examined element
     */
    uint64_t used;
    uint64_t filesize;
    const KFile * kfile = kvoid;
    rc_t rc;

    TAR_FUNC_ENTRY();

    /* -----
     * save the KFS File and TOC references
     */

    if ( kfile == NULL )
    {
        if ( !silent )
            LOGMSG ( klogFatal, "Called with a bad KFile parameter" );
        return RC (rcFS, rcArc, rcParsing, rcParam, rcNull );
    }
    else if ( self == NULL )
    {
        if ( !silent )
            LOGMSG (klogFatal, "Called with a bad KToc parameter");
        return RC (rcFS, rcArc, rcParsing, rcSelf, rcNull );
    }

    memset (&state, 0, sizeof (state));
    state.kfile = kfile;
    state.toc = self;

    if ( ( rc = KFileSize ( state.kfile, &filesize ) ) != 0 )
    {
        if ( !silent )
            LOGERR ( klogFatal, rc, "Failed to get file size of tarfile" );
    }
    else if ( filesize == 0 )
    {
        rc = RC ( rcFS, rcArc, rcAccessing, rcArc, rcEmpty );
        if ( !silent )
            LOGERR( klogFatal, rc, "Empty file" );
    }
    else if ( ( rc = map_tar_file ( &state, 0 ) ) != 0 )
    {
        if ( !silent )
            PLOGMSG( klogFatal, 
                    ( klogFatal, "Failed to $(operation) of size $(size)",
                      "operation=%s,size=%lu", "mmap", filesize ) );
        return rc;
    }
    else
    {
        int stalled = 0;

        for ( offset = 0; offset < filesize; )
        {
            /* -----
             * evaluate the tar file header at the current offset into the file
             *
             * the return is a positive number of bytes used
             * OR a negative on a file parse error
             * OR zero for a file that would exceed the current window
             */
            used = process_one_entry ( &state, offset, state.buffer_limit, silent );

            if ( used == 0 )
                stalled ++;
            else
                stalled = 0;

            /* kill any left over sparse data - safely handles an empty queue */
            sparse_data_kill ( &state.sparse_q );

            if ( state.found_second_zero_block )
            {
                break;
            }
            if ( used > 0 )
            {
                offset += used;
            }
            else if ( used == 0 )   /* if the validate returns 0 we need more buffer */
            {
                if ( stalled == 5 ) /* sure why not 5 tries */
                {
                    rc = RC ( rcFS, rcArc, rcParsing, rcArc, rcIncomplete );
                    break;
                }
                map_tar_file ( &state, offset ); /* slide the window up to current location */
                /* -----
                 * TODO:
                 *  Handle a repeated call from the same location as a failure in file format
                 * (truncation) of the tar file
                 */
            }
            else /*if (used < 0)*/      /* if it is negative it means abort the tar file */
            {
                rc = RC ( rcFS, rcArc, rcParsing, rcArc, rcUnexpected );
                break;
            }
        }
        if ( offset > filesize )
        {
            rc = RC ( rcFS, rcArc, rcParsing, rcToc, rcIncomplete );
            TAR_DEBUG (( "%s %R File offset %ju exceeds filesize %ju\n",
                         __func__, rc, offset, filesize ));
        }
        release_map ( &state );
    }
    return rc;
}

/* ======================================================================
 * validating a tar file is listing the files, links and directories
 * in that archive.
 *
 * partial results are not to be accepted in the end but it is the responsibility
 * of the caller to clear them.
 *
 * returns 0 for good archive and -1 for bad archive
 */
LIB_EXPORT rc_t CC KArcParseTAR ( KToc * self, 
              const void * kvoid,
              bool ( CC * ignored )( const KDirectory *, const char *, void * ),
              void *also_ignored )
{
    return KArcParseTAR_intern ( self, kvoid, false );
}


LIB_EXPORT rc_t CC KArcParseTAR_silent ( KToc * self, 
              const void * kvoid,
              bool ( CC * ignored )( const KDirectory *, const char *, void * ),
              void *also_ignored )
{
    return KArcParseTAR_intern ( self, kvoid, true );
}


LIB_EXPORT int CC KDirectoryVOpenTarArchiveRead ( struct KDirectory const *self,
    struct KDirectory const **tar_dir, int chroot, const char *fmt, va_list args )
{
    char path [ 4096 ];
    int size = (args == NULL) ?
        snprintf  ( path, sizeof path, "%s", fmt ) :
        vsnprintf ( path, sizeof path, fmt, args );
    if ( size < 0 || size >= ( int ) sizeof path )
        return RC ( rcFS, rcDirectory, rcOpening, rcPath, rcExcessive );

    /* putting off parameter validation into this call */
    return KDirectoryOpenArcDirRead ( self, tar_dir, false, path, tocKFile,
                                      KArcParseTAR, NULL, NULL );
}

LIB_EXPORT int CC KDirectoryVOpenTarArchiveRead_silent ( struct KDirectory const *self,
    struct KDirectory const **tar_dir, int chroot, const char *fmt, va_list args )
{
    char path [ 4096 ];
    int size = (args == NULL) ?
        snprintf  ( path, sizeof path, "%s", fmt ) :
        vsnprintf ( path, sizeof path, fmt, args );
    if ( size < 0 || size >= ( int ) sizeof path )
        return RC ( rcFS, rcDirectory, rcOpening, rcPath, rcExcessive );

    /* putting off parameter validation into this call */
    return KDirectoryOpenArcDirRead_silent ( self, tar_dir, false, path, tocKFile,
                                      KArcParseTAR_silent, NULL, NULL );
}


LIB_EXPORT int CC KDirectoryVOpenTarArchiveRead_silent_preopened ( struct KDirectory const *self,
    struct KDirectory const **tar_dir, int chroot, const struct KFile * f, const char *fmt, va_list args )
{
    char path [ 4096 ];
    int size = (args == NULL) ?
        snprintf  ( path, sizeof path, "%s", fmt ) :
        vsnprintf ( path, sizeof path, fmt, args );
    if ( size < 0 || size >= ( int ) sizeof path )
        return RC ( rcFS, rcDirectory, rcOpening, rcPath, rcExcessive );

    /* putting off parameter validation into this call */
    return KDirectoryOpenArcDirRead_silent_preopened ( self, tar_dir, false, path, tocKFile, 
                                      (void*)f, KArcParseTAR_silent, NULL, NULL );
}


LIB_EXPORT int CC KDirectoryOpenTarArchiveRead ( struct KDirectory const *self,
    struct KDirectory const **tar_dir, int chroot, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDirectoryVOpenTarArchiveRead ( self, tar_dir, chroot, path, args );
    va_end ( args );

    return rc;
}


LIB_EXPORT int CC KDirectoryOpenTarArchiveRead_silent ( struct KDirectory const *self,
    struct KDirectory const **tar_dir, int chroot, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDirectoryVOpenTarArchiveRead_silent ( self, tar_dir, chroot, path, args );
    va_end ( args );

    return rc;
}


LIB_EXPORT int CC KDirectoryOpenTarArchiveRead_silent_preopened ( struct KDirectory const *self,
    struct KDirectory const **tar_dir, int chroot, const KFile * f, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDirectoryVOpenTarArchiveRead_silent_preopened ( self, tar_dir, chroot, f, path, args );
    va_end ( args );

    return rc;
}


/* ----------
 * Validate that the compiler packed all the character arrays into the correct sizes
 * to use the header.
 *
 * a bunch of magic numbers are in the function because we are making sure that
 * they are reached by using the structure members.  They are listed in comments
 * within comments in the tar_header.h file.
 *
 * No return value
 */
LIB_EXPORT bool CC validate_header_offsets( void )
{
    bool pass_fail = true; /* assume pass */
#if _DEBUGGING
    /* -----
     * checking the size entails making sure the compiler made the structure
     * the right length to match the series of octents in the file
     */
#define check_size(T,V)     if( sizeof(T) != (size_t)V) { pass_fail = false; printf ("*** BAD_SIZE: %s is %u not %u\n", #T, (unsigned)sizeof(T), (unsigned)V);}
    /* -----
     * checking the offset entails making sure the compiler put the field at the
     * right absolute location within a structure
     */
#define check_offset(T,M,V) if( OFFSET_OF(T,M) != (size_t)V ) { pass_fail = false; printf ("*** BAD_OFFSET: %s.%s is %u not %d\n", #T, #M, (unsigned)OFFSET_OF(T,M), V);}

    check_size(tar_raw_block,512);
    check_size(tar_v7_header,257);
    check_size(tar_posix_header,500);
    check_size(tar_sun_header,512);
    check_size(tar_star_85_header,512);
    check_size(tar_star_94_header,512);
    check_size(tar_gnu_89_header,495);
    /*check_size(tar_new_gnu_header,512); */
    check_size(tar_sparse_header,505);
  

    /* tar_header union */
    check_offset(tar_header,raw,0);
    check_offset(tar_header,tar,0);
    check_offset(tar_header,posix,0);
    check_offset(tar_header,suntar,0);
    check_offset(tar_header,star_85,0);
    check_offset(tar_header,star_94,0);
    check_offset(tar_header,gnu_89,0);
    /*check_offset(tar_header,new_gnu,0); */
    check_offset(tar_header,sparse,0);

    /* tar_v7_header */
    check_offset(tar_v7_header,name,0);
    check_offset(tar_v7_header,mode,100);
    check_offset(tar_v7_header,uid,108);
    check_offset(tar_v7_header,gid,116);
    check_offset(tar_v7_header,size,124);
    check_offset(tar_v7_header,mtime,136);
    check_offset(tar_v7_header,csum,148);
    check_offset(tar_v7_header,link,156);
    check_offset(tar_v7_header,linkname,157);

    /* tar_posix_header */
    check_offset(tar_posix_header,name,0);
    check_offset(tar_posix_header,mode,100);
    check_offset(tar_posix_header,uid,108);
    check_offset(tar_posix_header,gid,116);
    check_offset(tar_posix_header,size,124);
    check_offset(tar_posix_header,mtime,136);
    check_offset(tar_posix_header,csum,148);
    check_offset(tar_posix_header,link,156);
    check_offset(tar_posix_header,linkname,157);
    check_offset(tar_posix_header,magic,257);
    check_offset(tar_posix_header,version,263);
    check_offset(tar_posix_header,uname,265);
    check_offset(tar_posix_header,gname,297);
    check_offset(tar_posix_header,devmajor,329);
    check_offset(tar_posix_header,devminor,337);
    check_offset(tar_posix_header,prefix,345);

    /* tar_sun_header */
    check_offset(tar_sun_header,name,0);
    check_offset(tar_sun_header,mode,100);
    check_offset(tar_sun_header,uid,108);
    check_offset(tar_sun_header,gid,116);
    check_offset(tar_sun_header,size,124);
    check_offset(tar_sun_header,mtime,136);
    check_offset(tar_sun_header,csum,148);
    check_offset(tar_sun_header,link,156);
    check_offset(tar_sun_header,linkname,157);
    check_offset(tar_sun_header,magic,257);
    check_offset(tar_sun_header,version,263);
    check_offset(tar_sun_header,uname,265);
    check_offset(tar_sun_header,gname,297);
    check_offset(tar_sun_header,devmajor,329);
    check_offset(tar_sun_header,devminor,337);
    check_offset(tar_sun_header,prefix,345);
    check_offset(tar_sun_header,extnum,500);
    check_offset(tar_sun_header,extcount,501);
    check_offset(tar_sun_header,fullsize,502);

    /* tar_star_85_header */
    check_offset(tar_star_85_header,name,0);
    check_offset(tar_star_85_header,mode,100);
    check_offset(tar_star_85_header,uid,108);
    check_offset(tar_star_85_header,gid,116);
    check_offset(tar_star_85_header,size,124);
    check_offset(tar_star_85_header,mtime,136);
    check_offset(tar_star_85_header,csum,148);
    check_offset(tar_star_85_header,link,156);
    check_offset(tar_star_85_header,linkname,157);
    check_offset(tar_star_85_header,starversion,257);
    check_offset(tar_star_85_header,starfiletype,258);
    check_offset(tar_star_85_header,startype,266);
    check_offset(tar_star_85_header,rdev,278);
    check_offset(tar_star_85_header,atime,290);
    check_offset(tar_star_85_header,ctime,302);
    check_offset(tar_star_85_header,uname,314);
    check_offset(tar_star_85_header,gname,330);
    check_offset(tar_star_85_header,prefix,345);
    check_offset(tar_star_85_header,xmagic,508);

    /* tar_star_94_header */
    check_offset(tar_star_94_header,name,0);
    check_offset(tar_star_94_header,mode,100);
    check_offset(tar_star_94_header,uid,108);
    check_offset(tar_star_94_header,gid,116);
    check_offset(tar_star_94_header,size,124);
    check_offset(tar_star_94_header,mtime,136);
    check_offset(tar_star_94_header,csum,148);
    check_offset(tar_star_94_header,link,156);
    check_offset(tar_star_94_header,linkname,157);
    check_offset(tar_star_94_header,magic,257);
    check_offset(tar_star_94_header,version,263);
    check_offset(tar_star_94_header,uname,265);
    check_offset(tar_star_94_header,gname,297);
    check_offset(tar_star_94_header,devmajor,329);
    check_offset(tar_star_94_header,devminor,337);
    check_offset(tar_star_94_header,prefix,345);
    check_offset(tar_star_94_header,isextended,355);
    check_offset(tar_star_94_header,sparse,356);
    check_offset(tar_star_94_header,realsize,452);
    check_offset(tar_star_94_header,offset,464);
    check_offset(tar_star_94_header,atime,476);
    check_offset(tar_star_94_header,ctime,488);
    check_offset(tar_star_94_header,xmagic,508);

    /* tar_gnu_89_header */
    check_offset(tar_gnu_89_header,name,0);
    check_offset(tar_gnu_89_header,mode,100);
    check_offset(tar_gnu_89_header,uid,108);
    check_offset(tar_gnu_89_header,gid,116);
    check_offset(tar_gnu_89_header,size,124);
    check_offset(tar_gnu_89_header,mtime,136);
    check_offset(tar_gnu_89_header,csum,148);
    check_offset(tar_gnu_89_header,link,156);
    check_offset(tar_gnu_89_header,linkname,157);
    check_offset(tar_gnu_89_header,magic,257);
    check_offset(tar_gnu_89_header,uname,265);
    check_offset(tar_gnu_89_header,gname,297);
    check_offset(tar_gnu_89_header,atime,345);
    check_offset(tar_gnu_89_header,ctime,357);
    check_offset(tar_gnu_89_header,offset,369);
    check_offset(tar_gnu_89_header,longnames,381);
    check_offset(tar_gnu_89_header,sparse,386);
    check_offset(tar_gnu_89_header,isextended,482);

    /* tar_extended_header */
    check_offset(tar_sparse_header,sparse,0);
    check_offset(tar_sparse_header,isextended,504);
#endif
    return pass_fail;
}

/* end of file */
