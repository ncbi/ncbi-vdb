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

#ifndef _h_insdc_sra_
#define _h_insdc_sra_

#ifndef _h_insdc_insdc_
#include <insdc/insdc.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*==========================================================================
 * INSDC SRA types, constants
 */


/* spotid_t
 *  unique id given to every spot
 */
typedef int64_t INSDC_SRA_spotid_t;


/* spot_ids_found
 *  returns a tuple of spot ids
 */
typedef uint64_t INSDC_SRA_spot_ids_found [ 4 ];


/* read filter
 */
#define sra_read_filter_t "INSDC:SRA:read_filter"
typedef INSDC_read_filter INSDC_SRA_read_filter;
#if 0
already defined in sra.h
enum
{
    SRA_READ_FILTER_PASS = READ_FILTER_PASS,
    SRA_READ_FILTER_REJECT = READ_FILTER_REJECT,
    SRA_READ_FILTER_CRITERIA = READ_FILTER_CRITERIA,
    SRA_READ_FILTER_REDACTED = READ_FILTER_REDACTED
};
#endif

/* read type
 */
#define sra_read_type_t "INSDC:SRA:xread_type"
//typedef INSDC_read_type INSDC_SRA_xread_type;
typedef INSDC_SRA_xread_type INSDC_SRA_read_type;
#if 0
already defined in sra.h
enum
{
    /* read_type and xread_type */
    SRA_READ_TYPE_TECHNICAL  = READ_TYPE_TECHNICAL,
    SRA_READ_TYPE_BIOLOGICAL = READ_TYPE_BIOLOGICAL,

    /* xread_type only - applied as bits, e.g.:
       type = SRA_READ_TYPE_BIOLOGICAL | SRA_READ_TYPE_REVERSE */
    SRA_READ_TYPE_FORWARD = READ_TYPE_FORWARD,
    SRA_READ_TYPE_REVERSE = READ_TYPE_REVERSE
};
#endif

/* platform id
 * see also sra.vschema
 */
#define sra_platform_id_t "INSDC:SRA:platform_id"
typedef uint8_t INSDC_SRA_platform_id;
enum
{
    SRA_PLATFORM_UNDEFINED         = 0,
    SRA_PLATFORM_454               = 1,
    SRA_PLATFORM_ILLUMINA          = 2,
    SRA_PLATFORM_ABSOLID           = 3,
    SRA_PLATFORM_COMPLETE_GENOMICS = 4,
    SRA_PLATFORM_HELICOS           = 5,
    SRA_PLATFORM_PACBIO_SMRT       = 6,
    SRA_PLATFORM_ION_TORRENT       = 7,
    SRA_PLATFORM_CAPILLARY         = 8,
    SRA_PLATFORM_OXFORD_NANOPORE   = 9,
    SRA_PLATFORM_ELEMENT_BIO       = 10,
    SRA_PLATFORM_TAPESTRI          = 11,
    SRA_PLATFORM_VELA_DIAG         = 12,
    SRA_PLATFORM_GENAPSYS          = 13,
    SRA_PLATFORM_ULTIMA            = 14,
    SRA_PLATFORM_GENEMIND          = 15,
    SRA_PLATFORM_BGISEQ            = 16,
    SRA_PLATFORM_DNBSEQ            = 17,
    SRA_PLATFORM_SINGULAR_GENOMICS = 18
};

/*
 * C string names for the platform enum values
 *
 * suggested use:
 *     static const char *platform_symbolic_names[] = { INSDC_SRA_PLATFORM_SYMBOLS };
 *     INSDC_SRA_platform_id p = ...;
 *     const char * platform_str = platform_symbolic_names[ p ];
 */
#define INSDC_SRA_PLATFORM_SYMBOLS    \
    "SRA_PLATFORM_UNDEFINED",         \
    "SRA_PLATFORM_454",               \
    "SRA_PLATFORM_ILLUMINA",          \
    "SRA_PLATFORM_ABSOLID",           \
    "SRA_PLATFORM_COMPLETE_GENOMICS", \
    "SRA_PLATFORM_HELICOS",           \
    "SRA_PLATFORM_PACBIO_SMRT",       \
    "SRA_PLATFORM_ION_TORRENT",       \
    "SRA_PLATFORM_CAPILLARY",         \
    "SRA_PLATFORM_OXFORD_NANOPORE",   \
    "SRA_PLATFORM_ELEMENT_BIO",       \
    "SRA_PLATFORM_TAPESTRI",          \
    "SRA_PLATFORM_VELA_DIAG",         \
    "SRA_PLATFORM_GENAPSYS",          \
    "SRA_PLATFORM_ULTIMA",            \
    "SRA_PLATFORM_GENEMIND",          \
    "SRA_PLATFORM_BGISEQ",            \
    "SRA_PLATFORM_DNBSEQ",            \
    "SRA_PLATFORM_SINGULAR_GENOMICS"

#ifdef __cplusplus
}
#endif

#endif /*  _h_insdc_sra_ */
