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
* ============================================================================*/


#include <klib/debug.h> /* KDbgFlag */
#include <klib/writer.h> /* KWrtWriter */


#undef KDbgCondToFlag
LIB_EXPORT KDbgFlag CC KDbgCondToFlag(KDbgCond cond) { return 0; }

#undef KDbgHandlerSet
LIB_EXPORT rc_t CC KDbgHandlerSet(KWrtWriter writer, void * writer_data)
{ return 0; }

#undef KDbgHandlerSetStdErr
LIB_EXPORT rc_t CC KDbgHandlerSetStdErr(void) { return 0; }

#undef KDbgHandlerSetStdOut
LIB_EXPORT rc_t CC KDbgHandlerSetStdOut(void) { return 0; }

#undef KDbgInit
LIB_EXPORT rc_t CC KDbgInit(void) { return 0; }

#undef KDbgMsg
LIB_EXPORT rc_t CC KDbgMsg(const char * fmt, ...) { return 0; }

#undef KDbgSetString
LIB_EXPORT rc_t CC KDbgSetString(const char * string) { return 0; }

#undef KDbgTestModConds
LIB_EXPORT bool CC KDbgTestModConds(KDbgMod mod, KDbgMask flags)
{ return false; }

#undef KDbgWriterGet
LIB_EXPORT KWrtWriter CC KDbgWriterGet(void) { return 0; }
