/*

  vdb3.cmn.str-iter

 */

#include <vdb3/cmn/string.hpp>
#include <vdb3/kfc/mem.hpp>

#include <utf8proc/utf8proc.h>

namespace vdb3
{
    
    /*=====================================================*
     *                       UTILITY                       *
     *=====================================================*/

    static
    void check_valid ( const StringIterator & it )
    {
        if ( ! it . isValid () )
        {
            throw IteratorInvalid (
                XP ( XLOC )
                << xprob
                << "failed to search"
                << xcause
                << "iterator is not within String"
                );
        }
    }

    static
    void check_ctext_overlap ( const CText & a, const CText & b )
    {
        if ( a . toCRgn () . intersect ( b . toCRgn () ) . isEmpty () )
        {
            throw IteratorsUnrelated (
                XP ( XLOC )
                << xprob
                << "failed compare iterators"
                << xcause
                << "iterators are not from same String"
                );
        }
    }
    
    /*=====================================================*
     *                    StringIterator                   *
     *=====================================================*/

    /**
     * fwdFind
     * @overload find and relocate to position of sub string
     * @param sub the substring to locate
     * @return Boolean true if found
     *
     * The idea is to find a substring and position the iterator
     * to point to its first character. If the substring is not found,
     * the iterator remains unchanged.
     */
    bool StringIterator :: fwdFind ( const String & sub )
    {
        // iterator must be somewhere within its region
        check_valid ( self );

        // limit search to current pointer and beyond
        CText fwd_rgn = rgn . subRgn ( cur );

        // search for sub-string
        CharLoc found = fwd_rgn . fwdFind ( sub . data () );

        // test if it was found
        if ( ! fwd_rgn . contains ( found ) )
            return false;

        // translate current location
        cur += found;
        return true;
    }

    /**
     * fwdFind
     * @overload forward search to find a sub-region
     * @param sub the sub-region being sought
     * @return Boolean true if found
     */
    bool StringIterator :: fwdFind ( const Text & sub )
    {
        // iterator must be somewhere within its region
        check_valid ( self );

        // limit search to current pointer and beyond
        CText fwd_rgn = rgn . subRgn ( cur );

        // search for sub-string
        CharLoc found = fwd_rgn . fwdFind ( sub );

        // test if it was found
        if ( ! fwd_rgn . contains ( found ) )
            return false;

        // translate current location
        cur += found;
        return true;
    }

    /**
     * fwdFind
     * @overload forward search to find a single character
     * @param ch a UNICODE character
     * @return Boolean true if found
     */
    bool StringIterator :: fwdFind ( UTF32 ch )
    {
        // iterator must be somewhere within its region
        check_valid ( self );

        // limit search to current pointer and beyond
        CText fwd_rgn = rgn . subRgn ( cur );

        // search for sub-string
        CharLoc found = fwd_rgn . fwdFind ( ch );

        // test if it was found
        if ( ! fwd_rgn . contains ( found ) )
            return false;

        // translate current location
        cur += found;
        return true;
    }

    /**
     * revFind
     * @overload reverse search to find a sub-region
     * @param sub the sub-region being sought
     * @return Boolean true if found
     */
    bool StringIterator :: revFind ( const String & sub )
    {
        // iterator must be somewhere within its region
        check_valid ( self );

        // limit search to current pointer and before
        CText rev_rgn = rgn . subRgn ( 0, cur );

        // search for sub-string
        CharLoc found = rev_rgn . revFind ( sub . data () );

        // test if it was found
        if ( ! rev_rgn . contains ( found ) )
            return false;

        // translate current location
        cur = found;
        return true;
    }

    /**
     * revFind
     * @overload reverse search to find a sub-region
     * @param sub the sub-region being sought
     * @return Boolean true if found
     */
    bool StringIterator :: revFind ( const Text & sub )
    {
        // iterator must be somewhere within its region
        check_valid ( self );

        // limit search to current pointer and before
        CText rev_rgn = rgn . subRgn ( 0, cur );

        // search for sub-string
        CharLoc found = rev_rgn . revFind ( sub );

        // test if it was found
        if ( ! rev_rgn . contains ( found ) )
            return false;

        // translate current location
        cur = found;
        return true;
    }

    /**
     * revFind
     * @overload reverse search to find a single character
     * @param ch a UNICODE character
     * @return Boolean true if found
     */
    bool StringIterator :: revFind ( UTF32 ch )
    {
        // iterator must be somewhere within its region
        check_valid ( self );

        // limit search to current pointer and before
        CText rev_rgn = rgn . subRgn ( 0, cur );

        // search for sub-string
        CharLoc found = rev_rgn . revFind ( ch );

        // test if it was found
        if ( ! rev_rgn . contains ( found ) )
            return false;

        // translate current location
        cur = found;
        return true;
    }
    
    /**
     * findFirstOf
     * @overload forward search to find a character of a set
     * @param cset the set of characters being sought
     * @return Boolean true if found
     */
    bool StringIterator :: findFirstOf ( const String & cset )
    {
        // iterator must be somewhere within its region
        check_valid ( self );

        // limit search to current pointer and beyond
        CText fwd_rgn = rgn . subRgn ( cur );

        // search for sub-string
        CharLoc found = fwd_rgn . findFirstOf ( cset . data () );

        // test if it was found
        if ( ! fwd_rgn . contains ( found ) )
            return false;

        // translate current location
        cur += found;
        return true;
    }
    
    /**
     * findFirstOf
     * @overload forward search to find a character of a set
     * @param cset the set of characters being sought
     * @return Boolean true if found
     */
    bool StringIterator :: findFirstOf ( const Text & cset )
    {
        // iterator must be somewhere within its region
        check_valid ( self );

        // limit search to current pointer and beyond
        CText fwd_rgn = rgn . subRgn ( cur );

        // search for sub-string
        CharLoc found = fwd_rgn . findFirstOf ( cset );

        // test if it was found
        if ( ! fwd_rgn . contains ( found ) )
            return false;

        // translate current location
        cur += found;
        return true;
    }
    
    /**
     * findLastOf
     * @overload reverse search to find a character of a set
     * @param cset the set of characters being sought
     * @return Boolean true if found
     */
    bool StringIterator :: findLastOf ( const String & cset )
    {
        // iterator must be somewhere within its region
        check_valid ( self );

        // limit search to current pointer and before
        CText rev_rgn = rgn . subRgn ( 0, cur );

        // search for sub-string
        CharLoc found = rev_rgn . findLastOf ( cset . data () );

        // test if it was found
        if ( ! rev_rgn . contains ( found ) )
            return false;

        // translate current location
        cur = found;
        return true;
    }
    
    /**
     * findLastOf
     * @overload reverse search to find a character of a set
     * @param cset the set of characters being sought
     * @return Boolean true if found
     */
    bool StringIterator :: findLastOf ( const Text & cset )
    {
        // iterator must be somewhere within its region
        check_valid ( self );

        // limit search to current pointer and before
        CText rev_rgn = rgn . subRgn ( 0, cur );

        // search for sub-string
        CharLoc found = rev_rgn . findLastOf ( cset );

        // test if it was found
        if ( ! rev_rgn . contains ( found ) )
            return false;

        // translate current location
        cur = found;
        return true;
    }


    /*=================================================*
     *           C++ OPERATOR OVERLOADS                *
     *=================================================*/

    bool StringIterator :: operator < ( const StringIterator & i ) const
    {
        // the comparison is slightly tricky in that
        // it's not just the offset and index that are
        // compared, but also that both iterators cover
        // the same region of VAddrSpace.
        check_ctext_overlap ( self . rgn, i . rgn );

        // now we compare starting addresses
        return ( self . rgn . addr () + self . cur . offset () ) <
            ( i . rgn . addr () + i . cur . offset () );
    }
    
    bool StringIterator :: operator <= ( const StringIterator & i ) const
    {
        check_ctext_overlap ( self . rgn, i . rgn );
        return ( self . rgn . addr () + self . cur . offset () ) <=
            ( i . rgn . addr () + i . cur . offset () );
    }
    
    bool StringIterator :: operator == ( const StringIterator & i ) const
    {
        check_ctext_overlap ( self . rgn, i . rgn );
        return ( self . rgn . addr () + self . cur . offset () ) ==
            ( i . rgn . addr () + i . cur . offset () );
    }
    
    bool StringIterator :: operator != ( const StringIterator & i ) const
    {
        check_ctext_overlap ( self . rgn, i . rgn );
        return ( self . rgn . addr () + self . cur . offset () ) !=
            ( i . rgn . addr () + i . cur . offset () );
    }
    
    bool StringIterator :: operator >= ( const StringIterator & i ) const
    {
        check_ctext_overlap ( self . rgn, i . rgn );
        return ( self . rgn . addr () + self . cur . offset () ) >=
            ( i . rgn . addr () + i . cur . offset () );
    }
    
    bool StringIterator :: operator > ( const StringIterator & i ) const
    {
        check_ctext_overlap ( self . rgn, i . rgn );
        return ( self . rgn . addr () + self . cur . offset () ) >
            ( i . rgn . addr () + i . cur . offset () );
    }
        
    StringIterator & StringIterator :: operator = ( StringIterator && i ) noexcept
    {
        self . rgn_lock = std :: move ( i . rgn_lock );
        self . rgn = std :: move ( i . rgn );
        self . cur = std :: move ( i . cur );
        return self;
    }
    
    StringIterator & StringIterator :: operator = ( const StringIterator & i ) noexcept
    {
        self . rgn_lock = i . rgn_lock;
        self . rgn = i . rgn;
        self . cur = i . cur;
        return self;
    }
    
    StringIterator :: StringIterator ( StringIterator && i ) noexcept
        : rgn_lock ( std :: move ( i . rgn_lock ) )
        , rgn ( std :: move ( i . rgn ) )
        , cur ( std :: move ( i . cur ) )
    {
    }
    
    StringIterator :: StringIterator ( const StringIterator & i ) noexcept
        : rgn_lock ( i . rgn_lock )
        , rgn ( i . rgn )
        , cur ( i . cur )
    {
    }

    StringIterator :: StringIterator () noexcept
    {
    }
    
    StringIterator :: ~ StringIterator () noexcept
    {
    }

    StringIterator :: StringIterator ( const CRgnLockRef & lock, const CText & _rgn, const CharLoc & _cur ) noexcept
        : rgn_lock ( lock )
        , rgn ( _rgn )
        , cur ( _cur )
    {
    }
}
