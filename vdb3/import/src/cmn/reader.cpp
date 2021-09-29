/*

  vdb3.cmn.reader

 */

#include <vdb3/cmn/reader.hpp>

namespace vdb3
{
    
    /*=====================================================*
     *                       PosReader                     *
     *=====================================================*/

    PosReader & PosReader :: operator = ( PosReader && _r ) noexcept
    {
        r = std :: move ( _r . r );
        return self;
    }
    
    PosReader & PosReader :: operator = ( const PosReader & _r ) noexcept
    {
        r = _r . r;
        return self;
    }
    
    PosReader :: PosReader ( PosReader && _r ) noexcept
        : r ( std :: move ( _r . r ) )
    {
    }
    
    PosReader :: PosReader ( const PosReader & _r ) noexcept
        : r ( _r . r )
    {
    }

    PosReader :: PosReader () noexcept
    {
    }
    
    PosReader :: ~ PosReader () noexcept
    {
    }

    PosReader :: PosReader ( const PosReaderRef & _r ) noexcept
        : r ( _r )
    {
    }

    
    /*=====================================================*
     *                     StreamReader                    *
     *=====================================================*/

    struct Pos2StrmReader
        : Refcount
        , StreamReaderItf
    {
        METH ( CRgn read ( const MRgn & rgn ) )
        {
            CRgn sub = r . read ( rgn, marker );
            marker += sub . size ();
            return sub;
        }
        
        Pos2StrmReader ( const PosReader & rdr, bytes_t _marker ) noexcept
            : r ( rdr )
            , marker ( _marker )
        {
        }

        METH ( ~ Pos2StrmReader () noexcept )
        {
            marker = 0;
        }

        PosReader r;
        bytes_t marker;
    };

    StreamReader :: StreamReader ( const PosReader & rdr )
        : r ( new Pos2StrmReader ( rdr, 0 ) )
    {
    }
    
    StreamReader :: StreamReader ( const PosReaderRef & rdr )
        : r ( new Pos2StrmReader ( rdr, 0 ) )
    {
    }
    
    StreamReader :: StreamReader ( const PosReader & rdr, bytes_t marker )
        : r ( new Pos2StrmReader ( rdr, marker ) )
    {
    }
    
    StreamReader :: StreamReader ( const PosReaderRef & rdr, bytes_t marker )
        : r ( new Pos2StrmReader ( rdr, marker ) )
    {
    }
    
    StreamReader & StreamReader :: operator = ( StreamReader && _r ) noexcept
    {
        r = std :: move ( _r . r );
        return self;
    }
    
    StreamReader & StreamReader :: operator = ( const StreamReader & _r ) noexcept
    {
        r = _r . r;
        return self;
    }
    
    StreamReader :: StreamReader ( StreamReader && _r ) noexcept
        : r ( std :: move ( _r . r ) )
    {
    }
    
    StreamReader :: StreamReader ( const StreamReader & _r ) noexcept
        : r ( _r . r )
    {
    }

    StreamReader :: StreamReader () noexcept
    {
    }
    
    StreamReader :: ~ StreamReader () noexcept
    {
    }

    StreamReader :: StreamReader ( const StreamReaderRef & _r ) noexcept
        : r ( _r )
    {
    }
    
}
