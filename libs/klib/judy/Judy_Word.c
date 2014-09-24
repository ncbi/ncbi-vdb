#define JUDYL

#ifdef JU_64BIT
#   include "JudyLTables.64.c"
#else
#   include "JudyLTables.32.c"
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

