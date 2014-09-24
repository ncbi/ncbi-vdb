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

#define KONST const
#include "table-priv.h"
#undef KONST

#include <klib/namelist.h>
#include <klib/rc.h>
#include <sysalloc.h>

/* List
 *  (moved from table-cmn.c to here, because needs access to VCursor...)
 *  create a namelist of all usable columns :
 *  columns, that can be added to a cursor - and the following cursor-open will not fail
 */
LIB_EXPORT rc_t CC VTableListWritableColumns ( struct VTable *self, KNamelist **names )
{
    rc_t rc;

    if ( names == NULL )
        rc = RC ( rcVDB, rcTable, rcListing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcTable, rcListing, rcSelf, rcNull );
        else
        {
            VNamelist *list;
            rc = VNamelistMake ( & list, 0 );
            if ( rc == 0 )
            {
                rc = VNamelistToNamelist ( list, names );
                VNamelistRelease ( list );
                return rc;
            }
        }

        * names = NULL;
    }
    return rc;
}


/* ListDatatypes
 *  returns list of typedecls for named column
 *
 *  "col" [ IN ] - column name
 *
 *  "typedecls" [ OUT ] - list of datatypes available for named column
 *
 *  availability: v2.1
 */
LIB_EXPORT rc_t CC VTableListWritableDatatypes ( struct VTable *self,
    const char *col, KNamelist **typedecls )
{
    rc_t rc;
    
    if ( typedecls == NULL )
        rc = RC ( rcVDB, rcTable, rcListing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcTable, rcListing, rcSelf, rcNull );
        else if ( col == NULL )
            rc = RC ( rcVDB, rcSchema, rcListing, rcName, rcNull );
        else if ( col [ 0 ] == 0 )
            rc = RC ( rcVDB, rcSchema, rcListing, rcName, rcEmpty );
        else
        {
            /* make eventually returned */
            VNamelist *vlist;
            rc = VNamelistMake ( &vlist, 0 );
            if ( rc == 0 )
            {
                rc = VNamelistToNamelist ( vlist, typedecls );
                VNamelistRelease ( vlist );
                return rc;
            }
        }

        * typedecls = NULL;
    }

    return rc;
}
