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
#ifndef _h_kapp_log_xml_h_
#define _h_kapp_log_xml_h_

#ifndef _kapp_extern_
#include <kapp/extern.h>
#endif

#ifndef _kapp_args_
#include <kapp/args.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct KDirectory;

KAPP_EXTERN_DATA const OptDef XMLLogger_Args [];
KAPP_EXTERN_DATA const size_t XMLLogger_ArgsQty;

typedef struct XMLLogger XMLLogger;

/*
  Creates XML logging based on dir and command line
 */
KAPP_EXTERN rc_t CC XMLLogger_Make(const XMLLogger** cself,
    struct KDirectory* dir, const Args *args);

/*
  Creates XML logging based on dir, logpath pair or fd directly

  dir     [IN,NULL] - directory object used to create and open logpath file
  logpath [IN,NULL] - file name of the log file to _truncate_ and log into
  fd  [IN,NEGATIVE] - direct file descriptor, if < 0 than not set

  if both logpath == NULL and fd < 0 than no XML log is produced normal log gets full inforamtion
 */
KAPP_EXTERN rc_t CC XMLLogger_Make2(const XMLLogger** cself,
    struct KDirectory* dir, const char* logpath, const int fd);

KAPP_EXTERN void CC XMLLogger_Usage(void);

KAPP_EXTERN void CC XMLLogger_Release(const XMLLogger* cself);

KAPP_EXTERN rc_t CC XMLLogger_Encode(const char* src, char *dst, size_t dst_sz, size_t *num_writ);

#ifdef __cplusplus
}
#endif

#endif /* _h_kapp_log_xml_h_ */
