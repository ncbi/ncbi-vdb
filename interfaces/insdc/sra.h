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
typedef uint32_t INSDC_SRA_spotid_t;


/* spot_ids_found
 *  returns a tuple of spot ids
 */
typedef uint64_t INSDC_SRA_spot_ids_found [ 4 ];


/* read filter
 */
#define sra_read_filter_t "INSDC:SRA:read_filter"
typedef INSDC_read_filter INSDC_SRA_read_filter;
enum
{
    SRA_READ_FILTER_PASS = READ_FILTER_PASS,
    SRA_READ_FILTER_REJECT = READ_FILTER_REJECT,
    SRA_READ_FILTER_CRITERIA = READ_FILTER_CRITERIA,
    SRA_READ_FILTER_REDACTED = READ_FILTER_REDACTED
};


/* read type
 *  describes the type of read within a spot
 *  the extended version also describes its orientation
 */
#define sra_read_type_t "INSDC:SRA:xread_type"
typedef INSDC_read_type INSDC_SRA_xread_type;
typedef INSDC_SRA_xread_type INSDC_SRA_read_type;
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


/* platform id
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
    SRA_PLATFORM_SANGER            = 8
};

#ifdef __cplusplus
}
#endif

#endif /*  _h_insdc_sra_ */
