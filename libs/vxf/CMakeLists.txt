# ===========================================================================
#
#                            PUBLIC DOMAIN NOTICE
#               National Center for Biotechnology Information
#
#  This software/database is a "United States Government Work" under the
#  terms of the United States Copyright Act.  It was written as part of
#  the author's official duties as a United States Government employee and
#  thus cannot be copyrighted.  This software/database is freely available
#  to the public for use. The National Library of Medicine and the U.S.
#  Government have not placed any restriction on its use or reproduction.
#
#  Although all reasonable efforts have been taken to ensure the accuracy
#  and reliability of the software and data, the NLM and the U.S.
#  Government do not and cannot warrant the performance or results that
#  may be obtained by using this software or data. The NLM and the U.S.
#  Government disclaim all warranties, express or implied, including
#  warranties of performance, merchantability or fitness for any particular
#  purpose.
#
#  Please cite the author in any work or product based on this material.
#
# ===========================================================================

set( VXF_SRC
    ceil
    clip
    cut
    echo
    exists
    floor
    paste
    round
    trunc
    unzip
    map
    funzip
    vec-sum
    min
    max
    rldecode
    pack
    unpack
    izip
    iunzip
    diff
    sum
    bit_or
    trim
    deriv
    integral
    integral_0
    delta
    undelta
    add-row-id
    subtract-row-id
    irzip
    rand_4na_2na
    outlier-encoder
    outlier-decoder
    bunzip
    simple-sub-select
    extract_token
    strtonum
    sprintf
    wgs-tokenize-accession
    lower-case-tech-reads
    unzstd
)

set( WVXF_SRC
    ${VXF_SRC}
    zip
    bzip
    fzip
    rlencode
    checksum
    zstd
)

GenerateStaticLibs( vxf "${VXF_SRC}" )
GenerateStaticLibs( wvxf "${WVXF_SRC}" )

add_compile_definitions (__mod__="libs/vxf")