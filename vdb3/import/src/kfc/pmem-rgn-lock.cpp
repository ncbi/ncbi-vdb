/*

  vdb3.kfc.pmem-rgn-lock

 */

#include "pmem-rgn-lock.hpp"
#include "pmem.hpp"

#include <vdb3/kfc/ktrace.hpp>

namespace vdb3
{
    MRgn PMemMRgnLockImpl :: getMRgn () noexcept
    {
        return rgn;
    }

    PMemMRgnLockImpl :: PMemMRgnLockImpl ( const IRef < PMemImpl > & _mem )
        : mem ( _mem )
        , lock ( _mem -> busy )
        , rgn ( _mem -> rgn )
    {
        KTRACE ( TRACE_PRG, "acquired exclusive lock on memory region" );
    }
    
    PMemMRgnLockImpl :: ~ PMemMRgnLockImpl () noexcept
    {
        KTRACE ( TRACE_PRG, "releasing exclusive lock on memory region" );
    }

    CRgn PMemCRgnLockImpl :: getCRgn () const noexcept
    {
        return rgn;
    }

    PMemCRgnLockImpl :: PMemCRgnLockImpl ( const IRef < PMemImpl > & _mem )
        : mem ( _mem )
        , lock ( _mem -> busy )
        , rgn ( _mem -> rgn )
    {
        KTRACE ( TRACE_PRG, "acquired shared lock on memory region" );
    }
    
    PMemCRgnLockImpl :: ~ PMemCRgnLockImpl () noexcept
    {
        KTRACE ( TRACE_PRG, "releasing shared lock on memory region" );
    }

}
