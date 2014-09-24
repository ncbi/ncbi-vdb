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

#ifndef _hpp_klib_namelist_
#define _hpp_klib_namelist_

#ifndef _h_klib_namelist_
#include <klib/namelist.h>
#endif


/*--------------------------------------------------------------------------
 * KNamelist
 *  a generic list of NUL-terminated name strings
 */
struct KNamelist
{
    /* AddRef
     * Release
     *  ignores NULL references
     */
    inline int AddRef () const throw()
    { return KNamelistAddRef ( this ); }
    inline int Release () const throw()
    { return KNamelistRelease ( this ); }

    /* Count
     *  returns the number of entries
     *
     *  "count" [ OUT ] - return parameter for entry count
     */
    rc_t Count ( uint32_t *count ) const throw()
    { return KNamelistCount ( this, count ); }

    /* Get
     *  get an indexed name
     *
     *  "idx" [ IN ] - a zero-based name index
     *
     *  "name" [ OUT ] - return parameter for NUL terminated name
     */
    rc_t Get ( uint32_t idx, const char **name ) const throw()
    { return KNamelistGet ( this, idx, name ); }

private:
    KNamelist ();
    ~ KNamelist ();
    KNamelist ( const KNamelist& );
    KNamelist &operator = ( const KNamelist& );
};


/*--------------------------------------------------------------------------
 * VNamelist
 *  a vector-based list of NUL-terminated name strings
 */
struct VNamelist
{

    inline static rc_t Make ( VNamelist **namelist, const uint32_t alloc_blocksize ) throw ()
    { return VNamelistMake ( namelist, alloc_blocksize ); }

    /* AddRef
     * Release
     *  ignores NULL references
     */
    inline int AddRef () const throw()
    { return KNamelistAddRef ( ( const KNamelist * )this ); }
    inline int Release () const throw()
    { return VNamelistRelease ( this ); }

    /* Count
     *  returns the number of entries
     *
     *  "count" [ OUT ] - return parameter for entry count
     */
    rc_t Count ( uint32_t *count ) const throw()
    { return KNamelistCount ( ( const KNamelist * )this, count ); }

    /* Get
     *  get an indexed name
     *
     *  "idx" [ IN ] - a zero-based name index
     *
     *  "name" [ OUT ] - return parameter for NUL terminated name
     */
    rc_t Get ( uint32_t idx, const char **name ) const throw()
    { return KNamelistGet ( ( const KNamelist * )this, idx, name ); }
    
    /* Append
     *  append a string...
     *
     *  "src" [ IN ] - the string to append
     */
    rc_t Append ( const char *src ) throw()
    { return VNamelistAppend ( this, src ); }


private:
    VNamelist ();
    ~ VNamelist ();
    VNamelist ( const VNamelist& );
    VNamelist &operator = ( const VNamelist& );
};

 
#endif // _hpp_klib_namelist_
