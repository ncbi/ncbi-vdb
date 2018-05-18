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

#ifndef _h_kfc_xc_
#define _h_kfc_xc_

#ifndef _h_kfc_extern_
#include <kfc/extern.h>
#endif

#ifndef _h_kfc_defs_
#include <kfc/defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * XOBJ
 *  objects that can have problems
 */
XOBJ ( xoSelf, "target object reference", rcSelf );
XOBJ ( xoParam, "parameter", rcParam );
XOBJ ( xoString, "string", rcString );
XOBJ ( xoMemory, "process memory", rcMemory );
XOBJ ( xoError, "error", rcNoObj );
XOBJ ( xoBehavior, "behavior", rcFunction );
XOBJ ( xoTable, "table", rcTable );
XOBJ ( xoCursor, "cursor", rcCursor );
XOBJ ( xoColumn, "column", rcColumn );
XOBJ ( xoInteger, "integer", rcParam );
XOBJ ( xoRow, "row", rcRow );
XOBJ ( xoRefcount, "number of references", rcRefcount );
XOBJ ( xoResource, "resource manager", rcMgr );
XOBJ ( xoIterator, "iterator", rcIterator );
XOBJ ( xoArc, "archive", rcArc );
XOBJ ( xoType, "object type", rcType );
XOBJ ( xoReference, "reference", rcIterator );
XOBJ ( xoReadGroup, "read group", rcParam );
XOBJ ( xoFile, "file", rcFile );
XOBJ ( xoDirectory, "directory", rcDirectory );
XOBJ ( xoPath, "pat", rcPath );
XOBJ ( xoInterface, "interface", rcInterface );
XOBJ ( xoTransfer, "transfer", rcTransfer );
XOBJ ( xoFunction, "function", rcFunction );
XOBJ ( xoBuffer, "buffer", rcBuffer );
XOBJ ( xoFileDescriptor, "file descriptor", rcFileDesc );
XOBJ ( xoStorage, "storage", rcStorage );
XOBJ ( xoArgv, "argument vector", rcArgv );
XOBJ ( xoZlib, "zlib stream", rcBlob );
XOBJ ( xoAlignment, "alignment", rcData );

/*--------------------------------------------------------------------------
 * XSTATE
 *  states that things can be in
 *  THESE ARE BEING BADLY FILLED OUT
 *  and I started the problem... but it needs to be straightened out.
 */
XSTATE ( xsIsNull, "is null", rcNull );
XSTATE ( xsEmpty, "is empty", rcEmpty );
XSTATE ( xsExhausted, "exhausted", rcExhausted );
XSTATE ( xsUnexpected, "unexpected", rcUnexpected );
XSTATE ( xsUnimplemented, "unimplemented", rcUnknown );
XSTATE ( xsCreateFailed, "failed to create", rcUnknown );
XSTATE ( xsOpenFailed, "failed to open", rcUnknown );
XSTATE ( xsNotFound, "not found", rcNotFound );
XSTATE ( xsReadFailed, "failed to read", rcUnknown );
XSTATE ( xsOutOfBounds, "out of bounds", rcOutofrange );
XSTATE ( xsAccessFailed, "failed to access", rcUnknown );
XSTATE ( xsZombie, "is already freed", rcUnknown );
XSTATE ( xsExists, "already exists", rcExists );
XSTATE ( xsWriteOnly, "is write only", rcWriteonly );
XSTATE ( xsUninitialized, "uninitialized", rcNotOpen );
XSTATE ( xsIncorrect, "incorrect", rcIncorrect );
XSTATE ( xsReadOnly, "is read only", rcReadonly );
XSTATE ( xsInvalid, "invalid", rcInvalid );
XSTATE ( xsInsufficient, "insufficient", rcInsufficient );
XSTATE ( xsIncomplete, "incomplete", rcIncomplete );
XSTATE ( xsFailed, "failed", rcFailed );
XSTATE ( xsTimeout, "timed out", rcTimeout );
XSTATE ( xsExcessive, "excessive", rcExcessive );
XSTATE ( xsUnknown, "unknown", rcUnknown );
XSTATE ( xsInconsistent, "inconsistent", rcInconsistent );
XSTATE ( xsUnsupported, "unsupported", rcUnsupported );
XSTATE_EXT ( xsInitFailed, xsFailed, "init failed", rcFailed );
XSTATE ( xsNoPrimary, "missing primary", rcInvalid );
XSTATE ( xsBadVersion, "bad version", rcBadVersion );
XSTATE ( xsViolated, "violated", rcViolated );

/*--------------------------------------------------------------------------
 * XC
 *  error types
 */
XC ( xcSelfNull, xoSelf, xsIsNull );
XC ( xcParamNull, xoParam, xsIsNull );
XC ( xcStringEmpty, xoString, xsEmpty );
XC ( xcNoMemory, xoMemory, xsExhausted );
XC ( xcUnexpected, xoError, xsUnexpected );
XC ( xcUnimplemented, xoBehavior, xsUnimplemented );
XC ( xcTableOpenFailed, xoTable, xsOpenFailed );
XC ( xcCursorCreateFailed, xoCursor, xsCreateFailed );
XC ( xcCursorOpenFailed, xoCursor, xsOpenFailed );
XC ( xcColumnNotFound, xoColumn, xsNotFound );
XC ( xcColumnReadFailed, xoColumn, xsReadFailed );
XC ( xcIntegerOutOfBounds, xoInteger, xsOutOfBounds );
XC ( xcCursorAccessFailed, xoCursor, xsAccessFailed );
XC ( xcRowNotFound, xoRow, xsNotFound );
XC ( xcSelfZombie, xoSelf, xsZombie );
XC ( xcRefcountOutOfBounds, xoRefcount, xsOutOfBounds );
XC ( xcParamOutOfBounds, xoParam, xsOutOfBounds );
XC ( xcResourceExists, xoResource, xsExists );
XC ( xcCursorExhausted, xoCursor, xsExhausted );
XC ( xcStringCreateFailed, xoString, xsCreateFailed );
XC ( xcIteratorUninitialized, xoIterator, xsUninitialized );
XC ( xcWrongReference, xoParam, xsIncorrect );
XC ( xcIteratorExhausted, xoIterator, xsExhausted );
XC ( xcStringNotFound, xoString, xsNotFound );
XC ( xcParamUnexpected, xoParam, xsUnexpected );
XC ( xcArcIncorrect, xoArc, xsIncorrect );
XC ( xcTypeIncorrect, xoType, xsIncorrect );
XC ( xcWrongReadGroup, xoReadGroup, xsIncorrect );
XC ( xcInterfaceNull, xoInterface, xsIsNull );
XC ( xcTransferIncomplete, xoTransfer, xsIncomplete );
XC ( xcReleaseFailed, xoRefcount, xsFailed );
XC ( xcFunctionNull, xoFunction, xsIsNull );
XC ( xcFunctionIncomplete, xoFunction, xsIncomplete );
XC ( xcPathNull, xoPath, xsIsNull );
XC ( xcPathEmpty, xoPath, xsEmpty );
XC ( xcPathInvalid, xoPath, xsInvalid );
XC ( xcParamInvalid, xoParam, xsInvalid );
XC ( xcStorageExhausted, xoStorage, xsExhausted );
XC ( xcFunctionUnimplemented, xoFunction, xsUnimplemented );
XC ( xcErrorUnknown, xoError, xsUnknown );
XC ( xcRefcountInconsistent, xoRefcount, xsInconsistent );
XC ( xcRefcountExcessive, xoRefcount, xsExcessive );
XC ( xcBufferInsufficient, xoBuffer, xsInsufficient );
XC ( xcFileReadOnly, xoFile, xsReadOnly );
XC ( xcFileWriteOnly, xoFile, xsWriteOnly );
XC ( xcFileInvalidVersion, xoFile, xsInvalid );
XC ( xcFileNotFound, xoFile, xsNotFound );
XC ( xcFileTimeout, xoFile, xsTimeout );
XC ( xcFileUnexpected, xoFile, xsUnexpected );
XC ( xcFileDescInvalid, xoFileDescriptor, xsInvalid );
XC ( xcFileExcessive, xoFile, xsExcessive );
XC ( xcDirectoryInvalidVersion, xoDirectory, xsInvalid );
XC ( xcDirectoryWriteOnly, xoDirectory, xsWriteOnly );
XC ( xcFunctionUnsupported, xoFunction, xsUnsupported );
XC ( xcInterfaceIncorrect, xoInterface, xsIncorrect );
XC ( xcInterfaceInvalid, xoInterface, xsInvalid );
XC ( xcColumnEmpty, xoColumn, xsEmpty );
XC ( xcParamNotFound, xoParam, xsNotFound );
XC ( xcArgvInvalid, xoArgv, xsInvalid );
XC ( xcDirectoryNotFound, xoDirectory, xsNotFound );
XC ( xcFileAccessFailed, xoFile, xsAccessFailed );
XC ( xcZlibInitFailed, xoZlib, xsInitFailed );
XC ( xcZlibDataInvalid, xoZlib, xsInvalid );
XC ( xcSecondaryAlignmentMissingPrimary, xoAlignment, xsNoPrimary );
XC ( xcInterfaceBadVersion, xoInterface, xsBadVersion );
XC ( xcRefcountViolated, xoRefcount, xsViolated );
XC ( xcSelfInvalid, xoSelf, xsInvalid );
XC ( xcParamIncorrect, xoParam, xsIncorrect );

#ifdef __cplusplus
}
#endif

#endif /* _h_kfc_xc_ */
