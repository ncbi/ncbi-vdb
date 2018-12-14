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

#include <kfg/extern.h>

#include <kfg/keystore-priv.h>

#include <klib/refcount.h>
#include <klib/rc.h>
#include <klib/text.h>
#include <klib/printf.h>

#include <kfg/config.h>
#include <kfg/repository.h>
#include <kfg/kfg-priv.h>

#include <kfs/directory.h>
#include <kfs/file.h>
#include <kfs/lockfile.h>
#include <kfs/nullfile.h>

#include <strtol.h>

#include <sysalloc.h>
#include <stdlib.h>
#include <string.h>

#define PASSWORD_MAX_SIZE 4096
#define MAX_PATH_SIZE 4096

static char defaultBindingsFile[MAX_PATH_SIZE];

static rc_t KEncryptionKeyMakeFromFile(const char* path, KEncryptionKey** self);

/* 
 * Non-keyring implementation, using a combination of location-based and global keys
 */

struct KKeyStore
{
    KKeyStore_vt vt;
    KRefcount refcount;
    
    KEncryptionKey* temp_key; /* temp key provided by the client code*/
    
    const KConfig* kfg;
    char* bindingsFile;
};

static rc_t KKeyStoreWhack(KKeyStore* self);

static KKeyStore_vt_v1 v1 = {
    1, 0,
    KKeyStoreWhack,
    KKeyStoreGetKey
};

rc_t KKeyStoreWhack(KKeyStore* self)
{
    rc_t rc = KConfigRelease(self->kfg);
    rc_t rc2 = KEncryptionKeyRelease(self->temp_key);
    if (rc == 0)
        rc = rc2;
    if (self->bindingsFile != defaultBindingsFile)
        free(self->bindingsFile);
    free(self);
    return rc;
}

LIB_EXPORT rc_t CC KKeyStoreMake(KKeyStore** self, KConfig* kfg)
{
    rc_t rc = 0;
    KKeyStore* ret;
    
    if ( self == NULL )
        return RC ( rcKFG, rcStorage, rcCreating, rcSelf, rcNull );

    *self = NULL;
    
    ret = malloc(sizeof(*ret));
    if (ret == NULL)
        return RC ( rcKFG, rcStorage, rcCreating, rcMemory, rcExhausted );
    
    ret -> vt . v1 =  &v1;
    
    KRefcountInit ( & ret -> refcount, 1, "KKeyStore", "init", "" );
    
    ret->temp_key = NULL;
    ret->bindingsFile = NULL;
    ret->kfg = kfg;
    if (kfg != NULL)
    {
        rc = KConfigAddRef(kfg);
        if (rc == 0)
            ret->kfg = kfg;
    }
    else
        ret->kfg = NULL;
    
    if (rc == 0)
        *self = ret;
    else
        KKeyStoreWhack(ret);
    
    return rc;
}

LIB_EXPORT rc_t CC KKeyStoreAddRef ( const KKeyStore *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAdd ( & self -> refcount, "KKeyStore" ) )
        {
        case krefLimit:
            return RC ( rcKFG, rcStorage, rcAttaching, rcRange, rcExcessive );
        }
    }
    return 0;
}

LIB_EXPORT rc_t CC KKeyStoreRelease ( const KKeyStore *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDrop ( & self -> refcount, "KKeyStore" ) )
        {
        case krefWhack:
            self -> vt . v1 -> destroy ( ( KKeyStore* ) self );
        break;
        case krefLimit:
            return RC ( rcKFG, rcStorage, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
}

/*
 * the result is always 0-terminated
 */
static
rc_t ReadEncKey(const KFile* file, char* buf, size_t bufsize)
{
    /* at this point we are only getting the password from a 
     * file but in the future if we can get it from a pipe of
     * some sort we can't count on the ReadAll to really know
     * if we hit end of file and not just a pause in the
     * streaming.  VDB 3 / VFS/KFS 2 will have to fix this somehow
     */
    size_t readNum;
    rc_t rc = KFileReadAll (file, 0, buf, bufsize - 1, &readNum);

    if (rc == 0)
    {
        char * pc;

        /* ensure buffer is NUL terminated */
        buf [ readNum ] = 0;
        
        /* -----
         * trim back the contents of the file to
         * a single ASCII/UTF-8 text line
         * We actually only check for the two normal
         * end of line characters so it could have other
         * control characters...
         */
        pc = memchr ( buf, '\r', readNum );
        if ( pc == NULL )
            pc = memchr ( buf, '\n', readNum );
        if (pc != NULL)
            *pc = 0;

        /* disallow a length of 0 */
        if ( buf [ 0 ] == 0 )
            rc = RC (rcKFG, rcEncryptionKey, rcRetrieving, rcSize, rcTooShort);
        else if ( memcmp ( buf, "n/a", 4 ) == 0 )
        {
            /* download-only NGC file */
            rc = RC ( rcKFG, rcEncryptionKey, rcRetrieving, rcEncryptionKey, rcNoPerm );
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC KKeyStoreSetTemporaryKeyFromFile(KKeyStore* self, const struct KFile* file)
{
    rc_t rc;
    char key[PASSWORD_MAX_SIZE + 1];
    
    if ( self == NULL )
        return RC ( rcKFG, rcEncryptionKey, rcRetrieving, rcSelf, rcNull );
        
    if ( file == NULL )
    {
        rc = KEncryptionKeyRelease(self->temp_key);
        self->temp_key = NULL;
    }
    else
    {    
        rc = ReadEncKey(file, key, PASSWORD_MAX_SIZE);
        if (rc == 0)
        {
            rc = KEncryptionKeyRelease(self->temp_key);
            if ( rc == 0)
                rc = KEncryptionKeyMake(key, &self->temp_key);
            memset(key, 0, PASSWORD_MAX_SIZE); /* is this enough security? */
        }
    }
    
    return rc;
}


static rc_t CC KKeyStoreGetKeyInt(const KKeyStore* self, const char* obj_key,
    KEncryptionKey** enc_key, bool by_project_id, uint32_t projectId)
{
    rc_t rc = 0;
    
    if ( self == NULL )
        return RC ( rcKFG, rcEncryptionKey, rcRetrieving, rcSelf, rcNull );
    if ( enc_key == NULL )
        return RC ( rcKFG, rcEncryptionKey, rcRetrieving, rcParam, rcNull );
    
    if (self->temp_key != NULL)
    {
        rc = KEncryptionKeyAddRef(self->temp_key);
        if (rc == 0)
            *enc_key = self->temp_key;
    }
    else
    {
        const char * env = getenv (ENV_KRYPTO_PWFILE);
        if (env != NULL)
            rc = KEncryptionKeyMakeFromFile(env, enc_key);
        else 
        {   /* try protected repository */
            if ( self -> kfg == NULL )
                rc = KConfigMake( (KConfig**) &(self -> kfg), NULL );
            if ( rc == 0 )
            {
                rc_t rc2;
                const KRepositoryMgr *rmgr;
                rc = KConfigMakeRepositoryMgrRead ( self -> kfg, & rmgr );
                if ( rc == 0 )
                {
                    const KRepository *protected;
                    if (by_project_id) {
                        rc = KRepositoryMgrGetProtectedRepository
                            ( rmgr, projectId, & protected );
                    }
                    else {
                        rc = KRepositoryMgrCurrentProtectedRepository
                            ( rmgr, & protected );
                    }
                    if ( rc == 0 )
                    {   /* in a protected area */
                        char path [ MAX_PATH_SIZE ];
                        rc = KRepositoryEncryptionKeyFile ( protected, path, sizeof(path), NULL );
                        if ( rc == 0 && path [ 0 ] != 0 )
                            rc = KEncryptionKeyMakeFromFile(path, enc_key);
                        else {
                            rc = KRepositoryEncryptionKey ( protected,
                                path, sizeof(path), NULL );
                            if ( rc == 0 )
                                rc = KEncryptionKeyMake ( path, enc_key );
                        }
                            
                        rc2 = KRepositoryRelease ( protected );
                        if (rc == 0)
                            rc = rc2;
                    }
                    else /* use global password file */
                    {
                        String* path;
                        rc = KConfigReadString ( self -> kfg, KFG_KRYPTO_PWFILE, &path );        
                        if (rc == 0)
                        {
                            rc = KEncryptionKeyMakeFromFile(path->addr, enc_key);
                            StringWhack(path);
                        }
                        else 
                            rc = RC(rcKFG, rcEncryptionKey, rcRetrieving, rcItem, rcNotFound); 
                    }
                    rc2 = KRepositoryMgrRelease ( rmgr );
                    if (rc == 0)
                        rc = rc2;
                }
            }
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC KKeyStoreGetKey(const KKeyStore* self,
    const char* obj_key, KEncryptionKey** enc_key)
{
    return KKeyStoreGetKeyInt(self, obj_key, enc_key, false, 0);
}

LIB_EXPORT rc_t CC KKeyStoreGetKeyByProjectId(const KKeyStore* self,
    const char* obj_key, KEncryptionKey** enc_key, uint32_t projectId)
{
    return KKeyStoreGetKeyInt(self, obj_key, enc_key, true, projectId);
}


LIB_EXPORT rc_t CC KKeyStoreSetConfig(struct KKeyStore* self, const struct KConfig* kfg)
{
    rc_t rc = 0;
    if ( self == NULL )
        return RC ( rcKFG, rcEncryptionKey, rcRetrieving, rcSelf, rcNull );
        
    if ( kfg != NULL )
        rc = KConfigAddRef(kfg);
        
    if ( rc == 0 )
    {
        if ( self->kfg != NULL )
            KConfigRelease(self->kfg);
        self->kfg = kfg;
    }
        
    return rc;
}

/* 
 * KEncryptionKey
 */

LIB_EXPORT rc_t CC KEncryptionKeyAddRef ( KEncryptionKey *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAdd ( & self -> refcount, "KEncryptionKey" ) )
        {
        case krefLimit:
            return RC ( rcKFG, rcEncryptionKey, rcAttaching, rcRange, rcExcessive );
        }
    }
    return 0;
}

LIB_EXPORT rc_t CC KEncryptionKeyRelease ( KEncryptionKey *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDrop ( & self -> refcount, "KEncryptionKey" ) )
        {
        case krefWhack:
            free((char*)self->value.addr); /*TODO: unlock memory? */
            free(self);
            break;
        case krefLimit:
            return RC ( rcKFG, rcEncryptionKey, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
}

static rc_t KEncryptionKeyMakeInt(const char* value, KEncryptionKey** self)
{
    KEncryptionKey* ret;
    
    *self = NULL;
    
    ret = malloc(sizeof(KEncryptionKey));
    if (ret == NULL)
        return RC ( rcKFG, rcEncryptionKey, rcCreating, rcMemory, rcExhausted );
    else
    {
/* VDB-3590: Encryption key is a sequence of bytes.
        It is not a string and can represent an invalid UNICODE sequence */
        size_t size = string_size   (value);

        char* data = malloc(size+1);/*TODO: place in protected memory*/
        if (data == NULL)
        {
            free(ret);
            return RC ( rcKFG, rcEncryptionKey, rcCreating, rcMemory, rcExhausted );
        }

        memmove(data, value, size);    

        StringInit( & ret -> value, data, size, (uint32_t)size ); /* do not include the 0-terminator */
        
        KRefcountInit ( & ret -> refcount, 1, "KEncryptionKey", "init", "" );
        
        *self = ret;
        return 0;
    }
}

LIB_EXPORT rc_t CC KEncryptionKeyMake(const char* value, KEncryptionKey** self)
{
    if ( value == NULL )
        return RC ( rcKFG, rcEncryptionKey, rcCreating, rcParam, rcNull );
    if ( self == NULL )
        return RC ( rcKFG, rcEncryptionKey, rcCreating, rcSelf, rcNull );

    return KEncryptionKeyMakeInt(value, self);
}

static rc_t KEncryptionKeyMakeFromFile(const char* path, KEncryptionKey** self)
{
    rc_t rc;
    KDirectory* wd;
    
    if ( path == NULL )
        return RC ( rcKFG, rcEncryptionKey, rcCreating, rcParam, rcNull );
    if ( self == NULL )
        return RC ( rcKFG, rcEncryptionKey, rcCreating, rcSelf, rcNull );
    
    rc = KDirectoryNativeDir(&wd);
    if (rc == 0)
    {
        rc_t rc2;
        const KFile* file;
        const char dev_stdin [] = "/dev/stdin";
        const char dev_null [] = "/dev/null";        
        
        if (strcmp (dev_stdin, path) == 0)
            rc = KFileMakeStdIn (&file);
        else if (strcmp (dev_null, path) == 0)
            rc = KFileMakeNullRead (&file);
        else 
            rc = KDirectoryOpenFileRead(wd, &file, "%s", path);
        if (rc == 0)
        {
            char key[PASSWORD_MAX_SIZE + 1];
            rc = ReadEncKey(file, key, PASSWORD_MAX_SIZE);
            if (rc == 0)
                rc = KEncryptionKeyMakeInt(key, self);
                
            rc2 = KFileRelease(file);
            if (rc == 0)
                rc = rc2;
        }
        rc2 = KDirectoryRelease(wd);
        if (rc == 0)
            rc = rc2;
    }
    return rc;
}

LIB_EXPORT rc_t CC KKeyStoreSetBindingsFile(struct KKeyStore* self, const char* path)
{
    if ( self == NULL )
        return RC ( rcKFG, rcStorage, rcInitializing, rcSelf, rcNull );
        
    if (self->bindingsFile != defaultBindingsFile)
        free(self->bindingsFile);
    if (path == NULL)
        self->bindingsFile = NULL;
    else
    {
        self->bindingsFile = string_dup(path, string_size(path));
        if (self->bindingsFile == NULL)
            return RC ( rcKFG, rcStorage, rcInitializing, rcMemory, rcExhausted );
    }
    return 0;
}

LIB_EXPORT const char* KKeyStoreGetBindingsFile(const struct KKeyStore* self)
{
    if ( self == NULL )
        return NULL;
        
    return self->bindingsFile;
}


#define MAX_OBJID_SIZE 20
#define MAX_NAME_SIZE 4096

static rc_t LocateObjectId(const KFile* file, const char* key, char* value, size_t value_size, size_t* value_read)
{
    char buf[MAX_OBJID_SIZE + 1 + MAX_NAME_SIZE + 1];
    size_t num_read;
    uint64_t pos = 0;
    size_t key_size = string_size(key);
    bool eof = false;
    do
    {
        rc_t rc = KFileReadAll(file, pos, buf, sizeof(buf), &num_read);
        if (rc != 0)
            return rc;
        else
        {
            size_t start;
            if (num_read == sizeof(buf))
            {   /* locate the last incomplete line and ignore it; will be considered after the next read() */
                while (num_read > 0)
                {
                    --num_read;
                    if (buf[num_read] == '\r' || buf[num_read] == '\n')
                        break;
                }
                if (num_read == 0)
                    return RC ( rcVFS, rcMgr, rcReading, rcRow, rcTooLong);
            }
            else
                eof = true;
            
            start = 0;
            while (start + key_size < num_read)
            {   
                /* find the beginning of a line */
                while (buf[start] == '\r' || buf[start] == '\n') 
                {
                    ++start;
                    if (start >= num_read)
                        break;
                }
                /* check the key */
                if (string_cmp(key, key_size, buf + start, key_size, (uint32_t) ( num_read - start )) == 0)
                {
                    if (buf [ start + key_size ] == '=')
                    {
                        size_t i = 0;
                        size_t sourceIdx = start + key_size + 1; /* 1st character after '=' */
                        while (sourceIdx < num_read)
                        {
                            if (i >= value_size)
                                return RC ( rcVFS, rcMgr, rcReading, rcName, rcTooLong);
                                
                            if (buf[sourceIdx] == '\r' || buf[sourceIdx] == '\n') 
                                break;
                                
                            value[i] = buf[sourceIdx];
                            
                            ++i;
                            ++sourceIdx;
                        }
                        *value_read = i;
                        return 0;
                    }
                }
                
                /* skip until the next end of line */
                while (buf[start] != '\r' && buf[start] != '\n') 
                {
                    ++start;
                    if (start >= num_read)
                        break;
                }
            }
            
            pos += num_read;
        }
    }
    while (!eof);
    
    return RC ( rcVFS, rcMgr, rcReading, rcId, rcNotFound );
}

/*
 * Reverse lookup
 */
static rc_t LocateObject(const KFile* file, const char* value, const size_t value_size, char* key, size_t key_size, size_t* key_read)
{
    char buf[MAX_OBJID_SIZE + 1 + MAX_NAME_SIZE + 1];
    size_t num_read;
    uint64_t pos = 0;
    bool eof = false;
    do
    {
        rc_t rc = KFileReadAll(file, pos, buf, sizeof(buf), &num_read);
        if (rc != 0)
            return rc;
        else
        {
            size_t start;
            if (num_read == sizeof(buf))
            {   /* locate the last incomplete line and ignore it; will be considered after the next read() */
                while (num_read > 0)
                {
                    --num_read;
                    if (buf[num_read] == '\r' || buf[num_read] == '\n')
                        break;
                }
                if (num_read == 0)
                    return RC ( rcVFS, rcMgr, rcReading, rcRow, rcTooLong);
            }
            else
                eof = true;
            
            start = 0; 
            while (start + value_size < num_read)
            {   
                size_t key_start, key_end;
                size_t value_start;
                
                /* find the beginning of a line */
                while (buf[start] == '\r' || buf[start] == '\n') 
                {
                    ++start;
                    if (start >= num_read)
                        break;
                }
                if (start >= num_read)
                    continue;
                    
                key_start = start;
                
                /* find the separator */
                while (buf[start] != '=') 
                {
                    if (buf[start] == '\r' || buf[start] == '\n') /* separator not found, ignore the line*/
                        break;
                        
                    ++start;
                    if (start >= num_read)
                        break;
                }
                if (start >= num_read || buf[start] != '=')
                    continue;
                
                key_end = start;
                
                ++start; /* skip the separator */

                value_start = start;
                /* find the next end of line */
                while (buf[start] != '\r' && buf[start] != '\n') 
                {
                    ++start;
                    if (start >= num_read)
                        break;
                }

                /* compare the value */
                if (string_cmp(value, value_size, buf + value_start, start - value_start, (uint32_t) ( start - value_start )) == 0)
                {
                    *key_read = key_end - key_start;
                    if (string_copy(key, key_size, buf + key_start, *key_read) != *key_read)
                        return RC ( rcVFS, rcMgr, rcReading, rcId, rcTooLong);
                    return 0;
                }
            }
            
            pos += num_read;
        }
    }
    while (!eof);
    
    return RC ( rcVFS, rcMgr, rcReading, rcId, rcNotFound );
}

static rc_t AppendObject(KFile* file, const char* name, const String* value)
{
    uint64_t size;
    rc_t rc = KFileSize ( file, &size );
    if (rc == 0)
    {
        char buf[MAX_OBJID_SIZE + 1 + MAX_NAME_SIZE + 1];
        size_t num_writ;
        rc = string_printf(buf, sizeof(buf), &num_writ, "%s=%S\n", name, value);
        if (rc == 0)
            rc = KFileWrite ( file, size, buf, num_writ, &num_writ );
    }
    return rc;
}

static rc_t OpenBindingsFile(KKeyStore* self, const KFile** ret)
{
    rc_t rc = 0;
    if (self->bindingsFile == NULL)
    {
        String* home;
        if ( self->kfg == NULL )
            rc = KConfigMake( (KConfig**) &(self -> kfg), NULL );
        if (rc == 0)
        {
            if (KConfigReadString(self->kfg, "NCBI_HOME", &home) == 0 ||
                KConfigReadString(self->kfg, "HOME", &home) == 0 ||
                KConfigReadString(self->kfg, "USERPROFILE", &home) == 0 )
            {
                size_t num_writ;
                rc = string_printf(defaultBindingsFile, sizeof(defaultBindingsFile), &num_writ, "%S/objid.mapping", home);
                if (rc == 0)
                    self->bindingsFile = defaultBindingsFile;
                StringWhack(home);
            }
            else
                rc = RC ( rcVFS, rcMgr, rcOpening, rcEnvironment, rcNull );
        }
    }
    
    if (rc == 0)
    {
        rc_t rc2;
        KDirectory* wd;
        rc = KDirectoryNativeDir (&wd);
        if (rc == 0)
            rc = KDirectoryOpenFileRead(wd, ret, "%s", self->bindingsFile);
        else
            *ret = NULL;
            
        rc2 = KDirectoryRelease(wd);
        if (rc == 0)
            rc = rc2;
    }
    return rc;
}
 
LIB_EXPORT rc_t CC KKeyStoreRegisterObject(struct KKeyStore* self, uint32_t oid, const struct String* newName)
{
    rc_t rc = 0;
    if ( self == NULL )
        return RC ( rcKFG, rcStorage, rcInitializing, rcSelf, rcNull );
    else if ( newName == NULL )
        rc = RC ( rcKFG, rcStorage, rcInitializing, rcParam, rcNull );
    else
    {
        char oidString[MAX_OBJID_SIZE];
        size_t num_writ;
        rc = string_printf(oidString, sizeof(oidString), &num_writ, "%d", oid);
        if (rc == 0)
        {
            rc_t rc2;
            const KFile* file;
            char oldName[MAX_NAME_SIZE];
            size_t num_read;
            
            rc = OpenBindingsFile(self, &file);
            if (rc == 0)
            {   /* see if already registered */
                if (LocateObjectId(file, oidString, oldName, sizeof(oldName), &num_read) == 0 &&
                    string_cmp(oldName, num_read, newName->addr, newName->size, (uint32_t)newName->size) != 0)
                    rc = RC ( rcVFS, rcMgr, rcRegistering, rcId, rcExists );
                rc2 = KFileRelease(file);
                if (rc == 0)
                    rc = rc2;
            }
            else /* no bindings file; will create */
                rc = 0;
            
            if (rc == 0)
            {   /* not registered; lock the bindings file and append the binding */
                KDirectory* wd;
                rc = KDirectoryNativeDir (&wd);
                if (rc == 0)
                {
                    KFile* lockedFile;
                    /*TODO: loop until locking succeeds or times out */
                    rc = KDirectoryCreateExclusiveAccessFile(wd, &lockedFile, true, 0600, kcmOpen, "%s", self->bindingsFile);
                    if (rc == 0)
                    {
                        if (LocateObjectId(lockedFile, oidString, oldName, sizeof(oldName), &num_read) == 0)
                        {   /* somebody inserted the binding before we locked - make sure their binding was the same */
                            if (string_cmp(oldName, num_read, newName->addr, newName->size, (uint32_t)newName->size) != 0)
                                rc = RC ( rcVFS, rcMgr, rcRegistering, rcId, rcExists );
                        }
                        else
                            rc = AppendObject(lockedFile, oidString, newName);
                            
                        rc2 = KFileRelease(lockedFile);
                        if (rc == 0)
                            rc = rc2;
                    }
                    rc2 = KDirectoryRelease(wd);
                    if (rc == 0)
                        rc = rc2;
                }
            }
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC VKKeyStoreGetObjectId(const struct KKeyStore* self, const struct String* name, uint32_t* oid)
{
    rc_t rc = 0;
    if ( self == NULL )
        return RC ( rcKFG, rcStorage, rcRetrieving, rcSelf, rcNull );
    else if ( oid == NULL )
        rc = RC ( rcKFG, rcStorage, rcRetrieving, rcParam, rcNull );
    else
    {
        const KFile* file;
        rc = OpenBindingsFile((KKeyStore*)self, &file);
        if (rc == 0)
        {
            rc_t rc2;
            char id_buf[MAX_OBJID_SIZE+1];
            size_t id_read;
            if (LocateObject(file, name->addr, name->size, id_buf, MAX_OBJID_SIZE, &id_read) == 0)
            {
                char* endptr;
                unsigned long int res;
                id_buf[id_read] = 0;
                res = strtou32(id_buf, &endptr, 10);
                if (*endptr != 0)
                    rc = RC ( rcKFG, rcStorage, rcRetrieving, rcId, rcCorrupt);
                else
                    *oid = (uint32_t)res;
            }
            else
                rc = RC ( rcKFG, rcStorage, rcRetrieving, rcName, rcNotFound);
            
            rc2 = KFileRelease(file);
            if (rc == 0)
                rc = rc2;
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC KKeyStoreGetObjectName(const struct KKeyStore* self, uint32_t oid, const struct String** result)
{
    rc_t rc = 0;
    if ( self == NULL )
        return RC ( rcKFG, rcStorage, rcRetrieving, rcSelf, rcNull );
    else if ( result == NULL )
        rc = RC ( rcKFG, rcStorage, rcRetrieving, rcParam, rcNull );
    else
    {
        char oidString[MAX_OBJID_SIZE];
        size_t num_writ;
        rc = string_printf(oidString, sizeof(oidString), &num_writ, "%d", oid);
        if (rc == 0)
        {
            const KFile* file;
            
            rc = OpenBindingsFile((KKeyStore*)self, &file);
            if (rc == 0)
            {   
                rc_t rc2;
                char name[MAX_NAME_SIZE];
                size_t num_read;
                if (LocateObjectId(file, oidString, name, sizeof(name), &num_read) == 0)
                {
                    String* res = (String*)malloc(sizeof(String) + num_read);
                    if (res != NULL)
                    {
                        string_copy((char*)res + sizeof(String), num_read, name, num_read);
                        StringInit(res, (char*)res + sizeof(String), num_read, (uint32_t)num_read);
                        *result = res;
                    }
                    else
                        rc = RC ( rcKFG, rcStorage, rcRetrieving, rcMemory, rcExhausted );
                }
                else
                    rc = RC ( rcKFG, rcStorage, rcRetrieving, rcId, rcNotFound);
                    
                rc2 = KFileRelease(file);
                if (rc == 0)
                    rc = rc2;
            }
        }
    }
    return rc;
}

