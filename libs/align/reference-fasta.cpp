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
#include <array>

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
    cDataBufferBase subBuffer(size_t first, size_t count) {
        KDataBuffer sub;
        throw_if(KDataBufferSub(this, &sub, first, count));
        return sub;
    }
    cDataBufferBase subBuffer(size_t first = 0) {
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
    cDataBuffer subBuffer(size_t offset = 0) const {
        return cDataBufferBase::subBuffer(offset);
    }
    cDataBuffer subBuffer(T const *begp, T const *endp = nullptr) const {
        if (beg < begin() || beg >= end())
            throw std::out_of_range("beg");
        if (endp == nullptr || endp > end())
            endp = end();
        return cDataBufferBase::subBuffer(begin() - begp, endp - begp);
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

template <typename T>
static std::vector< T > split(T const &string, char separator)
{
    std::vector< T > result;
    auto at = std::string::size_type{0};
    do {
        auto const sep = string.find(separator, at);
        if (sep == string.npos) {
            result.emplace_back(string.substr(at));
            break;
        }
        result.emplace_back(string.substr(at, sep - at));
        at = sep + 1;
    } while (at < string.length());
    return result;
}

/// @brief Find `a` in `b`, using case-insensitive comparison
/// @tparam T 
/// @param a the query string
/// @param b the string to search
/// @return the offset of `a` in `b`, or `b.npos`
template <typename T, typename S = typename T::size_type>
static S findCaseInsensitive(T const &a, T const &b) {
    if (b.length() == a.length()) {
        auto i = a.begin();
        auto j = b.begin();
        auto const ii = a.end();
        auto const jj = b.end();

        while (i != ii && j != jj) {
            if (std::toupper(*i++) != std::toupper(*j++))
                return b.npos;
        }
        return 0;
    }
    if (b.length() < a.length())
        return b.npos;
    S const over = b.length() - a.length();
    for (S i = 0; i != over; ++i) {
        if (findCaseInsensitive(a, b.substr(i, a.length())) == 0)
            return i;
    }
    return b.npos;
}

template <typename T1, typename T2, typename S = typename T1::size_type>
static S SeqId_commonPrefixLength(T1 const &seqId, T2 const &qry)
{
    auto const ends = seqId.end();
    auto const endq = qry.end();
    auto s = seqId.begin();
    auto q = qry.begin();
    auto m = &(*s);
    auto const m0 = m;
    auto wild = 0;

    while (s != ends && q != endq) {
        if (*q == '|')
            ++wild;
        else
            wild = 0;
        
        if (wild < 2) {
            if (std::toupper(*s) != std::toupper(*q))
                break;
            if (*s != '.' && *s != '|')
                m = &(*s);
            ++s; ++q;
        }
        else {
            if (*s == '|')
                wild = 0;
            m = &(*s);
            ++s;
        }
    }
    return (S)(m - m0);
}

struct SeqIdNamespace {
    char const *name;
    unsigned minParts;
    unsigned maxParts;

    constexpr SeqIdNamespace(char const *name_, unsigned minParts_, unsigned maxParts_)
    : name(name_)
    , minParts(minParts_)
    , maxParts(maxParts_)
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

    static constexpr unsigned unknownID = 22;
    static constexpr unsigned patentID = 12;
    static constexpr unsigned patPendID = 14; ///< canonically same namespace as patentID
    static constexpr unsigned referenceID = 17;
    static constexpr unsigned otherID = 11; ///< canonically same namespace as referenceID

    static unsigned canonicalID(unsigned id) {
        if (id > unknownID) return unknownID;
        if (id == patentID) return patentID;
        if (id == otherID)  return referenceID;
        return id;
    }

    static SeqIdNamespace const all[unknownID + 1];

    static unsigned find(char const *qry_cs) {
        auto const qry = SeqIdNamespace{qry_cs, 0, 0};
        auto f = 0;
        auto e = unknownID;

        while (f < e) {
            auto const step = e - f;
            if (step < 5) {
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
};

SeqIdNamespace const SeqIdNamespace::all[SeqIdNamespace::unknownID + 1] = {
    { "bbm", 1, 1 }, // 0
    { "bbs", 1, 1 }, // 1
    { "dbj", 1, 2 }, // 2
    { "emb", 1, 2 }, // 3
    { "gb", 1, 2 },  // 4
    { "gi", 1, 1 },  // 5
    { "gim", 1, 1 }, // 6
    { "gnl", 2, 2 }, // 7
    { "gpp", 1, 2 }, // 8
    { "lcl", 1, 1 }, // 9
    { "nat", 1, 2 }, // 10
    { "oth", 1, 3 }, // 11
    { "pat", 3, 3 }, // 12
    { "pdb", 2, 2 }, // 13
    { "pgp", 3, 3 }, // 14
    { "pir", 2, 2 }, // 15
    { "prf", 2, 2 }, // 16
    { "ref", 1, 3 }, // 17
    { "sp", 2, 2 },  // 18
    { "tpd", 1, 2 }, // 19
    { "tpe", 1, 2 }, // 20
    { "tpg", 1, 2 }, // 21
    { "???", 0, 0 }  // 22
};

struct Defline {
    char const *value;

    Defline(char const *p) : value(p) {}
    
    enum Match {
        match_none,
        match_substring,
        match_prefix_part,
        match_prefix,
        match_no_version_part,
        match_no_version,
        match_part,
        match,
    };

    /// @returns first word
    std::string_view seqId() const {
        auto len = size_t{0};
        while (value[len] != '\0' && !isspace(value[len]))
            ++len;
        return std::string_view{value, len};
    }

    template< typename T, typename F >
    void match_1(T const &qry, F && func) const {
        for (auto && part : Parts{ seqId() }) {
            auto && n = part.commonPrefixLength(qry);
            if (n > 0)
                func(n, part);
        }
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
            return id == SeqIdNamespace::unknownID ? -1 : int{id};
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

        template< typename FUNC >
        static void forEach(Super const &sv, FUNC && func)
        {
            auto part = Part{ sv };
            while (part) {
                auto p = part.split();
                func(p.first, bool{ p.second });
                part = p.second;
            }
        }
    };

};

struct FastaFileEntry {
    char const *defline;
    char const *sequence;
    uint8_t md5sum[16];
};

struct FastaFile {
private:
    struct Entry : public FastaFileEntry {
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
                        table[*cp] = table[tolower(*cp)] = *cp;
                    table['X'] = table['x'] = 'N';
                    table['U'] = table['u'] = 'T';
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
                result.emplace_back(FastaFileEntry{const_cast<char const *>(&ch), nullptr});
            
            nl = (ch == '\n' || ch == '\r');
            if (nl) ws = true;
        }
        result.emplace_back(FastaFileEntry{buffer.cend(), nullptr});

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

            Defline::Part::forEach(defline.seqId(), [&](Defline::Part const &part, bool last) {
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
    char const *defline(unsigned index) const {
        return (index < count()) 
            ? entries[index].defline
            : nullptr;
    }
    std::string_view id(unsigned index) const {
        return (index < count())
            ? Defline{entries[index].defline}.seqId()
            : std::string_view{};
    }
    void md5sum(uint8_t *out, unsigned index) const {
        if (index < count())
            std::copy(entries[index].md5sum, entries[index].md5sum + 16, out);
    }
    Buffer sequence(unsigned index) const {
        if (index >= count())
            throw std::out_of_range("index");
        
        auto const endp = std::find(entries[index].sequence, buffer.end(), '\0');
        assert(endp != buffer.end());

        return buffer.subBuffer(entries[index].sequence, endp);
    }
    unsigned bestMatch(std::string_view const &qry_sv) const {
        using IndexEntry = std::pair< int, unsigned >;
        std::vector< IndexEntry > index;

        for (auto qry = Defline::Part(qry_sv); qry; ) {
            auto [part, rest] = qry.split();
            auto nsId = part.seqIdNamespace();
            
            qry = rest;
            if (nsId < 0 || qry.empty()) {
                auto const kw = part.withoutVersion();
                auto const range = keywordIndex.equal_range(kw);
                for (auto i = range.first; i != range.second; ++i) {
                    auto const &entry = entries[i->second];
                    Defline::Part::forEach(Defline{ entry.defline }.seqId(), [&](Defline::Part const &fnd, bool last) {
                        if (fnd == part)
                            index.emplace_back(Defline::match, i->second);
                        else if (fnd.withoutVersion() == kw)
                            index.emplace_back(Defline::match_no_version, i->second);
                    });
                }
            }
            else {
                auto const cId = SeqIdNamespace::canonicalID(nsId);
                auto const nsr = namespaceIndex.equal_range(cId);
                auto const &ns = SeqIdNamespace::all[cId];
                Defline::Part parts[4];
                int n = 0;
                auto rest = qry;

                while (n < ns.minParts && !rest.empty()) {
                    auto const && p = rest.split();
                    parts[n++] = p.first;
                    rest = p.second;
                }
                while (n < ns.maxParts && !rest.empty()) {
                    auto const && p = rest.split();
                    if (p.first.seqIdNamespace() < 0) {
                        parts[n++] = p.first;
                        rest = p.second;
                    }
                    else
                        break;
                }
                if (n >= ns.minParts) {
                    qry = rest;

                    /// will contain the intersection of the keyword index and the namespace index
                    auto candidates = std::set< unsigned >{};
                    for (auto i = 0; i < n; ++i) {
                        if (parts[i].empty()) continue;
                        auto const range = keywordIndex.equal_range(parts[i].withoutVersion());
                        std::set_intersection(range.first, range.second, nsr.first, nsr.second, std::back_inserter(candidates));
                    }

                    for (auto i : candidates) {
                        auto const &entry = entries[i];
                        auto const seqId = Defline{ entry.defline }.seqId();
                        Defline::Part const *p = nullptr;
                        Defline::Part const *const e = parts + n;

                        Defline::Part::forEach(seqId, [&](Defline::Part const &fnd, bool last) {
                            if (p == nullptr) {
                                if (SeqIdNamespace::canonicalID(fnd.seqIdNamespace()) == cId)
                                    p = parts;
                            }
                            else if (p < e) {
                                auto const &part = *p++;
                                if (part.empty() || fnd.empty())
                                    return;
                                if (part == fnd) {
                                    index.emplace_back(Defline::match, i);
                                    return;
                                }
                                if (part.withoutVersion() == fnd.withoutVersion()) {
                                    index.emplace_back(Defline::match_no_version, i);
                                    return;
                                }
                                p = &parts[3];
                            }
                        });
                    }
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

rc_t ImportFastaBuffer(KDataBuffer *out, char **fastaSeqId, uint8_t md5[], KDataBuffer *const buf)
{
    try {
        auto const p_out = static_cast<cDataBuffer<char> *>(out);
        auto const imported = FastaFile(*static_cast<cDataBuffer<char> *>(buf));
        auto const id = imported.id(0);

        *fastaSeqId = reinterpret_cast<char *>(malloc(id.length() + 1));
        if (*fastaSeqId == NULL)
            return RC(rcAlign, rcFile, rcReading, rcMemory, rcExhausted);

        std::copy(id.begin(), id.end(), *fastaSeqId);
        (*fastaSeqId)[id.length()] = '\0';

        imported.md5sum(md5, 0);

        *p_out = imported.sequence(0);

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

rc_t ImportFastaCheckEnv(KDataBuffer *out, char const **fastaSeqId, uint8_t md5[], unsigned length, char const *const qry)
{
    static FastaFile *envFastaFile = nullptr;
    if (envFastaFile == nullptr) {
        try { envFastaFile = ImportFastaCheckEnv_1(); }
        catch (rc_t rc) { return rc; }
    }
    if (envFastaFile) {
        auto const entry = envFastaFile->bestMatch(std::string_view{ qry, length });
        if (entry < envFastaFile->count()) {
            if (out)
                *out = envFastaFile->sequence(entry);
            if (fastaSeqId)
                *fastaSeqId = envFastaFile->defline(entry);
            if (md5)
                envFastaFile->md5sum(md5, entry);
            return 0;
        }
    }
    return RC(rcAlign, rcFile, rcReading, rcData, rcNotFound);
}
