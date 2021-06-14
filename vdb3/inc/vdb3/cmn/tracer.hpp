/*

  vdb3.cmn.tracer


 */

#pragma once

#include <vdb3/kfc/ref.hpp>
#include <vdb3/cmn/string.hpp>

namespace vdb3
{

    /*=====================================================*
     *                      TraceQueue                     *
     *=====================================================*/

    /**
     * TraceQueue
     */
    class TraceQueue
    {
    public:

        /**
         * getId
         *  @return numeric queue id
         */
        inline N32 getId () const noexcept
        { return id; }

        /**
         * getName
         *  @return queue name
         */
        inline const String & getName () const noexcept
        { return nm; }


        /*=================================================*
         *                       C++                       *
         *=================================================*/

        TraceQueue & operator = ( TraceQueue && lq ) = delete;
        TraceQueue & operator = ( const TraceQueue & lq ) = delete;
        TraceQueue ( TraceQueue && lq ) = delete;
        TraceQueue ( const TraceQueue & lq ) = delete;

        ~ TraceQueue ();
        
    private:

        TraceQueue ( const String & nm, N32 id ) noexcept;

        String nm;
        N32 id;

        friend class TraceQueueFactory;
    };

    /**
     * TraceQueueId
     */
    typedef SRef < TraceQueue > TraceQueueId;
    
    
    /*=====================================================*
     *                        Tracer                       *
     *=====================================================*/

    /**
     * TracerItf
     */
    interface TracerItf
    {

        /**
         * getQueueId
         *  @brief looks up a queue id by name
         *  @param name is the desired queue name
         *  @return queue id
         */
        MSG ( TraceQueueId getQueueId ( const String & name ) const );

        /**
         * makeQueue
         *  @brief creates a new queue or finds existing one
         *  @param name is the desired queue name
         *  @return queue id
         */
        MSG ( TraceQueueId makeQueue ( const String & name ) );

        /**
         * write
         *  @overload writes a message to default tracing queue
         *  @param lvl the trace level
         *  @param file the filename
         *  @param func the function name
         *  @param line the source lineno
         *  @param msg the UTF-8 text message
         */
        MSG ( void write ( Z32 lvl,
            const char * file, const char * func, N32 line,
            const CText & msg ) const noexcept );

        /**
         * write
         *  @overload writes a message to specific tracing queue
         *  @param qid the queue id
         *  @param lvl the trace level
         *  @param file the filename
         *  @param func the function name
         *  @param line the source lineno
         *  @param msg the UTF-8 text message
         */
        MSG ( void write ( const TraceQueueId & qid, Z32 lvl,
            const char * file, const char * func, N32 line,
            const CText & msg ) const noexcept );

        /**
         * flush
         *  @brief signals backend to flush buffers
         */
        MSG ( void flush () const );


        /*=================================================*
         *                       C++                       *
         *=================================================*/

        virtual ~ TracerItf () noexcept {}
    };

    /**
     * TracerRef
     */
    typedef IRef < TracerItf > TracerRef;

    /**
     * Tracer
     *  @brief it is assumed that the tracer is idepmpotent
     *   in that its operations should be purely constant.
     */
    class Tracer
    {
    public:

        /**
         * getQueueId
         *  @brief looks up a queue id by name
         *  @param name is the desired queue name
         *  @return queue id
         */
        TraceQueueId getQueueId ( const String & name ) const;

        /**
         * makeQueue
         *  @brief creates a new queue or finds existing one
         *  @param name is the desired queue name
         *  @return queue id
         */
        TraceQueueId makeQueue ( const String & name ) const;
        
        /**
         * write
         *  @overload writes a message to default tracing queue
         *  @param lvl the trace level
         *  @param file the filename
         *  @param func the function name
         *  @param line the source lineno
         *  @param msg the UTF-8 text message
         */
        inline void write ( Z32 lvl,
            const char * file, const char * func, N32 line,
            const CText & msg ) const noexcept
        { tracer -> write ( lvl, file, func, line, msg ); }

        /**
         * write
         *  @overload writes a message to specific tracing queue
         *  @param qid the tracing queue
         *  @param lvl the trace level
         *  @param file the filename
         *  @param func the function name
         *  @param line the source lineno
         *  @param msg the UTF-8 text message
         */
        inline void write ( const TraceQueueId & qid, Z32 lvl,
            const char * file, const char * func, N32 line,
            const CText & msg ) const noexcept
        { tracer -> write ( qid, lvl, file, func, line, msg ); }

        /**
         * flush
         *  @brief signals backend to flush buffers
         */
        inline void flush () const
        { tracer -> flush (); }


        /*=================================================*
         *                       C++                       *
         *=================================================*/
        
        CXX_RULE_OF_EIGHT_EQOPS_NE ( Tracer, tracer );

    private:

        TracerRef tracer;
    };

   
    /*=====================================================*
     *                  TraceQueueFactory                  *
     *=====================================================*/

    /**
     * @class TraceQueueFactory
     * @brief a RH mixin class giving an implementation
     *  the ability to create a TraceQueue structure
     */
    class TraceQueueFactory
    {
    protected:

        static inline TraceQueueId makeQueueId ( const String & name, N32 qid ) noexcept
        { return TraceQueueId ( new TraceQueue ( name, qid ) ); }
    };
    


    /*=====================================================*
     *                     EXCEPTIONS                      *
     *=====================================================*/

}
