
GenerateStaticLibs(judy
    JudyMalloc
    JudySL
    JudyHS
)

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

GenerateStaticLibs(judyL
    ${JUDY_CMN}
    _JudyLTables
)
target_compile_definitions(judyL PRIVATE JUDYL)

GenerateStaticLibs(judy1
    ${JUDY_CMN}
    _Judy1Tables
)
target_compile_definitions(judy1 PRIVATE JUDY1)

add_compile_definitions (__mod__="libs/klib/judy")
include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/..)
