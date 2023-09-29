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

set( KDB_CMN
    btree
#    rowset
#    rowset-simple

    kdb-cmn
    kdb
    wkdb

    dbmgr-cmn
    manager-base
    dbmgr
    wdbmgr

    database-base
    database
    wdatabase

    table-base
    table
    wtable

    column-base
    column
    wcolumn

    columnblob-base
    rcolumnblob
    wcolumnblob

    meta-base
    meta
    wmeta

    metanode-base
    rmetadatanode
    wmetadatanode

    index-base
    index
    windex

    idxblk
)

set( KDB_SRC
    ${KDB_CMN}


    coldata
    colidx
    colidx0
    colidx1
    colidx2
    idstats
    trieidx-v1
    trieval-v1
    ptrieval-v1
    trieidx-v2
    trieval-v2
    ptrieval-v2
    u64idx-v3
)

set( WKDB_SRC
    ${KDB_CMN}

    wcoldata
    wcolidx
    wcolidx0
    wcolidx1
    wcolidx2
    idxblk
    widxblk
    wtrieidx-v1
    wtrieidx-v2
    wu64idx-v3
)


GenerateStaticLibs( kdb "${KDB_SRC}" )
GenerateStaticLibs( wkdb "${WKDB_SRC}" )

add_compile_definitions (__mod__="libs/kdb")