/* ===========================================================================
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

#include "samextract.h"
#include "samextract-pool.h"
#include "samextract-tokens.h"
#include <align/samextract-lib.h>
#include <byteswap.h>
#include <ctype.h>
#include <errno.h>
#include <kfs/file.h>
#include <klib/defs.h>
#include <klib/rc.h>
#include <klib/time.h>
#include <klib/vector.h>
#include <kproc/lock.h>
#include <kproc/queue.h>
#include <kproc/thread.hpp>
#include <kproc/timeout.h>
#ifdef _MSC_VER
#else
#include <mmintrin.h>
#include <pthread.h>
#endif
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// All memmoves in this file are not overlapping and performance
// is measurably improved by memcpy.
#undef memcpy
#define memmove memcpy

typedef enum BGZF_state { empty, compressed, uncompressed } BGZF_state;
typedef struct BGZF_s
{
    KLock* lock;
    Bytef in[READBUF_SZ + 1024];
    Bytef out[READBUF_SZ + 1024];
    uInt insize;
    uInt outsize;
    BGZF_state state;
} BGZF;

class BGZFview
{
  public:
    BGZFview() : bgzf(NULL), cur(NULL) {}

    ~BGZFview()
    {
        releasebuf();
        bgzf = NULL;
        cur = NULL;
    }

// TODO: Handle _MSC_VER
// TODO: Move to a USE_ include that defines features:
// __HAS_RVALUE_REFERENCES, ...
// __HAS_METHOD_DELETE, ...
// __has_cpp_attribute
// __CPPVER=98,11,14,..
#ifndef __cplusplus
    // C++98
    BGZFview(const BGZFview&);
#elif __cplusplus <= 199711L
    // C++98
    BGZFview(const BGZFview&);
#elif __cplusplus >= 201103L
    // C++11, C++14
    BGZFview(const BGZFview&) = delete;             // No copy ctor
    BGZFview& operator=(const BGZFview&) = delete;  // No assignment
    BGZFview(const BGZFview&&) = delete;            // No move ctor
    BGZFview& operator=(const BGZFview&&) = delete; // No move assignment
#endif

  private:
    void releasebuf()
    {
        if (bgzf) {
            DBG("releasing");
            KLockUnlock(bgzf->lock);
            KLockRelease(bgzf->lock);
            bgzf->state = empty;
            free(bgzf);
            bgzf = NULL;
        }
    }

    bool getnextBGZF(KQueue* que)
    {
        releasebuf();

        struct timeout_t tm;

        while (true) {
            void* where = NULL;
            TimeoutInit(&tm, 10); // 10 milliseconds
            rc_t rc = KQueuePop(que, &where, &tm);
            DBG("popped");
            if (rc == 0) {
                bgzf = (BGZF*)where;
                DBG("Acquiring parser lock");
                KLockAcquire(bgzf->lock); // ready for parsing?
                DBG("Acquired parser lock");

                if (bgzf->state != uncompressed) {
                    ERR("\t\tParser queue bad state");
                    RC(rcAlign, rcFile, rcConstructing, rcNoObj,
                       rcUnexpected);
                    return false;
                }

                DBG("\t\tParser BGZF %p size %u", bgzf->out, bgzf->outsize);
                break;
            } else if ((int)GetRCObject(rc) == rcTimeout
                       || (int)GetRCObject(rc) == rcData) {
                DBG("\t\tParser queue empty");
                if (KQueueSealed(que)) {
                    DBG("\t\tQueue sealed, Parsing complete");
                    return false;
                }
            } else {
                ERR("Parser rc=%d", rc);
                return RC(rcAlign, rcFile, rcConstructing, rcNoObj,
                          rcUnexpected);
            }
        }
        return true;
    }

  public:
    inline bool getbytes(KQueue* que, char* dest, size_t len)
    {
        DBG("Getting %d", len);
        if (dest == NULL) {
            ERR("NULL dest");
            *dest = 0;
            return false;
        }
        if (len == 0) {
            ERR("Empty get");
            *dest = 0;
            return false;
        }
        while (len) {
            if (bgzf == NULL || bgzf->outsize == 0) {
                DBG("need %d more", len);
                if (!getnextBGZF(que)) {
                    DBG("no more data");
                    return false;
                }
                cur = bgzf->out;
            }

            size_t howmany = MIN(len, bgzf->outsize);

            memmove(dest, cur, howmany);
            len -= howmany;
            bgzf->outsize -= howmany;
            dest += howmany;
            cur += howmany;
        }
        return true;
    }

  private:
    BGZF* bgzf;
    Bytef* cur;
};

static BGZFview bview;

static rc_t seeker(const KThread* kt, void* in)
{
    rc_t rc;
    SAMExtractor* state = (SAMExtractor*)in;

#ifdef _MSC_VER
    u64 threadid = 0;
#else
    pthread_t threadid = pthread_self();
#endif
    DBG("\tSeeker thread %p %lu started.", kt, threadid);

    state->file_pos = 0;
    while (true) {
        if (state->readbuf_sz < 28) {
            ERR("Small block:%d", state->readbuf_sz);
        }

        if (!memcmp(state->readbuf,
                    "\x1f\x8b\x08\x04\x00\x00\x00\x00\x00\xff\x06\x00\x42\x43"
                    "\x02\x00\x1b\x00\x03\x00\x00\x00\x00\x00\x00\x00\x00"
                    "\x00",
                    28)) {
            DBG("complete EOF marker found");
            break;
        }

        z_stream strm;
        memset(&strm, 0, sizeof strm);
        strm.next_in = (Bytef*)state->readbuf;
        strm.avail_in = (uInt)state->readbuf_sz;
        int zrc = inflateInit2(&strm, MAX_WBITS + 16); // Only gzip format
        switch (zrc) {
            case Z_OK:
                break;
            case Z_MEM_ERROR:
                ERR("error: Out of memory in zlib");
                rc = RC(rcAlign, rcFile, rcReading, rcMemory, rcExhausted);
                state->rc = rc;
                return rc;
            case Z_VERSION_ERROR:
                ERR("zlib version is not compatible; need version %s "
                    "but "
                    "have %s",
                    ZLIB_VERSION, zlibVersion());
                rc = RC(rcAlign, rcFile, rcConstructing, rcNoObj,
                        rcUnexpected);
                state->rc = rc;
                return rc;
            case Z_STREAM_ERROR:
                ERR("zlib stream error");
                rc = RC(rcAlign, rcFile, rcConstructing, rcNoObj,
                        rcUnexpected);
                state->rc = rc;
                return rc;
            default:
                ERR("zlib error");
                rc = RC(rcAlign, rcFile, rcConstructing, rcNoObj,
                        rcUnexpected);
                state->rc = rc;
                return rc;
        }

        gz_header head;
        u8 extra[256];
        memset(&head, 0, sizeof head);
        head.extra = extra;
        head.extra_max = sizeof extra;
        char outbuf[64];
        strm.next_out = (Bytef*)outbuf;
        strm.avail_out = 64;
        zrc = inflateGetHeader(&strm, &head);
        if (zrc != Z_OK) {
            ERR("zlib inflate error %d %s", zrc, strm.msg);
            rc = RC(rcAlign, rcFile, rcConstructing, rcNoObj, rcUnexpected);
            state->rc = rc;
            return rc;
        }
        while (head.done == 0) {
            DBG("inflating gzip header");
            zrc = inflate(&strm, Z_BLOCK);
            if (zrc != Z_OK) {
                for (int i = 0; i != 4; ++i)
                    DBG("readbuf: %x", (unsigned char)state->readbuf[i]);
                ERR("zlib inflate error %d %s", zrc, strm.msg);
                rc = RC(rcAlign, rcFile, rcConstructing, rcNoObj,
                        rcUnexpected);
                state->rc = rc;
                return rc;
            }
        }

        DBG("found gzip header");
        // BC 02 bb
        if (head.extra && head.extra_len == 6 && head.extra[0] == 'B'
            && head.extra[1] == 'C' && head.extra[2] == 2
            && head.extra[3] == 0) {
            u16 bsize = head.extra[4] + head.extra[5] * 256;
            inflateEnd(&strm);
            DBG("total_in:%d", strm.avail_in);
            if (bsize <= 28) {
                ERR("small block found");
                break;
            }

            size_t block_size = 12; // Up to and including XLEN
            block_size += head.extra_len;
            block_size += (bsize - head.extra_len - 19); // CDATA
            block_size += 8;                             // CRC32 and isize
            DBG("block_size is %d bsize is %d", block_size, bsize);

            state->file_pos += block_size;

            BGZF* bgzf = (BGZF*)malloc(sizeof(*bgzf));
            KLockMake(&bgzf->lock);
            KLockAcquire(bgzf->lock); // Not ready for parsing
            bgzf->state = compressed;
            bgzf->insize = bsize + 1;
            memmove(bgzf->in, state->readbuf, block_size + 1);
            bgzf->outsize = sizeof(bgzf->out);

            struct timeout_t tm;
            while (true) {
                // Add to Inflate queue
                TimeoutInit(&tm, 10); // 10 milliseconds
                rc_t rc = KQueuePush(state->inflatequeue, (void*)bgzf, &tm);
                if ((int)GetRCObject(rc) == rcTimeout) {
                    DBG("inflate queue full");
                } else if (rc == 0) {
                    DBG("inflate queued: %p %d %d %d", bgzf->in, bgzf->insize,
                        rc, rcTimeout);
                    break;
                } else {
                    ERR("inflate queue %d", rc);
                }
            }

            while (true) {
                // Add to parse queue
                // lock will prevent parsing until inflater
                // thread finished with this chunk.
                TimeoutInit(&tm, 1000); // 1 second
                rc_t rc = KQueuePush(state->parsequeue, (void*)bgzf, &tm);
                if ((int)GetRCObject(rc) == rcTimeout) {
                    DBG("parse queue full");
                } else if (rc == 0) {
                    DBG("parse queued: %p %d %d %d", bgzf->in, bgzf->insize,
                        rc, rcTimeout);
                    break;
                } else {
                    DBG("parse queued%d", rc);
                }
            }
        } else {
            ERR("error: BAM required extra extension not found");
            rc = RC(rcAlign, rcFile, rcParsing, rcData, rcInvalid);
            state->rc = rc;
            return rc;
        }

        DBG("reading in at %d", state->file_pos);
        size_t sz = state->readbuf_sz;
        rc_t rc = KFileReadAll(state->infile, state->file_pos, state->readbuf,
                               sz, &sz);
        state->readbuf_sz = (u32)sz;
        if (rc) {
            ERR("readfile error");
            state->rc = rc;
            return rc;
        }
        DBG("Read in %d", state->readbuf_sz);
        state->readbuf_pos = 0;
        if (state->readbuf_sz == 0) {
            DBG("Buffer complete. EOF");
            break;
        }
    }

    KQueueSeal(state->parsequeue);
    DBG("seeker thread complete, sealed");
    return 0;
}

static rc_t inflater(const KThread* kt, void* in)
{
    SAMExtractor* state = (SAMExtractor*)in;
    struct timeout_t tm;

    z_stream strm;
#ifdef _MSC_VER
    u64 threadid = 0;
#else
    pthread_t threadid = pthread_self();
#endif
    DBG("\tInflater thread %p %lu started.", kt, threadid);

    while (true) {
        void* where = NULL;
        DBG("\t\tthread %lu checking queue", threadid);
        TimeoutInit(&tm, 10); // 10 milliseconds
        rc_t rc = KQueuePop(state->inflatequeue, &where, &tm);
        DBG("checked");
        if (rc == 0) {
            BGZF* bgzf = (BGZF*)where;
            DBG("\t\tinflater thread %lu BGZF %p size %u", threadid, bgzf->in,
                bgzf->insize);
            if (bgzf->state != compressed) {
                ERR("Inflater bad state");
                rc = RC(rcAlign, rcFile, rcReading, rcData, rcInvalid);
                state->rc = rc;
                return rc;
            }

            memset(&strm, 0, sizeof strm);
            DBG("\tinflating %d bytes", bgzf->insize);
            if (!bgzf->insize || !bgzf->outsize)
                ERR("Empty buffers %d %d", bgzf->insize, bgzf->outsize);
            strm.next_in = bgzf->in;
            strm.avail_in = bgzf->insize;
            strm.next_out = bgzf->out;
            strm.avail_out = bgzf->outsize;
            int zrc = inflateInit2(&strm, MAX_WBITS + 16); // Only gzip format
            switch (zrc) {
                case Z_OK:
                    break;
                case Z_MEM_ERROR:
                    ERR("Out of memory in zlib");
                    rc = RC(rcAlign, rcFile, rcReading, rcMemory,
                            rcExhausted);
                    state->rc = rc;
                    return rc;
                case Z_VERSION_ERROR:
                    ERR("zlib version is not compatible; need "
                        "version %s but "
                        "have %s",
                        ZLIB_VERSION, zlibVersion());
                    rc = RC(rcAlign, rcFile, rcConstructing, rcNoObj,
                            rcUnexpected);
                    state->rc = rc;
                    return rc;
                case Z_STREAM_ERROR:
                    ERR("zlib stream error");
                    rc = RC(rcAlign, rcFile, rcConstructing, rcNoObj,
                            rcUnexpected);
                    state->rc = rc;
                    return rc;
                default:
                    ERR("zlib error %s", strm.msg);
                    rc = RC(rcAlign, rcFile, rcConstructing, rcNoObj,
                            rcUnexpected);
                    state->rc = rc;
                    return rc;
            }

            zrc = inflate(&strm, Z_FINISH);
            switch (zrc) {
                case Z_OK:
                case Z_STREAM_END:
                    DBG("\t\tthread %lu OK %d %d %lu", threadid,
                        strm.avail_in, strm.avail_out, strm.total_out);
                    bgzf->outsize = (u32)strm.total_out;
                    bgzf->state = uncompressed;
                    DBG("Ready for parsing, unlocking");
                    KLockUnlock(bgzf->lock); // OK to parse now
                    break;
                case Z_MEM_ERROR:
                    ERR("error: Out of memory in zlib");
                    rc = RC(rcAlign, rcFile, rcReading, rcMemory,
                            rcExhausted);
                    state->rc = rc;
                    return rc;
                case Z_VERSION_ERROR:
                    ERR("zlib version is not compatible; need "
                        "version %s but "
                        "have %s",
                        ZLIB_VERSION, zlibVersion());
                    rc = RC(rcAlign, rcFile, rcConstructing, rcNoObj,
                            rcUnexpected);
                    state->rc = rc;
                    return rc;
                case Z_STREAM_ERROR:
                    ERR("zlib stream error %s", strm.msg);
                    rc = RC(rcAlign, rcFile, rcConstructing, rcNoObj,
                            rcUnexpected);
                    state->rc = rc;
                    return rc;
                default:
                    ERR("zlib inflate error %d %s", zrc, strm.msg);
                    rc = RC(rcAlign, rcFile, rcConstructing, rcNoObj,
                            rcUnexpected);
                    state->rc = rc;
                    return rc;
            }
            inflateEnd(&strm);
        } else if ((int)GetRCObject(rc) == rcTimeout
                   || (int)GetRCObject(rc) == rcData) {
            DBG("\t\tthread %lu queue empty", threadid);
            if (KQueueSealed(state->parsequeue)) {
                DBG("\t\tparse queue sealed, inflater thread %lu "
                    "terminating.",
                    threadid);
                return 0;
            }
        } else {
            WARN("rc=%d", rc);
            state->rc = rc;
            return rc;
        }
    }

    ERR("\t\tinflater thread %lu wrongly terminating.", threadid);
    return 0;
}

rc_t BAMGetHeaders(SAMExtractor* state)
{
    DBG("BAMGetHeaders");
    char magic[4];
    if (!bview.getbytes(state->parsequeue, magic, 4))
        return RC(rcAlign, rcFile, rcParsing, rcData, rcInvalid);
    if (memcmp(magic, "BAM\x01", 4)) {
        ERR("BAM magic not found");
        return RC(rcAlign, rcFile, rcParsing, rcData, rcInvalid);
    }
    i32 l_text;
    if (!bview.getbytes(state->parsequeue, (char*)&l_text, 4))
        return RC(rcAlign, rcFile, rcParsing, rcData, rcInvalid);
    if (l_text < 0 || l_text > 3000000) {
        ERR("error: invalid l_text %d", l_text);
        return RC(rcAlign, rcFile, rcParsing, rcData, rcInvalid);
    }
    DBG("l_text=%d", l_text);
    if (l_text) {
        char* text = (char*)pool_alloc((u32)l_text + 1);
        if (!bview.getbytes(state->parsequeue, text, l_text))
            return RC(rcAlign, rcFile, rcParsing, rcData, rcInvalid);
        text[l_text] = '\0';

        DBG("SAM header %d %d:'%s'", l_text, strlen(text), text);
        char* t = text;
        while (strlen(t)) {
            char* nl = (char*)strchr(t, '\n');
            if (!nl) {
                size_t linelen = strlen(t);
                DBG("noln linelen %d", linelen);
                memmove(curline, t, linelen);
                curline[linelen + 1] = '\n';
                curline[linelen + 2] = '\0';
                t += linelen;
            } else {
                size_t linelen = 1 + nl - t;
                DBG("ln linelen %d", linelen);
                memmove(curline, t, linelen);
                curline[linelen] = '\0';
                t += linelen;
            }
            DBG("curline is '%s'", curline);
            if (curline[0] != '@') {
                ERR("Not a SAM header line: '%s'", curline);
                return RC(rcAlign, rcFile, rcParsing, rcData, rcInvalid);
            }
            curline_len = strlen(curline);
            SAMparse(state);
            if (state->rc) {
                ERR("Parser returned error in BAMGetHeaders");
                return state->rc;
            }
        }
        text = NULL;
    } else {
        WARN("No SAM header");
    }

    if (!bview.getbytes(state->parsequeue, (char*)&state->n_ref, 4))
        return RC(rcAlign, rcFile, rcParsing, rcData, rcInvalid);
    if (state->n_ref < 0 || state->n_ref > 100000000) {
        ERR("error: invalid n_ref: %d", state->n_ref);
        return RC(rcAlign, rcFile, rcParsing, rcData, rcInvalid);
    }
    DBG("# references %d", state->n_ref);

    for (int i = 0; i != state->n_ref; ++i) {
        i32 l_name;
        if (!bview.getbytes(state->parsequeue, (char*)&l_name, 4))
            return RC(rcAlign, rcFile, rcParsing, rcData, rcInvalid);
        DBG("ref #%d/%d: l_name=%d", i, state->n_ref, l_name);
        if (l_name < 0) {
            ERR("error: invalid reference name length:%d", l_name);
            return RC(rcAlign, rcFile, rcParsing, rcData, rcInvalid);
        }
        if (l_name > 256) {
            ERR("warning: Long reference name:%d", l_name);
            return RC(rcAlign, rcFile, rcParsing, rcData, rcInvalid);
        }
        char* name
            = (char*)malloc(l_name + 1); // These need to persist across pools
        if (!bview.getbytes(state->parsequeue, name, l_name))
            return RC(rcAlign, rcFile, rcParsing, rcData, rcInvalid);
        DBG("ref #%d: name='%s'", i, name);
        VectorAppend(&state->bam_references, NULL, name);
        name = NULL;
        i32 l_ref;
        if (!bview.getbytes(state->parsequeue, (char*)&l_ref, 4))
            return RC(rcAlign, rcFile, rcParsing, rcData, rcInvalid);
        if (l_ref < 0) ERR("Bad l_ref %d", l_ref);
        DBG("length of reference sequence %d=%d", i, l_ref);
    }
    DBG("End of references");

    DBG("BAM done with headers");

    return 0;
}

static void init_decode_seq_map(u16* seqbytemap)
{
    static const unsigned char seqmap[] = "=ACMGRSVTWYHKDBN";

    for (size_t i = 0; i != 256; ++i) {
        u16 p;
        p = seqmap[i >> 4];
        p = p << 8;
        p |= seqmap[i & 0xf];
        seqbytemap[i] = bswap_16(p);
    }

    // Do some self-checks during initialization
    if (memcmp(&seqbytemap[0], "==", 2) || memcmp(&seqbytemap[1], "=A", 2)
        || memcmp(&seqbytemap[16], "A=", 2)
        || memcmp(&seqbytemap[255], "NN", 2)) {
        ERR("Self-check failed: seqbytemap %s", seqbytemap);
    }

    //           ==    NN    =A    A=    =N    N=    ==    =A    =A    =A
    u8 test[] = {0x00, 0xFF, 0x01, 0x10, 0x0F, 0xF0, 0x00, 0x01, 0x01, 0x01};
    char dest[30];

    decode_seq(test, 0, dest);
    if (strcmp(dest, "")) ERR("Self-check failed: %s", dest);
    decode_seq(test, 1, dest);
    if (strcmp(dest, "=")) ERR("Self-check failed: %s", dest);
    decode_seq(test, 2, dest);
    if (strcmp(dest, "==")) ERR("Self-check failed: %s", dest);
    decode_seq(test, 3, dest);
    if (strcmp(dest, "==N")) ERR("Self-check failed: %s", dest);
    decode_seq(test, 4, dest);
    if (strcmp(dest, "==NN")) ERR("Self-check failed: %s", dest);
    decode_seq(test, 5, dest);
    if (strcmp(dest, "==NN=")) ERR("Self-check failed: %s", dest);
    decode_seq(test, 20, dest);
    if (strcmp(dest, "==NN=AA==NN====A=A=A"))
        ERR("Self-check failed: %s", dest);

    DBG("Self-checks OK");
}

void decode_seq(const u8* seqbytes, size_t l_seq, char* seq)
{
    static u16 seqbytemap[256]; // NB: big endian

    if (!seqbytemap[0]) // Build seqbytemap
    {
        init_decode_seq_map(seqbytemap);
    }

    size_t remain = (l_seq + 1) / 2;

    const u64* in = (const u64*)seqbytes;
    u16* out = (u16*)seq;
    while (remain >= 8) {
        const u64 w = *in++;
        // ~25% of wall clock spent here.
        // Convince g++/clang to emit one load and one store for every two
        // bases
        *out++ = seqbytemap[(w >> 0) & 0xff];
        *out++ = seqbytemap[(w >> 8) & 0xff];
        *out++ = seqbytemap[(w >> 16) & 0xff];
        *out++ = seqbytemap[(w >> 24) & 0xff];
        *out++ = seqbytemap[(w >> 32) & 0xff];
        *out++ = seqbytemap[(w >> 40) & 0xff];
        *out++ = seqbytemap[(w >> 48) & 0xff];
        *out++ = seqbytemap[(w >> 56) & 0xff];

        remain -= 8;
    }

    const u8* b = (const u8*)in;
    while (remain) {
        *out = seqbytemap[*b];
        ++out;
        ++b;
        --remain;
    }
    // Final character might be junk from last nybble.
    seq[l_seq] = '\0';
}

void decode_qual(const u8* qualbytes, size_t l_seq, char* qual)
{
    if (qualbytes[0] == 255) {
        qual[0] = '*';
        qual[1] = '\0';
        return;
    }
    // Both qualbytes and qual are pool allocated and 8 byte aligned
    u64* out = (u64*)qual;
    const u64* in = (const u64*)qualbytes;
    // Assume no overflow/carry of quality
    for (u32 i = 0; i != l_seq / 8; ++i)
        out[i] = in[i] + 0x2121212121212121u; // 33 33 33...

    for (u32 i = 8 * (l_seq / 8); i != l_seq; ++i)
        qual[i] = qualbytes[i] + 33;

    qual[l_seq] = '\0';
}

size_t fast_u32toa(char* buf, u32 val)
{
    static u64 pow10[20];
    static char map10[100][2];

    // fast path:
    if (val <= 9) {
        buf[0] = val + '0';
        buf[1] = '\0';
        return 1;
    }

    // Initialize lookup tables
    if (map10[0][0] != '0') {
        // gcc double unrolls these, clang once
        for (int i = 0; i != 10; ++i)
            for (int j = 0; j != 10; ++j) {
                map10[10 * i + j][0] = i + '0';
                map10[10 * i + j][1] = j + '0';
            }
        u64 v = 1;
        for (int i = 0; i != 20; ++i) {
            pow10[i] = v;
            v *= 10;
        }
        pow10[0] = 0;
    }

// See http://graphics.stanford.edu/~seander/bithacks.html
#ifdef _MSC_VER
    // TODO: Untested
    u64 lg2;
    _BitScanReverse(&lg2, val | 1);
    lg2 = (u32)(64 - lg2);
#else
    u32 lg2 = (64 - __builtin_clzll(val | 1));
#endif
    u32 lg10 = lg2 * 1233 >> 12;
    lg10 = lg10 - (val < pow10[lg10]) + 1;

    buf += lg10;
    *buf = '\0';

    while (val >= 10) {
        buf -= 2;
        memmove(buf, &map10[val % 100], 2);
        val /= 100;
    }

    if (val) *--buf = val + '0';

    return lg10;
}

size_t fast_i32toa(char* buf, i32 val)
{
    u32 u = (u32)val;
    if (val < 0) {
        *buf++ = '-';
        u = ~u + 1; // twos-complement
        return fast_u32toa(buf, u) + 1;
    } else
        return fast_u32toa(buf, u);
}

char* decode_cigar(u32* cigar, u16 n_cigar_op)
{
    // Worst case:
    //   9 digits (28 bits of oplen) + 1 byte opcode
    //   Likely 1/5 that, but pool allocation cheaper than computing.
    char* scigar = (char*)pool_alloc(n_cigar_op * 10 + 1);
    char* p = scigar;
    for (int i = 0; i != n_cigar_op; ++i) {
        i32 oplen = cigar[i] >> 4;
        i32 op = cigar[i] & 0xf;

        size_t sz = fast_u32toa(p, oplen);
        p += sz;

        static const char opmap[] = "MIDNSHP=X???????";
        *p++ = (char)opmap[op];
    }

    *p = '\0';
    return scigar;
}

rc_t BAMGetAlignments(SAMExtractor* state)
{
    bamalign align;

    const char* ref_name = "";
    const char* next_ref_id = "";
    while (bview.getbytes(state->parsequeue, (char*)&align, sizeof(align))) {
        DBG("alignment block_size=%d refID=%d pos=%d", align.block_size,
            align.refID, align.pos);

        if (align.block_size < 0) {
            ERR("error: invalid block_size:%d", align.block_size);
            return RC(rcAlign, rcFile, rcParsing, rcData, rcInvalid);
        }
        if (align.pos < -1) {
            ERR("error: invalid pos:%d", align.pos);
            return RC(rcAlign, rcFile, rcParsing, rcData, rcInvalid);
        }
        if (align.refID < -1 || align.refID > state->n_ref) {
            ERR("error: bad refID:%d", align.refID);
            return RC(rcAlign, rcFile, rcParsing, rcData, rcInvalid);
        }
        if (align.next_refID < -1 || align.next_refID > state->n_ref) {
            ERR("error: bad next_refID:%d", align.next_refID);
            return RC(rcAlign, rcFile, rcParsing, rcData, rcInvalid);
        }
        if (align.next_pos < -1) {
            ERR("error: bad next_pos:%d", align.next_pos);
            return RC(rcAlign, rcFile, rcParsing, rcData, rcInvalid);
        }
        if (align.refID >= 0) {
            ref_name = (char*)VectorGet(&state->bam_references, align.refID);
            if (!ref_name) {
                WARN("Bad refID %d", align.refID);
                ref_name = "";
            }
        }

        DBG("next_ref_ID=%d", align.next_refID);
        if (align.next_refID >= 0) {
            next_ref_id
                = (char*)VectorGet(&state->bam_references, align.next_refID);
            if (!next_ref_id) {
                WARN("Bad next_ref_ID %d", align.next_refID);
                next_ref_id = "";
            }
            DBG("next_ref_id='%s'\n", next_ref_id);
        }

        DBG("align.bin_mq_nl=%d", align.bin_mq_nl);
        u16 bin = align.bin_mq_nl >> 16;
        u8 mapq = (align.bin_mq_nl >> 8) & 0xff;
        u8 l_read_name = align.bin_mq_nl & 0xff;
        DBG("bin=%d mapq=%d l_read_name=%d", bin, mapq, l_read_name);
        if (l_read_name == 0) {
            ERR("Empty read_name");
            return RC(rcAlign, rcFile, rcParsing, rcData, rcInvalid);
        }
        if (l_read_name > 90) ERR("Long (%d) read_name", l_read_name);

        u16 flag = align.flag_nc >> 16;
        u16 n_cigar_op = align.flag_nc & 0xffff;
        DBG("flag=%x n_cigar_op=%d", flag, n_cigar_op);

        char* read_name = (char*)pool_alloc(l_read_name);
        if (!bview.getbytes(state->parsequeue, read_name, l_read_name))
            return RC(rcAlign, rcFile, rcParsing, rcData, rcInvalid);
        if (strlen(read_name) != (size_t)l_read_name - 1) {
            ERR("read_name length mismatch: '%s' not length %d", read_name,
                l_read_name);
            return RC(rcAlign, rcFile, rcParsing, rcData, rcInvalid);
        }
        DBG("read_name='%s'", read_name);
        // TODO: Early out, check filter here, based on rname and pos
        char* scigar = NULL;
        if (n_cigar_op > 0) {
            u32* cigar = (u32*)pool_alloc(n_cigar_op * sizeof(u32));

            if (!bview.getbytes(state->parsequeue, (char*)cigar,
                                n_cigar_op * 4))
                return RC(rcAlign, rcFile, rcParsing, rcData, rcInvalid);
            // Worst case:
            //   9 digits (28 bits of oplen) + 1 byte opcode
            //   Likely 1/5 that, but pool allocation cheaper than
            //   computing.
            scigar = decode_cigar(cigar, n_cigar_op);
            cigar = NULL;
            DBG("scigar is '%s'", scigar);
        } else {
            DBG("close, but no cigar");
            scigar = pool_strdup("*");
        }

        u64 bytesofseq = 0;
        char* seq = NULL;
        char* qual = NULL;
        if (align.l_seq) {
            bytesofseq = (align.l_seq + 1) / 2;
            u8* seqbytes
                = (u8*)pool_alloc(bytesofseq); // Must be 8 byte aligned
            if (!bview.getbytes(state->parsequeue, (char*)seqbytes,
                                bytesofseq))
                return RC(rcAlign, rcFile, rcParsing, rcData, rcInvalid);

            u8* qualbytes = (u8*)pool_alloc(align.l_seq);
            if (!bview.getbytes(state->parsequeue, (char*)qualbytes,
                                align.l_seq))
                return RC(rcAlign, rcFile, rcParsing, rcData, rcInvalid);

            seq = (char*)pool_alloc(align.l_seq + 1);
            decode_seq(seqbytes, align.l_seq, seq);
            DBG("seq is '%s'", seq);

            // TODO: Make optional, most users don't care about quality
            qual = (char*)pool_alloc(align.l_seq + 1);
            decode_qual(qualbytes, align.l_seq, qual);

            DBG("%d pairs in sequence", align.l_seq);
            DBG("seq='%s'", seq);
            DBG("qual='%s'", qual);
            seqbytes = NULL;
        } else {
            seq = (char*)pool_alloc(1);
            seq[0] = '\0';
            qual = seq;
        }

        int remain = align.block_size
            - (sizeof(align) + l_read_name + n_cigar_op * 4 + bytesofseq
               + align.l_seq)
            + 4; // TODO, why 4?
        DBG("%d bytes remaining for ttvs", remain);
        char* ttvs = NULL;
        if (remain > 0) {
            ttvs = (char*)pool_alloc(remain);
            if (!bview.getbytes(state->parsequeue, ttvs, remain))
                return RC(rcAlign, rcFile, rcParsing, rcData, rcInvalid);
            char* cur = ttvs;
            DBG("Skipping TTVs");
            while (false && (cur < ttvs + remain)) // TODO
            {
                char tag[2];
                char c;
                i8 i8;
                u8 u8;
                i16 i16;
                u16 u16;
                i32 i32;
                u32 u32;
                char* z;
                tag[0] = *cur++;
                tag[1] = *cur++;
                char val_type = *cur++;
                DBG("ttv: %c%c:%c", tag[0], tag[1], val_type);
                switch (val_type) {
                    case 'A':
                        c = *cur++;
                        DBG("val='%c'", c);
                        break;
                    case 'c':
                        i8 = *cur++;
                        DBG("val=%d", i8);
                        break;
                    case 'C':
                        u8 = *cur++;
                        DBG("val=%d", u8);
                        break;
                    case 's':
                        memmove(&i16, cur, 2);
                        DBG("val=%d", i16);
                        cur += 2;
                        break;
                    case 'S':
                        memmove(&u16, cur, 2);
                        DBG("val=%d", u16);
                        cur += 2;
                        break;
                    case 'i':
                        memmove(&i32, cur, 4);
                        cur += 4;
                        break;
                    case 'I':
                        memmove(&u32, cur, 4);
                        cur += 4;
                        break;
                    case 'f':
                        // float f;
                        break;
                    case 'Z':
                        z = cur;
                        while (isprint(*cur)) ++cur;
                        DBG("val='%s'", z);
                        ++cur;
                        break;
                    case 'H':
                        z = cur;
                        while (isalnum(*cur)) ++cur;
                        DBG("val='%s'", z);
                        // TODO: Convert to ?
                        ++cur;
                        break;
                    case 'B':
                        val_type = *cur++;
                        memmove(&u32, cur, 4);
                        cur += 4;
                        cur += u32 * 1; // TODO, based on size of
                                        // val_type
                        break;
                    default:
                        ERR("Bad val_type:%c", val_type);
                        return RC(rcAlign, rcFile, rcParsing, rcData,
                                  rcInvalid);
                }
            }
        }
        DBG("no more ttvs");

        char sflag[16]; // Enough for i32/u32
        char spos[16] = "0";
        char spnext[16] = "0";
        char stlen[16] = "0";
        char smapq[16];
        fast_u32toa(sflag, flag);
        if (align.pos + 1)
            fast_i32toa(spos, align.pos + 1); // Convert to SAM's 1-based
        if (align.next_pos + 1) fast_i32toa(spnext, align.next_pos + 1); // ""
        if (align.tlen) fast_i32toa(stlen, align.tlen);
        fast_u32toa(smapq, mapq);

        process_alignment(state, read_name, sflag, ref_name, spos,
                          smapq,       // mapq
                          scigar,      // cigar
                          next_ref_id, // rnext
                          spnext,      // pnext
                          stlen,       // tlen
                          seq,         // read
                          qual         // qual
                          );

        read_name = NULL;
        seq = NULL;
        qual = NULL;
        ttvs = NULL;
        scigar = NULL;

        if (VectorLength(&state->alignments) == 64) {
            DBG("Have %d BAM alignments", VectorLength(&state->alignments));
            if (state->rc) {
                ERR("Something went wrong: %d", state->rc);
                return state->rc;
            }
            return 0;
        }
    }

    if (!KQueueSealed(state->parsequeue)) {
        ERR("out of data but queue not sealed");
        return RC(rcAlign, rcFile, rcParsing, rcData, rcInvalid);
    }
    DBG("parser complete");

    return 0;
}

void releasethreads(SAMExtractor* state)
{
    KSleepMs(100); // Wait 100 ms for threads to timeout in their queues
    DBG("Releasing threads");
    for (u32 i = 0; i != VectorLength(&state->threads); ++i) {
        KThread* t = (KThread*)VectorGet(&state->threads, i);
        // KThreadCancel(t);
        // KThreadWait(t, NULL);
        KThreadRelease(t);
    }
    DBG("Released threads");
}

rc_t threadinflate(SAMExtractor* state)
{
    rc_t rc;
    DBG("Starting threads");

    // Benchmarking on 32-core E5-2650 shows that even a memory resident BAM
    // file can't utilize more than 12 inflater threads.
    // ~8 seems to be the sweet spot.
    // This may change if quality parsing is made optional.
    size_t num_inflaters = MAX(1, MIN(state->num_threads - 1, 9));
    DBG("num_inflaters is %u", num_inflaters);
    // Inflater threads
    for (u32 i = 0; i != num_inflaters; ++i) {
        KThread* inflaterthread;
        rc = KThreadMake(&inflaterthread, inflater, (void*)state);
        if (rc) return rc;
        // rc = KThreadDetach(inflaterthread);
        // if (rc) return rc;
        VectorAppend(&state->threads, NULL, inflaterthread);
    }

    // Seeker thread
    KThread* seekerthread;
    rc = KThreadMake(&seekerthread, seeker, (void*)state);
    if (rc) return rc;
    rc = KThreadDetach(seekerthread);
    if (rc) return rc;
    VectorAppend(&state->threads, NULL, seekerthread);

    DBG("Threads started.");

    return 0;
}
