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

#include <sysalloc.h>

#include <Shlobj.h>

#define LNK_RES_ERROR 0
#define LNK_RES_FILE 1
#define LNK_RES_DIR 2

#define LNK_FLAG_HasLinkTargetIDList 0x1
#define LNK_FLAG_HasLinkInfo 0x2
#define LNK_FLAG_HasName 0x4
#define LNK_FLAG_HasRelativePath 0x8
#define LNK_FLAG_HasWorkingDir 0x10
#define LNK_FLAG_HasArguments 0x20
#define LNK_FLAG_HasIconLocation 0x40
#define LNK_FLAG_IsUniCode 0x80
#define LNK_FLAG_ForceNoLinkInfo 0x100
#define LNK_FLAG_HasExpString 0x200
#define LNK_FLAG_RunInSeparateProcess 0x400
#define LNK_FLAG_Unused1 0x800
#define LNK_FLAG_HasDarwinID 0x1000
#define LNK_FLAG_RunAsUser 0x2000
#define LNK_FLAG_HasExpIcon 0x4000
#define LNK_FLAG_NoPidIAlias 0x8000
#define LNK_FLAG_Unused2 0x10000
#define LNK_FLAG_RunWithShimLayer 0x20000
#define LNK_FLAG_ForceNoLinkTrack 0x40000
#define LNK_FLAG_EnableTargetMatdata 0x80000
#define LNK_FLAG_DisableLinkPathTracking 0x100000
#define LNK_FLAG_DisableKnowFolderTracking 0x200000
#define LNK_FLAG_DisableKnowFolderAlias 0x400000
#define LNK_FLAG_AllowLinkToLink 0x800000
#define LNK_FLAG_UnaliasOnSave 0x1000000


#define LNK_ATTR_FILE_ATTRIBUTE_READONLY 0x1
#define LNK_ATTR_FILE_ATTRIBUTE_HIDDEN 0x2
#define LNK_ATTR_FILE_ATTRIBUTE_SYSTEM 0x4
#define LNK_ATTR_Reserved1 0x8
#define LNK_ATTR_FILE_ATTRIBUTE_DIRECTORY 0x10
#define LNK_ATTR_FILE_ATTRIBUTE_ARCHIVE 0x20
#define LNK_ATTR_Reserved2 0x40
#define LNK_ATTR_FILE_ATTRIBUTE_NORMAL 0x80
#define LNK_ATTR_FILE_ATTRIBUTE_TEMPORARY 0x100
#define LNK_ATTR_FILE_ATTRIBUTE_SPARSE_FILE 0x200
#define LNK_ATTR_FILE_ATTRIBUTE_REPARSE_POINT 0x400
#define LNK_ATTR_FILE_ATTRIBUTE_COMPRESSED 0x800
#define LNK_ATTR_FILE_ATTRIBUTE_OFFLINE 0x1000
#define LNK_ATTR_FILE_ATTRIBUTE_NOT_CONTENT_INDEXED 0x2000
#define LNK_ATTR_FILE_ATTRIBUTE_ENCRYPTED 0x4000

/* the first 20 "magic" bytes of a MS-lnk-file
   ( 4 bytes header-size and 16 bytes LinkCLSID ) */
const unsigned char lnk_ref[ 20 ] =
{
    0x4C, 0x00, 0x00, 0x00, 0x01, 0x14, 0x02, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x46
};


static bool has_lnk_extension( const wchar_t *path )
{
    size_t len = wcslen( path );

    if ( len < 5 ) return false;
    if ( path[ len - 4 ] != '.' ) return false;
    if ( path[ len - 3 ] != 'l' && path[ len - 3 ] != 'L' ) return false;
    if ( path[ len - 2 ] != 'n' && path[ len - 2 ] == 'N' ) return false;
    if ( path[ len - 1 ] != 'k' && path[ len - 1 ] == 'K' ) return false;
    return true;
}


static bool add_lnk_extension( wchar_t *path, const size_t path_size )
{
    size_t len = wcslen( path );
    if ( len + 5 >= path_size )
        return false;
    path[ len + 0 ] = '.';
    path[ len + 1 ] = 'l';
    path[ len + 2 ] = 'n';
    path[ len + 3 ] = 'k';
    path[ len + 5 ] = 0;
    return true;
}


static HANDLE lnk_file_open( const wchar_t *path, DWORD * filesize )
{
    HANDLE hfile = CreateFileW( path,           // file to open
                         GENERIC_READ,          // open for reading
                         FILE_SHARE_READ,       // share for reading
                         NULL,                  // default security
                         OPEN_EXISTING,         // existing file only
                         FILE_ATTRIBUTE_NORMAL, // normal file
                         NULL );                // no attr. template
    if ( hfile != INVALID_HANDLE_VALUE )
    {
        if ( filesize != NULL )
            *filesize = GetFileSize( hfile, NULL );
    }
    return hfile;
}


static bool lnk_file_exists( const wchar_t *path )
{
    HANDLE hFile = lnk_file_open( path, NULL );
    if ( hFile != INVALID_HANDLE_VALUE )
    {
        CloseHandle( hFile );
        return true;
    }
    else
        return false;
} 


static unsigned char * lnk_file_read( HANDLE hfile, const size_t n_bytes, DWORD * read )
{
    unsigned char * res;

    if ( read != NULL ) *read = 0;
    res = (unsigned char *)malloc( n_bytes );
    if ( res != NULL )
    {
        DWORD dwBytesRead;
        if ( FALSE == ReadFile( hfile, res, ( DWORD ) n_bytes, &dwBytesRead, NULL ) )
        {
            free( res );
            res = NULL;
        }
        else
        {
            if ( read != NULL )
                *read = dwBytesRead;
        }
    }
    return res;
}


static DWORD lnk_file_get_DWORD( const unsigned char * buffer, const int idx )
{
    DWORD res = 0;
    memmove( &res, &(buffer[ idx ]), sizeof( res ) );
    return res;
}

static WORD lnk_file_get_WORD( const unsigned char * buffer, const int idx )
{
    WORD res = 0;
    memmove( &res, &(buffer[ idx ]), sizeof( res ) );
    return res;
}


static bool lnk_file_validate_buffer( const unsigned char * buffer, const size_t buffsize )
{
    bool res = ( buffsize >= sizeof lnk_ref );
    if ( res )
        res = ( 0 == memcmp ( buffer, lnk_ref, sizeof lnk_ref ) );
    return res;
}


/* the path is given in windows-native notation and wide-char */
static bool lnk_file_validate( const wchar_t *path )
{
    HANDLE hfile;
    bool res;

    hfile = lnk_file_open( path, NULL );
    res = ( hfile != INVALID_HANDLE_VALUE );
    if ( res )
    {
        DWORD dwBytesRead;
        unsigned char * buffer = lnk_file_read( hfile, 32, &dwBytesRead );
        CloseHandle( hfile );
        res = ( buffer != NULL );
        if ( res )
        {
            res = lnk_file_validate_buffer( buffer, dwBytesRead );
            free( buffer );
        }
    }
    return res;
}

/*  resolves a lnk file, after it's content was loaded into a buffer
    buffer   ...    IN  points to the file-content
    buffsize ...    IN  also the size of the lnk-file
    resolved ...    OUT resolved path as wchar_t in windows-native form

    return-value    0 ... resolving failed
                    1 ... resolved path is a file
                    2 ... resolved path is a directory
*/
static int lnk_file_resolve_buffer( const unsigned char * buffer, const size_t buffsize,
                                    wchar_t ** resolved )
{
    char * base_path = NULL;
    int res = LNK_RES_ERROR;
    if ( lnk_file_validate_buffer( buffer, buffsize ) )
    {
        int idx = 0x4C;
        DWORD flags = lnk_file_get_DWORD( buffer, 0x14 );
        DWORD attr  = lnk_file_get_DWORD( buffer, 0x18 );

        /* we have to handle the TargetIDList even if it is of no relevance
        to resolving the link, just to get the correct index for the next
        section */
        if ( flags & LNK_FLAG_HasLinkTargetIDList )
            idx += ( lnk_file_get_WORD( buffer, idx ) + 2 );
        if ( flags & LNK_FLAG_HasLinkInfo )
        {
            DWORD base_path_ofs = lnk_file_get_DWORD( buffer, idx + 16 );
            if ( base_path_ofs > 0 )
            {
                const char * src = (const char *)&( buffer[ idx + base_path_ofs ]);
                if ( *src != 0 )
                {
                    size_t required;
                    errno_t e = mbstowcs_s( &required, NULL, 0, src, 0 );
                    if ( required > 0 )
                    {
                        *resolved = (wchar_t *) malloc( ( required + 1 ) * 2 );
                        if ( *resolved )
                        {
                            size_t converted;
                            mbstowcs_s( &converted, *resolved, required + 1, src, required + 1 );
                            if ( attr & LNK_ATTR_FILE_ATTRIBUTE_DIRECTORY )
                                res = LNK_RES_DIR;
                            else
                                res = LNK_RES_FILE;
                        }
                    }
                }
            }
        }
    }
    return res;
}


/* translate a given lnk-file (full path) 'c:\somewhere\alink.lnk' 
   into the path the lnk-file contains: 
   'c:\somewhere\subpath\file.txt'
   or 
   'c:\anotherpath\sub\sub\file.txt' 
   */
static int lnk_file_resolve( const wchar_t *lnk_file, wchar_t ** resolved )
{
    HANDLE hfile;
    DWORD dwFileSize;
    int res = LNK_RES_ERROR;

    if ( resolved != NULL )
    {
        *resolved = NULL;
        hfile = lnk_file_open( lnk_file, &dwFileSize );
        if ( hfile != INVALID_HANDLE_VALUE )
        {
            unsigned char * buffer = lnk_file_read( hfile, (size_t)dwFileSize, NULL );
            if ( buffer != NULL )
            {
                res = lnk_file_resolve_buffer( buffer, dwFileSize, resolved );
                free( buffer );
            }
        }
    }
    return res;
}


static bool win_path_exists( const wchar_t * path )
{
    bool res = false;
    if ( path != NULL )
    {
        HANDLE hfile = CreateFileW( path, 
                                0, /* do not ask for RD or WR, that prevents access-denied-err */
                                FILE_SHARE_READ, /* needed to get a handle to obj's that are shared */
                                NULL,
                                OPEN_EXISTING,
                                FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS,
                                NULL );
        if ( hfile != INVALID_HANDLE_VALUE )
        {
            res = true;
            CloseHandle( hfile );
        }
#if _DEBUGGING
        else
        {
            DWORD status = GetLastError ();
            switch ( status )
            {
            case ERROR_FILE_NOT_FOUND:
            case ERROR_PATH_NOT_FOUND:
                break;
            default:
                DBGMSG ( DBG_KFS, DBG_FLAG_ANY, ( "win_path_exists: WARNING - unrecognized error code - %u.", status ) );
            }
        }
#endif
    }
    return res;
}

static
wchar_t * less_brain_dead_wcsdup ( const wchar_t * path )
{
    if ( path != NULL )
    {
        size_t len = wcslen ( path ) + 3;
        wchar_t *cpy = malloc ( len * sizeof * cpy );
        if ( cpy != NULL )
            return wcscpy ( cpy, path );
    }
    return NULL;
}

static size_t win_existing_path( const wchar_t *path )
{
    size_t res = 0;
    wchar_t * temp = less_brain_dead_wcsdup( path );
    if ( temp != NULL )
    {
        wchar_t * part = wcsrchr( temp, L'\\' );
        while ( part != NULL && res == 0 )
        {
            *part = 0;
            if ( win_path_exists( temp ) )
                res = ( part - temp );
            else
                part = wcsrchr( temp, L'\\' );
        }
        free( temp );
    }
    return res;
}


static wchar_t * win_assemble_lnk( const wchar_t *path, const size_t exists, size_t * remaining )
{
    wchar_t * res = NULL;
    if ( path != NULL && remaining != NULL )
    {
        size_t path_len, buff_len;

        *remaining = 0;
        path_len = wcslen( path );
        #define ExtraChars 5
        buff_len = ( path_len + ExtraChars ) * sizeof( * path );
        res = (wchar_t *)malloc( buff_len );
        if ( res != NULL )
        {
            size_t term;
            const wchar_t * from = ( path + exists + 1 );
            const wchar_t * next_bs = wcschr( from, L'\\' );
            if ( next_bs == NULL )
                term = path_len;
            else
            {
                term = ( next_bs - path );
                *remaining = term;
            }
            /* if the string to be copied does not fit into the buffer 
               wcsncpy_s will terminate the application instead of returning
               an error !!! */
            wcsncpy_s( res, path_len + ExtraChars, path, term ); /* 2nd argument is in elements, not bytes */
            res[ term + 0 ] = L'.';
            res[ term + 1 ] = L'l';
            res[ term + 2 ] = L'n';
            res[ term + 3 ] = L'k';
            res[ term + 4 ] = 0;
        }
        #undef ExtraChars
    }
    return res;
}


static wchar_t * win_assemble_path( const wchar_t *part1, const wchar_t *part2 )
{
    wchar_t *res = NULL;
    if ( part1 != NULL && part2 != NULL )
    {
        size_t len1 = wcslen( part1 );
        size_t len2 = wcslen( part2 );
        res = (wchar_t *)malloc( ( len1 + len2 + 1 ) * sizeof( * part1 ) );
        if ( res != NULL )
        {
            /* if the string to be copied does not fit into the buffer 
               wcsncpy_s will terminate the application instead of returning
               an error !!! */
            wcsncpy_s( res, len1+1, part1, len1 );
            wcsncpy_s( res+len1, len2+1, part2, len2 );
            res[ len1 + len2 ] = 0;
        }
    }
    return res;
}


static bool win_resolve_path( const wchar_t *path, wchar_t ** resolved, const int depth )
{
    bool res = false;

    if ( depth > 10 )
        return res;
    if ( resolved != NULL )
    {
        *resolved = NULL;
        res = win_path_exists( path );
        if ( res )
            /* the given path exists, no need to resolve links... */
            *resolved = less_brain_dead_wcsdup( path );
        else
        {
            /* detect the part of the path that does exist
            exists points at the backslash behind the existing path */
            size_t exists = win_existing_path( path );
            if ( exists > 0 )
            {
                size_t remaining;
                wchar_t * possible_lnk = win_assemble_lnk( path, exists, &remaining );
                if ( possible_lnk != NULL )
                {
                    wchar_t * lnk_resolved = NULL;
                    int status = lnk_file_resolve( possible_lnk, &lnk_resolved );
                    if ( status != LNK_RES_ERROR )
                    {
                        if ( remaining > 0 )
                        {
                            wchar_t * new_path = win_assemble_path( lnk_resolved, path + remaining );
                            free( lnk_resolved );
                            if ( new_path != NULL )
                            {
                                res = win_resolve_path( new_path, resolved, depth + 1 );
                                free( new_path );
                            }
                        }
                        else
                        {
                            *resolved = lnk_resolved;
                            res = true;
                        }
                    }
                    free( possible_lnk );
                }
            }
        }
    }
    return res;
}


bool win_CreateLink( const wchar_t * target, const wchar_t * lnk_file, const wchar_t * desc )
{ 
    bool res = false;
    HRESULT hres;
    IShellLink * psl; 

    // Get a pointer to the IShellLink interface. 
    hres = CoCreateInstance( &CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, 
                             &IID_IShellLink, (LPVOID*)&psl ); 
    if ( SUCCEEDED( hres ) ) 
    { 
        IPersistFile* ppf; 
 
        // Set the path to the shortcut target and add the description. 
        psl -> lpVtbl -> SetPath( psl, target );
        if ( desc != NULL )
            psl -> lpVtbl -> SetDescription( psl, desc ); 
 
        // Query IShellLink for the IPersistFile interface for saving the 
        // shortcut in persistent storage. 
        hres = psl -> lpVtbl -> QueryInterface( psl, &IID_IPersistFile, (LPVOID*)&ppf ); 
        if ( SUCCEEDED( hres ) ) 
        { 
            // Save the link by calling IPersistFile::Save. 
            hres = ppf -> lpVtbl -> Save( ppf, lnk_file, TRUE );
            res = SUCCEEDED( hres );
            ppf -> lpVtbl -> Release( ppf );
        } 
        psl -> lpVtbl -> Release( psl );
    } 
    return res; 
}
