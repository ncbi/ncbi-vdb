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

#pragma once

#include <sysalloc.h>
#include <kdb/manager.h>
#include <kdb/database.h>
#include <kdb/index.h>
#include <kdb/table.h>
#include <kdb/column.h>
#include <kdb/meta.h>
#include <kdb/kdb-priv.h>
#include <kdb/namelist.h>

#include <memory>
#include <string>

#ifndef REFCOUNTING_MESSAGES
#ifndef NDEBUG
#define REFCOUNTING_MESSAGES (1)
#endif
#endif

/// @brief Shim for refcounting; See `RETAIN_RELEASE` convenience macro
/// @tparam KOBJ klib object type
template <class KOBJ>
struct RetainRelease {
    using ConstPointer = KOBJ const *;
    using Pointer = KOBJ *;
    void retain(ConstPointer) const { static_assert(false, "purely devirtual!"); };
    void release(ConstPointer) const noexcept { static_assert(false, "purely devirtual!"); };
};
#define RETAIN_RELEASE(KTYPE) template <> struct RetainRelease<KTYPE> {         \
    using ConstPointer = KTYPE const *;                                         \
    using Pointer = KTYPE *;                                                    \
    void retain(ConstPointer const p) const { KTYPE ## AddRef(p); }             \
    void release(ConstPointer const p) const noexcept { KTYPE ## Release(p); }  \
}

namespace Klib {

    /// @brief A `Klib::Object` class can declare that it is a `Consumer` of some other `Klib::Object` class to get raw acccess to its inner pointer.
    /// @tparam OBJ klib object type
    template < class OBJ >
    struct Consumer {
    protected:
        using Pointer = typename OBJ::Pointer;
        using ConstPointer = typename OBJ::ConstPointer;
        static Pointer getPointer(OBJ &obj) {
            return obj.get();
        }
        static ConstPointer getPointer(OBJ const &obj) {
            return obj.get();
        }
    };

    /// @brief a smart pointer for klib objects
    /// @tparam KOBJ klib object type
    /// @tparam RR the refcounting shim for the klib object
    template < class KOBJ, typename RR = RetainRelease<KOBJ> >
    class Object {
    public:
        using ConstPointer = typename RR::ConstPointer;
        using Pointer = typename RR::Pointer;
    private:
        /// @brief the smart pointer implementation
        struct P : private RR {
            Pointer p;

            P(Pointer q) : p(q) {}

            Pointer retain() const {
#if REFCOUNTING_MESSAGES
                std::cerr << ((void const *)this) << " retaining " << ((void const *)p) << std::endl;
#endif
                RR::retain(p);
                return p;
            }
            void release() const noexcept {
#if REFCOUNTING_MESSAGES
                std::cerr << ((void const *)this) << " releasing " << ((void const *)p) << std::endl;
#endif
                RR::release(p);
            }
            Pointer give() {
#if REFCOUNTING_MESSAGES
                std::cerr << ((void const *)this) << " giving away " << ((void const *)p) << std::endl;
#endif
                auto const q = p;
                p = nullptr;
                return q;
            }
            void take(Pointer q) {
                release();
#if REFCOUNTING_MESSAGES
                std::cerr << ((void const *)this) << " replaced " << ((void const *)p) << " with " << ((void const *)q) << std::endl;
#endif
                p = q;
            }
        } p;
        
        /// @brief shares ownership.
        /// @return retained
        Pointer share() const { return p.retain(); }

        /// @brief loses ownership.
        /// @return the raw pointer, unretained.
        Pointer give() { return p.give(); }
    protected:

        /// @brief shares ownership.
        /// @return the raw pointer, unretained.
        Pointer get() const { return p.p; }
    public:

        /// @brief assumes ownership.
        Object(Pointer q) : p(q) {}

        /// @brief assumes ownership, casts off const-ness.
        Object(ConstPointer q) : p(const_cast<Pointer>(q)) {}

        using KType = KOBJ;
        using Self = Object;

        ~Object() {
            static_assert(sizeof(*this) == sizeof(void *), "I AM NOT A POINTER!?");
            p.release();
        }

        /// @brief shares ownership.
        Object(Object const &rhs)
        : p(rhs.share())
        {}

        /// @brief assumes ownership.
        /// @param rhs loses ownership.
        Object(Object &&rhs)
        : p(rhs.give())
        {}

        /// @brief shares ownership.
        Object const &operator =(Object const &rhs) {
            p.take(rhs.share());
        }

        /// @brief is inner pointer the same?
        /// @tparam U a klib object type.
        /// @param other KLib::Object.
        /// @return true if inner pointers of the same.
        template <class U>
        bool isSame(Object<U> const &other) const {
            return reinterpret_cast<void const *>(p.p) == reinterpret_cast<void const *>(other.p.p);
        }

        // A Consumer of Self is a friend of self.
        friend Consumer<Object>;
        using Consumer = Consumer<Object>;
    };
}

#define KlibObject(KTYPE, TYPE) class TYPE : public Klib::Object<KTYPE>

#include <klib/rc.h>

namespace Klib {
    /// @brief A simple value type for result codes that are returned by klib functions.
    struct ResultCode {
    private:
        /// @brief Used for conveying the exitcode of a child process that exited normally.
        struct ExitCodeRC {
            unsigned module: 5
                    , target: 6
                    , context: 7
                    , padding: 6
                    , exitCode: 8;

            explicit ExitCodeRC(uint8_t exitCode)
            : module(rcExitCode)
            , target(rcProcess)
            , context(rcClosing)
            , padding(0)
            , exitCode(exitCode)
            {}
        };
        struct NormalRC {
            unsigned module: 5
                    , target: 6
                    , context: 7
                    , object: 8
                    , state: 6;

            explicit NormalRC(rc_t rc)
            : module(GetRCModule(rc))
            , target(GetRCTarget(rc))
            , context(GetRCContext(rc))
            , object(GetRCObject(rc))
            , state(GetRCState(rc))
            {}
        };
        union {
            NormalRC rc;
            ExitCodeRC exitCode;
            uint32_t raw;
        } value;
    public:
        bool is_exit_code_rc() const {
            return value.rc.module == rcExitCode;
        }
        bool isState(enum RCState st) const {
            return is_exit_code_rc() ? false : ((unsigned)st == value.rc.state);
        }
        explicit ResultCode(rc_t rc)
        : value({.rc = NormalRC(rc)})
        {
        }
        operator rc_t() const {
            return SILENT_RC(  value.rc.module
                             , value.rc.target
                             , value.rc.context
                             , value.rc.object
                             , value.rc.state);
        }
        operator bool() const { return 0 == value.raw; }
        operator std::string() const {
            auto result = std::string();

            if (is_exit_code_rc()) {
                result = "exit code: " + std::to_string(value.exitCode.exitCode);
            }
            else {
                RC_String rcString[5];
                Get_RC_Strings(*this, rcString);

                result = "result code: {";
                result += "module: \"" + std::string(rcString[rcf_module].text, rcString[rcf_module].size);
                result += "\", target: \"" + std::string(rcString[rcf_target].text, rcString[rcf_target].size);
                result += "\", context: \"" + std::string(rcString[rcf_context].text, rcString[rcf_context].size);
                result += "\", object: \"" + std::string(rcString[rcf_object].text, rcString[rcf_object].size);
                result += "\", state: \"" + std::string(rcString[rcf_state].text, rcString[rcf_state].size);
                result += "\"}";
            }
            return result;
        }
    };
}

#include <stdexcept>

namespace Klib {

    /// @brief Exception class for RCs
    class ResultCodeException
    : public std::runtime_error
    {
        static void csv(std::string &accum, RC_String const string[5]) {
            accum += std::string(string[0].text, string[0].size);
            for (auto i = 1; i < 5; ++i) {
                accum += ',';
                accum += std::string(string[i].text, string[i].size);
            }
        }
        static std::string makeWhat(rc_t const rc) {
            std::string result;
            RC_String rcString[5];

            Get_RC_Strings(rc, rcString);
            result = "RC(";
#if _DEBUGGING
            result += GetRCFilename(); result += ':';
            result += std::to_string(GetRCLineno()); result += ':';
            result += GetRCFunction(); result += ' ';
#endif
            csv(result, rcString);
            result += ")";
            return result;
        }
        static std::string makeWhat(rc_t const rc, std::string const &from) {
            std::string result;
            RC_String rcString[5];

            Get_RC_Strings(rc, rcString);
            result = from;
            result += " returned RC(";
            csv(result, rcString);
            result += ")";
#if _DEBUGGING
            result += " at ";
            result += GetRCFilename(); result += ':';
            result += std::to_string(GetRCLineno()); result += ':';
            result += GetRCFunction();
#endif
            return result;
        }
    public:
        rc_t value;

        explicit ResultCodeException(rc_t rc)
        : std::runtime_error(makeWhat(rc))
        , value(rc)
        {}
        ResultCodeException(rc_t rc, std::string const &from)
        : std::runtime_error(makeWhat(rc, from))
        , value(rc)
        {}

        static void throw_if(rc_t const &rc) {
            if (rc != 0)
                throw ResultCodeException(rc);
        }
        static void throw_if(rc_t const &rc, std::string const &from) {
            if (rc != 0)
                throw ResultCodeException(rc, from);
        }
    };
}

#include <klib/namelist.h>

RETAIN_RELEASE(KNamelist);

namespace Klib {
    KlibObject(KNamelist, Namelist)
    {
        char const *at(unsigned const i) const {
            char const *value = nullptr;
            auto const rc = KNamelistGet(get(), i, &value);
            ResultCodeException::throw_if(rc, "KNamelistGet");
            return value;
        }
        using Base = Object<KNamelist>;
        using Pointer = Base::Pointer;
        using ConstPointer = Base::ConstPointer;

    public:
        Namelist(Pointer p) : Base(p) {}
        Namelist(ConstPointer p) : Base(p) {}

        unsigned count() const {
            uint32_t n = 0;
            auto const rc = KNamelistCount(get(), &n);
            ResultCodeException::throw_if(rc, "KNamelistCount");
            return (unsigned)n;
        }
        char const *operator [](unsigned i) const {
            return at(i);
        }
        template <typename F>
        void foreach(F && f) const {
            auto const n = count();
            for (auto i = decltype(n)(0); i < n; ++i) {
                auto const value = at(i);
                f(value);
            }
        }
    };
}

RETAIN_RELEASE(KDirectory);

namespace KFS {
    KlibObject(KDirectory, Directory)
    {
    protected:
        using Base = Object<KDirectory>;
        using Pointer = Base::Pointer;
        using ConstPointer = Base::ConstPointer;

        static Pointer current() {
            Pointer p = nullptr;
            Klib::ResultCodeException::throw_if(KDirectoryNativeDir(&p), "KDirectoryNativeDir");
            return p;
        }
        static ConstPointer open_(std::string const &path) {
            ConstPointer p = nullptr;
            Klib::ResultCodeException::throw_if(KDirectoryOpenDirRead(current(), &p, false, "%s", path.c_str()));
            return p;
        }

    public:
        Directory(Pointer p) : Base(p) {}

        Directory()
        : Base(current())
        {}

        Directory(std::string const &path) : Base(open_(path)) {}

        Directory root() const {
            ConstPointer p = nullptr;
            Klib::ResultCodeException::throw_if(KDirectoryOpenDirRead(get(), &p, true, "/"), "KDirectoryOpenDirRead");
            return const_cast<Pointer>(p);
        }

        Directory open(std::string const &path) const {
            ConstPointer p = nullptr;
            Klib::ResultCodeException::throw_if(KDirectoryOpenDirRead(get(), &p, false, "%s", path.c_str()), "KDirectoryOpenDirRead");
            return const_cast<Pointer>(p);
        }
    };

    class MutatingDirectory : public Directory {
        MutatingDirectory(Pointer p) : Directory(p) {}

        static Pointer open_(std::string const &path) {
            Pointer p = nullptr;
            Klib::ResultCodeException::throw_if(KDirectoryOpenDirUpdate(current(), &p, false, "%s", path.c_str()));
            return p;
        }
    public:
        MutatingDirectory() {}

        MutatingDirectory(std::string const &path) : Directory(open_(path)) {}

        MutatingDirectory root() {
            Pointer p = nullptr;
            Klib::ResultCodeException::throw_if(KDirectoryOpenDirUpdate(get(), &p, true, "/"), "KDirectoryOpenDirUpdate");
            return p;
        }

        MutatingDirectory open(std::string const &path) {
            Pointer p = nullptr;
            Klib::ResultCodeException::throw_if(KDirectoryOpenDirUpdate(get(), &p, false, "%s", path.c_str()), "KDirectoryOpenDirUpdate");
            return p;
        }

        void remove(std::string const &name, bool force = false) {
            Klib::ResultCodeException::throw_if(KDirectoryRemove(get(), force, "%s", name.c_str()), "KDirectoryRemove");
        }
    };
}

using auto_c_string = std::unique_ptr<char, decltype(&free)>;
#define AUTO_C_STRING(PTR) auto_c_string(PTR, free)

RETAIN_RELEASE(KDBManager);
RETAIN_RELEASE(KTable);
RETAIN_RELEASE(KColumn);
RETAIN_RELEASE(KMetadata);
RETAIN_RELEASE(KMDataNode);

using RC_Exception = Klib::ResultCodeException;
#define THROW_IF(FUNC, ARGS) Klib::ResultCodeException::throw_if(FUNC ARGS, # FUNC)

namespace KDB {
    KlibObject(KMDataNode, Metadata)
    {
    public:
        using Base = Klib::Object<KMDataNode>;

        // Allow direct conversion from KMDataNode pointers
        Metadata(ConstPointer p) : Base(p) {}

        struct RawValue {
            void const *value;
            size_t size;

            bool operator ==(RawValue const &other) const {
                return size == other.size && memcmp(value, other.value, size) == 0;
            }
            char const *begin() const { return (char const *)value; }
            char const *end() const { return begin() + size; }
        private:
            friend class Metadata;
            RawValue(void const *p, size_t const size)
            : value(p)
            , size(size)
            {}
            explicit RawValue(char const *str)
            : value(str)
            , size(strlen(str))
            {}
            explicit RawValue(std::string const &str)
            : value(str.data())
            , size(str.size())
            {}
        };

    protected:
        Metadata(Pointer p) : Base(p) {}

        auto_c_string attrValue(char const *name) const {
            size_t size = 0;

            KMDataNodeReadAttr(get(), name, nullptr, 0, &size);
            auto result = (char *)malloc(size + 1);
            RC_Exception::throw_if(KMDataNodeReadAttr(get(), name, result, size + 1, &size), "KMDataNodeReadAttr");
            return AUTO_C_STRING(result);
        }
        Metadata child(char const *name) const {
            ConstPointer p = nullptr;
            RC_Exception::throw_if(KMDataNodeOpenNodeRead(get(), &p, "%s", name), "KMDataNodeOpenNodeRead");
            return p;
        }
    public:
        using Namelist = Klib::Namelist;

        Namelist attributes() const {
            KNamelist *p = nullptr;
            RC_Exception::throw_if(KMDataNodeListAttr(get(), &p));
            return Namelist(p);
        }

        Namelist children() const {
            KNamelist *p = nullptr;
            RC_Exception::throw_if(KMDataNodeListChildren(get(), &p));
            return Namelist(p);
        }

        bool operator ==(Metadata const &other) const {
            auto eq = true;

            try {
                if (!(nodeValue() == other.nodeValue()))
                    eq = false;
                if (eq) {
                    auto const &names = attributes();
                    auto const &others = other.attributes();

                    eq &= (names.count() == others.count());
                    names.foreach([&](char const *name) {
                        eq &= (strcmp(attrValue(name).get(), other.attrValue(name).get()) == 0);
                    });
                }
                if (eq) {
                    auto const &names = children();
                    auto const &others = other.children();

                    eq &= (names.count() == others.count());
                    names.foreach([&](char const *name) {
                        eq &= (child(name) == other.child(name));
                    });
                }
            }
            catch (...) { eq = false; }
            return eq;
        }
        Metadata operator [](std::string const &name) const {
            return child(name.c_str());
        }
        std::string attribute(std::string const &name) const {
            return attrValue(name.c_str()).get();
        }
        RawValue nodeValue() const {
            void const *addr = nullptr;
            size_t size = 0;

            THROW_IF(KMDataNodeAddr, (get(), &addr, &size));
            return {addr, size};
        }
        std::string value() const {
            auto const &nv = nodeValue();
            return std::string(nv.begin(), nv.end());
        }
        template <typename T>
        T value() const {
            T x;

            switch (sizeof(T)) {
            case 1:
                THROW_IF(KMDataNodeReadB8, (get(), &x));
                return x;
            }
            throw std::logic_error("invalid size for value type");
        }
        template <> int16_t value<int16_t>() const {
            int16_t x;
            THROW_IF(KMDataNodeReadAsI16, (get(), &x));
            return x;
        }
        template <> uint16_t value<uint16_t>() const {
            uint16_t x;
            THROW_IF(KMDataNodeReadAsU16, (get(), &x));
            return x;
        }
        template <> int32_t value<int32_t>() const {
            int32_t x;
            THROW_IF(KMDataNodeReadAsI32, (get(), &x));
            return x;
        }
        template <> uint32_t value<uint32_t>() const {
            uint32_t x;
            THROW_IF(KMDataNodeReadAsU32, (get(), &x));
            return x;
        }
        template <> int64_t value<int64_t>() const {
            int64_t x;
            THROW_IF(KMDataNodeReadAsI64, (get(), &x));
            return x;
        }
        template <> uint64_t value<uint64_t>() const {
            uint64_t x;
            THROW_IF(KMDataNodeReadAsU64, (get(), &x));
            return x;
        }
    };

    class MutatingMetadata final
    : public Metadata
    , Metadata::Consumer
    {
        MutatingMetadata child(char const *name) {
            Pointer p = nullptr;
            THROW_IF(KMDataNodeOpenNodeUpdate, (get(), &p, "%s", name));
            return p;
        }
    public:
        Metadata readOnly() {
            return *this;
        }

        MutatingMetadata(Pointer p) : Metadata(p) {};

        MutatingMetadata operator [](std::string const &name) {
            return child(name.c_str());
        }
        void setAttribute(std::string const &name, std::string const &value) {
            THROW_IF(KMDataNodeWriteAttr, (get(), name.c_str(), value.c_str()));
        }
        void setValue(size_t const size, void const *const value) {
            THROW_IF(KMDataNodeWrite, (get(), value, size));
        }
        void setValue(std::string const &value) {
            setValue(value.size(), value.data());
        }
        void copy(Metadata const &other) {
            assert(!isSame(other));
            THROW_IF(KMDataNodeCopy, (get(), getPointer(other)));
        }
    };

    KlibObject(KMetadata, MetadataCollection)
    {
    protected:
        MetadataCollection(Pointer p) : Base(p) {}
    public:
        using Base = Klib::Object<KMetadata>;
        MetadataCollection(ConstPointer p) : Base(p) {}

        Metadata root() const {
            Metadata::ConstPointer p = nullptr;
            THROW_IF(KMetadataOpenNodeRead, (get(), &p, ""));
            return p;
        }
        Metadata operator [](std::string const &name) const {
            Metadata::ConstPointer p = nullptr;
            THROW_IF(KMetadataOpenNodeRead, (get(), &p, "%s", name.c_str()));
            return p;
        }
    };

    class MutatingMetadataCollection final
    : public MetadataCollection
    {
    public:
        MetadataCollection readOnly() {
            return *this;
        }

        MutatingMetadataCollection(Pointer p) : MetadataCollection(p) {}

        MutatingMetadata root() {
            Metadata::Pointer p = nullptr;
            THROW_IF(KMetadataOpenNodeUpdate, (get(), &p, ""));
            return p;
        }
        MutatingMetadata operator [](std::string const &name) {
            Metadata::Pointer p = nullptr;
            THROW_IF(KMetadataOpenNodeUpdate, (get(), &p, "%s", name.c_str()));
            return p;
        }
    };

    KlibObject(KColumn, Column)
    {
    public:
        using Base = Klib::Object<KColumn>;
        Column(Pointer p) : Base(p) {}
        Column(ConstPointer p) : Base(p) {}

        MetadataCollection metadata() const {
            MetadataCollection::Base::ConstPointer p = nullptr;
            THROW_IF(KColumnOpenMetadataRead, (get(), &p));
            return MetadataCollection(p);
        }
        Metadata operator [](std::string const &name) const {
            return metadata()[name];
        }
    };

    class MutatingColumn final
    : public Column
    {
    public:
        Column readOnly() {
            return *this;
        }

        MutatingColumn(Pointer p) : Column(p) {}

        MutatingMetadataCollection metadata() {
            MetadataCollection::Pointer p = nullptr;
            THROW_IF(KColumnOpenMetadataUpdate, (get(), &p));
            return p;
        }
        MutatingMetadata operator [](std::string const &name) {
            return metadata()[name];
        }
    };

    KlibObject(KTable, Table)
    {
    public:
        using Base = Klib::Object<KTable>;
        Table(ConstPointer p) : Base(p) {}

    public:
        MetadataCollection metadata() const {
            MetadataCollection::ConstPointer p = nullptr;
            THROW_IF(KTableOpenMetadataRead, (get(), &p));
            return p;
        }

        Column operator [](std::string const &name) const {
            Column::ConstPointer p = nullptr;
            THROW_IF(KTableOpenColumnRead, (get(), &p, "%s", name.c_str()));
            return p;
        }

    protected:
        Table(Pointer p) : Base(p) {}
    };

    class MutatingTable final
    : public Table
    , Table::Consumer
    {
    public:
        Table readOnly() {
            return *this;
        }

        MutatingTable(Pointer p) : Table(p) {}

        MutatingMetadataCollection metadata() {
            MetadataCollection::Pointer p = nullptr;
            THROW_IF(KTableOpenMetadataUpdate, (get(), &p));
            return p;
        }

        MutatingColumn createColumn(std::string const &name) {
            Column::Pointer p = nullptr;
            THROW_IF(KTableCreateColumn, (get(), &p, kcmCreate, 0, 0, "%s", name.c_str()));
            return p;
        }

        MutatingColumn openColumn(std::string const &name) {
            Column::Pointer p = nullptr;
            THROW_IF(KTableOpenColumnUpdate, (get(), &p, "%s", name.c_str()));
            return p;
        }

        MutatingColumn operator [](std::string const &name) {
            try {
                return openColumn(name);
            }
            catch (RC_Exception const &rce) {
                auto const &rc = Klib::ResultCode(rce.value);
                if (!rc.isState(rcNotFound))
                    throw rce;
            }
            return createColumn(name);
        }

        void copyColumn(std::string const &name, Table const &from) {
            assert(!isSame(from));
            THROW_IF(KTableCopyColumn, (get(), getPointer(from), name.c_str()));
        }
    };

    KlibObject(KDatabase, Database)
    {
    public:
        using Base = Klib::Object<KDatabase>;
        Database(ConstPointer p) : Base(p) {}

        Database openDatabase(std::string const &name) const {
            ConstPointer p = nullptr;
            THROW_IF(KDatabaseOpenDBRead, (get(), &p, "%s", name.c_str()));
            return p;
        }
        Table openTable(std::string const &name) const {
            Table::ConstPointer p = nullptr;
            THROW_IF(KDatabaseOpenTableRead, (get(), &p, "%s", name.c_str()));
            return p;
        }

        MetadataCollection metadata() const {
            MetadataCollection::ConstPointer p = nullptr;
            THROW_IF(KDatabaseOpenMetadataRead, (get(), &p));
            return p;
        }

        Table operator [](std::string const &name) const {
            return openTable(name);
        }
    protected:
        Database(Pointer p) : Base(p) {}
    };

    class MutatingDatabase final
    : public Database
    {
    public:
        Database readOnly() {
            return *this;
        }

        MutatingDatabase(Pointer p) : Database(p) {}

        MutatingDatabase createDatabase(std::string const &name) {
            Database::Pointer p = nullptr;
            THROW_IF(KDatabaseCreateDB, (get(), &p, kcmInit|kcmCreate|kcmParents, "%s", name.c_str()));
            return p;
        }

        MutatingTable createTable(std::string const &name) {
            Table::Pointer p = nullptr;
            THROW_IF(KDatabaseCreateTable, (get(), &p, kcmInit|kcmCreate|kcmParents, "%s", name.c_str()));
            return p;
        }

        MutatingDatabase openDatabase(std::string const &name) {
            Pointer p = nullptr;
            THROW_IF(KDatabaseOpenDBUpdate, (get(), &p, "%s", name.c_str()));
            return p;
        }

        MutatingTable openTable(std::string const &name) {
            Table::Pointer p = nullptr;
            THROW_IF(KDatabaseOpenTableUpdate, (get(), &p, "%s", name.c_str()));
            return p;
        }

        MutatingMetadataCollection metadata() {
            MetadataCollection::Pointer p = nullptr;
            THROW_IF(KDatabaseOpenMetadataUpdate, (get(), &p));
            return p;
        }

        MutatingTable operator [](std::string const &name) {
            try {
                return openTable(name);
            }
            catch (RC_Exception const &rce) {
                auto const &rc = Klib::ResultCode(rce.value);
                if (!rc.isState(rcNotFound))
                    throw rce;
            }
            return createTable(name);
        }
    };

    KlibObject(KDBManager, Manager)
    , public KFS::Directory::Consumer
    {
    public:
        using Base = Klib::Object<KDBManager>;
        using Pointer = Base::Pointer;
        using ConstPointer = Base::ConstPointer;

    protected:
        using Consumer = KFS::Directory::Consumer;

        static ConstPointer make() {
            ConstPointer p = nullptr;
            THROW_IF(KDBManagerMakeRead, (&p, nullptr));
            return p;
        }
        static ConstPointer make(KFS::Directory const &dir) {
            ConstPointer p = nullptr;
            THROW_IF(KDBManagerMakeRead, (&p, getPointer(dir)));
            return p;
        }

    public:
        Manager() : Base(make()) {}
        Manager(KFS::Directory dir) : Base(make(dir)) {}

        Database openDatabase(std::string const &name) const {
            Database::ConstPointer p = nullptr;
            THROW_IF(KDBManagerOpenDBRead, (get(), &p, "%s", name.c_str()));
            return p;
        }

        Table openTable(std::string const &name) const {
            Table::ConstPointer p = nullptr;
            THROW_IF(KDBManagerOpenTableRead, (get(), &p, "%s", name.c_str()));
            return p;
        }

        int pathType(std::string const &path) const {
            return KDBManagerPathType(get(), "%.*s", path.length(), path.data());
        }

    protected:
        Manager(Pointer p) : Base(p) {}
    };

    class MutatingManager final : public Manager
    {
        static Pointer make() {
            Pointer p = nullptr;
            THROW_IF(KDBManagerMakeUpdate, (&p, nullptr));
            return p;
        }
        static Pointer make(KFS::MutatingDirectory &dir) {
            Pointer p = nullptr;
            THROW_IF(KDBManagerMakeUpdate, (&p, getPointer(dir)));
            return p;
        }
    public:
        Manager readOnly() {
            return *this;
        }

        MutatingManager() {
            static_assert(sizeof(*this) == sizeof(void *), "OH NO");
        }

        MutatingManager(KFS::MutatingDirectory &dir) : Manager(make(dir)) {}

        MutatingDatabase createDatabase(std::string const &name) {
            Database::Pointer p = nullptr;
            THROW_IF(KDBManagerCreateDB, (get(), &p, kcmInit|kcmCreate|kcmParents, "%s", name.c_str()));
            return p;
        }

        MutatingTable createTable(std::string const &name) {
            Table::Pointer p = nullptr;
            THROW_IF(KDBManagerCreateTable, (get(), &p, kcmInit|kcmCreate|kcmParents, "%s", name.c_str()));
            return p;
        }

        MutatingDatabase openDatabase(std::string const &name) {
            Database::Pointer p = nullptr;
            THROW_IF(KDBManagerOpenDBUpdate, (get(), &p, "%s", name.c_str()));
            return p;
        }

        MutatingTable openTable(std::string const &name) {
            Table::Pointer p = nullptr;
            THROW_IF(KDBManagerOpenTableUpdate, (get(), &p, "%s", name.c_str()));
            return p;
        }
    };
}
