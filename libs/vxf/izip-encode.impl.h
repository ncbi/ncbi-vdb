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

static int ANALYZE(stats_t stats[], const unsigned scnt, unsigned *fcnt, const STYPE Y[], const unsigned dcnt, STYPE *MIN, STYPE *MAX) {
    stats_t **base;
    unsigned i;
    unsigned k;
    unsigned n = (dcnt + scnt - 1) / scnt;
    int loops;
    unsigned split;
    unsigned merged = 0;
    STYPE min = Y[0];
    STYPE max = Y[0];
    
    memset(stats, 0, sizeof(stats[0]));
    
    for (i = 0, k = 0; i != dcnt; ++i) {
        if (min > Y[i])
            min = Y[i];
        if (max < Y[i])
            max = Y[i];
        
        stats[k].sy  += (double_t)Y[i];
        stats[k].syy += (double_t)Y[i] * Y[i];
        stats[k].sxy += (double_t)Y[i] * i;
        
        stats[k].sx  += (double_t)i;
        stats[k].sxx += (double_t)i * i;
        
        ++stats[k].n;
        if (stats[k].n != n)
            continue;
        
        stats[k].fit = fitness( stats + k );
        ++k;
        if (k == scnt)
            break;
        memset(stats + k, 0, sizeof(stats[0]));
        stats[k].x = i + 1;
    }
    n = k;
    if (k != scnt && stats[k].n) {
        stats[k].fit = fitness( stats + k );
        ++n;
    }
    
    *MIN = min;
    *MAX = max;
    
    if (n == 1) {
        *fcnt = 1;
        return 0;
    }
    for (k = 0, i = 1; i != n; ++i) {
        stats_t temp;
        
        merge(&temp, &stats[k], &stats[i]);
        if (temp.fit >= stats[k].fit) {
            stats[k] = temp;
        }
        else {
            stats[++k] = stats[i];
        }
    }
    n = k + 1;
    
    if (n == 1) {
        *fcnt = 1;
        return 0;
    }
    
    base = malloc(scnt * sizeof(base[0]));
    if (base == NULL)
        return 1;
    
    for (loops = dcnt; loops > 0; loops >>= 1) {
        for (i = 0, k = 0; i != n; ++i) {
            assert(stats[i].x == k);
            k += stats[i].n;
            base[i] = &stats[i];
            stats[i].i = i;
        }
        assert( k == dcnt );
        ksort( base, n, sizeof(base[0]), comp_fitness, NULL );
        if (n < scnt) {
            for (i = 0, split = 0; i != n && n + split != scnt; ++i) {
                stats_t R;
                stats_t L = *base[i];
                unsigned u = L.n / 2;
                unsigned j;
                
                if (u == 0)
                    continue;
                
                memset(&R, 0, sizeof(R));
                                
                R.x = L.x + L.n - u;
                R.n = u;
                u = L.x + L.n;
                for (j = R.x; j != u; ++j) {
                    R.sx  += j;
                    R.sxx += (double_t)j * j;
                    R.sy  += Y[j];
                    R.syy += (double_t)Y[j] * Y[j];
                    R.sxy += (double_t)Y[j] * j;
                }
                L.sx  -= R.sx;
                L.sxx -= R.sxx;
                L.sy  -= R.sy;
                L.syy -= R.syy;
                L.sxy -= R.sxy;
                assert(L.n > R.n);
                L.n   -= R.n;
                
                L.fit = fitness( &L );
                R.fit = fitness( &R );
                
                if (L.n == 1 || R.n == 1 || L.fit > base[i]->fit || R.fit > base[i]->fit) {
                    assert(L.x + L.n == R.x);
                    assert(L.n + R.n == stats[L.i].n);
                    L.i = R.i = dcnt; /* invalid value */
                    *base[i] = L;
                    stats[n + split] = R;
                    ++split;
                }
            }
            if (split) {
                n += split;
                ksort( stats, n, sizeof(stats[0]), comp_position, NULL );
                continue;
            }
        }
        
        merged = 0;
        for (i = n; i; ) {
            stats_t temp;
            unsigned j;
            
            --i;
            j = base[i]->i;
            
            if (j == n - 1 || stats[j].n == 0 || stats[j + 1].n == 0)
                continue;
            
            merge(&temp, stats + j, stats + j + 1);
            if (temp.n > 2 && temp.fit >= stats[j].fit) {
                stats[j] = temp;
                stats[j + 1].n = 0;
                stats[j + 1].x = dcnt;
                ++merged;
            }
        }
        if (merged == 0)
            break;
        
        for (k = 0, i = 1; i < n; ++i) {
            if (stats[i].n)
                stats[++k] = stats[i];
        }
        assert(k + 1 == n - merged);
        n -= merged;
    }
    
    *fcnt = n;
    free(base);
    return 0;
}

#ifndef ABS
#define ABS(X) ((uint64_t)(X >= 0 ? (X) : (-(X))))
#endif

static
rc_t ENCODE(uint8_t *dst, unsigned dsize, unsigned *psize, const STYPE Y[], unsigned N, int DUMP) {
    stats_t *stats = 0;
    unsigned m = (N + CHUNK_SIZE - 1) / CHUNK_SIZE;
    unsigned n;
    rc_t rc = 0;
    unsigned i;
    unsigned j;
    unsigned k;
    unsigned u;
    unsigned v;
    STYPE min = 0;
    STYPE max = 0;
    unsigned pack_size = 0;
    szbuf zipped;
    struct decoded decoded;
    struct encoded encoded;
    /*int debugging = 0;*/

    if (psize)
        *psize = 0;
    memset(&decoded, 0, sizeof(decoded));

    do {
        stats = malloc(m * sizeof(stats[0]));
        if (stats == NULL) {
            rc = RC(rcVDB, rcFunction, rcExecuting, rcMemory, rcExhausted);
            break;
        }
        rc = ANALYZE(stats, m, &n, Y, N, &min, &max);
        if (rc) {
            rc = RC(rcVDB, rcFunction, rcExecuting, rcMemory, rcExhausted);
            break;
        }
        
        if ((ABS(min) >> 8) == 0 && (ABS(max) >> 8) == 0) {
            decoded.size_type = 0;
            pack_size = N;
        }
        else if ((ABS(min) >> 16) == 0 && (ABS(max) >> 16) == 0) {
            decoded.size_type = 1;
            pack_size = N * 2;
        }
        else if ((ABS(min) >> 32) == 0 && (ABS(max) >> 32) == 0) {
            decoded.size_type = 2;
            pack_size = N * 4;
        }
        else {
            decoded.size_type = 3;
            pack_size = N * 8;
        }
        
        decoded.diff = alloc_raw_nbuf(N);
        if (decoded.diff == NULL) {
            rc = RC(rcVDB, rcFunction, rcExecuting, rcMemory, rcExhausted);
            break;
        }
        decoded.outlier = alloc_raw_nbuf(N);
        if (decoded.outlier == NULL) {
            rc = RC(rcVDB, rcFunction, rcExecuting, rcMemory, rcExhausted);
            break;
        }
        decoded.type = malloc(n);
        if (decoded.type == NULL) {
            rc = RC(rcVDB, rcFunction, rcExecuting, rcMemory, rcExhausted);
            break;
        }
        decoded.length = alloc_raw_nbuf(n);
        if (decoded.length == NULL) {
            rc = RC(rcVDB, rcFunction, rcExecuting, rcMemory, rcExhausted);
            break;
        }

        for (v = u = k = i = 0; i != n; ++i) {
            decoded.length->data.raw[i] = stats[i].n;
            if (stats[i].n >= OUTLIER_LIMIT) {
                double_t M;
                int64_t dy;
                int64_t dx;
                int64_t a;
                
                ++v;
                regress_params(stats + i, &dy, &dx, &a);
                M = dy / (double_t)dx;
                for (j = 0; j != stats[i].n; ++j, ++k) {
                    decoded.diff->data.raw[k] = Y[k] - (STYPE)(M * j + a);
                    if (decoded.diff->min > decoded.diff->data.raw[k] || k == 0)
                        decoded.diff->min = decoded.diff->data.raw[k];
                    if (decoded.diff->max < decoded.diff->data.raw[k] || k == 0)
                        decoded.diff->max = decoded.diff->data.raw[k];
                }
                decoded.type[i] = 0;
            }
            else {
                for (j = 0; j != stats[i].n; ++j, ++k, ++u) {
                    decoded.diff->data.raw[k] = 0;
                    if (decoded.diff->min > 0 || k == 0)
                        decoded.diff->min = 0;
                    if (decoded.diff->max < 0 || k == 0)
                        decoded.diff->max = 0;
                    decoded.outlier->data.raw[u] = Y[k];
                    if (decoded.outlier->min > decoded.outlier->data.raw[u] || u == 0)
                        decoded.outlier->min = decoded.outlier->data.raw[u];
                    if (decoded.outlier->max < decoded.outlier->data.raw[u] || u == 0)
                        decoded.outlier->max = decoded.outlier->data.raw[u];
                }
                ++decoded.outliers;
                decoded.type[i] = 1;
            }
        }
        decoded.length->used = n;
        decoded.outlier->used = u;
        decoded.diff->used = k;

        decoded.dy = alloc_raw_nbuf(v);
        if (decoded.dy == NULL) {
            rc = RC(rcVDB, rcFunction, rcExecuting, rcMemory, rcExhausted);
            break;
        }
        decoded.dx = alloc_raw_nbuf(v);
        if (decoded.dx == NULL) {
            rc = RC(rcVDB, rcFunction, rcExecuting, rcMemory, rcExhausted);
            break;
        }
        decoded.a = alloc_raw_nbuf(v);
        if (decoded.a == NULL) {
            rc = RC(rcVDB, rcFunction, rcExecuting, rcMemory, rcExhausted);
            break;
        }
        for (u = i = 0; i < n; ++i) {
            if (decoded.type[i] == 0) {
                regress_params(stats + i, decoded.dy->data.raw + u, decoded.dx->data.raw + u, decoded.a->data.raw + u);
                if (u == 0) {
                    decoded.dy->min = decoded.dy->data.raw[u];
                    decoded.dy->max = decoded.dy->data.raw[u];
                    decoded.dx->min = decoded.dx->data.raw[u];
                    decoded.dx->max = decoded.dx->data.raw[u];
                    decoded.a->min = decoded.a->data.raw[u];
                    decoded.a->max = decoded.a->data.raw[u];
                }
                else {
                    if (decoded.dy->min > decoded.dy->data.raw[u])
                        decoded.dy->min = decoded.dy->data.raw[u];
                    if (decoded.dy->max < decoded.dy->data.raw[u])
                        decoded.dy->max = decoded.dy->data.raw[u];
                    if (decoded.dx->min > decoded.dx->data.raw[u])
                        decoded.dx->min = decoded.dx->data.raw[u];
                    if (decoded.dx->max < decoded.dx->data.raw[u])
                        decoded.dx->max = decoded.dx->data.raw[u];
                    if (decoded.a->min > decoded.a->data.raw[u])
                        decoded.a->min = decoded.a->data.raw[u];
                    if (decoded.a->max < decoded.a->data.raw[u])
                        decoded.a->max = decoded.a->data.raw[u];
                }
                ++u;            
            }
        }
        decoded.data_count = N;
        decoded.lines = decoded.dy->used = decoded.dx->used = decoded.a->used = u;
        
        free(stats); stats = NULL;
        
        for (u = k = 0, i = 1; i < n; ++i) {
            if (decoded.type[i] == 1 && decoded.type[k] == 1) {
                --decoded.length->used;
                --decoded.outliers;
                decoded.length->data.raw[k] += decoded.length->data.raw[i];
            }
            else {
                ++k;
                if (k != i) {
                    decoded.type[k] = decoded.type[i];
                    decoded.length->data.raw[k] = decoded.length->data.raw[i];
                }
            }
        }
        for (i = 0; i != decoded.length->used; ++i) {
            if (decoded.length->min > decoded.length->data.raw[i] || i == 0)
                decoded.length->min = decoded.length->data.raw[i];
            if (decoded.length->max < decoded.length->data.raw[i] || i == 0)
                decoded.length->max = decoded.length->data.raw[i];
        }
        if (DUMP) {
            if (N == 121) {
                DEBUG_PRINT("Y:", 0);
                for (i = 0; i != N; ++i)
                    DEBUG_PRINT("    %u: %i, diff: %lli", i, (int)Y[i], decoded.diff->data.raw[i]);

                DEBUG_PRINT("%u elements, %u stored, %u lines", N, decoded.outliers, decoded.lines);
                for (i = 0; i != decoded.lines; ++i) {
                    DEBUG_PRINT("line %u: dy: %lli; dx: %lli; a: %lli", i, decoded.dy->data.raw[i], decoded.dx->data.raw[i], decoded.a->data.raw[i]);
                }
            }
            if (decoded.diff)   free(decoded.diff);
            if (decoded.type)   free(decoded.type);
            if (decoded.length) free(decoded.length);
            if (decoded.dy)     free(decoded.dy);
            if (decoded.dx)     free(decoded.dx);
            if (decoded.a)      free(decoded.a);
            if (decoded.outlier) free(decoded.outlier);
                            
            if (stats)  free(stats);
            return 0;
        }
        pack_nbuf(decoded.diff);
        pack_nbuf(decoded.length);
        pack_nbuf(decoded.dy);
        pack_nbuf(decoded.dx);
        pack_nbuf(decoded.a);
        pack_nbuf(decoded.outlier);
        
        rc = encode_decoded(&encoded, &decoded);
        if (rc == 0) {
            unsigned act_size;
            
            rc = serialize_encoded(dst, dsize, &act_size, &encoded);
            /* DEBUG_PRINT("izip compress to %u bytes", act_size); */
            if (psize)
                *psize = act_size;
            free_encoded(&encoded);
        }
    } while (0);
    
    if (decoded.diff)   free(decoded.diff);
    if (decoded.type)   free(decoded.type);
    if (decoded.length) free(decoded.length);
    if (decoded.dy)     free(decoded.dy);
    if (decoded.dx)     free(decoded.dx);
    if (decoded.a)      free(decoded.a);
    if (decoded.outlier) free(decoded.outlier);
                            
    if (stats)  free(stats);
    
    if (*psize && pack_size && (pack_size + 13) < *psize) {
        decoded.diff = alloc_raw_nbuf(N);
        if (decoded.diff) {
            decoded.diff->max = max;
            decoded.diff->min = min;
            decoded.diff->used = N;
            
            switch (decoded.size_type) {
            case 0:
                decoded.diff->var = 4;
                for (i = 0; i != N; ++i)
                    decoded.diff->data.u8[i] = (uint8_t)( Y[i] - min );
                break;
            case 1:
                decoded.diff->var = 3;
                for (i = 0; i != N; ++i)
                    decoded.diff->data.u16[i] = (uint16_t)( Y[i] - min );
                break;
            case 2:
                decoded.diff->var = 2;
                for (i = 0; i != N; ++i)
                    decoded.diff->data.u32[i] = (uint32_t)( Y[i] - min );
                break;
            default:
                decoded.diff->var = 1;
                for (i = 0; i != N; ++i)
                    decoded.diff->data.u64[i] = Y[i] - min;
                break;
            }
            zipped.buf = malloc(zipped.size = nbuf_size(decoded.diff));

            if (zipped.buf) {
                rc_t rc2 = zlib_compress(&zipped, decoded.diff->data.u8, zipped.size, Z_DEFAULT_STRATEGY, Z_BEST_SPEED);
                if (rc2 == 0 && zipped.used > 0) {
                    encoded.flags = (decoded.size_type << 2) | 3;
                    encoded.data_count = N;
                    encoded.u.zipped.data_size = zipped.used;
                    encoded.u.zipped.data = zipped.buf;
                    encoded.u.packed.min = min;
                    rc = serialize_encoded(dst, dsize, psize, &encoded);
/*                    DEBUG_PRINT("packed+zipped to %u bytes", *psize); */
                    /* debugging = 3; */
                }
                else {
                    encoded.flags = (decoded.size_type << 2) | 2;
                    encoded.data_count = N;
                    encoded.u.packed.data_size = nbuf_size(decoded.diff);
                    encoded.u.packed.data = decoded.diff->data.u8;
                    encoded.u.packed.min = min;
                    rc = serialize_encoded(dst, dsize, psize, &encoded);
/*                    DEBUG_PRINT("packed to %u bytes", *psize); */
                    /* debugging = 2; */
                }
                free(zipped.buf);
            }
            else {
                rc = RC(rcVDB, rcFunction, rcExecuting, rcMemory, rcExhausted);
            }
            free(decoded.diff);
        }
        else {
            rc = RC(rcVDB, rcFunction, rcExecuting, rcMemory, rcExhausted);
        }
    }
    else {
        /* DEBUG_PRINT("packed would have been %u", pack_size + 13); */
    }

    if (*psize) {
        size_t bsize = N * sizeof(Y[0]);
        assert(bsize <= UINT32_MAX);
        zipped.buf = malloc(zipped.size = (unsigned)bsize);
        if (zipped.buf) {
            rc_t rc2 = zlib_compress(&zipped, Y, zipped.size, Z_DEFAULT_STRATEGY, Z_BEST_SPEED);
            if (rc2 == 0 && zipped.used + 5 <= *psize) {
                encoded.flags = ((sizeof(Y[0]) == 1 ? 0 : sizeof(Y[0]) == 2 ? 1 : sizeof(Y[0]) == 4 ? 2 : 3) << 2) | 1;
                encoded.data_count = N;
                encoded.u.zipped.data_size = zipped.used;
                encoded.u.zipped.data = zipped.buf;
                rc = serialize_encoded(dst, dsize, psize, &encoded);
                /* DEBUG_PRINT("zipped to %u bytes", *psize); */
                /* debugging = 1; */
            }
            else {
                /* DEBUG_PRINT("zipped would have been %u", zipped.used + 5); */
            }

            free(zipped.buf);
        }
    }
    /* if (debugging == 0) { */
    /*     ENCODE(NULL, 0, NULL, Y, N, 1); */
    /* } */
    return rc;
}
