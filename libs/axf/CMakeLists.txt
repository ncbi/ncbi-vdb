add_library(axf STATIC
    align-local_ref_id.c
    align-local_ref_start.c
    align-ref-name.c
    align-ref-pos.c
    align-ref-seq-id.c
    align-restore-read.c
    cigar.c
    generate-mismatch-qual.c
    get-sam-flags.c
    get_mate_align_ids.c
    make-cmp-read_desc.c
    Makefile
    mismatch-restore-qual.c
    not-my-row.c
    project_read_from_sequence.c
    range-list.c
    raw-restore-qual.c
    raw-restore-read.c
    ref-preserve_qual.c
    ref-restore-read.c
    ref-tbl-sub-select.c
    ref-tbl.c
    refseq-stats.c
    refseq.c
    restore-read.c
    seq-construct-read.c
    seq-restore-linkage-group.c
    seq-restore-read.c
    template-len.c
    wgs.c
)

add_definitions (-D__mod__="libs/axf")
