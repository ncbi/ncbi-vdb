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
    RC_ENTRY ( rcExe, NULL )                                    /*   0 */ /* 1 bit  */
    RC_ENTRY ( rcRuntime, "runtime" )                           /*   1 */
    RC_ENTRY ( rcText, "text" )                                 /*   2 */ /* 2 bits */
    RC_ENTRY ( rcCont, "container" )                            /*   3 */
    RC_ENTRY ( rcCS, "checksum" )                               /*   4 */ /* 3 bits */
    RC_ENTRY ( rcFF, "file format detection" )                  /*   5 */
    RC_ENTRY ( rcFS, "file system" )                            /*   6 */
    RC_ENTRY ( rcPS, "process system" )                         /*   7 */
    RC_ENTRY ( rcXF, "transform" )                              /*   8 */ /* 4 bits */
    RC_ENTRY ( rcDB, "database" )                               /*   9 */
    RC_ENTRY ( rcVDB, "virtual database" )                      /*  10 */
    RC_ENTRY ( rcApp, "application support" )                   /*  11 */
    RC_ENTRY ( rcXML, "xml support" )                           /*  12 */
    RC_ENTRY ( rcSRA, "short read archive" )                    /*  13 */
    RC_VLAST ( rcLastModule_v1_0 )                              /*  14 */
    RC_VFIRST ( rcFirstModule_v1_1, rcLastModule_v1_0 )         /*  13 */
    RC_ENTRY ( rcKFG, "configuration" )                         /*  14 */
    RC_ENTRY ( rcAlign, "alignment" )                           /*  15 */
    RC_ENTRY ( rcKrypto, "cryptographic" )                      /*  16 */ /* 5 bits */
    RC_ENTRY ( rcRDBMS, "RDBMS" )                               /*  17 */
    RC_ENTRY ( rcNS, "network system" )                         /*  18 */
    RC_ENTRY ( rcVFS, "virtual file system" )                   /*  19 */
    RC_VLAST ( rcLastModule_v1_1 )                              /*  20 */
    RC_VFIRST ( rcFirstModule_v1_2, rcLastModule_v1_1 )         /*  19 */
    RC_ENTRY ( rcExitCode, "exit code" )                        /*  20 */
    RC_ENTRY ( rcTLS, "transport layer security" )              /*  21 */
    RC_ENTRY ( rcCloud, "cloud" )                               /*  22 */
    RC_LAST ( rcLastModule_v1_2 )                               /*  23 */

        /* RCModule requires 5 bits
           has headroom up to code 31 */
};

/* RCTarget
 *  describes the target object of the message
 */
RC_ENUM ( RCTarget )
{
    RC_ENTRY ( rcNoTarg, NULL )                                 /*   0 */ /* 1 bit  */
    RC_ENTRY ( rcArc, "archive file" )                          /*   1 */
    RC_ENTRY ( rcToc, "file system table of contents" )         /*   2 */ /* 2 bits */
    RC_ENTRY ( rcTocEntry, "file table of contents entry" )     /*   3 */
    RC_ENTRY ( rcArgv, "argument list" )                        /*   4 */ /* 3 bits */
    RC_ENTRY ( rcAttr, "attribute" )                            /*   5 */
    RC_ENTRY ( rcBarrier, "barrier" )                           /*   6 */
    RC_ENTRY ( rcBlob, "binary large object" )                  /*   7 */
    RC_ENTRY ( rcBuffer, "buffer" )                             /*   8 */ /* 4 bits */
    RC_ENTRY ( rcChar, "character" )                            /*   9 */
    RC_ENTRY ( rcColumn, "column" )                             /*  10 */
    RC_ENTRY ( rcCondition, "condition" )                       /*  11 */
    RC_ENTRY ( rcCursor, "cursor" )                             /*  12 */
    RC_ENTRY ( rcDatabase, "database" )                         /*  13 */
    RC_ENTRY ( rcDirectory, "directory" )                       /*  14 */
    RC_ENTRY ( rcDoc, "document" )                              /*  15 */
    RC_ENTRY ( rcXmlDoc, "XML document" )                       /*  16 */ /* 5 bits */
    RC_ENTRY ( rcFile, "file" )                                 /*  17 */
    RC_ENTRY ( rcFileDesc, "file descriptor" )                  /*  18 */
    RC_ENTRY ( rcFileFormat, "file format" )                    /*  19 */
    RC_ENTRY ( rcFunction, "function" )                         /*  20 */
    RC_ENTRY ( rcFormatter, "formatter" )                       /*  21 */
    RC_ENTRY ( rcFunctParam, "function parameter" )             /*  22 */
    RC_ENTRY ( rcHeader, "header" )                             /*  23 */
    RC_ENTRY ( rcIndex, "index" )                               /*  24 */
    RC_ENTRY ( rcIterator, "iterator" )                         /*  25 */
    RC_ENTRY ( rcLock, "lock" )                                 /*  26 */
    RC_ENTRY ( rcLog, "log" )                                   /*  27 */
    RC_ENTRY ( rcMD5SumFmt, "MD5 sum file" )                    /*  28 */
    RC_ENTRY ( rcMemMap, "memory map" )                         /*  29 */
    RC_ENTRY ( rcMetadata, "metadata" )                         /*  30 */
    RC_ENTRY ( rcMgr, "manager" )                               /*  31 */
    RC_ENTRY ( rcNamelist, "name list" )                        /*  32 */ /*  6 bits */
    RC_ENTRY ( rcNode, "node" )                                 /*  33 */
    RC_ENTRY ( rcNumeral, "numeral" )                           /*  34 */
    RC_ENTRY ( rcPagemap, "page map" )                          /*  35 */
    RC_ENTRY ( rcPath, "path" )                                 /*  36 */
    RC_ENTRY ( rcProcess, "process" )                           /*  37 */
    RC_ENTRY ( rcQueue, "queue" )                               /*  38 */
    RC_ENTRY ( rcRWLock, "read/write lock" )                    /*  39 */
    RC_ENTRY ( rcSchema, "schema" )                             /*  40 */
    RC_ENTRY ( rcSemaphore, "semaphore" )                       /*  41 */
    RC_ENTRY ( rcStorage, "storage" )                           /*  42 */
    RC_ENTRY ( rcString, "string" )                             /*  43 */
    RC_ENTRY ( rcTable, "table" )                               /*  44 */
    RC_ENTRY ( rcThread, "thread" )                             /*  45 */
    RC_ENTRY ( rcTimeout, "timeout" )                           /*  46 */
    RC_ENTRY ( rcToken, "token" )                               /*  47 */
    RC_ENTRY ( rcTree, "tree" )                                 /*  48 */
    RC_ENTRY ( rcTrie, "trie" )                                 /*  49 */
    RC_ENTRY ( rcType, "type" )                                 /*  50 */
    RC_ENTRY ( rcVector, "vector" )                             /*  51 */
    RC_ENTRY ( rcDylib, "dynamic library" )                     /*  52 */
    RC_ENTRY ( rcExpression, "expression" )                     /*  53 */
    RC_VLAST ( rcLastTarget_v1_0 )                              /*  54 */
    RC_VFIRST ( rcFirstTarget_v1_1, rcLastTarget_v1_0 )         /*  53 */
    RC_ENTRY ( rcProduction, "schema production" )              /*  54 */
    RC_ENTRY ( rcEncryptionKey, "encryption key" )              /*  55 */
    RC_ENTRY ( rcRng, "random number generator" )               /*  56 */
    RC_ENTRY ( rcCmd, "command" )                               /*  57 */
    RC_ENTRY ( rcData, "data" )                                 /*  58 */
    RC_ENTRY ( rcQuery, "query" )                               /*  59 */
    RC_ENTRY ( rcUri, "uri" )                                   /*  60 */
    RC_VLAST  ( rcLastTarget_v1_1 )                             /*  61 */

        /* NB - due to a mistake in how RCObject
           was initialized, remaining RCTarget values
           are NOT compatible with RCObject */

    RC_VFIRST ( rcFirstTarget_v1_2, rcLastTarget_v1_1 )         /*  60 */
    RC_ENTRY ( rcProvider, "provider" )                         /*  61 */
    RC_VLAST  ( rcLastTarget_v1_2 )                             /*  62 */

        /* RCTarget requires 6 bits
           has headroom up to code 63 */
};

/* RCContext
 *  context under which error occurred
 */
RC_ENUM ( RCContext )
{
    RC_ENTRY ( rcAllocating, "allocating" )                     /*   0 */ /* 1 bit  */
    RC_ENTRY ( rcCasting, "type-casting" )                      /*   1 */
    RC_ENTRY ( rcConstructing, "constructing" )                 /*   2 */ /* 2 bits */
    RC_ENTRY ( rcDestroying, "destroying" )                     /*   3 */
    RC_ENTRY ( rcReleasing, "releasing" )                       /*   4 */ /* 3 bits */
    RC_ENTRY ( rcAccessing, "accessing" )                       /*   5 */
    RC_ENTRY ( rcListing, "listing" )                           /*   6 */
    RC_ENTRY ( rcVisiting, "visiting" )                         /*   7 */
    RC_ENTRY ( rcResolving, "resolving" )                       /*   8 */ /* 4 bits */
    RC_ENTRY ( rcLocking, "locking" )                           /*   9 */
    RC_ENTRY ( rcUnlocking, "unlocking" )                       /*  10 */
    RC_ENTRY ( rcRenaming, "renaming" )                         /*  11 */
    RC_ENTRY ( rcAliasing, "aliasing" )                         /*  12 */
    RC_ENTRY ( rcSelecting, "selecting" )                       /*  13 */
    RC_ENTRY ( rcProjecting, "projecting" )                     /*  14 */
    RC_ENTRY ( rcInserting, "inserting" )                       /*  15 */
    RC_ENTRY ( rcRemoving, "removing" )                         /*  16 */ /* 5 bits */
    RC_ENTRY ( rcClearing, "clearing" )                         /*  17 */
    RC_ENTRY ( rcUpdating, "updating" )                         /*  18 */
    RC_ENTRY ( rcCreating, "creating" )                         /*  19 */
    RC_ENTRY ( rcOpening, "opening" )                           /*  20 */
    RC_ENTRY ( rcClosing, "closing" )                           /*  21 */
    RC_ENTRY ( rcResizing, "resizing" )                         /*  22 */
    RC_ENTRY ( rcReading, "reading" )                           /*  23 */
    RC_ENTRY ( rcWriting, "writing" )                           /*  24 */
    RC_ENTRY ( rcCommitting, "committing" )                     /*  25 */
    RC_ENTRY ( rcReverting, "reverting" )                       /*  26 */
    RC_ENTRY ( rcResetting, "resetting" )                       /*  27 */
    RC_ENTRY ( rcPersisting, "persisting" )                     /*  28 */
    RC_ENTRY ( rcFreezing, "freezing" )                         /*  29 */
    RC_ENTRY ( rcCopying, "copying" )                           /*  30 */
    RC_ENTRY ( rcConcatenating, "concatenating" )               /*  31 */
    RC_ENTRY ( rcFormatting, "formatting" )                     /*  32 */ /* 6 bits */
    RC_ENTRY ( rcPositioning, "positioning" )                   /*  33 */
    RC_ENTRY ( rcPacking, "packing" )                           /*  34 */
    RC_ENTRY ( rcUnpacking, "unpacking" )                       /*  35 */
    RC_ENTRY ( rcEncoding, "encoding" )                         /*  36 */
    RC_ENTRY ( rcDecoding,"decoding" )                          /*  37 */
    RC_ENTRY ( rcValidating, "validating" )                     /*  38 */
    RC_ENTRY ( rcExecuting, "executing" )                       /*  39 */
    RC_ENTRY ( rcHuffmanCoding, "Huffman coding" )              /*  40 */
    RC_ENTRY ( rcReindexing, "re-indexing" )                    /*  41 */
    RC_ENTRY ( rcRegistering, "registering" )                   /*  42 */
    RC_ENTRY ( rcTokenizing, "tokenizing" )                     /*  43 */
    RC_ENTRY ( rcParsing, "parsing" )                           /*  44 */
    RC_ENTRY ( rcConverting, "converting" )                     /*  45 */
    RC_ENTRY ( rcSignaling, "signaling" )                       /*  46 */
    RC_ENTRY ( rcWaiting, "waiting" )                           /*  47 */
    RC_ENTRY ( rcAttaching, "attaching" )                       /*  48 */
    RC_ENTRY ( rcDetaching, "detaching" )                       /*  49 */
    RC_ENTRY ( rcLogging, "logging" )                           /*  50 */
    RC_ENTRY ( rcFPCoding, "floating point coding" )            /*  51 */
    RC_ENTRY ( rcMultiplexing, "(de)multiplexing" )             /*  52 */
    RC_ENTRY ( rcClassifying, "classifying" )                   /*  53 */
    RC_ENTRY ( rcSearching, "searching" )                       /*  54 */
    RC_ENTRY ( rcLoading, "loading" )                           /*  55 */
    RC_ENTRY ( rcEvaluating, "evaluating" )                     /*  56 */
    RC_ENTRY ( rcInflating, "inflating" )                       /*  57 */
    RC_VLAST ( rcLastContext_v1_0 )                             /*  58 */
    RC_VFIRST ( rcFirstContext_v1_1, rcLastContext_v1_0 )       /*  57 */
    RC_ENTRY ( rcFlushing, "flushing" )                         /*  58 */
    RC_ENTRY ( rcAppending, "appending" )                       /*  59 */
    RC_ENTRY ( rcEncrypting, "encrypting" )                     /*  60 */
    RC_ENTRY ( rcDecrypting, "decrypting" )                     /*  61 */
    RC_ENTRY ( rcComparing, "comparing" )                       /*  62 */
    RC_ENTRY ( rcInitializing, "initializing" )                 /*  63 */
    RC_ENTRY ( rcRetrieving, "retrieving" )                     /*  64 */ /* 7 bits */
    RC_ENTRY ( rcSending, "sending" )                           /*  65 */
    RC_ENTRY ( rcProcessing, "processing" )                     /*  66 */
    RC_ENTRY ( rcIdentifying, "identifying" )                   /*  67 */
    RC_LAST ( rcLastContext_v1_1 )                              /*  68 */

        /* RCContext requires 7 bits
           has headroom up to code 127 */
};

/* RCObject
 *  type of object described by state
 */
RC_ENUM ( RCObject )
{
    RC_ENTRY ( rcNoObj, NULL )                                  /*   0 */

        /* NB - due to this mistake in how rcFirstObject
           was initialized, no RCTarget value post v1.1
           can be compatible with RCObject */

    RC_VFIRST ( rcFirstObject, rcLastTarget_v1_1 )              /*  60 */
    RC_ENTRY ( rcLink, "symbolic link" )                        /*  61 */
    RC_ENTRY ( rcSelf, "self" )                                 /*  62 */
    RC_ENTRY ( rcParam, "param" )                               /*  63 */
    RC_ENTRY ( rcOffset, "offset" )                             /*  64 */ /* 7 bits */
    RC_ENTRY ( rcMemory, "memory" )                             /*  65 */
    RC_ENTRY ( rcName, "name" )                                 /*  66 */
    RC_ENTRY ( rcFormat, "format" )                             /*  67 */
    RC_ENTRY ( rcTransfer, "transfer" )                         /*  68 */
    RC_ENTRY ( rcInterface, "interface" )                       /*  69 */
    RC_ENTRY ( rcId, "id" )                                     /*  70 */
    RC_ENTRY ( rcRange, "range" )                               /*  71 */
    RC_ENTRY ( rcConstraint, "constraint" )                     /*  72 */
    RC_ENTRY ( rcByteOrder, "byte order" )                      /*  73 */
    RC_ENTRY ( rcMessage, "message" )                           /*  74 */
    RC_ENTRY ( rcTag, "tag" )                                   /*  75 */
    RC_ENTRY ( rcResources, "system resources" )                /*  76 */
    RC_ENTRY ( rcDirEntry, "directory-toc entry" )              /*  77 */
    RC_ENTRY ( rcArcHardLink, "archive hard link" )             /*  78 */
    RC_ENTRY ( rcRow, "row" )                                   /*  79 */
    RC_ENTRY ( rcLibrary, "loadable library" )                  /*  80 */
    RC_VLAST ( rcLastObject_v1_0 )                              /*  81 */
    RC_VFIRST ( rcFirstObject_v1_1, rcLastObject_v1_0 )         /*  80 */
    RC_ENTRY ( rcItem, "item" )                                 /*  81 */
    RC_ENTRY ( rcMode, "mode" )                                 /*  82 */
    RC_ENTRY ( rcEncryption, "encryption" )                     /*  83 */
    RC_ENTRY ( rcCrc, "crc" )                                   /*  84 */
    RC_ENTRY ( rcChecksum, "checksum" )                         /*  85 */
    RC_ENTRY ( rcSeed, "seed" )                                 /*  86 */
    RC_ENTRY ( rcConnection, "connection" )                     /*  87 */
    RC_ENTRY ( rcError, "error" )                               /*  88 */
    RC_ENTRY ( rcEnvironment, "environment" )                   /*  89 */
    RC_ENTRY ( rcSignalSet, "signal set" )                      /*  90 */
    RC_ENTRY ( rcSize, "size" )                                 /*  91 */
    RC_ENTRY ( rcRefcount, "reference count" )                  /*  92 */
    RC_VLAST ( rcLastObject_v1_1 )                              /*  93 */
    RC_VFIRST ( rcFirstObject_v1_2, rcLastObject_v1_1 )         /*  92 */
    RC_ENTRY ( rcCloudProvider, "cloud provider" )              /*  93 */
    RC_ENTRY ( rcComputeEnvTok, "compute environment token" )   /*  94 */
    RC_ENTRY ( rcUserPayCred, "user-pays credential" )          /*  95 */
    RC_VLAST ( rcLastObject_v1_2 )                              /*  96 */

        /* RCObject extends RCTarget
           and requires at least 7 bits - given 8
           has headroom up to code 255 */
};

/* RCState
 *  state of object described
 */
RC_ENUM ( RCState )
{
    RC_ENTRY ( rcNoErr, "no error" )                            /*   0 */ /* 1 bit  */
    RC_ENTRY ( rcDone, "done" )                                 /*   1 */
    RC_ENTRY ( rcUnknown, "unknown" )                           /*   2 */ /* 2 bits */
    RC_ENTRY ( rcUnsupported, "unsupported" )                   /*   3 */
    RC_ENTRY ( rcUnexpected, "unexpected" )                     /*   4 */ /* 3 bits */
    RC_ENTRY ( rcUnrecognized, "unrecognized" )                 /*   5 */
    RC_ENTRY ( rcAmbiguous, "ambiguous" )                       /*   6 */
    RC_ENTRY ( rcNull, "NULL" )                                 /*   7 */
    RC_ENTRY ( rcBadVersion, "bad version" )                    /*   8 */ /* 4 bits */
    RC_ENTRY ( rcDestroyed, "destroyed" )                       /*   9 */
    RC_ENTRY ( rcInvalid, "invalid" )                           /*  10 */
    RC_ENTRY ( rcCorrupt, "corrupt" )                           /*  11 */
    RC_ENTRY ( rcIncorrect, "incorrect" )                       /*  12 */
    RC_ENTRY ( rcInconsistent, "inconsistent" )                 /*  13 */
    RC_ENTRY ( rcBusy, "busy" )                                 /*  14 */
    RC_ENTRY ( rcIncomplete, "incomplete" )                     /*  15 */
    RC_ENTRY ( rcInterrupted, "interrupted" )                   /*  16 */ /* 5 bits */
    RC_ENTRY ( rcCanceled, "canceled" )                         /*  17 */
    RC_ENTRY ( rcEmpty, "empty" )                               /*  18 */
    RC_ENTRY ( rcExhausted, "exhausted" )                       /*  19 */
    RC_ENTRY ( rcInsufficient, "insufficient" )                 /*  20 */
    RC_ENTRY ( rcExcessive, "excessive" )                       /*  21 */
    RC_ENTRY ( rcViolated, "violated" )                         /*  22 */
    RC_ENTRY ( rcExists, "exists" )                             /*  23 */
    RC_ENTRY ( rcNotFound, "not found" )                        /*  24 */
    RC_ENTRY ( rcLocked, "locked" )                             /*  25 */
    RC_ENTRY ( rcUnlocked, "unlocked" )                         /*  26 */
    RC_ENTRY ( rcDetached, "detached" )                         /*  27 */
    RC_ENTRY ( rcDeadlock, "deadlock" )                         /*  28 */
    RC_ENTRY ( rcUnauthorized, "unauthorized" )                 /*  29 */
    RC_ENTRY ( rcReadonly, "read-only" )                        /*  30 */
    RC_ENTRY ( rcWriteonly, "write-only" )                      /*  31 */
    RC_ENTRY ( rcNoPerm, "no permission" )                      /*  32 */ /* 6 bits */
    RC_ENTRY ( rcInPlaceNotAllowed, "update in-place prohibited" ) /*  33 */
    RC_ENTRY ( rcTooShort, "too short" )                        /*  34 */
    RC_ENTRY ( rcTooLong, "too long" )                          /*  35 */
    RC_ENTRY ( rcTooBig, "too big" )                            /*  36 */
    RC_ENTRY ( rcDuplicate, "duplicate" )                       /*  37 */
    RC_ENTRY ( rcOutOfKDirectory, "path out of this KDirectory FS" ) /*  38 */
    RC_ENTRY ( rcIgnored, "ignored" )	                        /*  39 */
    RC_ENTRY ( rcOutofrange, "out of range" )                   /*  40 */
    RC_VLAST ( rcLastState_v1_0 )                               /*  41 */
    RC_VFIRST ( rcFirstState_v1_1, rcLastState_v1_0 )           /*  40 */
    RC_ENTRY ( rcOpen, "open" )                                 /*  41 */
    RC_ENTRY ( rcOutoforder, "out of order" )                   /*  42 */
    RC_ENTRY ( rcNotOpen, "not open" )                          /*  43 */
    RC_ENTRY ( rcUndefined, "undefined" )                       /*  44 */
    RC_ENTRY ( rcUnequal, "unequal" )                           /*  45 */
    RC_ENTRY ( rcFailed, "failed" )                             /*  46 */
    RC_ENTRY ( rcNotAvailable, "not available" )                /*  47 */
    RC_ENTRY ( rcWrongType, "wrong type" )                      /*  48 */
    RC_LAST ( rcLastState_v1_1 )                                /*  49 */

        /* RCState requires 6 bits
           has headroom up to code 63 */
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
