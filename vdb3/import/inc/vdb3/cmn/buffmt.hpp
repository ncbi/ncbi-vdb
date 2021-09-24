/*

  vdb3.cmn.buffmt


 */

#pragma once

#include <vdb3/cmn/fmt.hpp>
#include <vdb3/cmn/str-buffer.hpp>

namespace vdb3
{

    /*=====================================================*
     *                     BufferedFmt                     *
     *=====================================================*/

    class BufferedFmt : public Fmt
    {
    public:

        BufferedFmt ( const RsrcKfc & rsrc, const TextStreamWriter & writer,
            bool add_eoln = false, bool use_crlf = false );
        virtual ~ BufferedFmt () noexcept;

    protected:

        //!< write as much as possible to Buffer
        virtual CText writeAll ( const CText & rgn ) override;

        //!< write everything to Buffer or blow exception
        virtual void writeExactly ( const CText & rgn ) override;

        //!< write buffer contents to output and reset
        virtual void flush () override;

    private:
        
        StringBuffer b;
    };
}
