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

#ifndef _h_klib_sra_release_version_
#define _h_klib_sra_release_version_


#ifndef _h_klib_extern_
#include <klib/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif


/* major . minor . release [ - [ type - ] revision ]
 *
 * Examples:
 *  "2.3.4-a0"
 *  "2.3.4-b3"
 *  "2.3.4-rc1"
 *  "2.3.4"
 *  "2.3.4-2"
 */
typedef struct SraReleaseVersion SraReleaseVersion;
struct SraReleaseVersion
{
    ver_t version;     /* major . minor . release */
    uint32_t revision;
    enum {
        eSraReleaseVersionTypeDev, /* developmemt */
        eSraReleaseVersionTypeAlpha,
        eSraReleaseVersionTypeBeta,
        eSraReleaseVersionTypeRC,  /* release candidate */
        eSraReleaseVersionTypeFinal,
    } type;
};


/* Get
 *  Get release version of this build of SRA Toolkit */
KLIB_EXTERN rc_t CC SraReleaseVersionGet ( SraReleaseVersion *version );

/* Cmp
 *  Compare two release versions
 *  result return values:
 *       0 - the versions are the same 
 *       1 - version2 is more recent than self
 *      -1 - self is more recent than version2
 */
KLIB_EXTERN rc_t CC SraReleaseVersionCmp ( const SraReleaseVersion *self,
    const SraReleaseVersion *version2, int32_t *result );

/* Parse
 *  Initialize SraReleaseVersion from char version[size]
 */
KLIB_EXTERN rc_t CC SraReleaseVersionInit ( SraReleaseVersion *self,
    const char *version, size_t size );

/* Print
 *  Convert SraReleaseVersion to version[size]
 */
KLIB_EXTERN rc_t CC SraReleaseVersionPrint ( const SraReleaseVersion *self,
    char *version, size_t size, size_t *num_writ );

#ifdef __cplusplus
}
#endif


#endif /* _h_klib_sra_release_version_ */
