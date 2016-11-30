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

#define TRACK_REFERENCES 0

#include <vdb/extern.h>
#include <klib/rc.h>
#include <atomic.h>

#include <bitstr.h>

#include <klib/pack.h>
#include <klib/vlen-encode.h>
#include <sysalloc.h>
#include "page-map.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include <zlib.h>

/* Page maps describe the layout of rows within a blob.
 * The data within a page map is run-length encoded.
 * I.e. if two identical rows are stored sequentially 
 */



#define VALIDATE_PAGEMAPS 0
#define PAGEMAP_STATISTICS 0
#if PAGEMAP_STATISTICS
static struct {
    size_t currentFootprint;
    size_t maxFootprint;
    size_t currentWaste;
    size_t maxWaste;
    unsigned alive;
    unsigned maxAlive;
    unsigned create;
    unsigned createStatic;
    unsigned createFixedRow;
    unsigned createSingle;
    unsigned grows;
    unsigned expands;
    unsigned expandCalls;
    unsigned appends;
} pm_stats;
#endif

elem_count_t PageMapLastLength(const PageMap *cself) {
    return cself->leng_recs > 0 ? cself->length[cself->leng_recs - 1] : 0;
}



static rc_t PageMapRegionExpand(PageMap *self,pm_expand_region_type_t TYPE,row_count_t numrows,elem_count_t length,elem_count_t data_offset)
{
	rc_t rc;
	if(   self->exp_rgn_last == 0           /** not started **/
	   || self->exp_rgn_last->type != TYPE 	/** another type **/
           || (TYPE == PM_REGION_EXPAND_SAMEDATA && self->exp_rgn_last->data_offset != data_offset)  /** not the same data */
           || (TYPE!=PM_REGION_EXPAND_FULL && self->exp_rgn_last->length != length) /** all non-full types are length-dependant**/
		){ /** New region **/
		self->exp_rgn_cnt++;
		rc = KDataBufferResize(&self->istorage, self->exp_rgn_cnt);
		if(rc) return rc;

		self->exp_rgn_last = (PageMapRegion *)self->istorage.base + self->exp_rgn_cnt -1;
		self->exp_rgn_last->type=TYPE;
		self->exp_rgn_last->start_row=self->exp_row_last;
		self->exp_rgn_last->data_offset = data_offset;
		self->exp_rgn_last->length = length;
		self->exp_rgn_last->numrows = numrows;
		self->exp_rgn_last->expanded = false;
 	} else { /** continue with existing region **/
		if(TYPE == PM_REGION_EXPAND_FULL){
			row_count_t i;
			elem_count_t *base;
			uint64_t old_cnt = self->dstorage.elem_count;
			uint64_t new_cnt = old_cnt + 2*numrows;
			if(!self->exp_rgn_last->expanded){/*** need to start expansion ***/
				new_cnt += 2*self->exp_rgn_last->numrows; /*** need to catch up **/
				rc = KDataBufferResize(&self->dstorage, new_cnt);
				if(rc) return rc;
				base = (elem_count_t *)self->dstorage.base + old_cnt;
				for(i=0;i<self->exp_rgn_last->numrows;i++){
					base[2*i] = self->exp_rgn_last->length;
					base[2*i+1] = self->exp_rgn_last->data_offset;
				}
				self->exp_rgn_last->data_offset = old_cnt;
				self->exp_rgn_last->expanded=true;
			} else {
				rc = KDataBufferResize(&self->dstorage, new_cnt);
				if(rc) return rc;
				base = (elem_count_t *)self->dstorage.base + self->exp_rgn_last->data_offset;
			}
			base += 2*self->exp_rgn_last->numrows;
			for(i=0;i<numrows;i++){
				base[2*i]   = length;
				base[2*i+1] = data_offset;
			}
		} else if(TYPE == PM_REGION_EXPAND_SAMELEN){
			 row_count_t i;
                        elem_count_t *base;
                        uint64_t old_cnt = self->dstorage.elem_count;
                        uint64_t new_cnt = old_cnt + numrows;
                        if(!self->exp_rgn_last->expanded){/*** need to start expansion ***/
                                new_cnt += self->exp_rgn_last->numrows; /*** need to catch up **/
                                rc = KDataBufferResize(&self->dstorage, new_cnt);
                                if(rc) return rc;
                                base = (elem_count_t *)self->dstorage.base + old_cnt;
                                for(i=0;i<self->exp_rgn_last->numrows;i++){
                                        base[i] = self->exp_rgn_last->data_offset;
                                }
                                self->exp_rgn_last->data_offset = old_cnt;
				self->exp_rgn_last->expanded=true;
                        } else {
				rc = KDataBufferResize(&self->dstorage, new_cnt);
				if(rc) return rc;
                                base = (elem_count_t *)self->dstorage.base + self->exp_rgn_last->data_offset;
                        }
			base += self->exp_rgn_last->numrows;
                        for(i=0;i<numrows;i++){
                                base[i]   = data_offset;
                        }
		}
		self->exp_rgn_last->numrows += numrows;
	}
	return 0;
}

rc_t PageMapPreExpandFull(const PageMap *cself, row_count_t upto) /*** mostly for use as a temporary pagemap ***/
{
	rc_t    rc=0;
	if(upto > 0){
		PageMap *self = (PageMap *)cself;
		assert(self->exp_rgn_last == 0);
		rc = KDataBufferResize(&self->istorage, 1); /** one and only one pagemap **/
		if(rc) return rc;
		
		self->exp_rgn_last = (PageMapRegion *)self->istorage.base + self->exp_rgn_cnt;
		self->exp_rgn_cnt = ++self->exp_dr_last;

		self->exp_rgn_last->data_offset = 0;
		rc=KDataBufferResize(&self->dstorage, upto*2);
		if(rc) return rc;
		self->exp_rgn_last->start_row   = 0;
		self->exp_rgn_last->numrows = 0;
		self->exp_rgn_last->type = PM_REGION_EXPAND_FULL;
		self->exp_rgn_last->expanded = true;
		self->pre_exp_row_count = upto;
	}
	return rc;
}

rc_t PageMapExpandFull(const PageMap *cself)
{
	rc_t rc;
	PageMap *self = (PageMap *)cself;
	if(self->leng_recs == 1){ 
		if(   self->data_recs == 1  /* static */
                   || self->random_access ){ 
			return 0;
		} else if(self->data_recs == self->row_count && !self->random_access){ /** equidistant **/
			rc=PageMapRegionExpand(self,PM_REGION_EXPAND_EQUIDISTANT,self->row_count,self->length[0],0);
			if(rc) return rc;
			self->exp_data_offset_last += self->row_count*self->length[0];
			self->exp_row_last = self->row_count ;
			self->exp_dr_last =  self->row_count;
			self->exp_lr_last =  1;
			self->exp_lr_used =  self->row_count;
			return  0;
		}
        }
	return 0;
/*** OTHER VARIANTS  ***
	return PageMapExpand(cself, cself->row_count);
	return PageMapExpand(cself, cself->row_count < 1024?cself->row_count:1024);
******************/
}

rc_t PageMapExpand(const PageMap *cself, row_count_t upto)
{
	rc_t	rc;
        PageMap *self = (PageMap *)cself;
#define LENG_RUN_TRIGGER 8
#define DATA_RUN_TRIGGER 8
#define EQUI_RUN_TRIGGER 8
	if( self->leng_recs == 1 && self->row_count > self->data_recs*12/10 && !self->random_access){ /*** Shortcut to make tight loop ***/

		if(self->exp_rgn_last == 0){
			rc = KDataBufferResize(&self->istorage, self->data_recs);
			if(rc) return rc;
		}
		self->exp_rgn_last = (PageMapRegion *)self->istorage.base + self->exp_rgn_cnt;
		while(self->exp_dr_last < self->data_recs && self->exp_row_last <= upto + 128){
			self->exp_rgn_last->start_row   = self->exp_row_last;
			self->exp_rgn_last->data_offset = self->length[0]*self->exp_dr_last;
			self->exp_rgn_last->numrows = self->data_run?self->data_run[self->exp_dr_last]:1;
			self->exp_row_last += self->data_run?self->data_run[self->exp_dr_last]:1;
			self->exp_rgn_last->type = PM_REGION_EXPAND_SAMEDATA;
			self->exp_rgn_last->length      = self->length[0];
			self->exp_rgn_last->expanded = false;
			self->exp_dr_last  ++;
			self->exp_rgn_last ++;
		}
		self->exp_rgn_cnt = self->exp_dr_last;
	} else for(;self->exp_lr_last < self->leng_recs && self->exp_dr_last < self->data_recs && self->exp_row_last <= upto + 128;){
		row_count_t leng_run=self->leng_run[self->exp_lr_last] - self->exp_lr_used;
		elem_count_t length =self->length[self->exp_lr_last];
		if(leng_run == 0) {
			self->exp_lr_last++;
			self->exp_lr_used=0;
			continue;
		}
		if(leng_run >= LENG_RUN_TRIGGER && !self->random_access){
			while(leng_run > 0 && self->exp_row_last <= upto + 128){
				row_count_t data_run=self->data_run?self->data_run[self->exp_dr_last]:1;
				assert(leng_run >= data_run);/** data runs should have the same lengths **/
				assert(data_run > 0);
				if(data_run >= DATA_RUN_TRIGGER){
					rc=PageMapRegionExpand(self,PM_REGION_EXPAND_SAMEDATA,data_run, length,self->exp_data_offset_last);
					if(rc) return rc;
					assert(leng_run >= data_run);
					leng_run -= data_run;
					self->exp_lr_used += data_run;
					self->exp_row_last+= data_run; 
					self->exp_dr_last++;
					self->exp_data_offset_last += length;
				} else if( /**** quick check if all remaining rows are equidistant ****/
					     self->exp_lr_last == self->leng_recs - 1 /*** last leng_run ***/
					  && self->row_count - self->exp_row_last == self->data_recs - self->exp_dr_last /*** all remaining data_runs are 1 ***/
					  && self->row_count - self->exp_row_last > EQUI_RUN_TRIGGER){ 
						assert( leng_run == self->row_count - self->exp_row_last);
						assert(data_run == 1);
						rc=PageMapRegionExpand(self,PM_REGION_EXPAND_EQUIDISTANT,leng_run, length,self->exp_data_offset_last);
						if(rc) return rc;
						self->exp_data_offset_last += leng_run*length;
						self->exp_row_last = self->row_count ;
						self->exp_dr_last =  self->data_recs;
						self->exp_lr_last = self->leng_recs;
						self->exp_lr_used += leng_run;
						leng_run = 0;
						
				} else {
					if( data_run == 1){
						row_count_t data_run_unique;
						for( data_run_unique =0;
						     data_run_unique < leng_run && ((self->data_run==NULL)||self->data_run[self->exp_dr_last+data_run_unique]==1);
						     data_run_unique++){}
						if(data_run_unique >= EQUI_RUN_TRIGGER){
							rc=PageMapRegionExpand(self,PM_REGION_EXPAND_EQUIDISTANT,data_run_unique,length,self->exp_data_offset_last);
							self->exp_dr_last += data_run_unique;
							self->exp_data_offset_last += data_run_unique*length;
							assert(leng_run >= data_run_unique);
							leng_run -= data_run_unique;
						        self->exp_lr_used  += data_run_unique;
							self->exp_row_last += data_run_unique;
						} else while(data_run_unique){
							rc=PageMapRegionExpand(self,PM_REGION_EXPAND_SAMELEN,data_run,length,self->exp_data_offset_last);
							if(rc) return rc;
							assert(leng_run > 0);
							leng_run --;
							self->exp_lr_used ++;
							self->exp_row_last++;
							self->exp_dr_last++;
							self->exp_data_offset_last += length;
							data_run_unique--;
						}
					} else {
						rc=PageMapRegionExpand(self,PM_REGION_EXPAND_SAMELEN,data_run,length,self->exp_data_offset_last);
						if(rc) return rc;
						assert(leng_run >= data_run);
						leng_run -= data_run;
						self->exp_lr_used += data_run;
						self->exp_row_last+= data_run;
						self->exp_dr_last++;
						self->exp_data_offset_last += length;
					}
				}
			}
			assert(self->exp_row_last <= self->row_count);
		} else {
			while(leng_run > 0){
				row_count_t data_run=cself->data_run?cself->data_run[self->exp_dr_last]:1;
				assert(leng_run >= data_run);/** data runs should have the same lengths **/
				if(self->random_access){
					rc=PageMapRegionExpand(self,PM_REGION_EXPAND_FULL,data_run,length,self->data_offset[self->exp_row_last]);
					self->exp_data_offset_last  = self->data_offset[self->exp_row_last] + length;
				} else {
					rc=PageMapRegionExpand(self,PM_REGION_EXPAND_FULL,data_run,length,self->exp_data_offset_last);
					self->exp_data_offset_last += length;
				}
				if(rc) return rc;
				self->exp_row_last += data_run;
				assert(leng_run >= data_run);
				leng_run -= data_run;
				self->exp_lr_used += data_run;
				self->exp_dr_last++;
			}
		}
	}
	return 0;
}

static rc_t PageMapFindRegion(const PageMap *cself,uint64_t row,PageMapRegion **pmr)
{
	/*** in PageMap rows are 0-based **/
	rc_t	rc;
	pm_size_t left,right,i_rgn;
	if(row >= cself->row_count )
		return  RC (rcVDB, rcPagemap, rcSearching, rcRow, rcNotFound );
	if(cself -> exp_row_last <= row){
		rc=PageMapExpand(cself,row);
		if(rc) return rc;
	}

	if(cself->exp_rgn_cnt > 1){
		i_rgn = cself->i_rgn_last;
		left = 0;
		right = cself->exp_rgn_cnt - 1;
		while(right > left){
			PageMapRegion*  rgn=  (PageMapRegion*)cself->istorage.base + i_rgn;
			assert(i_rgn < cself->exp_rgn_cnt);
			if(row < rgn->start_row){
				right = i_rgn-1;
				i_rgn  = (left + right) / 2;
			} else if(row == rgn->start_row + rgn->numrows){ /*** special case for positive sequentual scans ***/
				i_rgn++;
			} else if(row > rgn->start_row + rgn->numrows){
				left = i_rgn+1;
				i_rgn =  (left + right + 1) / 2;
			} else {
				break;
			}
		}
	} else {
		i_rgn = 0;
	}
	{
		PageMap *self = (PageMap *)cself;
		self->i_rgn_last = i_rgn;
		self->rgn_last = (PageMapRegion*)self->istorage.base+cself->i_rgn_last;
	}
	assert(cself->rgn_last->start_row <= row);
	assert(cself->rgn_last->start_row + cself->rgn_last->numrows > row);
	if(pmr) *pmr=(PageMapRegion*)cself->istorage.base + i_rgn;
	return 0;
}

static rc_t PageMapRegionGetData(const PageMapRegion *cself,const elem_count_t *exp_base,uint64_t row,uint32_t * data_offset,uint32_t * data_length,uint32_t * repeat_count)
{
	if(row < cself->start_row || row >= cself->start_row + cself->numrows)
		return  RC (rcVDB, rcPagemap, rcSearching, rcRow, rcNotFound );

	if(data_offset != NULL || data_length != NULL || repeat_count != NULL){
		if(!cself->expanded){
			if(data_length) *data_length=cself->length;
			if(data_offset){
				*data_offset=cself->data_offset;
				if(cself->type == PM_REGION_EXPAND_EQUIDISTANT){
					*data_offset += cself->length * (row - cself->start_row);
				}
			}
			if(repeat_count) {
				if(cself->type == PM_REGION_EXPAND_EQUIDISTANT) *repeat_count=1;
				else *repeat_count = cself->numrows + cself->start_row - row;
			}
		} else switch(cself->type){
			case PM_REGION_EXPAND_FULL:
				{
					row_count_t  i = row - cself->start_row;
					exp_base += cself->data_offset;
					if(data_length)  *data_length = exp_base[2*i];
					if(data_offset)  *data_offset = exp_base[2*i+1];
					if(repeat_count){
						int j;
						for(j=i+1;   j<cself->numrows
							  && exp_base[2*i]   == exp_base[2*j]
							  && exp_base[2*i+1] == exp_base[2*j+1];j++){}
						*repeat_count = j-i;
					}
				}
				break;
			case PM_REGION_EXPAND_SAMELEN:
				if(data_length) *data_length=cself->length;
				if(data_offset || repeat_count){
					row_count_t  i = row - cself->start_row;
					exp_base += cself->data_offset;
					if(data_offset) *data_offset = exp_base[i];
					if(repeat_count){
						int j;
						for(j=i+1;j<cself->numrows && exp_base[i] == exp_base[j];j++){}
						*repeat_count = j-i;
					}
				}
				break;
			default: /*** should not happen to anything but 2 above **/
				return RC (rcVDB, rcPagemap, rcSearching, rcData, rcInconsistent ); 
		}
	}
	return 0;
}

/*** This is a heavy-weight function which gets data by integration all the time - useful for debugging ****/
#if _HEAVY_PAGEMAP_DEBUGGING
static rc_t PageMapGetData_HW(const PageMap *cself,uint64_t row,uint32_t * data_offset,uint32_t * data_length,uint32_t * repeat_count)
{
	if(row >= cself->row_count)
		return  RC (rcVDB, rcPagemap, rcSearching, rcRow, rcNotFound );
	if(data_offset != NULL || data_length != NULL || repeat_count != NULL){
		uint64_t i,j,r;
		uint32_t dto,dtl,dtr;
		uint32_t leng_run,data_run;
		r=dto=i=j=0;
		leng_run=cself->leng_run[0];
		while(1){
			dtl  = cself->length[i];
			data_run=cself->data_run?cself->data_run[j]:1;
			j++;
			r+= data_run;
			if(r > row){
				dtr = r - row;
				 break;
			}
			dto += dtl;
			leng_run -= data_run;
			if(leng_run == 0){
				i++;
				leng_run=cself->leng_run[i];
			}
		}
		if(data_length)   *data_length=dtl;
		if(data_offset)   *data_offset=dto;
		if(repeat_count)  *repeat_count=dtr;
	}
	return 0;
}
#endif

/*** another function to help debugging ***/
#if 0
static int DEBUG_PageMapPrintSum(const PageMap *pm, uint32_t row)
{
	int i,r;
	for(i=0,r=row; i < pm->data_recs && r > 0;r-=pm->data_run?pm->data_run[i]:1,i++){}
	printf("Data recs= <%d>, rows_left = <%d>\n",i,r);
	for(i=0,r=row; i < pm->leng_recs && r >0;r-=pm->leng_run[i++]){}
	printf("Leng recs= <%d>, rows_left = <%d>\n",i,r);
	return 0;
}
#endif

rc_t PageMapFindRow(const PageMap *cself,uint64_t row,uint32_t * data_offset,uint32_t * data_length,uint32_t * repeat_count)
{
	rc_t	rc=0;
	PageMapRegion *pmr;

	if(cself->data_recs == 1){ /** static **/
		if(repeat_count) *repeat_count = UINT32_MAX;
		if(data_offset)  *data_offset = 0;
		if(data_length)  *data_length = cself->length[0];
		return 0;
        }
	if(cself->random_access && cself->leng_recs == 1){
		if(row >= cself->data_recs)
			return RC(rcVDB, rcPagemap, rcAccessing, rcRow, rcOutofrange);
		if(data_length)  *data_length = cself->length[0];
		if(data_offset)  *data_offset =  cself->data_offset[row];
		if(repeat_count){
			uint64_t i;
			for( i=row+1, repeat_count[0]=1; i < cself->data_recs &&  cself->data_offset[i] == cself->data_offset[row]; i++, repeat_count[0]++ ){}
		}
		return 0;
	}

	rc = PageMapFindRegion(cself,row,&pmr);
	if(rc) return rc;

        rc = PageMapRegionGetData(pmr,cself->dstorage.base,row,data_offset,data_length,repeat_count);
	if(rc) return rc;

#if _HEAVY_PAGEMAP_DEBUGGING
	{
		uint32_t dto,dtl,dtr;
		rc = PageMapGetData_HW(cself,row,&dto,&dtl,&dtr);
		assert(rc == 0);
		if(data_length) assert(dtl==*data_length);
		if(data_offset) assert(dto==*data_offset);
		if(repeat_count)  assert(dtr==*repeat_count);
	}
#endif
	return 0;
}

LIB_EXPORT rc_t PageMapNewIterator(const PageMap *self, PageMapIterator *lhs, uint64_t first_row, uint64_t num_rows)
{
    rc_t rc;

    if (first_row + num_rows > self->row_count)
        num_rows = self->row_count - first_row;

#if _DEBUGGING
    if (self->data_recs > 1) {
        assert(first_row == (row_count_t)first_row);
        assert(num_rows == (row_count_t)num_rows);
        assert(first_row + num_rows == (row_count_t)(first_row + num_rows));
    }
#endif
    
    memset(lhs, 0, sizeof(*lhs));

#if _HEAVY_PAGEMAP_DEBUGGING
    lhs->parent = self;
#endif

    lhs->last_row = first_row + num_rows;
    lhs->cur_row = first_row;
    if(self->data_recs == 1){
	lhs->repeat_count  = (num_rows < UINT32_MAX)?num_rows:UINT32_MAX;
	lhs->static_datalen = self->length[0];
	return 0;
    }
    if(self->random_access && self->leng_recs == 1 && first_row < self->row_count){ /** simple random access - no explosion needed **/
	lhs->static_datalen = self->length[0];
	lhs->exp_base = ( elem_count_t ** )&self->data_offset;
	if(lhs->last_row > self->row_count) lhs->last_row = self->row_count;
	return 0;
    }
    if( self->exp_row_last < lhs->last_row){
	    rc = PageMapExpand(self,lhs->last_row-1);
	    if(rc) return rc;
    }
    rc = PageMapFindRow(self,first_row,NULL,NULL,NULL);
    if(rc) return rc;
    lhs->rgns    = (PageMapRegion**) &self->istorage.base;
    lhs->exp_base = (elem_count_t**) &self->dstorage.base;
    lhs->cur_rgn  = self->i_rgn_last;
    lhs->cur_rgn_row = lhs->cur_row - (*lhs->rgns)[self->i_rgn_last].start_row;
    assert(lhs->cur_rgn_row < (*lhs->rgns)[self->i_rgn_last].numrows);
    return  0;
}

static PageMap *new_PageMap(void) {

    PageMap *y;
    y = malloc(sizeof(*y));
    if (y) {
	memset(y,0,sizeof(*y));
        KRefcountInit(&y->refcount, 1, "PageMap", "new", "");
	y->istorage.elem_bits = sizeof(PageMapRegion)*8;
	y->dstorage.elem_bits = sizeof(elem_count_t)*8;
    }
    return y;
}

static PageMap *new_StaticPageMap(unsigned length, unsigned data) {
    union {
        PageMap pm;
        row_count_t rcnt;
        elem_count_t ecnt;
    } *y;
    size_t const sz = sizeof(*y)
                    + sizeof(y->pm.length[0]) * length
                    + sizeof(y->pm.leng_run[0]) * length
                    + sizeof(y->pm.data_run[0]) * data;
    
    y = malloc(sz);
    if (y) {
#if PAGEMAP_STATISTICS
        ++pm_stats.createStatic;
        ++pm_stats.alive;
        if (pm_stats.maxAlive < pm_stats.alive)
            pm_stats.maxAlive = pm_stats.alive;
        pm_stats.currentFootprint += sz;
        if (pm_stats.maxFootprint < pm_stats.currentFootprint)
            pm_stats.maxFootprint = pm_stats.currentFootprint;
#endif
        memset(&y->pm, 0, sizeof(y->pm));
        KRefcountInit(&y->pm.refcount, 1, "PageMap", "new_Static", "");
        y->pm.length = (elem_count_t *)&y[1];
        y->pm.leng_run = (row_count_t *)&y->pm.length[length];
        y->pm.data_run = &y->pm.leng_run[length];
        y->pm.reserve_leng = length;
        y->pm.reserve_data = data;
	y->pm.istorage.elem_bits = sizeof(PageMapRegion)*8;
	y->pm.dstorage.elem_bits = sizeof(elem_count_t)*8;
    }
    return &y->pm;
}

static
rc_t PageMapGrow(PageMap *self, uint32_t new_reserve_leng, uint32_t new_reserve_data) {
    uint32_t sz;
    PageMap temp = *self;
    uint32_t reserve_data = self->reserve_data;
    uint32_t reserve_leng = self->reserve_leng;
    rc_t rc;

#if PAGEMAP_STATISTICS
    ++pm_stats.grows;
#endif
    if (new_reserve_leng > (1UL << 31) || new_reserve_data > (1UL << 31))
        return RC(rcVDB, rcPagemap, rcAllocating, rcParam, rcExcessive);
    
    if (reserve_leng == 0)
        reserve_leng = 1;
    if (reserve_data == 0)
        reserve_data = 1;
#define MIN_KBUF_RESERVE_SIZE 256  
    while (reserve_leng < new_reserve_leng)
        reserve_leng <<= 1;
    if (reserve_leng < MIN_KBUF_RESERVE_SIZE) reserve_leng = MIN_KBUF_RESERVE_SIZE;
    while (reserve_data < new_reserve_data)
        reserve_data <<= 1;
    if (reserve_data < MIN_KBUF_RESERVE_SIZE) reserve_data = MIN_KBUF_RESERVE_SIZE;

    sz = reserve_leng * 2 + reserve_data;
#if PAGEMAP_STATISTICS
    pm_stats.currentFootprint -= KDataBufferMemorySize(&self->estorage);
#endif
    {
        KDataBuffer new_buffer;
        
        rc = KDataBufferMake(&new_buffer, 8 * sizeof(uint32_t), sz);
        if (rc)
            return rc;
#if PAGEMAP_STATISTICS
        pm_stats.currentFootprint += KDataBufferMemorySize(&self->estorage);
        if (pm_stats.maxFootprint < pm_stats.currentFootprint)
            pm_stats.maxFootprint = pm_stats.currentFootprint;
#endif
        self->cstorage = new_buffer;
    }
    self->length = self->cstorage.base;
    self->leng_run = self->length + reserve_leng;
    self->data_run = self->leng_run + reserve_leng;
    self->start_valid = 0;
    
    if (self->leng_recs > 0 && temp.length != NULL) {
        memmove(self->length  , temp.length  , self->reserve_leng * sizeof(uint32_t));
        memmove(self->leng_run, temp.leng_run, self->reserve_leng * sizeof(uint32_t));
    }
    if (self->data_recs > 0 && temp.data_run != NULL)
        memmove(self->data_run, temp.data_run, self->reserve_data * sizeof(uint32_t));

    self->reserve_leng = reserve_leng;
    self->reserve_data = reserve_data;
#if PAGEMAP_STATISTICS
    pm_stats.currentWaste += (self->reserve_data - self->data_recs) * sizeof(self->data_run[0]) +
                             (self->reserve_leng - self->leng_recs) * (sizeof(self->leng_run[0]) + sizeof(self->length[0]));
    if (pm_stats.maxWaste < pm_stats.currentWaste)
        pm_stats.maxWaste = pm_stats.currentWaste;
#endif
    KDataBufferWhack(&temp.cstorage);

    return 0;
}

rc_t PageMapNew(PageMap **lhs, uint32_t reserve) {
    PageMap *y = new_PageMap();

    if (y == NULL)
        return RC(rcVDB, rcPagemap, rcConstructing, rcMemory, rcExhausted);

    if (reserve > 0) {
        rc_t rc = PageMapGrow(y, reserve, reserve);
        if (rc) {
            free(y);
            return rc;
        }
#if PAGEMAP_STATISTICS
        --pm_stats.grows;
#endif
    }
    *lhs = y;
#if PAGEMAP_STATISTICS
    ++pm_stats.create;
    ++pm_stats.alive;
    if (pm_stats.maxAlive < pm_stats.alive)
        pm_stats.maxAlive = pm_stats.alive;
    pm_stats.currentFootprint += sizeof(*y);
    if (pm_stats.maxFootprint < pm_stats.currentFootprint)
        pm_stats.maxFootprint = pm_stats.currentFootprint;
#endif
    return 0;
}

rc_t PageMapToRandomAccess(PageMap **rslt, PageMap * src,uint32_t *data_offset) /** data_offset should have dimension of data_run ***/
{
	rc_t rc;
	bool simple = (src->row_count == src->data_recs);
	PageMap *dst;
	assert(src->row_count >= src->leng_recs);
	rc = PageMapNew(&dst, 0);
	if(rc == 0){
		dst->leng_recs=src->leng_recs;
		dst->row_count=src->row_count;
		rc = KDataBufferMake(&dst->cstorage, 8 * sizeof(uint32_t), 2*dst->leng_recs+(data_offset?dst->row_count:0));
		if(rc == 0){
			dst->length = dst->cstorage.base;
			dst->leng_run    = dst->length   + dst->leng_recs;
			dst->data_recs = src->row_count;
			memmove(dst->length,  src->length,  sizeof(uint32_t)*dst->leng_recs);
			memmove(dst->leng_run,src->leng_run,sizeof(uint32_t)*dst->leng_recs);
			if(data_offset){
				dst->data_offset = dst->leng_run + dst->leng_recs;
				if(simple){
					memmove(dst->data_offset,data_offset,sizeof(uint32_t)*dst->row_count);
				} else {
					uint32_t i,j;
					for(i=j=0;i<src->data_recs;i++){
						elem_count_t data_len=src->data_run[i];
						while(data_len > 0){
							assert(j < src->row_count);
							dst->data_offset[j++] = data_offset[i];
							data_len--;
						}
					}
				}
				dst->random_access=true;
			} 
			dst->reserve_leng = dst->leng_recs;
			dst->reserve_data = dst->row_count;
			dst->start_valid = dst->row_count;
			*rslt = dst;
			return 0;
		}
		PageMapRelease(dst);
	}
	return rc;
}


rc_t PageMapNewFixedRowLength(PageMap **lhs, uint64_t row_count, uint64_t row_len) {
    PageMap *y;
    rc_t rc;

    if (row_count >> 32 != 0 || row_len >> 32 != 0)
        return RC(rcVDB, rcPagemap, rcConstructing, rcParam, rcTooBig);
    
    rc = PageMapNew(&y, 0);
#if PAGEMAP_STATISTICS
    ++pm_stats.createFixedRow;
    --pm_stats.create;
#endif
    if (rc)
        return rc;
    rc = PageMapGrow(y, 1, (uint32_t)row_count);
#if PAGEMAP_STATISTICS
    --pm_stats.grows;
#endif
    if (rc == 0) {
	int i;
        *lhs = y;
        y->length[0] = (uint32_t)row_len;
        y->leng_run[0] = (uint32_t)row_count;
        y->leng_recs = 1;
        y->data_recs = (uint32_t)row_count;
        y->row_count = (uint32_t)row_count;
        for (i = 0; i != (uint32_t)row_count; ++i) {
            y->data_run[i] = 1;
        }

    }
    if (rc)
        PageMapRelease(y);
    return rc;
}

rc_t PageMapNewSingle(PageMap **lhs, uint64_t row_count, uint64_t row_len) {
    PageMap *y;
    rc_t rc;
    
    if (row_count >> 32 != 0)
        return RC(rcVDB, rcPagemap, rcConstructing, rcParam, rcTooBig);
    
    rc = PageMapNewFixedRowLength(&y, 1, row_len);
    if (rc == 0) {
#if PAGEMAP_STATISTICS
        ++pm_stats.createSingle;
        --pm_stats.createFixedRow;
#endif
        y->row_count = y->data_run[0] = y->leng_run[0] = (uint32_t)row_count;
        *lhs = y;
    }
    return rc;
}

bool PageMapHasRows(const PageMap *self) {
    return self->data_recs > 0;
}

uint32_t PageMapFixedRowLength (const PageMap *self) {
    uint32_t rslt = 0;
    
    if (self->leng_recs == 1)
        rslt = self->length[0];
    return rslt;
}

rc_t PageMapRowLengthRange (const PageMap *self,elem_count_t *min,elem_count_t *max)
{
    int i;
    *min = *max = self->length[0];
    for(i=1;i<self->leng_recs;i++){
	if      (self->length[i] < *min) *min = self->length[i];
	else if (self->length[i] > *max) *max = self->length[i];
    }
    return 0;
}


uint32_t PageMapHasSimpleStructure(const PageMap *self) {
    uint32_t rslt = PageMapFixedRowLength(self);
    
    if (rslt == 0)
        return rslt;

    if(self->data_recs != self->row_count) return 0;

    return rslt;
}

#if 0
static bool PageMapValidate(const PageMap *cself) {
    unsigned i;
    uint32_t n;
    uint32_t m;
        
    for (m = 0, i = 0; i != cself->leng_recs; ++i) m += cself->leng_run[i];
    for (n = 0, i = 0; i != cself->data_recs; ++i) n += cself->data_run[i];

    return m == n ? true : false;
}

static
rc_t PageMapAppendRun(PageMap *self, uint32_t run_length, uint32_t row_length) {
    return -1;
}
#endif

rc_t PageMapAppendRows(PageMap *self, uint64_t row_length, uint64_t run_length, bool same_data) {
    rc_t rc;
    uint32_t leng_cur = self->leng_recs - 1;
    uint32_t data_cur = self->data_recs - 1;
    
    if ((uint32_t)row_length != row_length)
        return RC(rcVDB, rcPagemap, rcConstructing, rcParam, rcTooBig);
    
    if ((uint32_t)run_length != run_length)
        return RC(rcVDB, rcPagemap, rcConstructing, rcParam, rcTooBig);
    
    if (self->leng_recs && row_length == self->length[leng_cur])
        self->leng_run[leng_cur] += run_length;
    else {
        same_data = false;
        leng_cur = self->leng_recs;
        ++self->leng_recs;

#if PAGEMAP_STATISTICS
        pm_stats.currentWaste -= sizeof(self->leng_run[0]) + sizeof(self->length[0]);
#endif
        
        if (self->leng_recs >= self->reserve_leng) {
            rc = PageMapGrow(self, self->leng_recs, 0);
            if (rc)
                return rc;
        }
        self->leng_run[leng_cur] = run_length;
        self->length[leng_cur] = (uint32_t)row_length;
    }
    if (self->data_recs && same_data)
        self->data_run[data_cur] += run_length;
    else {
        data_cur = self->data_recs;
        ++self->data_recs;
        
#if PAGEMAP_STATISTICS
        pm_stats.currentWaste -= sizeof(self->data_run[0]);
#endif
        
        if (self->data_recs >= self->reserve_data) {
            rc = PageMapGrow(self, 0, self->data_recs);
            if (rc)
                return rc;
        }
        self->data_run[data_cur] = run_length;
    }
    if(self->row_count < self->pre_exp_row_count){
	int i;
	elem_count_t *exp_base = self->dstorage.base;
	elem_count_t data_offset;
	if(same_data){
		if(self->row_count == 0){
			data_offset = 0;
		} else {
			data_offset = exp_base[2*self->row_count-1];
		}
	} else {
		data_offset = self->exp_data_offset_last;
		self->exp_data_offset_last += row_length;
	}
	for(i=self->row_count;i<run_length+self->row_count;i++){
		exp_base[2*i]  =  row_length;
		exp_base[2*i+1]=  data_offset;
	}
	self->exp_row_last += run_length;
	self->exp_rgn_last->numrows += run_length;
	
    }
    self->row_count += run_length;
    return 0;
}

static rc_t serialize_lengths(
                              KDataBuffer *Dst,
                              uint64_t offset, 
                              const uint32_t run[],
                              unsigned runs,
                              uint64_t *consumed
) {
    rc_t rc;
    unsigned i;
    uint64_t j;
    uint64_t n;
    uint8_t *dst = Dst->base;
    
    dst += offset;
    *consumed = 0;
    for (i = 0, j = 0, n = 0, rc = 0; rc == 0 && i != runs; ++i, j += n)
        rc = vlen_encodeU1(dst + j, 5, &n, run[i]);
    
    if (rc == 0)
        *consumed = j;
    
    return rc;
}

#if 0

static rc_t deserialize_lengths(
                                uint32_t run[],
                                unsigned runs,
                                uint8_t const src[],
                                unsigned ssize,
                                uint64_t *consumed
) {
    unsigned i;
    unsigned j;
    uint64_t n;
    
    *consumed = 0;
    for (i = 0, j = 0; i != runs; ++i, j += (unsigned)n) {
        uint64_t val;
        rc_t rc = vlen_decodeU1(&val, src + j, ssize - j, &n);
        
        if (rc) return rc;
        run[i] = (uint32_t)val;
    }
    *consumed = j;
    return 0;
}
#else

#if _ARCH_BITS == 32
#define FLAG_64 0x8080808080808080ULL
#define VAL_64  0x7F7F7F7F7F7F7F7FULL
#else
#define FLAG_64 0x8080808080808080UL
#define VAL_64  0x7F7F7F7F7F7F7F7FUL
#endif

static rc_t deserialize_lengths(
                                uint32_t run[],
                                unsigned runs,
                                uint8_t const src[],
                                unsigned ssize,
                                uint64_t *consumed
) {
    uint8_t	to_align = 16 - (((long)src)&15);/** alignmentto 16 bytes will make optimizer and cpu happy. don't care about loss in cast - we need the lowest byte **/
    int		si=0;/**source index **/
    int		di=0;/** destination index **/
    int		pcnt = (ssize - to_align )/8; 

    memset(run,0,runs*sizeof(*run));
    if(pcnt){ /** does it make sense at all ***/
	uint64_t    *ptr=(uint64_t*) (src + to_align);
	int         pi;
	
	for(;si<to_align && di < runs ;si++){ /** align the pointer first **/
		if(((int8_t)src[si]) < 0) {
			run[di] += src[si] & 0x7F;
			run[di] <<= 7;
		} else {
			run[di++] += src[si];
		}
	}	
	assert((((long)ptr)&15) == 0);
	for(pi = 0; pi < pcnt && di < runs; pi++){ /** do bulk staff **/
		uint64_t flag = ptr[pi] & FLAG_64;
		uint64_t  val = ptr[pi] & VAL_64;
		int  i;
		int8_t     *cf = (int8_t*)&flag;
		uint8_t	   *cv = (uint8_t*)&val;
		for(i=0;i<8 && di < runs;i++){ /** no endian problem - casting of ptr matches casting of flag & val **/
			si++;
			run[di] += cv[i];
			if(cf[i] < 0){
				run[di] <<= 7;
			} else {
				di++;
			}
		}
	}
    }
   
    for(;si < ssize && di < runs;si++){/** deal with tails **/
	if(((int8_t)src[si]) < 0) {
		run[di] += src[si] & 0x7F;
		run[di] <<= 7;
	} else {
		run[di++] += src[si];
	}
    }
    *consumed = si;
    return 0;
}

#endif

static
rc_t serialize(const PageMap *self, KDataBuffer *buffer, uint64_t *size) {
/*
 variant 0: fixed row length, data_run[..] == 1
    vlencode(row_length)
 version 2: same as 0 but random access
    adds serialize(data_offset)

 variant 1: fixed row length, variable data_run
    vlencode(row_length)
    vlencode(data_recs)
    serialize(data_run)

 variant 2: variable row length, data_run[..] == 1
    vlencode(leng_recs)
    serialize(length)
    serialize(leng_run)
 versuion 2: same as 2 but random access
    adds serialize(data_offset)
 
 variant 3: variable row length, variable data_run
    vlencode(leng_recs)
    vlencode(data_recs)
    serialize(length)
    serialize(leng_run)
    serialize(data_run)

  

 */
    uint8_t version = self->random_access?2:1;
    uint8_t variant = 0;
    uint8_t header;
    rc_t rc = 0;
    uint64_t sz;
    KDataBuffer compress;



    memset(&compress, 0, sizeof(compress));

    variant = ( (self->data_recs == self->row_count)? 0 : 1) | ((self->leng_recs == 1 ? 0 : 1) << 1);
    header = (version << 2) | variant;
    switch (variant) {
    case 0:
        rc = KDataBufferResize(buffer, 6);
        if (rc == 0) {
            ((uint8_t *)buffer->base)[0] = header;
            vlen_encodeU1(((uint8_t *)buffer->base) + 1, 5, &sz, self->length[0]);
	    buffer->elem_count = sz + 1;
	    if(self->random_access){
		rc = KDataBufferMakeBytes(&compress, 5*self->row_count);
		if(rc == 0){
			rc = serialize_lengths(&compress, 0, self->data_offset, self->row_count, &sz);
			compress.elem_count = sz;
		}
	    }
        }
        break;
    case 1:
        rc = KDataBufferResize(buffer, 11);
        if (rc == 0) {
            rc = KDataBufferMakeBytes(&compress, 5 * self->data_recs);
            if (rc == 0) {
                ((uint8_t *)buffer->base)[0] = header;
                vlen_encodeU1(((uint8_t *)buffer->base) + 1, 5, &sz, self->length[0]);
                buffer->elem_count = sz + 1;
                vlen_encodeU1(((uint8_t *)buffer->base) + 1 + sz, 5, &sz, self->data_recs);
                buffer->elem_count += sz;
                
                rc = serialize_lengths(&compress, 0, self->data_run, self->data_recs, &sz);
                compress.elem_count = sz;
            }
        }
        break;
    case 2:
        rc = KDataBufferResize(buffer, 6);
        if (rc == 0) {
            rc = KDataBufferMakeBytes(&compress, 10 * self->leng_recs + (self->random_access?5*self->row_count:0));
            if (rc == 0) {
                ((uint8_t *)buffer->base)[0] = header;
                vlen_encodeU1(((uint8_t *)buffer->base) + 1, 5, &sz, self->leng_recs);
                buffer->elem_count = sz + 1;

                rc = serialize_lengths(&compress, 0, self->length, self->leng_recs, &sz);
                compress.elem_count = sz;
                if (rc == 0) {
                    rc = serialize_lengths(&compress, compress.elem_count, self->leng_run, self->leng_recs, &sz);
                    compress.elem_count += sz;
		    if(self->random_access && rc == 0) {
                        rc = serialize_lengths(&compress, compress.elem_count, self->data_offset, self->row_count, &sz);
                        compress.elem_count += sz;
		    }
                }
            }
        }
	break;
    case 3:
        rc = KDataBufferResize(buffer, 11);
        if (rc == 0) {
            rc = KDataBufferMakeBytes(&compress, 10 * self->leng_recs + 5 * self->data_recs);
            if (rc == 0) {
                ((uint8_t *)buffer->base)[0] = header;
                vlen_encodeU1(((uint8_t *)buffer->base) + 1, 5, &sz, self->leng_recs);
                buffer->elem_count = sz + 1;
                vlen_encodeU1(((uint8_t *)buffer->base) + 1 + sz, 5, &sz, self->data_recs);
                buffer->elem_count += sz;
                
                rc = serialize_lengths(&compress, 0, self->length, self->leng_recs, &sz);
                compress.elem_count = sz;
                if (rc == 0) {
                    rc = serialize_lengths(&compress, compress.elem_count, self->leng_run, self->leng_recs, &sz);
                    compress.elem_count += sz;
                    if (rc == 0) {
                        rc = serialize_lengths(&compress, compress.elem_count, self->data_run, self->data_recs, &sz);
                        compress.elem_count += sz;
                    }
                }
            }
        }
        break;
    }
    if (rc == 0 && compress.base) {
        uint64_t hsize = buffer->elem_count;
        
        rc = KDataBufferResize(buffer, hsize + compress.elem_count);
        if (rc == 0) {
            if (version == 0)
                memmove(((uint8_t *)buffer->base) + hsize, compress.base, compress.elem_count);
            else {
                z_stream zs;
                int zr;
                
                memset(&zs, 0, sizeof(zs));
                
                assert(compress.elem_count >> 32 == 0);
                
                zs.next_out = ((Bytef *)buffer->base) + hsize;
                zs.avail_out = (uInt)compress.elem_count;
                
                zs.next_in = compress.base;
                zs.avail_in = (uInt)compress.elem_count;
                
                zr = deflateInit2(&zs, Z_BEST_SPEED, Z_DEFLATED, -15, 9, Z_DEFAULT_STRATEGY);
                switch (zr) {
                case Z_OK:
                    for (; ;) {
                        zr = deflate(&zs, Z_FINISH);
                        if (zr == Z_OK) {
                            unsigned const offset = (unsigned)(zs.next_out - (Bytef *)buffer->base);
                            rc = KDataBufferResize(buffer, buffer->elem_count * 2);
                            if (rc)
                                break;
                            zs.next_out = ((Bytef *)buffer->base) + offset;
                            zs.avail_out = (uInt)(buffer->elem_count - offset);
                        }
                        else if (zr == Z_STREAM_END) {
                            KDataBufferResize(buffer, zs.total_out + hsize);
                            break;
                        }
                        else {
                            rc = RC(rcVDB, rcPagemap, rcWriting, rcParam, rcInvalid);
                            break;
                        }
                    }
                    deflateEnd(&zs);
                    break;
                case Z_MEM_ERROR:
                    rc = RC(rcVDB, rcPagemap, rcWriting, rcMemory, rcExhausted);
                    break;
                default:
                    rc = RC(rcVDB, rcPagemap, rcWriting, rcParam, rcInvalid);
                }
            }
        }
    }
    KDataBufferWhack(&compress);
    if (rc == 0)
        *size = buffer->elem_count;
    
    return rc;
}

rc_t PageMapSerialize (const PageMap *self, KDataBuffer *buffer, uint64_t offset, uint64_t *size) {
    rc_t rc;
    KDataBuffer temp;

    assert(buffer->elem_bits == 8);
    assert(buffer->bit_offset == 0);
    
    rc = KDataBufferMakeBytes(&temp, 0);
    if (rc == 0) {
        uint64_t sz;
        
        rc = serialize(self, &temp, &sz);
        if (rc == 0) {
            rc = KDataBufferResize(buffer, offset + sz);
            if (rc == 0)
                memmove(&((char *)buffer->base)[offset], temp.base, sz);
            *size = sz;
        }
        KDataBufferWhack(&temp);
    }
    return rc;
}

static
rc_t PageMapDeserialize_v0(PageMap **lhs, uint8_t const src[], unsigned ssize, row_count_t row_count) {
    uint8_t variant = src[0] & 3;
    uint8_t version =src[0] >> 2;
    unsigned cur = 1;
    uint64_t row_len;
    uint64_t data_recs;
    uint64_t leng_recs;
    uint64_t sz;
    bool     random_access=(version==2?true:false);
    rc_t rc = 0;
    
    switch (variant) {
    case 0:
        /*
         variant 0: fixed row length, data_run[..] == 1
         vlencode(row_length)
         */
        rc = vlen_decodeU1(&row_len, &src[cur], ssize - cur, &sz); if (rc) return rc;
	cur += sz;
        
        *lhs = new_StaticPageMap(1, random_access?row_count:1);
        if (*lhs == NULL)
            return RC(rcVDB, rcPagemap, rcConstructing, rcMemory, rcExhausted);
        
        (**lhs).data_recs = row_count;
        (**lhs).leng_recs = 1;
        
        (**lhs).length[0] = (elem_count_t)row_len;
        (**lhs).leng_run[0] = row_count;

	if(random_access){
		(**lhs).data_offset= (**lhs).data_run;
		rc = deserialize_lengths((**lhs).data_offset, (unsigned)row_count, &src[cur], ssize - cur, &sz);
	}
	(**lhs).data_run = 0;
        break;
    case 1:
        /*
         variant 1: fixed row length, variable data_run
         vlencode(row_length)
         vlencode(data_recs)
         serialize(data_run)
         */
        rc = vlen_decodeU1(&row_len, &src[cur], ssize - cur, &sz); if (rc) return rc;
        cur += sz;
        
        rc = vlen_decodeU1(&data_recs, &src[cur], ssize - cur, &sz); if (rc) return rc;
        cur += sz;
        
        *lhs = new_StaticPageMap(1, data_recs);
        if (*lhs == NULL)
            return RC(rcVDB, rcPagemap, rcConstructing, rcMemory, rcExhausted);
        
        (**lhs).data_recs = data_recs;
        (**lhs).leng_recs = 1;
        
        (**lhs).length[0] = (elem_count_t)row_len;
        (**lhs).leng_run[0] = row_count;
        
        rc = deserialize_lengths((**lhs).data_run, (unsigned)data_recs, &src[cur], ssize - cur, &sz);
        break;
    case 2:
        /*
         variant 2: variable row length, data_run[..] == 1
         vlencode(leng_recs)
         serialize(length)
         serialize(leng_run)
         */
        rc = vlen_decodeU1(&leng_recs, &src[cur], ssize - cur, &sz); if (rc) return rc;
        cur += sz;
        
        *lhs = new_StaticPageMap((unsigned)leng_recs, random_access?row_count:1);
        if (*lhs == NULL)
            return RC(rcVDB, rcPagemap, rcConstructing, rcMemory, rcExhausted);
        
        (**lhs).data_recs = row_count;
        (**lhs).leng_recs = leng_recs;
        
#if 0  
        rc = deserialize_lengths((**lhs).length, (unsigned)leng_recs, &src[cur], ssize - cur, &sz);
        if (rc == 0) {
            cur += sz;
            rc = deserialize_lengths((**lhs).leng_run, (unsigned)leng_recs, &src[cur], ssize - cur, &sz);
        }
#else /** can use the fact that both length and leng_run a sequentually allocated ***/
	rc = deserialize_lengths((**lhs).length, (unsigned)(2*leng_recs), &src[cur], ssize - cur, &sz);
	if(rc == 0 && random_access){
		cur+=sz;
		(**lhs).data_offset= (**lhs).data_run;
		rc = deserialize_lengths((**lhs).data_offset, (unsigned)row_count, &src[cur], ssize - cur, &sz);
	}
	(**lhs).data_run = 0;
		
#endif
        break;
    case 3:
        /*
         variant 3: variable row length, variable data_run
         vlencode(leng_recs)
         vlencode(data_recs)
         serialize(length)
         serialize(leng_run)
         serialize(data_run)
         */
        rc = vlen_decodeU1(&leng_recs, &src[cur], ssize - cur, &sz);  if (rc) return rc;
        cur += sz;
        
        rc = vlen_decodeU1(&data_recs, &src[cur], ssize - cur, &sz);  if (rc) return rc;
        cur += sz;
        
        *lhs = new_StaticPageMap(leng_recs, data_recs);
        if (*lhs == NULL)
            return RC(rcVDB, rcPagemap, rcConstructing, rcMemory, rcExhausted);
        
        (**lhs).data_recs = data_recs;
        (**lhs).leng_recs = leng_recs;
#if 0   
        rc = deserialize_lengths((**lhs).length, leng_recs, &src[cur], ssize - cur, &sz);
        if (rc == 0) {
            cur += sz;
            rc = deserialize_lengths((**lhs).leng_run, leng_recs, &src[cur], ssize - cur, &sz);
            if (rc == 0) {
                cur += sz;
                rc = deserialize_lengths((*lhs)->data_run, data_recs, &src[cur], ssize - cur, &sz);
            }
        }
#else /** can use the fact that both length and leng_run a sequentually allocated ***/
	rc = deserialize_lengths((**lhs).length, (unsigned)(2*leng_recs+data_recs), &src[cur], ssize - cur, &sz);
#endif
        break;
     default:
	rc = RC(rcVDB, rcPagemap, rcConstructing, rcFormat, rcUnrecognized);
    }
    (**lhs).random_access = random_access;
    return rc;
}

static
rc_t PageMapDeserialize_v1(PageMap **lhs, const uint8_t *Src, uint64_t ssize, uint32_t row_count) {
    const uint8_t *src = Src;
    const uint8_t * const endp = src + ssize;
    uint8_t variant = *src & 3;
    uint64_t bsize;
    uint64_t hsize;
    uint64_t val;
    uint64_t sz;
    rc_t rc = 0;
    KDataBuffer decompress;
    z_stream zs;
    int zr;
    bool random_access = ((src[0]>>2)==2);
    
    switch (variant) {
    case 0:
	if(!random_access)
		return PageMapDeserialize_v0(lhs, src, ssize, row_count);
	++src;
	rc = vlen_decodeU1(&val, src, endp - src, &sz);
	if( rc == 0){
		src += sz;
		hsize = src - Src;
		bsize = 5 * row_count;
	}
	break;
    case 1:
        ++src;
        rc = vlen_decodeU1(&val, src, endp - src, &sz);
        if (rc == 0) {
            src += sz;
            rc = vlen_decodeU1(&val, src, endp - src, &sz);
            if (rc == 0) {
                src += sz;
                hsize = src - Src;
                bsize = 5 * val;
            }
        }
        break;
    case 2:
        ++src;
        rc = vlen_decodeU1(&val, src, endp - src, &sz);
        if (rc == 0) {
            src += sz;
            hsize = src - Src;
            bsize = 10 * val;
	    if(random_access){
		bsize +=  5 * row_count;
	    }
        }
        break;
    case 3:
        ++src;
        rc = vlen_decodeU1(&val, src, endp - src, &sz);
        if (rc == 0) {
            src += sz;
            bsize = 10 * val;
            rc = vlen_decodeU1(&val, src, endp - src, &sz);
            if (rc == 0) {
                src += sz;
                hsize = src - Src;
                bsize += 5 * val;
            }
        }
        break;
    default:
        return RC(rcVDB, rcPagemap, rcConstructing, rcData, rcInvalid);
    }
    if (rc)
        return rc;
    
    rc = KDataBufferMakeBytes(&decompress, hsize + bsize);
    if (rc)
        return rc;

    memmove(decompress.base, Src, hsize);
    memset(&zs, 0, sizeof(zs));
    
    zs.next_in = (Bytef *)src;
    assert((endp - src) == (uInt)(endp - src));
    zs.avail_in = (uInt)(endp - src);
    
    zs.next_out = ((Bytef *)decompress.base) + hsize;
    assert(bsize == (uInt)(bsize));
    zs.avail_out = (uInt)bsize;
    
    zr = inflateInit2(&zs, -15);
    if (zr == Z_OK) {
        zr = inflate(&zs, Z_FINISH);
        if (zr != Z_STREAM_END)
            rc = RC(rcVDB, rcPagemap, rcConstructing, rcData, rcInvalid);
        inflateEnd(&zs);
    }
    else
        rc = RC(rcVDB, rcPagemap, rcConstructing, rcMemory, rcExhausted);
    
    if (rc == 0)
        rc = PageMapDeserialize_v0(lhs, decompress.base, hsize + zs.total_out, row_count);
    KDataBufferWhack(&decompress);
    return rc;
}

rc_t PageMapDeserialize (PageMap **lhs, const void *src, uint64_t ssize, uint64_t row_count) {
    rc_t rc;

    if ((uint32_t)row_count != row_count)
        return RC(rcVDB, rcPagemap, rcConstructing, rcParam, rcTooBig);
        
    if (lhs == NULL)
        return RC(rcVDB, rcPagemap, rcConstructing, rcParam, rcNull);
    
    *lhs = NULL;
    if (src == NULL || ssize == 0)
        return 0;

    switch (*(const uint8_t *)src >> 2) {
    case 0:
        rc = PageMapDeserialize_v0(lhs, src, ssize, (uint32_t)row_count);
        break;
    case 1:
    case 2:
        rc = PageMapDeserialize_v1(lhs, src, ssize, (uint32_t)row_count);
        break;
    default:
        return RC(rcVDB, rcPagemap, rcConstructing, rcData, rcBadVersion);
    }
    if (rc == 0)
        (**lhs).row_count = (uint32_t)row_count;
    else
        PageMapRelease(*lhs);
    return rc;
}

static
rc_t PageMapDestroy(PageMap *that)
{
#if PAGEMAP_STATISTICS
    pm_stats.currentWaste -= (that->reserve_data - that->data_recs) * sizeof(that->data_run[0]) +
                             (that->reserve_leng - that->leng_recs) * (sizeof(that->leng_run[0]) + sizeof(that->length[0]));
    pm_stats.currentFootprint -= sizeof(*that) + that->reserve_data * sizeof(that->data_run[0]) +
                                 that->reserve_leng * (sizeof(that->leng_run[0]) + sizeof(that->length[0]));
    --pm_stats.alive;
    if (pm_stats.alive == 0) {
        fprintf(stderr, 
                "PM Stats:\n"
                "Footprint (cur/max): %u/%u\n"
                "Waste (cur/max): %u/%u\n"
                "Alive (cur/max): %u/%u\n"
                "Created (static/fixed/single/total): %u/%u/%u/%u\n"
                "Grows: %u\n"
                "Expands (act/calls): %u/%u\n"
                "Appends: %u\n\n",
                (unsigned)pm_stats.currentFootprint,
                (unsigned)pm_stats.maxFootprint,
                (unsigned)pm_stats.currentWaste,
                (unsigned)pm_stats.maxWaste,
                pm_stats.alive,
                pm_stats.maxAlive,
                pm_stats.createStatic,
                pm_stats.createFixedRow,
                pm_stats.createSingle,
                pm_stats.create + pm_stats.createStatic + pm_stats.createFixedRow + pm_stats.createSingle,
                pm_stats.grows,
                pm_stats.expands,
                pm_stats.expandCalls,
                pm_stats.appends
                );
    }
#endif
    KDataBufferWhack(&that->istorage);
    KDataBufferWhack(&that->dstorage);
    KDataBufferWhack(&that->cstorage);
    free(that);
    return 0;
}

rc_t PageMapRelease (const PageMap *self) {
    if (self) {
        if (KRefcountDrop(&self->refcount, "PageMap") == krefWhack)
          return PageMapDestroy((PageMap *)self);
    }
    return 0;
}

rc_t PageMapAddRef (const PageMap *self) {
    if (self)
        KRefcountAdd(&self->refcount, "PageMap");
    return 0;
}

uint32_t PageMapGetIdxRowInfo (const PageMap *cself, uint32_t idx, uint32_t *starting_element,uint32_t *repeat_count)
{
	rc_t rc=0;
	elem_count_t len;
    row_count_t data_run = cself->data_run ? cself->data_run[0] : 1;
    
	if (idx  < data_run || cself->data_recs == 1) {
		if (starting_element)
            *starting_element = 0;
		if (cself->leng_recs == 0)
            len = 0;
		else
            len = cself->length[0];
		if (repeat_count)
            *repeat_count = data_run;
		return len;
	} else {
		rc = PageMapFindRow(cself, idx, starting_element, &len, repeat_count);
		if (rc)
            len = 0;
	}
	return len;
}

int PageMapCompare(const PageMap *a, const PageMap *b) {
    unsigned ai;
    unsigned bi;
    unsigned ar; /* runs in a */
    unsigned br; /* runs in b */
    
    if (
        a == b || (
        a->leng_recs == b->leng_recs &&
        a->data_recs == b->data_recs &&
        memcmp(a->length, b->length, sizeof(a->length[0]) * a->leng_recs) == 0 &&
        memcmp(a->leng_run, b->leng_run, sizeof(a->leng_run[0]) * a->leng_recs) == 0 &&
        memcmp(a->data_run, b->data_run, sizeof(a->data_run[0]) * a->data_recs) == 0
    ))
        return 2;
    
    ai = bi = 0;
    ar = a->leng_run[0];
    br = b->leng_run[0];
    
    while (ai < a->leng_recs && bi < b->leng_recs) {
        if (a->length[ai] != b->length[bi])
            return 0;
        if (ar < br) {
            br -= ar;
            ar = a->leng_run[++ai];
        }
        else if (ar == br) {
            ar = a->leng_run[++ai];
            br = b->leng_run[++bi];
        }
        else {
            ar -= br;
            br = b->leng_run[++bi];
        }
    }
    if (ai != a->leng_recs || bi != b->leng_recs)
        return -1;
    else
        return 1;
}

int PageMapCompareStatic (const PageMap *a, const PageMap *b) {
    if (
	  a->data_recs != 1 ||
	  a->leng_recs != 1 ||
	  a->leng_run[0] != a->data_run[0]
    )
        return -1; /* *a is not static */
    
    if (a == b)
        return 2; /* static and identical */

    if (
	  b->data_recs != 1 ||
	  b->leng_recs != 1 ||
	  b->leng_run[0] != b->data_run[0]
    )
        return -1; /* *b is not static */
    
    if (a->length[0] == b->length[0] && a->leng_run[0] <= b->leng_run[0])
        /* static and compatible, but we want to prevent deeper inspection
         * so we're lying and saying that they are identical */
        return 2;

    return 0;  /* not same */
}

rc_t PageMapAppend(PageMap *self, const PageMap *other) {
    KDataBuffer cstorage;
    rc_t rc;
    
    rc = KDataBufferMake(&cstorage, sizeof(self->length[0]) * 8, (self->leng_recs + other->leng_recs) * 2 + self->data_recs + other->data_recs);
    if (rc == 0) {
        uint32_t *const length = cstorage.base;
        uint32_t *const leng_run = length + self->leng_recs + other->leng_recs;
        uint32_t *const data_run = leng_run + self->leng_recs + other->leng_recs;
        
#if PAGEMAP_STATISTICS
        ++pm_stats.appends;
        pm_stats.currentWaste -= (self->reserve_data - self->data_recs) * sizeof(self->data_run[0]) +
                                 (self->reserve_leng - self->leng_recs) * (sizeof(self->leng_run[0]) + sizeof(self->length[0]));
        pm_stats.currentFootprint -= self->reserve_data * sizeof(self->data_run[0]) +
                                     self->reserve_leng * (sizeof(self->leng_run[0]) + sizeof(self->length[0]));
#endif
        memmove(length                  , self->length , self->leng_recs  * sizeof(length[0]));
        memmove(length + self->leng_recs, other->length, other->leng_recs * sizeof(length[0]));

        memmove(leng_run                  , self->leng_run , self->leng_recs  * sizeof(leng_run[0]));
        memmove(leng_run + self->leng_recs, other->leng_run, other->leng_recs * sizeof(leng_run[0]));
        
        memmove(data_run                  , self->data_run , self->data_recs  * sizeof(data_run[0]));
        memmove(data_run + self->data_recs, other->data_run, other->data_recs * sizeof(data_run[0]));
        
        KDataBufferWhack(&self->cstorage);
        self->cstorage = cstorage;

        self->length = length;
        self->leng_run = leng_run;
        self->data_run = data_run;
        
        self->leng_recs += other->leng_recs;
        self->data_recs += other->data_recs;
        self->row_count += other->row_count;
        self->reserve_leng = self->leng_recs;
        self->reserve_data = self->data_recs;
        
#if PAGEMAP_STATISTICS
        pm_stats.currentWaste += (self->reserve_data - self->data_recs) * sizeof(self->data_run[0]) +
                                 (self->reserve_leng - self->leng_recs) * (sizeof(self->leng_run[0]) + sizeof(self->length[0]));
        pm_stats.currentFootprint += self->reserve_data * sizeof(self->data_run[0]) +
                                     self->reserve_leng * (sizeof(self->leng_run[0]) + sizeof(self->length[0]));
        if (pm_stats.maxFootprint < pm_stats.currentFootprint)
            pm_stats.maxFootprint = pm_stats.currentFootprint;
#endif
        return 0;
    }
    return rc;
}
