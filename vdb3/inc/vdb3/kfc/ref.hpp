/*

  vdb3.kfc.ref

 */

#pragma once

#include <vdb3/kfc/except.hpp>
#include <vdb3/kfc/ref2.hpp>

/**
 * @file kfc/ref.hpp
 * @brief C++ reference-objects
 */

#if NDEBUG
#define REF_KTRACE( lvl, ... ) ( void ) 0
#else
#include <vdb3/kfc/ktrace.hpp>
#define REF_KTRACE( lvl, ... ) KTRACE ( lvl, __VA_ARGS__ )
#endif

namespace vdb3
{
#if ! NDEBUG
    const Z32 TRACE_REF = 7;
#endif

    /*=====================================================*
     *                        XRef                         *
     *=====================================================*/
    
    /**
     * @class XRef
     * @brief the hot-potato reference
     *
     *  ensures that only a single reference holds the object pointer,
     *  while allowing for that pointer to be passed freely between
     *  references.
     *
     *  the point of such a reference is exactly for tracking an allocation
     *  through a message chain, and ensuring that an exception at any
     *  point will cause the object to be collected, while at the same
     *  time allowing for efficient transfer of the pointer.
     */
    template < class T >
    class XRef
    {
    public:

        inline T * get () const noexcept
        { return p . get (); }

        inline T * release () const noexcept
        { return p . release (); }

        inline void reset ( T * ptr ) noexcept
        { p . reset ( ptr ); }

        inline bool operator ! () const noexcept
        { return p . get () == nullptr; }

        inline T * operator -> () const
        {
            T * t = p . get ();
            if ( t == nullptr )
            {
                throw NullReferenceException (
                    XP ( XLOC, rc_logic_err )
                    << "dereferencing reference to null object"
                    );
            }
            return t;
        }

        inline T & operator * () const
        {
            T * t = p . get ();
            if ( t == nullptr )
            {
                throw NullReferenceException (
                    XP ( XLOC, rc_logic_err )
                    << "dereferencing reference to null object"
                    );
            }
            return * t;
        }

        inline T const & operator [] ( index_t idx ) const
        {
            T * t = p . get ();
            if ( t == nullptr )
            {
                throw NullReferenceException (
                    XP ( XLOC, rc_logic_err )
                    << "dereferencing reference to null object"
                    );
            }
            return t [ idx ];
        }

        inline T & operator [] ( index_t idx )
        {
            T * t = p . get ();
            if ( t == nullptr )
            {
                throw NullReferenceException (
                    XP ( XLOC, rc_logic_err )
                    << "dereferencing reference to null object"
                    );
            }
            return t [ idx ];
        }

        inline bool operator < ( const XRef < T > & r ) const noexcept
        { return p < r . p; }

        inline bool operator <= ( const XRef < T > & r ) const noexcept
        { return p <= r . p; }

        inline bool operator == ( const XRef < T > & r ) const noexcept
        { return p == r . p; }

        inline bool operator != ( const XRef < T > & r ) const noexcept
        { return p != r . p; }

        inline bool operator >= ( const XRef < T > & r ) const noexcept
        { return p >= r . p; }

        inline bool operator > ( const XRef < T > & r ) const noexcept
        { return p > r . p; }

        inline bool operator == ( std :: nullptr_t ) const noexcept
        { return p == nullptr; }

        inline bool operator == ( const T * ptr ) const noexcept
        { return p == const_cast < T * > ( ptr ); }

        inline bool operator != ( std :: nullptr_t ) const noexcept
        { return p != nullptr; }

        inline bool operator != ( const T * ptr ) const noexcept
        { return p != const_cast < T * > ( ptr ); }

        inline XRef & operator = ( T * ptr ) noexcept
        {
            p . reset ( ptr );
            return self;
        }

        inline XRef & operator = ( XRef < T > && r ) noexcept
        {
            p = std :: move ( r . p );
            return self;
        }
        inline XRef & operator = ( const XRef < T > & r ) noexcept
        {
            p . reset ( r . p . release () );
            return self;
        }

        inline XRef ( XRef < T > && r ) noexcept
            : p ( std :: move ( r . p ) )
        {
        }
        inline XRef ( const XRef < T > & r ) noexcept
            : p ( r . p . release () )
        {
        }

        inline XRef ( T * ptr ) noexcept
            : p ( ptr )
        {
        }

        inline XRef () noexcept {}
        inline ~ XRef () noexcept {}

    private:

        mutable vdb3 :: unique_ptr < T > p;
    };


    /*=====================================================*
     *                        SRef                         *
     *=====================================================*/
    
    /**
     * @class SRef
     * @brief a shared, counted reference
     *
     *  For our purposes, this allows for co-equal shared ownership
     *  that is useful for immutable objects like JWKs.
     */
    template < class T >
    class SRef
    {
    public:

        inline T * get () const noexcept
        { return p . get (); }

        inline void release () noexcept
        { p . reset ( ( T * ) nullptr ); }

        inline void reset ( T * ptr ) noexcept
        { p . reset ( ptr ); }

        inline bool unique () const noexcept
        { return p . unique (); }

        inline long int use_count () const noexcept
        { return p . use_count (); }

        bool operator ! () const noexcept
        { return p . get () == nullptr; }

        inline T * operator -> () const
        {
            T * t = p . get ();
            if ( t == nullptr )
            {
                throw NullReferenceException (
                    XP ( XLOC, rc_logic_err )
                    << "dereferencing reference to null object"
                    );
            }
            return t;
        }

        inline T & operator * () const
        {
            T * t = p . get ();
            if ( t == nullptr )
            {
                throw NullReferenceException (
                    XP ( XLOC, rc_logic_err )
                    << "dereferencing reference to null object"
                    );
            }
            return * t;
        }

        inline bool operator < ( const SRef < T > & r ) const noexcept
        { return p < r . p; }

        inline bool operator <= ( const SRef < T > & r ) const noexcept
        { return p <= r . p; }

        inline bool operator == ( const SRef < T > & r ) const noexcept
        { return p == r . p; }

        inline bool operator != ( const SRef < T > & r ) const noexcept
        { return p != r . p; }

        inline bool operator >= ( const SRef < T > & r ) const noexcept
        { return p >= r . p; }

        inline bool operator > ( const SRef < T > & r ) const noexcept
        { return p > r . p; }

        inline bool operator == ( std :: nullptr_t ) const noexcept
        { return p == nullptr; }

        inline bool operator == ( const T * ptr ) const noexcept
        { return p == const_cast < T * > ( ptr ); }

        inline bool operator != ( std :: nullptr_t ) const noexcept
        { return p != nullptr; }

        inline bool operator != ( const T * ptr ) const noexcept
        { return p != const_cast < T * > ( ptr ); }

        inline SRef & operator = ( T * ptr ) noexcept
        {
            p . reset ( ptr );
            return self;
        }

        inline SRef & operator = ( SRef < T > && r ) noexcept
        {
            p = std :: move ( r . p );
            return self;
        }
        inline SRef & operator = ( const SRef < T > & r ) noexcept
        {
            p = r . p;
            return self;
        }

        inline SRef ( SRef < T > && r ) noexcept
            : p ( std :: move ( r . p ) )
        {
        }
        inline SRef ( const SRef < T > & r ) noexcept
            : p ( r . p )
        {
        }

        inline SRef ( T * ptr ) noexcept
            : p ( ptr )
        {
        }

        inline SRef () noexcept {}
        inline ~ SRef () noexcept {}

    private:

        vdb3 :: shared_ptr < T > p;
    };


    /*=====================================================*
     *                        IRef                         *
     *=====================================================*/

    /**
     * Refcount
     * @brief base class for reference-counted objects
     *
     *  The counter itself must be updated when passed between
     *  reference objects, regardless of the mutability of
     *  the rest of the object.
     *
     *  Contrast this with SRef which creates a non-intrusive
     *  reference-counting block that owns a single pointer
     *  to a dynamically allocated object.
     */
    class Refcount
    {
    public:

        /**
         * ~Refcount
         * @brief required virtual destructor so delete works
         */
        virtual ~ Refcount () noexcept {}

    protected:

        /**
         * Refcount
         * @brief constructor initializes count to zero
         */
        inline Refcount () {}

    private:
        
        // allow pointer to const Refcount to be usable
        // since const objects are also counted
        mutable ref_counter refcount;
        friend class IRefBase;
    };

    /**
     * IRefBase
     * @brief base class for IRef
     *
     *  All methods that are not sensitive to interface class
     *  are implemented herein, e.g. everything dealing with
     *  reference counting.
     */
    class IRefBase
    {
    public:

        /**
         * unique
         * @return true if this holds last reference to object
         */
        inline bool unique () const noexcept
        {
            if ( inst == nullptr )
                return true;
            return inst -> refcount . get () <= 1;
        }

        /**
         * use_count
         * @return snapshot of volatile reference count
         */
        inline ref_counter :: val_t use_count () const noexcept
        {
            if ( inst == nullptr )
                return 0;
            return inst -> refcount . get ();
        }

    protected:

        /**
         * attach
         * @brief increments a reference count
         */
        static inline void attach ( const Refcount * obj )
        {
            REF_KTRACE ( TRACE_REF, "attaching to %p\n", obj );
            if ( obj != nullptr )
                obj -> refcount . inc_and_clip ();
        }

        /**
         * detach
         * @brief decrements reference count and deletes object if last
         */
        static inline void detach ( const Refcount * obj ) noexcept
        {
            REF_KTRACE ( TRACE_REF, "detaching from %p\n", obj );
            if ( obj != nullptr )
            {
                ref_counter :: val_t count = obj -> refcount . dec ();
                if ( count == 0 )
                {
                    REF_KTRACE ( TRACE_REF, "deleting Refcount * %p\n", obj );
                    delete obj;
                }
            }
        }
        
        /**
         * IRefBase
         * @overload copy operator rebinds to object
         * @param obj a pointer to dynamically allocated object
         */
        inline void operator = ( const Refcount * obj )
        {
            REF_KTRACE ( TRACE_REF, "assigning from ptr %p\n", obj );
            const Refcount * old = inst;
            inst = obj;
            attach ( obj );
            detach ( old );
        }

        /**
         * IRefBase
         * @overload constructor binds to object
         * @param obj a pointer to dynamically allocated object
         */
        inline IRefBase ( const Refcount * obj )
            : inst ( obj )
        {
            REF_KTRACE ( TRACE_REF, "constructing from ptr %p\n", obj );
            attach ( inst );
        }

        /**
         * IRefBase
         * @overload move operator drops existing and steals incoming
         * @param rb IRefBase to pillage
         */
        inline void operator = ( IRefBase && rb )
        {
            REF_KTRACE ( TRACE_REF, "moving ptr %p from ref %p\n", rb . inst, & rb );
            const Refcount * old = inst;
            inst = rb . inst;
            rb . inst = nullptr;
            detach ( old );
        }

        /**
         * IRefBase
         * @overload move constructor steals object
         * @param rb IRefBase to pillage
         */
        inline IRefBase ( IRefBase && rb )
            : inst ( rb . inst )
        {
            REF_KTRACE ( TRACE_REF, "moving ptr %p from ref %p\n", inst, & rb );
            rb . inst = nullptr;
        }

        /**
         * IRefBase
         * @overload copy operator rebinds to object
         * @param rb IRefBase to copy
         */
        inline void operator = ( const IRefBase & rb )
        {
            REF_KTRACE ( TRACE_REF, "copying ptr %p from ref %p\n", rb . inst, & rb );
            self . operator = ( rb . inst );
        }

        /**
         * IRefBase
         * @brief constructor binds to object
         * @param obj a pointer to dynamically allocated object
         */
        inline IRefBase ( const IRefBase & rb )
            : IRefBase ( rb . inst )
        {
            REF_KTRACE ( TRACE_REF, "copyied ptr %p from ref %p\n", inst, & rb );
        }

        /**
         * ~IRefBase
         * @brief destructor detaches from object
         */
        inline ~ IRefBase () noexcept
        {
            REF_KTRACE ( TRACE_REF, "destroying ref %p with inst %p\n", this, inst );
            const Refcount * obj = inst;
            inst = nullptr;
            detach ( obj );
        }

        // retain const pointer in order to hold reference
        // to a const object, yet allow self to be modified.
        const Refcount * inst;
    };

    template < class T > class IRefFactory;

    template < class T >
    class IRef : public IRefBase
    {
    public:

        /* this is an old C-hack to get around compiler checks for const
           allow self to be const but return mutable interface pointer.
           blows the whole compiler thing out of the water, but it doesn't
           work right anyway... since "const" and "mutable" and "volatile"
           don't propagate for containers. it's just a broken language.

           the means of controlling const-ness is by separation of
           interface classes into const and mutable. the const-ness of
           the exported pointer will have no effect if all messages
           are labeled const.                                         */
        
        inline T * get () const noexcept
        { return itf; }

        inline void release () noexcept
        {
            itf = nullptr;
            IRefBase :: operator = ( nullptr );
        }

        inline bool operator ! () const noexcept
        { return inst == nullptr; }

        // see comment about "get()"
        inline T * operator -> () const
        {
            if ( itf == nullptr )
            {
                throw NullReferenceException (
                    XP ( XLOC, rc_logic_err )
                    << "dereferencing reference to null object"
                    );
            }
            return itf;
        }

        // see comment about "get()"
        inline T & operator * () const
        {
            if ( itf == nullptr )
            {
                throw NullReferenceException (
                    XP ( XLOC, rc_logic_err )
                    << "dereferencing reference to null object"
                    );
            }
            return * itf;
        }

        inline bool operator < ( const IRef < T > & r ) const noexcept
        { return itf < r . itf; }

        inline bool operator <= ( const IRef < T > & r ) const noexcept
        { return itf <= r . itf; }

        inline bool operator == ( const IRef < T > & r ) const noexcept
        { return itf == r . itf; }

        inline bool operator != ( const IRef < T > & r ) const noexcept
        { return itf != r . itf; }

        inline bool operator >= ( const IRef < T > & r ) const noexcept
        { return itf >= r . itf; }

        inline bool operator > ( const IRef < T > & r ) const noexcept
        { return itf > r . itf; }

        inline bool operator == ( std :: nullptr_t ) const noexcept
        { return inst == nullptr; }

        inline bool operator == ( const T * ptr ) const noexcept
        { return itf == const_cast < T * > ( ptr ); }

        inline bool operator != ( std :: nullptr_t ) const noexcept
        { return inst != nullptr; }

        inline bool operator != ( const T * ptr ) const noexcept
        { return itf != const_cast < T * > ( ptr ); }

        inline IRef < T > & operator = ( IRef < T > && r ) noexcept
        {
            IRefBase :: operator = ( std :: move ( r ) );

            // steal from "r"
            itf = std :: move ( r . itf );
            r . itf = nullptr;
            
            return self;
        }
        inline IRef ( IRef < T > && r ) noexcept
            : IRefBase ( std :: move ( r ) )
            , itf ( r . itf )
        {
            // "r" is supposed to be left in undefined state
            r . itf = nullptr;
        }

        template < class I > inline
        IRef < T > & operator = ( const IRef < I > & r ) noexcept
        {
            // assign
            IRefBase :: operator = ( r );
            itf = r . get ();
            
            return self;
        }
        template < class I > inline
        IRef ( const IRef < I > & r ) noexcept
            : IRefBase ( r )
            , itf ( r . get () )
        {
        }

        inline IRef < T > & operator = ( const IRef < T > & r ) noexcept
        {
            // assign
            IRefBase :: operator = ( r );
            itf = r . itf;
            
            return self;
        }
        inline IRef ( const IRef < T > & r ) noexcept
            : IRefBase ( r )
            , itf ( r . itf )
        {
        }
        
        inline IRef () noexcept
            : IRefBase ( nullptr )
            , itf ( nullptr )
        {
        }
        
        inline ~ IRef () noexcept
        {
            itf = nullptr;
        }

        template < class I > inline
        IRef < T > & operator = ( const I * impl ) noexcept
        {
            IRefBase :: operator = ( impl );
            itf = const_cast < I * > ( impl );
            
            return self;
        }

        template < class I > inline
        IRef ( const I * impl ) noexcept
            : IRefBase ( impl )
            , itf ( const_cast < I * > ( impl ) )
        {
        }

    private:

        // the standard case when implementation class
        // differs from the interface class
        inline IRef ( const IRefBase & obj, const T * _itf ) noexcept
            : IRefBase ( obj )
            , itf ( const_cast < T * > ( _itf ) )
        {
            assert ( ( ( inst != nullptr ) && ( itf != nullptr ) ) ||
                     ( ( inst == nullptr ) && ( itf == nullptr ) ) );
        }

        inline IRef ( const Refcount * obj, const T * _itf ) noexcept
            : IRefBase ( obj )
            , itf ( const_cast < T * > ( _itf ) )
        {
            assert ( ( ( inst != nullptr ) && ( itf != nullptr ) ) ||
                     ( ( inst == nullptr ) && ( itf == nullptr ) ) );
        }

        T * itf;

        friend class IRefFactory < T >;
    };

    template < class T >
    class IRefFactory
    {
    protected:

        static inline
        IRef < T > makeIRef ( const IRefBase & obj, const T * itf ) noexcept
        { return IRef < T > ( obj, itf ); }

        static inline
        IRef < T > makeIRef ( const Refcount * obj, const T * itf ) noexcept
        { return IRef < T > ( obj, itf ); }
    };


    /*=====================================================*
     *                     EXCEPTIONS                      *
     *=====================================================*/

}
