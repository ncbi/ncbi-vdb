/*

  vdb3.cmn.buffer-tst

 */

#include "buffer.cpp"

#include <vdb3/test/gtest.hpp>

namespace vdb3
{

    class BufferTestFixture : public :: testing :: Test
    {
    public:

        virtual void SetUp () override
        {
        }

        virtual void TearDown () override
        {
        }

        BufferTestFixture ()
            : pay ( rsrc, 0 )
        {
        }

    protected:

        RsrcKfc rsrc;
        Buffer pay;

    };

    TEST_F ( BufferTestFixture, constructor_destructor )
    {
    }
#if 0
    // copy constructor is no longer supported
    TEST_F ( BufferTestFixture, copy_constructor_op )
    {
        Buffer p2 ( pay );
        pay = p2;
    }
#endif
    TEST_F ( BufferTestFixture, move_constructor_op )
    {
        Buffer p1 ( rsrc, 0 );
        Buffer p2 ( std :: move ( p1 ) );
        //Buffer p3 = std :: move ( p2 );

        Buffer p4 ( rsrc, 0 );
    }
    //#endif
    TEST ( BufferTest, construct_with_initial_cap )
    {
        RsrcKfc rsrc;
        Buffer pay ( rsrc, 752 );
    }

    TEST ( BufferTest, access_with_zero_cap )
    {
        RsrcKfc rsrc;
        Buffer pay ( rsrc, 0 );

        // we cannot know that the address of a region
        // will be NULL or not if size is 0 - either are legal
        EXPECT_EQ ( pay . buffer () . size (), 0U );
        EXPECT_EQ ( pay . content () . size (), 0U );
    }
    
    TEST_F ( BufferTestFixture, default_fixture_buffer )
    {
        EXPECT_NE ( pay . buffer () . addr (), nullptr );
        EXPECT_EQ ( pay . buffer () . size (), 0U );
        EXPECT_EQ ( pay . content () . size (), 0U );
    }

    TEST_F ( BufferTestFixture, set_size_beyond_capacity )
    {
        EXPECT_ANY_THROW ( pay . setSize ( 12345 ) );
        EXPECT_EQ ( pay . content () . size (), 0U );
    }

    TEST_F ( BufferTestFixture, increase_capacity )
    {
        pay . increaseCapacity ( 12345 );
        EXPECT_NE ( pay . buffer () . addr (), nullptr );
        EXPECT_EQ ( pay . buffer () . size (), 12345U );
        EXPECT_NE ( pay . content () . addr (), nullptr );
        EXPECT_EQ ( pay . content () . size (), 0U );
    }

    TEST_F ( BufferTestFixture, increase_capacity_and_set_size_beyond_capacity )
    {
        pay . increaseCapacity ( 12345 );
        EXPECT_NE ( pay . buffer () . addr (), nullptr );
        EXPECT_EQ ( pay . buffer () . size (), 12345U );
        EXPECT_NE ( pay . content () . addr (), nullptr );
        EXPECT_EQ ( pay . content () . size (), 0U );

        EXPECT_ANY_THROW ( pay . setSize ( pay . capacity () + 1 ) );
    }

    TEST_F ( BufferTestFixture, increase_capacity_and_set_size_to_capacity )
    {
        pay . increaseCapacity ( 12345 );
        EXPECT_NE ( pay . buffer () . addr (), nullptr );
        EXPECT_EQ ( pay . buffer () . size (), 12345U );
        EXPECT_NE ( pay . content () . addr (), nullptr );
        EXPECT_EQ ( pay . content () . size (), 0U );
        
        pay . setSize ( 12345 );
        EXPECT_EQ ( pay . buffer () . size (), 12345U );
        EXPECT_EQ ( pay . content () . size (), 12345U );
    }

    TEST_F ( BufferTestFixture, fill_wipe_and_reinit )
    {
        pay . increaseCapacity ( 256 );
        EXPECT_EQ ( pay . buffer () . size (), 256U );
        const char msg [] = "secret key data";
        :: memmove ( pay . buffer () . addr (), msg, sizeof msg );
        pay . setSize ( sizeof msg );
        EXPECT_STREQ ( ( const char * ) pay . content () . addr (), msg );
        pay . wipe ();
        EXPECT_STREQ ( ( const char * ) pay . content () . addr (), "" );
        EXPECT_EQ ( pay . content () . size (), 0U );
        EXPECT_EQ ( pay . buffer () . size (), 256U );
        pay . reinitialize ( 0 );
        EXPECT_EQ ( pay . content () . size (), 0U );
        EXPECT_EQ ( pay . buffer () . size (), 0U );
    }

    TEST_F ( BufferTestFixture, fill_and_increase )
    {
        pay . increaseCapacity ( 256 );
        EXPECT_EQ ( pay . buffer () . size (), 256U );
        :: memset ( pay . buffer () . addr (), 'A', pay . buffer () . size () );
        pay . setSize ( pay . buffer () . size () );

        pay . increaseCapacity ( 1234567 );
        EXPECT_EQ ( pay . buffer () . size (), 256U + 1234567U );
        :: memset ( pay . buffer () . addr () + 256, 'B', pay . buffer () . size () - 256 );
        pay . setSize ( pay . buffer () . size () );

        size_t i;
        const char * cp = ( const char * ) pay . content () . addr ();
        for ( i = 0; i < 256U; ++ i )
        {
            EXPECT_EQ ( cp [ i ], 'A' );
        }
        // ** here I manually hoisted the CSE to avoid exceptions on RPi
        size_t end = pay . content () . size ();
        for ( ; i < end; ++ i )
        {
            EXPECT_EQ ( cp [ i ], 'B' );
        }
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
