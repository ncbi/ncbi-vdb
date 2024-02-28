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

#include "../libs/kdb/meta-base.h"

#include <klib/json.h>
#include <klib/rc.h>

#include <string>

typedef struct KTextMetadata KTextMetadata;
struct KTextMetadata
{
    KMetadata dad;
};

namespace KDBText
{
    class Metanode;

    class Metadata : public KTextMetadata
    {
    public:
        static void addRef( const Metadata* );
        static void release( const Metadata *);

    public:
        Metadata( const KJsonObject * p_json );
        ~Metadata();

        rc_t inflate( char * error, size_t error_size );

        const std::string & getName() const { return m_name; }

        uint32_t getRevision() const { return m_revision; }

        const Metanode * getRoot() const { return m_root; }

    private:
        const KJsonObject * m_json = nullptr;
        std::string m_name;
        uint32_t m_revision = 0;
        Metanode * m_root = nullptr;
    };
}
