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

set( KDB_SRC
    btree
#    rowset
#    rowset-simple

    kdb-cmn
    rkdb
    wkdb

    dbmgr-cmn
    manager-base
    database-base
    table-base
    column-base
    columnblob-base.cpp
    meta-base
    metanode-base
    index-base

    ptrieval-v1
    ptrieval-v2

    idstats

    database-cc
    table-cc
    column-cc
    dbcc-cmn

    trieval-v1
    trieval-v2

    index-cmn

    rindex
    ridxblk
    rtrieidx-v1
    rtrieidx-v2
    ru64idx-v3

    windex
    widxblk
    wtrieidx-v1
    wtrieidx-v2
    wu64idx-v3

    rcolumn
#    rcolumnblob
    RColumnBlob.cpp

    wcolumn
#    wcolumnblob
    WColumnBlob.cpp

    rmeta
    rmetadatanode

    wmeta
    wmetadatanode

    coldata
    rcoldata
    rcolidx
    rcolidx0
    rcolidx1
    rcolidx2

    wcoldata
    wcolidx
    wcolidx0
    wcolidx1
    wcolidx2

    rdbmgr
    rtable
    rdatabase

    wdbmgr
    wdatabase
    wtable

    page-map
)

GenerateStaticLibs( kdb "${KDB_SRC}" )

add_compile_definitions (__mod__="libs/kdb")