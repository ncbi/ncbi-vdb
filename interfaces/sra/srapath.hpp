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

#ifndef _hpp_sra_srapath_
#define _hpp_sra_srapath_

#ifndef _h_sra_srapath_
#include <sra/srapath.h>
#endif


/*--------------------------------------------------------------------------
 * SRAPath
 *  archive specific path manager
 */
struct SRAPath
{
    /* Make
     *  create path manager
     *
     *  the path manager should already be configured with
     *  standard search paths, but can be augmented by using
     *  the Add*Path messages.
     *
     *  "dir" [ IN, NULL OKAY ] - optional root directory to use
     *  attaches a new reference
     */
    static inline rc_t Make ( SRAPath **pm,
        struct KDirectory const *dir = NULL ) throw()
    { return SRAPathMake ( pm, dir ); }


    /* AddRef
     * Release
     */
    inline rc_t AddRef () const  throw()
    { return SRAPathAddRef ( this ); }

    inline rc_t Release () const  throw()
    { return SRAPathRelease ( this ); }


    /* Version
     *  returns the library version
     */
    inline rc_t Version ( uint32_t *version ) const  throw()
    { return SRAPathVersion ( this, version ); }


    /* Clear
     *  forget all existing server and volume paths
     */
    inline rc_t Clear () throw()
    { return SRAPathClear ( this ); }

    /* AddRepPath
     *  add a replication path
     *
     *  "rep" [ IN ] - NUL-terminated server search path
     *  may be a compound path with ':' separator characters, e.g.
     *  "/panfs/traces01:/panfs/traces31"
     *
     *  NB - servers are searched in the order provided,
     *  first to last, until one of them satisfies a request,
     *  at which time the successful server is placed at the
     *  head of the search path.
     */
    inline rc_t AddRepPath ( const char *rep )  throw()
    { return SRAPathAddRepPath ( this, rep ); }


    /* AddVolPath
     *  add a volume path
     *
     *  "vol" [ IN ] - NUL-terminated volume search path
     *  may be a compound path with ':' separator characters, e.g.
     *  "sra2:sra1:sra0"
     *
     *  NB - volumes are searched in the order provided,
     *  first to last. they are never re-ordered.
     */
    inline rc_t AddVolPath ( const char *vol )  throw()
    { return SRAPathAddVolPath ( this, vol ); }


    /* Full
     *  creates full path from server, volume & accession
     *
     *  "rep" [ IN ] - NUL terminated full path of replication
     *  server, e.g. "/panfs/traces01"
     *
     *  "vol" [ IN ] - NUL terminated relative path of volume,
     *  e.g. "sra2"
     *
     *  "accession" [ IN ] - NUL terminated run accession,
     *  e.g. "SRR000001"
     *
     *  "path" [ OUT ] and "path_max" [ IN ] - return buffer for
     *  NUL-terminated full path to accession.
     */
    inline rc_t Full ( const char *rep, const char *vol,
        const char *accession, char *path, size_t path_max ) const  throw()
    { return SRAPathFull ( this, rep, vol, accession, path, path_max ); }


    /* Test
     *  returns true if path appears to be accession
     *  the test is a heuristic, and may return false positives
     *
     *  "path" [ IN ] - NUL terminated path to run
     */
    inline bool Test ( const char *path ) const  throw()
    { return SRAPathTest ( this, path ); }


    /* Find
     *  finds location of run within rep-server/volume matrix
     *
     *  "accession" [ IN ] - NUL terminated run accession,
     *   e.g. "SRR000001"
     *
     *  "path" [ OUT ] and "path_max" [ IN ] - return buffer for
     *  NUL-terminated full path to accession.
     *
     *  returns 0 if path exists, rc state rcNotFound if
     *  path cannot be found, and rcInsufficient if buffer is
     *  too small.
     */
    inline rc_t Find ( const char *accession, char *path,
        size_t path_max ) const throw()
    { return SRAPathFind ( this, accession, path, path_max ); }


    /* List
     *  list all runs known to SRAPath
     *  this can be VERY SLOW due to the large number of runs
     *
     *  "runs" [ OUT ] - return parameter for ordered list of accessions
     *
     *  "deep" [ IN ] - when false, list only first responding server
     */
    inline rc_t List ( struct KNamelist **runs,
        bool deep = false ) const  throw()
    { return SRAPathList ( this, runs, deep ); }

private:
    SRAPath ();
    ~ SRAPath ();
    SRAPath ( const SRAPath& );
    SRAPath &operator = ( const SRAPath& );
};

#endif // _hpp_sra_srapath_
