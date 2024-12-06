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

#pragma once

#include <klib/rc.h>

/*--------------------------------------------------------------------------
 * Vdb initialization/termination API
 */

#ifdef __cplusplus
extern "C" {
#endif

rc_t VdbInitialize( int argc, char *argv [], ver_t vers );

void VdbTerminate( rc_t rc );

#ifdef __cplusplus

    namespace VDB
    {

        class VdbApp
        {
        public:
            VdbApp( int argc, char *argv [], ver_t vers = 0 ) { m_rc = VdbInitialize( argc, argv, vers ); }
            ~VdbApp() { VdbTerminate( m_rc ); }

            operator bool() const { return m_rc == 0; }
            rc_t getRc() const { return m_rc; }
            void setRc( rc_t p_rc ) { m_rc = p_rc; }

        private:
            rc_t m_rc;
        };

    }
}
#endif

