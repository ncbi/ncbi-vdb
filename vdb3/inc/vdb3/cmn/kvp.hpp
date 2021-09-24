/*

  vdb3.cmn.kvp

 */

#include <vdb3/cmn/defs.hpp>
#include <vdb3/cmn/string.hpp>

#include <map>
#include <vector>
#include <utility>

#pragma once

namespace vdb3
{
    
    /*=====================================================*
     *                    KeyValuePairs                    *
     *=====================================================*/

    class KeyValuePairs
    {
    public:

        /**
         * count
         * @return Natural number with the number of ( key, value ) pairs
         */
        count_t count () const;

        /**
         * getPairs
         * @return std::vector of ( key, value ) pairs
         */
        std :: vector < std :: pair < String, String > > getPairs () const;

        /**
         * getKeys
         * @return std::vector of keys
         */
        std :: vector < String > getKeys () const;

        /**
         * getValue
         * @return String with the value corresponding to key
         */
        const String & getValue ( const String & key ) const;

        /**
         * setValue
         * @brief modifies value of an existing pair or inserts a new pair
         */
        void setValue ( const String & key, const String & value );

        /*=================================================*
         *                       C++                       *
         *=================================================*/

        CXX_RULE_OF_SIX_NE ( KeyValuePairs );

    private:

        std :: map < String, String > kvp;
    };
}
