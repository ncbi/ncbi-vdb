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

#ifndef _h_klib_status_
#define _h_klib_status_

#ifndef _h_klib_extern_
#include <klib/extern.h>
#endif

#ifndef _h_klib_writer_
#include <klib/writer.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t KStsLevel;

KLIB_EXTERN KStsLevel CC KStsLevelGet( void );
KLIB_EXTERN void CC KStsLevelSet( KStsLevel level );
KLIB_EXTERN void CC KStsLevelAdjust( int32_t adjust );

/* -----
 * Handlers for application and library writers.
 */
KLIB_EXTERN KWrtHandler* CC KStsHandlerGet (void);
KLIB_EXTERN KWrtHandler* CC KStsLibHandlerGet (void);

KLIB_EXTERN KWrtWriter CC KStsWriterGet (void);
KLIB_EXTERN KWrtWriter CC KStsLibWriterGet (void);

KLIB_EXTERN void* CC KStsDataGet (void);
KLIB_EXTERN void* CC KStsLibDataGet (void);

/*
 * Init()
 * Initialize the debug messages module to a known state
 */
KLIB_EXTERN rc_t CC KStsInit (void);

KLIB_EXTERN rc_t CC KStsMsg (const char * fmt, ...);
KLIB_EXTERN rc_t CC KStsLibMsg (const char * fmt, ...);

/* Set
 */
KLIB_EXTERN rc_t CC KStsHandlerSet    (KWrtWriter writer, void * data);
KLIB_EXTERN rc_t CC KStsLibHandlerSet (KWrtWriter writer, void * data);

KLIB_EXTERN rc_t CC KStsHandlerSetStdOut    ( void );
KLIB_EXTERN rc_t CC KStsLibHandlerSetStdOut ( void );
KLIB_EXTERN rc_t CC KStsHandlerSetStdErr    ( void );
KLIB_EXTERN rc_t CC KStsLibHandlerSetStdErr ( void );

/* formatting */
typedef uint32_t KStsFmtFlags;
enum KStsFmtFlagsEnum
{
    kstsFmtTimestamp = 0x00000001,
    kstsFmtPid = 0x00000002,
    kstsFmtAppName = 0x00000004,
    kstsFmtAppVersion = 0x00000008,
    kstsFmtMessage = 0x00000010, /* actual message */
    kstsFmtLocalTimestamp = 0x00000020 /* print time stamp in local time, if both bits set local takes over */
};

KLIB_EXTERN KFmtHandler* CC KStsFmtHandlerGet (void);
KLIB_EXTERN KFmtHandler* CC KStsLibFmtHandlerGet (void);

KLIB_EXTERN KStsFmtFlags CC KStsFmtFlagsGet ( void );
KLIB_EXTERN KStsFmtFlags CC KStsLibFmtFlagsGet ( void );

KLIB_EXTERN KFmtWriter CC KStsFmtWriterGet (void);
KLIB_EXTERN KFmtWriter CC KStsLibFmtWriterGet (void);
KLIB_EXTERN void* CC KStsFmtDataGet (void);
KLIB_EXTERN void* CC KStsLibFmtDataGet (void);

KLIB_EXTERN rc_t CC KStsFmtFlagsSet    (KStsFmtFlags flags);
KLIB_EXTERN rc_t CC KStsLibFmtFlagsSet (KStsFmtFlags flags);

KLIB_EXTERN rc_t CC KStsFmtHandlerSet    (KFmtWriter formatter, KStsFmtFlags flags, void * data);
KLIB_EXTERN rc_t CC KStsLibFmtHandlerSet (KFmtWriter formatter, KStsFmtFlags flags, void * data);

KLIB_EXTERN rc_t CC KStsFmtHandlerSetDefault(void);
KLIB_EXTERN rc_t CC KStsLibFmtHandlerSetDefault(void);

/*
 * A usage could look like
 *
 *  STSMSG(4, ("Current Status Level is %d\n", KStsLevelGet()));
 *
 */
#ifdef _LIBRARY

#define STSMSG(lvl,msg) \
    (void)((((unsigned)lvl) <= KStsLevelGet()) ? KStsLibMsg msg : 0)

#else

#define STSMSG(lvl,msg) \
    (void)((((unsigned)lvl) <= KStsLevelGet()) ? KStsMsg msg : 0)

#endif

#ifdef __cplusplus
}
#endif

#endif /*  _h_klib_status_ */
