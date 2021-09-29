/*

  vdb3.cmn.writer

 */

#include <vdb3/cmn/writer.hpp>

namespace vdb3
{

    /*=====================================================*
     *                       PosWriter                     *
     *=====================================================*/

    /**
     * writeAll
     *  @brief writes as many times as possible until all bytes are written
     *  @param rgn has the mapped data to be written
     *  @param pos is starting offset into Object for writing
     *  @return sub-region containing all bytes that were written
     */
    CRgn PosWriter :: writeAll ( const CRgn & rgn, bytes_t pos )
    {
        // write as much as we can
        CRgn total = w -> write ( rgn, pos );

        // if there are unwritten bytes
        if ( total . size () < rgn . size () )
        {
            // and the writer did not already indicate "STOP!" ...
            if ( ! total . isEmpty () )
            {
                do
                {
                    // go into a loop and try to write some more
                    // use an expired timestamp of 0
                    CRgn writ = w -> write
                        ( rgn . subRgn ( total . size () ),
                          pos + total . size () );

                    // stop if writer refuses any more
                    if ( writ . isEmpty () )
                        break;

                    // incorporate this into total
                    // don't use "join" because it cannot detect that it
                    // can safely join two non-overlapping regions.
                    total = rgn . subRgn ( 0, total . size () + writ . size () );
                }
                while ( total . size () < rgn . size () );
            }
        }
        return total;
    }
    
    /**
     * writeExactly
     *  @brief does not return until all bytes are written or an exception
     *  @param rgn has the mapped data to be written
     *  @param pos is starting offset into Object for writing
     */
    void PosWriter :: writeExactly ( const CRgn & rgn, bytes_t pos )
    {
        // starts empty
        CRgn total = rgn . subRgn ( 0, 0 );
        do
        {
            // go into a loop and try to write some more
            CRgn writ = w -> write
                ( rgn . subRgn ( total . size () ),
                  pos + total . size () );

            // stop if writer refuses any more
            if ( writ . isEmpty () )
            {
                throw TransferIncomplete (
                    XP ( XLOC )
                    << xprob
                    << "failed to write all data"
                    << xcause
                    << "wrote "
                    << total . size ()
                    << " out of "
                    << rgn . size ()
                    << " total bytes at offset "
                    << pos
                    );
            }

            // incorporate this into total
            // don't use "join" because it cannot detect that it
            // can safely join two non-overlapping regions.
            total = rgn . subRgn ( 0, total . size () + writ . size () );
        }
        while ( total . size () < rgn . size () );
    }
    
    PosWriter & PosWriter :: operator = ( PosWriter && _w ) noexcept
    {
        w = std :: move ( _w . w );
        return self;
    }
    
    PosWriter & PosWriter :: operator = ( const PosWriter & _w ) noexcept
    {
        w = _w . w;
        return self;
    }
    
    PosWriter :: PosWriter ( PosWriter && _w ) noexcept
        : w ( std :: move ( _w . w ) )
    {
    }
    
    PosWriter :: PosWriter ( const PosWriter & _w ) noexcept
        : w ( _w . w )
    {
    }

    PosWriter :: PosWriter () noexcept
    {
    }
    
    PosWriter :: ~ PosWriter () noexcept
    {
    }

    PosWriter :: PosWriter ( const PosWriterRef & _w ) noexcept
        : w ( _w )
    {
    }


    /*=====================================================*
     *                     StreamWriter                    *
     *=====================================================*/

    struct Pos2StrmWriter
        : Refcount
        , StreamWriterItf
    {
        METH ( CRgn write ( const CRgn & rgn ) )
        {
            CRgn writ = w . write ( rgn, marker );
            marker += writ . size ();
            return writ;
        }
        
        METH ( void flush () )
        {
            w . flush ();
        }
        
        Pos2StrmWriter ( const PosWriter & pw, bytes_t _marker ) noexcept
            : w ( pw )
            , marker ( _marker )
        {
        }

        METH ( ~ Pos2StrmWriter () noexcept )
        {
            marker = 0;
        }

        PosWriter w;
        bytes_t marker;
    };

    /**
     * writeAll
     *  @brief writes as many times as possible until all bytes are written
     *  @param rgn has the mapped data to be written
     *  @return sub-region containing all bytes that were written
     */
    CRgn StreamWriter :: writeAll ( const CRgn & rgn )
    {
        // write as much as we can
        CRgn total = w -> write ( rgn );

        // if there are unwritten bytes
        if ( total . size () < rgn . size () )
        {
            // and the writer did not already indicate "STOP!" ...
            if ( ! total . isEmpty () )
            {
                do
                {
                    // go into a loop and try to write some more
                    CRgn writ = w -> write ( rgn . subRgn ( total . size () ) );

                    // stop if writer refuses any more
                    if ( writ . isEmpty () )
                        break;

                    // incorporate this into total
                    // don't use "join" because it cannot detect that it
                    // can safely join two non-overlapping regions.
                    total = rgn . subRgn ( 0, total . size () + writ . size () );
                }
                while ( total . size () < rgn . size () );
            }
        }
        return total;
    }

    /**
     * writeExactly
     *  @brief does not return until all bytes are written or an exception
     *  @param rgn has the mapped data to be written
     */
    void StreamWriter :: writeExactly ( const CRgn & rgn )
    {
        // starts empty
        CRgn total = rgn . subRgn ( 0, 0 );
        do
        {
            // go into a loop and try to write some more
            CRgn writ = w -> write ( rgn . subRgn ( total . size () ) );

            // stop if writer refuses any more
            if ( writ . isEmpty () )
            {
                throw TransferIncomplete (
                    XP ( XLOC )
                    << xprob
                    << "failed to write all data"
                    << xcause
                    << "wrote "
                    << total . size ()
                    << " out of "
                    << rgn . size ()
                    << " total bytes"
                    );
            }

            // incorporate this into total
            // don't use "join" because it cannot detect that it
            // can safely join two non-overlapping regions.
            total = rgn . subRgn ( 0, total . size () + writ . size () );
        }
        while ( total . size () < rgn . size () );
    }
    
    StreamWriter :: StreamWriter ( const PosWriter & pw )
        : w ( new Pos2StrmWriter ( pw, 0 ) )
    {
    }
    
    StreamWriter :: StreamWriter ( const PosWriterRef & pw )
        : w ( new Pos2StrmWriter ( pw, 0 ) )
    {
    }
    
    StreamWriter :: StreamWriter ( const PosWriter & pw, bytes_t marker )
        : w ( new Pos2StrmWriter ( pw, marker ) )
    {
    }
    
    StreamWriter :: StreamWriter ( const PosWriterRef & pw, bytes_t marker )
        : w ( new Pos2StrmWriter ( pw, marker ) )
    {
    }

    StreamWriter & StreamWriter :: operator = ( StreamWriter && _w ) noexcept
    {
        w = std :: move ( _w . w );
        return self;
    }
    
    StreamWriter & StreamWriter :: operator = ( const StreamWriter & _w ) noexcept
    {
        w = _w . w;
        return self;
    }
    
    StreamWriter :: StreamWriter ( StreamWriter && _w ) noexcept
        : w ( std :: move ( _w . w ) )
    {
    }
    
    StreamWriter :: StreamWriter ( const StreamWriter & _w ) noexcept
        : w ( _w . w )
    {
    }

    StreamWriter :: StreamWriter () noexcept
    {
    }
    
    StreamWriter :: ~ StreamWriter () noexcept
    {
    }
    
    StreamWriter & StreamWriter :: operator = ( const StreamWriterRef & _w ) noexcept
    {
        w = _w;
        return self;
    }

    StreamWriter :: StreamWriter ( const StreamWriterRef & _w ) noexcept
        : w ( _w )
    {
    }
    
}
