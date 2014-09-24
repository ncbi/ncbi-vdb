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

#ifndef _h_kdbfmt_priv_
#define _h_kdbfmt_priv_

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * KDBHdr
 *  the various file types used:
 *
 *  a) metadata
 *  b) key => id
 *  c) id => key
 *
 * will begin with a standard header to tag format
 */
#define eByteOrderTag 0x05031988
#define eByteOrderReverse 0x88190305

typedef struct KDBHdr KDBHdr;
struct KDBHdr
{
    uint32_t endian;
    uint32_t version;
};

/* KDBHdrInit
 *  simple initialization
 */
#define KDBHdrInit( hdr, vers ) \
    ( void ) ( ( hdr ) -> endian = eByteOrderTag, ( hdr ) -> version = vers )

/* KDBHdrValidate
 *  validates that a header sports a supported byte order
 *  and that the version is within range
 */
rc_t KDBHdrValidate ( const KDBHdr *hdr, size_t size,
    uint32_t min_vers, uint32_t max_vers );

#ifdef __cplusplus
}
#endif

#endif /* _h_kdbfmt_priv_ */
