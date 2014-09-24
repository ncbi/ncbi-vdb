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

#include "keyring.h"

#include <klib/rc.h>
#include <klib/refcount.h>
#include <klib/text.h>
#include <klib/data-buffer.h>
#include <klib/log.h>

#include <kns/stream.h>
#include <kns/endpoint.h>
#include <kns/socket.h>
#include <kns/manager.h>

#include <vfs/keyring-priv.h>

#include <sysalloc.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/*TODO: define protocol messages in a central location */
/*requests*/
static const char* shutdownMsg = "\x01X";
static const char* initMsg     = "\x01I";
/*responses*/
static const char* initMsgSuccess = "IY";
static const char* initMsgFailure = "IN";

#define rcTarget rcNoTarg

#define KEYRING_MAX_PATH 4096

struct KKeyRing
{
    KRefcount refcount;
    
    char serverDataDir[KEYRING_MAX_PATH];

    bool read_only;
    bool shutting_down;
    
    KStream* ipc;
};

static
rc_t CC ShakeHands(KStream* ipc)
{   /* Send Init message, wait for a response (give new server a chance to ask for password */
    rc_t rc;
    LogMsg ( klogInfo, "KeyringClient: sending Init");
    rc = KStreamWrite(ipc, initMsg, string_size(initMsg), NULL);
    if (rc == 0)
    {
        unsigned char msg_size;
        size_t numRead = 0;
        rc = KStreamReadAll(ipc, &msg_size, 1, &numRead);
        if (rc == 0 && numRead == 1)
        {
            char resp[2];
            if (msg_size == sizeof(resp))
            {
                rc = KStreamReadAll(ipc, resp, msg_size, &numRead);
                if (rc == 0 && numRead == msg_size)
                {
                    pLogMsg ( klogInfo, "KeyringClient: received '$(b1)$(b2)'", "b1=%c,b2=%c", resp[0], resp[1] );
                    if (string_cmp(resp, sizeof(resp), initMsgSuccess, sizeof(initMsgSuccess), msg_size) == 0)
                        rc = 0;
                    else if (string_cmp(resp, sizeof(resp), initMsgFailure, sizeof(initMsgFailure), msg_size) == 0)
                        rc = RC ( rcVFS, rcTarget, rcAttaching, rcParam, rcIncorrect);
                    else
                        rc = RC ( rcVFS, rcTarget, rcAttaching, rcMessage, rcUnexpected);
                }
                else
                    LogErr(klogErr, rc, "KeyringClient: KStreamReadAll(2) failed");
            }
            else
            {
                pLogMsg ( klogInfo, "KeyringClient: received '$(bytes)' bytes, expected 2", "bytes=%d", (int32_t)msg_size );
                rc = RC ( rcVFS, rcTarget, rcAttaching, rcMessage, rcUnexpected);
            }
        }
        else
        {
            pLogMsg ( klogInfo, "KeyringClient: received '$(bytes)' bytes, expected 1", "bytes=%d", (int32_t)numRead );
            LogErr(klogErr, rc, "KeyringClient: KStreamReadAll(1) failed");
        }
    }
        
    return rc;
}

static
rc_t CC KKeyRingInit(KKeyRing* self, const char* path)
{
    rc_t rc = 0;
    memset(self, 0, sizeof(KKeyRing));

    KRefcountInit ( & self -> refcount, 1, "KKeyRing", "init", "" );

    LogMsg ( klogInfo, "KeyringClient: starting keyring");
    rc = StartKeyRing(path);
    LogMsg ( klogInfo, "KeyringClient: keyring started");
    
    if (rc == 0)
    {
        KNSManager* kns;
        rc = KNSManagerMake ( &kns );
        if (rc == 0)
        {
            rc_t rc2;
            KEndPoint ep;
            String name; 
            CONST_STRING(&name, KEYRING_IPC_NAME);
            rc = KNSManagerInitIPCEndpoint(kns, &ep, &name);    
            if (rc == 0)
            {
                LogMsg ( klogInfo, "KeyringClient: caling KNSMakeIPCConnection");
                rc = KNSManagerMakeIPCConnection ( kns, &self->ipc, &ep, 5 );
                if (rc == 0)
                {
                    LogMsg ( klogInfo, "KeyringClient: KNSMakeIPCConnection successful");
                    rc = ShakeHands(self->ipc);
                }
            }
            rc2 = KNSManagerRelease(kns);
            if (rc == 0)
                rc2 = rc;
        }
    }
    
    return rc;    
}

static
rc_t CC KKeyRingWhack(KKeyRing* self)
{
    rc_t rc = 0;
    rc_t rc2;
    if (self->ipc != NULL && self->shutting_down)  
    {    /* send a shutdown message to the server */
        LogMsg ( klogInfo, "KeyringClient: sending Shutdown");
        rc = KStreamWrite(self->ipc, shutdownMsg, string_size(shutdownMsg), NULL);
    }
        
    rc2 = KStreamRelease(self->ipc);
    if (rc == 0)
        rc = rc2;
        
    free(self);
    
    return rc;
}

LIB_EXPORT
rc_t CC KKeyRingAddRef ( const KKeyRing *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAdd ( & self -> refcount, "KKeyRing" ) )
        {
        case krefLimit:
            return RC ( rcVFS, rcTarget, rcAttaching, rcRange, rcExcessive );
        }
    }
    return 0;
}

LIB_EXPORT 
rc_t CC KKeyRingRelease ( KKeyRing *self, bool shutdown_server )
{
    if ( self != NULL )
    {
        if (shutdown_server)
            self->shutting_down = true; /* request shutdown when refcount reaches 0 */
            
        switch ( KRefcountDrop ( & self -> refcount, "KKeyRing" ) )
        {
        case krefWhack:
            return KKeyRingWhack ( ( KKeyRing* ) self );
        break;
        case krefLimit:
            return RC ( rcVFS, rcTarget, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
}

LIB_EXPORT rc_t CC KKeyRingMakeRead( const KKeyRing** cself, const char* path )
{
    KKeyRing** self = (KKeyRing**)cself;
    rc_t rc = KKeyRingMakeUpdate(self, path);
    if (rc == 0)
        (*self)->read_only = true;
    return rc;
}

LIB_EXPORT 
rc_t CC KKeyRingMakeUpdate(KKeyRing** self, const char* path)
{
    KKeyRing* obj;
    rc_t rc;
    
    if ( self == NULL )
        rc = RC ( rcVFS, rcTarget, rcCreating, rcParam, rcNull );
    else
    {
        obj = malloc(sizeof(KKeyRing));
        if (obj == NULL)
            rc = RC ( rcVFS, rcTarget, rcCreating, rcMemory, rcExhausted );
        {
            rc = KKeyRingInit(obj, path);
            if (rc == 0)
               *self = obj;
            else
                free(obj);
        }
    }
    
    return rc;
}

rc_t CC KKeyRingAddProject(KKeyRing* self, const char* project_key, const char* dl_key, const char* enc_key)
{
    rc_t rc = 0;
    if ( self == NULL )
        rc = RC ( rcVFS, rcTarget, rcSending, rcSelf, rcNull );
    else if (project_key == NULL || dl_key == NULL || enc_key == NULL)
        rc = RC ( rcVFS, rcTarget, rcSending, rcParam, rcNull );
    else
    {   /* Project Add: "<uint8 msg_length>PA<uint8 length>name<uint8 length>dl_ticket<uint8 length>enc_key" */
        size_t pkSize = string_size(project_key);
        size_t dkSize = string_size(dl_key);
        size_t ekSize = string_size(enc_key);

        char buf[256];
        uint8_t msg_size = 2 + sizeof(uint8_t) + pkSize + sizeof(uint8_t) + dkSize + sizeof(uint8_t) + ekSize;
        if ( 1 + msg_size > sizeof(buf) )
            rc = RC ( rcVFS, rcTarget, rcSending, rcSize, rcExcessive );
        else
        {
            size_t idx = 0;
        
            buf[0] = msg_size; ++idx;

            buf[idx]='P'; ++idx;
            buf[idx]='A'; ++idx;
            
            buf[idx] = (char)pkSize; ++idx;
            string_copy(buf + idx, sizeof(buf)-idx, project_key, pkSize);
            idx += pkSize;
            
            buf[idx] = (char)dkSize; ++idx;
            string_copy(buf + idx, sizeof(buf)-idx, dl_key, dkSize);
            idx += dkSize;
            
            buf[idx] = (char)ekSize; ++idx;
            string_copy(buf + idx, sizeof(buf)-idx, enc_key, ekSize);
            idx += ekSize;
            
            assert(idx == msg_size + 1);
            rc = KStreamWrite(self->ipc, buf, idx, NULL);
        }
    }
    return rc;
        
}

