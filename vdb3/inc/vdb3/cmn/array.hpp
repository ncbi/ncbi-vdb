/*

  vdb3.cmn.array

 */

#pragma once

namespace vdb3
{
    /**
     * @file cmn/array.hpp
     * @brief it would be great to have a reliable implementation of an array
     *  or sequence that did not depend upon std C++ library.
     *
     *  the essential problem is that C++ as a language has such severe
     *  shortcomings when it comes to talking about containers. The latter need
     *  to have their own description while remaining independent of or perhaps
     *  responsive to their contents. The colossal hack that is the template
     *  facility allows for some expression of containers at the cost of requiring
     *  all-inline everything.
     *
     *  this approach gives source-level code reuse, but no actual polymorphism.
     *  as a result, you can't have interfaces separate from implementations.
     *  so a sequence or array can't present the same interface as a list because
     *  the underlying implementation is exposed with templates.
     *
     *  so while I have long wanted to replace std C++ vectors, maps and the like,
     *  the struggle against the language is so severe that it's more trouble than
     *  it's worth.
     */
}
