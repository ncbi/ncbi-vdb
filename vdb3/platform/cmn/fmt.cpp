/*

  vdb3.cmn.fmt


 */

#include <vdb3/cmn/fmt.hpp>
#include <vdb3/cmn/nconv.hpp>

namespace vdb3
{

    /*=====================================================*
     *                         Fmt                         *
     *=====================================================*/

    template < class T >
    Fmt & Fmt :: operator << ( const DecFltPoint < T > & val )
    {
        // use a temporary buffer
        ASCII buffer [ 1024 ];
        MText mrgn = makeMText ( buffer, sizeof buffer, 0, 0 );

        // determine precision
        Z32 prec = 6;
        if ( self . have_precision )
            prec = self . min_frac_width;

        // convert
        NConv () . realToText ( mrgn, val, prec );

        // print
        return self << mrgn;
    }

    template Fmt & Fmt :: operator << < Z64 > ( const DecFltPoint < Z64 > & val );
    template Fmt & Fmt :: operator << < Z128 > ( const DecFltPoint < Z128 > & val );

    Fmt & Fmt :: operator << ( const UTF8 * val )
    {
        putTxtRgn ( CTextLiteral ( val ) );
        return self;
    }
    
    Fmt & Fmt :: operator << ( const DayTime & dt )
    {
        MilliSeconds mS ( dt . sec . frac () );
        return self
            << intw ( 2 )
            << dt . hour
            << ':'
            << intw ( 2 )
            << dt . min
            << ':'
            << intw ( 2 )
            << dt . sec . trunc () . toZ64 ()
            << '.'
            << intw ( 3 )
            << mS . toZ64 ()
            ;
    }
    
    Fmt & Fmt :: operator << ( const CalTime & ct )
    {
        MilliSeconds mS ( ct . sec . frac () );
        return self
            << ct . year
            << '/'
            << intw ( 2 )
            << ct . month
            << '/'
            << intw ( 2 )
            << ct . day
            << '-'
            << intw ( 2 )
            << ct . hour
            << ':'
            << intw ( 2 )
            << ct . min
            << ':'
            << intw ( 2 )
            << ct . sec . trunc () . toZ64 ()
            << '.'
            << intw ( 3 )
            << mS . toZ64 ()
            << ( ct . utc ? "UTC" : "" )
            ;
    }

    Fmt & Fmt :: operator << ( const std :: string & val )
    {
        putTxtRgn ( CTextLiteral ( val . data (), val . size () ) );
        return self;
    }

    Fmt & Fmt :: operator << ( const XMsg & val )
    {
        putTxtRgn ( CTextLiteral ( val . zmsg, val . msg_size ) );
        return self;
    }

    void Fmt :: putTxtRgn ( const TextRgn & t )
    {
        writeField ( t, left );
    }
    
    void Fmt :: indent ( Z32 num_tabs )
    {
        if ( -1024 <= num_tabs && num_tabs <= 1024 )
        {
            indentation += num_tabs;
            if ( indentation < 0 )
                indentation = 0;
        }
    }

    Fmt :: Fmt ( const TextStreamWriter & _writer, bool _add_eoln, bool _use_crlf ) noexcept
        : writer ( _writer )
        , indentation ( 0 )
        , tabwidth ( 4 )
        , at_line_start ( true )
        , last_char_is_nl ( false )
        , add_eoln ( _add_eoln )
        , use_crlf ( _use_crlf )
    {
    }
    
    Fmt :: ~ Fmt () noexcept
    {
        flush ();
    }

    Fmt & Fmt :: operator = ( Fmt && fmt ) noexcept
    {
        writer = std :: move ( fmt . writer );
        indentation = std :: move ( fmt . indentation );
        tabwidth = std :: move ( fmt . tabwidth );
        at_line_start = std :: move ( fmt . at_line_start );
        last_char_is_nl = std :: move ( fmt . last_char_is_nl );
        add_eoln = std :: move ( fmt . add_eoln );
        use_crlf = std :: move ( fmt . use_crlf );

        return self;
    }

    Fmt :: Fmt ( Fmt && fmt ) noexcept
        : writer ( std :: move ( fmt . writer ) )
        , indentation ( std :: move ( fmt . indentation ) )
        , tabwidth ( std :: move ( fmt . tabwidth ) )
        , at_line_start ( std :: move ( fmt . at_line_start ) )
        , last_char_is_nl ( std :: move ( fmt . last_char_is_nl ) )
        , add_eoln ( std :: move ( fmt . add_eoln ) )
        , use_crlf ( std :: move ( fmt . use_crlf ) )
    {
    }
        
    void Fmt :: write ( const TextRgn & val )
    {
        // capture TextRgn as a richer CText
        // which is the same structure, but has more functionality
        CText t ( val );

        // ignore empty writes
        if ( ! t . isEmpty () )
        {
            // if there are no indentations going on,
            // just write everything out as-is
            if ( indentation == 0 )
            {
                writeExactly ( t );
                at_line_start = last_char_is_nl = ( t . lastChar () == '\n' );
            }
            else
            {
                // in this case, scan for eoln within text
                CharLoc pos = t . fwdFind ( '\n' );
                while ( t . contains ( pos ) )
                {
                    // advance past eoln
                    pos = t . nextLoc ( pos );
                    
                    // split into two
                    CText left = t . subRgn ( 0, pos );
                    t = t . subRgn ( pos );

                    // indent if at line start
                    if ( at_line_start )
                        indent ();

                    // write the left portion
                    writeExactly ( left );

                    // we KNOW the last character written was '\n'
                    // because of the fwdFind().
                    at_line_start = last_char_is_nl = true;

                    // search ahead
                    pos = t . fwdFind ( '\n' );
                }

                // write any remainder
                if ( ! t . isEmpty () )
                {
                    if ( at_line_start )
                        indent ();

                    writeExactly ( t );
                    
                    // we KNOW the last character written was NOT '\n'
                    // because of the failed fwdFind().
                    at_line_start = last_char_is_nl = false;
                }
            }
        }
    }
    
    void Fmt :: fill ( ASCII ch, count_t repeat )
    {
        // ignore zero-length fills
        if ( repeat == 0 )
            return;
        
        // if the fill character happened to be '\n', then
        // don't bother with indentations. otherwise, indent
        // if have have an indentation and are at line start
        if ( at_line_start && indentation != 0 && ch != '\n' )
            indent ();

        // now, write out the fill character
        // create a fixed capacity region
        UTF8 buffer [ 256 ];
        MRgn rgn = makeMRgn ( buffer, sizeof buffer );

        // fill with character
        // this will fill up to total or 256, whichever is smaller
        count_t total = repeat;
        rgn . subRgn ( 0, ( size_t ) total ) . fill ( ch );

        // convert back to text
        CText txt = makeCText ( rgn, ( count_t ) rgn . size () );

        // write to output as many times as needed to hit everything
        for ( count_t num_writ = 0; num_writ < total; )
        {
            // write out a bunch of spaces
            CText writ = writeAll ( txt . subRgn ( 0, total - num_writ ) );
            if ( writ . size () == 0 )
            {
                throw TransferIncomplete (
                    XP ( XLOC )
                    << xprob
                    << "failed to write fill character to formatter output"
                    << xcause
                    << "wrote "
                    << num_writ
                    << " out of "
                    << total
                    << " total fill bytes"
                    );
            }

            // for newline
            at_line_start = last_char_is_nl = ( ch == '\n' );

            // account for characters written
            num_writ += writ . count ();
        }
    }

    CText Fmt :: writeAll ( const CText & rgn )
    {
        return writer . writeAll ( rgn );
    }

    void Fmt :: writeExactly ( const CText & rgn )
    {
        writer . writeExactly ( rgn );
    }

    void Fmt :: flush ()
    {
        writer . flush ();
    }

    void Fmt :: indent ()
    {
        // need to indent this far
        // usually going to be say 0..16 bytes
        count_t total = indentation * tabwidth;

        // create a fixed capacity region
        UTF8 buffer [ 256 ];
        MRgn rgn = makeMRgn ( buffer, sizeof buffer );

        // fill with spaces
        // this will fill up to total or 256, whichever is smaller
        rgn . subRgn ( 0, ( size_t ) total ) . fill ( ' ' );

        // convert back to text rgn
        CText txt = makeCText ( rgn, ( count_t ) rgn . size () );

        // write to output as many times as needed to hit everything
        for ( count_t num_writ = 0; num_writ < total; )
        {
            // write out a bunch of spaces
            CText writ = writeAll ( txt . subRgn ( 0, total - num_writ ) );
            if ( writ . size () == 0 )
            {
                throw TransferIncomplete (
                    XP ( XLOC )
                    << xprob
                    << "failed to write indentation to formatter output"
                    << xcause
                    << "wrote "
                    << num_writ
                    << " out of "
                    << total
                    << " total indentation bytes"
                    );
            }

            // certainly no longer at a line start
            at_line_start = last_char_is_nl = false;

            // account for characters written
            num_writ += writ . count ();
        }
    }

    // include a newline
    Fmt & eoln ( Fmt & f )
    {
        f << ASCIIChar ( '\n' );
        assert ( f . atLineStart () );
        return f;
    }

    // this is how to indicate end of format message
    // will automatically terminate with eoln if "add_eoln" is true
    Fmt & endm ( Fmt & f )
    {
        // don't generate any further indentation
        f . indentation = 0;

        // add newline if needed
        if ( f . add_eoln && ! f . last_char_is_nl )
            f << ASCIIChar ( '\n' );

        // flush the writer
        f . flush ();

        // reset state
        f . reset ();
        f . tabwidth = 4;
        f . at_line_start = true;
        f . last_char_is_nl = false;

        return f;
    }
}
