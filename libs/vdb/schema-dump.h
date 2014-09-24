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

#ifndef _h_schema_dump_
#define _h_schema_dump_

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct String;
struct KSymbol;
struct VSchema;
struct VTypedecl;
struct VFormatdecl;


/*--------------------------------------------------------------------------
 * SDumper
 */
#define MARKED_MODE 0x8000

enum VSchemaDumpClass
{
    sdcTypes = 1,
    sdcTypesets,
    sdcFormats,
    sdcConstants,
    sdcFunctions,
    sdcColumns,
    sdcTables,
    sdcDatabases
};

typedef struct SDumper SDumper;
struct SDumper
{
    struct VSchema const *schema;

    struct KSymbol const *alias;

    rc_t ( CC * flush ) ( void *fd, const void *buffer, size_t size );
    void *fd;

    const char *sep;

    rc_t rc;
    uint16_t indent;
    uint16_t mode;

    size_t total;
    char buffer [ 4096 ];
};

/* Init
 *  sets up block
 */
void SDumperInit ( SDumper *self, struct VSchema const *schema, uint32_t mode,
    rc_t ( CC * flush ) ( void *fd, const void *buffer, size_t size ), void *fd );

/* Whack
 *  flushes buffer if necessary
 */
rc_t SDumperWhack ( SDumper *self );

/* Write
 *  write some text to the dumper
 */
rc_t SDumperWrite ( SDumper *self,
    const char *buffer, size_t size );

/* IndentLevel
 *  increase or decrease indentation level
 */
void SDumperIncIndentLevel ( SDumper *self );
void SDumperDecIndentLevel ( SDumper *self );

/* Indent
 *  writes indentation spacing
 */
rc_t SDumperIndent ( SDumper *self );

/* SepString
 *  sets separator string
 */
void SDumperSepString ( SDumper *self, const char *sep );

/* Sep
 *  write separator string
 */
rc_t SDumperSep ( SDumper *self );

/* Print
 *  \t   - indent
 *  \n   - end of line
 *  \v   - separator
 *  %d   - int
 *  %u   - unsigned int
 *  %x   - unsigned int ( lower-case )
 *  %X   - unsigned int ( UPPER-case )
 *  %f   - double
 *  %ld  - int64_t
 *  %lu  - uint64_t
 *  %lx  - uint64_t ( lower-case )
 *  %lX  - uint64_t ( UPPER-case )
 *  %p   - pointer ( UPPER-case )
 *  %z   - size_t ( decimal )
 *  %s   - const char* ( NUL-terminated )
 *  %.*s - int, const char*
 *  %S   - const String*
 *  %N   - const KSymbol*
 *  %T   - const VTypedecl*
 *  %F   - const VFormatdecl*
 *  %V   - uint32_t version
 *  %E   - const SExpression*
 */
rc_t SDumperPrint ( SDumper *self, const char *fmt, ... );
rc_t SDumperVPrint ( SDumper *self, const char *fmt, va_list args );


/* Mode
 *  get the dump mode
 */
#define SDumperMode( self ) \
    ( ( self ) -> mode & 0xFF )

/* Class
 *  get the class of object to dump
 */
#define SDumperClass( self ) \
    ( ( ( self ) -> mode >> 8 ) & 0x7F )

/* MarkedMode
 *  tells if only running in marked mode
 */
#define SDumperMarkedMode( self ) \
    ( ( self ) -> mode & MARKED_MODE )


/*--------------------------------------------------------------------------
 * various types
 */

/* Version
 */
rc_t SDumperVersion ( SDumper *self, uint32_t version );

/* String
 */
rc_t StringDump ( struct String const *self, SDumper *d );

/* KSymbol
 */
rc_t KSymbolDump ( struct KSymbol const *self, SDumper *b );

/* FQN
 *  writes fully qualified name in schema format
 */
rc_t FQNDump ( struct KSymbol const *self, SDumper *b );

/* Alias
 */
rc_t AliasDump ( struct KSymbol const *self, SDumper *b );

/* VTypedecl
 * VFormatdecl
 */
rc_t VTypedeclDump ( struct VTypedecl const *self, SDumper *b );
rc_t VFormatdeclDump ( struct VFormatdecl const *self, SDumper *b );

#ifdef __cplusplus
}
#endif

#endif /* _h_schema_dump_ */
