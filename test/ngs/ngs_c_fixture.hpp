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

#ifndef _h_ngs_c_fixture_
#define _h_ngs_c_fixture_

/**
* Unit tests for NGS C interface, common definitions
*/

#include <sysalloc.h>

#include <string>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include <ktst/unit_test.hpp>

#include <kfc/xcdefs.h>
#include <kfc/except.h>

#include <kfc/ctx.h>
#include <kfc/rsrc.h>

#include "NGS_ReadCollection.h"
#include "NGS_Read.h"
#include "NGS_ReadGroup.h"
#include "NGS_Reference.h"
#include "NGS_Alignment.h"
#include "NGS_Statistics.h"

#include <NGS_String.h>

#define SHOW_UNIMPLEMENTED 0

#define ENTRY \
    HYBRID_FUNC_ENTRY ( rcSRA, rcRow, rcAccessing ); \
    m_ctx = ctx; \

#define ENTRY_ACC(acc) \
    ENTRY; \
    m_acc = acc; \
    m_coll = NGS_ReadCollectionMake ( ctx, acc );

#define ENTRY_GET_READ(acc, readNo) \
    ENTRY_ACC(acc); \
    GetRead(readNo);

#define ENTRY_GET_REF(acc,ref) \
    ENTRY_ACC(acc); \
    GetReference ( ref );

#define EXIT \
    REQUIRE ( ! FAILED () ); \
    Release()

////// additional REQUIRE macros

#define REQUIRE_FAILED() ( REQUIRE ( FAILED () ), CLEAR() )

// use for NGS calls returning NGS_String*
#define REQUIRE_STRING(exp, call) \
{\
    string str = toString ( call, ctx, true );\
    REQUIRE ( ! FAILED () );\
    REQUIRE_EQ ( string(exp), str);\
}

//////

std :: string
toString ( const NGS_String* str, ctx_t ctx, bool release_source = false )
{
    if ( str == 0 )
    {
        throw std :: logic_error ( "toString ( NULL ) called" );
    }
    std :: string ret = std::string ( NGS_StringData ( str, ctx ), NGS_StringSize ( str, ctx ) );
    if ( release_source )
    {
        NGS_StringRelease ( str, ctx );
    }
    return ret;
}

class NGS_C_Fixture
{
public:
    NGS_C_Fixture()
    : m_ctx(0), m_coll(0), m_read(0), m_readGroup (0), m_ref (0)
    {
    }
    ~NGS_C_Fixture()
    {
    }

    virtual void Release()
    {
        if (m_ctx != 0)
        {
            if (m_coll != 0)
            {
                NGS_RefcountRelease ( ( NGS_Refcount* ) m_coll, m_ctx );
            }
            if (m_read != 0)
            {
                NGS_ReadRelease ( m_read, m_ctx );
            }
            if (m_readGroup != 0)
            {
                NGS_ReadGroupRelease ( m_readGroup, m_ctx );
            }
            if (m_ref != 0)
            {
                NGS_ReferenceRelease ( m_ref, m_ctx );
            }
            m_ctx = 0; // a pointer into the caller's local memory
        }
    }

    std::string ReadId(int64_t id) const
    {
        std::ostringstream s;
        s << m_acc << ".R." << id;
        return s . str ();
    }
    void GetRead(const std::string & id)
    {
        m_read = NGS_ReadCollectionGetRead ( m_coll, m_ctx, id.c_str() );
        if (m_read != 0)
        {   // initialize the fragment iterator
            NGS_FragmentIteratorNext ( (NGS_Fragment*)m_read, m_ctx );
        }
    }
    void GetRead(int64_t id)
    {
        GetRead ( ReadId ( id ) );
    }

    void GetReference(const char* name)
    {
        m_ref = NGS_ReadCollectionGetReference ( m_coll, m_ctx, name );
        if ( m_ctx -> rc != 0 || m_ref == 0 )
            throw std :: logic_error ( "GetReference() failed" );
    }

    const KCtx*         m_ctx;  // points into the test case's local memory
    std::string         m_acc;
    NGS_ReadCollection* m_coll;
    NGS_Read*           m_read;
    NGS_ReadGroup*      m_readGroup;
    NGS_Reference*      m_ref;
};

#endif

