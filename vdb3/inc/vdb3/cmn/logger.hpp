/*

  vdb3.cmn.logger


 */

#pragma once

#include <vdb3/kfc/ref.hpp>
#include <vdb3/cmn/string.hpp>

namespace vdb3
{

    
    /*=====================================================*
     *                       LogQueue                      *
     *=====================================================*/

    /**
     * LogQueue
     */
    class LogQueue
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

        LogQueue & operator = ( LogQueue && lq ) = delete;
        LogQueue & operator = ( const LogQueue & lq ) = delete;
        LogQueue ( LogQueue && lq ) = delete;
        LogQueue ( const LogQueue & lq ) = delete;

        ~ LogQueue ();
        
    private:

        LogQueue ( const String & nm, N32 id ) noexcept;

        String nm;
        N32 id;

        friend class LogQueueFactory;
    };

    /**
     * LogQueueId
     */
    typedef SRef < LogQueue > LogQueueId;

    
    /*=====================================================*
     *                       LogLevel                      *
     *=====================================================*/

    /**
     * @enum LogLevel
     * @brief log priority levels taken from <sys/syslog.h>
     */
    enum LogLevel
    {
        LOG_INVALID = -1,       // an invalid level
#undef LOG_EMERG
        LOG_EMERG = 0,          // system is unusable
#undef LOG_ALERT
        LOG_ALERT = 1,          // action must be taken immediately
#undef LOG_CRIT
        LOG_CRIT = 2,           // critical conditions
#undef LOG_ERR
        LOG_ERR = 3,            // error conditions
#undef LOG_WARNING
        LOG_WARN = 4,           // warning conditions
        LOG_WARNING = 4,
#undef LOG_NOTICE
        LOG_NOTICE = 5,         // normal but significant condition
#undef LOG_INFO
        LOG_INFO = 6,           // informational
#undef LOG_DEBUG
        LOG_DEBUG = 7           // debug-level messages
    };

    
    /*=====================================================*
     *                        Logger                       *
     *=====================================================*/

    /**
     * LoggerItf
     * @brief interace to a logger implementation
     *
     *  TBD - this should probably be backfitted with a Rsrc block
     *        e.g. during the write calls; which take binary memory
     *        regions rather than text regions due to plumbing.
     */
    interface LoggerItf
    {

        /**
         * getHostname
         * @return the hostname used to configure logger
         */
        MSG ( String getHostname () const noexcept );

        /**
         * getProcname
         * @return the process name used to configure logger
         */
        MSG ( String getProcname () const noexcept );
        
        /**
         * getQueueId
         *  @brief looks up a queue id by name
         *  @param name is the desired queue name
         *  @return queue id
         */
        MSG ( LogQueueId getQueueId ( const String & name ) const );

        /**
         * makeQueue
         *  @brief creates a new queue or finds existing one
         *  @param name is the desired queue name
         *  @return queue id
         */
        MSG ( LogQueueId makeQueue ( const String & name ) );

        /**
         * write
         *  @overload writes a message to default logging queue
         *  @param lvl the priority level
         *  @param pid the current process id
         *  @param ts the timestamp of log event
         *  @param msg the UTF-8 text message of event
         *
         *  The "msg" is sent as a binary CRgn rather than a CText
         *  because of the underlying plumbing that sends the message,
         *  where the data typing is lost. In order to recover it,
         *  the text would have to be measured to count characters
         *  only to write it out in binary form anyway...
         */
        MSG ( void write ( LogLevel lvl, pid_t pid, const Timestamp & ts, const CText & msg ) const );

        /**
         * write
         *  @overload writes a message to specified logging queue
         *  @param qid the queue id
         *  @param lvl the priority level
         *  @param pid the current process id
         *  @param ts the timestamp of log event
         *  @param msg the UTF-8 text message of event
         *
         *  The "msg" is sent as a binary CRgn rather than a CText
         *  because of the underlying plumbing that sends the message,
         *  where the data typing is lost. In order to recover it,
         *  the text would have to be measured to count characters
         *  only to write it out in binary form anyway...
         */
        MSG ( void write ( const LogQueueId & qid, LogLevel lvl,
              pid_t pid, const Timestamp & ts, const CText & msg ) const );

        /**
         * flush
         *  @brief signals backend to flush buffers
         */
        MSG ( void flush () const );


        /*=================================================*
         *                       C++                       *
         *=================================================*/

        virtual ~ LoggerItf () noexcept {}
    };

    /**
     * LoggerRef
     */
    typedef IRef < LoggerItf > LoggerRef;

    /**
     * Logger
     *  @brief API to polymorphic logger implementation
     */
    class Logger
    {
    public:

        /**
         * getHostname
         * @return the hostname used to configure logger
         */
        inline String getHostname () const noexcept
        { return logger -> getHostname (); }

        /**
         * getProcname
         * @return the process name used to configure logger
         */
        inline String getProcname () const noexcept
        { return logger -> getProcname (); }

        /**
         * getQueueId
         *  @brief looks up a queue id by name
         *  @param name is the desired queue name
         *  @return queue id
         */
        LogQueueId getQueueId ( const String & name ) const;

        /**
         * makeQueue
         *  @brief creates a new queue or finds existing one
         *  @param name is the desired queue name
         *  @return queue id
         */
        LogQueueId makeQueue ( const String & name ) const;
        
        /**
         * write
         *  @brief writes a message to default logging queue
         *  @param lvl the priority level
         *  @param pid the current process id
         *  @param ts the timestamp of log event
         *  @param msg the UTF-8 text message of event
         *
         *  The "msg" is sent as a binary CRgn rather than a CText
         *  because of the underlying plumbing that sends the message,
         *  where the data typing is lost. In order to recover it,
         *  the text would have to be measured to count characters
         *  only to write it out in binary form anyway...
         */
        inline void write ( LogLevel lvl, pid_t pid, const Timestamp & ts, const CText & msg ) const
        { logger -> write ( lvl, pid, ts, msg ); }

        /**
         * write
         *  @brief writes a message to default logging queue
         *  @param qid the queue id
         *  @param lvl the priority level
         *  @param pid the current process id
         *  @param ts the timestamp of log event
         *  @param msg the UTF-8 text message of event
         *
         *  The "msg" is sent as a binary CRgn rather than a CText
         *  because of the underlying plumbing that sends the message,
         *  where the data typing is lost. In order to recover it,
         *  the text would have to be measured to count characters
         *  only to write it out in binary form anyway...
         */
        inline void write ( const LogQueueId & qid, LogLevel lvl,
            pid_t pid, const Timestamp & ts, const CText & msg ) const
        { logger -> write ( qid, lvl, pid, ts, msg ); }

        /**
         * flush
         *  @brief signals backend to flush buffers
         */
        inline void flush () const
        { logger -> flush (); }


        /*=================================================*
         *                       C++                       *
         *=================================================*/

        CXX_RULE_OF_EIGHT_EQOPS_NE ( Logger, logger );

    private:

        LoggerRef logger;
    };

   
    /*=====================================================*
     *                   LogQueueFactory                   *
     *=====================================================*/

    /**
     * @class LogQueueFactory
     * @brief a RH mixin class giving an implementation
     *  the ability to create a LogQueue structure
     */
    class LogQueueFactory
    {
    protected:

        static inline LogQueueId makeQueueId ( const String & name, N32 qid ) noexcept
        { return new LogQueue ( name, qid ); }
    };
    


    /*=====================================================*
     *                     EXCEPTIONS                      *
     *=====================================================*/
}
