/*

  vdb3.cmn.auto-rgn

 */

#pragma once

#include <vdb3/kfc/rgn.hpp>
#include <vdb3/cmn/text.hpp>

namespace vdb3
{
    /*=====================================================*
     *                       AutoRgn                       *
     *=====================================================*/

    /**
     * AutoRgn
     * @brief ability to create a modifiable region
     *  usually on the stack
     */
    template < size_t S = 256 >
    class AutoRgn : protected RgnFactory, TextFactory
    {
    public:

        /**
         * getMRgn
         * @brief return the region
         */
        MRgn getMRgn () noexcept
        { return makeMRgn ( a, sizeof a ); }

        /**
         * getMText
         * @brief return the region
         */
        MText getMText () noexcept
        { return makeMText ( ( UTF8 * ) a, sizeof a, 0, 0 ); }

    private:

        //!< stack space
        byte_t a [ S ];
    };
}
