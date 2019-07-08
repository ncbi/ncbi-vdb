/*===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software/database is a "United States Government Work" under the
*  terms of the United States Copyright Act.  It was written as part of
*  the author's official duties as a United States Government employee and
*  thus cannot be copyrighted.  This software/database is freely available
*  to the public for use. The National Library of Medicine and the U.S.
*  Government have not placed any restriction on its use or reproduction.
*
*  Although all reasonable efforts have been taken to ensure the accuracy
*  and reliability of the software and data, the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data. The NLM and the U.S.
*  Government disclaim all warranties, express or implied, including
*  warranties of performance, merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
*/


#ifndef _h_klib_debug_
#define _h_klib_debug_

#if _DEBUGGING

#ifndef _h_klib_extern_
#include <klib/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifndef _h_klib_status_
#include <klib/status.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ----------
 * KDbgModule and KDbgCondition
 *
 * KDbg messages are filtered by a pair of values: a module and a condition.
 *
 * A KDbgFlag is a single bit representing an enumerated condition ID
 *
 * In this context the module is merely a set and not specifically tied to any
 * other use of the term module.
 */



/*
 * To add a new module, just add it to the list in MODULE_NAMES but as a 
 * parameter to the macro "_module"
 *
 * Then add the conditions for that module.  There must be at least one.
 * Put them in the macro "_condition"
 *
 * NOTE:
 * There must be at least one condition for a module.
 *
 * NOTE:
 * Do not seperate the items in eith MODULES or xxx_CONDITIONS by anything 
 * other than white space.
 *
 * NOTE:
 * The exact text must used in the _module(MOD) and the MOD_CONDITIONS()
 * so if you want all lower, all upper ot mixed case it must be the same.
 * this exact case will apply to the command line argument as well.
 *
 * NOTE:
 * The MOD and CONDITIONS must be legal C Identifiers. The exception is a
 * condition may start with a numeric or even be all numeric.  Punctuation other
 * than '_' is not allowed.
 *
 * NOTE:
 * No not use condition "ANY" or "NONE" as they are special flag names.
 *
 * The rest of the file will take care of adding the new strings, enumerations
 * and structures for the new modules and/or new conditions.
 *
 * The expected command line will be "-D mod" to add "any" for the module or
 * "-D mod-cond" to add only that condition with in the module.
 * "-D" and "--debug" are expected to be equivalent"
 *
 * The expected usage in a C source file is to use the flag value.
 * For an added module (MMM) and condition (CCC) the symbols used would be
 * "dbg_MMM" and "dbg_MMM_CCC" that will get an index
 * to a flag value and the flag to test for a condition.
 *
 * Any number of individual "-D conditions" are accepted.
 */

#define MODULE_NAMES() \
    _module(AES)    _module(ALIGN)   _module(APP)  _module(ARGS) \
    _module(BLAST)  _module(KDB) \
    _module(KFG)    _module(KFS)     _module(KNS)  _module(KRYPTO) \
    _module(LEGREF) _module(LOADLIB) \
    _module(REF)    _module(SEARCH)  _module(SRA) \
    _module(VDB)    _module(VFS)     _module(XARC) _module(XML)  \
    _module(TLS)

#define APP_CONDITIONS() \
    _condition(APP,0)  _condition(APP,1)  _condition(APP,2)  _condition(APP,3)  \
    _condition(APP,4)  _condition(APP,5)  _condition(APP,6)  _condition(APP,7)  \
    _condition(APP,8)  _condition(APP,9)  _condition(APP,10) _condition(APP,11) \
    _condition(APP,12) _condition(APP,13) _condition(APP,14) _condition(APP,15) \
    _condition(APP,16) _condition(APP,17) _condition(APP,18) _condition(APP,19) \
    _condition(APP,20) _condition(APP,21) _condition(APP,22) _condition(APP,23) \
    _condition(APP,24) _condition(APP,25) _condition(APP,26) _condition(APP,27) \
    _condition(APP,28) _condition(APP,29) _condition(APP,30) _condition(APP,31) \
    _condition(APP,32) _condition(APP,33) _condition(APP,34) _condition(APP,35) \
    _condition(APP,36) _condition(APP,37) _condition(APP,38) _condition(APP,39) \
    _condition(APP,40) _condition(APP,41) _condition(APP,42) _condition(APP,43) \
    _condition(APP,44) _condition(APP,45) _condition(APP,46) _condition(APP,47) \
    _condition(APP,48) _condition(APP,49) _condition(APP,50) _condition(APP,51) \
    _condition(APP,52) _condition(APP,53) _condition(APP,54) _condition(APP,55) \
    _condition(APP,56) _condition(APP,57) _condition(APP,58) _condition(APP,59) \
    _condition(APP,60) _condition(APP,61) _condition(APP,62) _condition(APP,63) 

#define BLAST_CONDITIONS() \
    _condition(BLAST,BLAST)

#define KDB_CONDITIONS() \
    _condition(KDB,KDB) _condition(KDB,POS)

/* place holder should be replaced by the first object that gets the debug stuff tested for krefcount_t */
#define REF_CONDITIONS() \
    _condition(REF,PLACEHOLDER)

#define LEGREF_CONDITIONS() \
    _condition(LEGREF,MD5) _condition(LEGREF,CRC)

#define KFS_CONDITIONS() \
    _condition(KFS,MD5)      _condition(KFS,DLL) _condition(KFS,KFFENTRY) _condition(KFS,KFF)  \
    _condition(KFS,ARCENTRY) _condition(KFS,ARC) _condition(KFS,TOCENTRY) _condition(KFS,TOC)  \
    _condition(KFS,TARENTRY) _condition(KFS,TAR) _condition(KFS,SRASORT)  _condition(KFS,GZIP) \
    _condition(KFS,DIR)  _condition(KFS,COUNTER) _condition(KFS,BZIP)     _condition(KFS,SYS) \
    _condition(KFS,POS)  _condition(KFS,PAGE) _condition(KFS,FILE)

#define KNS_CONDITIONS() \
    _condition(KNS,DNS) _condition(KNS,ERR)   _condition(KNS,HTTP) \
    _condition(KNS,MGR) _condition(KNS,PROXY) _condition(KNS,SOCKET) \
    _condition(KNS,TLS)

#define VFS_CONDITIONS() \
    _condition(VFS,JSON)     _condition(VFS,KFG)     _condition(VFS,MGR) \
    _condition(VFS,PATH)     _condition(VFS,SERVICE)

#define XML_CONDITIONS() \
    _condition(XML,XML)

#define VDB_CONDITIONS() \
    _condition(VDB,RESOLVE) \
    _condition(VDB,COMPARE) \
    _condition(VDB,FUNCTION) \
    _condition(VDB,VDB) \
    _condition(VDB,PARSE) \
    _condition(VDB,MTCURSOR)

#define SRA_CONDITIONS() \
    _condition(SRA,INFO) _condition(SRA,SORT) _condition(SRA,SUB) 

#define XARC_CONDITIONS() \
    _condition(XARC,ENTRY) \
    _condition(XARC,ARC)

#define ALIGN_CONDITIONS() \
    _condition(ALIGN,WRITER) _condition(ALIGN,COMPRESS) _condition(ALIGN,COVERAGE) \
    _condition(ALIGN,BAM) _condition(ALIGN,BGZF) _condition(ALIGN,CFG)

#define KFG_CONDITIONS() \
    _condition(KFG,LOAD) _condition(KFG,NODE)

#define KRYPTO_CONDITIONS() \
    _condition(KRYPTO,STS) \
    _condition(KRYPTO,CFG) \
    _condition(KRYPTO,ENCRYPT) \
    _condition(KRYPTO,DECRYPT)

#define AES_CONDITIONS() \
    _condition(AES,KEYEXP) _condition(AES,CIPHER) \
    _condition(AES,INVKEYEXP) _condition(AES,INVCIPHER) \
    _condition(AES,OBJECT)

#define SEARCH_CONDITIONS() \
    _condition(SEARCH,MYERS)

#define LOADLIB_CONDITIONS() \
    _condition(LOADLIB,PBAR) _condition(LOADLIB,FILE) _condition(LOADLIB,XLOG)

#define ARGS_CONDITIONS() \
    _condition(ARGS,WRITER)

#define TLS_CONDITIONS() \
    _condition(TLS,TLS)
    
/*
 * Nothing below here needs to be changed when just adding new modules
 * and/or conditions
 */

/* macros to do preprocessor token pasting and stringification */
#define DBG_PASTE_2(a,b)        a##b
#define DBG_PASTE_3(a,b,c)      a##b##c
#define DBG_PASTE_4(a,b,c,d)    a##b##c##d
#define DBG_PASTE_5(a,b,c,d,e)  a##b##c##d##e
#define DBG_STRING(a)           #a

/*
 * Build the module enumerations/flags.
 */
#define _module(mod)   DBG_PASTE_2(DBG_,mod),

typedef int32_t KDbgMod;
enum
{
    DBG_MOD_NOT_FOUND = -1,
    MODULE_NAMES()
    DBG_MOD_COUNT
};

#undef _module


/* 
 * the dbg_id and dbg_flag types are for ease of generically
 * typing flags with more specific types defined below.  These are more fully
 * filled out to help debuggers more than any real need for the code to work
 *
 * We'll just assume a limit of 32 bits for an enumeration.
 */

/* dbg_id is approprite as a array index */
    typedef uint32_t KDbgCond;
enum
{
    DBG_COND_MIN = 0,
    DBG_COND_0 = DBG_COND_MIN,      DBG_COND_1,     DBG_COND_2,
    DBG_COND_3,     DBG_COND_4,     DBG_COND_5,     DBG_COND_6,
    DBG_COND_7,     DBG_COND_8,     DBG_COND_9,     DBG_COND_10,
    DBG_COND_11,    DBG_COND_12,    DBG_COND_13,    DBG_COND_14,
    DBG_COND_15,    DBG_COND_16,    DBG_COND_17,    DBG_COND_18,
    DBG_COND_19,    DBG_COND_20,    DBG_COND_21,    DBG_COND_22,
    DBG_COND_23,    DBG_COND_24,    DBG_COND_25,    DBG_COND_26,
    DBG_COND_27,    DBG_COND_28,    DBG_COND_29,    DBG_COND_30,
    DBG_COND_31,    DBG_COND_32,    DBG_COND_33,    DBG_COND_34,
    DBG_COND_35,    DBG_COND_36,    DBG_COND_37,    DBG_COND_38,
    DBG_COND_39,    DBG_COND_40,    DBG_COND_41,    DBG_COND_42,
    DBG_COND_43,    DBG_COND_44,    DBG_COND_45,    DBG_COND_46,
    DBG_COND_47,    DBG_COND_48,    DBG_COND_49,    DBG_COND_50,
    DBG_COND_51,    DBG_COND_52,    DBG_COND_53,    DBG_COND_54,
    DBG_COND_55,    DBG_COND_56,    DBG_COND_57,    DBG_COND_58,
    DBG_COND_59,    DBG_COND_60,    DBG_COND_61,    DBG_COND_62,
    DBG_COND_63,    DBG_COND_MAX = DBG_COND_63
};

typedef uint64_t KDbgFlag;
typedef uint64_t KDbgMask;


/* to make a KDbgFlag out of a KDbgCond, shift 1 one by the condition id */
/* to make as mask out of flags just OR them together */

KLIB_EXTERN KDbgFlag CC KDbgCondToFlag ( KDbgCond cond );

#if LOG_INLINING
#define DBG_FLAG(debug_cond) (((KDbgFlag)1)<<debug_cond)
#else
#define DBG_FLAG(debug_cond) KDbgCondToFlag(debug_cond)
#endif


/* dbg_flag is a type that can be used to and against the enabled fkags/masks */
typedef uint64_t dbg_flag;

#define DBG_FLAG_NONE 0
#define DBG_FLAG_0  (((dbg_flag)1)<<DBG_COND_0)
#define DBG_FLAG_1  (((dbg_flag)1)<<DBG_COND_1)
#define DBG_FLAG_2  (((dbg_flag)1)<<DBG_COND_2)
#define DBG_FLAG_3  (((dbg_flag)1)<<DBG_COND_3)
#define DBG_FLAG_4  (((dbg_flag)1)<<DBG_COND_4)
#define DBG_FLAG_5  (((dbg_flag)1)<<DBG_COND_5)
#define DBG_FLAG_6  (((dbg_flag)1)<<DBG_COND_6)
#define DBG_FLAG_7  (((dbg_flag)1)<<DBG_COND_7)
#define DBG_FLAG_8  (((dbg_flag)1)<<DBG_COND_8)
#define DBG_FLAG_9  (((dbg_flag)1)<<DBG_COND_9)
#define DBG_FLAG_10 (((dbg_flag)1)<<DBG_COND_10)
#define DBG_FLAG_11 (((dbg_flag)1)<<DBG_COND_11)
#define DBG_FLAG_12 (((dbg_flag)1)<<DBG_COND_12)
#define DBG_FLAG_13 (((dbg_flag)1)<<DBG_COND_13)
#define DBG_FLAG_14 (((dbg_flag)1)<<DBG_COND_14)
#define DBG_FLAG_15 (((dbg_flag)1)<<DBG_COND_15)
#define DBG_FLAG_16 (((dbg_flag)1)<<DBG_COND_16)
#define DBG_FLAG_17 (((dbg_flag)1)<<DBG_COND_17)
#define DBG_FLAG_18 (((dbg_flag)1)<<DBG_COND_18)
#define DBG_FLAG_19 (((dbg_flag)1)<<DBG_COND_19)
#define DBG_FLAG_20 (((dbg_flag)1)<<DBG_COND_20)
#define DBG_FLAG_21 (((dbg_flag)1)<<DBG_COND_21)
#define DBG_FLAG_22 (((dbg_flag)1)<<DBG_COND_22)
#define DBG_FLAG_23 (((dbg_flag)1)<<DBG_COND_23)
#define DBG_FLAG_24 (((dbg_flag)1)<<DBG_COND_24)
#define DBG_FLAG_25 (((dbg_flag)1)<<DBG_COND_25)
#define DBG_FLAG_26 (((dbg_flag)1)<<DBG_COND_26)
#define DBG_FLAG_27 (((dbg_flag)1)<<DBG_COND_27)
#define DBG_FLAG_28 (((dbg_flag)1)<<DBG_COND_28)
#define DBG_FLAG_29 (((dbg_flag)1)<<DBG_COND_29)
#define DBG_FLAG_30 (((dbg_flag)1)<<DBG_COND_30)
#define DBG_FLAG_31 (((dbg_flag)1)<<DBG_COND_31)
#define DBG_FLAG_32 (((dbg_flag)1)<<DBG_COND_32)
#define DBG_FLAG_33 (((dbg_flag)1)<<DBG_COND_33)
#define DBG_FLAG_34 (((dbg_flag)1)<<DBG_COND_34)
#define DBG_FLAG_35 (((dbg_flag)1)<<DBG_COND_35)
#define DBG_FLAG_36 (((dbg_flag)1)<<DBG_COND_36)
#define DBG_FLAG_37 (((dbg_flag)1)<<DBG_COND_37)
#define DBG_FLAG_38 (((dbg_flag)1)<<DBG_COND_38)
#define DBG_FLAG_39 (((dbg_flag)1)<<DBG_COND_39)
#define DBG_FLAG_40 (((dbg_flag)1)<<DBG_COND_40)
#define DBG_FLAG_41 (((dbg_flag)1)<<DBG_COND_41)
#define DBG_FLAG_42 (((dbg_flag)1)<<DBG_COND_42)
#define DBG_FLAG_43 (((dbg_flag)1)<<DBG_COND_43)
#define DBG_FLAG_44 (((dbg_flag)1)<<DBG_COND_44)
#define DBG_FLAG_45 (((dbg_flag)1)<<DBG_COND_45)
#define DBG_FLAG_46 (((dbg_flag)1)<<DBG_COND_46)
#define DBG_FLAG_47 (((dbg_flag)1)<<DBG_COND_47)
#define DBG_FLAG_48 (((dbg_flag)1)<<DBG_COND_48)
#define DBG_FLAG_49 (((dbg_flag)1)<<DBG_COND_49)
#define DBG_FLAG_50 (((dbg_flag)1)<<DBG_COND_50)
#define DBG_FLAG_51 (((dbg_flag)1)<<DBG_COND_51)
#define DBG_FLAG_52 (((dbg_flag)1)<<DBG_COND_52)
#define DBG_FLAG_53 (((dbg_flag)1)<<DBG_COND_53)
#define DBG_FLAG_54 (((dbg_flag)1)<<DBG_COND_54)
#define DBG_FLAG_55 (((dbg_flag)1)<<DBG_COND_55)
#define DBG_FLAG_56 (((dbg_flag)1)<<DBG_COND_56)
#define DBG_FLAG_57 (((dbg_flag)1)<<DBG_COND_57)
#define DBG_FLAG_58 (((dbg_flag)1)<<DBG_COND_58)
#define DBG_FLAG_59 (((dbg_flag)1)<<DBG_COND_59)
#define DBG_FLAG_60 (((dbg_flag)1)<<DBG_COND_60)
#define DBG_FLAG_61 (((dbg_flag)1)<<DBG_COND_61)
#define DBG_FLAG_62 (((dbg_flag)1)<<DBG_COND_62)
#define DBG_FLAG_63 (((dbg_flag)1)<<DBG_COND_63)
#define DBG_FLAG_ANY (~(dbg_flag)0)

/*
 * Build the list of enums for the various modules.
 * These are descendant types of KDbgCond and KDbgFlag
 *
 * The use of ',' or ';' at the end of macros is precise to suit the use
 * in the initializers and enum definitions.
 *
 * These usually means no ',' or ';' at the end of instantiations.
 */
#define _module(mod)                            \
    typedef int32_t DBG_PASTE_2(DBG_COND_,mod); \
    enum                                        \
        {                                       \
        DBG_PASTE_2(mod,_CONDITIONS())          \
        DBG_PASTE_3(DBG_,mod,_COUNT)            \
    };
#define _condition(mod,flag) DBG_PASTE_4(DBG_,mod,_,flag),

MODULE_NAMES()

#undef _condition
#undef _module

typedef struct dbg_s_flag dbg_s_flag;
struct dbg_s_flag
{
    const char * name;
    KDbgFlag flag;
};

typedef struct dbg_s_mod dbg_s_mod;
struct dbg_s_mod
{
    const char *       name;  /* matches item in MODULE_NAMES() */
    const dbg_s_flag *  conds; /* array of structures for setting flags */
    KDbgMask           flags; /* which flags are active */
};

extern dbg_s_mod dbg_flag_mod     [DBG_MOD_COUNT+1];


#define _module(mod) \
    extern KDbgMask DBG_PASTE_3(DBG_,mod,_ANY);

MODULE_NAMES()

#undef _module


/*
 * strings will be defined only in the single C file that should
 * define the symbol below before including this file.
 *
 * This could have been done in the C file but was done here to
 * keep the spirit of the work done all in one place.
 */

#ifdef _KLIB_DEBUG_C_

#define _module(mod) \
    KDbgFlag DBG_PASTE_3(DBG_,mod,_ANY) = ( DBG_PASTE_2(mod,_CONDITIONS()) 0);
#define _condition(mod,flag) \
    (((KDbgFlag)1)<<DBG_PASTE_4(DBG_,mod,_,flag)) |

MODULE_NAMES()

#undef _module
#undef _condition

#define _module(mod)                            \
    dbg_s_flag DBG_PASTE_2(dbg_s_flag_,mod) [] = \
    {                                            \
        DBG_PASTE_2(mod,_CONDITIONS())           \
        { NULL, DBG_FLAG_NONE }                      \
    };

#define _condition(mod,flag)                                           \
    { DBG_STRING(flag), ((KDbgFlag)1)<<DBG_PASTE_4(DBG_,mod,_,flag) },

MODULE_NAMES()

#undef _module
#undef _condition

#define _module(mod)                           \
    {                                           \
        DBG_STRING(mod),                         \
        DBG_PASTE_2(dbg_s_flag_,mod),        \
        0                                       \
    },

dbg_s_mod dbg_flag_mod [] = 
{
    MODULE_NAMES()
    { NULL, NULL }
};
#undef _module

#define _module(mod) 

#endif /* if _KLIB_TEST_C_ */


/*
 * For module 'mod' turn on or off the specific flags in
 * mask flags
 *
 * bits in mask are effected.
 * return previous 
 */
KLIB_EXTERN KDbgMask CC KDbgSetModConds ( KDbgMod mod, KDbgMask mask, KDbgMask flags );
KLIB_EXTERN bool CC KDbgTestModConds ( KDbgMod mod, KDbgMask flags );


/*
 * Init()
 * Initialize the debug messages module to a known state
 */
KLIB_EXTERN rc_t CC KDbgInit (void);

/* 
 * param is coming in as utf-8/ASCII with NUL terminator or we fail
 * we also assume no more than 127 significant characters
 *
 * These strings can be utf-8 or ASCII even if we are using 
 * clib strXXX functions.
 */
KLIB_EXTERN rc_t CC KDbgSetString (const char * string);

/* KDbgMask KDbgFlagsGet  (KDbgMod mod); */
/* bool     KDbgTestCond  (KDbgMask mask); */


/*
 * Get the KDbgMod associated with a name.
 */
KLIB_EXTERN rc_t CC KDbgGetModId (KDbgMod * mod, 
                   const char * mod_name, size_t mod_size);

/*
 * Get the KDbgCond associated with a name.
 */
KLIB_EXTERN rc_t CC KDbgGetCndFlag (KDbgMod mod, KDbgFlag * flag,
                      const char * cnd_name, size_t cnd_size);

KLIB_EXTERN KDbgMask CC KDbgGetModFlags ( KDbgMod mod );

KLIB_EXTERN rc_t CC KDbgMsg (const char * fmt, ...);

#define DBGMSG(mod,flags,msg) \
    (void)(((KDbgWriterGet() != NULL) && (KDbgTestModConds (mod, flags)))  \
           ? KDbgMsg msg : 0)

/* -----
 * Handlers for application and library writers.
 */
KLIB_EXTERN KWrtHandler* CC KDbgHandlerGet ( void );
KLIB_EXTERN KWrtWriter CC KDbgWriterGet ( void );
KLIB_EXTERN void* CC KDbgWriterDataGet ( void );

/* Handler
 *  sets output handler for standard output
 *
 *  "logger" [ IN ] and "self" [ IN, OPAQUE ] - callback function
 *  to handle log output
 */
KLIB_EXTERN rc_t CC KDbgHandlerSet ( KWrtWriter writer, void * data );

KLIB_EXTERN rc_t CC KDbgHandlerSetStdOut();
KLIB_EXTERN rc_t CC KDbgHandlerSetStdErr();


KLIB_EXTERN void CC KDbgSetRowId( uint64_t row_id );
KLIB_EXTERN uint64_t CC KDbgGetRowId( void );
KLIB_EXTERN void CC KDbgSetColName( const char * col_name );
KLIB_EXTERN const char * CC KDbgGetColName( void );


#ifdef __cplusplus
}
#endif

#else /* #if _DEBUGGING */

#define DBGMSG(mod,flags,msg) ((void)0)

#define KDbgInit() ((rc_t)0)

#define KDbgHandlerSetStdOut() ((rc_t)0)
#define KDbgHandlerSetStdErr() ((rc_t)0)

#define KDbgSetString(s) ((rc_t)0)

#define KDbgHandlerSet(a,b) ((rc_t)0)

#define KDbgHandlerGet() ((void*)NULL)
#define KDbgWriterGet() ((void*)NULL)
#define KDbgWriterDataGet() ((void*)NULL)

#endif /* #if _DEBUGGING */

#endif /*  _h_klib_debug_ */
