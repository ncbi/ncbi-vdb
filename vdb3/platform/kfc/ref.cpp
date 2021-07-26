/*

  vdb3.kfc.ref

 */

#include <vdb3/kfc/ref.hpp>
#include <vdb3/kfc/except.hpp>
#include <vdb3/kfc/ktrace.hpp>

#include <atomic>
#include <memory>

#if DEBUG
#define DETECT_REFCOUNT_LIMIT 1
#else
#define DETECT_REFCOUNT_LIMIT 0
#endif

namespace vdb3
{
    /*=====================================================*
     *                   unique_void_ptr                   *
     *=====================================================*/

#if NO_ATOMIC_OPS
    typedef void * volatile volatile_void_ptr_t;
#else
    typedef std :: atomic < void * > volatile_void_ptr_t;
#endif

    inline volatile_void_ptr_t & access_ptr ( byte_t ptr_space [] )
    {
        return * ( volatile_void_ptr_t * ) ptr_space;
    }

    inline const volatile_void_ptr_t & access_ptr ( const byte_t ptr_space [] )
    {
        return * ( const volatile_void_ptr_t * ) ptr_space;
    }

    void * volatile_void_ptr :: get () const noexcept
    {
        const volatile_void_ptr_t & p = access_ptr ( ptr_space );
        return p;
    }

    void * volatile_void_ptr :: exchange ( void * ptr ) noexcept
    {
        volatile_void_ptr_t & p = access_ptr ( ptr_space );

        void * prior;
        
#if NO_ATOMIC_OPS

        prior = p;
        
        // guard against deleting the pointer we're holding onto
        if ( prior == ptr )
            return nullptr;
        
        p = ptr;
#else
        prior = p . exchange ( ( void * ) ptr );

        // guard against deleting the pointer we're holding onto
        if ( prior == ptr )
            return nullptr;
#endif
        
        return prior;
    }

    volatile_void_ptr :: volatile_void_ptr ( void * ptr ) noexcept
    {
        volatile_void_ptr_t & p = access_ptr ( ptr_space );

#if NO_ATOMIC_OPS
        
        assert ( sizeof ( volatile_void_ptr_t ) >= sizeof ptr );
        p = ptr;
#else
        // use placement operator
        ( void ) new ( & p ) std :: atomic < void * > ( ptr );
#endif
    }

    volatile_void_ptr :: ~ volatile_void_ptr () noexcept
    {
        volatile_void_ptr_t & p = access_ptr ( ptr_space );
        
#if NO_ATOMIC_OPS
        
        p = nullptr;
#else
        // run whatever destructor is required
        p . std :: atomic < void * > :: ~ atomic ();
#endif
    }
    

    /*=====================================================*
     *                     ref_counter                     *
     *=====================================================*/

    const ref_counter :: val_t RC_MAX =
        ( sizeof ( ref_counter :: val_t ) == sizeof ( Z64 ) ) ? 
        ( ref_counter :: val_t ) Z64_MAX :
        ( ref_counter :: val_t ) Z32_MAX ;

#if NO_ATOMIC_OPS
    typedef ref_counter :: val_t volatile_ref_counter_val_t;
#else
    typedef std :: atomic < ref_counter :: val_t > volatile_ref_counter_val_t;
#endif

    inline volatile_ref_counter_val_t & access_ctr ( byte_t ctr_space [] )
    {
        return * ( volatile_ref_counter_val_t * ) ctr_space;
    }

    inline const volatile_ref_counter_val_t & access_ctr ( const byte_t ctr_space [] )
    {
        return * ( const volatile_ref_counter_val_t * ) ctr_space;
    }

    // read current count
    ref_counter :: val_t ref_counter :: get () const noexcept
    {
        const volatile_ref_counter_val_t & ctr = access_ctr ( ctr_space );
        return ctr;
    }
        
    // increment counter by one
    // stops at max int value
    // returns resulting count after increment
    // returns negative on rollover
    ref_counter :: val_t ref_counter :: inc () noexcept
    {
        REF_KTRACE ( TRACE_REF, "incrementing counter @ %p\n", this );
        
        volatile_ref_counter_val_t & ctr = access_ctr ( ctr_space );

#if NO_ATOMIC_OPS

        // safely increment counter only if less than max
        REF_KTRACE ( TRACE_REF, "non-atomic before value = %ld, limit = %ld\n", ctr, RC_MAX );
        if ( ctr < RC_MAX )
            return ++ ctr;
        
#elif DETECT_REFCOUNT_LIMIT

        // use compare and swap loop to advance counter by 1
        // bail if counter is ever seen at max
        val_t before = ctr;
        REF_KTRACE ( TRACE_REF, "limited atomic before value = %ld, limit = %ld\n", before, RC_MAX );
        while ( before < RC_MAX )
        {
            // this won't overflow because before < RC_MAX
            val_t after = before + 1;

            // attempt increment
            if ( ctr . compare_exchange_strong ( before, after ) )
                return after;
        }
        REF_KTRACE ( 1, "limited atomic inc failed: before value = %ld, limit = %ld\n", before, RC_MAX );
#else
        // unsafely increment counter, throwing caution to the wind
        val_t after = ++ ctr;
        REF_KTRACE ( TRACE_REF, "unlimited atomic after value = %ld, limit = %ld\n", after, RC_MAX );
        // detect success
        if ( after > 0 )
            return after;

        // undo increment
        REF_KTRACE ( TRACE_REF, "unlimited atomic inc failed - subtracting 1 from ctr\n" );
        -- ctr;
#endif
        // won't increment past max
        return -1;
    }

    // increment counter by one
    // stops at max int value
    // returns resulting count after increment
    // throws exception on rollover
    ref_counter :: val_t ref_counter :: inc_and_clip ()
    {
        // run standard increment
        val_t after = inc ();

        // detect clip and throw exception
        if ( after < 0 )
        {
            throw OverflowException (
                XP ( XLOC )
                << "number of references is at max ( "
                << RC_MAX
                << " ) and would overflow"
                );
        }

        // return updated value
        return after;
    }

    // decrement counter by one
    // stops at zero
    // returns resulting count after decrement
    // returns negative on rollover
    ref_counter :: val_t ref_counter :: dec () noexcept
    {
        REF_KTRACE ( TRACE_REF, "decrementing counter @ %p\n", this );
        
        volatile_ref_counter_val_t & ctr = access_ctr ( ctr_space );

#if NO_ATOMIC_OPS

        // safely decrement counter only if greater than zero
        REF_KTRACE ( TRACE_REF, "non-atomic before value = %ld\n", ctr );
        if ( ctr > 0 )
            return -- ctr;
        
#elif DETECT_REFCOUNT_LIMIT

        // use compare and swap loop to decrement counter by 1
        // bail if counter is ever seen at zero
        val_t before = ctr;
        REF_KTRACE ( TRACE_REF, "limited atomic before value = %ld\n", before );
        while ( before > 0 )
        {
            // this won't underflow because before > 0
            val_t after = before - 1;

            // attempt decrement
            if ( ctr . compare_exchange_strong ( before, after ) )
                return after;
        }
        REF_KTRACE ( 1, "limited atomic dec failed: before value = %ld\n", before );
#else
        // unsafely decrement counter, throwing caution
        val_t after = -- ctr;
        REF_KTRACE ( TRACE_REF, "unlimited atomic after value = %ld, limit = %ld\n", after, RC_MAX );

        // detect success
        if ( after >= 0 )
            return after;

        // undo decrement
        REF_KTRACE ( TRACE_REF, "unlimited atomic dec failed - adding 1 back to ctr\n" );
        ++ ctr;
#endif
        // won't decrement past zero
        return -1;
    }

    // initialize the counter to value 0
    ref_counter :: ref_counter () noexcept
    {
        REF_KTRACE ( TRACE_REF, "constructor setting initial value to 0\n" );
        volatile_ref_counter_val_t & ctr = access_ctr ( ctr_space );
        ctr = 0;
    }
        
    // here basically in case of running std::atomic::~atomic
    ref_counter :: ~ ref_counter () noexcept
    {
        volatile_ref_counter_val_t & ctr = access_ctr ( ctr_space );
        
        REF_KTRACE ( TRACE_REF, "destructor setting value back to 0 from %ld\n", ( val_t ) ctr );

#if NO_ATOMIC_OPS
        ctr = 0;
#else
        // run whatever destructor is required
        ctr . std :: atomic < val_t > :: ~ atomic ();
#endif
    }

    
    /*=====================================================*
     *                 refcounted_void_ptr                 *
     *=====================================================*/

    refcounted_void_ptr refcounted_void_ptr :: zrcvp;

    refcounted_void_ptr * refcounted_void_ptr :: make ( void * ptr ) noexcept
    {
        if ( ptr == nullptr )
            return & zrcvp;

        return new refcounted_void_ptr ( ptr );
    }

    void * refcounted_void_ptr :: whack () noexcept
    {
        if ( this == & zrcvp )
            return nullptr;
        
        void * prior = p;
        delete this;
        
        return prior;
    }
    
    refcounted_void_ptr :: refcounted_void_ptr ( void * ptr ) noexcept
        : p ( ptr )
    {
    }
    
    refcounted_void_ptr :: ~ refcounted_void_ptr () noexcept
    {
        p = nullptr;
    }

    
    /*=====================================================*
     *                   shared_void_ptr                   *
     *=====================================================*/

    // create a new refcounted pointer
    // detach from old and attach to new rcp
    // returns result of detaching from old
    void * shared_void_ptr :: reset ( const void * ptr ) noexcept
    {
        // ignore resetting to the same pointer
        if ( self -> get () == ( void * ) ptr )
            return nullptr;

        // create a new refcounted void pointer
        // yes, might be nice to use a unique_ptr!
        // can't though, because it assumes it can delete...
        // and everything is marked "noexcept"
        refcounted_void_ptr * new_rcvp = refcounted_void_ptr :: make ( ( void * ) ptr );

        // now exchange with the existing guy
        refcounted_void_ptr * old_rcvp = ( refcounted_void_ptr * )
            vp . exchange ( ( void * ) new_rcvp -> attach () );

        // detach from the old guy and return his pointer value
        return old_rcvp -> detach ();
    }

    // detach from old and attach to new rcp
    // returns result of detaching from old
    void * shared_void_ptr :: attach ( const shared_void_ptr & p )
    {
        refcounted_void_ptr * new_rcvp = p . rvp ();

        // now exchange with the existing guy
        refcounted_void_ptr * old_rcvp = ( refcounted_void_ptr * )
            vp . exchange ( ( void * ) new_rcvp -> attach () );

        // detach from the old guy and return his pointer value
        return old_rcvp -> detach ();
    }

    // detach from old and steal new
    // returns result of detaching from old
    void * shared_void_ptr :: steal ( shared_void_ptr & p ) noexcept
    {
        // get a substitute - the refcounted pointer to null
        refcounted_void_ptr * subst_rcvp = refcounted_void_ptr :: make ( nullptr );

        // steal the incoming refcounted void pointer
        refcounted_void_ptr * new_rcvp = ( refcounted_void_ptr * )
            p . vp . exchange ( ( void * ) subst_rcvp -> attach () );

        // now exchange with the existing guy
        refcounted_void_ptr * old_rcvp = ( refcounted_void_ptr * )
            vp . exchange ( ( void * ) new_rcvp );

        // detach from the old guy and return his pointer value
        return old_rcvp -> detach ();
    }

    // detach and nullify refcounted_void_ptr
    // returns result of detaching from old
    void * shared_void_ptr :: detach () noexcept
    {
        return reset ( nullptr );
    }

    // initialize with a new refcounted_void_ptr
    shared_void_ptr :: shared_void_ptr ( const void * ptr )
        : vp ( ( void * ) refcounted_void_ptr :: make ( ( void * ) ptr ) -> attach () )
    {
    }

    // attach to refcounted_void_ptr
    shared_void_ptr :: shared_void_ptr ( const shared_void_ptr & p )
        : vp ( ( void * ) p . rvp () -> attach () )
    {
    }

    // steal refcounted_void_ptr
    shared_void_ptr :: shared_void_ptr ( shared_void_ptr && p )
        : vp ( nullptr )
    {
        ( void ) vp . exchange (
            p . vp . exchange (
                ( void * ) refcounted_void_ptr :: make ( nullptr ) -> attach ()
                )
            );
    }

    // detach from guy
    shared_void_ptr :: ~ shared_void_ptr () noexcept
    {
        refcounted_void_ptr * rcvp = ( refcounted_void_ptr * ) vp . exchange ( nullptr );
        rcvp -> detach ();
    }
}
