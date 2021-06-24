set( SRAXF_SRC
    untyped
    v0-decompress-init
    v0-decompress
    v0-funcs
    bio-start
    dynamic-454-read-descriptor
    format-spot-name
    extract-name_coord
    tokenize-spot_name-454
    tokenize-spot_name-illumina
    tokenize-spot_name-abi
    tokenize-spot_name-helicos
    tokenize-spot_name-ion-torrent
    tokenize-spot_name-generic-fastq
    color-from-dna
    dna-from-color
    use-rna-flag
    spot-desc
    read-desc
    rotate
    swap
    denormalize
    normalize
    qual4_decode
    read-seg-from-readn
    rewrite-spot-name
    make-position
    process-position
    extract-coordinates
    fpcodec
    index_lookup
    varloc
    spot-filter
    syn_quality
)

set( WSRAXF_SRC
    ${SRAXF_SRC}
    extract-name_fmt
    extract-spot_name
    stats
    stats_quality
    qual4_encode
    set-rna-flag
)

add_library( sraxf STATIC ${SRAXF_SRC} )
add_library( wsraxf STATIC ${WSRAXF_SRC} )

add_definitions (-D__mod__="libs/sraxf")