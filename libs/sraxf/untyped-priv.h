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

#ifndef _h_untyped_priv
#define _h_untyped_priv

/* 454_untyped_0
 *  recognizes all runs produced with v0 loader
 */
bool NCBI_SRA__454__untyped_0 ( const KTable *tbl, const KMetadata *meta );


/* 454_untyped_1_2a
 *  recognizes runs produced with v1.2 loader
 *  where the linker is present
 *  and a physical READ_SEG is present
 */
bool NCBI_SRA__454__untyped_1_2a ( const KTable *tbl, const KMetadata *meta );


/* 454_untyped_1_2b
 *  recognizes runs produced with v1.2 loader
 *  where the linker is missing or empty
 *  and a physical READ_SEG is present
 */
bool NCBI_SRA__454__untyped_1_2b ( const KTable *tbl, const KMetadata *meta );


/* Illumina_untyped_0a
 *  recognizes runs produced with v0 loader with 4-channel QUALITY
 */
bool NCBI_SRA_Illumina_untyped_0a ( const KTable *tbl, const KMetadata *meta );


/* Illumina_untyped_0b
 *  recognizes runs produced with v0 loader with single-channel QUALITY or QUALITY2
 */
bool NCBI_SRA_Illumina_untyped_0b ( const KTable *tbl, const KMetadata *meta );


/* Illumina_untyped_1a
 *  recognizes runs produced with v1 srf and native loaders
 *  having 4-channel qualities
 */
bool NCBI_SRA_Illumina_untyped_1a ( const KTable *tbl, const KMetadata *meta );


/* Illumina_untyped_1b
 *  recognizes runs produced with all v1 loaders
 *  having single-channel qualities
 */
bool NCBI_SRA_Illumina_untyped_1b ( const KTable *tbl, const KMetadata *meta );


/* ABI_untyped_1
 *  recognizes runs produced with all v1 loaders
 */
bool NCBI_SRA_ABI_untyped_1 ( const KTable *tbl, const KMetadata *meta );

#endif
