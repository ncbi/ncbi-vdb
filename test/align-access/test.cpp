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

extern "C" {
#include <klib/rc.h>
#include <align/bam.h>
#include <align/align-access.h>
#include <vfs/path.h>
#include <vfs/manager.h>
}

#define VERY_VERBOSE (0)
static bool const verbose = false;

namespace AlignAccess {
    class AlignmentEnumerator;
    class Database;
    class Manager;

    class AlignmentEnumerator {
        friend class Database;
        AlignAccessAlignmentEnumerator *const self;
        mutable bool skip;
        explicit AlignmentEnumerator(AlignAccessAlignmentEnumerator *Self) : self(Self), skip(true) {}
    public:
        AlignmentEnumerator(AlignmentEnumerator const &rhs) : self(rhs.self) {
            rc_t const rc = AlignAccessAlignmentEnumeratorAddRef(self);
            if (rc) throw std::logic_error("AlignAccessAlignmentEnumeratorAddRef failed");
        }
        ~AlignmentEnumerator() {
            AlignAccessAlignmentEnumeratorRelease(self);
        }
        bool next() const {
            if (self) {
                if (skip) {
                    skip = false;
                    return true;
                }
                rc_t const rc = AlignAccessAlignmentEnumeratorNext(self);
                if (rc == 0) return true;
                if ((int)GetRCObject(rc) == rcRow && (int)GetRCState(rc) == rcNotFound)
                    return false;
                throw std::runtime_error("AlignAccessAlignmentEnumeratorNext failed");
            }
            return false;
        }
        int position() const {
            if (self) {
                uint64_t p = 0;
                AlignAccessAlignmentEnumeratorGetRefSeqPos(self, &p);
                return (int)p;
            }
            return -1;
        }
    };
    class Database {
        friend class Manager;
        AlignAccessDB const *const self;
        explicit Database(AlignAccessDB const *Self) : self(Self) {}
    public:
        Database(Database const &rhs) : self(rhs.self) {
            rc_t const rc = AlignAccessDBAddRef(self);
            if (rc) throw std::logic_error("AlignAccessDBAddRef failed");
        }
        ~Database() {
            AlignAccessDBRelease(self);
        }

        AlignmentEnumerator slice(std::string const &refName, int start, int end) const
        {
            AlignAccessAlignmentEnumerator *p = 0;
            if (verbose) std::cerr << "Generating slice " << refName << ':' << (start + 1) << '-' << end << std::endl;
            rc_t const rc = AlignAccessDBWindowedAlignments(self, &p, refName.c_str(), start, end - start);
            if (rc == 0) return AlignmentEnumerator(p);
            AlignAccessAlignmentEnumeratorRelease(p);
            if ((int)GetRCObject(rc) == rcRow && (int)GetRCState(rc) == rcNotFound)
                return AlignmentEnumerator(0);
            throw std::logic_error("AlignAccessDBWindowedAlignments failed");
        }
    };
    class Manager {
        AlignAccessMgr const *const self;
        explicit Manager(AlignAccessMgr const *Self) : self(Self) {}
        Manager() : self(0) {}
    public:
        ~Manager() {
            AlignAccessMgrRelease(self);
        }

        Database open(std::string const &path, std::string const &indexPath) const {
            VPath *dbp = 0;
            VPath *idxp = 0;
            rc_t rc = 0;
            {
                VFSManager *fsm = 0;
                
                rc = VFSManagerMake(&fsm);
                if (rc) throw std::logic_error("VFSManagerMake failed");
            
                rc = VFSManagerMakeSysPath(fsm, &dbp, path.c_str());
                if (rc) throw std::logic_error("VFSManagerMakeSysPath failed");
            
                rc = VFSManagerMakeSysPath(fsm, &idxp, indexPath.c_str());
                if (rc) throw std::logic_error("VFSManagerMakeSysPath failed");

                VFSManagerRelease(fsm);
            }
            AlignAccessDB const *db = 0;
            rc = AlignAccessMgrMakeIndexBAMDB(self, &db, dbp, idxp);
            VPathRelease(dbp);
            VPathRelease(idxp);
            if (rc) throw std::runtime_error(std::string("failed to open ") + path + " with index " + indexPath);
            if (verbose) std::cerr << "opened " << path << " with index " << indexPath << std::endl;
            return Database(db);
        }

        Database open(std::string const &path) const {
            try {
                return open(path, path + ".bai");
            }
            catch (std::runtime_error const &e) {}
            catch (...) { throw; }

            VPath *dbp = 0;
            rc_t rc = 0;
            {
                VFSManager *fsm = 0;

                rc = VFSManagerMake(&fsm);
                if (rc) throw std::logic_error("VFSManagerMake failed");
            
                rc = VFSManagerMakeSysPath(fsm, &dbp, path.c_str());
                if (rc) throw std::logic_error("VFSManagerMakeSysPath failed");
                VFSManagerRelease(fsm);
            }
            AlignAccessDB const *db = 0;
            rc = AlignAccessMgrMakeBAMDB(self, &db, dbp);
            VPathRelease(dbp);
            if (rc) throw std::runtime_error(std::string("failed to open ") + path);
            if (verbose) std::cerr << "opened " << path << std::endl;
            return Database(db);
        }

        static Manager make() {
            AlignAccessMgr const *mgr = 0;
            rc_t rc = AlignAccessMgrMake(&mgr);
            if (rc != 0)
                throw std::logic_error("AlignAccessMgrMake failed");
            return Manager(mgr);
        }
    };
};

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
        AlignAccess::AlignmentEnumerator e = db.slice(ref, start, start + length);
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
            { "chr1", 100000000, 1, 348, -1 },
            { "chr1", 141484029, 11733, 0, -1 },
        };
        int const n = 2;
        bool failed = false;
        for (int i = 0; i < n; ++i)
            test[i].result = test1Range(test[i].ref, test[i].start, test[i].length);
        for (int i = 0; i < n; ++i) {
            std::cout << "slice " << test[i].ref << ':' << test[i].start + 1 << '-' << test[i].start + test[i].length << " should have " << test[i].expected << " alignments, got " << test[i].result << std::endl;
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
            { "1", 1100000, 100000, 10387, -1 },
            { "1", 1200000, 100000, 14957, -1 },
        };
        int const n = 2;
        bool failed = false;
        for (int i = 0; i < n; ++i)
            test[i].result = test1Range(test[i].ref, test[i].start, test[i].length);
        for (int i = 0; i < n; ++i) {
            std::cout << "slice " << test[i].ref << ':' << test[i].start + 1 << '-' << test[i].start + test[i].length << " should have " << test[i].expected << " alignments, got " << test[i].result << std::endl;
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
    AlignAccess::AlignmentEnumerator e = db.slice(ref, start, end);
    int first = 0;
    int last = 0;
    int records = 0;
    while (e.next()) {
        last = e.position() + 1;
        if (first == 0)
            first = last;
        if (verbose)
            std::cout << "# " << last << std::endl;
        ++records;
    }
    std::cout << path << ' ' << ref << ':' << start << '-' << end << ' ' << records << ' ' << first << ' ' << last << std::endl;
}

static int testMain(int argc, char *argv[])
{
    if (argc >= 5) {
        test1Range(argv[1], argv[2], atoi(argv[3]), atoi(argv[4]), argc > 5);
        return 0;
    }
    return IndexTestSuite(argc, argv);
}

//////////////////////////////////////////// Main
#include <kapp/args.h>
#include <klib/out.h>
#include <kfg/config.h>

extern "C"
{
    
ver_t CC KAppVersion ( void )
{
    return 0x1000000;
}

const char UsageDefaultName[] = "test-loader";

rc_t CC UsageSummary (const char * progname)
{
    return KOutMsg ( "Usage:\n" "\t%s [options]\n\n", progname );
}

rc_t CC Usage( const Args* args )
{
    return 0;
}

rc_t CC KMain ( int argc, char *argv [] )
{
    return testMain(argc, argv);
}

}

