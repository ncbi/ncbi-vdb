/*

  vdb3.cmn.dserial-stream

 */

#include <vdb3/cmn/dserial.hpp>
#include <vdb3/cmn/serial.hpp>
#include <vdb3/kfc/rsrc-mem.hpp>

namespace vdb3
{
    
    /*=====================================================*
     *                   DeserialStream                    *
     *=====================================================*/

    size_t DeserialStream :: bytesAvailable () const
    {
        // this is the valid part of the buffer
        MRgn valid = b . getContent ();

        // the bytes that haven't been read yet
        CRgn unread = valid . subRgn ( offset );
        return unread . size ();
    }

    size_t DeserialStream :: readAhead ()
    {
        assert ( offset <= restore );

        // this is the valid part of the buffer
        MRgn valid = b . getContent ();

        // the bytes that haven't been read yet
        CRgn unread = valid . subRgn ( offset );
        if ( unread . size () != 0 )
            
            // shift down within buffer
            valid . copy ( unread );

        // update ( valid ) buffer size
        b . setSize ( unread . size () );

        // offset is zero
        offset = 0;

        // now read into buffer
        MRgn free = b . getBuffer () . subRgn ( b . getSize () );
        CRgn num_read = r . read ( free );

        // update buffer contents
        b . setSize ( b . getSize () + num_read . size () );

        // set contents
        rgn = b . getContent ();

        // return number of bytes read
        return num_read . size ();
    }

    void DeserialStream :: begin ()
    {
        // record wherever offset was
        restore = offset;
    }

    void DeserialStream :: commit ()
    {
        // set restoration so high
        // that it can never be lower than offset
        restore = SIZE_T_MAX;
    }

    void DeserialStream :: rollback ()
    {
        // rollback succeeds only if
        // begin happened but no commit
        if ( restore < offset )
            offset = restore;
        restore = SIZE_T_MAX;
    }

    /*

      MOST, if not all, of this makes no real sense.
      We don't want to copy the region, we don't want
      to copy the buffers...

     */
    DeserialStream & DeserialStream :: operator = ( DeserialStream && d ) noexcept
    {
        Deserial :: operator = ( std :: move ( d ) );
        r = std :: move ( d . r );
        b = std :: move ( d . b );
        restore = std :: move ( d . restore );
        return self;
    }
    
    DeserialStream & DeserialStream :: operator = ( const DeserialStream & d ) noexcept
    {
        Deserial :: operator = ( d );
        r = d . r;
        b = d . b;
        restore = d . restore;
        return self;
    }
    
    DeserialStream :: DeserialStream ( DeserialStream && d ) noexcept
        : Deserial ( std :: move ( d ) )
        , r ( std :: move ( d . r ) )
        , b ( std :: move ( d . b ) )
        , restore ( std :: move ( d . restore ) )
    {
    }
    
    DeserialStream :: DeserialStream ( const DeserialStream & d ) noexcept
        : Deserial ( d )
        , r ( d . r )
        , b ( d . b )
        , restore ( d . restore )
    {
    }
    
    DeserialStream :: DeserialStream ( const RsrcKfc & rsrc, const StreamReader & _r )
        : r ( _r )
        , b ( rsrc )
        , restore ( SIZE_T_MAX )
    {
    }

    DeserialStream :: DeserialStream ( const RsrcKfc & rsrc, size_t bsize, const StreamReader & _r )
        : r ( _r )
        , b ( rsrc, bsize )
        , restore ( SIZE_T_MAX )
    {
    }

    DeserialStream :: ~ DeserialStream () noexcept
    {
    }
    
    size_t DeserialStream :: moreData ( size_t requested )
    {
#if 1
        return 0;
#else
        // current scheme ( 2020-09-26 )
        // is to simply read entire thing into a buffer
        // and not bother with trimming data already read, etc.

        // expand storage if needed
        b . ensureAdditionalCapacity ( requested );

        // get entire buffer
        MRgn mrgn = b . buffer ();

        // get free portion
        MRgn free = mrgn . subRgn ( b . size () );

        // read as much as possible into the region
        CRgn redd = r . read ( free );

        // incorporate this into valid region of buffer
        b . setSize ( b . size () + redd . size () );

        // remember Deserial's current region size
        size_t cur_size = rgn . size ();

        // update the current readable region from buffer
        rgn = b . content ();

        // return the size difference
        return rgn . size () - cur_size;
#endif
    }
}
