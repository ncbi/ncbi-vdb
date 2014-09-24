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

#include <tui/extern.h>

#include <klib/rc.h>
#include <klib/text.h>
#include <klib/printf.h>
#include <kfs/directory.h>

#include <vfs/manager.h>
#include <vfs/path.h>

#include <tui/tui_dlg.h>

#include <sysalloc.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


rc_t native_to_internal( VFSManager * vfs_mgr, const char * native, char * buffer, uint32_t buffer_size, size_t * written )
{
    VPath * temp_v_path;
    rc_t rc = VFSManagerMakeSysPath ( vfs_mgr, &temp_v_path, native );
    if ( rc == 0 )
    {
        rc = VPathReadPath ( temp_v_path, buffer, buffer_size, written );
        VPathRelease ( temp_v_path );
    }
    return rc;
}


rc_t internal_to_native( VFSManager * vfs_mgr, const char * internal, char * buffer, uint32_t buffer_size, size_t * written )
{
	rc_t rc = 0;
	if ( internal[ 0 ] == '/' && internal[ 1 ] == 0 )
	{
		buffer[ 0 ] = '/'; buffer[ 1 ] = 0; *written = 1;
	}
	else
	{
		VPath * temp_v_path;
		rc = VFSManagerMakePath ( vfs_mgr, &temp_v_path, "%s", internal );
		if ( rc == 0 )
		{
			rc = VPathReadSysPath ( temp_v_path, buffer, buffer_size, written );
			VPathRelease ( temp_v_path );
		}
	}
    return rc;
}


rc_t set_native_caption( struct KTUIDlg * dlg, VFSManager * vfs_mgr, uint32_t id, const char * internal_path )
{
	tui_rect r;
	rc_t rc = KTUIDlgGetWidgetRect ( dlg, id, &r );
	if ( rc == 0 )
	{
		size_t written = 0;
		char native[ 4096 ] = "";
		rc = internal_to_native( vfs_mgr, internal_path, native, sizeof native, &written );
		if ( rc == 0 )
		{
			if ( written <= ( r.w - 2 ) )
				rc = KTUIDlgSetWidgetCaption ( dlg, id, native );
			else
			{
				size_t written_2;
				char temp[ 4096 ];
				uint32_t part = ( r.w - 3 ) / 2;
				rc = string_printf ( temp, sizeof temp, &written_2,
						"%.*s ... %s", part, native, &native[ ( written - part ) - 1 ] );
				if ( rc == 0 )
					rc = KTUIDlgSetWidgetCaption ( dlg, id, temp );
			}
		}
	}
	return rc;
}


typedef struct dir_callback_ctx
{
	struct KTUIDlg * dlg;
	const char * str_to_focus;
	uint32_t widget_id;
	uint32_t string_id;
	uint32_t to_focus;
    bool in_root;
} dir_callback_ctx;


static rc_t add_str_to_listbox( dir_callback_ctx * dctx, const char * name )
{
    rc_t rc = 0;
    if ( dctx->in_root )
    {
#ifdef WINDOWS
        size_t written;
        char buffer[ 16 ];
        rc = string_printf( buffer, sizeof buffer, &written, "%s:\\", name );
        if ( rc == 0 && written > 0 )
            rc = KTUIDlgAddWidgetString ( dctx->dlg, dctx->widget_id, buffer );
#else
        rc = KTUIDlgAddWidgetString ( dctx->dlg, dctx->widget_id, name );
#endif        
    }
    else
        rc = KTUIDlgAddWidgetString ( dctx->dlg, dctx->widget_id, name );
    return rc;
}


static rc_t CC on_dir_entry( const KDirectory *dir, uint32_t type, const char * name, void * data )
{
    rc_t rc = 0;
    if ( name != NULL && data != NULL && name[ 0 ] != 0 && name[ 0 ] != '.' )
    {
        dir_callback_ctx * dctx = data;
        if ( ( type & ~kptAlias ) == kptDir )
		{
			rc = add_str_to_listbox( dctx, name );
			if ( rc == 0 )
			{
				if ( dctx->str_to_focus != NULL )
				{
					int cmp = string_cmp ( name, string_size( name ),
										   dctx->str_to_focus, string_size( dctx->str_to_focus ), 4096 );
					if ( cmp == 0 )
						dctx->to_focus = dctx->string_id;
				}
				dctx->string_id++;
			}
		}
    }
    return rc;
}


rc_t fill_widget_with_dirs( struct KTUIDlg * dlg, KDirectory * dir, uint32_t id, const char * path, const char * to_focus )
{
	rc_t rc;
	
	dir_callback_ctx dctx;
	dctx.string_id = 0;
	dctx.to_focus = 0;
	dctx.str_to_focus = to_focus;
	dctx.in_root = ( ( path[ 0 ] == '/' )&&( path[ 1 ] == 0 ) );

	rc = KTUIDlgRemoveAllWidgetStrings ( dlg, id );
	if ( rc == 0 )
	{
		if ( !dctx.in_root )
		{
			rc = KTUIDlgAddWidgetString ( dlg, id, "[ .. ]" );
			dctx.string_id++;
		}
	}

	if ( rc == 0 )
	{
		dctx.dlg = dlg;
		dctx.widget_id = id;

		/* we allow it to fail... */
		KDirectoryVisit ( dir, false, on_dir_entry, &dctx, "%s", path );
	}

	if ( rc == 0 && dctx.to_focus > 0 )
		rc = KTUIDlgSetWidgetSelectedString ( dlg, id, dctx.to_focus );

    return rc;
}


typedef struct file_callback_ctx
{
    struct KTUIDlg * dlg;
    const char * extension;
	uint32_t id;
} file_callback_ctx;


static rc_t CC on_file_entry( const KDirectory *dir, uint32_t type, const char * name, void * data )
{
    rc_t rc = 0;
    if ( name != NULL && data != NULL && name[ 0 ] != 0 && name[ 0 ] != '.' )
    {
        file_callback_ctx * fctx = data;
        if ( ( type & ~kptAlias ) == kptFile )
        {
            bool add = ( fctx->extension == NULL );
            if ( !add )
            {
                size_t name_size = string_size ( name );
                size_t ext_size = string_size ( fctx->extension );
                if ( name_size > ext_size )
                {
                    int cmp = string_cmp ( &name[ name_size - ext_size ], ext_size,
                                           fctx->extension, ext_size, (uint32_t)ext_size );
                    add = ( cmp == 0 );
                }
            }
            if ( add )
				rc = KTUIDlgAddWidgetString ( fctx->dlg, fctx->id, name );
        }
    }
    return rc;
}


rc_t fill_widget_with_files( struct KTUIDlg * dlg, KDirectory * dir, uint32_t id, const char * path, const char * extension )
{
	rc_t rc = KTUIDlgRemoveAllWidgetStrings ( dlg, id );
	if ( rc == 0 )
    {
        file_callback_ctx fctx;
        fctx.extension = extension;
		fctx.dlg = dlg;
		fctx.id = id;
        rc = KDirectoryVisit ( dir, false, on_file_entry, &fctx, "%s", path );
    }
    return rc;
}
