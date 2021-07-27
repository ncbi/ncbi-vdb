set( SRC_JUDY
    JudyMalloc
    JudySL
    JudyHS
)

GenerateStaticLibs( judy "${SRC_JUDY}" )

set( JUDY_CMN
    JudyGet
    JudyIns
    JudyInsArray
    JudyDel
    JudyCascade
    JudyCount
    JudyCreateBranch
    JudyDecascade
    JudyFirst
    JudyFreeArray
    JudyInsertBranch
    JudyMallocIF
    JudyMemActive
    JudyMemUsed
    JudyNext
    JudyNextEmpty
    JudyPrev
    JudyPrevEmpty
    JudyByCountWithDefs
    j__udyGet
)

# the debug build of Judy is broken!
remove_definitions( -DDEBUG )

set( SRC_JUDYL ${JUDY_CMN} _JudyLTables )
GenerateStaticLibsWithDefs( judyL "${SRC_JUDYL}" "JUDYL" )

set( SRC_JUDY1 ${JUDY_CMN} _Judy1Tables )
GenerateStaticLibsWithDefs( judy1 "${SRC_JUDY1}" "JUDY1" )

add_compile_definitions (__mod__="libs/klib/judy")
include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/..)
