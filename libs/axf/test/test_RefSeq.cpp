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

#include <iostream>
#include <string>
#include <cstdlib> // for abort, getenv, random

#define TESTING 1
#include "../refseq.hpp"

#undef TESTING

static bool verbose = false;
#define VERBOSE if (!verbose) {} else std::cerr

int main(int argc, char **argv)
{
    {
        auto const envvar = getenv("VERBOSE");
        auto const verbose_arg = (argc > 1 && std::string(argv[1]) == "-v");
        auto const verbose_env = (envvar != NULL && std::string(envvar) == "1");

        verbose = verbose_arg || verbose_env;
    }
    srandom(time(0));

    if (RefSeq::test())
        std::cerr << "RefSeq::test ok" << std::endl;
    else
        return 1;

    return 0;
}

#include <sstream>

bool RefSeq::test() {
    auto const seq = std::string("NNNACGTNNN");
    auto testSequence = [&](RefSeq const &rs) -> bool {
        char test[10];
        unsigned len;

        memset(test, 0, 10);
        len = rs.getBases(test, 0, 10);
        if (len != 10) {
            std::cerr << "RefSeq test failed: length should be 10, not " << len << std::endl;
            return false;
        }
        if (std::string(test, len) != seq) {
            std::cerr << "RefSeq test failed: sequence should be '" << seq << "', not '" << std::string(test, len) << "'" << std::endl;
            return false;
        }

        memset(test, 0, 10);
        len = rs.getBases(test, 0, 5);
        if (len != 5) {
            std::cerr << "RefSeq test failed: length should be 5, not " << len << std::endl;
            return false;
        }
        if (std::string(test, 5) != seq.substr(0, 5)) {
            std::cerr << "RefSeq test failed: sequence should be '" << seq.substr(0, 5) << "', not '" << std::string(test, len) << "'" << std::endl;
            return false;
        }

        memset(test, 0, 10);
        len = rs.getBases(test, 5, 5);
        if (len != 5) {
            std::cerr << "RefSeq test failed: length should be 5, not " << len << std::endl;
            return false;
        }
        if (std::string(test, len) != seq.substr(5)) {
            std::cerr << "RefSeq test failed: sequence should be '" << seq.substr(5) << "', not '" << std::string(test, len) << "'" << std::endl;
            return false;
        }
        return true;
    };
    {
        auto const &rs = RefSeq::loadFromStream(std::istringstream(seq));

        if (rs.length != 10) {
            std::cerr << "RefSeq test failed: length should be 10, not " << rs.length << std::endl;
            return false;
        }
        if (std::distance(rs.Ns.begin(), rs.Ns.end()) != 2) {
            std::cerr << "RefSeq test failed: " << rs.Ns << " should have 2 ranges" << std::endl;
            return false;
        }
        if (!testSequence(rs))
            return false;
    }
    {
        auto const &rs = RefSeq::loadFromStreamCircular(std::istringstream(seq));

        if (rs.length != 10) {
            std::cerr << "RefSeq test failed: length should be 10, not " << rs.length << std::endl;
            return false;
        }
        if (!rs.Ns.empty()) {
            std::cerr << "RefSeq test failed: " << rs.Ns << " should be empty" << std::endl;
            return false;
        }
        if (!testSequence(rs))
            return false;
    }
    return true;
}

#include <cctype>
RefSeq RefSeq::loadFromStream(std::istream && input) {
    RangeList Ns;
    std::vector<uint8_t> bases;
    unsigned position = 0;

    int n = 0;
    int accum = 0;

    char ch;
    while (input.get(ch)) {
        if (isspace(ch)) continue;

        int b4na = 0;
        int N = 1;

        switch (ch) {
        case 'A':
        case 'a':
            b4na = 0;
            N = 0;
            break;
        case 'C':
        case 'c':
            b4na = 1;
            N = 0;
            break;
        case 'G':
        case 'g':
            b4na = 2;
            N = 0;
            break;
        case 'T':
        case 't':
            b4na = 3;
            N = 0;
            break;
        default:
            break;
        }
        accum = (accum << 2) | b4na;
        n += 1;
        if (n == 4) {
            bases.push_back(accum);
            accum = 0;
            n = 0;
        }
        if (N)
            Ns.add(position);
        position += 1;
    }
    if (n != 0) {
        while (n < 4) {
            accum <<= 2;
            n += 1;
        }
        bases.push_back(accum);
    }
    return RefSeq(std::move(Ns), std::move(bases), position);
}

RefSeq RefSeq::loadFromStreamCircular(std::istream && input) {
    std::vector<uint8_t> bases;
    unsigned position = 0;

    int n = 0;
    int accum = 0;

    char ch;
    while (input.get(ch)) {
        if (isspace(ch)) continue;

        int b2na = 0;

        switch (ch) {
        case 'A':
        case 'a':
            b2na = 1;
            break;
        case 'C':
        case 'c':
            b2na = 2;
            break;
        case 'G':
        case 'g':
            b2na = 4;
            break;
        case 'T':
        case 't':
            b2na = 8;
            break;
        default:
            break;
        }
        accum = (accum << 4) | b2na;
        n += 1;
        if (n == 2) {
            bases.push_back(accum);
            accum = 0;
            n = 0;
        }
        position += 1;
    }
    if (n != 0) {
        while (n < 2) {
            accum <<= 4;
            n += 1;
        }
        bases.push_back(accum);
    }
    return RefSeq(std::move(bases), position);
}
