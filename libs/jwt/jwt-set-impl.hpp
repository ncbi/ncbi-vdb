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

#ifndef _hpp_jwt_set_impl_
#define _hpp_jwt_set_impl_

//
// VDB BSTree based imitation of std::set
//

#include <jwt/jwt-set.hpp>

namespace ncbi
{

////////////////// JwtSet :: Node

    template < class T >
    struct JwtSet < T > :: Node
    {
        Node(const T& t)
        : body ( t )
        {
            node . par = nullptr;
            node . child [0]  = nullptr;
            node . child [1]  = nullptr;
        }

        static int64_t CC sort ( const BSTNode *item, const BSTNode *n )
        {
            const Node * self = ( const Node * ) item;
            return cmp ( & self -> body, n );
        }
        static int64_t CC cmp ( const void *item, const BSTNode *n )
        {
            const T * self = ( const T * ) item;
            const Node * that = ( const Node * ) n;

            return self -> compare ( that -> body );
        }
        static void CC whack ( BSTNode *n, void *data )
        {
            Node * self = ( Node * ) n;
            self -> body . ~ T ();
            free ( self );
        }

        BSTNode node;
        T       body;
    };

////////////////// JwtSet

    template < class T > JwtSet < T > :: JwtSet()
    {
        BSTreeInit ( & m_tree );
    }

    template < class T > JwtSet < T > :: ~JwtSet()
    {
        BSTreeWhack ( & m_tree, Node::whack, nullptr );
    }

    template < class T >
    typename JwtSet < T > :: iterator
    JwtSet < T > :: find (const T& v)
    {
        return iterator ( ( Node * ) BSTreeFind ( & m_tree, & v, Node::cmp ) );
    }

    template < class T >
    typename JwtSet < T > :: iterator
    JwtSet < T > :: end() noexcept
    {
        return iterator ( nullptr );
    }

    template < class T >
    template <class... Args>
    JwtPair < typename JwtSet < T > :: iterator, bool >
    JwtSet < T > :: emplace (Args&&... args)
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

////////////////// JwtSet :: iterator

    template < class T > JwtSet < T > :: iterator ::
    iterator (Node * p_it)
    : it ( p_it )
    {
    };

    template < class T >
    bool
    JwtSet < T > :: iterator :: operator == (const iterator& p_it) const
    {
        return it == p_it.it;
    }

    template < class T >
    bool
    JwtSet < T > :: iterator :: operator != (const iterator& p_it) const
    {
        return it != p_it.it;
    }

    template < class T >
    T *
    JwtSet < T > :: iterator :: operator -> () const
    {
        if ( it == nullptr )
        {
            return nullptr;
        }
        return ( T * ) & it -> body;
    }

#if 0

////////////////// JwtSet

    template < class T >
    JwtSet < T > :: JwtSet()
    {
        BSTreeInit ( & m_tree );
    }

    template < class T >
    JwtSet < T > :: ~ JwtSet()
    {
        BSTreeWhack ( & m_tree, Node::whack, nullptr );
    }

    template < class T >
    bool
    JwtSet < T > :: empty() const
    {
        return m_tree . root == nullptr;
    }

    template < class T >
    size_t
    JwtSet < T > :: size() const
    {
        size_t ret = 0;
        BSTreeForEach ( & m_tree, false, Node::count, & ret );
        return ret;
    }

    template < class T >
    typename JwtSet < T > :: iterator
    JwtSet < T > :: find (const Key& k)
    {
        return iterator ( ( Node * ) BSTreeFind ( & m_tree, & k, Node::cmp ) );
    }

    template < class T >
    typename JwtSet < T > :: const_iterator
    JwtSet < T > :: find (const Key& k) const
    {
        return const_iterator ( ( const Node * ) BSTreeFind ( & m_tree, & k, Node::cmp ) );
    }

    template < class T >
    typename JwtSet < T > :: iterator
    JwtSet < T > :: begin() noexcept
    {
        return iterator ( ( Node * ) BSTreeFirst ( & m_tree ) );
    }
    template < class T >
    typename JwtSet < T > :: const_iterator
    JwtSet < T > :: begin() const noexcept
    {
        return const_iterator ( ( Node * ) BSTreeFirst ( & m_tree ) );
    }
    template < class T >
    typename JwtSet < T > :: const_iterator
    JwtSet < T > :: cbegin() const noexcept
    {
        return const_iterator ( ( Node * ) BSTreeFirst ( & m_tree ) );
    }

    template < class T >
    typename JwtSet < T > :: iterator
    JwtSet < T > :: end() noexcept
    {
        return iterator ( nullptr );
    }
    template < class T >
    typename JwtSet < T > :: const_iterator
    JwtSet < T > :: end() const noexcept
    {
        return const_iterator ( nullptr );
    }
    template < class T >
    typename JwtSet < T > :: const_iterator
    JwtSet < T > :: cend() const noexcept
    {
        return const_iterator ( nullptr );
    }

    template < class T >
    template <class... Args>
    JwtPair < typename JwtSet < T > :: iterator, bool >
    JwtSet < T > :: emplace (Args&&... args)
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

    template < class T >
    typename JwtSet < T > :: iterator
    JwtSet < T > :: erase (const_iterator position)
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


////////////////// JwtSet :: iterator

    template < class T > JwtSet < T > :: iterator ::
    iterator (Node * p_it)
    : it ( p_it )
    {
    };

    template < class T >
    bool
    JwtSet < T > :: iterator :: operator == (const iterator& p_it) const
    {
        return it == p_it.it;
    }

    template < class T >
    bool
    JwtSet < T > :: iterator :: operator != (const iterator& p_it) const
    {
        return it != p_it.it;
    }

    template < class T >
    typename JwtSet < T > :: iterator &
    JwtSet < T > :: iterator :: operator ++ ()
    {
        assert ( it != nullptr );
        it = ( Node * ) BSTNodeNext ( & it -> node );
        return *this;
    }

////////////////// JwtSet :: const_iterator

    template < class T > JwtSet < T > :: const_iterator ::
    const_iterator (const Node * p_it)
    : it ( p_it )
    {
    };
    template < class T > JwtSet < T > :: const_iterator ::
    const_iterator (const iterator& p_it)
    : it ( * p_it )
    {
    };

    template < class T >
    bool
    JwtSet < T > :: const_iterator :: operator == (const const_iterator& p_it) const
    {
        return it == p_it.it;
    }
    template < class T >
    bool
    JwtSet < T > :: const_iterator :: operator != (const const_iterator& p_it) const
    {
        return it != p_it.it;
    }

    template < class T >
    const typename JwtPair<const Key,Value> *
    JwtSet < T > :: const_iterator :: operator -> () const
    {
        if ( it == nullptr )
        {
            return nullptr;
        }
        return ( JwtPair<const Key,Value> * ) & it -> body;
    }

    template < class T >
    typename JwtSet < T > :: const_iterator &
    JwtSet < T > :: const_iterator :: operator ++ ()
    {
        assert ( it != nullptr );
        it = ( Node * ) BSTNodeNext ( & it -> node );
        return *this;
    }
#endif
}
#endif /* _hpp_jwt_set_ */
