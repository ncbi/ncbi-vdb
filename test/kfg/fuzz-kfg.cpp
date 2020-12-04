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

#include <kfs/directory.h>
#include <kfg/config.h>
#include <kfg/kfg-priv.h>

#include <klib/text.h>
#include <klib/out.h>
#include <klib/namelist.h>

#include <stdexcept>
#include <iostream>

using namespace std;

class KfgFuzzer
{
public:
    KfgFuzzer()
    : wd ( nullptr ), kfg ( nullptr )
    {
        if ( KDirectoryNativeDir ( & wd ) != 0 )
            throw logic_error("KDirectoryNativeDir failed");
        if ( KConfigMake ( & kfg, NULL ) != 0)
            throw logic_error("KConfigMake failed");
    }
    ~KfgFuzzer()
    {
        KConfigRelease ( kfg );
        KDirectoryRelease ( wd );
    }

    void Run ( const uint8_t * p_data, size_t p_size )
    {
        test_KConfigParse ( p_data, p_size );

        test_KConfigRead ( p_data, p_size );
        test_KConfigReadBool ( p_data, p_size );
        test_KConfigWriteBool ( p_data, p_size );
        test_KConfigReadI64 ( p_data, p_size );
        test_KConfigReadU64 ( p_data, p_size );
        test_KConfigReadF64 ( p_data, p_size );
        test_KConfigReadString ( p_data, p_size );
        test_KConfigWriteString ( p_data, p_size );
        test_KConfigWriteSString ( p_data, p_size );
        test_KConfigPrint ( p_data, p_size );
        test_KConfigOpenNodeRead ( p_data, p_size );
        test_KConfigNodeOpenNodeRead ( p_data, p_size );
        test_KConfigOpenNodeUpdate ( p_data, p_size );
        test_KConfigNodeOpenNodeUpdate ( p_data, p_size );
        test_KConfigNodeRead ( p_data, p_size );
        test_KConfigNodeReadBool ( p_data, p_size );
        test_KConfigNodeReadI64 ( p_data, p_size );
        test_KConfigNodeReadU64 ( p_data, p_size );
        test_KConfigNodeReadF64 ( p_data, p_size );
        test_KConfigNodeReadString ( p_data, p_size );
        test_KConfigNodeWrite ( p_data, p_size );
        test_KConfigNodeWriteBool ( p_data, p_size );
        test_KConfigNodeAppend ( p_data, p_size );
        test_KConfigNodeReadAttr ( p_data, p_size );
        test_KConfigNodeWriteAttr ( p_data, p_size );
        test_KConfigNodeDropAll ( p_data, p_size );
        test_KConfigNodeDropAttr ( p_data, p_size );
        test_KConfigNodeDropChild ( p_data, p_size );
        test_KConfigNodeRenameAttr ( p_data, p_size );
        test_KConfigNodeRenameChild ( p_data, p_size );
    }

private:
    KDirectory * wd;
    KConfig * kfg;

    template <typename T> T Extract ( const uint8_t *& p_data, size_t & p_size )
    {
        T ret = * ( T * ) p_data;
        p_data += sizeof ( T );
        p_size -= sizeof ( T );
        return ret;
    }

    void test_KConfigParse( const uint8_t * p_data, size_t p_size )
    {
        // does not matter if parsing fails, we are looking for crashes and leaks
        KConfigParse ( kfg, "", string ( (const char*)p_data, p_size ) . c_str() );
    }

    void test_KConfigRead( const uint8_t * p_data, size_t p_size )
    {    // buffer layout: (size_t)offset in value, (uin16_t)buffer p_size, (char*)key
        if ( p_size >= sizeof ( size_t ) + sizeof ( uint16_t ) )
        {
            auto offset = Extract<size_t> ( p_data, p_size );
            auto bsize = Extract<uint16_t> ( p_data, p_size );
            char * buffer = new char[ bsize ];
            size_t num_read;
            size_t remaining;

            KConfigRead ( kfg, string ( (const char*)p_data, p_size ) . c_str (), offset, buffer, bsize, & num_read, & remaining );

            delete [] buffer;
        }
    }

    void test_KConfigReadBool( const uint8_t * p_data, size_t p_size )
    {    // buffer layout: (char*)key
        bool value;
        KConfigReadBool ( kfg, string ( (const char*)p_data, p_size ) . c_str (), & value );
    }

    void test_KConfigWriteBool( const uint8_t * p_data, size_t p_size )
    {   // buffer layout: bool value, (char*)key
        if ( p_size > 0 )
        {
            auto value = Extract<bool> ( p_data, p_size );
            KConfigWriteBool ( kfg, string ( (const char*)p_data, p_size ) . c_str (), value );
        }
    }

    void test_KConfigReadI64( const uint8_t * p_data, size_t p_size )
    {   // buffer layout: (char*)key
        int64_t value;
        KConfigReadI64 ( kfg, string ( (const char*)p_data, p_size ) . c_str (), & value );
    }

    void test_KConfigReadU64( const uint8_t * p_data, size_t p_size )
    {   // buffer layout: (char*)key
        uint64_t value;
        KConfigReadU64 ( kfg, string ( (const char*)p_data, p_size ) . c_str (), & value );
    }

    void test_KConfigReadF64( const uint8_t * p_data, size_t p_size )
    {   // buffer layout: (char*)key
        double value;
        KConfigReadF64 ( kfg, string ( (const char*)p_data, p_size ) . c_str (), & value );
    }

    void test_KConfigReadString( const uint8_t * p_data, size_t p_size )
    {   // buffer layout: (char*)key
        String * value = NULL;
        KConfigReadString ( kfg, string ( (const char*)p_data, p_size ) . c_str (), & value );
        free ( value );
    }

    void test_KConfigWriteString( const uint8_t * p_data, size_t p_size )
    {   // buffer layout: (char*)key, (char*)value (each 1/2 p_size long)
        string value = string ( (const char*)p_data + p_size / 2, p_size / 2 );
        KConfigWriteString ( kfg, string ( (const char*)p_data, p_size / 2 ) . c_str (), value . c_str () );
    }

    void test_KConfigWriteSString( const uint8_t * p_data, size_t p_size )
    {   // buffer layout: (char*)key, (char*)value (each 1/2 p_size long)
        String value;
        StringInit ( & value, (const char*)p_data + p_size / 2, p_size / 2, p_size / 2 );
        value . len = string_len ( value . addr, StringSize ( & value ) );
        KConfigWriteSString ( kfg, string ( (const char*)p_data, p_size / 2 ) . c_str (), & value );
    }

    void test_KConfigPrint( const uint8_t * p_data, size_t p_size )
    {   // buffer layout: (int)indent
        if ( p_size >= sizeof ( int ) )
        {
            int indent = *(int*)p_data;
            // limit to MaxIndent to avoid unreasonably long execution times (negative is OK)
            const int MaxIndent = 25;
            if ( indent < MaxIndent )
            {
                KWrtWriter wr = KOutWriterGet ();
                KOutHandlerSet ( NULL, NULL );
                //std::cout<<"indent="<<*(uint8_t*)p_data<<std::endl;
                KConfigPrint ( kfg, *(uint8_t*)p_data ); // even this slows the test down too much
                KOutHandlerSet ( wr, NULL );
            }
        }
    }

    void test_KConfigOpenNodeRead( const uint8_t * p_data, size_t p_size )
    {   // buffer layout: (char*)key

        const KConfigNode *node = NULL;
        KConfigOpenNodeRead ( kfg, & node, "%s", string ( (const char*)p_data, p_size ) . c_str () );
        KConfigNodeRelease ( node );
    }

    void test_KConfigNodeOpenNodeRead( const uint8_t * p_data, size_t p_size )
    {   // buffer layout: (char*)key, (char*)subkey
        const KConfigNode *node = NULL;
        if ( KConfigOpenNodeRead ( kfg, & node, "%s", string ( (const char*)p_data, p_size / 2 ) . c_str () ) == 0 )
        {
            const KConfigNode *subnode;
            KConfigNodeOpenNodeRead ( node, & subnode, "%s", string ( (const char*)p_data + p_size /2, p_size / 2 ) . c_str () );
            KConfigNodeRelease ( subnode );
        }
        KConfigNodeRelease ( node );
    }

    void test_KConfigOpenNodeUpdate( const uint8_t * p_data, size_t p_size )
    {   // buffer layout: (char*)key
        KConfigNode *node = NULL;
        KConfigOpenNodeUpdate ( kfg, & node, "%s", string ( (const char*)p_data, p_size / 2 ) . c_str () );
        KConfigNodeRelease ( node );
    }

    void test_KConfigNodeOpenNodeUpdate( const uint8_t * p_data, size_t p_size )
    {   // buffer layout: (char*)key, (char*)subkey
        KConfigNode *node = NULL;
        if ( KConfigOpenNodeUpdate ( kfg, & node, "%s", string ( (const char*)p_data, p_size / 2 ) . c_str () ) == 0 )
        {
            KConfigNode *subnode;
            KConfigNodeOpenNodeUpdate ( node, & subnode, "%s", string ( (const char*)p_data + p_size  /2, p_size / 2 ) . c_str () );
            KConfigNodeRelease ( subnode );
        }
        KConfigNodeRelease ( node );
    }

    void test_KConfigNodeRead( const uint8_t * p_data, size_t p_size )
    {   // buffer layout: (size_t)offset in value, (uin16_t)buffer p_size, (char*)key
        if ( p_size >= sizeof ( size_t ) + sizeof ( uint16_t ) )
        {
            auto offset = Extract<size_t> ( p_data, p_size );
            auto bsize = Extract<uint16_t> ( p_data, p_size );
            const KConfigNode *node = NULL;
            if ( KConfigOpenNodeRead ( kfg, & node, "%s", string ( (const char*)p_data, p_size ) . c_str () ) == 0 )
            {
                char * buffer = new char[ bsize ];
                size_t num_read;
                size_t remaining;

                KConfigNodeRead ( node, offset, buffer, bsize, & num_read, & remaining );

                delete [] buffer;
                KConfigNodeRelease ( node );
            }
        }
    }

    void test_KConfigNodeReadBool( const uint8_t * p_data, size_t p_size )
    {   // buffer layout: (char*)key
        const KConfigNode *node = NULL;
        if ( KConfigOpenNodeRead ( kfg, & node, "%s", string ( (const char*)p_data, p_size ) . c_str () ) == 0 )
        {
            bool value;
            KConfigNodeReadBool ( node, & value );
            KConfigNodeRelease ( node );
        }
    }

    void test_KConfigNodeReadI64( const uint8_t * p_data, size_t p_size )
    {   // buffer layout: (char*)key
        const KConfigNode *node = NULL;
        if ( KConfigOpenNodeRead ( kfg, & node, "%s", string ( (const char*)p_data, p_size ) . c_str () ) == 0 )
        {
            int64_t value;
            KConfigNodeReadI64 ( node, & value );
            KConfigNodeRelease ( node );
        }
    }

    void test_KConfigNodeReadU64( const uint8_t * p_data, size_t p_size )
    {   // buffer layout: (char*)key
        const KConfigNode *node = NULL;
        if ( KConfigOpenNodeRead ( kfg, & node, "%s", string ( (const char*)p_data, p_size ) . c_str () ) == 0 )
        {
            uint64_t value;
            KConfigNodeReadU64 ( node, & value );
            KConfigNodeRelease ( node );
        }
    }

    void test_KConfigNodeReadF64( const uint8_t * p_data, size_t p_size )
    {   // buffer layout: (char*)key
        const KConfigNode *node = NULL;
        if ( KConfigOpenNodeRead ( kfg, & node, "%s", string ( (const char*)p_data, p_size ) . c_str () ) == 0 )
        {
            double value;
            KConfigNodeReadF64 ( node, & value );
            KConfigNodeRelease ( node );
        }
    }

    void test_KConfigNodeReadString( const uint8_t * p_data, size_t p_size )
    {   // buffer layout: (char*)key
        const KConfigNode *node = NULL;
        if ( KConfigOpenNodeRead ( kfg, & node, "%s", string ( (const char*)p_data, p_size ) . c_str () ) == 0 )
        {
            String * value = NULL;
            KConfigNodeReadString ( node, & value );
            KConfigNodeRelease ( node );
            free ( value );
        }
    }

    void test_KConfigNodeListChildren( const uint8_t * p_data, size_t p_size )
    {   // buffer layout: (char*)key
        const KConfigNode *node = NULL;
        if ( KConfigOpenNodeRead ( kfg, & node, "%s", string ( (const char*)p_data, p_size ) . c_str () ) == 0 )
        {
            struct KNamelist *names = NULL;
            KConfigNodeListChildren ( node, & names );
            KNamelistRelease ( names );
        }
    }

    void test_KConfigNodeWrite( const uint8_t * p_data, size_t p_size )
    {   // buffer layout: (char*)key, (uin16_t)buffer p_size (each 1/2 Size)
        KConfigNode *node = NULL;
        if ( KConfigOpenNodeUpdate ( kfg, & node, "%s", string ( (const char*)p_data, p_size / 2 ) . c_str () ) == 0 )
        {
            KConfigNodeWrite ( node, (const char*)p_data + p_size / 2, p_size / 2 );
            KConfigNodeRelease ( node );
        }
    }

    void test_KConfigNodeWriteBool( const uint8_t * p_data, size_t p_size )
    {   // buffer layout: uint8_t value, (char*)key
        if ( p_size >= sizeof ( uint8_t ) )
        {
            auto value = Extract<bool> ( p_data, p_size );
            KConfigNode *node = NULL;
            if ( KConfigOpenNodeUpdate ( kfg, & node, "%s", string ( (const char*)p_data, p_size ) . c_str () ) == 0 )
            {
                KConfigNodeWriteBool ( node, value );
                KConfigNodeRelease ( node );
            }
        }
    }

    void test_KConfigNodeAppend( const uint8_t * p_data, size_t p_size )
    {   // buffer layout: (char*)key, (uin16_t)buffer (each 1/2 p_ize)
        KConfigNode *node = NULL;
        if ( KConfigOpenNodeUpdate ( kfg, & node, "%s", string ( (const char*)p_data, p_size / 2 ) . c_str () ) == 0 )
        {
            KConfigNodeAppend ( node, (const char*)p_data + p_size / 2, p_size / 2 );
            KConfigNodeRelease ( node );
        }
    }

    void test_KConfigNodeReadAttr( const uint8_t * p_data, size_t p_size )
    {   // buffer layout: (uin16_t)buffer p_size, (char*)key, (char*)attrname
        if ( p_size >= sizeof ( uint16_t ) )
        {
            auto bsize = Extract<uint16_t> ( p_data, p_size );

            const KConfigNode *node = NULL;
            if ( KConfigOpenNodeRead ( kfg, & node, "%s", string ( (const char*)p_data, p_size / 2 ) . c_str () ) == 0 )
            {
                char * buffer = new char[ bsize ];
                size_t num_read;
                KConfigNodeReadAttr ( node, string ( (const char*)p_data + p_size / 2, p_size / 2 ) . c_str (), buffer, bsize, & num_read );

                delete [] buffer;
                KConfigNodeRelease ( node );
            }
        }
    }

    void test_KConfigNodeWriteAttr( const uint8_t * p_data, size_t p_size )
    {   // buffer layout: (char*)key, (char*)name, (char*)value, each one 1/3 of Size
        KConfigNode *node = NULL;
        if ( KConfigOpenNodeUpdate ( kfg, & node, "%s", string ( (const char*)p_data, p_size / 3 ) . c_str () ) == 0 )
        {
            KConfigNodeWriteAttr ( node,
                                   string ( (const char*)p_data + p_size / 3, p_size / 3 ) . c_str (),
                                   string ( (const char*)p_data + 2 * p_size / 3, p_size / 3 ) . c_str () );
            KConfigNodeRelease ( node );
        }
    }

    void test_KConfigNodeDropAll( const uint8_t * p_data, size_t p_size )
    {   // buffer layout: (char*)key
        KConfigNode *node = NULL;
        if ( KConfigOpenNodeUpdate ( kfg, & node, "%s", string ( (const char*)p_data, p_size / 3 ) . c_str () ) == 0 )
        {
            KConfigNodeDropAll ( node );
            KConfigNodeRelease ( node );
        }
    }

    void test_KConfigNodeDropAttr( const uint8_t * p_data, size_t p_size )
    {   // buffer layout: (char*)key, (char*)attr, each 1/2 p_size
        KConfigNode *node = NULL;
        if ( KConfigOpenNodeUpdate ( kfg, & node, "%s", string ( (const char*)p_data, p_size / 2 ) . c_str () ) == 0 )
        {
            KConfigNodeDropAttr ( node, string ( (const char*)p_data + p_size / 2, p_size / 2 ) . c_str () );
            KConfigNodeRelease ( node );
        }
    }

    void test_KConfigNodeDropChild( const uint8_t * p_data, size_t p_size )
    {   // buffer layout: (char*)key, (char*)subkey, each 1/2 p_size
        KConfigNode *node = NULL;
        if ( KConfigOpenNodeUpdate ( kfg, & node, "%s", string ( (const char*)p_data, p_size / 2 ) . c_str () ) == 0 )
        {
            KConfigNodeDropChild ( node, "%s", string ( (const char*)p_data + p_size / 2, p_size / 2 ) . c_str () );
            KConfigNodeRelease ( node );
        }
    }

    void test_KConfigNodeRenameAttr( const uint8_t * p_data, size_t p_size )
    {   // buffer layout: (char*)key, (char*)attr, (char*)newattr, each 1/3 p_size
        KConfigNode *node = NULL;
        if ( KConfigOpenNodeUpdate ( kfg, & node, "%s", string ( (const char*)p_data, p_size / 3 ) . c_str () ) == 0 )
        {
            KConfigNodeRenameAttr ( node,
                                   string ( (const char*)p_data + p_size / 3, p_size / 3 ) . c_str (),
                                   string ( (const char*)p_data + 2 * p_size / 3, p_size / 3 ) . c_str () );
            KConfigNodeRelease ( node );
        }
    }

    void test_KConfigNodeRenameChild( const uint8_t * p_data, size_t p_size )
    {   // buffer layout: (char*)key, (char*)subkey, (char*)newsubkey, each 1/3 p_size
        KConfigNode *node = NULL;
        if ( KConfigOpenNodeUpdate ( kfg, & node, "%s", string ( (const char*)p_data, p_size / 3 ) . c_str () ) == 0 )
        {
            KConfigNodeRenameChild ( node,
                                     string ( (const char*)p_data + p_size / 3, p_size / 3 ) . c_str (),
                                     string ( (const char*)p_data + 2 * p_size / 3, p_size / 3 ) . c_str () );
            KConfigNodeRelease ( node );
        }
    }

};

extern "C" int
LLVMFuzzerTestOneInput ( const uint8_t * const Data, size_t const Size )
{
    KfgFuzzer().Run(Data, Size);
    return 0;  // Non-zero return values are reserved for future use.
}

