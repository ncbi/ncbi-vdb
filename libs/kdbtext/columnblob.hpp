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

#include "../libs/kdb/ColumnBlob.hpp"

#include <klib/data-buffer.h>

#include <utility>
#include <vector>

struct PageMap;
struct KJsonValue;

namespace KDBText
{
    class ColumnBlob : public KColumnBlobBase
    {
    protected:
        virtual rc_t whack();

    public:
        /* Public read-side API */
        virtual rc_t addRef();
        virtual rc_t release();
        virtual rc_t read ( size_t offset, void *buffer, size_t bsize, size_t *num_read, size_t *remaining ) const;
        virtual rc_t readAll ( struct KDataBuffer * buffer, KColumnBlobCSData * opt_cs_data, size_t cs_data_size ) const;
        virtual rc_t validate() const;
        virtual rc_t validateBuffer ( struct KDataBuffer const * buffer, const KColumnBlobCSData * cs_data, size_t cs_data_size ) const;
        virtual rc_t idRange ( int64_t *first, uint32_t *count ) const;

    public:
        ColumnBlob( const KJsonValue * ); // ascii values
        ColumnBlob( const KJsonValue *, uint8_t intSizeBits ); // fixed size integer values
        virtual ~ColumnBlob();

        rc_t inflate( char * p_error, size_t p_error_size, int8_t intSizeBits = 0 );

        const KDataBuffer & getData() const { return m_data; }
        const PageMap & getPageMap() const { return * m_pm; }

        std::pair< int64_t, uint32_t > getIdRange() const{ return std::make_pair( m_startId, m_count ); }

        rc_t serialize( KDataBuffer & buf ) const;

    private:
        rc_t appendRow( const void * data, size_t sizeInElems, uint32_t repeatCount = 1 );

        const KJsonValue * m_json = nullptr;

        int64_t m_startId = 0;
        uint32_t m_count = 0;
        PageMap * m_pm = nullptr;
        KDataBuffer m_data;
    };
}
