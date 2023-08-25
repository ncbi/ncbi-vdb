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
* Unit tests for Reference config
*/

#include <klib/rc.h>
#include <klib/debug.h>
#include <kapp/args.h>
#include <vdb/manager.h>
#include <vdb/database.h>
#include <align/writer-reference.h>

#include <ktst/unit_test.hpp>
#include <ktst/unit_test_suite.hpp>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>

struct ErrorCode {
    rc_t rc;

    static inline void throwIf(rc_t const rc) {
        if (rc) throw ErrorCode{ rc };
    }
};

class cArgs {
    Args *args;
public:
    Args *take() {
        Args *p = args;
        args = nullptr;
        return p;
    }
    ~cArgs() { ArgsWhack(args); }
    cArgs(int argc, char *argv[], int tableSize, OptDef const table[])
    : args(nullptr)
    {
        ErrorCode::throwIf(ArgsMakeAndHandle(&args, argc, argv, 1, table, tableSize));
    }
    #define ARGS(DEFS) cArgs{ argc, argv, sizeof(DEFS)/sizeof(OptDef), DEFS }
    
    unsigned countOf(char const *name) const {
        uint32_t count = 0;
        ErrorCode::throwIf(ArgsOptionCount(args, name, &count));
        return count;
    }
    unsigned countOf(OptDef const &optDef) const {
        return countOf(optDef.name);
    }
    char const *valueOf(char const *name, unsigned index = 0) const {
        auto const count = countOf(name);
        if (index < count) {
            void const *value = nullptr;
            ErrorCode::throwIf(ArgsOptionValue(args, name, index, &value));
            return reinterpret_cast<char const *>(value);
        }
        return nullptr;
    }
    char const *valueOf(OptDef const &optDef, unsigned index = 0) const {
        return valueOf(optDef.name, index);
    }
};

struct CommandLine {
    CommandLine(int argc, char *argv[])
    : args_p(nullptr)
    , opt_skip_verify(false)
    , opt_only_verify(false)
    , opt_config_file(nullptr)
    , opt_fasta_file(nullptr)
    , references(nullptr)
    {
        char const *skip_verify_help[] = { "skip verifying config", NULL };
        char const *only_verify_help[] = { "don't attempt to lookup references", NULL };
        char const *config_help[] = { "path to config file", NULL };
        char const *ref_file_help[] = { "path to fasta file with references", NULL };
        char const *ref_list_help[] = { "path to file with list of references to lookup (else use stdin)", NULL };
        OptDef defs[] = {
            { "skip-verify", NULL, NULL, skip_verify_help, OPT_UNLIM, false, false, NULL },
            { "only-verify", NULL, NULL, only_verify_help, OPT_UNLIM, false, false, NULL },
            { "config"     , "k" , NULL, config_help     , 1, true, false, NULL },
            { "ref-file"   , "r" , NULL, ref_file_help   , 1, true, false, NULL },
            { "ref-list"   , NULL, NULL, ref_list_help   , 1, true, false, NULL },
        };
        auto args = ARGS(defs);
        
        opt_skip_verify = args.countOf(defs[0]) > 0;
        opt_only_verify = args.countOf(defs[1]) > 0;
        opt_config_file = args.valueOf(defs[2]);
        opt_fasta_file  = args.valueOf(defs[3]);

        auto const ref_list_path = args.valueOf(defs[4]);
        if (ref_list_path) {
            references = new std::ifstream();

            auto const save = references->exceptions();
            references->exceptions(std::ios::failbit);
            references->open(ref_list_path, std::ios::in);
            references->exceptions(save);
        }
        args_p = args.take();
    }
    ~CommandLine() { 
        delete references; 
        ArgsWhack(args_p);
    }

    char const *config_file() const {
        return opt_config_file;
    }

    char const *fasta_file() const {
        return opt_fasta_file;
    }

    std::istream &references_file() const {
        return references ? *references : std::cin;
    }
    
    /// is `--skip-verify` on   
    bool skip_verify() const { return opt_skip_verify; }
    
    /// is `--only-verify` on   
    bool only_verify() const { return opt_only_verify; }

private:
    Args *args_p;
    bool opt_skip_verify;
    bool opt_only_verify;
    char const *opt_config_file;
    char const *opt_fasta_file;

    mutable std::ifstream *references = nullptr;
};
static CommandLine const *arguments;

struct ConfigFile {
    struct Entry {
    	std::string name;
        std::string seqId;
        std::string extra;
        
        Entry() = default;
        Entry(std::string const &line)
        {
            std::istringstream strm(line);
            
            strm >> name >> seqId;
            if (strm) {
                strm >> std::ws;
                std::getline(strm, extra);
            }
            else
                throw std::ios_base::failure("unparsable");
        }

        bool circular() const {
            auto const at = extra.find("circular");
            if (at != std::string::npos) {
                if (at == 0 || isspace(extra[at - 1])) {
                    if (at + 8 <= extra.size() || isspace(extra[at + 8])) {
                        return true;
                    }
                }
            }
            return false;
        }
    };
    std::vector<Entry> entries;
    
    ConfigFile(char const *filePath) {
        auto file = std::ifstream{ filePath };
        
        file >> std::ws;
        for (std::string line; std::getline(file, line); file >> std::ws) {
#if ALLOW_COMMENT_LINES
            if (line.substr(0, 1) == "#")
                continue;
#endif
            try {
                Entry e(line);
                entries.emplace_back(e);
            }
            catch (std::ios_base::failure const &ex) {
                throw std::ios_base::failure("unparsable config file");
                (void)ex;
            }
        }
    }
    Entry const *find(std::string const &id) const {
        for (auto & e : entries) {
            if (e.name == id)
                return &e;
        }
        return nullptr;
    }
};

static std::vector<std::string> referenceList(CommandLine const &cmdline)
{
    auto result = std::vector<std::string>{};
    auto &strm = cmdline.references_file();

    for ( ; ; ) {
        auto word = std::string{};
        if (!(strm >> word))
            break;
        result.emplace_back(word);
    }
    return result;
}

using namespace std;
using ncbi::NK::test_skipped;

struct Fixture {
    Fixture()
    : mgr(referenceManager("db/empty.config"))
    {}
    
    Fixture(CommandLine const &cmdline)
    : mgr(referenceManager(cmdline.config_file()))
    {
        auto const fastaFile = cmdline.fasta_file();
        if (fastaFile) {
            auto const rc = ReferenceMgr_FastaPath(mgr, fastaFile);
            if (rc)
                throw ErrorCode{ rc };
        }
    }
    
    ~Fixture() {
        ReferenceMgr_Release(mgr, false, nullptr, false, nullptr);
    }
    void loadFasta(char const *path) const {
        auto const rc = ReferenceMgr_FastaPath(mgr, path);
        if (rc)
            throw ErrorCode{ rc };
    }
    ReferenceSeq const *findSeq(char const *key) const {
    	return ReferenceMgr_FindSeq(mgr, key);
    }
    ReferenceSeq const *findSeq(std::string const &key) const {
    	return findSeq(key.c_str());
    }
    ReferenceSeq const *getSeq(char const *key, bool *shouldUnmap = nullptr, bool* wasRenamed = nullptr) const
    {
        bool dummy1{ false }, dummy2{ false };
        ReferenceSeq const *seq{ nullptr };
        auto const rc = ReferenceMgr_GetSeq(mgr, &seq, key, shouldUnmap ? shouldUnmap : &dummy1, false, wasRenamed ? wasRenamed : &dummy2);
        return rc == 0 ? seq : nullptr;
    }
    ReferenceSeq const *getSeq(std::string const &key, bool *shouldUnmap = nullptr, bool* wasRenamed = nullptr) const
    {
        return getSeq(key.c_str(), shouldUnmap, wasRenamed);
    }
    bool verifySeq(char const *key, bool allowMultiMapping = true, bool* wasRenamed = nullptr, unsigned length = 0, uint8_t const *md5 = nullptr) const
    {
        bool dummy{ false };
        auto rc = ReferenceMgr_Verify(mgr, key, length, md5, allowMultiMapping, wasRenamed ? wasRenamed : &dummy);
        if (length == 0 && (int)GetRCObject(rc) == rcSize && (int)GetRCState(rc) == rcUnequal)
        	rc = 0;
        return rc == 0 || ((int)GetRCObject(rc) == rcId && (int)GetRCState(rc) == rcUndefined);
    }
    bool verifySeq(std::string const &key, bool allowMultiMapping = true, bool* wasRenamed = nullptr, unsigned length = 0, uint8_t const *md5 = nullptr) const
    {
        return verifySeq(key.c_str(), allowMultiMapping, wasRenamed, length, md5);
    }
private:
    ReferenceMgr const *mgr;

    static VDBManager *updateManager()
    {
        VDBManager *vmgr = nullptr;
        rc_t const rc = VDBManagerMakeUpdate(&vmgr, nullptr);
        if (rc)
            throw ErrorCode{ rc };
        return vmgr;
    }

    static ReferenceMgr const *referenceManager(char const *conf
                                                , char const *path = "."
                                                , VDBManager *vmgr_p = nullptr
                                                )
    {
        auto const vmgr = vmgr_p ? vmgr_p : updateManager();
        ReferenceMgr const *rmgr = nullptr;
        auto const rc = ReferenceMgr_Make(&rmgr, nullptr, vmgr, 0, conf, path, 5000, 1024, 4);
        if (vmgr_p == nullptr)
            VDBManagerRelease(vmgr);
        if (rc)
            throw ErrorCode{ rc };
        return rmgr;
    }
};

TEST_SUITE(LoaderTestSuite);

TEST_CASE ( LoadNoConfig )
{
    auto const h = Fixture{ };
    
    // not a valid RefSeq accession
    REQUIRE_NULL(h.findSeq("NC_000000"));
    REQUIRE_NOT_NULL(h.getSeq("NC_000001.11"));
}

TEST_CASE ( LoadConfig )
{
    if (arguments == nullptr)
        throw std::logic_error("no command line arguments!?");
    
    auto const &args = *arguments;
    auto const configFile = args.config_file();
    if (configFile == nullptr)
        throw test_skipped{ "no config file" };
    
    auto const fixture = Fixture{ args };
    auto const config = ConfigFile{ configFile };
    
    for (auto & entry : config.entries) {
        auto const seq = fixture.findSeq(entry.name);
        bool circular = false;

		// must be able to find every name that was in the config file
		REQUIRE_NOT_NULL(seq);

        REQUIRE_RC(ReferenceSeq_IsCircular(seq, &circular));
        REQUIRE_EQ(entry.circular(), circular);
    }
}

TEST_CASE ( VerifyConfig )
{
    if (arguments == nullptr)
        throw std::logic_error("no command line arguments!?");
    
    auto const &args = *arguments;
    auto const configFile = args.config_file();
    if (configFile == nullptr)
        throw test_skipped{ "no config file" };
    
    auto const references = referenceList(args);
    if (references.empty())
        throw test_skipped{ "no reference list" };

    auto const config = ConfigFile{ configFile };
    auto const fixture = Fixture{ args };

    if (!arguments->skip_verify()) {
        for (auto & ref : references) {
            REQUIRE(fixture.verifySeq(ref));
        }
    }
    if (!arguments->only_verify()) {
        for (auto & ref : references) {
            auto const seq = fixture.getSeq(ref);

			REQUIRE_NOT_NULL(seq);

            auto const e = config.find(ref);
            if (e && e->circular()) {
                bool circular = false;
                REQUIRE_RC(ReferenceSeq_IsCircular(seq, &circular));
                REQUIRE(circular);
            }
        }
    }
}

//////////////////////////////////////////// Main
#include <kapp/main.h>
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
    KConfigDisableUserSettings();
    {
        auto const args = CommandLine(argc, argv);
        arguments = &args;
        return LoaderTestSuite(argc, argv);
    }
}

}

