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

#ifndef _h_ncbi_ncbi_
#define _h_ncbi_ncbi_

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*==========================================================================
 * NCBI types, constants
 */


/* 16-bit integer sample data
 */
typedef int16_t NCBI_isamp1;

/* 32-bit floating point sample data
 */
typedef float NCBI_fsamp1, NCBI_fsamp4 [ 4 ];

/* ASN.1
 */
typedef uint8_t NCBI_asn_binary; 
typedef char NCBI_asn_text;

/* GenInfo id - 64 bit because we are almost out of 32 bit ids
 */
typedef uint64_t NCBI_gi;

/* Taxonomy id
 */
typedef uint32_t NCBI_taxid;

/* Genbank status
 */
typedef uint32_t NCBI_gb_state;

#ifdef __cplusplus
}
#endif

#endif /* _h_ncbi_ncbi_ */
