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

typedef struct KTextDatabase KTextDatabase;
#define KDATABASE_IMPL KTextDatabase
#include "../libs/kdb/database-base.h"

struct KTextDatabase
{
    KDatabaseBase dad;
};

#include "path.hpp"

#include <klib/json.h>
#include <klib/rc.h>

#include <string>
#include <vector>
#include <map>

namespace KDBText
{
    class Manager;
    class Table;
    class Metadata;

    class Database : public KTextDatabase
    {
    public:
        static void addRef( const Database* );
        static void release( const Database *);

    public:
        Database( const KJsonObject * json, const Manager * mgr = nullptr, const Database * parent = nullptr );
        ~Database();

        rc_t inflate( char * error, size_t error_size );

        const Manager * getManager() const { return m_mgr; }
        const Database * getParent() const { return m_parent; }
        const std::string & getName() const { return m_name; }

        const Database * openDatabase( Path & path ) const;
        const Database * openSubDatabase( const std::string & name ) const;

        const Table *openTable( Path & path ) const;
        const Table *openTable( const std::string & name ) const;

        int pathType( Path & ) const;
        bool exists( uint32_t requested, Path & p_path ) const;

        const Metadata * openMetadata() const;

        // verified Jsons:
        typedef std::map< std::string, const KJsonObject * > Subobjects;

        const Subobjects& getDatabases() const { return m_subdbs; }
        const Subobjects& getTables() const { return m_tables; }

    private:
        const Manager * m_mgr = nullptr;
        const Database * m_parent = nullptr;

        const KJsonObject * m_json = nullptr;
        std::string m_name;

        Subobjects m_subdbs;
        Subobjects m_tables;
        const Metadata * m_meta = nullptr;
    };
}
