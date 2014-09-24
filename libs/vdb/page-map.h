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

#ifndef _h_page_map_
#define _h_page_map_

#include <assert.h>

#ifndef _h_vdb_extern_
#include <vdb/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#include <klib/data-buffer.h>
#include <klib/refcount.h>

#if _DEBUGGING
#define _HEAVY_PAGEMAP_DEBUGGING 0
#endif

struct KDataBuffer;

typedef uint32_t pm_size_t;
typedef uint32_t row_count_t;
typedef uint32_t elem_count_t;

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




typedef struct PageMap {
    /* memory allocation object for length[], leng_run[], data_run[] */
    KDataBuffer cstorage;

    /* array of row lengths
     * has leng_recs elements
     * is sized to reserve_leng elements
     * == storage.base
     */
    bool   random_access;
    enum { eBlobPageMapOptimizedNone, eBlobPageMapOptimizedSucceeded, eBlobPageMapOptimizedFailed}  optimized;
    elem_count_t *length;

    /* array of run lengths of row lengths
     * has leng_recs elements
     * is sized to reserve_leng elements
     * == length + reserve_leng
     */
    row_count_t *leng_run;

    /* array of repeat counts of data
     * has data_recs elements
     * is sized to reserve_data elements
     * == leng_run + reserve_leng
     */
    row_count_t *data_run;
    /** expanded offsets into data - needed for random access ***/
    /** only valid when random_access is true **/
    elem_count_t *data_offset;

/******* DYNAMIC EXPANSION CONTROL *****************/
    PageMapRegion		*exp_rgn_last;
    row_count_t			exp_row_last;   /* last row analyzed for region expansion */
    row_count_t			exp_lr_used;    /* how much leng_run was used from the */
    pm_size_t			exp_lr_last;	/* index of last leng_run expanded */
    pm_size_t			exp_dr_last;	/* index of last data_run expanded */
    pm_size_t			exp_rgn_cnt;    /* current number of expanded regions */
    elem_count_t		exp_data_offset_last;/* last offset into data */
    

    KDataBuffer			istorage;	/* binary searchable storage for expansion regions */
    KDataBuffer			dstorage;	/* storage for expanded data */
/** LAST SEARCH CONTROL *****/
    pm_size_t			i_rgn_last; 	/* region index found in previous lookup **/
    PageMapRegion*		rgn_last; 	/* redundant - region found in previous lookup **/

/****************************/

    pm_size_t leng_recs;     /* number of valid elements in length[] and leng_run[] */
    pm_size_t data_recs;     /* number of valid elements in data_run[] */
    pm_size_t reserve_leng;  /* number of allocated elements in length[] and leng_run[] */
    pm_size_t reserve_data;  /* number of allocated elements in data_run[] */
    pm_size_t start_valid;   /* the expanded array contains valid data upto start_valid */
    row_count_t row_count;   /* total number of rows in page map */
    row_count_t pre_exp_row_count; /* number of rows pre-expanded */
    KRefcount refcount;
} PageMap;


/* a pessimistic estimate - actual size will *always* be less than or equal */
size_t PageMapGetMaxBufferSize(const PageMap *self);

rc_t PageMapSerialize(const PageMap *self, struct KDataBuffer *buffer, uint64_t byte_offset, uint64_t *actual_bytes);

rc_t PageMapDeserialize(PageMap **lhs, const void *src, uint64_t src_bytes, uint64_t row_count);

rc_t PageMapRelease(const PageMap *self);

rc_t PageMapAddRef(const PageMap *self);

/*  PageMapGetIdxRowInfo
 *
 *  Get row length and starting element number for a row.
 *  This is a potentially expensive operation; the first time
 *  it is called for any page map, the memory used by the page
 *  map nearly doubles and an index is built on the row length
 *  runs and the data runs.  Subsequent calls are O(N log N).
 *  However, asking for the information about the first row
 *  (i.e. idx = 0) never causes allocation or indexing and is
 *  always O(1).
 *
 *  Params:
 *      self: [IN] the page map
 *      idx:  the row number starting from 0
 *      starting_element: [OUT, OPTIONAL]
 *
 *  Returns:
 *      the length of the row
 *      or 0 if not found
 */
uint32_t PageMapGetIdxRowInfo(const PageMap *self, uint32_t idx, uint32_t *starting_element);

rc_t PageMapNew(PageMap **lhs, uint32_t reserve);

rc_t PageMapNewSingle(PageMap **lhs, uint64_t row_count, uint64_t row_length);

rc_t PageMapNewFixedRowLength(PageMap **lhs, uint64_t row_count, uint64_t row_len);

rc_t PageMapToRandomAccess(PageMap **rslt, PageMap * src,uint32_t *data_offset);

uint32_t PageMapFixedRowLength(const PageMap *self);

rc_t	PageMapRowLengthRange(const PageMap *self, elem_count_t *min,elem_count_t *max);

uint32_t PageMapHasSimpleStructure(const PageMap *self);

rc_t PageMapAppendRows(PageMap *self, uint64_t row_length, uint64_t run_length, bool same_data);

#define PageMapAppendRow(SELF, ROW_LENGTH, SAME_DATA) (PageMapAppendRows((SELF), (ROW_LENGTH), 1, SAME_DATA))

/* append some rows of the same data */
#define PageMapAppendSomeRows(SELF, ROW_LENGTH, RUN_LENGTH) (PageMapAppendRows((SELF), (ROW_LENGTH), (RUN_LENGTH), false))

/* concatenate two page maps */
rc_t PageMapAppend(PageMap *self, const PageMap *other);

/** Find data using pagemap ***/
rc_t PageMapFindRow(const PageMap *cself,uint64_t row,uint32_t * data_offset,uint32_t * data_length,uint32_t * repeat_count);


/*
 -1: error
 0: not same
 1: compatible (i.e. all rows same length)
 else: identical
 */
int PageMapCompare(const PageMap *a, const PageMap *b);
/* same but static columns */
int PageMapCompareStatic(const PageMap *a, const PageMap *b);

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
    PageMap *parent;
#endif
};

rc_t PageMapNewIterator(const PageMap *self, PageMapIterator *lhs, uint64_t first_row, uint64_t num_rows);

static __inline__ bool PageMapIteratorAdvance(PageMapIterator *self, row_count_t rows)
{
    if (self->cur_row + rows < self->last_row) {
	self->cur_row += rows;
	self->cur_rgn_row += rows;
	if(self->repeat_count > rows) self->repeat_count-= rows;
	else self->repeat_count = 0;
	if(self->rgns){/** non-static, non simple random access**/
		while((*self->rgns)[self->cur_rgn].numrows <= self->cur_rgn_row){
			self->cur_rgn_row -= (*self->rgns)[self->cur_rgn].numrows;
			self->cur_rgn++;
		}
        }
        return true;
    }
    return false;
}

#define PageMapIteratorNext(SELF) PageMapIteratorAdvance(SELF, 1)

static __inline__ elem_count_t PageMapIteratorDataLength(const PageMapIterator *self)
{
    elem_count_t datalen=0;
    if(self->rgns == NULL) {/* static or simple random-access*/
	return  self->static_datalen;
    }
    switch ((*self->rgns)[self->cur_rgn].type){
	case PM_REGION_EXPAND_FULL:
		if((*self->rgns)[self->cur_rgn].expanded){
			datalen = (*self->exp_base)[(*self->rgns)[self->cur_rgn].data_offset + 2*self->cur_rgn_row];
		} else {
			datalen = (*self->rgns)[self->cur_rgn].length;
		}
                break;
	case PM_REGION_EXPAND_SAMELEN:
	case PM_REGION_EXPAND_EQUIDISTANT:
	case PM_REGION_EXPAND_SAMEDATA:
		datalen = (*self->rgns)[self->cur_rgn].length;
		break;
	default:
		assert(0);
		break;
   }
#if _HEAVY_PAGEMAP_DEBUGGING
   {
	elem_count_t dtl,dto,dtr;
	PageMapFindRow(self->parent,self->cur_row,&dto,&dtl,&dtr);
	assert(dtl==datalen);
   }
#endif
   /*printf("DATA_LEN=%d\n",datalen);*/
   return datalen;
}

static __inline__ elem_count_t PageMapIteratorDataOffset(const PageMapIterator *self)
{
    elem_count_t dataoff=0;
    if(self->rgns == NULL){ /** static or simple random **/
	if(self->exp_base != NULL)  /** simple random access */
		dataoff= (*self->exp_base)[self->cur_row];
	return dataoff;
    }
    switch ((*self->rgns)[self->cur_rgn].type){
        case PM_REGION_EXPAND_FULL:
                if((*self->rgns)[self->cur_rgn].expanded){
                        dataoff = (*self->exp_base)[(*self->rgns)[self->cur_rgn].data_offset + 2*self->cur_rgn_row + 1];
                } else {
                        dataoff = (*self->rgns)[self->cur_rgn].data_offset;
                }
                break;
        case PM_REGION_EXPAND_SAMELEN:
		if((*self->rgns)[self->cur_rgn].expanded){
                        dataoff = (*self->exp_base)[(*self->rgns)[self->cur_rgn].data_offset + self->cur_rgn_row];
                } else {
                        dataoff = (*self->rgns)[self->cur_rgn].data_offset;
                }
                break;
        case PM_REGION_EXPAND_EQUIDISTANT:
		dataoff = (*self->rgns)[self->cur_rgn].data_offset + (*self->rgns)[self->cur_rgn].length * self->cur_rgn_row;
		break;
        case PM_REGION_EXPAND_SAMEDATA:
                dataoff = (*self->rgns)[self->cur_rgn].data_offset;
                break;
        default:
                assert(0);
                break;
    }
#if _HEAVY_PAGEMAP_DEBUGGING
   {
        elem_count_t dtl,dto,dtr;
        PageMapFindRow(self->parent,self->cur_row,&dto,&dtl,&dtr);
        assert(dto==dataoff);
   }
#endif
    return dataoff;
}

static __inline__ row_count_t PageMapIteratorRepeatCount(const PageMapIterator *cself)
{
    if(cself->repeat_count==0){
	PageMapIterator *self = (PageMapIterator*) cself;
	if(self->rgns==NULL){ /** must be simple random access **/
		uint64_t i;
		assert(*self->exp_base);
		for(i=self->cur_row+1,self->repeat_count=1;
                    i< self->last_row && (*self->exp_base)[i]==(*self->exp_base)[self->cur_row];
                    i++,self->repeat_count++){}
	} else {
		switch ((*self->rgns)[self->cur_rgn].type){
		 case PM_REGION_EXPAND_FULL:
			if((*self->rgns)[self->cur_rgn].expanded){
				row_count_t i;
				elem_count_t* base = (*self->exp_base) + (*self->rgns)[self->cur_rgn].data_offset;
				self->repeat_count = 1;
				for(i=self->cur_rgn_row+1;i<(*self->rgns)[self->cur_rgn].numrows;i++){
					if(base[2*self->cur_rgn_row]== base[2*i] && base[2*self->cur_rgn_row+1]== base[2*i+1]) self->repeat_count++;
					else break;
				}
			} else {
				self->repeat_count =  (*self->rgns)[self->cur_rgn].numrows - self->cur_rgn_row;
			}
			break;
		 case PM_REGION_EXPAND_SAMELEN:
			if((*self->rgns)[self->cur_rgn].expanded){
				row_count_t i;
				elem_count_t* base = (*self->exp_base) + (*self->rgns)[self->cur_rgn].data_offset;
				self->repeat_count = 1;
				for(i=self->cur_rgn_row+1;i<(*self->rgns)[self->cur_rgn].numrows;i++){
					if(base[self->cur_rgn_row] == base[i]) self->repeat_count++;
					else break;
				}
			} else {
				self->repeat_count = (*self->rgns)[self->cur_rgn].numrows - self->cur_rgn_row;
			}
			break;
		 case PM_REGION_EXPAND_EQUIDISTANT:
			self->repeat_count = 1;
			break;
		 case PM_REGION_EXPAND_SAMEDATA:
			self->repeat_count =  (*self->rgns)[self->cur_rgn].numrows - self->cur_rgn_row;
			break;
		 default:
			assert(0);
			break;
		}
	}
    }
#if _HEAVY_PAGEMAP_DEBUGGING
   {
        elem_count_t dtl,dto,dtr;
        PageMapFindRow(cself->parent,cself->cur_row,&dto,&dtl,&dtr);
        assert(dtr==cself->repeat_count);
   }
#endif
    return cself->repeat_count;
}

elem_count_t PageMapLastLength(const PageMap *cself);
bool PageMapHasRows(const PageMap *self);
rc_t PageMapExpand(const PageMap *cself, row_count_t upto);
rc_t PageMapExpandFull(const PageMap *cself);
rc_t PageMapPreExpandFull(const PageMap *cself, row_count_t upto);

#endif /* _h_page_map_ */
