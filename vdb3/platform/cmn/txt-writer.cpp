/*

  vdb3.cmn.txt-writer

 */

#include <vdb3/cmn/txt-writer.hpp>

namespace vdb3
{

    struct Strm2TextWriterAdapter
        : Refcount
        , TextStreamWriterItf
        , TextFactory
    {
        METH ( CText write ( const CText & txt ) )
        {
            if ( txt . isAscii () )
            {
                CRgn ascii = r . write ( txt . toCRgn () );
                return makeCText ( ascii, ( count_t ) ascii . size () );
            }

            // text has UTF-8
            CText utf8 = txt;
            
            // attempt to write the whole thing
            CRgn writ = r . writeAll ( txt . toCRgn () );
            if ( writ . size () != utf8 . size () )
            {
                // measure the number of whole characters written
                // pass special flag telling conversion to stop on partial
                CTextLiteral whole ( utf8 . addr (), writ . size (), true );

                // if the whole thing was accepted, then no characters split
                if ( whole . size () == writ . size () )
                    utf8 = makeCText ( writ, whole . count () );
                else
                {
                    // there was a split UTF-8 character... our worst nightmare.
                    // since we're writing to a Stream, we have no option other
                    // than to try to complete this character and then return

                    // test above shows sizes differ - assert this relationship
                    assert ( whole . size () < writ . size () );

                    // get the location of the split character
                    CharLoc pos = whole . end ();

                    // retrieve the character from original
                    assert ( txt . contains ( pos ) );
                    UTF32 ch = txt . getChar ( pos );

                    // convert it from UTF-32 to UTF-8
                    assert ( ch >= 128 );
                    Char c ( ch );
                    assert ( c . size () > 1 );

                    // assert that this character was indeed split
                    assert ( whole . size () + c . size () > writ . size () );

                    // gather bytes remaining
                    CRgn to_write = c . toCRgn () . subRgn ( writ . size () - whole . size () );
                    assert ( to_write . size () > 0 );
                    assert ( to_write . size () < 4 );

                    // write every single one or blow an exception trying
                    r . writeExactly ( to_write );

                    // since we did not fail, we can now incorporate the full character
                    pos = txt . nextLoc ( pos );
                    utf8 = makeCText ( txt . addr (), pos . offset (), pos . index () );

                    /*

                      the above is sort of clunky, and might also be implemented
                      by allowing a CText to have an operation sort of subRgn-like
                      but one that takes a size instead of character count.

                     */
                }
            }

            return utf8;
        }
        
        METH ( void flush () )
        {
            r . flush ();
        }

        Strm2TextWriterAdapter ( const StreamWriter & w ) noexcept
            : r ( w )
        {
        }

        METH ( ~ Strm2TextWriterAdapter () noexcept )
        {
        }

        StreamWriter r;
    };
    
    struct Pos2TextWriterAdapter
        : Refcount
        , TextStreamWriterItf
        , TextFactory
    {
        METH ( CText write ( const CText & txt ) )
        {
            if ( txt . isAscii () )
            {
                CRgn ascii = r . write ( txt . toCRgn (), marker );
                marker += ascii . size ();
                return makeCText ( ascii, ( count_t ) ascii . size () );
            }

            // text has UTF-8
            CText utf8 = txt;
            
            // attempt to write the whole thing
            CRgn writ = r . writeAll ( txt . toCRgn (), marker );
            if ( writ . size () != utf8 . size () )
            {
                // measure the number of whole characters written
                // pass special flag telling conversion to stop on partial
                // this will cause us to attempt to overwrite partial later on
                utf8 = CTextLiteral ( utf8 . addr (), writ . size (), true );
            }

            marker += utf8 . size ();
            return utf8;
        }
        
        METH ( void flush () )
        {
            r . flush ();
        }

        Pos2TextWriterAdapter ( const PosWriter & w, bytes_t _marker ) noexcept
            : r ( w )
            , marker ( _marker )
        {
        }

        METH ( ~ Pos2TextWriterAdapter () noexcept )
        {
            marker = 0;
        }

        PosWriter r;
        bytes_t marker;
    };

    /*=====================================================*
     *                   TextStreamWriter                  *
     *=====================================================*/

    CText TextStreamWriter :: writeAll ( const CText & rgn )
    {
        // write as much as we can
        CText total = r -> write ( rgn );

        // if there are unwritten characters
        if ( total . count () < rgn . count () )
        {
            // and the writer did not already indicate "STOP!" ...
            if ( ! total . isEmpty () )
            {
                do
                {
                    // go into a loop and try to write some more
                    CText writ = r -> write ( rgn . subRgn ( total . end () ) );

                    // stop if writer refuses any more
                    if ( writ . isEmpty () )
                        break;

                    // incorporate this into total
                    // don't use "join" because it cannot detect that it
                    // can safely join two non-overlapping regions.
                    CharLoc end = total . end ();
                    end += writ . end ();
                    total = rgn . subRgn ( 0, end );
                }
                while ( total . count () < rgn . count () );
            }
        }
        return total;
    }
    
    void TextStreamWriter :: writeExactly ( const CText & rgn )
    {
        // write as much as we can
        CText total = rgn . subRgn ( 0, 0 );
        do
        {
            // go into a loop and try to write some more
            CText writ = r -> write ( rgn . subRgn ( total . end () ) );

            // stop if writer refuses any more
            if ( writ . isEmpty () )
            {
                throw TransferIncomplete (
                    XP ( XLOC )
                    << xprob
                    << "failed to write all text"
                    << xcause
                    << "wrote "
                    << total . count ()
                    << " out of "
                    << rgn . count ()
                    << " total characters"
                    );
            }

            // incorporate this into total
            // don't use "join" because it cannot detect that it
            // can safely join two non-overlapping regions.
            CharLoc end = total . end ();
            end += writ . end ();
            total = rgn . subRgn ( 0, end );
        }
        while ( total . count () < rgn . count () );
    }
    
    TextStreamWriter :: TextStreamWriter ( const StreamWriter & w )
        : r ( new Strm2TextWriterAdapter ( w ) )
    {
    }
    
    TextStreamWriter :: TextStreamWriter ( const StreamWriterRef & w )
        : r ( new Strm2TextWriterAdapter ( w ) )
    {
    }
    
    TextStreamWriter :: TextStreamWriter ( const PosWriter & w )
        : r ( new Pos2TextWriterAdapter ( w, 0 ) )
    {
    }
    
    TextStreamWriter :: TextStreamWriter ( const PosWriterRef & w )
        : r ( new Pos2TextWriterAdapter ( w, 0 ) )
    {
    }
    
    TextStreamWriter :: TextStreamWriter ( const PosWriter & w, bytes_t marker )
        : r ( new Pos2TextWriterAdapter ( w, marker ) )
    {
    }
    
    TextStreamWriter :: TextStreamWriter ( const PosWriterRef & w, bytes_t marker )
        : r ( new Pos2TextWriterAdapter ( w, marker ) )
    {
    }

    TextStreamWriter & TextStreamWriter :: operator = ( TextStreamWriter && w ) noexcept
    {
        r = std :: move ( w . r );
        return self;
    }

    TextStreamWriter & TextStreamWriter :: operator = ( const TextStreamWriter & w ) noexcept
    {
        r = w . r;
        return self;
    }

    TextStreamWriter :: TextStreamWriter ( TextStreamWriter && w ) noexcept
        : r ( std :: move ( w . r ) )
    {
    }

    TextStreamWriter :: TextStreamWriter ( const TextStreamWriter & w ) noexcept
        : r ( w . r )
    {
    }

    TextStreamWriter :: TextStreamWriter () noexcept
    {
    }

    TextStreamWriter :: ~ TextStreamWriter () noexcept
    {
    }

    TextStreamWriter & TextStreamWriter :: operator = ( const TextStreamWriterRef & w ) noexcept
    {
        r = w;
        return self;
    }

    TextStreamWriter :: TextStreamWriter ( const TextStreamWriterRef & w ) noexcept
        : r ( w )
    {
    }
}
