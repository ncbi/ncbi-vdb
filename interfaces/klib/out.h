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

#ifndef _h_klib_out_
#define _h_klib_out_

#ifndef _h_klib_extern_
#include <klib/extern.h>
#endif

#ifndef _h_klib_writer_
#include <klib/writer.h>
#endif

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif


/* -----
 * Handlers for application and library writers.
 */

KLIB_EXTERN KWrtHandler* CC KOutHandlerGet (void);

/* Handler
 *  sets output handler for standard output
 *
 *  "logger" [ IN ] and "self" [ IN, OPAQUE ] - callback function
 *  to handle log output
 */
KLIB_EXTERN rc_t CC KOutHandlerSet          (KWrtWriter writer, void * data);
KLIB_EXTERN rc_t CC KOutHandlerSetStdOut    (void);
KLIB_EXTERN rc_t CC KOutHandlerSetStdErr    (void);

KLIB_EXTERN KWrtWriter CC KOutWriterGet (void);
KLIB_EXTERN void * CC KOutDataGet (void);

KLIB_EXTERN rc_t CC KOutInit (void);

KLIB_EXTERN rc_t CC KOutMsg (const char * fmt, ...);

#define KOutStr(str) KOutMsg("%s",str)

/*
 * A usage could look like
 *
 * OUTMSG (kout_2, "Current Out Level is %d\n", OutLevelGet());
 *
 */
#define OUTMSG(msg) \
    ((KOutWriterGet() != NULL) ? KOutMsg msg : 0)

#define OUTSTR(msg) \
    ((KOutWriterGet() != NULL) ? KOutStr (msg) : 0)

#ifdef __cplusplus
}
#endif

#endif /*  _h_klib_out_ */
