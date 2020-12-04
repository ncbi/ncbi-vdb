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

#ifndef _h_kfg_kart_
#define _h_kfg_kart_

#ifndef _h_kfg_extern_
#include <kfg/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifndef _h_klib_text_
#include <klib/text.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/* EObjectType
 * Extensible controlled vocabulary object type.
 * The list is defined in Described in Cart File Format 2.0 */
typedef enum {
    eOT_undefined,
    eOT_empty,
    eOT_dbgap,
    eOT_provisional,
    eOT_srapub,
    eOT_sragap,
    eOT_srapub_source,
    eOT_sragap_source,
    eOT_srapub_files,
    eOT_sragap_files,
    eOT_refseq,
    eOT_wgs,
    eOT_na,
    eOT_nakmer,

    eOT_sdlObject, /* a special type for KServiceAddObject */
} EObjectType;


struct KDirectory;
struct KSrvError;

/* AA-833 */

typedef struct KartItem KartItem;

KFG_EXTERN rc_t CC KartItemAddRef(const KartItem *self);
KFG_EXTERN rc_t CC KartItemRelease(const KartItem *self);

/** Do not release the returned String !
 *  N.B. returned String is not required to be NULL-terminated !
 */
KFG_EXTERN rc_t CC KartItemGetTicket(const KartItem *self,
    char * buffer, size_t buffer_size, size_t * written);
/* VERSION 1.0 ****************************************************************/
KFG_EXTERN rc_t CC KartItemProjId(const KartItem *self, const String **elem);
KFG_EXTERN rc_t CC KartItemProjIdNumber(const KartItem *self, uint64_t *id);
KFG_EXTERN rc_t CC KartItemItemId(const KartItem *self, const String **elem);
KFG_EXTERN rc_t CC KartItemItemIdNumber(const KartItem *self, uint64_t *id);
KFG_EXTERN rc_t CC KartItemAccession(const KartItem *self, const String **elem);
KFG_EXTERN rc_t CC KartItemName(const KartItem *self, const String **elem);
KFG_EXTERN rc_t CC KartItemItemDesc(const KartItem *self, const String **elem);
/* VERSION 2.0 ****************************************************************/
KFG_EXTERN rc_t CC KartItemObjType (const KartItem *self, const String **elem );
KFG_EXTERN rc_t CC KartItemPath (const KartItem *self, const String **elem );
KFG_EXTERN rc_t CC KartItemSize (const KartItem *self, const String **elem );

/* Kart object can be created:
 * - from a kart file,
 * - or as result to search service call. 
 * In the latter it can contain errors.
 */
KFG_EXTERN rc_t CC KartItemGetError (const KartItem *self,
    const struct KSrvError ** error );

typedef struct Kart Kart;

KFG_EXTERN rc_t CC KartAddRef(const Kart *self);
KFG_EXTERN rc_t CC KartRelease(const Kart *self);

KFG_EXTERN rc_t CC KartMake(const struct KDirectory *dir, const char *path,
    Kart **kart, bool *isKart);
KFG_EXTERN rc_t CC KartMakeWithNgc(const struct KDirectory *dir,
    const char *path, Kart **kart, bool *isKart, const char *ngcPath);
#ifdef _DEBUGGING
KFG_EXTERN rc_t CC KartMakeText(const struct KDirectory *dir, const char *path,
    Kart **kart, bool *isKart);
#endif

KFG_EXTERN rc_t CC KartPrint(const Kart *self);
KFG_EXTERN rc_t CC KartPrintNumbered(const Kart *self);

/* kart iterator */
KFG_EXTERN rc_t CC KartMakeNextItem(const Kart *self, const KartItem **item);
/* reset kart iterator */
KFG_EXTERN rc_t CC KartReset(const Kart *self);

KFG_EXTERN rc_t CC KartItemsProcessed(const Kart *self, uint16_t *number);


#ifdef __cplusplus
}
#endif

#endif /* _h_kfg_kart_ */
