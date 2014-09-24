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

#include <vdb/extern.h>

#include "schema-dump.h"
#include "schema-priv.h"
#include "schema-parse.h"
#include "schema-expr.h"

#include <klib/text.h>
#include <klib/printf.h>
#include <klib/symbol.h>
#include <klib/symtab.h>
#include <klib/log.h>
#include <klib/rc.h>
#include <os-native.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>


/*--------------------------------------------------------------------------
 * SDumper
 */


/* Init
 *  sets up block
 */
void SDumperInit ( SDumper *self, const VSchema *schema, uint32_t mode,
    rc_t ( CC * flush ) ( void *fd, const void *buffer, size_t size ), void *fd )
{
    self -> schema = schema;
    self -> flush = flush;
    self -> fd = fd;
    self -> sep = "";
    self -> rc = 0;
    self -> indent = 0;
    self -> mode = ( uint16_t ) mode;
    self -> total = 0;
}


/* Flush
 */
static
rc_t SDumperFlush ( SDumper *self )
{
    rc_t rc = ( * self -> flush ) ( self -> fd, self -> buffer, self -> total );
    if ( rc == 0 )
        self -> total = 0;
    return rc;
}

static
rc_t SDumperFlushLine ( SDumper *self )
{
#if ! _DEBUGGING
    if ( self -> total < sizeof self -> buffer / 2 )
        return 0;
#endif
    return SDumperFlush ( self );
}


/* Whack
 *  flushes buffer if necessary
 */
rc_t SDumperWhack ( SDumper *self )
{
    if ( self -> rc == 0 && self -> total != 0 )
        return SDumperFlush ( self );
    return 0;
}


/* Write
 *  writes data to buffer, flushes as necessary
 */
rc_t SDumperWrite ( SDumper *self, const char *buffer, size_t size )
{
    rc_t rc;
    size_t total, num_writ;

    for ( rc = 0, total = 0; total < size; total += num_writ )
    {
        if ( self -> total == sizeof self -> buffer )
        {
            rc = SDumperFlush ( self );
            if ( rc != 0 )
                break;
        }

        num_writ = size - total;
        if ( num_writ > sizeof self -> buffer - self -> total )
            num_writ = sizeof self -> buffer - self -> total;

        memcpy ( & self -> buffer [ self -> total ], & buffer [ total ], num_writ );
        self -> total += num_writ;
    }

    return rc;
}

/* IndentLevel
 *  increase or decrease indentation level
 */
void SDumperIncIndentLevel ( SDumper *self )
{
    ++ self -> indent;
}

void SDumperDecIndentLevel ( SDumper *self )
{
    if ( self -> indent > 0 )
        -- self -> indent;
}


/* Indent
 *  writes indentation spacing
 */
rc_t SDumperIndent ( SDumper *self )
{
    rc_t rc;
    uint32_t total, num_writ;

    /* use a tab */
    const char *tabs = "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";

    for ( rc = 0, total = 0; total < self -> indent; total += num_writ )
    {
        num_writ = ( ( self -> indent - total - 1 ) & 0xF ) + 1;
        rc = SDumperWrite ( self, tabs, num_writ );
        if ( rc != 0 )
            break;
    }

    return rc;
}

/* SepString
 *  sets separator string
 */
void SDumperSepString ( SDumper *self, const char *sep )
{
    self -> sep = sep;
}

/* Sep
 *  write separator string
 */
rc_t SDumperSep ( SDumper *self )
{
    if ( self -> sep == NULL )
        return 0;

    return SDumperWrite ( self, self -> sep, strlen ( self -> sep ) );
}

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
rc_t SDumperVPrint ( SDumper *self, const char *fmt, va_list args )
{
    rc_t rc;
    const char *start, *end;
    for ( rc = 0, start = end = fmt; * end != 0; ++ end )
    {
        int len;
        size_t size;
        char buffer [ 256 ];

        switch ( * end )
        {
        case '\t':
            if ( end > start )
                rc = SDumperWrite ( self, start, end - start );
            if ( rc == 0 )
                rc = SDumperIndent ( self );
            start = end + 1;
            break;
        case '\n':
            rc = SDumperWrite ( self, start, end - start + 1 );
            if ( rc == 0 )
                rc = SDumperFlushLine ( self );
            start = end + 1;
            break;
        case '\v':
            if ( end > start )
                rc = SDumperWrite ( self, start, end - start );
            if ( rc == 0 )
                rc = SDumperSep ( self );
            start = end + 1;
            break;
        case '%':
            if ( end > start )
            {
                rc = SDumperWrite ( self, start, end - start );
                if ( rc != 0 )
                    break;
            }
            switch ( * ( ++ end ) )
            {
            case 'd':
                len = sprintf ( buffer, "%d", va_arg ( args, int ) );
                rc = SDumperWrite ( self, buffer, len );
                break;
            case 'u':
                len = sprintf ( buffer, "%u", va_arg ( args, unsigned int ) );
                rc = SDumperWrite ( self, buffer, len );
                break;
            case 'x':
                len = sprintf ( buffer, "%x", va_arg ( args, unsigned int ) );
                rc = SDumperWrite ( self, buffer, len );
                break;
            case 'X':
                len = sprintf ( buffer, "%X", va_arg ( args, unsigned int ) );
                rc = SDumperWrite ( self, buffer, len );
                break;
            case 'f':
                len = sprintf ( buffer, "%f", va_arg ( args, double ) );
                rc = SDumperWrite ( self, buffer, len );
                break;
            case 'l':
                switch ( * ( ++ end ) )
                {
                case 'd':
                    rc = string_printf ( buffer, sizeof buffer, & size, "%ld", va_arg ( args, int64_t ) );
                    if ( rc == 0 )
                        rc = SDumperWrite ( self, buffer, size );
                    break;
                case 'u':
                    rc = string_printf ( buffer, sizeof buffer, & size, "%lu", va_arg ( args, uint64_t ) );
                    if ( rc == 0 )
                        rc = SDumperWrite ( self, buffer, size );
                    break;
                case 'x':
                    rc = string_printf ( buffer, sizeof buffer, & size, "%lx", va_arg ( args, uint64_t ) );
                    if ( rc == 0 )
                        rc = SDumperWrite ( self, buffer, size );
                    break;
                case 'X':
                    rc = string_printf ( buffer, sizeof buffer, & size, "%lX", va_arg ( args, uint64_t ) );
                    if ( rc == 0 )
                        rc = SDumperWrite ( self, buffer, size );
                    break;
                }
                break;
            case 'p':
                rc = string_printf ( buffer, sizeof buffer, & size, "0x%p", va_arg ( args, size_t ) );
                if ( rc == 0 )
                    rc = SDumperWrite ( self, buffer, size );
                break;
            case 'z':
                rc = string_printf ( buffer, sizeof buffer, & size, "%zu", va_arg ( args, size_t ) );
                if ( rc == 0 )
                    rc = SDumperWrite ( self, buffer, size );
                break;
            case 's':
                len = snprintf ( buffer, sizeof buffer, "%s", va_arg ( args, const char* ) );
                if ( len < 0 || len >= sizeof buffer )
                    rc = RC ( rcVDB, rcSchema, rcWriting, rcString, rcExcessive );
                else
                    rc = SDumperWrite ( self, buffer, len );
                break;
            case '.':
                if ( end [ 1 ] == '*' && end [ 2 ] == 's' )
                {
                    end += 2;
                    len = va_arg ( args, int );
                    if ( len >= sizeof buffer )
                        rc = RC ( rcVDB, rcSchema, rcWriting, rcString, rcExcessive );
                    else
                    {
                        len = sprintf ( buffer, "%.*s", len, va_arg ( args, const char* ) );
                        rc = SDumperWrite ( self, buffer, len );
                    }
                    break;
                }
                /* not handling anything else */
                rc = SDumperWrite ( self, "%.", 2 );
                break;
            case 'S':
                rc = StringDump ( va_arg ( args, const String* ), self );
                break;
            case 'N':
                rc = FQNDump ( va_arg ( args, const KSymbol* ), self );
                break;
#if SLVL >= 1
            case 'T':
                rc = VTypedeclDump ( va_arg ( args, struct VTypedecl const* ), self );
                break;
            case 'F':
                rc = VFormatdeclDump ( va_arg ( args, struct VFormatdecl const* ), self );
                break;
#endif
#if SLVL >= 2
            case 'E':
                rc = SExpressionDump ( va_arg ( args, const SExpression* ) , self );
                break;
#endif
            case 'V':
                rc = SDumperVersion ( self, va_arg ( args, uint32_t ) );
                break;
            case '%':
                rc = SDumperWrite ( self, "%", 1 );
                break;
            }
            start = end + 1;
            break;
        }

        if ( rc != 0 )
            break;
    }

    if ( rc == 0 && end > start )
    {
        rc = SDumperWrite ( self, start, end - start );
        if ( rc == 0 )
            rc = SDumperFlushLine ( self );
    }

    return rc;
}

rc_t SDumperPrint ( SDumper *self, const char *fmt, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, fmt );
    rc = SDumperVPrint ( self, fmt, args );
    va_end ( args );

    return rc;
}


/*--------------------------------------------------------------------------
 * various types
 */

/* Version
 */
rc_t SDumperVersion ( SDumper *self, uint32_t version )
{
    const char *fmt;

    if ( ( version & 0xFFFF ) != 0 )
        fmt =" #%u.%u.%u";
    else if ( ( version & 0xFF0000 ) != 0 )
        fmt =" #%u.%u";
    else
        fmt =" #%u";

    switch ( SDumperMode ( self ) )
    {
    case sdmCompact:
        ++ fmt;
        break;
    }

    return SDumperPrint ( self, fmt,
                          ( version >> 24 ),
                          ( version >> 16 ) & 0xFF,
                          ( version & 0xFFFF ) );
}

/* String
 */
rc_t StringDump ( const String *self, SDumper *b )
{
    return SDumperWrite ( b, self -> addr, self -> size );
}

/* KSymbol
 */
rc_t KSymbolDump ( const KSymbol *self, SDumper *b )
{
    if ( self == NULL )
        return SDumperWrite ( b, "NULL", 4 );

    return StringDump ( & self -> name, b );
}

/* FQN
 *  writes fully qualified name in schema format
 */
rc_t FQNDump ( const KSymbol *self, SDumper *b )
{
    rc_t rc = 0;

    if ( self != NULL && self -> dad != NULL )
    {
        rc = FQNDump ( self -> dad, b );
        if ( rc == 0 )
            rc = SDumperWrite ( b, ":", 1 );
    }

    if ( rc == 0 )
        rc = KSymbolDump ( self, b );

    return rc;
}

/* Alias
 */
LIB_EXPORT bool CC KSymbolDumpAlias ( void *item, void *data )
{
    SDumper *b = data;
    const KSymbol *self = ( const KSymbol* ) item;
    if ( self -> u . obj == b -> alias -> u . obj )
    {
        if ( SDumperMode ( b ) == sdmCompact )
            b -> rc = SDumperPrint ( b, "alias %N %N;", b -> alias, self );
        else
            b -> rc = SDumperPrint ( b, "alias %N %N;\n", b -> alias, self );
    }

    return false;
}

rc_t AliasDump ( const KSymbol *self, SDumper *b )
{
    const VSchema *schema = b -> schema;
    for ( b -> rc = 0, b -> alias = self; schema != NULL; schema = schema -> dad )
    {
        if ( VectorDoUntil ( & schema -> alias, false, KSymbolDumpAlias, b ) )
            return b -> rc;
    }
    return 0;
}

/* VSchema
 */
static
bool VSchemaDumpInt ( const VSchema *self, SDumper *b, int dump_class )
{
    /* never dump intrinsic schema */
    if ( self -> dad == NULL )
    {
#if SLVL >= 1
        if ( SDumperMode ( b ) == sdmCompact )
            SDumperPrint ( b, "version 1;" );
        else
            SDumperPrint ( b, "version 1;\n" );
#endif
        return false;
    }

    /* always dump parent */
    if ( VSchemaDumpInt ( self -> dad, b, dump_class ) )
        return true;

    /* dump self */
#if SLVL >= 1
    if ( ( dump_class == 0 || dump_class == sdcTypes ) &&
         VectorDoUntil ( & self -> dt, false, SDatatypeDefDump, b ) )
        return true;

    if ( ( dump_class == 0 || dump_class == sdcTypesets ) &&
         VectorDoUntil ( & self -> ts, false, STypesetDefDump, b ) )
        return true;
#endif
    if ( ( dump_class == 0 || dump_class == sdcFormats ) &&
         VectorDoUntil ( & self -> fmt, false, SFormatDefDump, b ) )
        return true;
#if SLVL >= 2
    if ( ( dump_class == 0 || dump_class == sdcConstants ) &&
         VectorDoUntil ( & self -> cnst, false, SConstantDefDump, b ) )
        return true;
#endif
#if SLVL >= 3
    if ( ( dump_class == 0 || dump_class == sdcFunctions ) &&
         VectorDoUntil ( & self -> func, false, SFunctionDeclDump, b ) )
        return true;
#endif
#if SLVL >= 5
    if ( ( dump_class == 0 || dump_class == sdcColumns ) &&
         VectorDoUntil ( & self -> phys, false, SPhysicalDefDump, b ) )
        return true;
#endif
#if SLVL >= 6
    if ( ( dump_class == 0 || dump_class == sdcTables ) &&
         VectorDoUntil ( & self -> tbl, false, STableDefDump, b ) )
        return true;
    if ( ( dump_class == 0 || dump_class == sdcDatabases ) &&
         VectorDoUntil ( & self -> db, false, SDatabaseDefDump, b ) )
        return true;
#endif
    return false;
}

LIB_EXPORT rc_t CC VSchemaDump ( const VSchema *self, uint32_t mode, const char *decl,
    rc_t ( CC * flush ) ( void *fd, const void *buffer, size_t bsize ), void *fd )
{
    rc_t rc;
    bool failed;
    int dump_class = ( mode >> 8 ) & 0x7F;

    SDumper b;
    SDumperInit ( & b, self, mode & 0x80FF, flush, fd );

    if ( decl != NULL && decl [ 0 ] != 0 )
    {
        uint32_t type;
        const void *obj;
        const SNameOverload *name;

        b . mode |= MARKED_MODE;
        VSchemaClearMark ( self );

        obj = VSchemaFind ( self, & name, & type, decl, "VSchemaDump", false );

        if ( obj != NULL ) switch ( type )
        {
        case eConstant:
            SConstantMark ( obj );
            break;
        case eFormat:
            SFormatMark ( obj );
            break;
        case eDatatype:
            SDatatypeMark ( obj );
            break;
        case eTypeset:
            STypesetMark ( obj, self );
            break;
        case eFunction:
        case eUntypedFunc:
        case eScriptFunc:
            SFunctionMark ( ( void * )obj, ( void * )self );
            break;
        case ePhysical:
            SPhysicalMark ( ( void * )obj, ( void * )self );
            break;
        case eTable:
            STableMark ( ( void * )obj, ( void * )self );
            break;
        case eDatabase:
            SDatabaseMark ( ( void * )obj, ( void * )self );
            break;
        }
        else if ( name != NULL ) switch ( type )
        {
        case eFunction:
        case eUntypedFunc:
        case eScriptFunc:
            SFuncNameMark ( name, self );
            break;
        case ePhysical:
            SPhysNameMark ( name, self );
            break;
        case eTable:
            STableNameMark ( name, self );
            break;
        case eDatabase:
            SDatabaseNameMark ( name, self );
            break;
        }
    }

    failed = VSchemaDumpInt ( self, & b, dump_class );

    rc = SDumperWhack ( & b );
    return failed ? b . rc : rc;
}

static rc_t CC dump_to_stderr(void *dst, const void *buf, size_t sz)
{
    fwrite(buf, 1, sz, stderr);
    return 0;
}

rc_t VSchemaDebugPrint ( const VSchema *self, const char *decl )
{
    return VSchemaDump ( self, sdmPrint, decl, dump_to_stderr, NULL );
}

/* ToText
 *  converts some object to textual representation
 */
typedef struct VSchema2TextData VSchema2TextData;
struct VSchema2TextData
{
    char *buff;
    size_t bsize;
    size_t total;
};

static
rc_t CC flush_to_text ( void *data, const void *buffer, size_t size )
{
    VSchema2TextData *pb = data;
    if ( size != 0 )
    {
        if ( pb -> total + size >= pb -> bsize )
            return RC ( rcVDB, rcSchema, rcWriting, rcBuffer, rcInsufficient );

        memcpy ( & pb -> buff [ pb -> total ], buffer, size );
        pb -> total += size;
    }
    return 0;
}

rc_t VSchemaToText ( const VSchema *self,
    char *buff, size_t bsize, size_t *num_writ, const char *fmt, ... )
{
    SDumper b;
    rc_t rc, wrc;
    VSchema2TextData pb;

    va_list args;
    va_start ( args, fmt );

    assert ( buff != NULL );
    assert ( bsize != 0 );

    SDumperInit ( & b, self, sdmCompact, flush_to_text, & pb );

    pb . buff = buff;
    pb . bsize = bsize;
    pb . total = 0;

    rc = SDumperVPrint ( & b, fmt, args );
    wrc = SDumperWhack ( & b );

    * num_writ = pb . total;

    va_end ( args );
    return rc != 0 ? rc : wrc;
}
