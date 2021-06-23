add_library(ngs STATIC
    # BAM_Alignment.c
    # BAM_ReadCollection.c
    # BAM_Record.c
    CSRA1_Alignment.c
    CSRA1_Pileup.c
    CSRA1_PileupEvent.c
    CSRA1_Read.c
    CSRA1_ReadCollection.c
    CSRA1_Reference.c
    CSRA1_ReferenceWindow.c
    EBI_ReferenceSequence.c
    NCBI-NGS.c
    NGS_Alignment.c
    NGS_Cursor.c
    NGS_ErrBlock.c
    NGS_Fragment.c
    NGS_FragmentBlob.c
    NGS_FragmentBlobIterator.c
    NGS_Id.c
    NGS_Pileup.c
    NGS_PileupEvent.c
    NGS_Read.c
    NGS_ReadCollection.c
    NGS_ReadGroup.c
    NGS_Refcount.c
    NGS_Reference.c
    NGS_ReferenceBlob.c
    NGS_ReferenceBlobIterator.c
    NGS_ReferenceSequence.c
    NGS_Statistics.c
    NGS_String.c
    SRA_DB_ReadCollection.c
    SRA_Read.c
    SRA_ReadCollection.c
    SRA_ReadGroup.c
    SRA_ReadGroupInfo.c
    SRA_ReferenceSequence.c
    SRA_Statistics.c
    VByteBlob.c
)

add_definitions (-D__mod__="libs/ngs")