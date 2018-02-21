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

#ifndef _h_kfc_rc_
#define _h_kfc_rc_

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * under normal usage, the declarations below will create simple enums.
 * however, they may be redefined to create textual lookup tables.
 */
#ifndef RC_ENUM

#define RC_ENUM( type ) enum type

#undef RC_ENTRY
#define RC_ENTRY( id, txt ) id,

#undef RC_VFIRST
#define RC_VFIRST( id1, id2 ) id1 = ( int ) ( ( id2 ) - 1 ),

#undef RC_VLAST
#define RC_VLAST( id ) id,

#undef RC_LAST
#define RC_LAST( id ) id

#define RC_EMIT 1

#endif

/*--------------------------------------------------------------------------
 * RC
 *  upon success, all functions will return code 0
 *  other codes indicate failure or additional status information
 */


/* RCModule
 *  what code module generated the error
 */
RC_ENUM ( RCModule )
{
    RC_ENTRY ( rcExe, NULL )
    RC_ENTRY ( rcRuntime, "runtime" )
    RC_ENTRY ( rcText, "text" )
    RC_ENTRY ( rcCont, "container" )
    RC_ENTRY ( rcCS, "checksum" )
    RC_ENTRY ( rcFF, "file format detection" )
    RC_ENTRY ( rcFS, "file system" )
    RC_ENTRY ( rcPS, "process system" )
    RC_ENTRY ( rcXF, "transform" )
    RC_ENTRY ( rcDB, "database" )
    RC_ENTRY ( rcVDB, "virtual database" )
    RC_ENTRY ( rcApp, "application support" )
    RC_ENTRY ( rcXML, "xml support" )
    RC_ENTRY ( rcSRA, "short read archive" )
    RC_VLAST ( rcLastModule_v1_0 )
    RC_VFIRST ( rcFirstModule_v1_1, rcLastModule_v1_0 )
    RC_ENTRY ( rcKFG, "configuration" )
    RC_ENTRY ( rcAlign, "alignment" )
    RC_ENTRY ( rcKrypto, "cryptographic" )
    RC_ENTRY ( rcRDBMS, "RDBMS" )
    RC_ENTRY ( rcNS, "network system" )
    RC_ENTRY ( rcVFS, "virtual file system" )
    RC_LAST ( rcLastModule_v1_1 )
};

/* RCTarget
 *  describes the target object of the message
 */
RC_ENUM ( RCTarget )
{
    RC_ENTRY ( rcNoTarg, NULL )
    RC_ENTRY ( rcArc, "archive file" )
    RC_ENTRY ( rcToc, "file system table of contents" )
    RC_ENTRY ( rcTocEntry, "file table of contents entry" )
    RC_ENTRY ( rcArgv, "argument list" )
    RC_ENTRY ( rcAttr, "attribute" )
    RC_ENTRY ( rcBarrier, "barrier" )
    RC_ENTRY ( rcBlob, "binary large object" )
    RC_ENTRY ( rcBuffer, "buffer" )
    RC_ENTRY ( rcChar, "character" )
    RC_ENTRY ( rcColumn, "column" )
    RC_ENTRY ( rcCondition, "condition" )
    RC_ENTRY ( rcCursor, "cursor" )
    RC_ENTRY ( rcDatabase, "database" )
    RC_ENTRY ( rcDirectory, "directory" )
    RC_ENTRY ( rcDoc, "document" )
    RC_ENTRY ( rcXmlDoc, "XML document" )
    RC_ENTRY ( rcFile, "file" )
    RC_ENTRY ( rcFileDesc, "file descriptor" )
    RC_ENTRY ( rcFileFormat, "file format" )
    RC_ENTRY ( rcFunction, "function" )
    RC_ENTRY ( rcFormatter, "formatter" )
    RC_ENTRY ( rcFunctParam, "function parameter" )
    RC_ENTRY ( rcHeader, "header" )
    RC_ENTRY ( rcIndex, "index" )
    RC_ENTRY ( rcIterator, "iterator" )
    RC_ENTRY ( rcLock, "lock" )
    RC_ENTRY ( rcLog, "log" )
    RC_ENTRY ( rcMD5SumFmt, "MD5 sum file" )
    RC_ENTRY ( rcMemMap, "memory map" )
    RC_ENTRY ( rcMetadata, "metadata" )
    RC_ENTRY ( rcMgr, "manager" )
    RC_ENTRY ( rcNamelist, "name list" )
    RC_ENTRY ( rcNode, "node" )
    RC_ENTRY ( rcNumeral, "numeral" )
    RC_ENTRY ( rcPagemap, "page map" )
    RC_ENTRY ( rcPath, "path" )
    RC_ENTRY ( rcProcess, "process" ) 
    RC_ENTRY ( rcQueue, "queue" )
    RC_ENTRY ( rcRWLock, "read/write lock" )
    RC_ENTRY ( rcSchema, "schema" )
    RC_ENTRY ( rcSemaphore, "semaphore" )
    RC_ENTRY ( rcStorage, "storage" )
    RC_ENTRY ( rcString, "string" )
    RC_ENTRY ( rcTable, "table" )
    RC_ENTRY ( rcThread, "thread" )
    RC_ENTRY ( rcTimeout, "timeout" )
    RC_ENTRY ( rcToken, "token" )
    RC_ENTRY ( rcTree, "tree" )
    RC_ENTRY ( rcTrie, "trie" )
    RC_ENTRY ( rcType, "type" )
    RC_ENTRY ( rcVector, "vector" )
    RC_ENTRY ( rcDylib, "dynamic library" )
    RC_ENTRY ( rcExpression, "expression" )
    RC_VLAST ( rcLastTarget_v1_0 )
    RC_VFIRST ( rcFirstTarget_v1_1, rcLastTarget_v1_0 )
    RC_ENTRY ( rcProduction, "schema production" )
    RC_ENTRY ( rcEncryptionKey, "encryption key" )
    RC_ENTRY ( rcRng, "random number generator" )
    RC_ENTRY ( rcCmd, "command" )
    RC_ENTRY ( rcData, "data" )
    RC_ENTRY ( rcQuery, "query" )
    RC_ENTRY ( rcUri, "uri" )
    RC_ENTRY ( rcHashtable, "hashtable" )
    RC_LAST  ( rcLastTarget_v1_1 )
};

/* RCContext
 *  context under which error occurred
 */
RC_ENUM ( RCContext )
{
    RC_ENTRY ( rcAllocating, "allocating" )
    RC_ENTRY ( rcCasting, "type-casting" )
    RC_ENTRY ( rcConstructing, "constructing" )
    RC_ENTRY ( rcDestroying, "destroying" )
    RC_ENTRY ( rcReleasing, "releasing" )
    RC_ENTRY ( rcAccessing, "accessing" )
    RC_ENTRY ( rcListing, "listing" )
    RC_ENTRY ( rcVisiting, "visiting" )
    RC_ENTRY ( rcResolving, "resolving" )
    RC_ENTRY ( rcLocking, "locking" )
    RC_ENTRY ( rcUnlocking, "unlocking" )
    RC_ENTRY ( rcRenaming, "renaming" )
    RC_ENTRY ( rcAliasing, "aliasing" )
    RC_ENTRY ( rcSelecting, "selecting" )
    RC_ENTRY ( rcProjecting, "projecting" )
    RC_ENTRY ( rcInserting, "inserting" )
    RC_ENTRY ( rcRemoving, "removing" )
    RC_ENTRY ( rcClearing, "clearing" )
    RC_ENTRY ( rcUpdating, "updating" )
    RC_ENTRY ( rcCreating, "creating" )
    RC_ENTRY ( rcOpening, "opening" )
    RC_ENTRY ( rcClosing, "closing" )
    RC_ENTRY ( rcResizing, "resizing" )
    RC_ENTRY ( rcReading, "reading" )
    RC_ENTRY ( rcWriting, "writing" )
    RC_ENTRY ( rcCommitting, "committing" )
    RC_ENTRY ( rcReverting, "reverting" )
    RC_ENTRY ( rcResetting, "resetting" )
    RC_ENTRY ( rcPersisting, "persisting" )
    RC_ENTRY ( rcFreezing, "freezing" )
    RC_ENTRY ( rcCopying, "copying" )
    RC_ENTRY ( rcConcatenating, "concatenating" )
    RC_ENTRY ( rcFormatting, "formatting" )
    RC_ENTRY ( rcPositioning, "positioning" )
    RC_ENTRY ( rcPacking, "packing" )
    RC_ENTRY ( rcUnpacking, "unpacking" )
    RC_ENTRY ( rcEncoding, "encoding" )
    RC_ENTRY ( rcDecoding,"decoding" )
    RC_ENTRY ( rcValidating, "validating" )
    RC_ENTRY ( rcExecuting, "executing" )
    RC_ENTRY ( rcHuffmanCoding, "Huffman coding" )
    RC_ENTRY ( rcReindexing, "re-indexing" )
    RC_ENTRY ( rcRegistering, "registering" )
    RC_ENTRY ( rcTokenizing, "tokenizing" )
    RC_ENTRY ( rcParsing, "parsing" )
    RC_ENTRY ( rcConverting, "converting" )
    RC_ENTRY ( rcSignaling, "signaling" )
    RC_ENTRY ( rcWaiting, "waiting" )
    RC_ENTRY ( rcAttaching, "attaching" )
    RC_ENTRY ( rcDetaching, "detaching" )
    RC_ENTRY ( rcLogging, "logging" )
    RC_ENTRY ( rcFPCoding, "floating point coding" )
    RC_ENTRY ( rcMultiplexing, "(de)multiplexing" )
    RC_ENTRY ( rcClassifying, "classifying" )
    RC_ENTRY ( rcSearching, "searching" )
    RC_ENTRY ( rcLoading, "loading" )
    RC_ENTRY ( rcEvaluating, "evaluating" )
    RC_ENTRY ( rcInflating, "inflating" )
    RC_VLAST ( rcLastContext_v1_0 )
    RC_VFIRST ( rcFirstContext_v1_1, rcLastContext_v1_0 )
    RC_ENTRY ( rcFlushing, "flushing" )
    RC_ENTRY ( rcAppending, "appending" )
    RC_ENTRY ( rcEncrypting, "encrypting" )
    RC_ENTRY ( rcDecrypting, "decrypting" )
    RC_ENTRY ( rcComparing, "comparing" )
    RC_ENTRY ( rcInitializing, "initializing" )
    RC_ENTRY ( rcRetrieving, "retrieving" )
    RC_ENTRY ( rcSending, "sending" )
    RC_ENTRY ( rcProcessing, "processing" )
    RC_ENTRY ( rcIdentifying, "identifying" )
    RC_LAST ( rcLastContext_v1_1 )
};

/* RCObject
 *  type of object described by state
 */
RC_ENUM ( RCObject )
{
    RC_ENTRY ( rcNoObj, NULL )
    RC_VFIRST ( rcFirstObject, rcLastTarget_v1_1 )
    RC_ENTRY ( rcLink, "symbolic link" )
    RC_ENTRY ( rcSelf, "self" )
    RC_ENTRY ( rcParam, "param" )
    RC_ENTRY ( rcOffset, "offset" )
    RC_ENTRY ( rcMemory, "memory" )
    RC_ENTRY ( rcName, "name" )
    RC_ENTRY ( rcFormat, "format" )
    RC_ENTRY ( rcTransfer, "transfer" )
    RC_ENTRY ( rcInterface, "interface" )
    RC_ENTRY ( rcId, "id" )
    RC_ENTRY ( rcRange, "range" )
    RC_ENTRY ( rcConstraint, "constraint" )
    RC_ENTRY ( rcByteOrder, "byte order" )
    RC_ENTRY ( rcMessage, "message" )
    RC_ENTRY ( rcTag, "tag" )
    RC_ENTRY ( rcResources, "system resources" )
    RC_ENTRY ( rcDirEntry, "directory-toc entry" )
    RC_ENTRY ( rcArcHardLink, "archive hard link" )
    RC_ENTRY ( rcRow, "row" )
    RC_ENTRY ( rcLibrary, "loadable library" )
    RC_VLAST ( rcLastObject_v1_0 )
    RC_VFIRST ( rcFirstObject_v1_1, rcLastObject_v1_0 )
    RC_ENTRY ( rcItem, "item" )
    RC_ENTRY ( rcMode, "mode" )
    RC_ENTRY ( rcEncryption, "encryption" )
    RC_ENTRY ( rcCrc, "crc" )
    RC_ENTRY ( rcChecksum, "checksum" )
    RC_ENTRY ( rcSeed, "seed" )
    RC_ENTRY ( rcConnection, "connection" )
    RC_ENTRY ( rcError, "error" )
    RC_ENTRY ( rcEnvironment, "environment" )
    RC_ENTRY ( rcSignalSet, "signal set" )
    RC_ENTRY ( rcSize, "size" )
    RC_ENTRY ( rcRefcount, "reference count" )
    RC_LAST ( rcLastObject_v1_1 )
};

/* RCState
 *  state of object described
 */
RC_ENUM ( RCState )
{
    RC_ENTRY ( rcNoErr, "no error" )
    RC_ENTRY ( rcDone, "done" )
    RC_ENTRY ( rcUnknown, "unknown" )
    RC_ENTRY ( rcUnsupported, "unsupported" )
    RC_ENTRY ( rcUnexpected, "unexpected" )
    RC_ENTRY ( rcUnrecognized, "unrecognized" )
    RC_ENTRY ( rcAmbiguous, "ambiguous" )
    RC_ENTRY ( rcNull, "NULL" )
    RC_ENTRY ( rcBadVersion, "bad version" )
    RC_ENTRY ( rcDestroyed, "destroyed" )
    RC_ENTRY ( rcInvalid, "invalid" )
    RC_ENTRY ( rcCorrupt, "corrupt" )
    RC_ENTRY ( rcIncorrect, "incorrect" )
    RC_ENTRY ( rcInconsistent, "inconsistent" )
    RC_ENTRY ( rcBusy, "busy" )
    RC_ENTRY ( rcIncomplete, "incomplete" )
    RC_ENTRY ( rcInterrupted, "interrupted" )
    RC_ENTRY ( rcCanceled, "canceled" )
    RC_ENTRY ( rcEmpty, "empty" )
    RC_ENTRY ( rcExhausted, "exhausted" )
    RC_ENTRY ( rcInsufficient, "insufficient" )
    RC_ENTRY ( rcExcessive, "excessive" )
    RC_ENTRY ( rcViolated, "violated" )
    RC_ENTRY ( rcExists, "exists" )
    RC_ENTRY ( rcNotFound, "not found" )
    RC_ENTRY ( rcLocked, "locked" )
    RC_ENTRY ( rcUnlocked, "unlocked" )
    RC_ENTRY ( rcDetached, "detached" )
    RC_ENTRY ( rcDeadlock, "deadlock" )
    RC_ENTRY ( rcUnauthorized, "unauthorized" )
    RC_ENTRY ( rcReadonly, "read-only" )
    RC_ENTRY ( rcWriteonly, "write-only" )
    RC_ENTRY ( rcNoPerm, "no permission" )
    RC_ENTRY ( rcInPlaceNotAllowed, "update in-place prohibited" )
    RC_ENTRY ( rcTooShort, "too short" )
    RC_ENTRY ( rcTooLong, "too long" )
    RC_ENTRY ( rcTooBig, "too big" )
    RC_ENTRY ( rcDuplicate, "duplicate" )
    RC_ENTRY ( rcOutOfKDirectory, "path out of this KDirectory FS" )
    RC_ENTRY ( rcIgnored, "ignored" )	
    RC_ENTRY ( rcOutofrange, "out of range" )
    RC_VLAST ( rcLastState_v1_0 )
    RC_VFIRST ( rcFirstState_v1_1, rcLastState_v1_0 )
    RC_ENTRY ( rcOpen, "open" )
    RC_ENTRY ( rcOutoforder, "out of order" )
    RC_ENTRY ( rcNotOpen, "not open" )
    RC_ENTRY ( rcUndefined, "undefined" )
    RC_ENTRY ( rcUnequal, "unequal" )
    RC_ENTRY ( rcFailed, "failed" )
    RC_ENTRY ( rcNotAvailable, "not available" )
    RC_ENTRY ( rcWrongType, "wrong type" )
    RC_LAST ( rcLastState_v1_1 )
};

/* RAW_CTX
 *  form a context from parts
 */
#define RAW_CTX( mod, targ, ctx )                   \
    ( ( ( rc_t ) ( mod )  << 27 ) | /* 5 bits */    \
      ( ( rc_t ) ( targ ) << 21 ) | /* 6 bits */    \
      ( ( rc_t ) ( ctx )  << 14 ) ) /* 7 bits */


#ifdef __cplusplus
}
#endif

#endif /*  _h_kfc_rc_ */
