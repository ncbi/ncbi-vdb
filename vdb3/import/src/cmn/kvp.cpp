/*

  vdb3.cmn.kvp

 */

#include <vdb3/cmn/kvp.hpp>

namespace vdb3
{

    std :: vector < std :: pair < String, String > > KeyValuePairs :: getPairs () const
    {
        std :: vector < std :: pair < String, String > > pairs;
        
        auto it = kvp . cbegin ();
        for ( ; it != kvp . cend (); ++ it )
            pairs . push_back ( * it );

        return pairs;
    }

    std :: vector < String > KeyValuePairs :: getKeys () const
    {
        std :: vector < String > keys;
        
        auto it = kvp . cbegin ();
        for ( ; it != kvp . cend (); ++ it )
            keys . push_back ( it -> first );

        return keys;
    }
    
    const String & KeyValuePairs :: getValue ( const String & key ) const
    {
        auto it = kvp . find ( key );
        if ( it == kvp . end () )
        {
            throw NotFoundException (
                XP ( XLOC )
                << "member '"
                << key
                << "' not found"
                );
        }

        return it -> second;
    }
    
    void KeyValuePairs :: setValue ( const String & key, const String & value )
    {
        kvp . emplace ( key, value );
    }

    KeyValuePairs & KeyValuePairs :: operator = ( KeyValuePairs && p ) noexcept
    {
        kvp = std :: move ( p . kvp );
        return self;
    }

    KeyValuePairs & KeyValuePairs :: operator = ( const KeyValuePairs & p ) noexcept
    {
        kvp = p . kvp;
        return self;
    }

    KeyValuePairs :: KeyValuePairs ( KeyValuePairs && p ) noexcept
        : kvp ( std :: move ( p . kvp ) )
    {
    }

    KeyValuePairs :: KeyValuePairs ( const KeyValuePairs & p ) noexcept
        : kvp ( p . kvp )
    {
    }

    KeyValuePairs :: KeyValuePairs () noexcept
    {
    }

    KeyValuePairs :: ~ KeyValuePairs () noexcept
    {
    }
}
