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

#ifndef _h_fmtdef_
#define _h_fmtdef_

/* int64_t is "long long int" in 32 bit */
#define LD64 "lld"
#define LU64 "llu"
#define LX64 "llX"
#define Lx64 "llx"

/* size_t is traditionally "unsigned int"
   although it should have been "long unsigned int" */
#define LUSZ "u"
#define LXSZ "X"
#define LxSZ "x"

#endif /* _h_fmtdef_ */
