/*===========================================================================
*
*                            Public Domain Notice
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

#ifndef _h_resolver_priv_
#define _h_resolver_priv_

#ifndef _h_vfs_resolver_
#include <vfs/resolver.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * KConfig Repository Structure
 *  expressed in pseudo BNF:
 *
 *  base-config
 *      = "repository" <category>*
 *      ;
 *
 *  category
 *      = "remote" <remote-sub-category>*
 *      | "site" <site-sub-category>*
 *      | "user" <user-sub-category>*
 *      ;
 *
 *  remote-sub-category
 *      = "main" <public-remote-named-repository>*
 *      | "aux" <public-remote-named-repository>*
 *      | "protected" <protected-remote-named-repository>*
 *      ;
 *
 *  site-sub-category
 *      = "main" <site-named-repository>*
 *      | "aux" <site-named-repository>*
 *      ;
 *
 *  user-sub-category
 *      = "public" <public-user-named-repository>*
 *      | "protected" ID <protected-user-named-repository>*
 *      ;
 *
 *  public-remote-named-repository
 *      = ID <public-remote-repository>
 *      ;
 *
 *  protected-remote-named-repository
 *      = ID <protected-remote-repository>
 *      ;
 *
 *  site-named-repository
 *      = ID <site-repository>
 *      ;
 *
 *  public-user-named-repository
 *      = ID <public-user-repository>
 *      ;
 *
 *  protected-user-named-repository
 *      = ID <protected-user-repository>
 *      ;
 *
 *  public-remote-repository
 *      = <public-remote-root> [ <disabled> ] <apps>*
 *      ;
 *
 *  protected-remote-repository
 *      = <protected-remote-root> [ <disabled> ] <apps>*
 *      ;
 *
 *  site-repository
 *      = <local-root> [ <disabled> ] <apps>*
 *      ;
 *
 *  public-user-repository
 *      = <local-root> [ <disabled> ] [ <cache-enabled> ] <apps>*
 *      ;
 *
 *  protected-user-repository
 *      = <local-root> <passwd> <download-ticket> [ <disabled> ] [ <cache-enabled> ] <apps>*
 *      ;
 *
 *  apps
 *      = "refseq" <app>
 *      | "sra" <app>
 *      | "wgs" <app>
 *      ;
 *
 *  app
 *      = [ <disabled> ] "volumes" <volume>*
 *      ;
 *
 *  volume
 *      = <expansion-alg> "=" <path-list>
 *      ;
 *
 *  expansion-alg
 *      = "flat" | "sra1000" | "sra1024" | "fuse1000" | "refseq" | ...
 *      ;
 *
 *  disabled
 *      = "disabled" "=" ( "true" | * )  # any value other than "true" is false
 *      ;
 *
 *  cache-enabled
 *      = "cache-enabled" "=" ( "true" | * )  # any value other than "true" is false
 *      ;
 */

#define rcResolver   rcTree
#define rcRepository rcDirectory


#ifdef __cplusplus
}
#endif

#endif /* _h_resolver_priv_ */
