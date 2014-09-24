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

#ifndef _h_writer_priv_
#define _h_writer_priv_

#include <klib/writer.h>

#ifndef _h_klib_rc_
#include <klib/rc.h>
#endif

#ifndef _h_klib_log_
#include <klib/log.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

rc_t KWrtSysInit(void** h_stdout, void** h_stderr);

size_t KWrtFmt_rc_t  (char * buffer, size_t bufsize, const char * fmt, rc_t rc_in);
size_t KWrtFmt_KTime_t (char * buffer, size_t bufsize, const char * fmt_ignored, KTime_t time_in);


/* -----
 * write into the buffer a textual string describing the system specific
 * error code represented by the third parameter.  Will be NUL terminated.
 * expected to be used within string_printf and associated functions only.
 */
size_t KWrtFmt_error_code (char * buffer, size_t bufsize, uint32_t error_code);

/* -----
 * We have a default writer is the part that will possibly vary by O/S type
 */
rc_t CC KWrt_DefaultWriter (void* data, const char * buffer, size_t bufsize, size_t * num_writ);

/* -----
 * The default writer uses this data type object.
 * The actual definition might vary to work on non-Unix operating systems.  The port to Windows
 * might well force the full definition to a system file.
 */
typedef struct KWrt_DefaultWriterData DefaultWriterData;

/*
 * Windows or other O/S port might have to move this struct definition to a system specific file
 */
struct KWrt_DefaultWriterData
{
    int sysfile_handle;
};

/*
 * We will have two common default writer data objects for the C library expected stdout and stderr
 *
 * These are the normal data stream and error data stream that will have to be set up for
 * windows console I/O
 */
extern void *KWrt_DefaultWriterDataStdOut;
extern void *KWrt_DefaultWriterDataStdErr;

/* fix up windows printf shortcomings for integer types 
 * current format is len long at fmt with max max size for the format
 * len is updated to new length if fmt is changed
 */
void print_int_fixup   (char * fmt, size_t * len, size_t max);
void print_float_fixup (char * fmt, size_t * len, size_t max);
void print_char_fixup  (char * fmt, size_t * len, size_t max);

extern const char *gRCModule_str [];
extern const char *gRCTarget_str [];
extern const char *gRCContext_str [];
extern const char *gRCObject_str [];
extern const char *gRCState_str [];

extern const char *gRCModuleIdx_str [];
extern const char *gRCTargetIdx_str [];
extern const char *gRCContextIdx_str [];
extern const char *gRCObjectIdx_str [];
extern const char *gRCStateIdx_str [];

/* GetRCModuleText
 */
const char *GetRCModuleText ( enum RCModule mod );

/* GetRCTargetText
 */
const char *GetRCTargetText ( enum RCTarget targ );

/* GetRCContextText
 */
const char *GetRCContextText ( enum RCContext ctx );

/* GetRCObjectText
 *  obj can be RCTarget or RCObject
 */
const char *GetRCObjectText ( int obj );

/* GetRCStateText
 */
const char *GetRCStateText ( enum RCState state );


/* GetRCModuleIdxText
 */
const char *GetRCModuleIdxText ( enum RCModule mod );

/* GetRCTargetIdxText
 */
const char *GetRCTargetIdxText ( enum RCTarget targ );

/* GetRCContextIdxText
 */
const char *GetRCContextIdxText ( enum RCContext ctx );

/* GetRCObjectIdxText
 *  obj can be RCTarget or RCObject
 */
const char *GetRCObjectIdxText ( int obj );

/* GetRCStateIdxText
 */
const char *GetRCStateIdxText ( enum RCState state );


extern const char * RCModuleStrings[];
extern const char * RCTargetStrings[];
extern const char * RCContextStrings[];
extern const char * RCObjectStrings[];
extern const char * RCStateStrings[];

enum ERCExplain2Options
{
    eRCExOpt_NoMessageIfNoError,
    eRCExOpt_ObjAndStateOnlyIfError,
    eRCExOpt_CompleteMsg
};

KLIB_EXTERN rc_t CC RCExplain2 ( rc_t rc, char *buffer, size_t bsize, size_t *num_writ, enum ERCExplain2Options options );

#ifdef __cplusplus
}
#endif

#endif /* _h_writer_priv_ */
