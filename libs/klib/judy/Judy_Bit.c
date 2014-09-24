#define JUDY1

#ifdef JU_64BIT
#   include "Judy1Tables.64.c"
#else
#   include "Judy1Tables.32.c"
#endif

#include "JudyGet.c"
#include "JudyIns.c"
#include "JudyInsArray.c"
#include "JudyDel.c"
#include "JudyCreateBranch.c"
#include "JudyFirst.c"
#include "JudyFreeArray.c"
#include "JudyInsertBranch.c"
#include "JudyMallocIF.c"
#include "JudyMemActive.c"
#include "JudyMemUsed.c"
#include "JudyPrevEmpty.c"
#include "JudyByCountWithDefs.c"
#include "j__udyGet.c"

