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

#ifndef _hpp_jwt_map_impl_
#define _hpp_jwt_map_impl_

//
// VDB BSTree based imitation of std::map
//

#include <jwt/jwt-map.hpp>

namespace ncbi
{

////////////////// JwtMap :: Node

    template < typename Key, typename Value >
    struct JwtMap < Key, Value > :: Node
    {
        typedef JwtPair < Key, Value > BodyType;

        Node(Key k, Value v)
        :   body ( k, v )
        {
            node . par = nullptr;
            node . child [0]  = nullptr;
            node . child [1]  = nullptr;
        }

        static int64_t CC sort ( const BSTNode *item, const BSTNode *n )
        {
            const Node * self = ( const Node * ) item;
            return cmp ( & self -> body . first, n );
        }
        static int64_t CC cmp ( const void *item, const BSTNode *n )
        {
            const Key * key = ( const Key * ) item;
            const Node * that = ( const Node * ) n;

            return key -> compare ( that -> body . first );
        }
        static void CC whack ( BSTNode *n, void *data )
        {
            Node * self = ( Node * ) n;
            self -> body . ~ BodyType ();
            free ( self );
        }
        static void CC count ( BSTNode *n, void *data )
        {
            size_t * count = (size_t*) data;
            ++ * count;
        }

        BSTNode     node;
        BodyType    body;
    };

////////////////// JwtMap

    template < typename Key, typename Value >
    JwtMap < Key, Value > :: JwtMap()
    {
        BSTreeInit ( & m_tree );
    }

    template < typename Key, typename Value >
    JwtMap < Key, Value > :: ~ JwtMap()
    {
        BSTreeWhack ( & m_tree, Node::whack, nullptr );
    }

    template < typename Key, typename Value >
    bool
    JwtMap < Key, Value > :: empty() const
    {
        return m_tree . root == nullptr;
    }

    template < typename Key, typename Value >
    size_t
    JwtMap < Key, Value > :: size() const
    {
        size_t ret = 0;
        BSTreeForEach ( & m_tree, false, Node::count, & ret );
        return ret;
    }

    template < typename Key, typename Value >
    typename JwtMap < Key, Value > :: iterator
    JwtMap < Key, Value > :: find (const Key& k)
    {
        return iterator ( ( Node * ) BSTreeFind ( & m_tree, & k, Node::cmp ) );
    }

    template < typename Key, typename Value >
    typename JwtMap < Key, Value > :: const_iterator
    JwtMap < Key, Value > :: find (const Key& k) const
    {
        return const_iterator ( ( const Node * ) BSTreeFind ( & m_tree, & k, Node::cmp ) );
    }

    template < typename Key, typename Value >
    typename JwtMap < Key, Value > :: iterator
    JwtMap < Key, Value > :: begin() noexcept
    {
        return iterator ( ( Node * ) BSTreeFirst ( & m_tree ) );
    }
    template < typename Key, typename Value >
    typename JwtMap < Key, Value > :: const_iterator
    JwtMap < Key, Value > :: begin() const noexcept
    {
        return const_iterator ( ( Node * ) BSTreeFirst ( & m_tree ) );
    }
    template < typename Key, typename Value >
    typename JwtMap < Key, Value > :: const_iterator
    JwtMap < Key, Value > :: cbegin() const noexcept
    {
        return const_iterator ( ( Node * ) BSTreeFirst ( & m_tree ) );
    }

    template < typename Key, typename Value >
    typename JwtMap < Key, Value > :: iterator
    JwtMap < Key, Value > :: end() noexcept
    {
        return iterator ( nullptr );
    }
    template < typename Key, typename Value >
    typename JwtMap < Key, Value > :: const_iterator
    JwtMap < Key, Value > :: end() const noexcept
    {
        return const_iterator ( nullptr );
    }
    template < typename Key, typename Value >
    typename JwtMap < Key, Value > :: const_iterator
    JwtMap < Key, Value > :: cend() const noexcept
    {
        return const_iterator ( nullptr );
    }

    template < typename Key, typename Value >
    template <class... Args>
    JwtPair < typename JwtMap < Key, Value > :: iterator, bool >
    JwtMap < Key, Value > :: emplace (Args&&... args)
    {
        Node * to_insert = ( Node * ) calloc( 1, sizeof ( Node ) );
        //TODO: throw if to_insert == nullptr
        * to_insert = Node ( args... );
        BSTNode * exist = nullptr;
        rc_t rc = BSTreeInsertUnique ( & m_tree, & to_insert -> node, & exist, Node::sort );
        if ( rc != 0 && exist != nullptr )
        {
            to_insert -> ~Node();
            free ( to_insert );
            return JwtPair < iterator, bool > ( iterator ( (Node*)exist ), false);
        }
        //TODO: throw if rc != 0
        return JwtPair < iterator, bool > ( to_insert, true );
    }

    template < typename Key, typename Value >
    typename JwtMap < Key, Value > :: iterator
    JwtMap < Key, Value > :: erase (const_iterator position)
    {
        BSTNode * elem = ( BSTNode * ) * position;
        Node * next = ( Node * ) BSTNodeNext ( elem );
        if ( ! BSTreeUnlink ( & m_tree, elem ) )
        {   //NB: undefined behavior
            return end();
        }
        ( * position ) -> ~Node();
        free ( elem );
        return iterator ( next );
    }


////////////////// JwtMap :: iterator

    template < typename Key, typename Value > JwtMap < Key, Value > :: iterator ::
    iterator (Node * p_it)
    : it ( p_it )
    {
    };

    template < typename Key, typename Value >
    bool
    JwtMap < Key, Value > :: iterator :: operator == (const iterator& p_it) const
    {
        return it == p_it.it;
    }

    template < typename Key, typename Value >
    bool
    JwtMap < Key, Value > :: iterator :: operator != (const iterator& p_it) const
    {
        return it != p_it.it;
    }

    template < typename Key, typename Value >
    typename JwtPair<const Key,Value> *
    JwtMap < Key, Value > :: iterator :: operator -> () const
    {
        if ( it == nullptr )
        {
            return nullptr;
        }
        return ( JwtPair<const Key,Value> * ) & it -> body;
    }

    template < typename Key, typename Value >
    typename JwtMap < Key, Value > :: iterator &
    JwtMap < Key, Value > :: iterator :: operator ++ ()
    {
        assert ( it != nullptr );
        it = ( Node * ) BSTNodeNext ( & it -> node );
        return *this;
    }

////////////////// JwtMap :: const_iterator

    template < typename Key, typename Value > JwtMap < Key, Value > :: const_iterator ::
    const_iterator (const Node * p_it)
    : it ( p_it )
    {
    };
    template < typename Key, typename Value > JwtMap < Key, Value > :: const_iterator ::
    const_iterator (const iterator& p_it)
    : it ( * p_it )
    {
    };

    template < typename Key, typename Value >
    bool
    JwtMap < Key, Value > :: const_iterator :: operator == (const const_iterator& p_it) const
    {
        return it == p_it.it;
    }
    template < typename Key, typename Value >
    bool
    JwtMap < Key, Value > :: const_iterator :: operator != (const const_iterator& p_it) const
    {
        return it != p_it.it;
    }

    template < typename Key, typename Value >
    const typename JwtPair<const Key,Value> *
    JwtMap < Key, Value > :: const_iterator :: operator -> () const
    {
        if ( it == nullptr )
        {
            return nullptr;
        }
        return ( JwtPair<const Key,Value> * ) & it -> body;
    }

    template < typename Key, typename Value >
    typename JwtMap < Key, Value > :: const_iterator &
    JwtMap < Key, Value > :: const_iterator :: operator ++ ()
    {
        assert ( it != nullptr );
        it = ( Node * ) BSTNodeNext ( & it -> node );
        return *this;
    }
}

#endif /* _hpp_jwt_map_ */
