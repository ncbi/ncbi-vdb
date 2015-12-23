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

#ifndef _h_kfs_defs_
#define _h_kfs_defs_

#ifndef _h_kfs_extern_
#include <kfs/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * KPathType
 */
typedef uint32_t KPathType;
enum
{
    /* add in below zero so as not to change behavior of existing */
    /* added for KDB that extended this enumeration */ 
    /* if added it will clash with the kptAlias bit */
    /* without care be taken */
    kptFirstDefined = 0,

    /* some paths are bad or non-existent */
    kptNotFound = kptFirstDefined,
    kptBadPath,

    /* existing paths describe a directory entry */
    kptFile,
    kptDir,
    kptCharDev,
    kptBlockDev,
    kptFIFO,

    /* special types that aren't quite right or normal */

    /* A zombie file is a file in the directory that can not be opened
     * or read because it is for some reason not really present
     * zombie as in not live and not dead */
    kptZombieFile,

    /* special cases for Windows file systems */
    kptFakeRoot,

    /* special cases for visiting a HDF5-file: */

    /* a dataset is a container for data like a file, but typed */
    kptDataset,
    /* a datatype is a object inside a HDF5-group (directory) */
    kptDatatype,

    /* the type enum may be extended by virtual directories */
    kptLastDefined,

    /* aliases to the above types have the alias bit set */
    kptAlias = 128
};


/*--------------------------------------------------------------------------
 * KCreateMode
 *  when creating a file or directory, the normal intent is to open it,
 *  creating it first if necessary, and then go on. alternatively, the
 *  intention may be to always return a newly initialized object, while
 *  the last may be to only proceed if actually created, i.e. if the
 *  object does not already exist in some form.
 */
enum
{
    /* mode values 0..7 */
    kcmOpen,
    kcmInit,
    kcmCreate,
    kcmSharedAppend,        /* special mode which works only on
                             * needed for proper logging
                             */
    kcmValueMask = 15,

    /* modifier bits 3..7 */
    kcmMD5     = ( 1 << 6 ),
    kcmParents = ( 1 << 7 ),
    kcmBitMask = ( 1 << 8 ) - kcmValueMask - 1
};


#ifdef __cplusplus
}
#endif

#endif /* _h_kfs_defs_ */
