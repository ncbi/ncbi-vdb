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

#include <kfc/rc.h>
#include <ktst/unit_test.hpp>

#include <cstdlib>
#include <cstring>
#include <set>
#include <stdexcept>
#include <stdint.h>
#include <utility>
#include <cstdio>

TEST_SUITE( KlibTestSuite );


TEST_SUITE( TestRCEnumSuite );

TEST_CASE( test )
{
    /////////////////////////////////////////////////////////////////////////////

    typedef struct senums
    {
        const char * rcstr;
        int rcenum;
        int constval;
    } senums;


#define __stringify_1(x...) #x
#define __stringify(x...) __stringify_1(x)
#define RCIT(RCENUM, VAL)  { __stringify(RCENUM) , RCENUM , VAL }
    senums enums[] = {
        RCIT(rcAccessing, 5),
        RCIT(rcAliasing, 12),
        RCIT(rcAlign, 15),
        RCIT(rcAllocating, 0),
        RCIT(rcAmbiguous, 6),
        RCIT(rcApp, 11),
        RCIT(rcAppending, 59),
        RCIT(rcArc, 1),
        RCIT(rcArcHardLink, 78),
        RCIT(rcArgv, 4),
        RCIT(rcAttaching, 48),
        RCIT(rcAttr, 5),
        RCIT(rcBadVersion, 8),
        RCIT(rcBarrier, 6),
        RCIT(rcBlob, 7),
        RCIT(rcBuffer, 8),
        RCIT(rcBusy, 14),
        RCIT(rcByteOrder, 73),
        RCIT(rcCS, 4),
        RCIT(rcCanceled, 17),
        RCIT(rcCasting, 1),
        RCIT(rcChar, 9),
        RCIT(rcChecksum, 85),
        RCIT(rcClassifying, 53),
        RCIT(rcClearing, 17),
        RCIT(rcClosing, 21),
        RCIT(rcCmd, 57),
        RCIT(rcColumn, 10),
        RCIT(rcCommitting, 25),
        RCIT(rcComparing, 62),
        RCIT(rcConcatenating, 31),
        RCIT(rcCondition, 11),
        RCIT(rcConnection, 87),
        RCIT(rcConstraint, 72),
        RCIT(rcConstructing, 2),
        RCIT(rcCont, 3),
        RCIT(rcConverting, 45),
        RCIT(rcCopying, 30),
        RCIT(rcCorrupt, 11),
        RCIT(rcCrc, 84),
        RCIT(rcCreating, 19),
        RCIT(rcCursor, 12),
        RCIT(rcDB, 9),
        RCIT(rcData, 58),
        RCIT(rcDatabase, 13),
        RCIT(rcDeadlock, 28),
        RCIT(rcDecoding, 37),
        RCIT(rcDecrypting, 61),
        RCIT(rcDestroyed, 9),
        RCIT(rcDestroying, 3),
        RCIT(rcDetached, 27),
        RCIT(rcDetaching, 49),
        RCIT(rcDirEntry, 77),
        RCIT(rcDirectory, 14),
        RCIT(rcDoc, 15),
        RCIT(rcDone, 1),
        RCIT(rcDuplicate, 37),
        RCIT(rcDylib, 52),
        RCIT(rcEmpty, 18),
        RCIT(rcEncoding, 36),
        RCIT(rcEncrypting, 60),
        RCIT(rcEncryption, 83),
        RCIT(rcEncryptionKey, 55),
        RCIT(rcEnvironment, 89),
        RCIT(rcError, 88),
        RCIT(rcEvaluating, 56),
        RCIT(rcExcessive, 21),
        RCIT(rcExe, 0),
        RCIT(rcExecuting, 39),
        RCIT(rcExhausted, 19),
        RCIT(rcExists, 23),
        RCIT(rcExitCode, 20),
        RCIT(rcExpression, 53),
        RCIT(rcFF, 5),
        RCIT(rcFPCoding, 51),
        RCIT(rcFS, 6),
        RCIT(rcFailed, 46),
        RCIT(rcFile, 17),
        RCIT(rcFileDesc, 18),
        RCIT(rcFileFormat, 19),
        RCIT(rcFirstContext_v1_1, 57),
        RCIT(rcFirstModule_v1_1, 13),
        RCIT(rcFirstModule_v1_2, 19),
        RCIT(rcFirstObject, 60),
        RCIT(rcFirstObject_v1_1, 80),
        RCIT(rcFirstState_v1_1, 40),
        RCIT(rcFirstTarget_v1_1, 53),
        RCIT(rcFlushing, 58),
        RCIT(rcFormat, 67),
        RCIT(rcFormatter, 21),
        RCIT(rcFormatting, 32),
        RCIT(rcFreezing, 29),
        RCIT(rcFunctParam, 22),
        RCIT(rcFunction, 20),
        RCIT(rcHeader, 23),
        RCIT(rcHuffmanCoding, 40),
        RCIT(rcId, 70),
        RCIT(rcIdentifying, 67),
        RCIT(rcIgnored, 39),
        RCIT(rcInPlaceNotAllowed, 33),
        RCIT(rcIncomplete, 15),
        RCIT(rcInconsistent, 13),
        RCIT(rcIncorrect, 12),
        RCIT(rcIndex, 24),
        RCIT(rcInflating, 57),
        RCIT(rcInitializing, 63),
        RCIT(rcInserting, 15),
        RCIT(rcInsufficient, 20),
        RCIT(rcInterface, 69),
        RCIT(rcInterrupted, 16),
        RCIT(rcInvalid, 10),
        RCIT(rcItem, 81),
        RCIT(rcIterator, 25),
        RCIT(rcKFG, 14),
        RCIT(rcKrypto, 16),
        RCIT(rcLibrary, 80),
        RCIT(rcLink, 61),
        RCIT(rcListing, 6),
        RCIT(rcLoading, 55),
        RCIT(rcLock, 26),
        RCIT(rcLocked, 25),
        RCIT(rcLocking, 9),
        RCIT(rcLog, 27),
        RCIT(rcLogging, 50),
        RCIT(rcMD5SumFmt, 28),
        RCIT(rcMemMap, 29),
        RCIT(rcMemory, 65),
        RCIT(rcMessage, 74),
        RCIT(rcMetadata, 30),
        RCIT(rcMgr, 31),
        RCIT(rcMode, 82),
        RCIT(rcMultiplexing, 52),
        RCIT(rcNS, 18),
        RCIT(rcName, 66),
        RCIT(rcNamelist, 32),
        RCIT(rcNoErr, 0),
        RCIT(rcNoObj, 0),
        RCIT(rcNoPerm, 32),
        RCIT(rcNoTarg, 0),
        RCIT(rcNode, 33),
        RCIT(rcNotAvailable, 47),
        RCIT(rcNotFound, 24),
        RCIT(rcNotOpen, 43),
        RCIT(rcNull, 7),
        RCIT(rcNumeral, 34),
        RCIT(rcOffset, 64),
        RCIT(rcOpen, 41),
        RCIT(rcOpening, 20),
        RCIT(rcOutOfKDirectory, 38),
        RCIT(rcOutoforder, 42),
        RCIT(rcOutofrange, 40),
        RCIT(rcPS, 7),
        RCIT(rcPacking, 34),
        RCIT(rcPagemap, 35),
        RCIT(rcParam, 63),
        RCIT(rcParsing, 44),
        RCIT(rcPath, 36),
        RCIT(rcPersisting, 28),
        RCIT(rcPositioning, 33),
        RCIT(rcProcess, 37),
        RCIT(rcProcessing, 66),
        RCIT(rcProduction, 54),
        RCIT(rcProjecting, 14),
        RCIT(rcQuery, 59),
        RCIT(rcQueue, 38),
        RCIT(rcRDBMS, 17),
        RCIT(rcRWLock, 39),
        RCIT(rcRange, 71),
        RCIT(rcReading, 23),
        RCIT(rcReadonly, 30),
        RCIT(rcRefcount, 92),
        RCIT(rcRegistering, 42),
        RCIT(rcReindexing, 41),
        RCIT(rcReleasing, 4),
        RCIT(rcRemoving, 16),
        RCIT(rcRenaming, 11),
        RCIT(rcResetting, 27),
        RCIT(rcResizing, 22),
        RCIT(rcResolving, 8),
        RCIT(rcResources, 76),
        RCIT(rcRetrieving, 64),
        RCIT(rcReverting, 26),
        RCIT(rcRng, 56),
        RCIT(rcRow, 79),
        RCIT(rcRuntime, 1),
        RCIT(rcSRA, 13),
        RCIT(rcSchema, 40),
        RCIT(rcSearching, 54),
        RCIT(rcSeed, 86),
        RCIT(rcSelecting, 13),
        RCIT(rcSelf, 62),
        RCIT(rcSemaphore, 41),
        RCIT(rcSending, 65),
        RCIT(rcSignalSet, 90),
        RCIT(rcSignaling, 46),
        RCIT(rcSize, 91),
        RCIT(rcStorage, 42),
        RCIT(rcString, 43),
        RCIT(rcTable, 44),
        RCIT(rcTag, 75),
        RCIT(rcText, 2),
        RCIT(rcThread, 45),
        RCIT(rcTimeout, 46),
        RCIT(rcTLS, 21),
        RCIT(rcToc, 2),
        RCIT(rcTocEntry, 3),
        RCIT(rcToken, 47),
        RCIT(rcTokenizing, 43),
        RCIT(rcTooBig, 36),
        RCIT(rcTooLong, 35),
        RCIT(rcTooShort, 34),
        RCIT(rcTransfer, 68),
        RCIT(rcTree, 48),
        RCIT(rcTrie, 49),
        RCIT(rcType, 50),
        RCIT(rcUnauthorized, 29),
        RCIT(rcUndefined, 44),
        RCIT(rcUnequal, 45),
        RCIT(rcUnexpected, 4),
        RCIT(rcUnknown, 2),
        RCIT(rcUnlocked, 26),
        RCIT(rcUnlocking, 10),
        RCIT(rcUnpacking, 35),
        RCIT(rcUnrecognized, 5),
        RCIT(rcUnsupported, 3),
        RCIT(rcUpdating, 18),
        RCIT(rcUri, 60),
        RCIT(rcVDB, 10),
        RCIT(rcVFS, 19),
        RCIT(rcValidating, 38),
        RCIT(rcVector, 51),
        RCIT(rcViolated, 22),
        RCIT(rcVisiting, 7),
        RCIT(rcWaiting, 47),
        RCIT(rcWriteonly, 31),
        RCIT(rcWriting, 24),
        RCIT(rcWrongType, 48),
        RCIT(rcXF, 8),
        RCIT(rcXML, 12)
    };


    size_t numtests = sizeof( enums ) / sizeof( enums[0] );

    fprintf(stderr, "Checking %zu RC_ENUMs for ABI compatibility", numtests );
    TEST_MESSAGE( "Checking RC_ENUMs for ABI compatibility");


    for ( size_t i = 0; i != numtests; ++i )
    {
        int rcenum = enums[i].rcenum;
        int constval = enums[i].constval;
        const char * rcstr = enums[i].rcstr;

        if ( rcenum != constval )
        {
            fprintf( stderr, "RC ABI breakage #%zu %d!=%d\n", i, rcenum, constval);
            fprintf(stderr, "\t(if correct, replace source with: RCIT(%s, %d)\n", rcstr, rcenum);
            REQUIRE_EQ( rcenum, constval);
        }
    }

    ////////////////////////////////////////////////////////////////////////////////
}

extern "C" {
    ver_t CC KAppVersion( void ) { return 0; }
    rc_t CC KMain( int argc, char * argv[] )
    {
        return TestRCEnumSuite( argc, argv );
    }
}
