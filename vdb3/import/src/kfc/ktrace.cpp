/*

  vdb3.kfc.trace


 */

#include <vdb3/kfc/ktrace.hpp>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <pthread.h>

namespace vdb3
{
    bool dbg_is_child;
    int dbg_trace_fd = 2;
    int dbg_dedicated_log = -1;
    Z32 dbg_trace_level;
    const char * tool_name = "<tool>";
    static pthread_mutex_t crit_section = PTHREAD_MUTEX_INITIALIZER;

    /* capture_tool
     *  captures simple ( leaf ) name of tool
     *  assumes "exe_path" is constant for the life of process
     */
    void capture_tool ( const char * exe_path )
    {
        // record tool name
        if ( exe_path == 0 )
            tool_name = "<unknown>";
        else
        {
            tool_name = :: strrchr ( exe_path, '/' );
            if ( tool_name ++ == 0 )
                tool_name = exe_path;
        }
    }

    /* move_stderr
     *  calls capture_tool() and also duplicates fd 2 to DBG_STDERR
     */
    void move_stderr ( const char * exe_path )
    {
        // move stderr to magic fd
        dup2 ( 2, DBG_STDERR );
        dbg_trace_fd = DBG_STDERR;
        capture_tool ( exe_path );
    }

    /* capture_early_trace_level
     *  scans through the arguments in old-fashioned style
     *  looking for occurrences of options named "short_opt"
     */
    void capture_early_trace_level ( int argc, const char * argv [], char short_opt )
    {
        if ( argv != 0 )
        {
            for ( int i = 1; i < argc; ++ i )
            {
                const char * arg = argv [ i ];
                if ( arg != 0 && arg [ 0 ] == '-' && arg [ 1 ] != '-' )
                {
                    if ( arg [ 1 ] == short_opt )
                    {
                        N32 increment = 1;
                        for ( N32 j = 2; arg [ j ] == short_opt; ++ j )
                            ++ increment;

                        dbg_trace_level += increment;
                    }
                }
            }
        }
    }


    /* dbg_printf
     *  the main print engine
     *  essentially performs sprintf() to a buffer
     *  and then writes the buffer to "dbg_trace_fd"
     */
    static
    void dbg_write ( const void * data, size_t bytes, bool needs_newline ) noexcept
    {
        :: pthread_mutex_lock ( & crit_section );

        int status = :: write ( dbg_trace_fd, data, bytes );
        if ( dbg_dedicated_log >= 0 )
            status = :: write ( dbg_dedicated_log, data, bytes );
        if ( needs_newline )
            status = :: write ( dbg_dedicated_log, "\n", 1 );

        pthread_mutex_unlock ( & crit_section );

        ( void ) status;
    }

    void dbg_printf ( const char * fmt, ... ) noexcept
    {
        va_list args;
        va_start ( args, fmt );

        char buffer [ 4096 ];
        int status = :: vsnprintf ( buffer, sizeof buffer, fmt, args );
        if ( status > 0 && ( size_t ) status < sizeof buffer )
        {
            bool needs_newline = false;
            if ( buffer [ status - 1 ] != '\n' )
            {
                if ( ( size_t ) status + 1 < sizeof buffer )
                    buffer [ status ++ ] = '\n';
                else
                    needs_newline = true;
            }
            
            dbg_write ( buffer, status, needs_newline );
        }

        va_end ( args );
    }

    /* print_trace
     *  the tracing print engine
     *  prints out tool name, function, line number plus trace level
     *  as well as the formatted message
     */
    void print_trace ( Z32 lvl, const char * file, N32 lineno,
        const char * func, const char * fmt, ... ) noexcept
    {
        // convert file path to leaf name
        const char * sep = :: strrchr ( file, '/' );
        if ( sep != 0 )
            file = sep + 1;

        va_list args;
        va_start ( args, fmt );

        char buffer [ 4096 ];
        int status = :: snprintf ( buffer, sizeof buffer
                                   , "[%u]%s%s:%s:%u:%s(): "
                                   , lvl
                                   , tool_name
                                   , dbg_is_child ? "(child)" : ""
                                   , file
                                   , lineno
                                   , func
            );
        if ( status > 0 && ( size_t ) status < sizeof buffer )
        {
            int len = :: vsnprintf ( & buffer [ status ], sizeof buffer - status, fmt, args );
            if ( len > 0 && ( size_t ) len + status < sizeof buffer )
            {
                status += len;
                bool needs_newline = false;
                if ( buffer [ status - 1 ] != '\n' )
                {
                    if ( ( size_t ) status + 1 < sizeof buffer )
                        buffer [ status ++ ] = '\n';
                    else
                        needs_newline = true;
                }
            
                dbg_write ( buffer, status, needs_newline );
            }
        }

        va_end ( args );
    }

    /* queue_trace
     *  the tracing print engine
     *  prints out tool name, function, line number plus trace level
     *  as well as the formatted message
     */
    void queue_trace ( Z32 lvl, const char * file, N32 lineno,
        const char * qname, size_t qsize, const char * func,
        const char * fmt, ... ) noexcept
    {
        // convert file path to leaf name
        const char * sep = :: strrchr ( file, '/' );
        if ( sep != 0 )
            file = sep + 1;

        va_list args;
        va_start ( args, fmt );

        char buffer [ 4096 ];
        int status = :: snprintf ( buffer, sizeof buffer
                                   , "[%.*s:%u]%s%s:%s:%u:%s(): "
                                   , ( int ) qsize, qname
                                   , lvl
                                   , tool_name
                                   , dbg_is_child ? "(child)" : ""
                                   , file
                                   , lineno
                                   , func
            );
        if ( status > 0 && ( size_t ) status < sizeof buffer )
        {
            int len = :: vsnprintf ( & buffer [ status ], sizeof buffer - status, fmt, args );
            if ( len > 0 && ( size_t ) len + status < sizeof buffer )
            {
                status += len;
                bool needs_newline = false;
                if ( buffer [ status - 1 ] != '\n' )
                {
                    if ( ( size_t ) status + 1 < sizeof buffer )
                        buffer [ status ++ ] = '\n';
                    else
                        needs_newline = true;
                }
            
                dbg_write ( buffer, status, needs_newline );
            }
        }

        va_end ( args );
    }

}
