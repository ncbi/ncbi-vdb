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