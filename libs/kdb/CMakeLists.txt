set( KDB_CMN
    btree
    rowset
    rowset-simple
    dbmgr-cmn
)

set( KDB_SRC
    ${KDB_CMN}
    kdb
    dbmgr
    dbcc-cmn
    database
    database-cc
    table
    table-cc
    column
    column-cc
    coldata
    colidx
    colidx0
    colidx1
    colidx2
    idxblk
    meta
    index
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
    wkdb
    wdbmgr
    wdatabase
    wtable
    wcolumn
    wcoldata
    wcolidx
    wcolidx0
    wcolidx1
    wcolidx2
    idxblk
    widxblk
    wmeta
    windex
    wtrieidx-v1
    wtrieidx-v2
    wu64idx-v3
)


add_library( kdb STATIC ${KDB_SRC} )
add_library( wkdb STATIC ${WKDB_SRC} )

add_definitions (-D__mod__="libs/kdb")