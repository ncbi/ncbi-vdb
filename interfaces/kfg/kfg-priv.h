/*==============================================================================
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

#ifndef _h_kfg_priv_
#define _h_kfg_priv_

#ifndef _h_kfg_extern_
#include <kfg/extern.h>
#endif

#ifndef _h_kfg_config_
#include <kfg/config.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------
 * forwards
 */
struct KDirectory;
struct KNamelist;


/*--------------------------------------------------------------------------
 * Environment and configuration keys
 */
#define ENV_KRYPTO_PWFILE   "VDB_PWFILE"
#define KFG_KRYPTO_PWFILE   "krypto/pwfile"
#define KFG_KRYPTO_PWFD     "krypto/pwfd"

/*--------------------------------------------------------------------------
 * KConfig
 *  configuration paramter manager
 */

/* ListIncluded
 *  list all included files
 */
KFG_EXTERN rc_t CC KConfigListIncluded ( const KConfig *self,
    struct KNamelist **names );

/* GetLoadPath
 *  return colon-separated search path to load KConfig
 * NB. path should not be released
 */
KFG_EXTERN rc_t CC KConfigGetLoadPath ( const KConfig *self,
    const char **path );

/* MakeLocal
 *  make a KConfig object that avoids singleton
 */
KFG_EXTERN rc_t CC KConfigMakeLocal ( KConfig **cfg,
    struct KDirectory const * cfgdir );

/* PrintDebug
 *  print configuration including some internal nodes information
 */
KFG_EXTERN rc_t CC KConfigPrintDebug ( const KConfig *self,
    const char *root_node_name );

#ifdef __cplusplus
}
#endif

#endif /* _h_kfg_priv_ */
