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

#ifndef _h_vfs_keyring_
#define _h_vfs_keyring_

#ifndef _h_vfs_extern_
#include <vfs/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* 
 * API for clients of a client/server KeyRing implementation.
 * Client code can use this interface and fall back to using KKeyStore in some cases, e.g. to work with a project that has not been
 * added to the keyring. This can be done behind KKeyRing interface (would have to define an object naming scheme that is compatible 
 * with project-id/type/object-id), or explicitly in the client code.
 */
typedef struct KKeyRing KKeyRing;
 
 /* Make
 * Open an IPC connection to a keyring server. Will start the server if none is running.
 * KKeyRingMakeRead will reject Add/Delete operations 
 * dataDir [ IN, NULL OK ] = path to the directory with keyring database. NULL - use default location (~/.ncbi)
 */
VFS_EXTERN rc_t CC KKeyRingMakeRead( const KKeyRing** self, const char* dataDir );
VFS_EXTERN rc_t CC KKeyRingMakeUpdate( KKeyRing** self, const char* dataDir );

 /* AddRef
 * Release
 *
 * shutdown_server [ IN ] - if this is the last reference, send a Shutdown message to server before closing the IPC connection.
 * This will not necessarily shut down the server immediately; if there are other active connections, the server will wait for them to close first.
 * Use KKeyRingIsServerRunning() to check it the server is down.
 */
VFS_EXTERN rc_t CC KKeyRingAddRef ( const KKeyRing *self );
VFS_EXTERN rc_t CC KKeyRingRelease ( KKeyRing *self, bool shutdown_server );
 
/* 
 * retrieving download/encyprtion keys
 */
VFS_EXTERN rc_t CC KKeyRingGetDownloadTicket(const KKeyRing* self, const char* project_key, const char** dl_key);
VFS_EXTERN rc_t CC KKeyRingGetProjectEncryptionKey(const KKeyRing* self, const char* project_key, const char** enc_key);

/*
 * managing projects 
*/                                                   
VFS_EXTERN rc_t CC KKeyRingAddProject(KKeyRing* self, const char* project_key, const char* dl_key, const char* enc_key);
VFS_EXTERN rc_t CC KKeyRingDeleteProject(KKeyRing* self, const char* project_key); /* will delete all associated objects */

/*
 * managing objects
 */
VFS_EXTERN rc_t CC KKeyRingAddObject(KKeyRing* self, 
                                      const char* project_key, uint8_t object_type, const char* object_key,
                                      const char* display_name, uint64_t size, const char* checksum);
VFS_EXTERN rc_t CC KKeyRingDeleteObject(KKeyRing* self, 
                                         const char* project_key, uint8_t object_type, const char* object_key);

/* KKeyRingReencrypt
 * Server can refuse to reencrypt if it detects other active servers.
 */
VFS_EXTERN rc_t CC KKeyRingReencrypt(KKeyRing** self, const char* new_passwd); 

 /* IsServerRunning
 * dataDir [ IN, NULL OK ] = path to the directory with keyring database. NULL - use default location (~/.ncbi)
 */
VFS_EXTERN bool CC KKeyRingIsServerRunning(const char* dataDir);

/* 
 * private API for libkrypto 
 */

VFS_EXTERN rc_t CC KKeyRingGetObjectEncryptionKey(const KKeyRing* self, const char* project_key, 
                                                   uint8_t object_type, const char* object_key, const char** enc_key);

                                                   
#ifdef __cplusplus
}
#endif

#endif /* _h_vfs_keyring_ */
