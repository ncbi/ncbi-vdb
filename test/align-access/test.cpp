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

/**
* Unit tests for the Loader module
*/
#include <ktst/unit_test.hpp>

#include <klib/printf.h>

#include <stdexcept> 
#include <string>
#include <algorithm>

#define VERY_VERBOSE (0)
static bool const verbose = false;

#include "align-access.hpp"

using namespace std;

TEST_SUITE(IndexTestSuite);

#if 1
class LoaderFixture1
{
    AlignAccess::Database const db;

    static std::string BAM_FILE_NAME(void) {
#if MAC
        return std::string("/net/traces04/giab05/ftp/data/AshkenazimTrio/HG002_NA24385_son/NIST_HiSeq_HG002_Homogeneity-10953946/NHGRI_Illumina300X_AJtrio_novoalign_bams/HG002.GRCh38.300x.bam");
#else
        return std::string("/netmnt/traces04/giab05/ftp/data/AshkenazimTrio/HG002_NA24385_son/NIST_HiSeq_HG002_Homogeneity-10953946/NHGRI_Illumina300X_AJtrio_novoalign_bams/HG002.GRCh38.300x.bam");
#endif
    }
    int test1Range(char const *ref, int const start, int const length) const {
        int alignments = 0;
        AlignAccess::AlignmentEnumerator e = db.slice(ref, start - 1, (start - 1) + length);
        while (e.next()) {
            ++alignments;
        }
        return alignments;
    }
public:
    LoaderFixture1() : db(AlignAccess::Manager::make().open(BAM_FILE_NAME()))
    {
    }
    void testIndex(void) const {
        struct {
            char const *ref;
            int start;
            int length;
            int expected;
            int result;
        } test[] = {
            { "chr1", 100000001, 1, 347, -1 },
            { "chr1", 141484030, 11733, 0, -1 },
        };
        int const n = 2;
        bool failed = false;
        for (int i = 0; i < n; ++i)
            test[i].result = test1Range(test[i].ref, test[i].start, test[i].length);
        for (int i = 0; i < n; ++i) {
            std::cout << "slice " << test[i].ref << ':' << test[i].start << '-' << (test[i].start - 1) + test[i].length << " should have " << test[i].expected << " alignments, got " << test[i].result << std::endl;
            if (test[i].result != test[i].expected) {
                failed = true;
            }
        }
        if (failed)
            throw std::runtime_error("failed");
    }
};    

FIXTURE_TEST_CASE ( LoadIndex1, LoaderFixture1 ) 
{
    testIndex();
}
#endif

#if 1
class LoaderFixture2
{
    AlignAccess::Database const db;

    static std::string BAM_FILE_NAME(void) {
#if MAC
        return std::string("/net/snowman/vol/projects/toolkit_test_data/traces04//1000genomes3/ftp/data/NA19240/exome_alignment/NA19240.mapped.SOLID.bfast.YRI.exome.20111114.bam");
#else
        return std::string("/net/snowman/vol/projects/toolkit_test_data/traces04//1000genomes3/ftp/data/NA19240/exome_alignment/NA19240.mapped.SOLID.bfast.YRI.exome.20111114.bam");
#endif
    }
    int test1Range(char const *ref, int const start, int const length) const {
        int alignments = 0;
        AlignAccess::AlignmentEnumerator e = db.slice(ref, start, start + length);
        while (e.next()) {
            ++alignments;
        }
        return alignments;
    }
public:
    LoaderFixture2() : db(AlignAccess::Manager::make().open(BAM_FILE_NAME()))
    {
    }
    void testIndex(void) const {
        struct {
            char const *ref;
            int start;
            int length;
            int expected;
            int result;
        } test[] = {
            { "1", 1100001, 100000, 10387, -1 },
            { "1", 1200001, 100000, 14957, -1 },
        };
        int const n = 2;
        bool failed = false;
        for (int i = 0; i < n; ++i)
            test[i].result = test1Range(test[i].ref, test[i].start, test[i].length);
        for (int i = 0; i < n; ++i) {
            std::cout << "slice " << test[i].ref << ':' << test[i].start << '-' << (test[i].start - 1) + test[i].length << " should have " << test[i].expected << " alignments, got " << test[i].result << std::endl;
            if (test[i].result != test[i].expected) {
                failed = true;
            }
        }
        if (failed)
            throw std::runtime_error("failed");
    }
};    

FIXTURE_TEST_CASE ( LoadIndex2, LoaderFixture2 ) 
{
    testIndex();
}
#endif

static void test1Range(char const *path, char const *ref, int const start, int const end, bool const verbose)
{
    AlignAccess::Database const db(AlignAccess::Manager::make().open(path));
    AlignAccess::AlignmentEnumerator e = db.slice(ref, start - 1, end);
    int first = 0;
    int last = 0;
    int records = 0;
    while (e.next()) {
        last = e.position() + 1;
        if (first == 0)
            first = last;
        if (verbose)
            std::cout << "# " << e.SAM();
        ++records;
    }
    std::cout << path << ' ' << ref << ':' << start << '-' << end << ' ' << records << ' ' << first << ' ' << last << std::endl;
}

extern "C" int testMain(int, char **);
int testMain(int argc, char *argv[])
{
    if (argc >= 5) {
        test1Range(argv[1], argv[2], atoi(argv[3]), atoi(argv[4]), argc > 5);
        return 0;
    }
    return IndexTestSuite(argc, argv);
}

