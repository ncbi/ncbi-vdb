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

#ifndef _h_kfg_config_
#define _h_kfg_config_

#ifndef _h_kfg_extern_
#include <kfg/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * This flag has important ramifications to all programs.
 * If set then all programs that use KMain()/KMane() 
 * become dependant on the kfg and kfs libraries.
 *
 * This will also modify the behavior of program tear down.
 * if unset the memory used by the singleton config manager
 * will not be freed.
 */
#define KFG_COMMON_CREATION 0


/*--------------------------------------------------------------------------
 * forwards
 */
struct KFile;
struct KDirectory;
struct KNamelist;
struct VPath;
struct String;

/*--------------------------------------------------------------------------
 * KConfig
 *  configuration paramter manager
 */
typedef struct KConfig KConfig;


/* Make
 *  create a process-global configuration manager
 *
 *  "cfg" [ OUT ] - return parameter for mgr
 *
 *  "optional_search_base" [ IN, NULL OKAY ]
 */
KFG_EXTERN rc_t CC KConfigMake ( KConfig **cfg,
    struct KDirectory const * optional_search_base );

/* AddRef
 * Release
 */
KFG_EXTERN rc_t CC KConfigAddRef ( const KConfig *self );
KFG_EXTERN rc_t CC KConfigRelease ( const KConfig *self );


/* LoadFile
 * loads a configuration file
 */
KFG_EXTERN rc_t CC KConfigLoadFile ( KConfig * self,
    const char * path, struct KFile const * file );


/* Commit
 *  Commits changes to user's private configuration file
 *
 *  Respects DisabledUserSettings(do not update default user settings)
 */
KFG_EXTERN rc_t CC KConfigCommit ( KConfig *self );

/* Read
 *  read a node value
 *
 *  "offset" [ IN ] - initial offset into configuration
 *
 *  "buffer" [ OUT ] and "bsize" [ IN ] - return buffer for read
 *
 *  "num_read" [ OUT ] - number of bytes actually read
 *
 *  "remaining" [ OUT, NULL OKAY ] - optional return parameter for
 *  the number of bytes remaining to be read.
 *  specifically, "offset" + "num_read" + "remaining" == sizeof node data
 */
KFG_EXTERN rc_t CC KConfigRead ( const KConfig *self, const char *path,
    size_t offset, char *buffer, size_t bsize,
    size_t *num_read, size_t *remaining );

/* ReadBool
 *  read a boolean node value
 *
 * self [ IN ] - KConfig object
 * path [ IN ] - path to the node
 * result [ OUT ] - return value (true if "TRUE", false if "FALSE"; rc != 0 if neither)
 *
 */
KFG_EXTERN rc_t CC KConfigReadBool ( const KConfig* self, const char* path, bool* result );
KFG_EXTERN rc_t CC KConfigWriteBool( KConfig *self, const char * path, bool value );
    
/* ReadI64
 *  read an integer node value
 *
 * self [ IN ] - KConfig object
 * path [ IN ] - path to the node
 * result [ OUT ] - return value (rc != 0 if cannot be converted)
 *
 */
KFG_EXTERN rc_t CC KConfigReadI64 ( const KConfig* self, const char* path, int64_t* result );

/* ReadU64
 *  read an unsigned node value
 *
 * self [ IN ] - KConfig object
 * path [ IN ] - path to the node
 * result [ OUT ] - return value (rc != 0 if cannot be converted)
 *
 */
KFG_EXTERN rc_t CC KConfigReadU64 ( const KConfig* self, const char* path, uint64_t* result );

/* ReadF64
 *  read an F64 node value
 *
 * self [ IN ] - KConfig object
 * path [ IN ] - path to the node
 * result [ OUT ] - return value (rc != 0 if cannot be converted)
 *
 */
KFG_EXTERN rc_t CC KConfigReadF64( const KConfig* self, const char* path, double* result );

#if 0
/*** NB - temporarily lives in vfs due to library interdependencies ***/

/* ReadVPath
 *  read a VPath node value
 *
 * self [ IN ] - KConfig object
 * path [ IN ] - path to the node
 * result [ OUT ] - return value (rc != 0 if cannot be converted)
 *
 */
KFG_EXTERN rc_t CC KConfigReadVPath ( const KConfig* self, const char* path, struct VPath** result );
#endif

/* ReadString
 *  read a String node value
 *
 * self [ IN ] - KConfig object
 * path [ IN ] - path to the node
 * result [ OUT, NUL-TERMINATED ] - return value; caller responsible for deallocation
 *
 */
KFG_EXTERN rc_t CC KConfigReadString ( const KConfig* self, const char* path, struct String** result );
KFG_EXTERN rc_t CC KConfigWriteString( KConfig *self, const char * path, const char * value );
KFG_EXTERN rc_t CC KConfigWriteSString( KConfig *self, const char * path, struct String const * value );

/* Print
 *  print configuration to output handler (using OUTMSG)
 */
KFG_EXTERN rc_t CC KConfigPrint ( const KConfig * self, int indent );

KFG_EXTERN rc_t CC KConfigToFile ( const KConfig * self, struct KFile * file );

/* DisableUserSettings
 *  for testing purposes
 */
KFG_EXTERN void CC KConfigDisableUserSettings ( void );

/* SetNgcFile
 */
KFG_EXTERN void CC KConfigSetNgcFile(const char * path);

/*--------------------------------------------------------------------------
 * KConfigNode
 *  node within configuration tree
 */
typedef struct KConfigNode KConfigNode;


/* AddRef
 * Release
 *  all objects are reference counted
 *  NULL references are ignored
 */
KFG_EXTERN rc_t CC KConfigNodeAddRef ( const KConfigNode *self );
KFG_EXTERN rc_t CC KConfigNodeRelease ( const KConfigNode *self );


KFG_EXTERN rc_t CC KConfigNodeGetMgr( const KConfigNode * self, KConfig ** mgr );

/* OpenNodeRead
 * VOpenNodeRead
 *  opens a configuration node
 *
 *  "node" [ OUT ] - return parameter for indicated configuration node
 *
 *  "path" [ IN, NULL OKAY ] - optional path for specifying named
 *  node within configuration hierarchy. paths will be interpreted as
 *  if they were file system paths, using '/' as separator. the
 *  special values NULL and "" are interpreted as "."
 */
KFG_EXTERN rc_t CC KConfigOpenNodeRead ( const KConfig *self,
    const KConfigNode **node, const char *path, ... );
KFG_EXTERN rc_t CC KConfigNodeOpenNodeRead ( const KConfigNode *self,
    const KConfigNode **node, const char *path, ... );

KFG_EXTERN rc_t CC KConfigVOpenNodeRead ( const KConfig *self,
    const KConfigNode **node, const char *path, va_list args );
KFG_EXTERN rc_t CC KConfigNodeVOpenNodeRead ( const KConfigNode *self,
    const KConfigNode **node, const char *path, va_list args );


/* OpenNodeUpdate
 * VOpenNodeUpdate
 *  opens a configuration node
 *
 *  "node" [ OUT ] - return parameter for indicated configuration node
 *
 *  "path" [ IN, NULL OKAY ] - optional path for specifying named
 *  node within configuration hierarchy. paths will be interpreted as
 *  if they were file system paths, using '/' as separator. the
 *  special values NULL and "" are interpreted as "."
 */
KFG_EXTERN rc_t CC KConfigOpenNodeUpdate ( KConfig *self,
    KConfigNode **node, const char *path, ... );
KFG_EXTERN rc_t CC KConfigNodeOpenNodeUpdate ( KConfigNode *self,
    KConfigNode **node, const char *path, ... );

KFG_EXTERN rc_t CC KConfigVOpenNodeUpdate ( KConfig *self,
    KConfigNode **node, const char *path, va_list args );
KFG_EXTERN rc_t CC KConfigNodeVOpenNodeUpdate ( KConfigNode *self,
    KConfigNode **node, const char *path, va_list args );


/* Read
 *  read a node value
 *
 *  "offset" [ IN ] - initial offset into configuration
 *
 *  "buffer" [ OUT ] and "bsize" [ IN ] - return buffer for read
 *
 *  "num_read" [ OUT ] - number of bytes actually read
 *
 *  "remaining" [ OUT, NULL OKAY ] - optional return parameter for
 *  the number of bytes remaining to be read.
 *  specifically, "offset" + "num_read" + "remaining" == sizeof node data
 */
KFG_EXTERN rc_t CC KConfigNodeRead ( const KConfigNode *self,
    size_t offset, char *buffer, size_t bsize,
    size_t *num_read, size_t *remaining );

/* ReadBool
 *  read a boolean node value
 *
 * self [ IN ] - KConfigNode object
 * result [ OUT ] - return value (true if "TRUE", false if "FALSE"; rc != 0 if neither)
 *
 */
KFG_EXTERN rc_t CC KConfigNodeReadBool ( const KConfigNode *self, bool* result );

    
/* ReadI64
 *  read an integer node value
 *
 * self [ IN ] - KConfigNode object
 * result [ OUT ] - return value (rc != 0 if cannot be converted)
 *
 */
KFG_EXTERN rc_t CC KConfigNodeReadI64 ( const KConfigNode *self, int64_t* result );

/* ReadU64
 *  read an unsigned node value
 *
 * self [ IN ] - KConfigNode object
 * result [ OUT ] - return value (rc != 0 if cannot be converted)
 *
 */
KFG_EXTERN rc_t CC KConfigNodeReadU64 ( const KConfigNode *self, uint64_t* result );

/* ReadF64
 *  read an F64 node value
 *
 * self [ IN ] - KConfigNode object
 * result [ OUT ] - return value (rc != 0 if cannot be converted)
 *
 */
KFG_EXTERN rc_t CC KConfigNodeReadF64 ( const KConfigNode *self, double* result );

#if 0
/*** NB - temporarily lives in vfs due to library interdependencies ***/

/* ReadVPath
 *  read a VPath node value
 *
 * self [ IN ] - KConfigNode object
 * result [ OUT ] - return value (rc != 0 if cannot be converted)
 *
 */
KFG_EXTERN rc_t CC KConfigNodeReadVPath ( const KConfigNode *self, struct VPath** result );
#endif

/* ReadString
 *  read a String node value
 *
 * self [ IN ] - KConfigNode object
 * result [ OUT ] - return value; caller responsible for deallocation
 *
 */
KFG_EXTERN rc_t CC KConfigNodeReadString ( const KConfigNode *self, struct String** result );

/* ListChildren - nee ListChild
 *  list all named children
 */
#define KConfigNodeListChild KConfigNodeListChildren
KFG_EXTERN rc_t CC KConfigNodeListChildren ( const KConfigNode *self,
    struct KNamelist **names );


/* Write
 *  write a node value or attribute
 *  overwrites anything already there
 *
 *  "buffer" [ IN ] and "size" [ IN ] - new value data
 */
KFG_EXTERN rc_t CC KConfigNodeWrite ( KConfigNode *self, const char *buffer, size_t size );


/* Write Boolean
 *  write a boolean value ( literally the text "true" or "false:
 *  overwrites anything already there
 *
 *  "state" [ IN ] - new value
 */
KFG_EXTERN rc_t CC KConfigNodeWriteBool ( KConfigNode *self, bool state );


/* Append
 *  append data to value
 *
 *  "buffer" [ IN ] and "size" [ IN ] - value data to be appended
 */
KFG_EXTERN rc_t CC KConfigNodeAppend ( KConfigNode *self, const char *buffer, size_t size );


/* ReadAttr
 *  reads as NUL-terminated string
 *
 *  "name" [ IN ] - NUL terminated attribute name
 *
 *  "buffer" [ OUT ] and "bsize" - return parameter for attribute value
 *
 *  "size" [ OUT ] - return parameter giving size of string
 *  not including NUL byte. the size is set both upon success
 *  and insufficient buffer space error.
 */
KFG_EXTERN rc_t CC KConfigNodeReadAttr ( const KConfigNode *self, const char *name,
    char *buffer, size_t bsize, size_t *size );


/* WriteAttr
 *  writes NUL-terminated string
 *
 *  "name" [ IN ] - NUL terminated attribute name
 *
 *  "value" [ IN ] - NUL terminated attribute value
 */
KFG_EXTERN rc_t CC KConfigNodeWriteAttr ( KConfigNode *self,
    const char *name, const char *value );


/* Drop
 * VDrop
 *  drop some or all node content
 */
KFG_EXTERN rc_t CC KConfigNodeDropAll ( KConfigNode *self );
KFG_EXTERN rc_t CC KConfigNodeDropAttr ( KConfigNode *self, const char *attr );
KFG_EXTERN rc_t CC KConfigNodeDropChild ( KConfigNode *self, const char *path, ... );
KFG_EXTERN rc_t CC KConfigNodeVDropChild ( KConfigNode *self, const char *path, va_list args );


/* Rename
 *  renames a contained object
 *
 *  "from" [ IN ] - NUL terminated string in UTF-8
 *  giving simple name of existing attr
 *
 *  "to" [ IN ] - NUL terminated string in UTF-8
 *  giving new simple attr name
 */
KFG_EXTERN rc_t CC KConfigNodeRenameAttr ( KConfigNode *self, const char *from, const char *to );
KFG_EXTERN rc_t CC KConfigNodeRenameChild ( KConfigNode *self, const char *from, const char *to );


#ifdef __cplusplus
}
#endif

#endif /* _h_kfg_config_ */
