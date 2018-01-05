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
* ===========================================================================
*
*/

#ifndef _hpp_vdb3_kfc_rsrc_
#define _hpp_vdb3_kfc_rsrc_

#include <kfc/except.hpp>
#include <kfc/memmgr.hpp>
#include <kfc/timemgr.hpp>
#include <kfc/fdmgr.hpp>
#include <kfc/log.hpp>
#include <kfc/stream.hpp>

namespace vdb3
{

    /*------------------------------------------------------------------
     * forwards and externs
     */

    class Rsrc;
    extern __thread Rsrc const * rsrc;


    /*------------------------------------------------------------------
     * rcaps_t
     *  resource manager capabilities
     */
    const rcaps_t RCAP_MMGR    = ( 1 <<  0 );
    const rcaps_t RCAP_TMMGR   = ( 1 <<  1 );
    const rcaps_t RCAP_FDMGR   = ( 1 <<  2 );
    const rcaps_t RCAP_LOG     = ( 1 <<  3 );
    const rcaps_t RCAP_ERR     = ( 1 <<  4 );
    const rcaps_t RCAP_ALL     = ( 1 <<  5 ) - 1;


    /*------------------------------------------------------------------
     * RsrcBase
     *  implements a destructor
     */
    class RsrcBase
    {
    public:

        ~ RsrcBase ();
    };


    /*------------------------------------------------------------------
     * Rsrc
     *  block of resource manager references
     */
    class Rsrc : public RsrcBase
    {
    public:

        /* RESOURCE MANAGERS
         */

        // memory manager is required for nearly everything
        MemMgr mmgr;

        // time manager is fundamental
        TimeMgr tmmgr;

        // file descriptors
        FDMgr fdmgr;


        /* ENVIRONMENT
         */

        // logging formatter
        Log log;

        // streams
        Stream err;

        /* C++
         */

        // clone current resources with potential reduction
        Rsrc ( rcaps_t mgrs );

        // cloning is allowed at any time
        Rsrc ( const Rsrc & rsrc );
        void operator = ( const Rsrc & rsrc );

        ~ Rsrc ();

    protected:

        // one-shot
        // allowed once per process
        Rsrc ( const MemMgr & pmmgr, const char * ident );

    };


    /*------------------------------------------------------------------
     * TopRsrc
     *  block of resource manager references
     */
    class TopRsrc : public Rsrc
    {
    public:

        // create and stack process resources
        TopRsrc ( const char * ident );

    };
}

#endif // _hpp_vdb3_kfc_rsrc_
