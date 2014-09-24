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

#ifndef _h_kfc_sysctx_priv_
#define _h_kfc_sysctx_priv_

/* Clear new_ctx and initialize special members.
   Make sure Unix and Windows version do the same. */
#define RESET_CTX(new_ctx, ctx, func_loc) \
    do { \
        memset ( ( void* ) & new_ctx -> rc, 0, sizeof * new_ctx \
                    - sizeof new_ctx -> rsrc \
                    - sizeof new_ctx -> loc \
                    - sizeof new_ctx -> caller  \
                    - sizeof new_ctx -> zdepth ); \
        new_ctx -> rsrc = ctx -> rsrc; \
        new_ctx -> loc = func_loc; \
        new_ctx -> caller = ctx; \
        new_ctx -> zdepth = ctx -> zdepth + 1; \
    } while (false)

#endif /* _h_kfc_sysctx_priv_ */
