/*

  vdb3.kfc.xbt


 */

#pragma once

#include <vdb3/kfc/defs.hpp>
#include <vdb3/kfc/except.hpp>

#include <ostream>

/**
 * @file kfc/xbt.hpp
 * @brief Exception backtrace classes
 */

namespace vdb3
{

    /*=====================================================*
     *                     XBackTrace                      *
     *=====================================================*/
    
    /**
     * @class XBackTrace
     * @brief a C++ style callstack that attempts to show a backtrace
     *
     * NB - may not be available on all platforms,
     * and may be disabled due to policies.
     *
     * Intended for use in debugging.
     */
    class XBackTrace
    {
    public:

        bool isValid () const noexcept;

        const XMsg getName () const noexcept;

        bool up () const noexcept;

        CXX_RULE_OF_FOUR_NE ( XBackTrace );

        XBackTrace ( const Exception & x ) noexcept;
        ~ XBackTrace () noexcept;

    private:

        mutable char ** frames;
        mutable int num_frames;
        mutable int cur_frame;
    };

    //!< support for XBackTrace and std::ostream
    std :: ostream & operator << ( std :: ostream & o, const XBackTrace & bt );
}
