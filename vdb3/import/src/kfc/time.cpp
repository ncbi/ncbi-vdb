/*

  vdb3.kfc.time

 */

#include <vdb3/kfc/time.hpp>
#include <vdb3/kfc/except.hpp>

namespace vdb3
{
    
    /*=====================================================*
     *                      TimeBase                       *
     *=====================================================*/

    const R64 R64_Z64_MAX = Z64_MAX;
    
    // access as real
    R64 TimeBase :: getReal ( Z64 factor ) const
    {
        if ( factor < 0 )
            return R64 ( x ) * ( - factor );
        
        assert ( factor != 0 );
        return R64 ( x ) / factor;
    }
    
    void TimeBase :: setReal ( R64 val, Z64 factor )
    {
        if ( factor < 0 )
        {
            if ( val < 0 )
                x = ( val + factor / 2 ) / ( - factor );
            else
                x = ( val - factor / 2 ) / ( - factor );
        }
        else
        {
            if ( val > R64_Z64_MAX / factor )
            {
                throw RuntimeException (
                    XP ( XLOC )
                    << xprob
                    << "failed to set time value"
                    << xcause
                    << "Real to Integer overflow"
                    );
            }
            
            x = val * factor;
        }
    }
        
    // access as integer
    Z64 TimeBase :: getInt ( Z64 factor ) const
    {
        if ( factor < 0 )
            return x * ( - factor );
        
        assert ( factor != 0 );
        if ( x < 0 )
            return ( x - factor / 2 ) / factor;
        
        return ( x + factor / 2 ) / factor;
    }
    
    void TimeBase :: setInt ( Z64 val, Z64 factor )
    {
        if ( factor < 0 )
        {
            if ( val < 0 )
                x = ( val + factor / 2 ) / ( - factor );
            else
                x = ( val - factor / 2 ) / ( - factor );
        }
        else
        {
            if ( val > Z64_MAX / factor )
            {
                throw RuntimeException (
                    XP ( XLOC )
                    << xprob
                    << "failed to set time value"
                    << xcause
                    << "scaled Integer overflow"
                    );
            }

            x = val * factor ;
        }
    }

    void TimeBase :: negate () noexcept
    {
        x = - x;
    }

    // add-assign
    void TimeBase :: add ( R64 val, Z64 factor )
    {
        add ( TimeBase ( val, factor ) );
    }
    
    void TimeBase :: add ( Z64 val, Z64 factor )
    {
        add ( TimeBase ( val, factor ) );
    }
    
    void TimeBase :: add ( const TimeBase & val )
    {
        x += val . x;
    }

    // subtract-assign
    void TimeBase :: sub ( R64 val, Z64 factor )
    {
        sub ( TimeBase ( val, factor ) );
    }
    
    void TimeBase :: sub ( Z64 val, Z64 factor )
    {
        sub ( TimeBase ( val, factor ) );
    }
    
    void TimeBase :: sub ( const TimeBase & val )
    {
        x -= val . x;
    }

    // multiply-assign
    void TimeBase :: mult ( R64 val )
    {
        x *= val;
    }
    
    void TimeBase :: mult ( Z64 val )
    {
        x *= val;
    }

    // divide-assign
    void TimeBase :: div ( R64 val )
    {
        if ( val == 0 )
        {
            throw ArithmeticException (
                XP ( XLOC )
                << "divide by zero"
                );
        }

        x /= val;
    }
    
    void TimeBase :: div ( Z64 val )
    {
        if ( val == 0 )
        {
            throw ArithmeticException (
                XP ( XLOC )
                << "divide by zero"
                );
        }

        x /= val;
    }

    // mod-assign
    void TimeBase :: mod ( Z64 val )
    {
        if ( val < 0 )
            x = 0;
        else
        {
            if ( val == 0 )
            {
                throw ArithmeticException (
                    XP ( XLOC )
                    << "divide by zero"
                    );
            }

            x %= val;
        }
    }

    // truncate, round & ceil
    void TimeBase :: trunc ( Z64 val )
    {
        if ( val > 0 )
        {
            div ( val );
            mult ( val );
        }
    }
    
    void TimeBase :: round ( Z64 val )
    {
        if ( val > 0 )
        {
            if ( x < 0 )
                x -= val / 2;
            else
                x += val / 2;
        
            div ( val );
            mult ( val );
        }
    }
    
    void TimeBase :: ceil ( Z64 val )
    {
        if ( val > 0 )
        {
            if ( x < 0 )
                x -= val - 1;
            else
                x += val - 1;
        
            div ( val );
            mult ( val );
        }
    }

    // relational
    bool TimeBase :: lt ( R64 val, Z64 factor ) const noexcept
    {
        return lt ( TimeBase ( val, factor ) );
    }
    
    bool TimeBase :: lt ( Z64 val, Z64 factor ) const noexcept
    {
        return lt ( TimeBase ( val, factor ) );
    }
    
    bool TimeBase :: lt ( const TimeBase & val ) const noexcept
    {
        return x < val . x;
    }
    
    bool TimeBase :: eq ( R64 val, Z64 factor ) const noexcept
    {
        return eq ( TimeBase ( val, factor ) );
    }
    
    bool TimeBase :: eq ( Z64 val, Z64 factor ) const noexcept
    {
        return eq ( TimeBase ( val, factor ) );
    }
    
    bool TimeBase :: eq ( const TimeBase & val ) const noexcept
    {
        return x == val . x;
    }
    
    bool TimeBase :: gt ( R64 val, Z64 factor ) const noexcept
    {
        return gt ( TimeBase ( val, factor ) );
    }
    
    bool TimeBase :: gt ( Z64 val, Z64 factor ) const noexcept
    {
        return gt ( TimeBase ( val, factor ) );
    }
    
    bool TimeBase :: gt ( const TimeBase & val ) const noexcept
    {
        return x > val . x;
    }

    // copy-assign
    void TimeBase :: copy ( R64 val, Z64 factor )
    {
        copy ( TimeBase ( val, factor ) );
    }
    
    void TimeBase :: copy ( Z64 val, Z64 factor )
    {
        copy ( TimeBase ( val, factor ) );
    }
    
    void TimeBase :: copy ( const TimeBase & val ) noexcept
    {
        x = val . x;
    }

    // copy-constructor
    TimeBase :: TimeBase ( R64 val, Z64 factor )
    {
        setReal ( val, factor );
    }
    
    TimeBase :: TimeBase ( Z64 val, Z64 factor )
    {
        setInt ( val, factor );
    }
    
    TimeBase :: TimeBase ( const TimeBase & val ) noexcept
        : x ( val . x )
    {
    }

    // default constructor/destructor
    TimeBase :: TimeBase () noexcept
        : x ( 0 )
    {
    }
    
    TimeBase :: ~ TimeBase () noexcept
    {
        x = 0;
    }

    /*=====================================================*
     *                       DayTime                       *
     *=====================================================*/

    DayTime :: DayTime () noexcept
        : sec ( 0 )
        , hour ( 0 )
        , min ( 0 )
    {
    }

    /*=====================================================*
     *                       CalTime                       *
     *=====================================================*/

    DayTime CalTime :: toDayTime () const noexcept
    {
        DayTime dt;
        dt . sec = self . sec;
        dt . hour = self . hour;
        dt . min = self . min;
        return dt;
    }

    CalTime :: CalTime () noexcept
        : sec ( 0 )
        , year ( 0 )
        , month ( 0 )
        , day ( 0 )
        , weekday ( 0 )
        , hour ( 0 )
        , min ( 0 )
        , utc ( false )
        , dst ( false )
    {
    }
    
}
