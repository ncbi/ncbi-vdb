/*

  vdb3.kfc.unix-tmmgr

 */

#include <vdb3/kfc/tmmgr.hpp>

#include "kfc-priv.hpp"

#include <unistd.h>

namespace vdb3
{

    struct UnixTimeMgrImpl
        : Refcount
        , TimeMgrItf
    {
        
        METH ( Timestamp now () const )
        {
            struct :: timespec ts;
            int status = :: clock_gettime ( CLOCK_REALTIME_COARSE, & ts );
            if ( status != 0 )
            {
                status = errno;
                throw RuntimeException (
                    XP ( XLOC )
                    << xprob
                    << "failed to capture current timestamp"
                    << xcause
                    << syserr ( status )
                    );
            }

            Timestamp tm ( ts . tv_sec );
            NanoSeconds ns ( ts . tv_nsec );
            return tm += ns;
        }

        static CalTime tmToCalTime ( const Seconds & S, struct :: tm const & t, bool is_utc )
        {
            /*

              cheat-sheet for struct tm:

              struct tm {
                  int tm_sec;    // Seconds (0-60 [1 leap second])
                  int tm_min;    // Minutes (0-59)
                  int tm_hour;   // Hours (0-23)
                  int tm_mday;   // Day of the month (1-31)
                  int tm_mon;    // Month (0-11)
                  int tm_year;   // Year - 1900
                  int tm_wday;   // Day of the week (0-6, Sunday = 0)
                  int tm_yday;   // Day in the year (0-365, 1 Jan = 0)
                  int tm_isdst;  // Daylight saving time (-1,0,1)
# ifdef	__USE_MISC
                  long int tm_gmtoff;    // Seconds east of UTC.
                  const char *tm_zone;   // Timezone abbreviation.
# else
                  long int __tm_gmtoff;  // Seconds east of UTC.
                  const char *__tm_zone; // Timezone abbreviation.
# endif
              };

              the meaning of "tm_isdst" is:
                1 = DST in effect
                0 = DST not in effect
               -1 = DST status is unknown

             */

            // copy into struct
            CalTime ctm;
            ctm . sec = t . tm_sec; // capture "leap-second"
            ctm . year = t . tm_year + 1900;
            ctm . month = t . tm_mon + 1;
            ctm . day = t . tm_mday;
            ctm . weekday = t . tm_wday + 1;
            ctm . hour = t . tm_hour;
            ctm . min = t . tm_min;
            ctm . utc = is_utc;
            if ( ! is_utc )
                ctm . dst = ( t . tm_isdst > 0 );

            // add fractional time back in
            ctm . sec += S . frac ();

            return ctm;
        }

        METH ( CalTime local ( const Timestamp & ts ) const )
        {
            // full timestamp
            // this is pedantic - we know Timestamp is in Seconds,
            // but just in case that ever changes, be certain.
            Seconds S ( ts );

            // scrape off the seconds as time_t
            // be certain to truncate, because
            // "toZ64()" on its own rounds.
            :: time_t secs = ( time_t ) S . trunc () . toZ64 ();

            // convert time_t to Unix "tm"
            struct :: tm t;
            :: localtime_r ( & secs, & t );

            // finish conversion
            return tmToCalTime ( S, t, false );
        }
        
        METH ( CalTime utc ( const Timestamp & ts ) const )
        {
            // full timestamp
            Seconds S ( ts );

            // scrape off the seconds as time_t
            :: time_t secs = ( time_t ) S . trunc () . toZ64 ();

            // convert time_t to Unix "tm"
            struct :: tm t;
            :: gmtime_r ( & secs, & t );

            // finish conversion
            return tmToCalTime ( S, t, true );
        }
        
        METH ( NanoSeconds sleep ( const TimeBase & tm ) const )
        {
            struct :: timespec req, rem;
            
            Seconds s ( tm );
            NanoSeconds nS = s . frac ();
            req . tv_sec = ( time_t ) s . toZ64 ();
            req . tv_nsec = ( long ) nS . toZ64 ();

            rem . tv_sec = 0;
            rem . tv_nsec = 0;

            int status = :: nanosleep ( & req, & rem );
            if ( status < 0 )
            {
                status = errno;
                if ( status != EINTR )
                {
                    throw RuntimeException (
                        XP ( XLOC )
                        << xprob
                        << "failed to sleep for "
                        << s . toR64 ()
                        << " seconds"
                        << xcause
                        << syserr ( status )
                        );
                }
            }

            s = rem . tv_sec;
            nS = rem . tv_nsec;
            nS += s;

            return nS;
        }

        UnixTimeMgrImpl () noexcept
        {
        }

        METH ( ~ UnixTimeMgrImpl () noexcept )
        {
        }
    };
    
    TimeMgrRef makeTimeMgr ()
    {
        return new UnixTimeMgrImpl;
    }
}
