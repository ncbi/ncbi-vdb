/*

  vdb3.cmn.trace-tst

 */

#include "trace.cpp"
#include "tracer.cpp"

#include "buffer.cpp"
#include "writer.cpp"
#include "text-utf8.cpp"
#include "serial.cpp"
#include "dserial.cpp"
#include "string.cpp"
#include "str-iter.cpp"
#include "nconv.cpp"
#include "txt-writer.cpp"
#include "fmt.cpp"
#include "str-buffer.cpp"
#include "buffmt.cpp"

#include <vdb3/test/gtest.hpp>

#include <map>

namespace vdb3
{
    struct FakeTracer
        : Refcount
        , TracerItf
        , TraceQueueFactory
    {
        METH ( TraceQueueId getQueueId ( const String & name ) const )
        {
            auto it = qmap . find ( name );
            if ( it == qmap . end () )
            {
                throw NotFoundException (
                    XP ( XLOC )
                    << xprob
                    << "failed to get queue id"
                    << xcause
                    << "queue '"
                    << name
                    << "' does not exist"
                    );
            }
            
            return TraceQueueId ( it -> second );
        }

        METH ( TraceQueueId makeQueue ( const String & name ) )
        {
            auto it = qmap . find ( name );
            if ( it != qmap . end () )
                return it -> second;

            static std :: atomic < N32 > qids;
            TraceQueueId qid = makeQueueId ( name, ++ qids );
            qmap . emplace ( name, qid );

            return qid;
        }

        METH ( void write ( Z32 lvl, const char * file,
            const char * func, N32 line, const CText & msg ) const noexcept )
        {
            try
            {
                std :: cout
                    << '['
                    << lvl
                    << ']'
                    << tool_name
                    << ( dbg_is_child ? "(child)" : "" )
                    << ':'
                    << file
                    << ':'
                    << line
                    << ':'
                    << func
                    << "(): "
                    ;
            }
            catch ( std :: exception & x )
            {
                std :: cerr
                    << "failed to write prologue to std::cout: "
                    << x . what ()
                    << '\n'
                    ;
            }
            catch ( ... )
            {
                std :: cerr
                    << "failed to write prologue to std::cout: "
                    << "unknown exception"
                    << '\n'
                    ;
            }

            try
            {
                std :: cout << msg;
            }
            catch ( std :: exception & x )
            {
                std :: cerr
                    << "failed to write "
                    << msg . size ()
                    << " bytes of msg body to std::cout: "
                    << x . what ()
                    << '\n'
                    ;
            }
            catch ( ... )
            {
                std :: cerr
                    << "failed to write "
                    << msg . size ()
                    << " bytes of msg body to std::cout: "
                    << "unknown exception"
                    << '\n'
                    ;
            }
            
            std :: cout << '\n';
        }

        METH ( void write ( const TraceQueueId & qid, Z32 lvl, const char * file,
            const char * func, N32 line, const CText & msg ) const noexcept )
        {
            try
            {
                std :: cout
                    << '['
                    << qid -> getName ()
                    << ':'
                    << lvl
                    << ']'
                    << tool_name
                    << ( dbg_is_child ? "(child)" : "" )
                    << ':'
                    << file
                    << ':'
                    << line
                    << ':'
                    << func
                    << "(): "
                    ;
            }
            catch ( std :: exception & x )
            {
                std :: cerr
                    << "failed to write prologue to std::cout: "
                    << x . what ()
                    << '\n'
                    ;
            }
            catch ( ... )
            {
                std :: cerr
                    << "failed to write prologue to std::cout: "
                    << "unknown exception"
                    << '\n'
                    ;
            }

            try
            {
                std :: cout << msg;
            }
            catch ( std :: exception & x )
            {
                std :: cerr
                    << "failed to write "
                    << msg . size ()
                    << " bytes of msg body to std::cout: "
                    << x . what ()
                    << '\n'
                    ;
            }
            catch ( ... )
            {
                std :: cerr
                    << "failed to write "
                    << msg . size ()
                    << " bytes of msg body to std::cout: "
                    << "unknown exception"
                    << '\n'
                    ;
            }
            
            std :: cout << '\n';
        }

        METH ( void flush () const )
        {
        }

        FakeTracer () noexcept
        {
        }

        ~ FakeTracer () noexcept
        {
        }

        // queue names and ids
        std :: map < String, TraceQueueId > qmap;
    };
        
    struct FakeTraceWriter
        : Refcount
        , TextStreamWriterItf
    {
        // called from BufferedFmt
        // when its message is collapsed
        METH ( CText write ( const CText & msg ) )
        {
            if ( ! qid )
                tracer . write ( lvl, file, func, line, msg );
            else
                tracer . write ( qid, lvl, file, func, line, msg );
            return msg;
        }
        
        // called from BufferedFmt
        // either after every write or ...
        METH ( void flush () )
        {
            tracer . flush ();
        }

        TextStreamWriterRef writer ()
        { return TextStreamWriterRef ( this ); }

        FakeTraceWriter ( const Tracer & t, Z32 _lvl,
                const char * _file, const char * _func, N32 _line ) noexcept
            : tracer ( t )
            , file ( _file )
            , func ( _func )
            , line ( _line )
            , lvl ( _lvl )
        {
        }

        FakeTraceWriter ( const Tracer & t, const TraceQueueId _qid, Z32 _lvl,
                const char * _file, const char * _func, N32 _line ) noexcept
            : tracer ( t )
            , qid ( _qid )
            , file ( _file )
            , func ( _func )
            , line ( _line )
            , lvl ( _lvl )
        {
        }

        ~ FakeTraceWriter () noexcept
        {
        }

        // this is where we need to transfer data
        Tracer tracer;

        // an optional TraceQueue id
        TraceQueueId qid;

        // strings
        const char * file;
        const char * func;

        // numbers
        N32 line;

        // the priority
        Z32 lvl;
    };
    
    struct FakeTraceMgr
        : Refcount
        , TraceMgrItf
    {
        
        METH ( Z32 getThreshold () const noexcept )
        {
            return lvl;
        }
        
        METH ( Z32 getThreshold ( const TraceQueueId & qid ) const noexcept )
        {
            try
            {
                return qlvl [ qid -> getId () - 1 ];
            }
            catch ( ... )
            {
            }
            return -1;
        }
        
        METH ( void setThreshold ( Z32 _lvl ) )
        {
            if ( _lvl >= 0 )
                lvl = _lvl;
        }
        
        METH ( void setThreshold ( const TraceQueueId & qid, Z32 _lvl ) )
        {
            if ( _lvl >= 0 )
            {
                // make a log-level entry
                N32 id = qid -> getId ();

                // extend our table as required
                if ( ( size_t ) id > qlvl . size () )
                {
                    qlvl . reserve ( id );
                    for ( N32 i = ( N32 ) qlvl . size (); i < id; ++ i )
                        qlvl . push_back ( -1 );
                }

                // update existing entry
                assert ( id != 0 );
                qlvl [ id - 1 ] = _lvl;
            }
        }
        
        METH ( TextStreamWriterRef msg ( const Tracer & tracer, Z32 level,
            const char * file, const char * func, N32 line ) noexcept )
        {
            return new FakeTraceWriter ( tracer, level, file, func, line );
        }
        
        METH ( TextStreamWriterRef msg ( const Tracer & tracer, const TraceQueueId & qid, Z32 level,
            const char * file, const char * func, N32 line ) noexcept )
        {
            return new FakeTraceWriter ( tracer, qid, level, file, func, line );
        }

        METH ( TraceMgrRef clone () const )
        {
            return new FakeTraceMgr ( self );
        }

        FakeTraceMgr ()
            : lvl ( dbg_trace_level )
        {
        }

        FakeTraceMgr ( const FakeTraceMgr & t )
            : lvl ( t . lvl )
        {
            for ( Z32 l : t . qlvl )
                qlvl . push_back ( l );
        }
        
        virtual ~ FakeTraceMgr () noexcept
        {
        }

        std :: vector < Z32 > qlvl;
        Z32 lvl;
    };

    class TraceTestFixture : public :: testing :: Test
    {
    public:

        virtual void SetUp () override
        {
        }

        virtual void TearDown () override
        {
        }

        TraceTestFixture ()
            : t ( new FakeTracer () )
            , tm ( new FakeTraceMgr () )
        {
        }

    protected:

        RsrcMem rsrc;
        Tracer t;
        TraceMgr tm;

    };
    
    TEST_F ( TraceTestFixture, constructor_destructor )
    {
    }
    
    TEST_F ( TraceTestFixture, set_trace_level_prg )
    {
        EXPECT_NO_THROW ( tm . setThreshold ( TRACE_PRG ) );
        EXPECT_EQ ( tm . getThreshold (), TRACE_PRG );
    }
    
    TEST_F ( TraceTestFixture, say_howdy )
    {
        EXPECT_NO_THROW ( tm . setThreshold ( TRACE_GEEK ) );
        tm . msg ( rsrc, t, TRACE_PRG, TLOC, [] ( Fmt & fmt ) noexcept
                {
                    fmt
                        << "how doo"
                        << endm
                        ;
                }
            );
    }
    
    TEST_F ( TraceTestFixture, say_silent_howdy )
    {
        EXPECT_NO_THROW ( tm . setThreshold ( TRACE_QA ) );
        tm . msg ( rsrc, t, TRACE_PRG, TLOC, [] ( Fmt & fmt ) noexcept
                {
                    fmt
                        << "how doo"
                        << endm
                        ;
                }
            );
    }
    
    TEST_F ( TraceTestFixture, create_queue )
    {
        EXPECT_NO_THROW ( TraceQueueId qid = t . makeQueue ( "blark" ) );
    }
    
    TEST_F ( TraceTestFixture, say_howdy_to_queue )
    {
        TraceQueueId qid = t . makeQueue ( "blark" );
        EXPECT_NO_THROW ( tm . setThreshold ( TRACE_GEEK ) );
        tm . msg ( rsrc, t, qid, TRACE_PRG, TLOC, [] ( Fmt & fmt ) noexcept
                {
                    fmt
                        << "how doo to the new queue"
                        << endm
                        ;
                }
            );
    }
}

extern "C"
{
    int main ( int argc, const char * argv [], const char * envp []  )
    {
        CAPTURE_TOOL ( argv [ 0 ] );
        CAPTURE_EARLY_TRACE_LEVEL ( argc, argv );
        testing :: InitGoogleTest ( & argc, ( char ** ) argv );
        return RUN_ALL_TESTS ();
    }
}
