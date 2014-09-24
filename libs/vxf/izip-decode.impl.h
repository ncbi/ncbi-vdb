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

static
rc_t DECODE(STYPE Y[], unsigned N, const uint8_t src[], unsigned ssize, int swap) {
    rc_t rc;
    struct encoded encoded;
    struct decoded decoded;
    unsigned i;
    unsigned j;
    unsigned k;
    unsigned u;
    unsigned v;
    
    rc = deserialize_encoded(&encoded, src, ssize, swap);
    if (rc)
        return rc;
    rc = decode_encoded(&decoded, &encoded);
    if (rc == 0) {
        if (decoded.diff->used != N)
            return RC(rcXF, rcFunction, rcExecuting, rcData, rcInvalid);
        
        if ((encoded.flags & 3) == 0) {
            if (N == 121)
                DEBUG_PRINT("decompressing izip", 0);
            if (swap) {
                unpack_nbuf_swap(decoded.diff);
                unpack_nbuf_swap(decoded.length);
                unpack_nbuf_swap(decoded.dy);
                unpack_nbuf_swap(decoded.dx);
                unpack_nbuf_swap(decoded.a);
                if (decoded.outlier)
                    unpack_nbuf_swap(decoded.outlier);
            }
            else {
                unpack_nbuf(decoded.diff);
                unpack_nbuf(decoded.length);
                unpack_nbuf(decoded.dy);
                unpack_nbuf(decoded.dx);
                unpack_nbuf(decoded.a);
                if (decoded.outlier)
                    unpack_nbuf(decoded.outlier);
            }

            if (N == 121) {
                for (i = 0; i != decoded.diff->used; ++i) {
                    DEBUG_PRINT("  %i - diff: %lli", i, decoded.diff->data.raw[i]);
                }
            }
            
            for (u = v = k = i = 0; i != decoded.lines + decoded.outliers; ++i) {
                unsigned n = (unsigned)decoded.length->data.raw[i];
                
                if (decoded.type[i]) {
                    if (N == 121)
                        DEBUG_PRINT("extracting stored segment %u; length: %u", i, n);
                    for (j = 0; j != n; ++j, ++k, ++v)
                        Y[k] = (STYPE)( decoded.outlier->data.raw[v] );
                }
                else {
                    const double m = decoded.dy->data.raw[u] / (double)decoded.dx->data.raw[u];
                    
                    if (N == 121)
                        DEBUG_PRINT("extracting line segment %u; length: %u; dy: %lli; dx: %lli; a: %lli", i, n, decoded.dy->data.raw[u], decoded.dx->data.raw[u], decoded.a->data.raw[u]);
                    for (j = 0; j != n; ++j, ++k) {
                        Y[k] = (STYPE)( decoded.diff->data.raw[k] + (STYPE)(decoded.a->data.raw[u] + j * m) );
                        if (N == 121)
                            DEBUG_PRINT("    %u: %i", k, (int)Y[k]);
                    }
                    ++u;
                }
            }
        }
        else if ((encoded.flags & 2) != 0) {
            if ((((size_t)1) << decoded.size_type) > sizeof(Y[0])) {
#if _DEBUGGING
                fprintf(stderr, "iunzip: refusing to decode from original element size of %u to %u\n", (unsigned)(((size_t)1) << decoded.size_type), (unsigned)sizeof(Y[0]));
#endif
                return RC(rcXF, rcFunction, rcExecuting, rcRange, rcExcessive);
            }
            switch (decoded.diff->var) {
            case 4:
                for (i = 0; i != decoded.diff->used; ++i)
                    Y[i] = (int32_t)( decoded.diff->data.u8[i] + decoded.diff->min );
                break;
            case 3:
                for (i = 0; i != decoded.diff->used; ++i)
                    Y[i] = (int32_t)( decoded.diff->data.u16[i] + decoded.diff->min );
                break;
            case 2:
                for (i = 0; i != decoded.diff->used; ++i)
                    Y[i] = (int32_t)( decoded.diff->data.u32[i] + decoded.diff->min );
                break;
            case 1:
            default:
                for (i = 0; i != decoded.diff->used; ++i)
                    Y[i] = (int32_t)( decoded.diff->data.u64[i] + decoded.diff->min );
                break;
            }
        }
        else {
            if ((((size_t)1) << decoded.size_type) > sizeof(Y[0])) {
#if _DEBUGGING
                fprintf(stderr, "iunzip: refusing to decode from original element size of %u to %u\n", (unsigned)(((size_t)1) << decoded.size_type), (unsigned)sizeof(Y[0]));
#endif
                return RC(rcXF, rcFunction, rcExecuting, rcRange, rcExcessive);
            }
            
            switch (decoded.diff->var) {
            case 4:
                for (i = 0; i != decoded.diff->used; ++i)
                    Y[i] = (int32_t)decoded.diff->data.u8[i];
                break;
            case 3:
                for (i = 0; i != decoded.diff->used; ++i)
                    Y[i] = (int32_t)decoded.diff->data.u16[i];
                break;
            case 2:
                for (i = 0; i != decoded.diff->used; ++i)
                    Y[i] = (int32_t)decoded.diff->data.u32[i];
                break;
            case 1:
            default:
                for (i = 0; i != decoded.diff->used; ++i)
                    Y[i] = (int32_t)decoded.diff->data.u64[i];
                break;
            }
        }
    }
    if (decoded.diff)   free(decoded.diff);
    if (decoded.type)   free(decoded.type);
    if (decoded.length) free(decoded.length);
    if (decoded.dy)     free(decoded.dy);
    if (decoded.dx)     free(decoded.dx);
    if (decoded.a)      free(decoded.a);
    if (decoded.outlier) free(decoded.outlier);
    
    return rc;
}
