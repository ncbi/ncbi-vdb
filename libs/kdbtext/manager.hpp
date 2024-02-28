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

#include "api-manager.hpp"

#include <string>

#include "database.hpp"
#include "table.hpp"
#include "path.hpp"

namespace KDBText
{
    class Manager : public KDBManager
    {
    public:
        static void addRef( const Manager* );
        static void release( const Manager *);

    public:
        Manager( const KDBManager_vt& );
        ~Manager();

        rc_t parse( const char * input, char * error, size_t error_size );

        const Database * getRootDatabase() const;
        const Table * getRootTable() const;

        int pathType( const Path & path ) const;

        bool exists( uint32_t requested, const Path & ) const;

        rc_t writable( const Path & )  const;

        rc_t openDatabase( const Path &, const Database *& ) const;
        rc_t openTable( const Path &, const Table *& ) const;

    private:
        KJsonValue * m_root = nullptr;
        bool m_isDb;
    };
}

