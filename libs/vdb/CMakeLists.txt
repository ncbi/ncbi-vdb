set( VDBCMN_SRC
    dbmgr-cmn
    schema-tok
    schema-parse
    schema-type
    schema-func
    schema-prod
    schema-expr
    schema-eval
    schema-tbl
    schema-view
    schema-db
    schema-dump
    schema-int
    schema
    linker-int
    linker-cmn
    database-cmn
    database-depend
    database-load
    table-cmn
    table-load
    cursor-cmn
    column-cmn
    prod-cmn
    prod-expr
    prod-func
    phys-cmn
    phys-load
    blob
    blob-headers
    page-map
    row-id
    row-len
    fixed-row-len
    merge
    split
    compare
    meta-read
    meta-value
    meta-attr-read
    cast
    range-validate
    parameter-read
    redimension
    index_project
    index_lookup
    transpose
    delta_average
    report-vdb
    bytecode
    view
    cursor-view
    cursor-table
)

set( VDB_SRC
    ${VDBCMN_SRC}
    dbmgr
    linker
    database
    table
    cursor
    column
    prod
    phys
)

set( WVDB_SRC
    ${VDBCMN_SRC}
    wdbmgr
    wlinker
    wdatabase
    wtable
    wcursor
    wcolumn
    wprod
    wphys
    meta-write
    meta-attr-write
    meta-append
    index_insert
)

GenerateStaticLibs( vdb ${VDB_SRC} )
GenerateStaticLibs( wvdb ${WVDB_SRC} )

add_compile_definitions (__mod__="libs/vdb")