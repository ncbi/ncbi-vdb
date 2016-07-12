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

#ifndef _h_kapp_args_conv_
#define _h_kapp_args_conv_

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifndef _h_kapp_args_
#include "args.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* 
 * No conversion, allocates memory and copies argument as is.
 * This may should be used from other conversion functions if they decide not to change argument.
 */
rc_t ArgsConvDefault(const Args * args, uint32_t arg_index, const char * arg, size_t arg_len, void ** result, WhackParamFnP * whack);
    
/*
 * Converts from utf-8 platform dependent (e.g. with back-slashes on Windows) to utf-8 POSIX full file path format
 */
rc_t ArgsConvFilepath(const Args * args, uint32_t arg_index, const char * arg, size_t arg_len, void ** result, WhackParamFnP * whack);
    
#ifdef __cplusplus
}
#endif

#endif /* _h_kapp_args_conv_ */
