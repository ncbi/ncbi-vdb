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

#include <string>
#include <cstdint>

struct WGS {
private:
    using ReadResult = struct { uint8_t const *string; unsigned length; };

    struct VPath const *url;
    struct VCursor const *curs;
    uint32_t colID;
    uint64_t lastAccessStamp;

    void stamp();

    void openCursor(struct VDatabase const *db);
    ReadResult read(int64_t row) const;

public:
    friend bool operator <(WGS const &a, WGS const &b) {
        return a.lastAccessStamp < b.lastAccessStamp;
    }

    static bool isScheme(std::string const &scheme) {
        return scheme == std::string("NCBI:WGS:db:contig");
    }

    WGS(VPath const *url, struct VDatabase const *db)
    : url(url)
    , curs(NULL)
    {
        stamp();
        openCursor(db);
    }

    ~WGS();

    bool is_open() const { return curs != NULL; }

    void close();
    void reopen(struct VDBManager const *mgr, std::string const &seq_id);

    unsigned getBases(uint8_t *const dst, unsigned const start, unsigned const len, int64_t const row)
    {
        stamp();

        try {
            auto const value = read(row);
            if (start < value.length) {
                auto const n = std::min(len, value.length - start);
                memmove(dst, value.string + start, n);
                return n;
            }
        }
        catch (...) {}
        return 0;
    }
    using SplitName = std::pair<std::string, int64_t>;
    static SplitName splitName(std::string const &name) {
        auto accession = std::string();
        int64_t row = 0;
        unsigned digits = 0;

        for (auto && ch : name) {
            if (ch >= '0' && ch <= '9')
                ++digits;
            else
                digits = 0;
        }
        if (digits > 2) {
            auto const row_len = digits - 2;
            auto const acc_len = name.size() - row_len;
            auto const &row_str = name.substr(acc_len);

            accession = name.substr(0, acc_len);
            for (auto && ch : row_str) {
                assert(ch >= '0' && ch <= '9');
                row = row * 10 + (ch - '0');
            }
        }
        return std::make_pair(accession, row);
    }
};
