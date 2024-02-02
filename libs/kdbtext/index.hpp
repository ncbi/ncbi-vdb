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

#include "../libs/kdb/index-base.h"

#include <klib/json.h>
#include <klib/rc.h>

#include <string>
#include <map>
#include <set>

typedef struct KTextIndex KTextIndex;
struct KTextIndex
{
    KIndex dad;
};

namespace KDBText
{
    class Table;

    class Index : public KTextIndex
    {
    public:
        static void addRef( const Index* );
        static void release( const Index *);

    public:
        Index( const KJsonObject * p_json, const Table * parent = nullptr );
        ~Index();

        rc_t inflate( char * error, size_t error_size );

        const std::string & getName() const { return m_name; }

        typedef std::map< std::string, std::pair<int64_t,uint64_t> > TextData;
        const TextData & getTextData() const { return m_textData; }

        struct IntDataNode
        {
            uint64_t key;
            uint64_t key_size;
            int64_t id_start;
            uint64_t id_count;
            bool operator < ( const IntDataNode& that ) const
            {
                return this->key < that.key || ( this->key == that.key && this->key_size < that.key_size );
            };
        };

        typedef std::set< IntDataNode > IntData;
        const IntData & getIntData() const { return m_intData; }

    private:
        const KJsonObject * m_json = nullptr;
        std::string m_name;

        const Table * m_parent = nullptr;

        TextData m_textData;
        IntData m_intData;
    };
}
