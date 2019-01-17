/*==============================================================================
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

#ifndef _hpp_jwt_map_
#define _hpp_jwt_map_

//
// VDB BSTree based imitation of std::map
//

#include <klib/container.h>

#include "jwt-pair.hpp"

namespace ncbi
{
    template < typename Key, typename Value > class JwtMap
    {
    private:
        struct Node;

    public:
        class iterator
        {
        public:
            iterator(Node *);

            bool operator == (const iterator&) const;
            bool operator != (const iterator&) const;

            iterator & operator ++ ();

            JwtPair<const Key,Value>* operator -> () const;
            Node* operator * () const { return it; }

        private:
            Node * it;
        };
        class const_iterator
        {
        public:
            const_iterator(const Node *);
            const_iterator(const iterator&);

            bool operator == (const const_iterator&) const;
            bool operator != (const const_iterator&) const;

            const_iterator & operator ++ ();

            const JwtPair<const Key, Value>* operator -> () const;
            const Node* operator * () const { return it; }

        private:
            const Node * it;
        };

    public:
        JwtMap();
        ~JwtMap();

        bool empty() const;
        size_t size() const;

        iterator        find (const Key& k);
        const_iterator  find (const Key& k) const;

        iterator        begin() noexcept;
        const_iterator  begin() const noexcept;
        const_iterator  cbegin() const noexcept;

        iterator        end() noexcept;
        const_iterator  end() const noexcept;
        const_iterator  cend() const noexcept;

        template <class... Args> JwtPair<iterator,bool> emplace (Args&&... args);

        iterator erase (const_iterator position);

    private:
        JwtMap(const JwtMap&);
        JwtMap& operator=(const JwtMap);

        struct BSTree m_tree;
    };
}

#endif /* _hpp_jwt_map_ */
