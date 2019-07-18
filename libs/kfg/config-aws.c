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


#include <klib/text.h>
#include <klib/rc.h>
#include <klib/printf.h>
#include <kfs/file.h>
#include <kfs/directory.h>


#include <ctype.h>
#include <os-native.h>
#include <stdlib.h>

#include <kfg/kfg-priv.h>

static
rc_t aws_KConfigNodeUpdateChild ( KConfigNode *self, String *name, String *value )
{
    KConfigNode * child;
    rc_t rc = KConfigNodeOpenNodeUpdate ( self, & child, "%S", name );
    if ( rc == 0 )
    {
        rc = KConfigNodeWrite ( child, value -> addr, value -> size );
        KConfigNodeRelease ( child );
    }

    return rc;
}

static
rc_t aws_extract_key_value_pair ( const String *source, String *key, String *val )
{
    String k, v;
    const char *start = source -> addr;
    const char *end = start + source -> size;

    char *eql = string_chr ( start, source -> size, '=' );
    if ( eql == NULL )
        return RC ( rcKFG, rcChar, rcSearching, rcFormat, rcInvalid );

    /* key */
    StringInit ( &k, start, eql - start, string_len ( start, eql - start ) );
    StringTrim ( &k, key );

    start = eql + 1;

    /* value */
    StringInit ( &v, start, end - start,  string_len ( start, end - start ) ); 
    StringTrim ( &v, val );
    return 0;
}

static
void aws_parse_file ( const KFile *self, KConfigNode *aws_node, 
                      char *buffer, size_t buf_size, bool isCredentialsFile )
{
    char *sep;
    const char *start = buffer;
    const char *end = start + buf_size;

    for ( ; start < end; start = sep + 1 )
    {
        rc_t rc;
        String string, trim;
        String key, value;
        
        sep = string_chr ( start, end - start, '\n' );
        if ( sep == NULL )
            sep = ( char * ) end;

        StringInit ( &string, start, sep - start, string_len ( start, sep - start ) );
        
        StringTrim ( &string, &trim );

        /* check for comment line and skip */
        if ( StringLength ( & trim ) != 0 && trim . addr [ 0 ] == '#' )
            continue;

        /* check for [default] line */
        if ( StringLength ( & trim ) != 0 && trim . addr [ 0 ] == '[' )
            continue;

        /* check for key/value pairs and skip if none found */
        rc = aws_extract_key_value_pair ( &trim, &key, &value );
        if ( rc != 0 )
            continue;

        /* now check keys we are looking for and populate the node*/
        if ( isCredentialsFile )
        {
            String access_key_id, secret_access_key;
            CONST_STRING ( &access_key_id, "aws_access_key_id" );
            CONST_STRING ( &secret_access_key, "aws_secret_access_key" );

            if ( StringCaseEqual ( &key, &access_key_id ) )
            {
                rc = aws_KConfigNodeUpdateChild ( aws_node, &key, &value );
                if ( rc != 0 )
                    return;
            }
            if ( StringCaseEqual ( &key, &secret_access_key  ) )
            {
                rc = aws_KConfigNodeUpdateChild ( aws_node, &key, &value );
                if ( rc != 0 )
                    return;
            }
        }
        else
        {
            String region, output;
            CONST_STRING ( &region, "region" );
            CONST_STRING ( &output, "output" );

            if ( StringCaseEqual ( &key, &region ) )
            {
                rc = aws_KConfigNodeUpdateChild ( aws_node, &key, &value );
                if ( rc != 0 )
                    return;
            }
            if ( StringCaseEqual ( &key, &output  ) )
            {
                rc = aws_KConfigNodeUpdateChild ( aws_node, &key, &value );
                if ( rc != 0 )
                    return;
            }
        }
    }
}

static
rc_t aws_find_nodes ( KConfigNode *aws_node, const char *aws_path )
{
    KDirectory *wd;

    rc_t rc = KDirectoryNativeDir ( &wd );
    if ( rc == 0 )
    {
        char *buffer;
        size_t num_read;
        uint64_t buf_size;

        const KFile *credentials, *config;

        rc = KDirectoryOpenFileRead ( wd, &credentials, "%s%s", aws_path, "/credentials" );
        if ( rc == 0 )
        {
            rc = KFileSize ( credentials, &buf_size );
            if ( rc == 0 )
            {
                buffer = malloc ( buf_size );
                if ( buffer != NULL )
                {
                    rc = KFileReadAll ( credentials, 0, buffer, ( size_t ) buf_size, &num_read );

                    if ( rc == 0 )
                        aws_parse_file ( credentials, aws_node, buffer, num_read, true );

                    free ( buffer );
                }

            }

            KFileRelease ( credentials );
        }

        rc = KDirectoryOpenFileRead ( wd, &config, "%s%s", aws_path, "/config" );
        if ( rc == 0 )
        {
            rc = KFileSize ( config, &buf_size );
            if ( rc == 0 )
            {
                buffer = malloc ( buf_size );
                if ( buffer != NULL )
                {
                    rc = KFileReadAll ( config, 0, buffer, ( size_t ) buf_size, &num_read );
                    
                    if ( rc == 0 )
                        aws_parse_file ( config, aws_node, buffer, num_read, false );

                    free ( buffer );
                }
            }

            KFileRelease ( config );
        }

        KDirectoryRelease ( wd );
    }            
           
    return rc;
}


static
void check_env ( const KConfig *self, char *path, size_t path_size )
{
    size_t num_read;
    const char *home;
    
    const KConfigNode *home_node;
    
    /* Check to see if home node exists */
    rc_t rc = KConfigOpenNodeRead ( self, &home_node, "HOME" );
    if ( home_node == NULL )
    {
        /* just grab the HOME env variable */
        home = getenv ( "HOME" );
        if ( home != NULL )
        {
            num_read = string_copy_measure ( path, path_size, home );
            if ( num_read >= path_size )
                path [ 0 ] = 0;
        }
    }
    else
    {
        /* if it exists check for a path */
        rc = KConfigNodeRead ( home_node, 0, path, path_size, &num_read, NULL );
        if ( rc != 0 )
        {
            home = getenv ( "HOME" );
            if ( home != NULL )
            {
                num_read = string_copy_measure ( path, path_size, home );
                if ( num_read >= path_size )
                    path [ 0 ] = 0;
            }

        }
        
        rc = KConfigNodeRelease ( home_node );
    }
}

extern
void add_aws_nodes ( KConfig *self )
{
    char home [ 4096 ] = "";

    size_t num_writ;
    char path [ 4096 ];
    rc_t rc;

    check_env ( self, home, sizeof home );
    /* if home environtment is found, create AWS root node */

    if ( home [ 0 ] != 0 )
    {
        rc = string_printf ( path, sizeof path, &num_writ, "%s/.aws", home );
        if ( rc == 0 && num_writ != 0 )
        {
            KConfigNode *aws_node;
            
            /* create aws node */
            rc = KConfigOpenNodeUpdate ( self, &aws_node, "AWS", NULL );
            if ( rc == 0 )
                rc = aws_find_nodes ( aws_node, path );
            
            rc = KConfigNodeRelease ( aws_node );
        }
    }
}
