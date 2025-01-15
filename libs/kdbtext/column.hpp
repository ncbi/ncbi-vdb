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

#include "../libs/kdb/column-base.h"

#include <klib/json.h>
#include <klib/rc.h>
#include <klib/data-buffer.h>

#include <string>
#include <map>
#include <memory>

typedef struct KTextColumn KTextColumn;
struct KTextColumn
{
    KColumn dad;
};

namespace KDBText
{
    class Table;
    class Manager;
    class Metadata;
    class ColumnBlob;

    class Column : public KTextColumn
    {
    public:
        static void addRef( const Column* );
        static void release( const Column *);

    public:
        Column( const KJsonObject * p_json, const Manager * mgr = nullptr, const Table * parent = nullptr );
        ~Column();

        rc_t inflate( char * error, size_t error_size );

        const Manager * getManager() const { return m_mgr; }
        const Table * getParent() const { return m_parent; }
        const std::string & getName() const { return m_name; }

        typedef std::pair< int64_t, uint64_t > IdRange;
        IdRange idRange() const; // { first, last - first + 1 }; { 0, 0 } if empty

        int64_t findFirst( int64_t row ) const;

        const Metadata * openMetadata() const { return m_meta; }

        const ColumnBlob * openBlob( int64_t id ) const;

    private:
        const Manager * m_mgr = nullptr;
        const Table * m_parent = nullptr;

        const KJsonObject * m_json = nullptr;
        std::string m_name;
        std::string m_type;

        typedef std::map< IdRange, std::shared_ptr<ColumnBlob> > BlobMap;
        BlobMap::const_iterator findBlob( int64_t row ) const;
        BlobMap m_data;

        const Metadata * m_meta = nullptr;
    };
}
