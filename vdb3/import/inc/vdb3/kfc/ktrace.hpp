/*

  vdb3.kfc.ktrace


 */

#pragma once

#include <vdb3/kfc/defs.hpp>

/* TRACE and friends
 *  TRACE() is defined as a macro
 *  all of the printf() functionality behaves very much
 *  like the std library printf, so you have to explicitly
 *  add line endings where you want them.
 */

// use this level for the most significant events
// it gives feedback that would potentially be useful
// to non-technical users ( or maybe not... )
const Z32 TRACE_USR = 1;

// use this level for greater detail of events
// it would be useful for power users who have more
// understanding of the mechanics of programs
const Z32 TRACE_PWR = 2;

// use this level for greatest detail of events
// for someone who is performing a QA task
// and probably did not write the code
const Z32 TRACE_QA = 3;

// use this level for greatest detail of events - period.
// it is probably only appreciable by the author
// who wants to have proof that things are working
// as designed.
const Z32 TRACE_PRG = 4;

// it is doubtful that there could be any further detail
// but I'll list it here just in case...
const Z32 TRACE_GEEK = 5;

    // for multi-process applications that manipulate file descriptors,
    // the currently open console fd can be duplicated from 2 up to 99
#define DBG_STDERR 99

    // used by multi-process applications upon child entry from fork()
#define DBG_IS_CHILD() \
    vdb3 :: dbg_is_child = true

    // captures information about the tool for use in TRACE() reporting
    // called immediately upon entry to main()
#define CAPTURE_TOOL( exe_path ) \
    vdb3 :: capture_tool ( exe_path )

    // the main purpose of this file is debug tracing
    // behaves much like DBG_PRINTF() except that output
    // requires "lvl" to be <= current "dbg_trace_level"
    // also prints app-name, file and line.
#define KTRACE( lvl, ... )                                                  \
    if ( ( lvl ) <= vdb3 :: dbg_trace_level ) do {                          \
      vdb3 :: print_trace ( lvl, __FILE__, __LINE__, __func__, __VA_ARGS__ ); \
    } while ( 0 )

#if DEBUG

    // captures tool information like CAPTURE_TOOL() and duplicates
    // fd 2 to DBG_STDERR to keep it open for TRACE() reporting
    // called immediately upon entry to main()
#define MOVE_STDERR( exe_path ) \
    vdb3 :: move_stderr ( exe_path )

    // scans command line for verbosity before the command line
    // is formally parsed. this makes it possible to TRACE()
    // before and into the cmdline parsing
#define CAPTURE_EARLY_TRACE_LEVEL( argc, ... ) \
    vdb3 :: capture_early_trace_level ( argc, __VA_ARGS__ )

    // used by tools when closing down unwanted open fds
    // allows any fd to be closed except the one that's in use
#define DBG_FDCLOSE( fd ) \
    ( ( vdb3 :: dbg_trace_level == 0 || ( fd ) != dbg_trace_fd ) ?  ( void ) close ( fd ) : ( void ) 0 )

    // a means of unconditionally printing to the debugging console
#define DBG_PRINTF( ... ) \
    vdb3 :: dbg_printf ( __VA_ARGS__ )

    // boosting the trace ( verbosity ) level will let more
    // calls to TRACE() succeed
#define INC_TRACE_LEVEL() \
    ++ vdb3 :: dbg_trace_level

#else // DEBUG

    // eat all of the code within these macros
#define MOVE_STDERR( exe_path ) \
    ( ( void ) 0 )
#define CAPTURE_EARLY_TRACE_LEVEL( argc, ... ) \
    ( ( void ) 0 )
#define DBG_FDCLOSE( fd ) \
    ( void ) close ( fd )
#define DBG_PRINTF( ... ) \
    ( ( void ) 0 )
#define INC_TRACE_LEVEL() \
    ( ( void ) 0 )

#endif // DEBUG

namespace vdb3
{
    /* IMPLEMENTATION DETAILS */

    // global data exposed for access from macros
    extern bool dbg_is_child;
    extern int dbg_trace_fd;
    extern int dbg_dedicated_log;
    extern Z32 dbg_trace_level;
    extern const char * tool_name;

    /* capture_tool
     *  captures simple ( leaf ) name of tool
     *  assumes "exe_path" is constant for the life of process
     */
    void capture_tool ( const char * exe_path );

    /* move_stderr
     *  calls capture_tool() and also duplicates fd 2 to DBG_STDERR
     */
    void move_stderr ( const char * exe_path );


    /* capture_early_trace_level
     *  scans through the arguments in old-fashioned style
     *  looking for occurrences of options named "short_opt"
     */
    void capture_early_trace_level ( int argc, const char * argv [], char short_opt = 'v' );

    /* dbg_printf
     *  the main print engine
     *  essentially performs sprintf() to a buffer
     *  and then writes the buffer to "dbg_trace_fd"
     */
    void dbg_printf ( const char * fmt, ... ) noexcept;

    /* print_trace
     *  the tracing print engine
     *  prints out trace level, tool name, line number and function
     *  as well as the formatted message
     */
    void print_trace ( Z32 lvl, const char * file, N32 lineno,
        const char * func, const char * fmt, ... ) noexcept;

    /* queue_trace
     *  the tracing print engine
     *  prints out trace level, tool name, line number, queue name and function
     *  as well as the formatted message
     */
    void queue_trace ( Z32 lvl, const char * file, N32 lineno,
        const char * qname, size_t qsize, const char * func,
        const char * fmt, ... ) noexcept;
}
