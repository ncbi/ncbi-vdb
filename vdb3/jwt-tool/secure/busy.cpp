/*===========================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Information
 *
 *  This software/database is a "United States Government Work" under the
 *  terms of the United States Copyright Act.  It was written as part of
 *  the author's official duties as a United States Government employee and
 *  thus cannot be copyrighted.  This software/database is freely available
 *  to the public for use. The National Library of Medicine and the U.S.
 *  Government have not placed any restriction on its use or reproduction.
 *
 *  Although all reasonable efforts have been taken to ensure the accuracy
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 *  Author: Kurt Rodarmer
 *
 * ===========================================================================
 *
 */

#include <ncbi/secure/busy.hpp>

namespace ncbi
{
#if BUSY_IS_BOOL
    inline
    void clear ( bool & busy )
    {
        busy = false;
    }

    inline
    bool test_and_set ( bool & busy )
    {
        bool prior = busy;
        busy = true;
        return prior;
    }
#else
    inline
    void clear ( std :: atomic_flag & busy )
    {
        busy . clear ();
    }

    inline
    bool test_and_set ( std :: atomic_flag & busy )
    {
        return busy . test_and_set ();
    }
#endif

    BusyLock :: BusyLock ()
    {
        clear ( busy );
    }

    BusyLock :: ~ BusyLock ()
    {
        // TBD - detect cases where value was true
        // and... throw? log?
#if BUSY_IS_BOOL
        busy = false;
#endif
    }

    BusyLock & BusyLock :: acquireShared () const
    {
        if ( test_and_set ( busy ) )
        {
            throw ObjectBusyException (
                XP ( XLOC )
                << "object is busy"
                );
        }

        return * const_cast < BusyLock * > ( this );
    }

    BusyLock & BusyLock :: acquireExclusive () const
    {
        if ( test_and_set ( busy ) )
        {
            throw ObjectBusyException (
                XP ( XLOC )
                << "object is busy"
                );
        }

        return * const_cast < BusyLock * > ( this );
    }

    void BusyLock :: release ()
    {
        // TBD - detect cases where value was false
        // but what to do?
        clear ( busy );
    }
}
