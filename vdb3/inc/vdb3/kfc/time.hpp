/*

  vdb3.kfc.time

 */

#pragma once

#include <vdb3/kfc/defs.hpp>
#include <vdb3/kfc/xp.hpp>

#include <ostream>

/**
 * @file kfc/time.hpp
 * @brief time quantity encapsulations
 *
 *  this file declares a number of different unit types
 *  that are in common use for measuring time, from
 *  nano-seconds to days ( and once we get to years and
 *  beyond, the quantities become irregular ).
 *
 *  there are two related and peculiar qualities about this
 *  implementation that are notable:
 *
 *    1. we use fixed-point rather than floating point
 *
 *    2. all time quantities are stored as Ticks, where
 *       the fixed decimal point is utilized only upon
 *       viewing the Ticks. This makes all time quantities
 *       compatible for arithmetic.
 */

namespace vdb3
{
    
    /*=====================================================*
     *                      TimeBase                       *
     *=====================================================*/

    class TimeBase
    {
    public:

        ~ TimeBase () noexcept;
        
    protected:

        // access as real
        R64 getReal ( Z64 factor ) const;
        void setReal ( R64 val, Z64 factor );
        
        // access as integer
        Z64 getInt ( Z64 factor ) const;
        void setInt ( Z64 val, Z64 factor );

        // negation
        void negate () noexcept;

        // add-assign
        void add ( R64 val, Z64 factor );
        void add ( Z64 val, Z64 factor );
        void add ( const TimeBase & val );

        // subtract-assign
        void sub ( R64 val, Z64 factor );
        void sub ( Z64 val, Z64 factor );
        void sub ( const TimeBase & val );

        // multiply-assign
        void mult ( R64 val );
        void mult ( Z64 val );

        // divide-assign
        void div ( R64 val );
        void div ( Z64 val );

        // mod-assign
        void mod ( Z64 val );

        // truncate, round & ceil
        void trunc ( Z64 val );
        void round ( Z64 val );
        void ceil ( Z64 val );

        // relational
        bool lt ( R64 val, Z64 factor ) const noexcept;
        bool lt ( Z64 val, Z64 factor ) const noexcept;
        bool lt ( const TimeBase & val ) const noexcept;
        bool eq ( R64 val, Z64 factor ) const noexcept;
        bool eq ( Z64 val, Z64 factor ) const noexcept;
        bool eq ( const TimeBase & val ) const noexcept;
        bool gt ( R64 val, Z64 factor ) const noexcept;
        bool gt ( Z64 val, Z64 factor ) const noexcept;
        bool gt ( const TimeBase & val ) const noexcept;

        // copy-assign
        void copy ( R64 val, Z64 factor );
        void copy ( Z64 val, Z64 factor );
        void copy ( const TimeBase & val ) noexcept;

        // copy-constructor
        TimeBase ( R64 val, Z64 factor );
        TimeBase ( Z64 val, Z64 factor );
        TimeBase ( const TimeBase & val ) noexcept;

        // default constructor
        TimeBase () noexcept;

    private:
        
        Z64 x;
    };

    /*=====================================================*
     *                   ScaledTimeBase                    *
     *=====================================================*/

    /**
     * ScaledTimeBase
     * @brief template class for time quantities
     *
     *  all time is represented by a single type, which is
     *  internally stored as a fixed point number, making
     *  them all comparable and compatible.
     *
     *  normal units for establishing the semantics of the "ticks"
     *  are given by a factor via template parameter "F".
     *  nano-seconds could have a factor of 1, micro-seconds
     *  of 1,000, milliseconds of 1,000,000, and so on.
     *
     *  a signed integer allows for some representation of
     *  date/time prior to 1970-01-01 0:00:00 UTC, which is
     *  the meaning of the value 0.
     *
     *  a 64-bit integer can be divided roughly as follows:
     *    30 bits to represent nano-seconds, or
     *    20 bits to represent micro-seconds
     *     6 bits for seconds
     *     6 bits for minutes
     *     5 bits for hours
     *     9 bits for days
     *     7 bits for a century worth of years
     *  that's rough, but it works out to a little over 292 years,
     *  or from years 1678 AD .. 2262 AD.
     *
     *  if "F" is allowed to go negative, it represents a
     *  divisor rather than factor, e.g. -10 means that every
     *  tick represents 10nS, i.e. it shifts the decimal
     *  point off the right edge of the fixed-point number.
     *
     *  using -10 as F for nano-seconds represents a spread
     *  from 954 BC .. 4894 AD. larger spreads are available
     *  to mimic C#, which says each tick is worth 100nS, i.e.
     *  F = -100.
     */
    template < Z64 F >
    class ScaledTimeBase : public TimeBase
    {
    public:

        // access as real numbers
        inline R64 toR64 () const noexcept
        { return TimeBase :: getReal ( F ); }

        // DEPRECATED... just don't want to stir up too much change
        inline R64 getReal () const noexcept
        { return TimeBase :: getReal ( F ); }
        inline void setReal ( R64 units )
        { TimeBase :: setReal ( units, F ); }

        // access as integers
        inline Z64 toZ64 () const noexcept
        { return TimeBase :: getInt ( F ); }

        // DEPRECATED
        inline Z64 getInt () const noexcept
        { return TimeBase :: getInt ( F ); }
        inline void setInt ( Z64 units )
        { TimeBase :: setInt ( units, F ); }

        // access whole units
        inline ScaledTimeBase < F > trunc () const
        { ScaledTimeBase < F > rtn ( self ); rtn . TimeBase :: trunc ( F ); return rtn; }
        inline ScaledTimeBase < F > round () const
        { ScaledTimeBase < F > rtn ( self ); rtn . TimeBase :: round ( F ); return rtn; }
        inline ScaledTimeBase < F > ceil () const
        { ScaledTimeBase < F > rtn ( self ); rtn . TimeBase :: ceil ( F ); return rtn; }

        // access fractional units
        inline ScaledTimeBase < F > frac () const
        { ScaledTimeBase < F > rtn ( self ); rtn . TimeBase :: mod ( F ); return rtn; }

        // cast operator
        inline operator R64 () const
        { return TimeBase :: getReal ( F ); }
        inline operator Z64 () const
        { return TimeBase :: getInt ( F ); }

        // negation
        inline ScaledTimeBase < F > operator - () const noexcept
        { return ScaledTimeBase < F > ( self ) . TimeBase :: negate (); }

        // addition
        inline ScaledTimeBase < F > operator + ( float val ) const
        { return ScaledTimeBase < F > ( self ) += ( R64 ) val; }
        inline ScaledTimeBase < F > operator + ( double val ) const
        { return ScaledTimeBase < F > ( self ) += ( R64 ) val; }
        inline ScaledTimeBase < F > operator + ( short int val ) const
        { return ScaledTimeBase < F > ( self ) += ( Z64 ) val; }
        inline ScaledTimeBase < F > operator + ( int val ) const
        { return ScaledTimeBase < F > ( self ) += ( Z64 ) val; }
        inline ScaledTimeBase < F > operator + ( long int val ) const
        { return ScaledTimeBase < F > ( self ) += ( Z64 ) val; }
        inline ScaledTimeBase < F > operator + ( long long int val ) const
        { return ScaledTimeBase < F > ( self ) += ( Z64 ) val; }
        inline ScaledTimeBase < F > operator + ( const TimeBase & val ) const
        { return ScaledTimeBase < F > ( self ) += val; }

        // subtraction
        inline ScaledTimeBase < F > operator - ( float val ) const
        { return ScaledTimeBase < F > ( self ) -= ( R64 ) val; }
        inline ScaledTimeBase < F > operator - ( double val ) const
        { return ScaledTimeBase < F > ( self ) -= ( R64 ) val; }
        inline ScaledTimeBase < F > operator - ( short int val ) const
        { return ScaledTimeBase < F > ( self ) -= ( Z64 ) val; }
        inline ScaledTimeBase < F > operator - ( int val ) const
        { return ScaledTimeBase < F > ( self ) -= ( Z64 ) val; }
        inline ScaledTimeBase < F > operator - ( long int val ) const
        { return ScaledTimeBase < F > ( self ) -= ( Z64 ) val; }
        inline ScaledTimeBase < F > operator - ( long long int val ) const
        { return ScaledTimeBase < F > ( self ) -= ( Z64 ) val; }
        inline ScaledTimeBase < F > operator - ( const TimeBase & val ) const
        { return ScaledTimeBase < F > ( self ) -= val; }

        // multiplication
        inline ScaledTimeBase < F > operator * ( float val ) const
        { return ScaledTimeBase < F > ( self ) *= ( R64 ) val; }
        inline ScaledTimeBase < F > operator * ( double val ) const
        { return ScaledTimeBase < F > ( self ) *= ( R64 ) val; }
        inline ScaledTimeBase < F > operator * ( short int val ) const
        { return ScaledTimeBase < F > ( self ) *= ( Z64 ) val; }
        inline ScaledTimeBase < F > operator * ( int val ) const
        { return ScaledTimeBase < F > ( self ) *= ( Z64 ) val; }
        inline ScaledTimeBase < F > operator * ( long int val ) const
        { return ScaledTimeBase < F > ( self ) *= ( Z64 ) val; }
        inline ScaledTimeBase < F > operator * ( long long int val ) const
        { return ScaledTimeBase < F > ( self ) *= ( Z64 ) val; }

        // division
        inline ScaledTimeBase < F > operator / ( float val ) const
        { return ScaledTimeBase < F > ( self ) /= ( R64 ) val; }
        inline ScaledTimeBase < F > operator / ( double val ) const
        { return ScaledTimeBase < F > ( self ) /= ( R64 ) val; }
        inline ScaledTimeBase < F > operator / ( short int val ) const
        { return ScaledTimeBase < F > ( self ) /= ( Z64 ) val; }
        inline ScaledTimeBase < F > operator / ( int val ) const
        { return ScaledTimeBase < F > ( self ) /= ( Z64 ) val; }
        inline ScaledTimeBase < F > operator / ( long int val ) const
        { return ScaledTimeBase < F > ( self ) /= ( Z64 ) val; }
        inline ScaledTimeBase < F > operator / ( long long int val ) const
        { return ScaledTimeBase < F > ( self ) /= ( Z64 ) val; }

        // relational <
        inline bool operator < ( float val ) const noexcept
        { return TimeBase :: lt ( ( R64 ) val, F ); }
        inline bool operator < ( double val ) const noexcept
        { return TimeBase :: lt ( ( R64 ) val, F ); }
        inline bool operator < ( short int val ) const noexcept
        { return TimeBase :: lt ( ( Z64 )  val, F ); }
        inline bool operator < ( int val ) const noexcept
        { return TimeBase :: lt ( ( Z64 )  val, F ); }
        inline bool operator < ( long int val ) const noexcept
        { return TimeBase :: lt ( ( Z64 ) val, F ); }
        inline bool operator < ( long long int val ) const noexcept
        { return TimeBase :: lt ( ( Z64 ) val, F ); }
        inline bool operator < ( const TimeBase & val ) const noexcept
        { return TimeBase :: lt ( val ); }

        // relational <=
        inline bool operator <= ( float val ) const noexcept
        { return ! TimeBase :: gt ( ( R64 ) val, F ); }
        inline bool operator <= ( double val ) const noexcept
        { return ! TimeBase :: gt ( ( R64 ) val, F ); }
        inline bool operator <= ( short int val ) const noexcept
        { return ! TimeBase :: gt ( ( Z64 )  val, F ); }
        inline bool operator <= ( int val ) const noexcept
        { return ! TimeBase :: gt ( ( Z64 )  val, F ); }
        inline bool operator <= ( long int val ) const noexcept
        { return ! TimeBase :: gt ( ( Z64 ) val, F ); }
        inline bool operator <= ( long long int val ) const noexcept
        { return ! TimeBase :: gt ( ( Z64 ) val, F ); }
        inline bool operator <= ( const TimeBase & val ) const noexcept
        { return ! TimeBase :: gt ( val ); }

        // relational ==
        inline bool operator == ( float val ) const noexcept
        { return TimeBase :: eq ( ( R64 ) val, F ); }
        inline bool operator == ( double val ) const noexcept
        { return TimeBase :: eq ( ( R64 ) val, F ); }
        inline bool operator == ( short int val ) const noexcept
        { return TimeBase :: eq ( ( Z64 )  val, F ); }
        inline bool operator == ( int val ) const noexcept
        { return TimeBase :: eq ( ( Z64 )  val, F ); }
        inline bool operator == ( long int val ) const noexcept
        { return TimeBase :: eq ( ( Z64 ) val, F ); }
        inline bool operator == ( long long int val ) const noexcept
        { return TimeBase :: eq ( ( Z64 ) val, F ); }
        inline bool operator == ( const TimeBase & val ) const noexcept
        { return TimeBase :: eq ( val ); }

        // relational !=
        inline bool operator != ( float val ) const noexcept
        { return ! TimeBase :: eq ( ( R64 ) val, F ); }
        inline bool operator != ( double val ) const noexcept
        { return ! TimeBase :: eq ( ( R64 ) val, F ); }
        inline bool operator != ( short int val ) const noexcept
        { return ! TimeBase :: eq ( ( Z64 )  val, F ); }
        inline bool operator != ( int val ) const noexcept
        { return ! TimeBase :: eq ( ( Z64 )  val, F ); }
        inline bool operator != ( long int val ) const noexcept
        { return ! TimeBase :: eq ( ( Z64 ) val, F ); }
        inline bool operator != ( long long int val ) const noexcept
        { return ! TimeBase :: eq ( ( Z64 ) val, F ); }
        inline bool operator != ( const TimeBase & val ) const noexcept
        { return ! TimeBase :: eq ( val ); }

        // relational >=
        inline bool operator >= ( float val ) const noexcept
        { return ! TimeBase :: lt ( ( R64 ) val, F ); }
        inline bool operator >= ( double val ) const noexcept
        { return ! TimeBase :: lt ( ( R64 ) val, F ); }
        inline bool operator >= ( short int val ) const noexcept
        { return ! TimeBase :: lt ( ( Z64 )  val, F ); }
        inline bool operator >= ( int val ) const noexcept
        { return ! TimeBase :: lt ( ( Z64 )  val, F ); }
        inline bool operator >= ( long int val ) const noexcept
        { return ! TimeBase :: lt ( ( Z64 ) val, F ); }
        inline bool operator >= ( long long int val ) const noexcept
        { return ! TimeBase :: lt ( ( Z64 ) val, F ); }
        inline bool operator >= ( const TimeBase & val ) const noexcept
        { return ! TimeBase :: lt ( val ); }

        // relational >
        inline bool operator > ( float val ) const noexcept
        { return TimeBase :: gt ( ( R64 ) val, F ); }
        inline bool operator > ( double val ) const noexcept
        { return TimeBase :: gt ( ( R64 ) val, F ); }
        inline bool operator > ( short int val ) const noexcept
        { return TimeBase :: gt ( ( Z64 )  val, F ); }
        inline bool operator > ( int val ) const noexcept
        { return TimeBase :: gt ( ( Z64 )  val, F ); }
        inline bool operator > ( long int val ) const noexcept
        { return TimeBase :: gt ( ( Z64 ) val, F ); }
        inline bool operator > ( long long int val ) const noexcept
        { return TimeBase :: gt ( ( Z64 ) val, F ); }
        inline bool operator > ( const TimeBase & val ) const noexcept
        { return TimeBase :: gt ( val ); }

        // add-assign
        inline ScaledTimeBase < F > & operator += ( float val )
        { TimeBase :: add ( ( R64 ) val, F ); return self; }
        inline ScaledTimeBase < F > & operator += ( double val )
        { TimeBase :: add ( ( R64 ) val, F ); return self; }
        inline ScaledTimeBase < F > & operator += ( short int val )
        { TimeBase :: add ( ( Z64 ) val, F ); return self; }
        inline ScaledTimeBase < F > & operator += ( int val )
        { TimeBase :: add ( ( Z64 ) val, F ); return self; }
        inline ScaledTimeBase < F > & operator += ( long int val )
        { TimeBase :: add ( ( Z64 ) val, F ); return self; }
        inline ScaledTimeBase < F > & operator += ( long long int val )
        { TimeBase :: add ( ( Z64 ) val, F ); return self; }
        inline ScaledTimeBase < F > & operator += ( const TimeBase & val )
        { TimeBase :: add ( val ); return self; }

        // subtract-assign
        inline ScaledTimeBase < F > & operator -= ( float val )
        { TimeBase :: sub ( ( R64 ) val, F ); return self; }
        inline ScaledTimeBase < F > & operator -= ( double val )
        { TimeBase :: sub ( ( R64 ) val, F ); return self; }
        inline ScaledTimeBase < F > & operator -= ( short int val )
        { TimeBase :: sub ( ( Z64 ) val, F ); return self; }
        inline ScaledTimeBase < F > & operator -= ( int val )
        { TimeBase :: sub ( ( Z64 ) val, F ); return self; }
        inline ScaledTimeBase < F > & operator -= ( long int val )
        { TimeBase :: sub ( ( Z64 ) val, F ); return self; }
        inline ScaledTimeBase < F > & operator -= ( long long int val )
        { TimeBase :: sub ( ( Z64 ) val, F ); return self; }
        inline ScaledTimeBase < F > & operator -= ( const TimeBase & val )
        { TimeBase :: sub ( val ); return self; }

        // multiply-assign
        inline ScaledTimeBase < F > & operator *= ( float val )
        { TimeBase :: mult ( ( R64 ) val ); return self; }
        inline ScaledTimeBase < F > & operator *= ( double val )
        { TimeBase :: mult ( ( R64 ) val ); return self; }
        inline ScaledTimeBase < F > & operator *= ( short int val )
        { TimeBase :: mult ( ( Z64 ) val ); return self; }
        inline ScaledTimeBase < F > & operator *= ( int val )
        { TimeBase :: mult ( ( Z64 ) val ); return self; }
        inline ScaledTimeBase < F > & operator *= ( long int val )
        { TimeBase :: mult ( ( Z64 ) val ); return self; }
        inline ScaledTimeBase < F > & operator *= ( long long int val )
        { TimeBase :: mult ( ( Z64 ) val ); return self; }

        // divide-assign
        inline ScaledTimeBase < F > & operator /= ( float val )
        { TimeBase :: div ( ( R64 ) val ); return self; }
        inline ScaledTimeBase < F > & operator /= ( double val )
        { TimeBase :: div ( ( R64 ) val ); return self; }
        inline ScaledTimeBase < F > & operator /= ( short int val )
        { TimeBase :: div ( ( Z64 ) val ); return self; }
        inline ScaledTimeBase < F > & operator /= ( int val )
        { TimeBase :: div ( ( Z64 ) val ); return self; }
        inline ScaledTimeBase < F > & operator /= ( long int val )
        { TimeBase :: div ( ( Z64 ) val ); return self; }
        inline ScaledTimeBase < F > & operator /= ( long long int val )
        { TimeBase :: div ( ( Z64 ) val ); return self; }

        // copy-assign
        inline ScaledTimeBase < F > & operator = ( float val )
        { TimeBase :: copy ( ( R64 ) val, F ); return self; }
        inline ScaledTimeBase < F > & operator = ( double val )
        { TimeBase :: copy ( ( R64 ) val, F ); return self; }
        inline ScaledTimeBase < F > & operator = ( short int val )
        { TimeBase :: copy ( ( Z64 ) val, F ); return self; }
        inline ScaledTimeBase < F > & operator = ( unsigned short int val )
        { TimeBase :: copy ( ( Z64 ) val, F ); return self; }
        inline ScaledTimeBase < F > & operator = ( int val )
        { TimeBase :: copy ( ( Z64 ) val, F ); return self; }
        inline ScaledTimeBase < F > & operator = ( unsigned int val )
        { TimeBase :: copy ( ( Z64 ) val, F ); return self; }
        inline ScaledTimeBase < F > & operator = ( long int val )
        { TimeBase :: copy ( ( Z64 ) val, F ); return self; }
        inline ScaledTimeBase < F > & operator = ( unsigned long int val )
        { TimeBase :: copy ( ( Z64 ) val, F ); return self; }
        inline ScaledTimeBase < F > & operator = ( long long int val )
        { TimeBase :: copy ( ( Z64 ) val, F ); return self; }
        inline ScaledTimeBase < F > & operator = ( const TimeBase & val ) noexcept
        { TimeBase :: copy ( val ); return self; }

        // copy-constructor
        inline ScaledTimeBase ( float val )
            : TimeBase ( ( R64 ) val, F ) {}
        inline ScaledTimeBase ( double val )
            : TimeBase ( ( R64 ) val, F ) {}
        inline ScaledTimeBase ( short int val )
            : TimeBase ( ( Z64 ) val, F ) {}
        inline ScaledTimeBase ( unsigned short int val )
            : TimeBase ( ( Z64 ) val, F ) {}
        inline ScaledTimeBase ( int val )
            : TimeBase ( ( Z64 ) val, F ) {}
        inline ScaledTimeBase ( unsigned int val )
            : TimeBase ( ( Z64 ) val, F ) {}
        inline ScaledTimeBase ( long int val )
            : TimeBase ( ( Z64 ) val, F ) {}
        inline ScaledTimeBase ( unsigned long int val )
            : TimeBase ( ( Z64 ) val, F ) {}
        inline ScaledTimeBase ( long long int val )
            : TimeBase ( ( Z64 ) val, F ) {}
        inline ScaledTimeBase ( const TimeBase & val ) noexcept
            : TimeBase ( val ) {}

        // default-constructor
        inline ScaledTimeBase () noexcept {}
        
    };

    
    /*=====================================================*
     *                        Ticks                        *
     *=====================================================*/

    typedef ScaledTimeBase < 1 > Ticks;
    
    /**
     * TIME_MIN, TIME_MAX, TIME_EPOCH
     * @brief generates minimum and maximum tick counts
     */
    const Ticks TIME_MIN   = Z64_MIN;
    const Ticks TIME_MAX   = Z64_MAX;
    const Ticks TIME_EPOCH = 0;

    
    /*=====================================================*
     *                     NanoSeconds                     *
     *=====================================================*/

    const Z64 nS_PER_TICK = -10; // 10^^-1
    typedef ScaledTimeBase < nS_PER_TICK > NanoSeconds;
    
    
    /*=====================================================*
     *                     CSharpTicks                     *
     *=====================================================*/

    const Z64 cs_100nS_PER_TICK = 10; // C# uses 100nS per tick
    typedef ScaledTimeBase < cs_100nS_PER_TICK > CSharpTicks;
    
    
    /*=====================================================*
     *                    MicroSeconds                     *
     *=====================================================*/
    
    const Z64 uS_PER_TICK = nS_PER_TICK < 0 ?
        1000 / -nS_PER_TICK : 1000 * nS_PER_TICK;
    typedef ScaledTimeBase < uS_PER_TICK > MicroSeconds;

    
    /*=====================================================*
     *                    MilliSeconds                     *
     *=====================================================*/

    const Z64 mS_PER_TICK = uS_PER_TICK * 1000;
    typedef ScaledTimeBase < mS_PER_TICK > MilliSeconds;

    
    /*=====================================================*
     *                       Seconds                       *
     *=====================================================*/
    
    const Z64 SEC_PER_TICK = mS_PER_TICK * 1000;
    typedef ScaledTimeBase < SEC_PER_TICK > Seconds;

    
    /*=====================================================*
     *                       Minutes                       *
     *=====================================================*/
    
    const Z64 MIN_PER_TICK = SEC_PER_TICK * 60;
    typedef ScaledTimeBase < MIN_PER_TICK > Minutes;

    
    /*=====================================================*
     *                        Hours                        *
     *=====================================================*/
    
    const Z64 HOUR_PER_TICK = MIN_PER_TICK * 60;
    typedef ScaledTimeBase < HOUR_PER_TICK > Hours;

    
    /*=====================================================*
     *                        Days                         *
     *=====================================================*/

    const Z64 DAY_PER_TICK = HOUR_PER_TICK * 24;
    typedef ScaledTimeBase < DAY_PER_TICK > Days;

    /*=====================================================*
     *                      Timestamp                      *
     *=====================================================*/

    class Timestamp : public Seconds
    {
    public:

        // copy-assign
        inline Timestamp & operator = ( float val )
        { Seconds :: operator = ( val ); return self; }
        inline Timestamp & operator = ( double val )
        { Seconds :: operator = ( val ); return self; }
        inline Timestamp & operator = ( short int val )
        { Seconds :: operator = ( val ); return self; }
        inline Timestamp & operator = ( unsigned short int val )
        { Seconds :: operator = ( val ); return self; }
        inline Timestamp & operator = ( int val )
        { Seconds :: operator = ( val ); return self; }
        inline Timestamp & operator = ( unsigned int val )
        { Seconds :: operator = ( val ); return self; }
        inline Timestamp & operator = ( long int val )
        { Seconds :: operator = ( val ); return self; }
        inline Timestamp & operator = ( unsigned long int val )
        { Seconds :: operator = ( val ); return self; }
        inline Timestamp & operator = ( long long int val )
        { Seconds :: operator = ( val ); return self; }
        inline Timestamp & operator = ( const TimeBase & val ) noexcept
        { Seconds :: operator = ( val ); return self; }

        // copy-constructor
        inline Timestamp ( float val )
            : Seconds ( val ) {}
        inline Timestamp ( double val )
            : Seconds ( val ) {}
        inline Timestamp ( short int val )
            : Seconds ( val ) {}
        inline Timestamp ( unsigned short int val )
            : Seconds ( val ) {}
        inline Timestamp ( int val )
            : Seconds ( val ) {}
        inline Timestamp ( unsigned int val )
            : Seconds ( val ) {}
        inline Timestamp ( long int val )
            : Seconds ( val ) {}
        inline Timestamp ( unsigned long int val )
            : Seconds ( val ) {}
        inline Timestamp ( long long int val )
            : Seconds ( val ) {}
        inline Timestamp ( const TimeBase & val ) noexcept
            : Seconds ( val ) {}
        
    };

    /*=====================================================*
     *                       DayTime                       *
     *=====================================================*/

    struct DayTime
    {
        DayTime () noexcept;
        
        Seconds sec;            // 0..59.99999...
        N8 hour;                // 0..23
        N8 min;                 // 0..59
    };

    /*=====================================================*
     *                       CalTime                       *
     *=====================================================*/

    struct CalTime
    {
        DayTime toDayTime () const noexcept;
        
        CalTime () noexcept;
        
        Seconds sec;            // 0..59.99999...
        Z32 year;               // full year, e.g. 2020
        N8 month;               // 1..12
        N8 day;                 // 1..31
        N8 weekday;             // 1..7: 1 = Sunday
        N8 hour;                // 0..23
        N8 min;                 // 0..59
        bool utc;               // true if converted in UTC
        bool dst;               // true if ! utc and date was in DST
    };

    template < Z64 F > inline
    std :: ostream & operator << ( std :: ostream & o, const ScaledTimeBase < F > & tm )
    {
        return o
            << tm . toR64 ()
            ;
    }

    template < Z64 F > inline
    XP & operator << ( XP & xp, const ScaledTimeBase < F > & tm )
    {
        return xp
            << tm . toR64 ()
            ;
    }
}
