#!perl -w
#/*===========================================================================
#*
#*                            PUBLIC DOMAIN NOTICE
#*               National Center for Biotechnology Information
#*
#*  This software/database is a "United States Government Work" under the
#*  terms of the United States Copyright Act.  It was written as part of
#*  the author's official duties as a United States Government employee and
#*  thus cannot be copyrighted.  This software/database is freely available
#*  to the public for use. The National Library of Medicine and the U.S.
#*  Government have not placed any restriction on its use or reproduction.
#*
#*  Although all reasonable efforts have been taken to ensure the accuracy
#*  and reliability of the software and data, the NLM and the U.S.
#*  Government do not and cannot warrant the performance or results that
#*  may be obtained by using this software or data. The NLM and the U.S.
#*  Government disclaim all warranties, express or implied, including
#*  warranties of performance, merchantability or fitness for any particular
#*  purpose.
#*
#*  Please cite the author in any work or product based on this material.
#*
#* ===========================================================================
#*
#*/
# gen-cast.pl

my $type = [
    { size =>  8, integer => 1, signed => 1, ctype => 'int8_t', minimum => 'INT8_MIN', maximum => 'INT8_MAX' },
    { size => 16, integer => 1, signed => 1, ctype => 'int16_t', minimum => 'INT16_MIN', maximum => 'INT16_MAX' },
    { size => 32, integer => 1, signed => 1, ctype => 'int32_t', minimum => 'INT32_MIN', maximum => 'INT32_MAX' },
    { size => 64, integer => 1, signed => 1, ctype => 'int64_t', minimum => 'INT64_MIN', maximum => 'INT64_MAX' },
    { size =>  8, integer => 1, signed => 0, ctype => 'uint8_t', minimum => '0', maximum => 'UINT8_MAX' },
    { size => 16, integer => 1, signed => 0, ctype => 'uint16_t', minimum => '0', maximum => 'UINT16_MAX' },
    { size => 32, integer => 1, signed => 0, ctype => 'uint32_t', minimum => '0', maximum => 'UINT32_MAX' },
    { size => 64, integer => 1, signed => 0, ctype => 'uint64_t', minimum => '0', maximum => 'UINT64_MAX' },
    { size => 32, integer => 0, signed => 1, ctype => 'float', minimum => '(-FLT_MAX)', maximum => 'FLT_MAX' },
    { size => 64, integer => 0, signed => 1, ctype => 'double', minimum => '(-DBL_MAX)', maximum => 'DBL_MAX' },
];

sub type_name {
    my $i = shift;
    
    return ($type->[$i]{'integer'} ? ($type->[$i]{'signed'} ? 'I' : 'U') : 'F') . $type->[$i]{'size'};
}

sub func_name {
    my ($i, $j) = (shift, shift);
    
    return $i == $j ? 'copy' : 'cast_' . type_name($i) . '_to_' . type_name($j);
}

print <<EOD;
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

#include <klib/defs.h>
#include <klib/rc.h>
#include <klib/data-buffer.h>
#include <klib/text.h>
#include <vdb/types.h>
#include <vdb/xform.h>
#include <vdb/schema.h>

#include <stdint.h>
#include <stdlib.h>

#include <limits.h>
#include <float.h>
#include <string.h>

#include <assert.h>

typedef struct self_t {
    VTypedesc src;
    VTypedesc dst;
} self_t;

static rc_t copy(
    void *Self,
    const VXformInfo *info,
    void *dst,
    const void *src,
    uint32_t num_elements
) {
    const self_t *self = Self;
    memcpy(dst, src, ((size_t)num_elements * self->src.intrinsic_bits * self->src.intrinsic_dim + 7) >> 3);
    return 0;
}

EOD

for (my $i = 0; $i != scalar(@$type); ++$i) {
    for (my $j = 0; $j != scalar(@$type); ++$j) {
        my $name;
        my $stype = $type->[$i]{'ctype'}; # source c type
        my $dtype = $type->[$j]{'ctype'}; # dest. c type
        my $intrm = $stype;
        my $lbc; # lower bounds check
        my $ubc; # upper bounds check
        
        next if ($j == $i);
        $name = func_name($i, $j);

        if ($type->[$i]{'integer'} == $type->[$j]{'integer'}) {
            if ($type->[$i]{'signed'} == $type->[$j]{'signed'}) {
                if ($type->[$i]{'size'} > $type->[$j]{'size'}) {
                    $lbc = "(($intrm)($type->[$j]{'minimum'}))" if ($type->[$i]{'signed'});
                    $ubc = "(($intrm)($type->[$j]{'maximum'}))";
                }
                goto PRINT_FUNC;
            }
            if ($type->[$i]{'signed'}) {
                # signed to unsigned
                $lbc = ' 0';
                $ubc = "(($intrm)($type->[$j]{'maximum'}))" if ($type->[$i]{'size'} > $type->[$j]{'size'});
                goto PRINT_FUNC;
            }
            # unsigned to signed
            $ubc = "(($intrm)($type->[$j]{'maximum'}))" if ($type->[$i]{'size'} >= $type->[$j]{'size'});
            goto PRINT_FUNC;
        }
        if ($type->[$i]{'integer'}) {
            #int -> float
            goto PRINT_FUNC if ($type->[$i]{'size'} < 64);
            if ($type->[$j]{'size'} < 64) {
                $intrm = 'double';
                $lbc = "(($intrm)($type->[$j]{'minimum'}))";
                $ubc = "(($intrm)($type->[$j]{'maximum'}))";
            }
            goto PRINT_FUNC;
        }
        $lbc = $type->[$j]{'signed'} ? "(($intrm)($type->[$j]{'minimum'}))" : ' 0.0';
        $ubc = "(($intrm)($type->[$j]{'maximum'}))";
        goto PRINT_FUNC if ($type->[$j]{'size'} < 64);
        $intrm = 'double';
        $lbc = $type->[$j]{'signed'} ? "(($intrm)($type->[$j]{'minimum'}))" : ' 0.0';
        $ubc = "(($intrm)($type->[$j]{'maximum'}))";
        
PRINT_FUNC:
        if ($lbc || $ubc) {
            if ($lbc && $ubc) {
                print <<EOD;
static rc_t $name(
    void *Self,
    const VXformInfo *info,
    void *Dst,
    const void *Src,
    uint32_t num_elements
) {
    const self_t *self = Self;
    const unsigned N = num_elements * self->src.intrinsic_dim;
    const $stype *src = Src;
    $dtype *dst = Dst;
    unsigned i;
    
    for (i = 0; i != N; ++i) {
        $intrm y = src[i];
        
        if ($lbc > y || y > $ubc)
            return RC(rcVDB, rcFunction, rcExecuting, rcRange, rcExcessive);

        dst[i] = y;
    }
    return 0;
}

EOD
            }
            else {
                if ($lbc) {
                    print <<EOD;
static rc_t $name(
    void *Self,
    const VXformInfo *info,
    void *Dst,
    const void *Src,
    uint32_t num_elements
) {
    const self_t *self = Self;
    const unsigned N = num_elements * self->src.intrinsic_dim;
    const $stype *src = Src;
    $dtype *dst = Dst;
    unsigned i;
    
    for (i = 0; i != N; ++i) {
        $intrm y = src[i];
        
        if ($lbc > y)
            return RC(rcVDB, rcFunction, rcExecuting, rcRange, rcExcessive);

        dst[i] = y;
    }
    return 0;
}

EOD
                }
                else {
                    print <<EOD;
static rc_t $name(
    void *Self,
    const VXformInfo *info,
    void *Dst,
    const void *Src,
    uint32_t num_elements
) {
    const self_t *self = Self;
    const unsigned N = num_elements * self->src.intrinsic_dim;
    const $stype *src = Src;
    $dtype *dst = Dst;
    unsigned i;
    
    for (i = 0; i != N; ++i) {
        $intrm y = src[i];
        
        if (y > $ubc)
            return RC(rcVDB, rcFunction, rcExecuting, rcRange, rcExcessive);

        dst[i] = y;
    }
    return 0;
}

EOD
                }
            }
        }
        else {
            print <<EOD;
static rc_t $name(
    void *Self,
    const VXformInfo *info,
    void *Dst,
    const void *Src,
    uint32_t num_elements
) {
    const self_t *self = Self;
    const unsigned N = num_elements * self->src.intrinsic_dim;
    const $stype *src = Src;
    $dtype *dst = Dst;
    unsigned i;
    
    for (i = 0; i != N; ++i)
        dst[i] = src[i];

    return 0;
}

EOD
        }
    }
}

print 'static const VArrayFunc funcs[' . scalar(@$type) . '][' . scalar(@$type) . '] = {' . "\n";
for (my $i = 0; $i != scalar(@$type); ++$i) {
    my @x;
    
    for (my $j = 0; $j != scalar(@$type); ++$j) {
        push @x, func_name($i, $j);
    }
    print "\t{ " . join(', ', @x) . "},\n";
}
print <<EOD;
};

static int desc_to_number(const VTypedesc *desc) {
    switch (desc->domain) {
    case vtdInt:
        switch (desc->intrinsic_bits) {
        case 8:
            return 0;
        case 16:
            return 1;
        case 32:
            return 2;
        case 64:
            return 3;
        }
        break;
    case vtdUint:
        switch (desc->intrinsic_bits) {
        case 8:
            return 4;
        case 16:
            return 5;
        case 32:
            return 6;
        case 64:
            return 7;
        }
        break;
    case vtdFloat:
        switch (desc->intrinsic_bits) {
        case 32:
            return 8;
        case 64:
            return 9;
        }
        break;
    }
    return -1;
}

static rc_t cast_to_ascii(
    void *Self,
    const VXformInfo *info,
    int64_t row_id,
    VRowResult *rslt,
    uint32_t argc,
    const VRowData argv[]
) {
    const self_t *self = Self;
    rc_t rc;
    uint32_t intrm;
    unsigned i;
    int j;
    unsigned k;
    
    rc = KDataBufferResize(rslt->data, argv[0].u.data.element_count);
    if (rc)
        return rc;
    
    for (i = 0, k = 0; k != argv[0].u.data.element_count && i < argv[0].u.data.element_count; ++k) {
        j = 1;
        switch (self->src.intrinsic_bits) {
        case 8:
            j = utf8_utf32(&intrm, ((const char *)argv[0].u.data.base) + i, ((const char *)argv[0].u.data.base) + argv[0].u.data.element_count);
            if (j == 0)
                return RC(rcVDB, rcFunction, rcExecuting, rcData, rcInsufficient);
            if (j < 0)
                return RC(rcVDB, rcFunction, rcExecuting, rcData, rcInvalid);
            break;
        case 16:
            intrm = ((const uint16_t *)argv[0].u.data.base)[i];
            break;
        case 32:
            intrm = ((const uint32_t *)argv[0].u.data.base)[i];
            break;
        }
        ((char *)rslt->data->base)[k] = intrm < 128 ? (char)intrm : '.';
        i += j;
    }
    rslt->element_count = k;
    return 0;
}


static rc_t cast_to_unicode(
    void *Self,
    const VXformInfo *info,
    int64_t row_id,
    VRowResult *rslt,
    uint32_t argc,
    const VRowData argv[]
) {
    const self_t *self = Self;
    rc_t rc;
    uint32_t intrm;
    unsigned i;
    int j;
    unsigned k;
    
    rc = KDataBufferResize(rslt->data, argv[0].u.data.element_count + 8);
    if (rc)
        return rc;
    
    for (i = 0, k = 0; i != argv[0].u.data.element_count; ) {
        if (k + 8 >= rslt->data->elem_count) {
            rc = KDataBufferResize(rslt->data, rslt->data->elem_count << 1);
            if (rc)
                return rc;
        }
        j = 1;
        switch (self->src.intrinsic_bits) {
        case 8:
            if (self->src.domain == vtdAscii)
                intrm = ((const char *)argv[0].u.data.base)[i];
            else {
                j = utf8_utf32(&intrm, ((const char *)argv[0].u.data.base) + i, ((const char *)argv[0].u.data.base) + argv[0].u.data.element_count);
                if (j == 0)
                    return RC(rcVDB, rcFunction, rcExecuting, rcData, rcInsufficient);
                if (j < 0)
                    return RC(rcVDB, rcFunction, rcExecuting, rcData, rcInvalid);
            }
            break;
        case 16:
            intrm = ((const uint16_t *)argv[0].u.data.base)[i];
            break;
        case 32:
            intrm = ((const uint32_t *)argv[0].u.data.base)[i];
            break;
        }
        i += j;
        j = 1;
        switch (self->dst.intrinsic_bits) {
        case 8:
            j = utf32_utf8(((char *)rslt->data->base) + k, ((char *)rslt->data->base) + k + 8, intrm);
            assert(j > 0);
            break;
        case 16:
            ((uint16_t *)rslt->data->base)[k] = intrm < 0x10000 ? intrm : '.';
            break;
        case 32:
            ((uint32_t *)rslt->data->base)[k] = intrm;
            break;
        }
        k += j;
    }
    rslt->element_count = k;    
    return 0;
}

/* 
 */
VTRANSFACT_IMPL(vdb_cast, 1, 0, 0) (const void *Self, const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rc_t rc = 0;
    self_t *self;
    
    self = malloc(sizeof(self_t));
    if (self) {
        int i = 0;
        int j = 0;
        
        rslt->self = self;
        rslt->whack = free;
        
        self->src = dp->argv[0].desc;
        self->dst = info->fdesc.desc;
        
        i = desc_to_number(&self->src);
        j = desc_to_number(&self->dst);
        
        if (i >= 0 && j >= 0) {
            rslt->variant = vftArray;
            rslt->u.af = funcs[i][j];
            
            return 0;
        }
        if ((self->dst.domain == vtdAscii || self->dst.domain == vtdUnicode) && (self->src.domain == vtdAscii || self->src.domain == vtdUnicode)) {
            rslt->variant = vftRow;
            rslt->u.rf = self->dst.domain == vtdAscii ? cast_to_ascii : cast_to_unicode;
            
            return 0;
        }
        rc = RC(rcVDB, rcFunction, rcConstructing, rcParam, rcInvalid);
        free(self);
    }
    else
        rc = RC(rcVDB, rcFunction, rcConstructing, rcMemory, rcExhausted);
    return rc;
}
EOD

__END__
