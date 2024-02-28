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

#include "../libs/kdb/metanode-base.h"

#include <klib/json.h>
#include <klib/rc.h>

#include <string>
#include <map>
#include <vector>

typedef struct KTextMetanode KTextMetanode;
struct KTextMetanode
{
    KMDataNode dad;
};

namespace KDBText
{
    class Path;

    class Metanode : public KTextMetanode
    {
    public:
        static void addRef( const Metanode* );
        static void release( const Metanode *);

    public:
        Metanode( const KJsonObject * p_json );
        ~Metanode();

        rc_t inflate( char * error, size_t error_size );

        const std::string & getName() const { return m_name; }
        const std::string & getValue() const { return m_value; }

        typedef std::map<std::string, std::string> Attributes;
        const Attributes& getAttributes() const { return m_attrs; }

        typedef std::vector<const Metanode*> Children;
        const Children& getChildren() const { return m_children; }

        const Metanode * getNode( Path & ) const; // destroys path

        bool operator == ( const Metanode& other ) const;

    private:
        const KJsonObject * m_json = nullptr;
        std::string m_name;
        std::string m_value;
        Attributes m_attrs;
        Children m_children;
    };
}
