#ifndef _h_vdb_quality_
#define _h_vdb_quality_

/*==============================================================================
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
* ============================================================================*/


#include <stdint.h> /* uint32_t */


#ifdef __cplusplus
extern "C" {
#endif


typedef int32_t VQuality;
typedef enum { /* Run Quality */
    eQualFullOnly = -2,/* use full quality run only, no double-quality run */
    eQualDblOnly,      /* use double quality run only */
    eQualDefault, /* default: in most cases: 'no'- if found, 'full' otherwise */
    eQualFull,    /* full quality (can be delivered by full-quality-only run
                                                       or double-quality run) */
    eQualNo,      /* no-quality = synthetic */
    eQualLast
} EQuality;


#ifdef __cplusplus
}
#endif

#endif /* _h_vdb_quality_ */
