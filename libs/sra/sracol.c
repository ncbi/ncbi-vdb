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

#include <sra/extern.h>
#include <sra/sradb.h>
#include <sra/types.h>
#include <vdb/schema.h>
#include <vdb/cursor.h>
#include <klib/refcount.h>
#include <klib/log.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include "sra-priv.h"

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define RC_MODULE (rcSRA)
#define RC_TARGET (rcColumn)
#define CLASS "SRAColumn"

/* AddRef
 * Release
 *  see REFERENCE COUNTING, above
 */
LIB_EXPORT rc_t CC SRAColumnAddRef( const SRAColumn *self ) {
    if ( self != NULL )
    {
        switch ( KRefcountAdd ( & self -> refcount, CLASS ) )
        {
        case krefLimit:
            return RC ( RC_MODULE, RC_TARGET, rcAttaching, rcRange, rcExcessive );
        }
    }
    return 0;
}

rc_t SRAColumnWhack ( SRAColumn *self )
{
    if ( self -> tbl != NULL )
    {
        rc_t rc = SRATableSever ( self -> tbl );
        if ( ! self -> read_only )
        {
            self -> tbl = NULL;
            return rc;
        }
    }

    free ( self );
    return 0;
}

LIB_EXPORT rc_t CC SRAColumnRelease ( const SRAColumn *self )
{
    if ( self != NULL )
    {
        rc_t rc;

        switch ( KRefcountDrop ( & self -> refcount, "SRAColumn" ) )
        {
        case krefWhack:
            return SRAColumnWhack ( ( SRAColumn* ) self );

        case krefNegative:
            rc = RC ( RC_MODULE, RC_TARGET, rcDestroying, rcSelf, rcDestroyed );
            PLOGERR (klogInt,(klogInt, rc, "Released a SRAColumn $(B) with no more references",
                      PLOG_P(self)));
            return rc;
        }
    }

    return 0;
}


/* OpenColumnRead
 *  open a column for read
 *
 *  "col" [ OUT ] - return parameter for newly opened column
 *
 *  "name" [ IN ] - NUL terminated string in UTF-8 giving column name
 *
 *  "datatype" [ IN, NULL OKAY ] - optional NUL terminated typedecl
 *  string describing fully qualified column data type, or if NULL
 *  the default type for column.
 */
LIB_EXPORT rc_t CC SRATableOpenColumnRead( const SRATable *self,
        const SRAColumn **rslt, const char *name, const char *datatype )
{
    rc_t rc;

    if ( rslt == NULL )
        rc = RC(RC_MODULE, rcColumn, rcOpening, rcParam, rcNull);
    else
    {
        if ( self == NULL )
            rc = RC(RC_MODULE, rcTable, rcAccessing, rcSelf, rcNull);
        else if ( name == NULL )
            rc = RC(RC_MODULE, rcColumn, rcOpening, rcName, rcNull);
        else if ( name [ 0 ] == 0 )
            rc = RC(RC_MODULE, rcColumn, rcOpening, rcName, rcEmpty);
        else
        {
            SRAColumn *col = malloc ( sizeof * col );
            if ( col == NULL )
                rc = RC(RC_MODULE, RC_TARGET, rcConstructing, rcMemory, rcExhausted);
            else
            {
                rc = ( datatype != NULL && datatype [ 0 ] != 0 ) ?
                    VCursorAddColumn ( self -> curs, & col -> idx, "(%s)%s", datatype, name ):
                    VCursorAddColumn ( self -> curs, & col -> idx, "%s", name );
                if ( rc == 0 || GetRCState ( rc ) == rcExists )
                {
                    KRefcountInit ( & col -> refcount, 1, CLASS, "OpenColumnRead", name );
                    col -> tbl = SRATableAttach ( self );
                    col -> read_only = true;
                    *rslt = col;
                    return 0;
                }
                else if ( GetRCState ( rc ) == rcUndefined )
                {
                    rc = RC ( rcSRA, rcTable, rcOpening, rcColumn, rcNotFound );
                }

                free ( col );
            }
        }

        * rslt = NULL;
    }

    return rc;
}


/* Datatype
 *  access data type
 *
 *  "type" [ OUT, NULL OKAY ] - returns the column type declaration
 *
 *  "def" [ OUT, NULL OKAY ] - returns the definition of the type
 *  returned in "type_decl"
 *
 * NB - one of "type" and "def" must be non-NULL
 */
LIB_EXPORT rc_t CC SRAColumnDatatype( const SRAColumn *self,
        VTypedecl *type, VTypedef *def )
{
    if (self != NULL)
        return SRATableColDatatype(self->tbl, self->idx, type, def);

    if ( type != NULL )
        memset ( type, 0, sizeof * type );
    if ( def != NULL )
        memset ( def, 0, sizeof * def );

    return RC(RC_MODULE, RC_TARGET, rcAccessing, rcSelf, rcNull);
}


/* GetRange
 *  get a contiguous range around a spot id, e.g. tile for Illumina
 *
 *  "id" [ IN ] - return parameter for 1-based spot id
 *
 *  "first" [ OUT, NULL OKAY ] and "last" [ OUT, NULL OKAY ] -
 *  id range is returned in these output parameters, where
 *  at least ONE must be NOT-NULL
 */
LIB_EXPORT rc_t CC SRAColumnGetRange( const SRAColumn *self,
        spotid_t id, spotid_t *first, spotid_t *last )
{
    if (self != NULL)
        return SRATableGetIdRange(self->tbl, self->idx, id, first, last);

    if ( first != NULL )
        * first = 0;
    if ( last != NULL )
        * last = 0;

    return RC(RC_MODULE, RC_TARGET, rcAccessing, rcSelf, rcNull);
}


/* Read
 *  read row data
 *
 *  "id" [ IN ] - spot row id between 1 and max(spot id)
 *
 *  "base" [ OUT ] and "offset" [ OUT ] - pointer and bit offset
 *  to start of spot row data.
 *
 *  "size" [ OUT ] - size in bits of row data
 */
LIB_EXPORT rc_t CC SRAColumnRead( const SRAColumn *self,
        spotid_t id, const void **base, bitsz_t *offset, bitsz_t *size )
{
    if (self != NULL)
        return SRATableRead(self->tbl, id, self->idx, base, offset, size);

    if ( base != NULL )
        * base = NULL;
    if ( offset != NULL )
        * offset = 0;
    if ( size != NULL )
        * size = 0;

    return RC(RC_MODULE, RC_TARGET, rcAccessing, rcSelf, rcNull);
}
