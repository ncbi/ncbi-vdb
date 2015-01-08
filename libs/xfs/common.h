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

#ifndef _h_common_
#define _h_common_

#include <xfs/xfs-defs.h>

/*))))
 ((((   This file contains all unsorted common things
  ))))
 ((((*/

/*))    Flavors ... all kind
 ((*/
static const uint32_t _sFlavorLess                = 0;
static const uint32_t _sFlavorOfFoo               = 1;
static const uint32_t _sFlavorOfBogus             = 2;
static const uint32_t _sFlavorOfSimpleContainer   = 3;
static const uint32_t _sFlavorOfKfs               = 4;

/*))    Common name defines for property names
 ((*/
#define XFS_MODEL_SOURCE    "source"

#endif /* _h_common_ */
