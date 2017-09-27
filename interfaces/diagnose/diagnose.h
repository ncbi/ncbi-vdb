#ifndef _h_diagnose_diagnose_
#define _h_diagnose_diagnose_

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

#ifndef _h_diagnose_extern_
#include <diagnose/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h> /* rc_t */
#endif

#include <stdarg.h> /* va_list */


#ifdef __cplusplus
extern "C" {
#endif


struct KConfig;
struct KNSManager;
struct VFSManager;


typedef struct KDiagnose KDiagnose;
typedef struct KDiagnoseError KDiagnoseError;
typedef struct KDiagnoseTest KDiagnoseTest;
typedef struct KDiagnoseTestDesc KDiagnoseTestDesc;


DIAGNOSE_EXTERN rc_t CC KDiagnoseMakeExt ( KDiagnose ** test,
    struct KConfig * kfg, struct KNSManager * kmg, struct VFSManager * vmgr );
DIAGNOSE_EXTERN rc_t CC KDiagnoseAddRef ( const KDiagnose * self );
DIAGNOSE_EXTERN rc_t CC KDiagnoseRelease ( const KDiagnose * self );


#define KVERBOSITY_NONE  -3
#define KVERBOSITY_ERROR -2
#define KVERBOSITY_INFO  -1
#define KVERBOSITY_MAX    0
DIAGNOSE_EXTERN rc_t CC KDiagnoseSetVerbosity ( KDiagnose * self,
    int verbosity );

DIAGNOSE_EXTERN rc_t CC KDiagnoseLogHandlerSet ( KDiagnose * self,
        rc_t ( CC * logger ) ( int verbosity,
                            unsigned type, /* TBD */
                            const char * fmt, va_list args )
    );

DIAGNOSE_EXTERN rc_t CC KDiagnoseLogHandlerSetKOutMsg ( KDiagnose * self );


/*
 * TestHandlerSet: set a test callback
 * the callback is called when each test is started or finished
 *
 * state - state of the test
 * test - test being executed.
 *      test remains valid when KDiagnose is valid;
 *      KDiagnoseTestAddRef if you want to keep it after KDiagnose is released
 */
typedef enum {
    eKDTS_NotStarted,/* test not started yet */
    eKDTS_Started,   /* test started */
    eKDTS_Succeed,   /* test finished successfully */
    eKDTS_Failed,    /* test finished with failure */
    eKDTS_Skipped,   /* test execution was skipped */
    eKDTS_Warning,   /* test finished successfully but has a warning for user */
    eKDTS_Paused,    /* KDiagnosePause was called */
    eKDTS_Resumed,   /* KDiagnoseResume was called */
    eKDTS_Canceled,  /* KDiagnoseCancel was called */
} EKDiagTestState;
/* test is NULL when state is one of:
 *                                    eKDTS_Paused
 *                                    eKDTS_Resumed
 *                                    eKDTS_Canceled
 */
DIAGNOSE_EXTERN rc_t CC KDiagnoseTestHandlerSet ( KDiagnose * self,
        void ( CC * callback )
             ( EKDiagTestState state, const KDiagnoseTest * test )
    );


#define DIAGNOSE_FAIL            1

#define DIAGNOSE_CONFIG_COMMON   2
#define DIAGNOSE_CONFIG_DB_GAP   4
#define DIAGNOSE_CONFIG ( DIAGNOSE_CONFIG_COMMON | DIAGNOSE_CONFIG_DB_GAP )

#define DIAGNOSE_NETWORK_NCBI    8
#define DIAGNOSE_NETWORK_HTTPS  16
#define DIAGNOSE_NETWORK_ASPERA 32
#define DIAGNOSE_NETWORK_DB_GAP 64
#define DIAGNOSE_NETWORK \
    ( DIAGNOSE_NETWORK_NCBI | DIAGNOSE_NETWORK_HTTPS | DIAGNOSE_NETWORK_ASPERA \
                            | DIAGNOSE_NETWORK_DB_GAP )

#define DIAGNOSE_ALL ( DIAGNOSE_CONFIG | DIAGNOSE_NETWORK )

/* Run
 *
 * tests is combination of DIAGNOSE_* values
 *
 * projectId, ... is the sequence of dbGaP projectId-s to check
 *  should be terminated by projectId = 0
 *
 * When "tests | DIAGNOSE_FAIL != 0" - KDiagnoseRun will return non-0 rc
 * When KDiagnoseCancel is called - KDiagnoseRun will return rcCanceled
 *
 *
 * If no 'projectId' is provided but 'tests' contain DIAGNOSE_CONFIG_DB_GAP
 * and/or DIAGNOSE_NETWORK_HTTPS/DIAGNOSE_NETWORK_ASPERA
 * - we will try to check existing dbGaP configuration and access to dbGaP
 * servers and issue WARNINGS if something is not correct.
 *
 * If a 'projectId' is provided - configuration WILL BE checked for VALIDIRY of 
 * projectId's project and access to gbGaP servers.
 * KDiagnoseRun WILL FAIL if configuration is not complete or no gbGaP server
 * can be accessed.
 */
DIAGNOSE_EXTERN rc_t CC KDiagnoseRun ( KDiagnose * self, uint64_t tests,
    uint32_t projectId, ... );

DIAGNOSE_EXTERN rc_t CC KDiagnosePause  ( KDiagnose * self );
DIAGNOSE_EXTERN rc_t CC KDiagnoseResume ( KDiagnose * self );
DIAGNOSE_EXTERN rc_t CC KDiagnoseCancel ( KDiagnose * self );


DIAGNOSE_EXTERN rc_t CC KDiagnoseGetErrorCount ( const KDiagnose * self,
    uint32_t * count );
DIAGNOSE_EXTERN rc_t CC KDiagnoseGetError ( const KDiagnose * self,
    uint32_t idx, const KDiagnoseError ** error );


DIAGNOSE_EXTERN rc_t CC KDiagnoseErrorAddRef ( const KDiagnoseError * self );
DIAGNOSE_EXTERN rc_t CC KDiagnoseErrorRelease ( const KDiagnoseError * self );

/* GetMsg:
 * Get Error Message.
 * Returned string remains valid while "self" is valid
 */
DIAGNOSE_EXTERN rc_t CC KDiagnoseErrorGetMsg ( const KDiagnoseError * self,
                                               const char ** message );


/* GetDesc:
 * Get description of available tests
 * Returned object remains valid while "self" is valid
 */
DIAGNOSE_EXTERN rc_t CC KDiagnoseGetDesc ( const KDiagnose * self,
                                           const KDiagnoseTestDesc ** desc );

/* KDiagnoseTestDesc...:
 * Get description KDiagnoseTestDesc
 * Returned object remains valid while "self" is valid
 */

/* Name:
 * test name
 */
DIAGNOSE_EXTERN rc_t CC KDiagnoseTestDescName ( const KDiagnoseTestDesc * self,
                                                const char ** name );

/* Desc:
 * test description
 */
DIAGNOSE_EXTERN rc_t CC KDiagnoseTestDescDesc ( const KDiagnoseTestDesc * self,
                                                const char ** desc );

/* Desc:
 * test code to be used to form 'tests' argument of KDiagnoseRun()
 */
DIAGNOSE_EXTERN rc_t CC KDiagnoseTestDescCode ( const KDiagnoseTestDesc * self,
                                                uint64_t * code );

/* Level:
 * test level in tests hierarchy.
 * 0 is the highest level
 * tests of 'level 1' are subtests of 'level 0' etc
 */
DIAGNOSE_EXTERN rc_t CC KDiagnoseTestDescLevel ( const KDiagnoseTestDesc * self,
                                                 uint32_t * level );

/* Next:
 * next test of the same level
 */
DIAGNOSE_EXTERN rc_t CC KDiagnoseTestDescNext ( const KDiagnoseTestDesc * self,
                                             const KDiagnoseTestDesc ** next );

/* Child:
 * the first child test of next level
 */
DIAGNOSE_EXTERN rc_t CC KDiagnoseTestDescChild ( const KDiagnoseTestDesc * self,
                                             const KDiagnoseTestDesc ** child );

/* Depends:
 * the test 'self' cannot be executed if test 'depends' fails
 */
DIAGNOSE_EXTERN rc_t CC KDiagnoseTestDescDepends (
    const KDiagnoseTestDesc * self, const KDiagnoseTestDesc * depends );


/* GetTests:
 * Get executed tests
 */
DIAGNOSE_EXTERN rc_t CC KDiagnoseGetTests ( const KDiagnose * self,
                                            const KDiagnoseTest ** test );


/* Level
 *
 * Get test level in tests hierarchy.
 * 0 is the highest level
 * tests of 'level 1' are run from test of 'level 0' etc
 */
DIAGNOSE_EXTERN rc_t CC KDiagnoseTestLevel ( const KDiagnoseTest * self,
                                             uint32_t * level );

/* Number
 *
 * Get hiererchical number of test inside of est level in tests hierarchy.
 * E.g., 0.2.2
 */
DIAGNOSE_EXTERN rc_t CC KDiagnoseTestNumber ( const KDiagnoseTest * self,
                                              const char ** number );

/* Name
 *
 * Get test name.
 */
DIAGNOSE_EXTERN rc_t CC KDiagnoseTestName ( const KDiagnoseTest * self,
                                            const char ** name );

/* Message
 *
 * Get test message (is set when test is finished)
 */
DIAGNOSE_EXTERN rc_t CC KDiagnoseTestMessage ( const KDiagnoseTest * self,
                                               const char ** message );

/* Message
 *
 * Get test state ( changes during test execution )
 */
DIAGNOSE_EXTERN rc_t CC KDiagnoseTestState ( const KDiagnoseTest * self,
                                             EKDiagTestState * state );

DIAGNOSE_EXTERN rc_t CC KDiagnoseTestNext ( const KDiagnoseTest * self,
                                            const KDiagnoseTest ** test );

DIAGNOSE_EXTERN rc_t CC KDiagnoseTestChild ( const KDiagnoseTest * self, 
                               uint32_t idx, const KDiagnoseTest ** test );


#ifdef __cplusplus
}
#endif

/******************************************************************************/

#endif /* _h_diagnose_diagnose_ */
