/*

  vdb3.kfc.memset_s

 */

#include <vdb3/kfc/memset_s.hpp>

#include <string>
#include <stdexcept>

#include <stdio.h>


namespace vdb3
{

#ifdef RSIZE_MAX
    const size_t PREDEFINED_RSIZE_MAX = RSIZE_MAX;
#undef RSIZE_MAX
    const size_t RSIZE_MAX = PREDEFINED_RSIZE_MAX;
#else
#if ARCHBITS == 64
    const size_t RSIZE_MAX = ( size_t ) INT64_MAX;
#else
    const size_t RSIZE_MAX = ( size_t ) INT32_MAX;
#endif
#endif

    static
    const std :: string what ( char * buffer, size_t bsize, const char * fmt, ... )
    {
        va_list args;

        va_start ( args, fmt );
        int status = :: vsnprintf ( buffer, bsize, fmt, args );
        va_end ( args );

        if ( status < 0 || ( size_t ) status > bsize )
        {
            status = snprintf ( buffer, bsize, "bad exception parameters" );
            if ( status < 0 )
            {
                buffer [ 0 ] = 0;
                status = 0;
            }
            else if ( ( size_t ) status > bsize )
                status = ( int ) bsize;
        }

        return std :: string ( buffer, status );
    }
    
    /**
     * memset_s
     *  @brief a version of memset() that is not supposed to be optimized away
     *   or apparently even implemented on Linux...
     *
     *  documented errors:
     *   1. dest is a NULL pointer
     *   2. destsz > RSIZE_MAX
     *   3. count > RSIZE_MAX
     *   4. count > destsz
     *
     *  in cases 3 & 4, the buffer from dest+0 .. dest+destsz will be filled
     *  with ch before throwing the exception.
     */
    int memset_s ( void * dest, size_t destsz, int ch, size_t count )
    {
        char buffer [ 256 ];

        // first error is an excessive "destsz"
        // if very large, it looks like garbage
        if ( destsz > RSIZE_MAX )
        {
            throw std :: length_error ( what ( buffer, sizeof buffer,
                "destination size ( %zu ) exceeds hard limit ( %zu )", destsz, RSIZE_MAX ) );
        }

        // all available specs say that "dest" cannot be NULL,
        // but proper behavior is that it cannot be NULL only
        // if "destsz" is not 0, in which case "dest" should be
        // irrelevant.
        if ( dest == nullptr && destsz != 0 )
            throw std :: invalid_argument ( "null pointer" );

        // detect count out of bounds
        if ( count > destsz )
        {
            // fill entire buffer
            :: memset ( dest, ch, destsz );

            // detect this higher limit first
            if ( count > RSIZE_MAX )
            {
                throw std :: length_error ( what ( buffer, sizeof buffer,
                    "fill count ( %zu ) exceeds hard limit ( %zu )", count, RSIZE_MAX ) );
            }

            throw std :: length_error ( what ( buffer, sizeof buffer,
                "fill count ( %zu ) would overflow buffer ( %zu )", count, destsz ) );
        }

        // assumed safe even if "dest" is NULL and "count" is 0
        // or if "dest" is non-NULL but "count" is 0
        :: memset ( dest, ch, count );
        return 0;
    }
}
