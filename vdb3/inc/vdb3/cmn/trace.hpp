/*

  vdb3.cmn.trace


 */

#pragma once

#include <vdb3/kfc/ref.hpp>
#include <vdb3/kfc/except.hpp>
#include <vdb3/cmn/buffmt.hpp>
#include <vdb3/cmn/tracer.hpp>

namespace vdb3
{
    
    /*=====================================================*
     *                     TRACE MACROS                    *
     *=====================================================*/

    /**
     * @macro TLOC
     * @brief a trace location giving file, line and function
     */
#define TLOC XLOC

    /**
     * @macro TRACE
     * @brief uses a RsrcTrace block's trace manager
     */
#define TRACE( rsrc, lvl, print )               \
    ( rsrc ) . trace . msg ( rsrc, ( rsrc ) . tracer, lvl, TLOC, print )

    /**
     * @macro TRACEQ
     * @brief uses a RsrcTrace block's trace manager
     */
#define TRACEQ( rsrc, qid, lvl, print )         \
    ( rsrc ) . trace . msg ( rsrc, ( rsrc ) . tracer, qid, lvl, TLOC, print )


    
    /*=====================================================*
     *                       TraceMgr                      *
     *=====================================================*/

    /**
     * TraceMgrItf
     *  @brief provides a formatted interface to execution tracing
     */
    interface TraceMgrItf
    {

        /**
         * getThreshold
         *  @return threshold for issuing trace messages
         */
        MSG ( Z32 getThreshold () const noexcept );
        
        /**
         * getThreshold
         *  @return threshold for issuing trace messages to queue
         */
        MSG ( Z32 getThreshold ( const TraceQueueId & qid ) const noexcept );

        /* setThreshold
         *  @overload set threshold for issuing trace messages
         *  @param level the new threshold for default queue
         */
        MSG ( void setThreshold ( Z32 level ) );

        /* setThreshold
         *  @overload set threshold for issuing trace messages to queue
         *  @param qid queue identifier
         *  @param level the new threshold for indicated queue
         */
        MSG ( void setThreshold ( const TraceQueueId & qid, Z32 level ) );

        /**
         * msg
         *  @overload evaluates a trace-level and executes provided function if applicable
         *  @param tracer the tracing implementation
         *  @param level the messagingthreshold
         *  @param file auto-generated constant NUL-terminated sourcefile path
         *  @param func auto-generated constant NUL-terminate function name
         *  @param line auto-generated sourcefile line number
         *  @return TextStreamWriterRef for generating buffered Fmt
         */
        MSG ( TextStreamWriterRef msg ( const Tracer & tracer, Z32 level,
              const char * file, const char * func, N32 line ) noexcept );

        /**
         * msg
         *  @overload like generic "msg" but to a specific queue
         *  @param tracer the tracing implementation
         *  @param qid trace queue id
         *  @param level the messagingthreshold
         *  @param file auto-generated constant NUL-terminated sourcefile path
         *  @param func auto-generated constant NUL-terminate function name
         *  @param line auto-generated sourcefile line number
         *  @return TextStreamWriterRef for generating buffered Fmt
         */
        MSG ( TextStreamWriterRef msg ( const Tracer & tracer, const TraceQueueId & qid, Z32 level,
            const char * file, const char * func, N32 line ) noexcept );


        /*=================================================*
         *                       C++                       *
         *=================================================*/

        MSG ( IRef < TraceMgrItf > clone () const );

        virtual ~ TraceMgrItf () noexcept {}
    };

    /**
     * TraceMgrRef
     */
    typedef IRef < TraceMgrItf > TraceMgrRef;

    /**
     * TraceMgr
     * @brief provides a formatted interface to tracing
     */
    class TraceMgr
    {
    public:

        /**
         * getThreshold
         *  @return threshold for issuing trace messages
         */
        inline Z32 getThreshold () const noexcept
        { return lvl; }

        /**
         * getThreshold
         *  @return threshold for issuing trace messages to queue
         */
        Z32 getThreshold ( const TraceQueueId & qid ) const noexcept;


        /* setThreshold
         *  @overload set threshold for issuing trace messages
         *  @param level the new threshold for default queue
         */
        void setThreshold ( Z32 level ) const;


        /* setThreshold
         *  @overload set threshold for issuing trace messages to queue
         *  @param qid queue identifier
         *  @param level the new threshold for indicated queue
         */
        void setThreshold ( const TraceQueueId & qid, Z32 level ) const;

        /**
         * msg
         *  @overload evaluates a trace-level and executes provided function if applicable
         *  @param rsrc resource manager for buffered formatter
         *  @param tracer tracing output resource
         *  @param level execute "print" iff <= thread's current tracing level
         *  @param file a NUL-terminated source file path
         *  @param func a NUL-terminated function name
         *  @param line source line within file
         *  @param print function to generate formatted trace data
         */
        template < class T >
        void msg ( const RsrcKfc & rsrc, const Tracer & tracer, Z32 level,
            const char * file, const char * func, N32 line, T print ) const noexcept
        {
            if ( level >= 0 && level <= getThreshold () ) try
            {
                BufferedFmt fmt ( rsrc, tm -> msg ( tracer, level, file, func, line ) );
                print ( fmt );
            }
            catch ( ... )
            {
            }
        }

        /**
         * msg
         *  @overload evaluates a trace-level and writes to tracer if applicable
         *  @param rsrc resource manager for buffered formatter
         *  @param tracer tracing output resource
         *  @param level execute "print" iff <= thread's current tracing level
         *  @param file a NUL-terminated source file path
         *  @param func a NUL-terminated function name
         *  @param line source line within file
         *  @param zstr a line of text to print to tracer
         */
        void msg ( const RsrcKfc & rsrc, const Tracer & tracer, Z32 level,
            const char * file, const char * func, N32 line, const UTF8 * zstr ) const noexcept
        {
            if ( level >= 0 && level <= getThreshold () ) try
            {
                BufferedFmt fmt ( rsrc, tm -> msg ( tracer, level, file, func, line ) );
                fmt << zstr << endm;
            }
            catch ( ... )
            {
            }
        }

        /**
         * msg
         *  @overload evaluates a trace-level and executes provided function if applicable
         *  @param rsrc resource manager for buffered formatter
         *  @param tracer tracing output resource
         *  @param qid queue identifier
         *  @param level execute "print" iff <= thread's or queue's current tracing levels
         *  @param file a NUL-terminated source file path
         *  @param func a NUL-terminated function name
         *  @param line source line within file
         *  @param print function to generate formatted trace data
         */
        template < class T >
        void msg ( const RsrcKfc & rsrc, const Tracer & tracer, const TraceQueueId & qid, Z32 level,
            const char * file, const char * func, N32 line, T print ) const noexcept
        {
            if ( level >= 0 ) try
            {
                if ( level <= getThreshold () || level <= getThreshold ( qid ) )
                {
                    BufferedFmt fmt ( rsrc, tm -> msg ( tracer, qid, level, file, func, line ) );
                    print ( fmt );
                }
            }
            catch ( ... )
            {
            }
        }

        /**
         * msg
         *  @overload evaluates a trace-level and writes to tracer if applicable
         *  @param rsrc resource manager for buffered formatter
         *  @param tracer tracing output resource
         *  @param qid queue identifier
         *  @param level output "zstr" iff <= thread's or queue's current tracing levels
         *  @param file a NUL-terminated source file path
         *  @param func a NUL-terminated function name
         *  @param line source line within file
         *  @param zstr a line of text to print to tracer
         */
        void msg ( const RsrcKfc & rsrc, const Tracer & tracer, const TraceQueueId & qid, Z32 level,
            const char * file, const char * func, N32 line, const UTF8 * zstr ) const noexcept
        {
            if ( level >= 0 ) try
            {
                if ( level <= getThreshold () || level <= getThreshold ( qid ) )
                {
                    BufferedFmt fmt ( rsrc, tm -> msg ( tracer, qid, level, file, func, line ) );
                    fmt << zstr << endm;
                }
            }
            catch ( ... )
            {
            }
        }


        /*=================================================*
         *                       C++                       *
         *=================================================*/

        TraceMgrRef clone () const;

        CXX_RULE_OF_EIGHT_EQOPS_NE ( TraceMgr, tm );

    private:

        TraceMgrRef tm;
        mutable Z32 lvl;
    };
    


    /*=====================================================*
     *                     EXCEPTIONS                      *
     *=====================================================*/

}
