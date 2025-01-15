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

#ifndef _h_page_map_priv_
#define _h_page_map_priv_

#ifndef _h_kdb_extern_
#include <kdb/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#if _DEBUGGING
#define _HEAVY_PAGEMAP_DEBUGGING 0
#endif

typedef uint32_t pm_size_t;
typedef uint32_t row_count_t;
typedef uint32_t elem_count_t;

struct PageMap;

typedef enum {  PM_REGION_EXPAND_UNKNOWN=0,     /** notset **/
                PM_REGION_EXPAND_FULL,          /** full expansion     - 2 arrays: offset,length**/
                PM_REGION_EXPAND_SAMELEN,       /** partial expansion  - 1 array: offset **/
                PM_REGION_EXPAND_EQUIDISTANT,   /** data have same length and always unique - no expansion needed **/
                PM_REGION_EXPAND_SAMEDATA       /** same data - no expansion needed ***/
} pm_expand_region_type_t;

typedef struct PageMapRegion {
	row_count_t	start_row;
	row_count_t	numrows;
	elem_count_t	data_offset;/** for unexpanded regions first direct offset into data**/
				    /** for expanded regions - offset into expanded storage **/
	elem_count_t	length;	    /** first length of the region ***/
	uint8_t		type;       /** one of the types from pm_expand_region_type_t*/
	bool		expanded;   /** if expandable storage is being used ***/
} PageMapRegion;

typedef struct PageMapIterator PageMapIterator;
struct PageMapIterator {
    row_count_t		last_row;
    row_count_t		cur_row;
    PageMapRegion**	rgns;	  /** all regions from the pagemap **/
    pm_size_t		cur_rgn;  /** offset of the current region **/
    row_count_t 	cur_rgn_row; /** row relative to offset of the region **/
    elem_count_t	**exp_base; /*** exp buffer ***/
    row_count_t		repeat_count; /** remaining repeat count **/
    elem_count_t	static_datalen;
#if _HEAVY_PAGEMAP_DEBUGGING
    struct PageMap *parent;
#endif
};


KDB_EXTERN rc_t PageMapNewIterator(const struct PageMap *self, PageMapIterator *lhs, uint64_t first_row, uint64_t num_rows);

// these functions are defined as static __inline__ in page-map.h
KDB_EXTERN elem_count_t CC PageMapIteratorDataLength_Ext(const PageMapIterator *self);
KDB_EXTERN elem_count_t CC PageMapIteratorDataOffset_Ext(const PageMapIterator *self);
KDB_EXTERN row_count_t CC PageMapIteratorRepeatCount_Ext(const PageMapIterator *cself);
KDB_EXTERN bool CC PageMapIteratorAdvance_Ext(PageMapIterator *self, row_count_t rows);
KDB_EXTERN bool CC PageMapIteratorNext_Ext(PageMapIterator *self);

#endif /* _h_page_map_ */
