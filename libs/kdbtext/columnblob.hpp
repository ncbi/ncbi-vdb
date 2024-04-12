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

#pragma once

#include "../libs/kdb/columnblob-base.h"

#include <klib/data-buffer.h>

#include <utility>
#include <vector>

struct PageMap;

typedef struct KTextBlob KTextBlob;
struct KTextBlob
{
    KColumnBlob dad;
};

namespace KDBText
{
    class ColumnBlob : public KTextBlob
    {
    public:
        static void addRef( const ColumnBlob* );
        static void release( const ColumnBlob *);

    public:
        ColumnBlob(){}
        ColumnBlob( int64_t startId );
        ~ColumnBlob();

        rc_t appendRow( const void * data, size_t size, uint32_t count = 1 );
        rc_t serialize( KDataBuffer & buf ) const;

        std::pair< int64_t, uint32_t > getIdRange() const{ return std::make_pair( m_startId, m_count ); }

    private:
        int64_t m_startId = 0;
        uint32_t m_count = 0;
        std::vector<KDataBuffer> m_rows;
        struct PageMap * m_pm = nullptr;
        KDataBuffer m_data;
    };
}
