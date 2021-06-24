set ( UNIX_BASE_SRC unix/misc.c unix/syslog.c unix/systime.c unix/syswriter.c )
set ( UNIX_SRC ${UNIX_BASE_SRC} unix/syserrcode.c )
set ( LINUX_SRC ${UNIX_BASE_SRC} linux/syserrcode.c )
set ( MAC_SRC ${UNIX_SRC} )
set ( WIN_SRC win/misc.c win/sysalloc.c win/syserrcode.c win/syslog.c win/systime.c win/syswriter.c )

if ( "win" STREQUAL ${OS} )
    set ( OS_SPECIFIC_SRC ${WIN_SRC} )
elseif( "linux" STREQUAL ${OS} )
    set ( OS_SPECIFIC_SRC ${LINUX_SRC} )
elseif( "mac" STREQUAL ${OS} )
    set ( OS_SPECIFIC_SRC ${MAC_SRC} )
endif()

find_package(FLEX) #TODO: specify minimal version
if( FLEX_FOUND )
    FLEX_TARGET( JsonFlex json-lex.l ${CMAKE_CURRENT_SOURCE_DIR}/json-lex.c
        COMPILE_FLAGS "--debug --noline" )
    set( JsonScanner ${FLEX_JsonFlex_OUTPUTS} )
else()
    set( JsonScanner json-lex.c )
endif()

find_package(BISON) #TODO: specify minimal version
if( BISON_FOUND )
    BISON_TARGET( JsonGrammar json-grammar.y ${CMAKE_CURRENT_SOURCE_DIR}/json-grammar.c
        COMPILE_FLAGS "-Wno-other --no-lines -r state" )
    set( JsonParser ${BISON_JsonGrammar_OUTPUT_SOURCE} )
    ADD_FLEX_BISON_DEPENDENCY(JsonFlex JsonGrammar)
else()
    set( JsonParser json-grammar.c )
endif()

# TODO: it's not working yet
if (CMAKE_BUILD_TYPE EQUAL "Debug")
    message("DEBUG mode klib")
    set( BUILD_SPECIFIC_SRC debug.c )
else()
    message("RELEASE mode klib")
    set( BUILD_SPECIFIC_SRC release.c )
endif (CMAKE_BUILD_TYPE EQUAL "Debug")


add_library(klib STATIC
    base64
    bsearch
    bstpersist #TODO: bstpersist.nopt
    btree
    container
    cp1252
    crc32
    data-buffer
    guid
    hashfile
    hashtable
    iso8859-1
    iso8859
    ${JsonParser}
    ${JsonScanner}
    json
    judy-vector
    ksort
    log
    md5
    namelist
    num-gen
    out
    pack
    pbstree-impl # TODO: pbstree-impl.nopt pbstree-impl.swap.nopt
    #pbstree-native.c
    #pbstree-swapped.c
    pbstree
    printf
    progressbar
    ptpersist # TODO: ptpersist.nopt
    ptrie # TODO: ptrie.nopt
    qsort
    rc-idx-tbl
    rc-tbl
    refcount
    report-klib
    SHA-32bit
    SHA-64bit
    sra-release-version
    status-rc-strings
    status-rc
    status
    symtab
    text
    time
    token
    trie
    unpack
    utf8
    vector
    vector_namelist
    vlen-encode
    writer
    ${BUILD_SPECIFIC_SRC}

# #    judy/Judy1Tables.32.c
# #    judy/Judy1Tables.64.c
    # judy/JudyByCount.c
    # judy/JudyByCountWithDefs.c
    # judy/JudyCascade.c
    # judy/JudyCount.c
    # judy/JudyCreateBranch.c
    # judy/JudyDecascade.c
    # judy/JudyDel.c
    # judy/JudyFirst.c
    # judy/JudyFreeArray.c
    # judy/JudyGet.c
    # judy/JudyHS.c
    # judy/JudyIns.c
    # judy/JudyInsArray.c
    # judy/JudyInsertBranch.c
# #    judy/JudyLTables.32.c
# #    judy/JudyLTables.64.c
    # judy/JudyMalloc.c
    # judy/JudyMallocIF.c
    # judy/JudyMemActive.c
    # judy/JudyMemUsed.c
    # judy/JudyNext.c
    # judy/JudyNextEmpty.c
    # judy/JudyPrev.c
    # judy/JudyPrevEmpty.c
    # judy/JudyPrevNext.c
    # judy/JudyPrevNextEmpty.c
    # judy/JudyPrintJP.c
    # judy/JudySL.c
    # judy/JudyTablesGen.c
    # judy/Judy_Bit.c
    # judy/Judy_Bit_Cascade.c
    # judy/Judy_Bit_Count.c
    # judy/Judy_Bit_Decascade.c
    # judy/Judy_Bit_Next.c
    # judy/Judy_Bit_NextEmpty.c
    # judy/Judy_Bit_Prev.c
    # judy/Judy_Word.c
    # judy/Judy_Word_Cascade.c
    # judy/Judy_Word_Count.c
    # judy/Judy_Word_Decascade.c
    # judy/Judy_Word_Next.c
    # judy/Judy_Word_NextEmpty.c
    # judy/Judy_Word_Prev.c
    # judy/j__udyGet.c
    # judy/_Judy1Tables.c
    # judy/_JudyLTables.c

    ${OS_SPECIFIC_SRC}
)

add_definitions (-D__mod__="libs/klib")
add_definitions (-D_ARCH_BITS=${BITS})
include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR} )

