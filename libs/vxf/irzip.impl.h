/*
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

#define DELTA_UND  0
#if _ARCH_BITS == 32
#define DELTA_POS  0x7ffffffffffffff0LL
#define DELTA_NEG  0x7ffffffffffffff1LL
#define DELTA_BOTH 0x7ffffffffffffff2LL
#else
#define DELTA_POS  0x7ffffffffffffff0
#define DELTA_NEG  0x7ffffffffffffff1
#define DELTA_BOTH 0x7ffffffffffffff2
#endif

#define TRIGGER_2ND_SERIES 10*1024

static rc_t ENCODE(uint8_t dst[], size_t dsize, size_t *used, int64_t *Min, int64_t *Slope, uint8_t *series_count,uint8_t *planes, const STYPE Y[], unsigned N)
{
    unsigned i;
    unsigned k;
    rc_t rc=0;
    STYPE min,a0;
    int64_t 	slope, delta_type = DELTA_UND,y_cur;
    uint64_t    sum_a0,sum_min,sum_y,sum_delta;
    uint64_t	max_diff;
    uint8_t	*scratch=NULL;
#ifdef TRY2SERIES
    uint8_t	*series=NULL;/*** for 2nd series flag ***/
    uint64_t    sum_delta2[2] = {0,0};
    uint64_t    max_diff2[2] = {0,0};
    uint32_t	last[2]; /** to run 2 series ***/
    int64_t	delta2_type[2] = {DELTA_UND,DELTA_UND};
#endif

    if( N < 10 ) /*** no reason to encode **/
		return RC(rcXF, rcFunction, rcExecuting, rcBuffer, rcInsufficient);

    scratch=malloc(N*sizeof(STYPE));
    if(scratch==NULL) return RC(rcXF, rcFunction, rcExecuting, rcBuffer, rcInsufficient);
#ifdef TRY2SERIES
    series=malloc(N*sizeof(*series));
    if(series==NULL) return RC(rcXF, rcFunction, rcExecuting, rcBuffer, rcInsufficient);
#endif
   
    /*** the code tries to best-fit one of 3 cases ****/
    /*** 1. delta - for monotonous data **/
    /*** 2. horizontal line through the minimum ****/
    /*** 3. sloped line from the first element y[i] = y[0] + i * slope ***/
    /*** all algorithms keep delta positive ****/
    /*** 1. is preferred ***/
    /*** the best between 2 and 3 is chosen by the sum of all deltas ****/
    /*** floating point calculations are avoided ***/
	a0 =  Y[0];
	y_cur = Y[1];
	sum_y =  Y[0] +  Y[1];
	slope = (int64_t)Y[1] - (int64_t)Y[0];
	if(slope < 0) {
		min = Y[1]; 
		sum_delta = -slope;
		delta_type = DELTA_NEG;
	} else {
		min = Y[0];
		sum_delta = slope;
		if( slope != 0 ) delta_type = DELTA_POS;
	}
	max_diff = sum_delta;
#ifdef TRY2SERIES
	if(sum_delta >= TRIGGER_2ND_SERIES){
		last[0]=0; last[1]=1;
		series[0]=0; series[1]=1;
	} else {
		last[0]=1; last[1]=0;
		series[0]=0; series[1]=0;
		delta2_type[0] = delta_type;
		sum_delta2[0]=sum_delta;
		max_diff2[0]= sum_delta;
	}
#endif
	for (i = 2; i < N; ++i) {
		int64_t  md = (int64_t)Y[i] - (int64_t)min;
		int64_t diff =  (int64_t)Y[i] - (int64_t)Y[i-1];
		uint64_t abs_diff = (diff >= 0) ? diff : -diff;
#ifdef TRY2SERIES
		int64_t	last_delta_type = delta_type ;
#endif
		sum_y += Y[i];
		sum_delta += abs_diff;
		if(abs_diff > max_diff) max_diff = abs_diff;
#define MACRO_SET_DELTA_TYPE(Diff,DeltaType) \
		if( DeltaType != DELTA_BOTH){ \
			if ( Diff > 0){ \
				if(DeltaType == DELTA_NEG ) DeltaType =DELTA_BOTH; \
				else if (DeltaType == DELTA_UND) DeltaType = DELTA_POS; \
			} else if ( Diff < 0 ){ \
				if(DeltaType == DELTA_POS ) DeltaType =DELTA_BOTH; \
				else if (DeltaType == DELTA_UND) DeltaType = DELTA_NEG; \
			} \
		}
		MACRO_SET_DELTA_TYPE(diff,delta_type);
#ifdef TRY2SERIES
		if(last[1] == 0){/** don't have a second series yet ***/
			if(abs_diff >= TRIGGER_2ND_SERIES){
				last[1]=i;
				series[i]=1;
				sum_delta2[0] = sum_delta - abs_diff;
				delta2_type[0] = last_delta_type;
				max_diff2[0] = max_diff;
			} else {
				last[0]=i;
				series[i]=0;
			}
		} else { /*** second series is in being built ***/
			int64_t diff1 =  (int64_t)Y[i] - (int64_t)Y[last[0]];
			int64_t diff2 =  (int64_t)Y[i] - (int64_t)Y[last[1]];
			uint64_t abs_diff1 = (diff1 >= 0) ? diff1 : -diff1;
			uint64_t abs_diff2 = (diff2 >= 0) ? diff2 : -diff2;
			if(abs_diff1 <= abs_diff2){
				last[0]=i;
				series[i]=0;
				sum_delta2[0] += abs_diff1;
				if(abs_diff1 > max_diff2[0]) max_diff2[0] = abs_diff1;
				MACRO_SET_DELTA_TYPE(diff1,delta2_type[0]);
			} else {
				last[1]=i;
				series[i]=1;
				sum_delta2[1] += abs_diff2;
				if(abs_diff2 > max_diff2[1]) max_diff2[1] = abs_diff2;
				MACRO_SET_DELTA_TYPE(diff2,delta2_type[1]);
			}
		}
#endif
		if ( md < 0 ) {
			min = Y[i];
		}
		y_cur += slope;
		if(y_cur > (int64_t)Y[i]){/*** reduce the slope ***/
			slope =  ((int64_t)Y[i] - (int64_t)Y[0] - i+1/**round down**/) /i;
			y_cur =  (int64_t)Y[0] + (int64_t)(i* slope);
			assert( y_cur <= (int64_t)Y[i]);
		}
	}

    /*** define which delta is the smallest **/
    *series_count=1;
    if(delta_type == DELTA_BOTH) sum_delta <<= 1; /*** in DELTA_BOTH case we will use extra bit ***/
    
    sum_min = sum_y - N*min;
    sum_a0  = sum_y - N*a0 - slope*N*(N-1)/2;
    
			
    if( (delta_type == DELTA_POS || delta_type == DELTA_NEG ||
        (delta_type == DELTA_BOTH &&  max_diff <  (((uint64_t)1)<<(sizeof(STYPE)*8-1))) /*** to prevent loosing a bit **/
        ) && sum_delta <= sum_min && sum_delta <= sum_a0){
	a0=Y[0];
	slope = delta_type;
    } else if(sum_a0 > sum_min){
	a0 = min;
	slope = 0;
    }
	

    *planes = 0;
    *used = 0;
    *Min= a0;
    *Slope = slope;

#ifdef TRY2SERIES
    if(last[1] > 0){
	bool delta2_is_ok=true;
	uint64_t    sum_delta2_total=0;
	if(delta2_type[0] == DELTA_UND) delta2_type[0] = DELTA_POS;
	if(delta2_type[1] == DELTA_UND) delta2_type[1] = DELTA_POS;

	if(delta2_type[0] == DELTA_BOTH){
		 delta2_is_ok = delta2_is_ok && (max_diff2[0] <  (((uint64_t)1)<<(sizeof(STYPE)*8-2)));
		 sum_delta2_total += sum_delta2[0] * 4;
	} else {
		 delta2_is_ok = delta2_is_ok && (max_diff2[0] <  (((uint64_t)1)<<(sizeof(STYPE)*8-1)));
		 sum_delta2_total += sum_delta2[0] * 2;
	}
	if(delta2_type[1] == DELTA_BOTH){
		delta2_is_ok = delta2_is_ok && (max_diff2[1] <  (((uint64_t)1)<<(sizeof(STYPE)*8-2)));
		sum_delta2_total += sum_delta2[1] * 4;
	} else {
		delta2_is_ok = delta2_is_ok && (max_diff2[1] <  (((uint64_t)1)<<(sizeof(STYPE)*8-1)));
		sum_delta2_total += sum_delta2[1] * 2;
	}
	
	if(delta2_is_ok && sum_delta2_total < sum_delta && sum_delta2_total < sum_min && sum_delta2_total< sum_a0){
		*series_count=2;
		Slope[0] = delta2_type[0];
		Slope[1] = delta2_type[1];
		Min[0]   = Y[0];
	}
    }
#endif

#define MACRO_FLUSH_VALUE \
	for(k=0;k<sizeof(STYPE);k++){ \
		if ((scratch[i+k*N] = (uint8_t)(val & 0xff)) != 0){ \
			*planes |= (1<<k); \
		} \
		val >>= 8; \
	}

    /*** rotate the array ***/
    if( *series_count > 1 ){
#ifdef TRY2SERIES
	for( i = 0,last[0]=0,last[1]=0; i != N; ++i){
		uint64_t val;
		if(series[i] == 0){
			if(delta2_type[0] == DELTA_POS)       { assert(Y[i] >= Y[last[0]]); val = Y[i] - Y[last[0]];}
			else if (delta2_type[0] == DELTA_NEG) { assert(Y[i] <= Y[last[0]]); val = Y[last[0]] - Y[i];}
			else if( Y[i] >= Y[last[0]] )         val = (Y[i] - Y[last[0]])*2;
			else				      val = (Y[last[0]] - Y[i])*2+1;
			last[0]=i;
			val<<=1;
		} else {
			if(last[1]==0){
				Min[1] = Y[i];
				val = 1;/*** flag second series with a bit ***/
			} else {
				if(delta2_type[1] == DELTA_POS)       { assert(Y[i] >= Y[last[1]]); val = Y[i] - Y[last[1]];}
				else if (delta2_type[1] == DELTA_NEG) { assert(Y[i] <= Y[last[1]]); val = Y[last[1]] - Y[i];}
				else if( Y[i] >= Y[last[1]] )         val = (Y[i] - Y[last[1]])*2;
				else				      val = (Y[last[1]] - Y[i])*2+1;
				val = val*2+1; /*** flag second series with a bit ***/
			}
			last[1]=i;
		}
                MACRO_FLUSH_VALUE;
        }
#else
	assert(0);
	rc=RC(rcXF, rcFunction, rcExecuting, rcParam, rcInconsistent);
#endif
    } else if(slope == DELTA_POS){
	for( i = 0; i != N; ++i){
		uint64_t val=Y[i] - a0;
		assert(Y[i] >= a0);
		a0 =  Y[i];
		MACRO_FLUSH_VALUE;
	}
    } else if( slope == DELTA_NEG){
	for( i = 0; i != N; ++i){
		uint64_t  val = a0 - Y[i];
		assert(Y[i] <= a0);
                a0 =  Y[i];
		MACRO_FLUSH_VALUE;
	}
    } else if( slope == DELTA_BOTH){
	for( i = 0; i != N; ++i){
		uint64_t val;
		if( Y[i] >= a0 ){ /** move sign bit into the lowest bit ***/
                        val = ( Y[i] - a0 ) << 1;
                } else {
                        val = ( a0 - Y[i]) * 2 + 1;
                }
                a0 =  Y[i];
		MACRO_FLUSH_VALUE;
	}
    } else {
	for( i = 0; i != N; ++i){
		uint64_t val =  Y[i] - a0 - slope*i;
		assert((int64_t)Y[i] >=  (int64_t)a0 + slope*i);
		MACRO_FLUSH_VALUE;
	}
    }

    /*** record the arrays ***/
    for(k=0;k<sizeof(STYPE) && rc == 0; k++){
	if (*planes & (1<<k)) {
		szbuf s2;
		s2.used = 0;
		s2.size = dsize - *used;
		s2.buf  = dst + *used;
		rc = zlib_compress(&s2, scratch+k*N, N,Z_RLE, Z_BEST_SPEED);
		if ( rc == 0 ) {
			*used += s2.used;
			if (s2.used == 0) /*** skip zipping **/
				 rc=RC(rcXF, rcFunction, rcExecuting, rcBuffer, rcInsufficient);
		}
	}
    }
    if(scratch) free(scratch);
#ifdef TRY2SERIES
    if(series) free(series);
#endif
    return rc;
}

static rc_t DECODE(STYPE Y[], unsigned N, int64_t* min, int64_t* slope, uint8_t series_count, uint8_t planes, const uint8_t src[], size_t ssize)
{
    unsigned k;
    size_t j;
    unsigned m;
    unsigned i;
    uint8_t *scratch=NULL;
    rc_t rc=0;
    bool    first;

    memset(Y, 0, sizeof(Y[0]) * N);
    for (j = k = 0, m = 1,first=true; m < 0x100; m <<= 1, k += 8) {
        size_t n;
        
        if ((planes & m) == 0)
            continue;
        
        n = 0;
	if( !scratch ) scratch = malloc(N);
	
        rc = zlib_decompress(scratch, N, &n, src + j, ssize - j);
        if (rc) goto DONE;
        j += n;
	if(first){
		for (i = 0; i != N; ++i) Y[i] = ((STYPE)scratch[i]) << k;
		first = false;
	} else {
		for (i = 0; i != N; ++i) Y[i] |= ((STYPE)scratch[i]) << k;
	}
    }
    if(series_count == 2){
#if 0 /** trying to unroll ***/
	STYPE Ylast;
	if(slope[0]==DELTA_BOTH){
		if(slope[1]==DELTA_BOTH){ 
			for(i = 0; i != N; ++i){
				scratch[i] = Y[i]&3;
				Y[i] = ((USTYPE)Y[i]) >> 2;
			}
		} else {
			for(i = 0; i != N; ++i){
				uint8_t tmp = Y[i]&1;
				if(tmp){
					scratch[i] = tmp;
					Y[i] = ((USTYPE)Y[i]) >> 1;
				} else {
					scratch[i] = Y[i]&3;
					Y[i] = ((USTYPE)Y[i]) >> 2;
				}
                        }

		}
	} else {
		if(slope[1]==DELTA_BOTH){
			for(i = 0; i != N; ++i){
                                uint8_t tmp = Y[i]&1;
                                if(!tmp){
                                        scratch[i] = tmp;
					Y[i] = ((USTYPE)Y[i]) >> 1;
                                } else {
                                        scratch[i] = Y[i]&3;
					Y[i] = ((USTYPE)Y[i]) >> 2;

                                }
                        }
		} else {
			for(i = 0; i != N; ++i){
				scratch[i] = Y[i]&1;
				Y[i] = ((USTYPE)Y[i]) >> 1;
			}
		}
	}
	/** k=0 - main series, k=1 -secondary **/
	for(k=0;k<2;k++){
		Ylast = min[k];
		if(slope[k]==DELTA_POS) {
			for (i = 0; i != N; ++i){
				if((scratch[i]&1)==k){
					Y[i]  = Ylast + Y[i];
					Ylast = Y[i];
				}
			}
		} else if (slope[k]==DELTA_NEG){
			for (i = 0; i != N; ++i){
				if((scratch[i]&1)==k){
					Y[i]  = Ylast - Y[i];
					Ylast = Y[i];
				}
			}
		} else for (i = 0; i != N; ++i){
			if((scratch[i]&1)==k){
				if(scratch[i]&2){
					Y[i]  = Ylast - Y[i];
				} else {
					Y[i]  = Ylast + Y[i];
				}
				Ylast = Y[i];
			}
		}
	}

#else
	uint32_t last[2]={0,0};
	for (i = 0; i != N; ++i){
		if( (Y[i]&1) == 0 ){/**main series **/
			USTYPE val = (USTYPE)Y[i];
			val >>= 1;
			if(i==0){
                                Y[i] = (STYPE)min[0];
                                last[0]=i;
                        }
			if(slope[0]==DELTA_POS)      Y[i] = Y[last[0]] + val;
			else if(slope[0]==DELTA_NEG) Y[i] = Y[last[0]] - val;
			else if ((val&1)==0)         Y[i] = Y[last[0]] + (val >>1);
			else 	                     Y[i] = Y[last[0]] - (val >>1);
			last[0]=i;
		} else { /**secondary series ***/
			USTYPE val = (USTYPE)Y[i];
			val >>= 1;
			if(last[1]==0){
				Y[i] = (STYPE)min[1];
				last[1]=i;
			}
                        if(slope[1]==DELTA_POS)      Y[i] = Y[last[1]] + val;
                        else if(slope[1]==DELTA_NEG) Y[i] = Y[last[1]] - val;
                        else if ((val&1)==0)         Y[i] = Y[last[1]] + (val >>1);
                        else                         Y[i] = Y[last[1]] - (val >>1);
                        last[1]=i;
		}
        }
#endif
    } else if(min[0]==0 && slope[0]==0){ /*** no slope no offset - nothing to do ***/
    } else if(slope[0] == DELTA_POS){
	assert(Y[0] == 0);
	Y[0] = (STYPE)min[0];
	for (i = 1; i != N; ++i){
		Y[i] = Y[i-1] + Y[i];
	}
    } else if (slope[0] == DELTA_NEG ) {
	assert(Y[0] == 0);
	Y[0] = (STYPE)min[0];
	for (i = 1; i != N; ++i){
		Y[i] =  Y[i-1] - Y[i];
	}
    } else if (slope[0] == DELTA_BOTH){
	assert(Y[0] == 0);
	Y[0] = (STYPE)min[0];
	for (i = 1; i != N; ++i){
		USTYPE val = (USTYPE)Y[i];
		val >>= 1;
		if(Y[i] & 1) Y[i] = Y[i-1] - val;
		else         Y[i] = Y[i-1] + val;
	}
    } else if(slope[0] == 0) {
	for (i = 0; i != N; ++i){
		Y[i]  += (STYPE)min[0];
	}
    } else {
	for (i = 0; i != N; ++i){
                Y[i]  += (STYPE)min[0];
                min[0] += slope[0];
        }
    }

DONE:
    if(scratch) free(scratch);
    return rc;
}
