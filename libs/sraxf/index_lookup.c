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

#include <vdb/extern.h>


#include <vdb/xform.h>
#include <vdb/table.h>
#include <vdb/vdb-priv.h>
#include <kdb/index.h>
#include <klib/data-buffer.h>
#include <klib/rc.h>
#include <klib/text.h>
#include <klib/pbstree.h>
#include <klib/log.h>
#include <sysalloc.h>
#include <atomic32.h>

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <assert.h>

typedef struct tag_self_t {
    const       KIndex *ndx;
    char        query_key[1024];
    uint32_t    query_key_len;
    uint8_t     name_fmt_version;
    const struct VCursorParams * parms;
    uint32_t    elem_bits;
} self_t;

static void CC self_whack( void *Self )
{
    self_t *self = Self;
    
    KIndexRelease( self->ndx );
    free( self );
}

typedef struct FindFmtDataXtra{
        int32_t x;
        int32_t y;
}FindFmtDataXtra;

static int CC SRAPTNodeFindFmt ( const void *item, const struct PBSTNode *n, void *data )
{
    int i,j;
    FindFmtDataXtra *u=(FindFmtDataXtra *)data;
    /* compare the remainder of the key */
    const String *rem = item;
    const char* q=rem->addr; /** query **/
    const char* db=n->data.addr; /** database **/

    for(i=j=0;q[i]!=0 && db[j]!= 0;){
        if(q[i] == db[j]){
                i++;j++;
        } else if(db[j] == '$'){
                switch(db[j+1]){
		 case 'q':
                 case 'Q':
		 {
			int k;
			uint32_t xy=0;
			j+=2;
			for(k=0;k<5 && q[i]!=0;k++,i++){
				xy *= 36;
				if(isdigit(q[i])){
					xy += q[i] - '0' + 26;
				} else if(isalpha(q[i])){
					xy += toupper(q[i]) - 'A';
				}
			}
			if(u){
				u->x = xy >> 12;
				u->y = xy &0xFFF;
			}
	       		break;
		 }
                 case 'x':
                 case 'X':
                        if(u) u->x=atoi(q+i);
                        j+=2;
                        while(isdigit(q[i]))i++;
			if(q[i]=='%' && isdigit(q[i+1])){
				u->x += 24*1024*(q[i+1]-'0');
				j+=2;
			}
                        break;
                 case 'y':
                 case 'Y':
                        if(u) u->y=atoi(q+i);
                        j+=2;
                        while(isdigit(q[i]))i++;
			if(q[i]=='%' && isdigit(q[i+1])){
				u->y += 24*1024*(q[i+1]-'0');
				j+=2;
			}
                        break;
                 default:
                        return  q[i]-db[j];
                        break;
                }
        } else {
                break;
        }
    }
    return  q[i]-db[j];
}


static
rc_t CC index_lookup_impl(
                          void *Self,
                          const VXformInfo *info,
                          int64_t row_id,
                          VRowResult *rslt,
                          uint32_t argc,
                          const VRowData argv[]
                          )
{
    rc_t rc;
    const self_t *self = Self;
    KDataBuffer *query_buf=NULL;
    char	query[1024];
    uint64_t id_count;
    int64_t start_id;
    FindFmtDataXtra fxdata={0,0};
    struct {
        uint64_t start_id;
        uint64_t id_count;
        uint64_t x;
        uint64_t y;
    } out;
    
    rc = VCursorParamsGet(self->parms,self->query_key,&query_buf);
    if (GetRCState(rc) == rcNotFound && GetRCObject(rc) == rcName) {
	rslt->elem_count = 0;
	rslt->no_cache = 1;
	return 0;
    }
    if(rc == 0){
        char    *qptr=((char*)query_buf->base)+(query_buf->bit_offset >>3);
        uint32_t qlen=query_buf->elem_count;
        
        if(argc > 0){
            char *   pptr=((char*)argv[0].u.data.base) + argv[0].u.data.first_elem;
            uint32_t plen=argv[0].u.data.elem_count;
            while(plen > 0 &&  qlen > 0 && *pptr == *qptr){
                pptr++;
                qptr++;
                plen--;
                qlen--;
            }
            if(plen > 0) rc = RC ( rcVDB,rcIndex,rcSearching,rcData,rcNotFound);
        }
        if( rc == 0){
            memcpy(query,qptr,qlen);
            query[query_buf->elem_count]='\0';
            if( self -> name_fmt_version == 1){ /*** this is a strange slx format name - very first implementation ***/
                int lane=-1,tile=-1,x=-1,y=-1;
                qptr=query;
                if(*qptr == ':'){ qptr++;qlen--;}
                sscanf(qptr,"%d:%d:%d:%d",&lane,&tile,&x,&y);
                if(lane < 0 || lane > 0xF || tile < 0 || tile > 0xFFF || x < 0 || x > 0xFFF || y < 0 || y > 0xFFF){
                    rc = RC ( rcVDB,rcIndex,rcSearching,rcData,rcNotFound);
                } else {
                    sprintf(query,"%1X%03X%03X%03X",lane,tile,x,y);
                }
            }
            if(rc == 0 ) {
                if( self -> name_fmt_version  >= 2) /*** X and Y are present ***/
                    rc = KIndexFindText(self->ndx, query, &start_id, &id_count,SRAPTNodeFindFmt,&fxdata);
                else
                    rc = KIndexFindText(self->ndx, query, &start_id, &id_count,NULL,NULL);
                if(rc == 0){
                    KDataBuffer *dst = rslt -> data;
                    rc = KDataBufferResize ( dst, 1 );
                    if( rc == 0) {
                        out.start_id = start_id;
                        out.id_count = id_count;
                        out.x        = fxdata.x;
                        out.y        = fxdata.y;
                        memcpy(dst -> base,&out,sizeof(out));
                        rslt -> elem_count = 1;
                        rslt -> no_cache = 1; /***** This row should never be cached **/
                    }
                }
            }
        }
    }
    return rc;
}

VTRANSFACT_IMPL(NCBI_SRA_lookup, 1, 0, 0) (
                                           const void *Self,
                                           const VXfactInfo *info,
                                           VFuncDesc *rslt,
                                           const VFactoryParams *cp,
                                           const VFunctionParams *dp
) {
    rc_t rc;
    const KIndex *ndx;
    KIdxType type;
    
    rc = VTableOpenIndexRead(info->tbl, &ndx, "%.*s", (int)cp->argv[0].count, cp->argv[0].data.ascii);
    if ( rc != 0 )
    {
        if ( GetRCState ( rc ) != rcNotFound )
            PLOGERR (klogErr, (klogErr, rc, "Failed to open index '$(index)'", "index=%.*s", (int)cp->argv[0].count, cp->argv[0].data.ascii));
        return rc;
    }
    
    rc = KIndexType(ndx, &type);
    if (rc == 0) {
        if (type == kitProj + kitText) {
            self_t *self;
            
            self = malloc(sizeof(*self));
            if (self) {
                self->ndx = ndx;
                self->elem_bits = VTypedescSizeof(&info->fdesc.desc);
		memcpy(self->query_key,cp->argv[1].data.ascii,cp->argv[1].count);
		self->query_key_len = cp->argv[1].count;
		self->query_key[self->query_key_len]='\0';
		self->parms=info->parms;
		self->name_fmt_version = *cp->argv[2].data.u8;
/*** testing::: VCursorParamsSet(self->parms,"QUERY_BY_NAME" ,"EM7LVYS01C1LWG");***/
		
                rslt->self = self;
                rslt->whack = self_whack;
                rslt->variant = vftNonDetRow;
                rslt->u.ndf = index_lookup_impl;
                return 0;
            }
            rc = RC(rcVDB, rcFunction, rcConstructing, rcMemory, rcExhausted);
        }
        else
            rc = RC(rcVDB, rcFunction, rcConstructing, rcIndex, rcIncorrect);
    }
    KIndexRelease(ndx);
    return rc;
}
