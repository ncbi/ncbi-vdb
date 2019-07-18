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

#ifndef _h_klib_writer_
#define _h_klib_writer_

#ifndef _h_klib_extern_
#include <klib/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifndef _h_klib_logfmt_
#include <klib/logfmt.h>
#endif

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif


/* ----------
 * The writer module (which contains the log, status, out, and debug sub-modules)
 * is an ecapsulation and virtualization of standard output.
 *
 *  this logging API - like most others - attempts to remain isolated
 *  from any actual logging implementation, such that output may be
 *  routed in several ways.
 *
 *  the default handler
 *
 *  the path from unstructured text to formatted XML is difficult in that
 *  it requires parsing, and is therefore error prone.
 *
 *  the path from structured reports to XML as well as less or unstructured
 *  text is simple to implement, but generally more of a burden to use.
 *
 *  the challenge is therefore to present an API that makes structured
 *  logging less painful than it might be.
 *
 *
 * There are two parts with an unfortunate legacy quirky combination.
 *  There is a formater that will be shared by all sub-modules.
 *  There is the actual stream writer.
 *
 * Each submodule will have one or more calls that call the formatter and writer functions.
 */
KLIB_EXTERN rc_t CC KWrtInit ( const char * appname, uint32_t vers );

/* -----
 * The writer module core file/stream output function:
 *
 * The function's prototype matches as much as possible the KFS file writer prototype.
 *
 * If the self structure contained a KFile * and a position it could use the KFS file write
 */

typedef rc_t ( CC * KWrtWriter ) ( void * self, const char * buffer, size_t bufsize, size_t * num_writ );

/* ----
 * a Writer handler combines the function and its self object
 */
typedef struct KWrtHandler
{
    KWrtWriter writer;
    void *     data;    /* the actual type depends upon the writer function */
} KWrtHandler;

/* ----
 * calls the platfrom-specific version of write() and isatty()
 * introduced to support the progressbar
 */
KLIB_EXTERN size_t CC simple_write( int fd, const void * buf, size_t count );
KLIB_EXTERN int CC is_a_tty( int fd );

/*--------------------------------------------------------------------------
 * nvp - name/value pair
 */
typedef struct wrt_nvp_t_struct {
    const char *name;
    const char *value;
} wrt_nvp_t;

KLIB_EXTERN void CC wrt_nvp_sort(size_t argc, wrt_nvp_t argv[]);
KLIB_EXTERN const wrt_nvp_t* CC wrt_nvp_find( size_t argc, const wrt_nvp_t argv[], const char* key );
KLIB_EXTERN const char* CC wrt_nvp_find_value( size_t argc, const wrt_nvp_t argv[], const char* key );

/* -----
 * The formatter module core file/stream output function:
 *
 */

typedef rc_t ( CC * KFmtWriter ) ( void* self, KWrtHandler* writer,
                                   size_t argc, const wrt_nvp_t args[],
                                   size_t envc, const wrt_nvp_t envs[]);
/* ----
 * a formatter handler combines the function and custom data
 */
typedef struct KFmtHandler KFmtHandler;
struct KFmtHandler
{
    KFmtWriter formatter;
    void* data;    /* the actual type depends upon the writer function */
};


/* kprintf
 *  performs a printf to our output writer
 */
KLIB_EXTERN rc_t CC kprintf ( size_t *num_writ, const char * fmt, ... );
KLIB_EXTERN rc_t CC vkprintf ( size_t *num_writ, const char * fmt, va_list args );


/* kfprintf
 *  performs a printf to a KWrtHandler
 */
KLIB_EXTERN rc_t CC kfprintf ( const KWrtHandler *out,
    size_t *num_writ, const char * fmt, ... );
KLIB_EXTERN rc_t CC vkfprintf ( const KWrtHandler *out,
    size_t *num_writ, const char * fmt, va_list args );


/* RCExplain
 *  explains rc code in "English'ish" language
 */
KLIB_EXTERN rc_t CC RCExplain ( rc_t rc, char *buffer, size_t bsize, size_t *num_writ );

KLIB_EXTERN rc_t CC LogInsertSpace(const char *spacer, char *buffer, size_t bsize, size_t *num_writ);

/* generates a timestamp-string in GMT-time */
KLIB_EXTERN rc_t CC LogTimestamp ( char *buffer, size_t bsize, size_t *num_writ );

/* generates a timestamp-string in local time */
KLIB_EXTERN rc_t CC LogSimpleTimestamp ( char *buffer, size_t bsize, size_t *num_writ );

KLIB_EXTERN rc_t CC LogPID ( char *buffer, size_t bsize, size_t *num_writ );
KLIB_EXTERN rc_t CC LogAppName ( char *buffer, size_t bsize, size_t *num_writ );
KLIB_EXTERN rc_t CC LogAppVersion ( char *buffer, size_t bsize, size_t *num_writ );

/* FlushLine
 *  flushes a buffer through output handler
 */
KLIB_EXTERN rc_t CC LogFlush ( const KWrtHandler * handler, const char *buffer, const size_t bsize );

#ifdef __cplusplus
}
#endif

#endif /* _h_klib_writer_ */
