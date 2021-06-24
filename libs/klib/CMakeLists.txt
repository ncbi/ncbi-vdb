if ( "win" STREQUAL ${OS} )
    set (OS_SPECIFIC_SRC win/misc.c win/sysalloc.c win/syserrcode.c win/syslog.c win/systime.c win/syswriter.c)
elseif( "unix" STREQUAL ${OS} )
    set (OS_SPECIFIC_SRC unix/misc.c unix/syserrcode.c unix/syslog.c unix/systime.c unix/syswriter.c)
elseif( "linux" STREQUAL ${OS} )
    set (OS_SPECIFIC_SRC linux/syserrcode.c)
elseif( "mac" STREQUAL ${OS} )
    set (OS_SPECIFIC_SRC unix/misc.c unix/syserrcode.c unix/syslog.c unix/systime.c unix/syswriter.c)
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

add_library(klib STATIC
    base64.c
    bsearch.c
    bstpersist.c
    btree.c
    container.c
    cp1252.c
    crc32.c
    data-buffer.c
    debug.c
    guid.c
    hashfile.c
    hashtable.c
    iso8859-1.c
    iso8859.c
    ${JsonParser}
    ${JsonScanner}
    json.c
    judy-vector.c
    ksort.c
    log.c
    md5.c
    namelist.c
    num-gen.c
    out.c
    pack.c
    pbstree-impl.c
    pbstree-native.c
    pbstree-swapped.c
    pbstree.c
    printf.c
    progressbar.c
    ptpersist.c
    ptrie.c
    qsort.c
    rc-idx-tbl.c
    rc-tbl.c
    refcount.c
    release.c
    report-klib.c
    SHA-32bit.c
    SHA-64bit.c
    sra-release-version.c
    status-rc-strings.c
    status-rc.c
    status.c
    symtab.c
    text.c
    time.c
    token.c
    trie.c
    unpack.c
    utf8.c
    vector.c
    vector_namelist.c
    vlen-encode.c
    writer.c

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

