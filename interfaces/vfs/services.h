#ifndef _h_vfs_services_
#define _h_vfs_services_

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
* ==============================================================================
* external SERVICES
*/


#include <kfg/kart.h>     /* EObjectType */
#include <vfs/resolver.h> /* VRemoteProtocols */


#ifdef __cplusplus
extern "C" {
#endif


struct Kart;
struct KNSManager;

typedef struct KService KService;
typedef struct KSrvError KSrvError;
typedef struct KSrvResponse KSrvResponse;

typedef struct KSrvRespObj KSrvRespObj;
typedef struct KSrvRespObjIterator KSrvRespObjIterator;
typedef struct KSrvRespFile KSrvRespFile;
typedef struct KSrvRespFileIterator KSrvRespFileIterator;

/* File Format from name resolver response - verson 4 */
typedef enum {
    eSFFInvalid,
    eSFFSkipped,
    eSFFSra,
    eSFFVdbcache,
    eSFFMax,
} ESrvFileFormat;

/******************************************************************************/
/* KService - EXTERNAL Service */

/* Make KService object */
rc_t KServiceMake ( KService ** self );

/* Release KService object */
rc_t KServiceRelease ( KService * self );

/* Add an Id ( Accession or Object-Id ) to service request */
rc_t KServiceAddId     ( KService * self, const char * id );

/* Add an Object ( ObjectId or dbgap|ObjectId ) to service request */
rc_t KServiceAddObject(KService * self, const char * id);

/* Add a dbGaP Project to service request */
rc_t KServiceAddProject ( KService * self, uint32_t id );

/* Set location of data in service request */
rc_t KServiceSetLocation(KService * self, const char * location );

/* Set accept-format-in of service request */
rc_t KServiceSetFormat(KService * self, const char * format);

/* Set jwt kart argument in service request */
rc_t KServiceSetJwtKartFile(KService * self, const char * path);

/* Set ngc file argument in service request */
rc_t KServiceSetNgcFile(KService * self, const char * path);

/************************** name service - version 3 **************************/
/* Execute Names Service Call using current default protocol version;
 * get KSrvResponse - it contains:
 *   - remote/local/cache location for every requested Id
 *   - or KSrvError
 */
rc_t KServiceNamesQuery ( KService * self, VRemoteProtocols protocols, 
                          const KSrvResponse ** response );

rc_t KServiceNamesQueryTo ( KService * self, VRemoteProtocols protocols,
    const char * outDir, const char * outFile, const KSrvResponse ** response );

/************************** search service - version 1 ************************/
/* Execute Search Service Call; get Kart response */
rc_t KServiceSearchExecute ( KService * self,
                             const struct Kart ** response );

/******************************************************************************/
/* GetResponseCStr
 * Returns a pointer to an array that contains a null-terminated sequence
 * of characters representing the current server response.
 */
const char * KServiceGetResponseCStr(const struct KService * self);

/************************** KSrvResponse **************************/
/* Release:
 * Release KSrvResponse object */
rc_t     KSrvResponseRelease ( const KSrvResponse * self );

/* Length:
 * Number of elements in KSrvResponse */
uint32_t KSrvResponseLength  ( const KSrvResponse * self );

/* GetPath:
 * Get KSrvResponse element number "idx" for "protocol":
 * - remote "path"/"vdbcache",
 * or "error"
 */
rc_t KSrvResponseGetPath ( const KSrvResponse * self, uint32_t idx,
    VRemoteProtocols p, const struct VPath ** path,
    const struct VPath ** vdbcache, const KSrvError ** error );

/* GetCache
 * get cache location for KSrvResponse element number "idx"
 */
rc_t KSrvResponseGetCache(const KSrvResponse * self, uint32_t idx,
    const struct VPath ** path);

/* GetNextToken:
 * is returned in case user need to ask for next page.
 * Do not release returned value.
 *
 * When returned rc == RC(rcVFS, rcQuery, rcExecuting, rcToken, rcUnexpected):
 *  the server returned nextToken but its processing is not implemented yet.
 */
rc_t KSrvResponseGetNextToken(const KSrvResponse * self,
    const char ** nextToken);

rc_t KSrvResponseGetLocation(const KSrvResponse * self,
    const char * acc, const char * name,
    const struct VPath ** local, rc_t * localRc,
    const struct VPath ** cache, rc_t * cacheRc);

/************************** KSrvError ******************************
 * KSrvError is generated for Id-s from request that produced an error response
 */
rc_t KSrvErrorRelease ( const KSrvError * self );
rc_t KSrvErrorAddRef  ( const KSrvError * self );

/* Rc - rc code corresponding to this Error */
rc_t KSrvErrorRc      ( const KSrvError * self, rc_t     * rc   );

/* Code - Status-Code returned by server */
rc_t KSrvErrorCode    ( const KSrvError * self, uint32_t * code );

/*  returns pointers to internal String data
 *  Strings remain valid while "self" is valid
 */
/* Message - message returned by server */
rc_t KSrvErrorMessage ( const KSrvError * self, String * message );
/* Object - Object-Id/Object-Type that produced this Error */
rc_t KSrvErrorObject  ( const KSrvError * self,
                        String * id, EObjectType * type );
/******************************************************************************/

/************************** KSrvRespObj ******************************/

rc_t KSrvRespObjRelease ( const KSrvRespObj * self );
rc_t KSrvResponseGetObjByIdx ( const struct KSrvResponse * self,
                               uint32_t idx, const KSrvRespObj ** obj );
rc_t KSrvResponseGetObjByAcc ( const struct KSrvResponse * self,
                               const char * acc, const KSrvRespObj ** obj );

/* Do not release returned acc */
rc_t KSrvRespObjGetAccOrId(const KSrvRespObj * self,
	                       const char ** acc, uint32_t * id);

/* Do not release returned msg */
rc_t KSrvRespObjGetError(const KSrvRespObj * self,
                         rc_t * rc, int64_t * code, const char ** msg);

rc_t KSrvRespObjGetFileCount ( const KSrvRespObj * self, uint32_t * count ); 

rc_t KSrvRespObjMakeIterator ( const KSrvRespObj * self,
                               KSrvRespObjIterator ** it ); 
rc_t KSrvRespObjIteratorRelease ( const KSrvRespObjIterator * self );

rc_t KSrvRespObjIteratorNextFile ( KSrvRespObjIterator * self,
                                   KSrvRespFile ** file );

/* Do not release returned char pointers */
rc_t KSrvRespFileGetAccOrId(const KSrvRespFile * self,
    const char ** acc, uint32_t * id);
rc_t KSrvRespFileGetClass(const KSrvRespFile * self, const char ** itemClass);

/* type: sra, vdbcache, etc. */
rc_t KSrvRespFileGetType(const KSrvRespFile * self, const char ** type);
rc_t KSrvRespFileGetFormat(const struct KSrvRespFile * self,
    ESrvFileFormat * ff);

rc_t KSrvRespFileGetSize(const KSrvRespFile * self, uint64_t *size);
rc_t KSrvRespFileGetCache ( const KSrvRespFile * self,
                            const struct VPath ** path );
rc_t KSrvRespFileGetLocal ( const KSrvRespFile * self,
                            const struct VPath ** path );

rc_t KSrvRespFileRelease  ( const KSrvRespFile * self );

rc_t KSrvRespFileMakeIterator ( const KSrvRespFile * self,
    KSrvRespFileIterator ** it );
rc_t KSrvRespFileIteratorRelease ( const KSrvRespFileIterator * self );
rc_t KSrvRespFileIteratorNextPath ( KSrvRespFileIterator * self,
                                    const struct VPath ** path );

/******************************************************************************/

#ifdef __cplusplus
}
#endif


#endif /* _h_vfs_services_ */
