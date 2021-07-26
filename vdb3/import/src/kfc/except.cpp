/*

  vdb3.kfc.except


 */

#include <vdb3/kfc/except.hpp>
#include <vdb3/kfc/ktrace.hpp>
#include <vdb3/kfc/memset_s.hpp>

namespace vdb3
{
    
    /*=====================================================*
     *                       UTILITY                       *
     *=====================================================*/

    XMsg copyXMsg ( const XBuf & b );
    XMsg copyXMsg ( const char * s );
    XMsg & concatXMsg ( XMsg & x, const XBuf & b );
    XMsg & concatXMsg ( XMsg & x, const char * s );


    /*=====================================================*
     *                      Exception                      *
     *=====================================================*/

    const XMsg Exception :: what () const noexcept
    {
        XMsg x = copyXMsg ( prob_msg );
        if ( ! ctx_msg . isEmpty () )
            concatXMsg ( concatXMsg ( x, " while " ), ctx_msg );
        if ( ! cause_msg . isEmpty () )
            concatXMsg ( concatXMsg ( x, ": " ), cause_msg );
        if ( ! suggest_msg . isEmpty () )
            concatXMsg ( concatXMsg ( x, ". " ), suggest_msg );
        return x;
    }

    const XMsg Exception :: problem () const noexcept
    {
        return copyXMsg ( prob_msg );
    }

    const XMsg Exception :: context () const noexcept
    {
        return copyXMsg ( ctx_msg );
    }

    const XMsg Exception :: cause () const noexcept
    {
        return copyXMsg ( cause_msg );
    }

    const XMsg Exception :: suggestion () const noexcept
    {
        return copyXMsg ( suggest_msg );
    }

    const XMsg Exception :: file () const noexcept
    {
        return copyXMsg ( file_name );
    }

    unsigned int Exception :: line () const noexcept
    {
        return lineno;
    }

    const XMsg Exception :: function () const noexcept
    {
        return copyXMsg ( func_name );
    }

    ReturnCodes Exception :: status () const noexcept
    {
        return rc;
    }

    Exception & Exception :: operator = ( Exception && x ) noexcept
    {
        // the "stack_frames" are pointers to code
        // and so are constant and not allocations
        for ( int i = 0; i < x . stack_frames; ++ i )
            callstack [ i ] = x . callstack [ i ];
        stack_frames = x . stack_frames;

        prob_msg = std :: move ( x . prob_msg );
        ctx_msg = std :: move ( x . ctx_msg );
        cause_msg = std :: move ( x . cause_msg );
        suggest_msg = std :: move ( x . suggest_msg );
        file_name = x . file_name;
        func_name = x . func_name;
        lineno = x . lineno;
        rc = x . rc;

        return self;
    }

    Exception & Exception :: operator = ( const Exception & x ) noexcept
    {
        for ( int i = 0; i < x . stack_frames; ++ i )
            callstack [ i ] = x . callstack [ i ];
        stack_frames = x . stack_frames;

        prob_msg = std :: move ( x . prob_msg );
        ctx_msg = std :: move ( x . ctx_msg );
        cause_msg = std :: move ( x . cause_msg );
        suggest_msg = std :: move ( x . suggest_msg );
        file_name = x . file_name;
        func_name = x . func_name;
        lineno = x . lineno;
        rc = x . rc;

        return self;
    }

    Exception :: Exception ( Exception && x ) noexcept
        : prob_msg ( std :: move ( x . prob_msg ) )
        , ctx_msg ( std :: move ( x . ctx_msg ) )
        , cause_msg ( std :: move ( x . cause_msg ) )
        , suggest_msg ( std :: move ( x . suggest_msg ) )
        , file_name ( x . file_name )
        , func_name ( x . func_name )
        , lineno ( x . lineno )
        , stack_frames ( x . stack_frames )
        , rc ( x . rc )
    {
        for ( int i = 0; i < x . stack_frames; ++ i )
            callstack [ i ] = x . callstack [ i ];
    }

    Exception :: Exception ( const Exception & x ) noexcept
        : prob_msg ( std :: move ( x . prob_msg ) )
        , ctx_msg ( std :: move ( x . ctx_msg ) )
        , cause_msg ( std :: move ( x . cause_msg ) )
        , suggest_msg ( std :: move ( x . suggest_msg ) )
        , file_name ( x . file_name )
        , func_name ( x . func_name )
        , lineno ( x . lineno )
        , stack_frames ( x . stack_frames )
        , rc ( x . rc )
                        {
        for ( int i = 0; i < x . stack_frames; ++ i )
            callstack [ i ] = x . callstack [ i ];
    }


#if armv7l
    // behavior on RPi seems fine
    const int num_stack_frames_to_skip = 1;
#else
    // the callstack captured by XP::XP() will have
    // itself as the first entry, so might skip it
    // for some reason, I see two of these... weird.
    // maybe it's an x86_64 thing or maybe I saw it
    // on the Mac...
    const int num_stack_frames_to_skip = 2;
#endif

    Exception :: Exception ( const XP & p ) noexcept
        : prob_msg ( std :: move ( p . problem ) )
        , ctx_msg ( std :: move ( p . context ) )
        , cause_msg ( std :: move ( p . cause ) )
        , suggest_msg ( std :: move ( p . suggestion ) )
        , file_name ( p . file_name )
        , func_name ( p . func_name )
        , lineno ( p . lineno )
        , stack_frames ( p . stack_frames - num_stack_frames_to_skip )
        , rc ( p . rc )
    {
        for ( int i = 0; i < stack_frames; ++ i )
            callstack [ i ] = p . callstack [ i + num_stack_frames_to_skip ];
    }

    inline
    void wipe ( XBuf & str ) noexcept
    {
        if ( str . buffer != nullptr && str . bsize != 0 )
        {
            try
            {
                KTRACE ( TRACE_GEEK, "wiping buffer @ %p", str . buffer );
                memset_s ( ( void * ) str . buffer, str . bsize, ' ', str . bsize );
            }
            catch ( ... )
            {
            }
        }
    }

    Exception :: ~ Exception () noexcept
    {
        KTRACE ( TRACE_GEEK, "wiping message buffers" );
        wipe ( prob_msg );
        wipe ( ctx_msg );
        wipe ( cause_msg );
        wipe ( suggest_msg );
        stack_frames = -1;
        KTRACE ( TRACE_GEEK, "destroying Exception" );
    }
}
