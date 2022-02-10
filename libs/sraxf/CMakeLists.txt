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

GenerateStaticLibs( sraxf "${SRAXF_SRC}" )
GenerateStaticLibs( wsraxf "${WSRAXF_SRC}" )

add_compile_definitions (__mod__="libs/sraxf")