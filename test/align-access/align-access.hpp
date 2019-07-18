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

extern "C" {
#include <klib/rc.h>
#include <align/bam.h>
#include <align/align-access.h>
#include <vfs/path.h>
#include <vfs/manager.h>

    rc_t AlignAccessAlignmentEnumeratorGetSAM(const AlignAccessAlignmentEnumerator *self, size_t *const actSize, size_t const maxsize, char *const buffer);
}

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
        std::string SAM() const {
            char buf[64 * 1024];
            size_t sz = 0;
            if (AlignAccessAlignmentEnumeratorGetSAM(self, &sz, sizeof(buf), buf) == 0) {
                return std::string(buf, sz);
            }
            else {
                return "";
            }
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

