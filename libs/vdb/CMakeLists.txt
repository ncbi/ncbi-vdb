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

GenerateStaticLibs( vdb "${VDB_SRC}" )
GenerateStaticLibs( wvdb "${WVDB_SRC}" )

# suppress certain compiler warnings
if ( "GNU" STREQUAL "${CMAKE_C_COMPILER_ID}")
    set_target_properties( vdb wvdb PROPERTIES COMPILE_FLAGS -Wstrict-aliasing=0 )
elseif ( "Clang" STREQUAL "${CMAKE_C_COMPILER_ID}")
    set_source_files_properties( cast PROPERTIES COMPILE_FLAGS -Wno-implicit-int-float-conversion )
endif()

add_compile_definitions (__mod__="libs/vdb")
