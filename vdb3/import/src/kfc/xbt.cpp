/*

  vdb3.kfc.xbt


 */

#include <vdb3/kfc/xbt.hpp>
#include <vdb3/kfc/ktrace.hpp>

#include <cxxabi.h>
#include <execinfo.h>

namespace vdb3
{
    
    /*=====================================================*
     *                       UTILITY                       *
     *=====================================================*/

    //XMsg copyXMsg ( const XBuf & b );
    XMsg copyXMsg ( const char * s );
    //XMsg & concatXMsg ( XMsg & x, const XBuf & b );
    //XMsg & concatXMsg ( XMsg & x, const char * s );
    //UTF8 * reallocate ( UTF8 * old_buffer, size_t & bytes, size_t additional );


    /*=====================================================*
     *                     XBackTrace                      *
     *=====================================================*/

    bool XBackTrace :: isValid () const noexcept
    {
        return cur_frame < num_frames;
    }

    const XMsg XBackTrace :: getName () const noexcept
    {
        if ( cur_frame >= num_frames )
            return copyXMsg ( "<NO MORE FRAMES>" ); 

        XMsg m;
        const char * name = frames [ cur_frame ];

#if armv7l || 1

        /* with -rdynamic enabled, I get some output.
           Here's a sample:

/home/kwr/tst/cmn/buffer-tst-dbg(_ZN7testing4Test3RunEv+0xd4) [0x86694]

           There does not appear to be a frame number, which is useful,
           so that needs to be generated ... don't know how it appears
           on x86_64 until I try it there.

        */
        size_t i, j, k;

        k = sprintf ( m . zmsg, "%u", ( unsigned int ) num_frames );
        j = sprintf ( m . zmsg, "%*u ", ( int ) k, ( unsigned int ) cur_frame );
        for ( i = 0; name [ i ] != 0; ++ i )
        {
            // discard process name
            if (  name [ i ] == '(' )
                break;
        }
        for ( k = ++ i; name [ i ] != 0; ++ i )
        {
            // locate the end of the mangled name
            if ( isspace ( name [ i ] ) || name [ i ] == '+' || name [ i ] == ')' )
                break;
        }

        // see if the name is mangled
        if ( name [ k ] == '_' && name [ k + 1 ] == 'Z' )
        {
            int status;
            char save = name [ i ];
            ( ( char * ) name ) [ i ] = 0;
            char * real_name = abi :: __cxa_demangle ( & name [ k ], 0, 0, & status );
            ( ( char * ) name ) [ i ] = save;


            for ( k = 0; j < sizeof m . zmsg - 1 && real_name [ k ] != 0; ++ j, ++ k )
                m . zmsg [ j ] = real_name [ k ];

            :: free ( real_name );

            for ( ; j < sizeof m . zmsg - 1 && name [ i ] != 0; ++ j, ++ i )
            {
                if ( name [ i ] == ')' )
                {
                    -- j;
                    continue;
                }
                m . zmsg [ j ] = name [ i ];
            }

            m . zmsg [ m . msg_size = j ] = 0;

            return m;
        }

        for ( ; j < sizeof m . zmsg - 1 && name [ k ] != 0; ++ j, ++ k )
        {
            if ( name [ k ] == ')' )
            {
                -- j;
                continue;
            }
            m . zmsg [ j ] = name [ k ];
        }

        m . zmsg [ m . msg_size = j ] = 0;

#else

        size_t i, j, k;
        for ( i = 0; name [ i ] != 0; ++ i )
        {
            // keep frame number
            if ( ! isdigit ( name [ i ] ) )
                break;

            m . zmsg [ i ] = name [ i ];
        }
        for ( ; name [ i ] != 0; ++ i )
        {
            // keep spaces before process name
            if ( ! isspace ( name [ i ] ) )
                break;

            m . zmsg [ i ] = name [ i ];
        }
        for ( j = i; name [ i ] != 0; ++ i )
        {
            // discard process name
            if ( isspace ( name [ i ] ) )
                break;
        }
        for ( ; name [ i ] != 0; ++ i )
        {
            // discard space up to function entrypoint
            if ( ! isspace ( name [ i ] ) )
                break;
        }
        for ( ; name [ i ] != 0; ++ i )
        {
            // discard function entrypoint
            if ( isspace ( name [ i ] ) )
                break;
        }
        for ( k = ++ i; name [ i ] != 0; ++ i )
        {
            // locate the end of the mangled name
            if ( isspace ( name [ i ] ) )
                break;
        }

        // see if the name is mangled
        if ( name [ k ] == '_' && name [ k + 1 ] == 'Z' )
        {
            int status;
            char save = name [ i ];
            ( ( char * ) name ) [ i ] = 0;
            char * real_name = abi :: __cxa_demangle ( & name [ k ], 0, 0, & status );
            ( ( char * ) name ) [ i ] = save;


            for ( k = 0; j < sizeof m . zmsg - 1 && real_name [ k ] != 0; ++ j, ++ k )
                m . zmsg [ j ] = real_name [ k ];

            free ( real_name );

            for ( ; j < sizeof m . zmsg - 1 && name [ i ] != 0; ++ j, ++ i )
                m . zmsg [ j ] = name [ i ];

            m . zmsg [ m . msg_size = j ] = 0;

            return m;
        }

        for ( ; j < sizeof m . zmsg - 1 && name [ k ] != 0; ++ j, ++ k )
            m . zmsg [ j ] = name [ k ];

        m . zmsg [ m . msg_size = j ] = 0;
#endif
        return m;
    }

    bool XBackTrace :: up () const noexcept
    {
        if ( cur_frame + 1 < num_frames )
        {
            ++ cur_frame;
            return true;
        }

        return false;
    }

    XBackTrace & XBackTrace :: operator = ( const XBackTrace & bt ) noexcept
    {
        if ( frames != nullptr )
            :: free ( ( void * ) frames );

        frames = bt . frames;
        num_frames = bt . num_frames;
        cur_frame = bt . cur_frame;

        bt . frames = nullptr;
        bt . num_frames = -1;
        bt . cur_frame = 0;

        return self;
    }

    XBackTrace :: XBackTrace ( const XBackTrace & bt ) noexcept
        : frames ( bt . frames )
        , num_frames ( bt . num_frames )
        , cur_frame ( bt . cur_frame )
    {
        bt . frames = nullptr;
        bt . num_frames = -1;
        bt . cur_frame = 0;
    }

    XBackTrace & XBackTrace :: operator = ( XBackTrace && bt ) noexcept
    {
        if ( frames != nullptr )
            :: free ( ( void * ) frames );

        frames = bt . frames;
        num_frames = bt . num_frames;
        cur_frame = bt . cur_frame;

        bt . frames = nullptr;
        bt . num_frames = -1;
        bt . cur_frame = 0;

        return self;
    }

    XBackTrace :: XBackTrace ( XBackTrace && bt ) noexcept
        : frames ( bt . frames )
        , num_frames ( bt . num_frames )
        , cur_frame ( bt . cur_frame )
    {
        bt . frames = nullptr;
        bt . num_frames = -1;
        bt . cur_frame = 0;
    }

    XBackTrace :: XBackTrace ( const Exception & x ) noexcept
        : frames ( nullptr )
        , num_frames ( -1 )
        , cur_frame ( 0 )
    {
        if ( x . stack_frames > 0 )
        {
            frames = :: backtrace_symbols ( x . callstack, x . stack_frames );
            if ( frames != nullptr )
                num_frames = x . stack_frames;
        }
    }

    XBackTrace :: ~ XBackTrace () noexcept
    {
        if ( frames != nullptr )
        {
            :: free ( ( void * ) frames );
            frames = nullptr;
        }
        num_frames = -1;
        cur_frame = 0;
    }

    std :: ostream & operator << ( std :: ostream & o, const XBackTrace & bt )
    {
        if ( bt . isValid () )
        {
            do
            {
                o
                    << bt . getName ()
                    << '\n'
                    ;
            }
            while ( bt . up () );
        }

        return o;
    }
}
