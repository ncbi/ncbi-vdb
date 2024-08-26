/*==============================================================================
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
*/

// #include <sysalloc.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include <memory>
#include <type_traits>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <set>
#include <atomic>

#include <klib/rc.h>
#include <klib/data-buffer.h>   /* KDatabuffer */
#include <klib/checksum.h>      /* MD5 */
#include <kfs/directory.hpp>
#include <kfs/file.hpp>
#include <sra/sradb.h>          /* INSDC_4na_map_CHARSET */

#include "reference-fasta.h"

static inline void throw_if(rc_t rc) { if (rc) throw rc; }

class cDataBufferBase : public KDataBuffer {
    cDataBufferBase(KDataBuffer const &rhs)
    : KDataBuffer(rhs)
    {}
public:
    ~cDataBufferBase() {
        KDataBufferWhack(this);
    }
    explicit cDataBufferBase(size_t elem_bits, size_t count = 0) {
        throw_if(KDataBufferMake(this, elem_bits, count));
    }
    cDataBufferBase subBuffer(size_t first, size_t count) const {
        KDataBuffer sub;
        throw_if(KDataBufferSub(this, &sub, first, count));
        return sub;
    }
    cDataBufferBase subBuffer(size_t first = 0) const {
        KDataBuffer sub;
        throw_if(KDataBufferSub(this, &sub, first, elem_count - first));
        return sub;
    }
    void resize(size_t newCount = 0) {
        throw_if(KDataBufferResize(this, newCount));
    }
    size_t count() const { return elem_count; }
    size_t bits() const { return count() * elem_bits; }
    size_t size() const { return (bits() + 7) / 8; }
};

template <typename T>
class cDataBuffer : public cDataBufferBase 
{
public:
    explicit cDataBuffer(size_t count = 0)
    : cDataBufferBase(sizeof(T) * 8, count)
    {}
    explicit cDataBuffer(cDataBufferBase const &base)
    : cDataBufferBase(base)
    {
        assert(base.elem_bits == sizeof(T) * 8);
    }
    cDataBuffer subBuffer(size_t offset = 0) const {
        return cDataBuffer{ cDataBufferBase::subBuffer(offset) };
    }
    cDataBuffer subBuffer(T const *begp, T const *endp = nullptr) const {
        if (endp == nullptr || endp > end())
            endp = end();
        if (begp < begin() || begp >= end() || begp >= endp)
            throw std::out_of_range("beg");
        size_t const offset = begin() - begp;
        size_t const count = endp - begp;
        return cDataBuffer{ cDataBufferBase::subBuffer(offset, count) };
    }

    T const *cbegin() const { return reinterpret_cast<T const *>(base); }
    T const *begin() const { return cbegin(); }
    T *begin() { return const_cast<T *>(cbegin()); }

    T const *cend() const { return cbegin() + count(); }
    T const *end() const { return cend(); }
    T *end() { return const_cast<T *>(cend()); }

    T &operator [](size_t index) {
        return cbegin()[index];
    }
    T const &operator [](size_t index) const {
        return begin()[index];
    }

    void append(size_t n, T const &value)
    {
        auto const m = count();
        resize(m + n);

        auto p = begin() + m;
        auto const endp = p + n;

        while (p < endp)
            *p++ = value;
    }
    void append(T const &value) {
        auto const m = count();
        resize(m + 1);
        begin()[m] = value;
    }
};

struct SeqIdNamespace {
    char const *name;
    unsigned maxParts;
    unsigned minParts;
    unsigned rank;

    constexpr SeqIdNamespace(char const *name_, unsigned maxParts_, unsigned minParts_, unsigned rank_)
    : name(name_)
    , maxParts(maxParts_)
    , minParts(minParts_)
    , rank(rank_)
    {}

    constexpr SeqIdNamespace(char const *name_, unsigned maxParts_, unsigned minParts_)
    : name(name_)
    , maxParts(maxParts_)
    , minParts(minParts_)
    , rank(60)
    {}

    constexpr SeqIdNamespace(char const *name_)
    : name(name_)
    , maxParts(0)
    , minParts(0)
    , rank(99)
    {}

    int compare(SeqIdNamespace const &rhs) const { return strcmp(name, rhs.name); }

    bool operator ==(SeqIdNamespace const &rhs) const { return compare(rhs) == 0; }
    bool operator !=(SeqIdNamespace const &rhs) const { return compare(rhs) != 0; }
    bool operator  <(SeqIdNamespace const &rhs) const { return compare(rhs) < 0; }
    bool operator  >(SeqIdNamespace const &rhs) const { return compare(rhs) > 0; }

    template< typename T >
    int compare(T const &rhs) const {
        auto cp = name;
        for (auto && rch : rhs) {
            auto && lch = *cp++;
            if (lch == rhs) continue;
            return lch < rch ? -1 : 1;
        }
        return *cp == '\0' ? 0 : -1;
    }

    static constexpr unsigned unknownID = 0;
    static constexpr unsigned patentID = 13;
    static constexpr unsigned patPendID = 15; ///< canonically same namespace as patentID
    static constexpr unsigned referenceID = 18;
    static constexpr unsigned otherID = 12; ///< canonically became referenceID
    static constexpr unsigned tremblID = 23;
    static constexpr unsigned swissprotID = 19;
    static constexpr unsigned lastID = 23;

    static unsigned canonicalID(unsigned id) {
        if (id > unknownID)  return unknownID;
        if (id == patPendID) return patentID;
        if (id == otherID)   return referenceID;
        if (id == tremblID)  return swissprotID;
        return id;
    }

    static SeqIdNamespace const all[lastID + 1];

    static unsigned find(char const *qry_cs) {
        auto const qry = SeqIdNamespace{qry_cs};
        auto f = 1;
        auto e = lastID + 1;

        while (f < e) {
            auto const step = e - f;
            if (step <= 8) {
                if (all[f] == qry)
                    return f;
                ++f;
            }
            else {
                auto const m = f + (step >> 1);
                auto const diff = all[m].compare(qry);
                if (diff < 0)
                    e = m;
                else if (diff > 0)
                    f = m + 1;
                else
                    return m;
            }
        }
        return unknownID;
    }

    bool operator !() const {
        return this < all || this > all + lastID;
    }
    operator bool() const {
        return !(operator !());
    }
    unsigned ID() const {
        return *this ? (this - all) : unknownID;
    }
    unsigned canonicalID() const {
        return SeqIdNamespace::canonicalID(ID());
    }
};

SeqIdNamespace const SeqIdNamespace::all[SeqIdNamespace::lastID + 1] = {
    { "???" },              // 0
    { "bbm", 1, 1, 70 },    // 1
    { "bbs", 1, 1, 70 },    // 2
    { "dbj", 2, 1 },        // 3
    { "emb", 2, 1 },        // 4
    { "gb" , 2, 1 },        // 5
    { "gi" , 1, 1, 51 },    // 6
    { "gim", 1, 1, 70 },    // 7
    { "gnl", 2, 2, 80 },    // 8
    { "gpp", 2, 1, 68 },    // 9
    { "lcl", 1, 1, 80 },    // 10
    { "nat", 2, 1, 69 },    // 11
    { "oth", 3, 1, 66 },    // 12
    { "pat", 3, 3, 67 },    // 13
    { "pdb", 2, 2 },        // 14
    { "pgp", 3, 3, 68 },    // 15
    { "pir", 2, 2 },        // 16
    { "prf", 2, 2 },        // 17
    { "ref", 3, 1, 65 },    // 18
    { "sp" , 2, 2, 60 },    // 19
    { "tpd", 2, 1 },        // 20
    { "tpe", 2, 1 },        // 21
    { "tpg", 2, 1 },        // 22
    { "tr" , 2, 2, 61 },    // 23
};

struct Defline {
    char const *value;

    Defline(char const *p) : value(p) {}
    
    enum Match {
        match_none,
        match_no_version,
        match_part,
        match,
        match_namespace = 8
    };

    /// @returns first word
    std::string_view seqId() const {
        auto len = size_t{0};
        while (value[len] != '\0' && !isspace(value[len]))
            ++len;
        return std::string_view{value, len};
    }

    struct Part 
        : public std::string_view 
    {
        using Super = std::string_view;
        using Pair = std::pair< Part, Part >;

        static constexpr Super::value_type separator = '|';

        Part() {}

        explicit Part(Super const &sv)
        : Super(sv)
        {}

        Pair split() const 
        {
            auto const sep = find(separator);
            if (sep == npos)
                return std::make_pair(*this, Part{});
            else
                return std::make_pair(Part{ substr(0, sep) }, Part{ substr(sep + 1) });
        }

        operator bool() const { return data() != nullptr; }

        int seqIdNamespace() const
        {
            auto const id = length() ? SeqIdNamespace::find(data()) : SeqIdNamespace::unknownID;
            return id == SeqIdNamespace::unknownID ? -1 : (int)id;
        }

        /// @brief Split into parts using the namespace rules.
        /// @param[out] nsId the namespace, or -1 if none.
        /// @param[out] count the number of parts found.
        /// @param[out] parts the parts found.
        /// @return the remainder.
        Part splitParts(int *nsId, unsigned *count, Part parts[4]) const {
            auto p = split();
            auto const realId = p.second ? p.first.seqIdNamespace() : -1;

            nsId[0] = p.second ? realId : -1;
            count[0] = 0;
            if (realId < 0) {
        CAN_NOT_SPLIT:
                nsId[0] = -1;
                parts[0] = p.first;
                count[0] = 1;
            }
            else {
                auto const &ns = SeqIdNamespace::all[realId];
                auto const save = p;

                while (count[0] < ns.minParts && p.second) {
                    p = p.second.split();
                    parts[count[0]++] = p.first;
                }
                while (count[0] < ns.maxParts && p.second) {
                    p = p.second.split();
                    if (p.first.seqIdNamespace() < 0)
                        parts[count[0]++] = p.first;
                    else
                        break;
                }
                if (count[0] < ns.minParts) {
                    p = save;
                    goto CAN_NOT_SPLIT;
                }
            }
            return p.second;
        }

        bool operator ==(Part const &rhs) const
        {
            auto id = seqIdNamespace();
            auto rid = rhs.seqIdNamespace();
            if (id < 0 && rid < 0) {
                auto i = rhs.begin();
                auto const e = rhs.end();
                for (auto && ch : *this) {
                    if (i == e)
                        return false;
                    if (std::toupper(ch) != std::toupper(*i++))
                        return false;
                }
                return i == e;
            }
            return id == rid;
        }

        /// @brief Find the part upto '.', checks that remainder is a version string.
        /// @return the part before the version, or self.
        Part withoutVersion() const
        {
            auto const dot = find('.');
            if (dot == npos) return *this;

            auto vers = false;
            for (auto ch : substr(dot)) {
                if (std::isdigit(ch))
                    vers = true;
                else if (ch == '.')
                    vers = false; ///< needs to be followed by a digit
                else
                    return *this;
            }
            return vers ? Part{ substr(0, dot) } : *this;
        }

        /// @brief How well do two match?
        /// @param rhs the other.
        /// @return 2 if equal, 1 if matches up-to version, else 0.
        int match(Part const &rhs) const
        {
            if (*this == rhs)
                return 2;
            if (this->withoutVersion() == rhs || rhs.withoutVersion() == *this)
                return 1;
            return 0;
        }

        template< typename FUNC >
        static void forEachPart(Super const &sv, FUNC && func)
        {
            auto part = Part{ sv };
            while (part) {
                auto const &p = part.split();
                func(p.first, bool{ p.second });
                part = p.second;
            }
        }

        template< typename FUNC >
        static void forEachParts(Super const &sv, FUNC && func)
        {
            auto part = Part{ sv };

            while (part) {
                int nsId = -1;
                unsigned n = 0;
                Part parts[4];

                part = part.splitParts(&nsId, &n, parts);

                auto const &ns = SeqIdNamespace::all[nsId < 0 ? SeqIdNamespace::unknownID : nsId];
                func(ns, n, parts);
            }
        }
    };

    enum Match matches(Part const &seqId) const
    {
        int best_match = 0;
        int best_rank = 99;

        Part::forEachParts(this->seqId(), [&](SeqIdNamespace const &my_ns, unsigned n, Part const my_parts[]) {
            for (unsigned i = 0; i < n; ++i) {
                auto const this_match = my_parts[i].match(seqId);
                if (best_match < this_match || (best_match == this_match && my_ns.rank < best_match)) {
                    best_match = this_match;
                    best_rank = my_ns.rank;
                }
            }
        });
        return best_match == 0 ? match_none : best_match == 1 ? match_no_version : match;
    }
    enum Match matches(unsigned nsId, unsigned count, Part const parts[4]) const
    {
        if (count < 1) return match_none;
        if (nsId == SeqIdNamespace::unknownID)
            return matches(parts[0]);

        int best_match = 0;

        Part::forEachParts(seqId(), [&](SeqIdNamespace const &my_ns, unsigned n, Part const my_parts[]) {
            if (my_ns.canonicalID() == SeqIdNamespace::canonicalID(nsId)) {
                for (unsigned i = 0; i < n; ++i) {
                    for (unsigned j = 0; j < count; ++j) {
                        auto const this_match = my_parts[i].match(parts[j]);
                        if (best_match < this_match)
                            best_match = this_match;
                    }
                }
            }
        });
        if (best_match)
            return (enum Match)((best_match == 1 ? match_no_version : match) | match_namespace);
        return match_none;
    }
};

struct FastaFileEntry {
    char const *defline;
    char const *sequence;
    uint8_t md5sum[16];
};

struct FastaFile {
private:
    struct Entry : public FastaFileEntry {
        Entry(FastaFileEntry const &base)
        : FastaFileEntry(base)
        {}
        Entry(FastaFileEntry && base)
        : FastaFileEntry(std::move(base))
        {}

        /// @brief `s/^>\s*//; s/\s+/ /;`.
        void cleanupDefline() {
            auto dst = const_cast<char *>(defline);
            auto endp = dst;

            assert(sequence != nullptr);

            // find end of line
            while (endp < sequence) {
                auto const ch = *endp;
                if (ch == '\n' || ch == '\r') {
                    *endp = '\0';
                    break;
                }
                ++endp;
            }
            assert(*endp == '\0');

            auto src = defline;
            assert(*src == '>');
            do { ++src; } while (src < endp && isspace(*src));

            auto ws = false;

            while (src < endp) {
                auto const ch = *src++;
                auto const is_ws = isspace(ch);

                if (!is_ws) {
                    if (ws) *dst++ = ' ';
                    *dst++ = ch;
                }
                ws = is_ws;
            }
            while (dst < endp)
                *dst++ = '\0';
        }

        /// @brief Check and clean up sequence. Also computes MDS sum.
        /// @param endp 
        void cleanupSequence(char const *const endp) {
            struct SequenceCharacterTranslation {
                enum {
                    ignored = -1,
                    disallowed = 0
                };
                int operator[](int ch) const {
                    assert(0 < ch && ch < 256);
                    return (int)table[ch];
                }
                SequenceCharacterTranslation() {
                    for (int i = 0; i < 256; ++i)
                        table[i] = isspace(i) ? ignored : disallowed;
                    for (auto cp = INSDC_4na_map_CHARSET; *cp; ++cp)
                        table[(int)*cp] = table[tolower(*cp)] = *cp;
                    table[(int)'X'] = table[(int)'x'] = 'N';
                    table[(int)'U'] = table[(int)'u'] = 'T';
                }
            private:
                int8_t table[256];
            };
            static SequenceCharacterTranslation tr;
            auto dst = const_cast<char *>(sequence);
            auto src = dst;
            MD5State mds;

            MD5StateInit(&mds);
            while (src < endp) {
                auto const ch = tr[*src++];
                if (ch == (int)SequenceCharacterTranslation::ignored)
                    ;
                else if (ch == (int)SequenceCharacterTranslation::disallowed)
                    throw RC(rcAlign, rcFile, rcReading, rcData, rcInvalid);
                else {
                    *dst = (char)ch;
                    MD5StateAppend(&mds, dst, 1);
                    ++dst;
                }
            }
            MD5StateFinish(&mds, md5sum);

            while (dst < endp)
                *dst++ = '\0';
        }
        void cleanup(char const *const endp) {
            cleanupDefline();
            cleanupSequence(endp);
        }

        cDataBuffer< char > copySequence(cDataBuffer< char > const &source) const
        {
            auto const endp = std::find(sequence, source.end(), '\0');
            assert(endp != source.end());

            return source.subBuffer(sequence, endp);
        }
        char *idCopy() const
        {
            auto const id = Defline{ defline }.seqId();
            auto result = reinterpret_cast<char *>(malloc(id.size() + 1));
            if (result) {
                std::copy(id.begin(), id.end(), result);
                result[id.length()] = '\0';
                return result;
            }
            throw std::bad_alloc();
        }
        void md5copy(uint8_t *out) const {
            std::copy(md5sum, md5sum + 16, out);
        }
    };

    using Buffer = cDataBuffer< char >;
    Buffer buffer;

    using Entries = std::vector< Entry >;
    Entries entries;

    using NamespaceIndex = std::multimap< unsigned, unsigned >;
    NamespaceIndex namespaceIndex;

    using KeywordIndex = std::multimap< Defline::Part, unsigned >;
    KeywordIndex keywordIndex;

    static void readFile(Buffer &buffer, KFile const &f, uint64_t max = 0) {
        for (uint64_t pos = 0; max == 0 || pos < max; ) {
            auto const at = buffer.begin() + pos;
            if (at == buffer.end()) {
                buffer.resize(pos + pos);
                continue;
            }
            assert(at < buffer.end());

            auto const remain1 = (uint64_t)(buffer.end() - at);
            auto const remain2 = max == 0 ? remain1 : (max - pos);
            auto const remain = remain1 < remain2 ? remain1 : remain2;
            auto nread = size_t{0};

            throw_if(f.Read(pos, at, remain, &nread));
            pos += nread;

            buffer.resize(pos);
            if (nread == 0)
                break;
        }
    }

    /// @brief find lines matching /^\s*>/
    /// @param buffer 
    /// @return vector of found deflines
    static Entries findDeflines(Buffer const &buffer) 
    {
        Entries result;

        bool nl = true;
        bool ws = true;

        for (auto && ch : buffer) {
            if (ws && isspace(ch))
                continue;
            
            ws = false;

            if (nl && ch == '>')
                result.emplace_back(FastaFileEntry{ &ch });
            
            nl = (ch == '\n' || ch == '\r');
            if (nl) ws = true;
        }
        result.emplace_back(FastaFileEntry{ buffer.cend() });

        return result;
    }

    static void findSeqLines(Entries &entries) 
    {
        char *defline = nullptr;

        for (auto && entry : entries) {
            if (defline) {
                auto cur = defline;
                int st = 0;

                while (cur < entry.defline && entry.sequence == nullptr) {
                    auto const ch = *cur++;
                    switch (st) {
                    case 0:
                        if (ch == '\n' || ch == '\r')
                            st = 1;
                        break;
                    case 1:
                        if (isspace(ch))
                            break;
                        entry.sequence = cur;
                    }
                }
            }
            defline = const_cast<char *>(entry.defline);
        }
    }

    static void cleanupLines(Entries &entries) 
    {
        Entry *prev{nullptr};

        for (auto && entry : entries) {
            if (prev && prev->sequence)
                prev->cleanup(entry.defline);
            prev = &entry;
        }
    }

    static Entries parseBuffer(Buffer &buffer)
    {
        auto result = findDeflines(buffer);
        findSeqLines(result);
        cleanupLines(result);
        result.erase(std::remove_if(result.begin(), result.end(), [](Entry const &e) { return e.sequence == nullptr; }));

        return result;
    }

    void populateIndices() {
        for (auto && entry : entries) {
            auto const i = (unsigned)(&entry - &entries[0]);
            auto defline = Defline{ entry.defline };
            std::set< unsigned > u_ns;
            std::set< Defline::Part > u_kw;

            Defline::Part::forEachPart(defline.seqId(), [&](Defline::Part const &part, bool last) {
                auto ns = part.seqIdNamespace();
                if (ns < 0 || last)
                    u_kw.insert(part.withoutVersion());
                else
                    u_ns.insert(SeqIdNamespace::canonicalID(ns));
            });

            for (auto ns : u_ns)
                namespaceIndex.emplace(ns, i);

            for (auto kw : u_kw)
                keywordIndex.emplace(kw, i);
        }
    }

public:
    /// @brief load from file of known size.
    /// @param f the file handle.
    FastaFile(KFile const &f, uint64_t fsize)
    : buffer(fsize)
    {
        readFile(buffer, f, fsize);
        entries = parseBuffer(buffer);
        populateIndices();
    }
    /// @brief load from file of unknown size, like stdin.
    /// @param f the file handle.
    explicit FastaFile(KFile const &f)
    : buffer(65536)
    {
        readFile(buffer, f);
        entries = parseBuffer(buffer);
        populateIndices();
    }
    explicit FastaFile(cDataBuffer<char> &buf)
    : buffer(buf.subBuffer())
    , entries(parseBuffer(buffer))
    {
        populateIndices();
    }

    unsigned count() const { return entries.size(); }
    void copyInfo(unsigned index, KDataBuffer *out, char **fastaSeqId, uint8_t md5[]) const
    {
        if (index >= count())
            throw std::out_of_range("index");

        auto const &entry = entries[index];
        if (out)
            *out = entry.copySequence(buffer);
        if (fastaSeqId)
            *fastaSeqId = entry.idCopy();
        if (md5)
            entry.md5copy(md5);
    }
    unsigned bestMatch(std::string_view const &qry_sv) const {
        using IndexEntry = std::pair< int, unsigned >;
        std::vector< IndexEntry > index;
        std::set< unsigned > candidate;
        auto ignoreNs = false;

        /// Matching proceedes in three phases. 
        /// The first phase populates a list of candidate entries
        /// by querying the indices for potential matching entries.
        /// The second phase visits each candidate and assignes a 
        /// score to each matching entry.
        /// The final phase sorts and picks the best entry.

        // Filter requiring search terms to match within namespaces.
        for (auto qry = Defline::Part{ qry_sv }; qry; ) {
            unsigned nq = 0;
            int ns = -1;
            Defline::Part qparts[4];
            std::set< unsigned > nsSet;

            qry = qry.splitParts(&ns, &nq, qparts);

            if (ns < 0 || ns == SeqIdNamespace::unknownID) 
                ;
            else {
                auto const range = namespaceIndex.equal_range(SeqIdNamespace::canonicalID(ns));
                for (auto j = range.first; j != range.second; ++j)
                    nsSet.emplace(j->second);
                if (nsSet.empty())
                    continue;
            }

            for (auto i = 0; i < nq; ++i) {
                if (qparts[i].empty()) continue;
                auto const kw = keywordIndex.equal_range(qparts[i].withoutVersion());
                for (auto j = kw.first; j != kw.second; ++j) {
                    if (nsSet.empty() || nsSet.find(j->second) != nsSet.end())
                        candidate.emplace(j->second);
                }
            }
        }
        if (candidate.empty()) {
            // no candidates yet, relax the namespace requirement and match on keyworks only.
            // This still parses the deflines using the namespace rules.
            ignoreNs = true;
            for (auto qry = Defline::Part{ qry_sv }; qry; ) {
                unsigned nq = 0;
                int ns = -1;
                Defline::Part qparts[4];

                qry = qry.splitParts(&ns, &nq, qparts);

                for (auto i = 0; i < nq; ++i) {
                    if (qparts[i].empty()) continue;
                    auto const kw = keywordIndex.equal_range(qparts[i].withoutVersion());
                    for (auto j = kw.first; j != kw.second; ++j)
                        candidate.emplace(j->second);
                }
            }
        }
        if (candidate.empty()) {
            // no candidates yet, relax parsing rules and match on keyworks only.
            // this is the logic in writer-reference.c
            // of matching on anything that isn't a namespace.
            ignoreNs = true;
            Defline::Part::forEachPart(qry_sv, [&](Defline::Part const &part, bool last){
                if (part.empty())
                    return;
                auto const ns = part.seqIdNamespace();
                if (ns < 0 || ns == SeqIdNamespace::unknownID) {
                    auto const kw = keywordIndex.equal_range(part.withoutVersion());
                    for (auto j = kw.first; j != kw.second; ++j)
                        candidate.emplace(j->second);
                }
            });
        }

        /// Assign a score to each candidate entry.
        for (auto qry = Defline::Part{ qry_sv }; qry; ) {
            unsigned nq = 0;
            int ns = -1;
            Defline::Part qparts[4];

            qry = qry.splitParts(&ns, &nq, qparts);

            for (auto i : candidate) {
                auto const &entry = entries[i];
                auto const defline = Defline{ entry.defline };
                if (ignoreNs) {
                    for (unsigned j = 0; j < nq; ++j)
                        index.emplace_back(std::make_pair(defline.matches(qparts[j]), i));
                }
                else {
                    auto const score = defline.matches(ns, nq, qparts);

                    index.emplace_back(std::make_pair((int)score, i));
                }
            }
        }

        if (index.empty())
            return count();

        if (index.size() > 1) {
            ///< sort by score descending
            std::sort(index.begin(), index.end(), [](IndexEntry const &a, IndexEntry const &b) {
                return b.first < a.first;
            });

            ///< remove all but the best scored
            auto best = index.front().first;
            index.erase(std::remove_if(index.begin(), index.end(), [best](IndexEntry const &a) {
                return a.first < best;
            }));
            assert(!index.empty());

            ///< sort by position
            std::sort(index.begin(), index.end(), [](IndexEntry const &a, IndexEntry const &b) {
                return a.second < b.second;
            });

            ///< update the score to be the hit count
            auto i = index.begin();
            while (i != index.end()) {
                auto const j = i++;
                j->first = 1;
                while (i != index.end() && j->second == i->second) {
                    ++j->first;
                    i->first = 0;
                    ++i;
                }
            }

            ///< sort by score descending
            std::sort(index.begin(), index.end(), [](IndexEntry const &a, IndexEntry const &b) {
                return b.first < a.first;
            });

            ///< remove all but the best scored
            best = index.front().first;
            index.erase(std::remove_if(index.begin(), index.end(), [best](IndexEntry const &a) {
                return a.first < best;
            }));
            assert(!index.empty());

            ///< sort by position
            std::sort(index.begin(), index.end(), [](IndexEntry const &a, IndexEntry const &b) {
                return a.second < b.second;
            });
        }
        return index.front().second;
    }

    static char const *pathFromEnvironment() {
#if WINDOWS
        return nullptr;
#else
        return getenv("NCBI_VDB_REF_FASTA");
#endif
    }
};

static KDirectory *nativeDir() {
    KDirectory *ndir{};
    throw_if( KDirectoryNativeDir(&ndir) );
    return ndir;
}

static FastaFile *ImportFastaCheckEnv_1()
{
    FastaFile *temp = nullptr;
    char const *const path = FastaFile::pathFromEnvironment();
    if (path) {
        KFile const *fp{};
        KDirectory const *ndir{nativeDir()};        
        auto const rc = ndir->OpenFileRead(&fp, "%s", path);
        ndir->Release();

        if (rc == 0) {
            try { 
                temp = new FastaFile(*fp);
                fp->Release();
            }
            catch (...) {
                fp->Release();
                throw;
            }
        }
    }
    return temp;
}

rc_t ImportFastaBuffer(KDataBuffer *out, char **fastaSeqId, uint8_t md5[], KDataBuffer *const buf)
{
    try {
        auto && imported = FastaFile{ *static_cast< cDataBuffer<char> * >(buf) };
        imported.copyInfo(0, out, fastaSeqId, md5);
        return 0;
    }
    catch (rc_t rc) {
        return rc;
    }
    catch (std::bad_alloc const &e) {
        return RC(rcAlign, rcFile, rcReading, rcMemory, rcExhausted);
        ((void)(e));
    }
    catch (...) {
        return RC(rcAlign, rcFile, rcReading, rcNoTarg, rcUnexpected);
    }
}

rc_t ImportFastaCheckEnv(KDataBuffer *out, char **fastaSeqId, uint8_t md5[], unsigned length, char const *const qry)
{
    static std::atomic< FastaFile * >envFastaFile = {nullptr};
    static std::atomic_bool loadedOrFailed = {false};

    try {
        if (envFastaFile) {
            auto const &fastaFile = *envFastaFile;
            auto const entry = fastaFile.bestMatch(std::string_view{ qry, length });
            if (entry < fastaFile.count()) {
                fastaFile.copyInfo(entry, out, fastaSeqId, md5);
                return 0;
            }
            return RC(rcAlign, rcFile, rcReading, rcData, rcNotFound);
        }
        else if (!loadedOrFailed) {
            auto temp = ImportFastaCheckEnv_1();
            auto dummyNull = decltype(temp){ nullptr };
            if (envFastaFile.compare_exchange_weak(dummyNull, temp))
                ;
            else if (temp)
                delete temp;
            loadedOrFailed.store(true);
            return ImportFastaCheckEnv(out, fastaSeqId, md5, length, qry);
        }
        return SILENT_RC(rcAlign, rcFile, rcReading, rcData, rcNotFound);
    }
    catch (rc_t rc) {
        return rc;
    }
    catch (std::bad_alloc const &e) {
        return RC(rcAlign, rcFile, rcReading, rcMemory, rcExhausted);
        ((void)(e));
    }
    catch (...) {
        return RC(rcAlign, rcFile, rcReading, rcNoTarg, rcUnexpected);
    }
}
