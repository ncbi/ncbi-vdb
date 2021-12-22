set( AXF_SRC
    ref-tbl-sub-select
    ref-restore-read
    align-restore-read
    raw-restore-read
    raw-restore-qual
    seq-restore-read
    seq-restore-linkage-group
    seq-construct-read
    cigar
    project_read_from_sequence
    not-my-row
    template-len
    get-sam-flags
    align-ref-pos
    align-ref-name
    align-ref-seq-id
    align-local_ref_id
    align-local_ref_start
    generate-mismatch-qual
    mismatch-restore-qual
    ref-preserve_qual
    ref-tbl
    make-cmp-read_desc
    get_mate_align_ids
    range-list
    refseq
    wgs
    restore-read
)

set( WAXF_SRC
    ${AXF_SRC}
    refseq-stats
)

GenerateStaticLibs( axf "${AXF_SRC}" )
GenerateStaticLibs( waxf "${WAXF_SRC}" )

add_compile_definitions( __mod__="libs/axf" )
