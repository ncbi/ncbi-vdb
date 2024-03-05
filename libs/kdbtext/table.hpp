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

#include "../libs/kdb/table-base.h"

#include "path.hpp"
#include "index.hpp"

#include <klib/json.h>
#include <klib/rc.h>

#include <string>
#include <map>

typedef struct KTextTable KTextTable;
struct KTextTable
{
    KTable dad;
};

namespace KDBText
{
    class Metadata;
    class Manager;
    class Database;
    class Column;

    class Table : public KTextTable
    {
    public:
        static void addRef( const Table* );
        static void release( const Table *);

    public:
        Table( const KJsonObject * p_json, const Manager * mgr = nullptr, const Database * parent = nullptr );
        ~Table();

        rc_t inflate( char * error, size_t error_size );

        const Manager * getManager() const { return m_mgr; }
        const Database * getParent() const { return m_parent; }
        const std::string & getName() const { return m_name; }

        bool hasColumn( const std::string& name ) const { return m_columns.find(name) != m_columns.end(); }
        const Column * openColumn( const std::string& name ) const;

        bool hasIndex( const std::string& name ) const { return m_indexes.find(name) != m_indexes.end(); }
        const Index * openIndex( const std::string& name ) const;

        int pathType( Path & ) const;
        bool exists( uint32_t requested, Path & p_path ) const;

        const Metadata * openMetadata() const { return m_meta; }

        // verified Jsons:
        typedef std::map< std::string, const KJsonObject * > Subobjects;

        const Subobjects& getColumns() const { return m_columns; }
        const Subobjects& getIndexes() const { return m_indexes; }

    private:
        const Manager * m_mgr = nullptr;
        const Database * m_parent = nullptr;

        const KJsonObject * m_json = nullptr;
        std::string m_name;

        // verified Jsons
        Subobjects m_columns;
        Subobjects m_indexes;

        const Metadata * m_meta = nullptr;
    };
}
