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
*
*/

#ifndef _h_vfs_json_response_
#define _h_vfs_json_response_

#include <kfc/defs.h> /* rc_t */
#include <klib/time.h> /* KTime */

#ifdef __cplusplus
extern "C" {
#endif

struct Data;
struct KJsonValue;
struct KSrvRespFile;
struct KSrvRespObj;
struct Node;
struct Status;
struct String;
struct VPath;

typedef struct Container Container;
typedef struct Item Item;
struct File;
typedef struct Response4 Response4;

typedef enum {
    eUnknown,
    eFalse,
    eTrue
} EState;

typedef struct Data {
    const char * acc;
    const char * bundle;
    int64_t      code; /* status/code */
    EState       ceRequired;
    int64_t      encryptedForProjectId;
    const char * sEncryptedForProjectId;
    int64_t      exp;  /* expDate */
    const char * fmt;  /* format */
    EState       qual; /* hasOrigQuality */
    const char * cls;  /* itemClass */
    const char * link; /* ??????????????????????????????????????????????????? */
    const char * md5;

    const char * modificationDate;
    int64_t      mod;  /* modDate */

    const char * msg;
    const char * name;
    const char * object;
    const char * objectType;
    EState       payRequired;
    int64_t      id;   /* oldCartObjId */
    const char * reg;  /* region */
    const char * sha;  /* sha256 */
    const char * srv;  /* service */
    const char * tic;
    int64_t      sz;   /* size */
    const char * type;
    const char * vsblt;
} Data;

rc_t Response4MakeEmpty  (       Response4 ** self );
rc_t Response4Make4      (       Response4 ** self, const char * input );
rc_t Response4MakeSdl    (       Response4 ** self, const char * input );
rc_t Response4AddRef     ( const Response4  * self );
rc_t Response4Release    ( const Response4  * self );
rc_t Response4AppendUrl  (       Response4  * self, const char * url );
rc_t Response4AddAccOrId (       Response4 * self, const char * acc,
                                 int64_t id, Container ** newItem );
rc_t Response4SetNextToken(Response4 * self, const char * nextToken);
rc_t Response4GetNextToken(const Response4 * self, const char ** nextToken);
rc_t Response4StatusInit(Response4 * self, int64_t code, const char * msg,
    bool error);
rc_t Response4GetRc      ( const Response4 * self, rc_t * rc );
rc_t ContainerStatusInit(Container * self, int64_t code, const char * msg);
bool ContainerIs200AndEmpty(const Container * self);
void ContainerProcessStatus(Container * self, const Data * data);
rc_t ContainerAdd ( Container * self, const char * acc, int64_t id,
                    Item ** newItem, const struct Data * data );
rc_t ItemAddFormat(Item * self, const char * cType, const Data * dad,
    struct File ** added, bool checkSameType);
rc_t ItemAddVPath(Item * self, const char * type, const struct VPath * path,
                    const struct VPath * mapping, bool setHttp, uint64_t osize);
rc_t ItemSetTicket ( Item * self, const struct String * ticket );
void ItemLogAdd(const Item * self);
void FileLogAddedLink(const struct File * self, const char * url);
rc_t FileAddVPath(struct File * self, const struct VPath * path,
    const struct VPath * mapping, bool setHttp, uint64_t osize);
rc_t ItemInitMapping(Item * self);
rc_t Response4GetKSrvRespObjCount ( const Response4 * self, uint32_t * n );
rc_t Response4GetKSrvRespObjByIdx ( const Response4 * self, uint32_t i,
                                    const struct KSrvRespObj ** box );
rc_t Response4GetKSrvRespObjByAcc ( const Response4 * self, const char * acc,
                                    const struct KSrvRespObj ** box );
rc_t Response4Fini(Response4 * self);

typedef struct {
    struct Node * nodes;
    size_t i;
    size_t n;
} JsonStack;

rc_t IntSet(int64_t * self, const struct KJsonValue * node,
    const char * name, JsonStack * path);
rc_t BulSet(EState * self, const struct KJsonValue * node,
    const char * name, JsonStack * path);
rc_t StrSet(const char ** self, const struct KJsonValue * node,
    const char * name, JsonStack * path);

#define THRESHOLD_NO_DEBUG 0
#define THRESHOLD_ERROR    1
extern int THRESHOLD;
void JsonStackPrintInput(const char * input);
rc_t JsonStackRelease(JsonStack * self, bool failed);
rc_t JsonStackInit(JsonStack * self);
void JsonStackPop(JsonStack * self);
rc_t JsonStackPushArr(JsonStack * self, const char * name);
rc_t JsonStackArrNext(JsonStack * self);

#ifdef __cplusplus
}
#endif

#endif /* _h_vfs_json_response_ */
