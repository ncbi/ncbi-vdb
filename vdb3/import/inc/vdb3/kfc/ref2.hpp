/*

  vdb3.kfc.ref2

 */

#pragma once

#include <vdb3/kfc/except.hpp>

#include <atomic>
#include <memory>

/**
 * @file kfc/ref2.hpp
 * @brief C++ reference-objects
 *
 * C++ supplies a standard library with different types of
 * references modeled around the "smart-pointer" paradigm.
 *
 * The unfortunate coupling of reference management - which
 * is type agnostic, with specifics of object deletion -
 * which is type specific, encourages use of templates that
 * are capable of implementing a basic algorithm with a
 * parameterized type.
 *
 * Templates, in turn, require 100% inline definition because
 * they cause the compiler to generate as many versions of
 * the code as required to cover the type usage. This is not
 * code reuse, but is source code reuse.
 *
 * When all code is inlined, binary isolation is destroyed
 * and variations cannot be confined to specific libraries
 * for debugging/testing or different environments. This
 * affects code making use of "atomic" functions difficult
 * to manage.
 *
 * Single-threaded application code does not need atomic
 * operations provided that an object is only accessed via
 * that single thread ( i.e. no kernel interrupt handlers ).
 * Atomic operations are demanding on data caches and main
 * memory busses, and often require stalling multiple cores
 * to guarantee atomicity of updates. This can be extremely
 * expensive, especially when used within a process known
 * to be single threaded.
 */

namespace vdb3
{
    
    /*=====================================================*
     *                  volatile_void_ptr                  *
     *=====================================================*/

    /**
     * @class volatile_void_ptr
     * @brief encapsulation of a volatile untyped pointer
     */
    class volatile_void_ptr
    {
    public:
        
        // return the void pointer
        void * get () const noexcept;

        // exchange pointer values
        void * exchange ( void * ptr ) noexcept;

        // construct
        // discards "const"
        volatile_void_ptr ( void * ptr ) noexcept;

        // here basically in case of running std::atomic::~atomic
        ~ volatile_void_ptr () noexcept;

    private:
        
        // large enough to hold the atomic type
        // under the assumption that it's >= sizeof ( void * )
        byte_t ptr_space [ sizeof ( std :: atomic < void * > ) ];
    };

    
    /*=====================================================*
     *                     unique_ptr                      *
     *=====================================================*/

    /**
     * @class unique_ptr
     * @brief typed volatile pointer storage
     *
     * uses a volatile_void_ptr for storage,
     * handles deletion and static typecasting
     */
    template < class T >
    class unique_ptr
    {
    public:

        // read the pointer value
        inline T * get () const noexcept
        { return ( T * ) ptr . get (); }

        // return pointer and null out storage
        inline T * release () noexcept
        { return ( T * ) ptr . exchange ( nullptr ); }

        // overwrite pointer
        // deletes any previously referenced object
        inline void reset ( const T * _ptr ) noexcept
        { delete ( T * ) ptr . exchange ( ( void * ) _ptr ); }

        // resets pointer value
        // steals pointer from source
        // deletes any previously referenced object
        inline void operator = ( unique_ptr < T > & p ) noexcept
        { delete ( T * ) ptr . exchange ( p . ptr . exchange ( nullptr ) ); }
        
        // resets pointer value
        // steals pointer from source
        // deletes any previously referenced object
        inline void operator = ( unique_ptr < T > && p ) noexcept
        { delete ( T * ) ptr . exchange ( p . ptr . exchange ( nullptr ) ); }

        // steal pointer from source
        inline unique_ptr ( unique_ptr < T > & p ) noexcept
            : ptr ( p . ptr . exchange ( nullptr ) )
        {
        }

        // steal pointer from source
        inline unique_ptr ( unique_ptr < T > && p ) noexcept
            : ptr ( p . ptr . exchange ( nullptr ) )
        {
        }

        // set pointer
        inline unique_ptr ( void * _ptr = nullptr ) noexcept
            : ptr ( _ptr )
        {
        }

        // delete object
        // delete pointer
        // null out pointer member
        inline ~ unique_ptr () noexcept
        {
            delete release ();
        }

    private:

        // opaque storage with atomic or simply volatile pointer
        volatile_void_ptr ptr;
    };

    
    /*=====================================================*
     *                     ref_counter                     *
     *=====================================================*/

    class ref_counter
    {
    public:
        
        // use "long int" as a signed "size_t" ( "ssize_t" )
        typedef long int val_t;
        
        // read current count
        val_t get () const noexcept;
        
        // increment counter by one
        // stops at max int value
        // returns resulting count after increment
        // returns negative on rollover
        val_t inc () noexcept;

        // increment counter by one
        // stops at max int value
        // returns resulting count after increment
        // throws exception on rollover
        val_t inc_and_clip ();

        // decrement counter by one
        // stops at zero
        // returns resulting count after decrement
        // returns negative on rollover
        val_t dec () noexcept;

        // initialize the counter to value 0
        ref_counter () noexcept;
        
        // here basically in case of running std::atomic::~atomic
        ~ ref_counter () noexcept;

    private:
        
        // large enough to hold the atomic type
        // under the assumption that it's >= sizeof ( val_t )
        byte_t ctr_space [ sizeof ( std :: atomic < val_t > ) ];
    };

    
    /*=====================================================*
     *                 refcounted_void_ptr                 *
     *=====================================================*/

    class refcounted_void_ptr
    {
    public:

        static refcounted_void_ptr * make ( void * ptr = nullptr ) noexcept;
        
        // return the void pointer
        inline void * get () const noexcept
        { return p; }

        // return true if not multiple references
        inline bool unique () const noexcept
        { return refcount . get () <= 1; }

        // we always expose it but have to warn about its use
        inline ref_counter :: val_t use_count () const noexcept
        { return refcount . get (); }

        // attach a new reference
        inline refcounted_void_ptr * attach ()
        { refcount . inc_and_clip (); return this; }

        // detach a reference
        // returns pointer value if was last
        // returns nullptr otherwise
        inline void * detach () noexcept
        {
            if ( refcount . dec () > 0 )
                return nullptr;
            return whack ();
        }

    private:

        // delete self
        // return the prior pointer value
        void * whack () noexcept;

        refcounted_void_ptr ( void * ptr = nullptr ) noexcept;
        ~ refcounted_void_ptr () noexcept;


        static refcounted_void_ptr zrcvp;
        ref_counter refcount;
        void * p;
    };

    
    /*=====================================================*
     *                   shared_void_ptr                   *
     *=====================================================*/

    class shared_void_ptr
    {
    public:
        
        // return the refcounted void pointer
        inline refcounted_void_ptr * rvp () const noexcept
        { return ( refcounted_void_ptr * ) vp . get (); }
        
        // access to refcounted pointer
        inline refcounted_void_ptr * operator -> () const noexcept
        { return rvp (); }

        // create a new refcounted pointer
        // detach from old and attach to new rcp
        // returns result of detaching from old
        void * reset ( const void * ptr ) noexcept;

        // detach from old and attach to new rcp
        // returns result of detaching from old
        void * attach ( const shared_void_ptr & p );

        // detach from old and steal new
        // returns result of detaching from old
        void * steal ( shared_void_ptr & p ) noexcept;

        // detach and nullify refcounted_void_ptr
        // returns result of detaching from old
        void * detach () noexcept;

        // initialize with a new refcounted_void_ptr
        shared_void_ptr ( const void * ptr = nullptr );

        // attach to refcounted_void_ptr
        shared_void_ptr ( const shared_void_ptr & p );

        // steal refcounted_void_ptr
        shared_void_ptr ( shared_void_ptr && p );

        // only useful to clean up whatever is there in std::atomic
        ~ shared_void_ptr () noexcept;

    private:
        
        // large enough to hold the atomic type
        // under the assumption that it's >= sizeof ( void * )
        volatile_void_ptr vp;
    };

    
    /*=====================================================*
     *                     shared_ptr                      *
     *=====================================================*/

    /**
     * @class shared_ptr
     * @brief typed atomic pointer storage
     *
     * uses a unique_void_ptr for storage,
     * handles deletion and static typecasting
     */
    template < class T >
    class shared_ptr
    {
    public:

        // read the pointer value
        inline T * get () const noexcept
        { return ( T * ) ptr -> get (); }

        // return true if count == 1
        inline bool unique () const noexcept
        { return ptr -> unique (); }

        // return volatile count
        inline long int use_count () const noexcept
        { return ptr -> use_count (); }

        // overwrite pointer
        inline void reset ( const T * _ptr ) noexcept
        { delete ( T * ) ptr . reset ( ( const void * ) _ptr ); }

        // steal pointer from source and overwrite self pointer
        inline void operator = ( const shared_ptr < T > & p ) noexcept
        { delete ( T * ) ptr . attach ( p . ptr ); }
        
        inline void operator = ( shared_ptr < T > && p ) noexcept
        { delete ( T * ) ptr . steal ( p . ptr ); }

        // steal pointer from source
        inline shared_ptr ( const shared_ptr < T > & p ) noexcept
            : ptr ( p . ptr )
        {
        }
        inline shared_ptr ( shared_ptr < T > && p ) noexcept
            : ptr ( p . ptr )
        {
        }

        // set pointer
        inline shared_ptr ( void * _ptr = nullptr ) noexcept
            : ptr ( _ptr )
        {
        }

        // delete object
        // null out pointer
        inline ~ shared_ptr () noexcept
        {
            delete ( T * ) ptr . detach ();
        }

    private:

        shared_void_ptr ptr;
    };


    /*=====================================================*
     *                     EXCEPTIONS                      *
     *=====================================================*/

}
