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
#include <vdb/database.h>
#include <vdb/table.h>
#include <vdb/vdb-priv.h>
#include <vdb/cursor.h>
#include <kdb/meta.h>
#include <klib/rc.h>

#include "ref-tbl.h"
#include <string.h>

rc_t AlignRefTableCursor( const VTable* table, const VCursor *native_curs,
                          const VCursor** ref_cursor,const VTable **reftbl )
{
	rc_t rc = 0;
	char ref_tbl_name[ 512 ] = "REFERENCE";
	const KMetadata * meta;
	const VCursor * curs;

    rc = VTableOpenMetadataRead( table, &meta );
	if ( rc == 0 )
    {
	    const KMDataNode* node;
        rc = KMetadataOpenNodeRead( meta, &node, "CONFIG/REF_TABLE" );
        if ( rc == 0 )
        {
            size_t sz;
            rc = KMDataNodeReadCString( node, ref_tbl_name, sizeof( ref_tbl_name ), &sz );
            ref_tbl_name[ sz ] = '\0';
            KMDataNodeRelease( node );
        }
        KMetadataRelease( meta );
    }

	rc = VCursorLinkedCursorGet( native_curs,ref_tbl_name,&curs );
	if ( rc != 0 )
    {
		const VDatabase *db;
        const VTable *tbl;

        /* get at the parent database */
        rc = VTableOpenParentRead ( table, & db );
        if ( rc != 0 )
            return rc;

        /* open the table */
        rc = VDatabaseOpenTableRead ( db, &tbl, "%s", ref_tbl_name );
        VDatabaseRelease ( db );
        if ( rc != 0 )
            return rc;

        /* create a cursor */
        rc = VTableCreateCachedCursorRead( tbl, &curs, 1024*1024*1024 );
		if ( reftbl != NULL )
        {
			*reftbl = tbl;
        }
        else
        {
			VTableRelease( tbl );
		}
        if ( rc != 0 )
            return rc;

        rc = VCursorPermitPostOpenAdd( curs );
        if ( rc != 0 )
            return rc;

        rc = VCursorOpen( curs );
        if ( rc != 0 )
            return rc;

        if ( native_curs != NULL )
            rc = VCursorLinkedCursorSet( native_curs, ref_tbl_name, curs );
        if ( rc != 0 )
            return rc;

	}
	*ref_cursor = curs;

	return 0;
}
