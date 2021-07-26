/*

  vdb3.cmn.buffmt


 */

#include <vdb3/cmn/buffmt.hpp>

namespace vdb3
{
    
    BufferedFmt :: BufferedFmt ( const RsrcMem & rsrc,
            const TextStreamWriter & writer, bool add_eoln, bool use_crlf )
        : Fmt ( writer, add_eoln, use_crlf )
        , b ( rsrc )
    {
    }
    
    BufferedFmt :: ~ BufferedFmt () noexcept
    {
    }

    CText BufferedFmt :: writeAll ( const CText & rgn )
    {
        b . append ( rgn );
        return rgn;
    }
    
    void BufferedFmt :: writeExactly ( const CText & rgn )
    {
        b . append ( rgn );
    }

    void BufferedFmt :: flush ()
    {
        Fmt :: writeExactly ( b . data () );
        b . clear ();
        Fmt :: flush ();
    }
}
