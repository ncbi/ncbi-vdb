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

#ifndef _h_sra_sradb_priv_
#define _h_sra_sradb_priv_

#ifndef _h_klib_container_
#include <klib/container.h>
#endif

#ifndef _h_klib_text
#include <klib/text.h>
#endif

#ifndef _h_klib_vector
#include <klib/vector.h>
#endif

#ifndef _h_sra_sradb_
#include <sra/sradb.h>
#endif

#ifndef _h_sra_srapath_
#include <sra/srapath.h>
#endif

#ifndef _h_sra_path_extern_
#include <sra/path-extern.h>
#endif

#ifndef _h_sra_sch_extern_
#include <sra/sch-extern.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct KFile;
struct KDirectory;
struct KDBManager;
struct KTable;
struct KLock;
struct KConfig;
struct VDBManager;
struct VTable;
struct VSchema;
struct SRAPath;
struct SRACacheUsage;
struct SRACacheMetrics;
/*--------------------------------------------------------------------------
 * SRAMgr
 *  opaque handle to SRA library
 */
 
/*  SRAMgrResolve
 *  Convert accession name into a file system path
 */
 
SRA_EXTERN rc_t CC SRAMgrResolve( const SRAMgr *self, const char* acc, char* buf, size_t buf_size );

/* Flush
 *  flushes least recently used accessions until the cache size is under the specified threshold
 */
SRA_EXTERN rc_t CC SRAMgrFlush ( struct SRAMgr const *self, const struct SRACacheMetrics* );

/* RunBGTasks
 *  perform single pass of garbage collection tasks and exit.
 *  also retrieves and processes update messages.
 */
SRA_EXTERN rc_t CC SRAMgrRunBGTasks ( struct SRAMgr const *self );

/* GetVDBManager
 *  returns a new reference to VDBManager used by SRAMgr
 */
SRA_EXTERN rc_t CC SRAMgrGetVDBManagerRead ( const SRAMgr *self, struct VDBManager const **vmgr );
SRA_EXTERN rc_t CC SRAMgrGetVDBManagerUpdate ( SRAMgr *self, struct VDBManager **vmgr );

/* GetKDBManager
 *  returns a new reference to KDBManager used indirectly by SRAMgr
 */
SRA_EXTERN rc_t CC SRAMgrGetKDBManagerRead ( const SRAMgr *self, struct KDBManager const **kmgr );
SRA_EXTERN rc_t CC SRAMgrGetKDBManagerUpdate ( SRAMgr *self, struct KDBManager **kmgr );

/* ModDate
 *  return a modification timestamp for table
 */
SRA_EXTERN rc_t CC SRAMgrVGetTableModDate ( const SRAMgr *self,
    KTime_t *mtime, const char *spec, va_list args );

SRA_EXTERN rc_t CC SRAMgrGetTableModDate ( const SRAMgr *self,
    KTime_t *mtime, const char *spec, ... );

/* ConfigReload
 *  update SRAPath object
 */
SRA_EXTERN rc_t CC SRAMgrConfigReload( const SRAMgr *self, struct KDirectory const *wd );

/* 
 *  Accession Cache usage stats
 */
SRA_EXTERN rc_t CC SRAMgrGetCacheUsage( const SRAMgr *self, struct SRACacheUsage* stats );

/* 
 *  Configure Accession Cache 
 *  soft_threshold, hard_threshold - new threshold values ( -1 : do not change; < -1 invalid )
 */
SRA_EXTERN rc_t CC SRAMgrConfigureCache( const SRAMgr *self,  int32_t soft_threshold, int32_t hard_threshold );

/*--------------------------------------------------------------------------
 * SRATable
 */

/* OpenAltTableRead
 *  opens a table within a database structure with a specific name
 */
SRA_EXTERN rc_t CC SRAMgrOpenAltTableRead ( const SRAMgr *self,
    const SRATable **tbl, const char *altname, const char *spec, ... );

/* GetVTable
 *  returns a new reference to underlying VTable
 */
SRA_EXTERN rc_t CC SRATableGetVTableRead ( const SRATable *self, struct VTable const **vtbl );
SRA_EXTERN rc_t CC SRATableGetVTableUpdate ( SRATable *self, struct VTable **vtbl );

/* GetKTable
 *  returns a new reference to underlying KTable
 */
SRA_EXTERN rc_t CC SRATableGetKTableRead ( const SRATable *self, struct KTable const **ktbl );
SRA_EXTERN rc_t CC SRATableGetKTableUpdate ( SRATable *self, struct KTable **ktbl );


/* MakeSingleFileArchive
 *  makes a single-file-archive file from an SRA table
 *
 *  contents are ordered by frequency and necessity of access
 *
 *  "lightweight" [ IN ] - when true, include only those components
 *  required for read and quality operations.
 *
 *  "ext" [OUT,NULL] - optional file name extension to use for file
 */
SRA_EXTERN rc_t CC SRATableMakeSingleFileArchive ( const SRATable *self,
    struct KFile const **sfa, bool lightweight, const char** ext );

/* SingleFileArchiveExt
 *  retrieve archive extension based on object in the spec
 */
SRA_EXTERN rc_t CC SRAMgrSingleFileArchiveExt(const SRAMgr *self,
    const char* spec, const bool lightweight, const char** ext);
/*--------------------------------------------------------------------------
 * SRAPath
 */

/* FindWithRepLen
 *  finds location of run within rep-server/volume matrix
 *  returns length of rep-server portion
 */
SRA_EXTERN rc_t CC SRAPathFindWithRepLen ( struct SRAPath const *self,
    const char *accession, char *path, size_t path_max, size_t *rep_len );



/*--------------------------------------------------------------------------
 * SRASchema - DEPRECATED
 */
SRA_SCH_EXTERN rc_t CC SRASchemaMake ( struct VSchema **schema, struct VDBManager const *mgr );


#if 0

/*--------------------------------------------------------------------------
 * SRATableData  - DEPRECATED
 *  a collection of spots with several data series, minimally including
 *  base or color calls and their quality ( confidence ) values, and
 *  optionally signal-related values ( signal, intensity, noise, ... ).
 */
union NucStrstr;

typedef struct SRASpotStructure SRASpotStructure;
struct SRASpotStructure
{ 
    /* preformatted query expression
       for fixed_seq when search is needed */
    union NucStrstr *q_str;

    /* read of fixed len if != 0
       either teminated by fixed_seq or by the end */
    uint16_t fixed_len;

    /* SRAReadTypes */
	uint8_t read_type;

    /* colorspace key */
    char cs_key;

    char fixed_seq [ 1024 ];

    /* label for the read */
    char read_label [ 54 ];

};

typedef struct SRASpotCoord SRASpotCoord;
struct SRASpotCoord
{
    uint32_t x, y, tile;
	uint32_t  lane;
	spotid_t id;

    /* prefix part of spotname */
	uint32_t platename_len;
	char spotname [ 1024 ];

};

typedef struct SRATableData SRATableData;
struct SRATableData
{
	uint64_t base_count;
	uint64_t spot_count;
	uint64_t bad_spot_count;
	spotid_t max_spotid;

    /* the spot is always fixed len read */
	uint32_t fixed_len;

    /* number of reads per spot */
	uint32_t num_reads;

    /* read mask containing bio reads */
	uint32_t read_mask_bio;

    /* read description */
	SRASpotStructure read_descr [ 32 ];
    uint16_t read_len [ 32 ];

    /* platform type and name */
	uint8_t platform;
	char platform_str [ 31 ];

	uint16_t prefix_len;

    /* spot coordinates */
	SRASpotCoord coord;

};

/* GetTableData
 *  returns a pointer to internal table data
 *  or NULL if "self" is invalid
 *
 * NB - THIS OBJECT IS NOT REFERENCE COUNTED
 */
SRA_EXTERN const SRATableData *CC SRATableGetTableData ( const SRATable *self );

#endif

/*--------------------------------------------------------------------------
 * SRA Accession Cache
 */
struct SRACacheIndex;

typedef struct SRACacheMetrics
{   
    uint32_t elements; /* open accessions */
    
    /* not in use currently: */
    uint64_t bytes; /* expanded cache bytes, i.e. cursor */
    uint32_t threads;
    uint32_t fds;
} SRACacheMetrics;

#define SRACacheThresholdSoftBytesDefault       ((uint64_t)0)
#define SRACacheThresholdSoftElementsDefault    ((uint32_t)1000)
#define SRACacheThresholdSoftThreadsDefault     ((uint32_t)0)
#define SRACacheThresholdSoftFdsDefault         ((uint32_t)0)

#define SRACacheThresholdHardBytesDefault       ((uint64_t)0)
#define SRACacheThresholdHardElementsDefault    ((uint32_t)10000)
#define SRACacheThresholdHardThreadsDefault     ((uint32_t)0)
#define SRACacheThresholdHardFdsDefault         ((uint32_t)0)

SRA_EXTERN bool CC SRACacheMetricsLessThan(const SRACacheMetrics* a, const SRACacheMetrics* b);

typedef struct SRACacheElement 
{
    DLNode dad;
    
    SRATable*   object;
    
    KTime_t lastAccessed;
    
    struct SRACacheIndex* index;
    uint32_t key;

    SRACacheMetrics metrics;
} SRACacheElement;

SRA_EXTERN rc_t CC SRACacheElementMake(SRACacheElement**        self, 
                                       SRATable*                object, 
                                       struct SRACacheIndex*    index, 
                                       uint32_t                 key, 
                                       const SRACacheMetrics*   metrics);
SRA_EXTERN rc_t CC SRACacheElementDestroy(SRACacheElement* self);

typedef struct SRACacheIndex
{
    BSTNode dad;
    
    String* prefix;
    KVector* body; /* KVector<SRACacheElement*> */
} SRACacheIndex;

SRA_EXTERN rc_t CC SRACacheIndexMake(SRACacheIndex** self, String* prefix);
SRA_EXTERN rc_t CC SRACacheIndexDestroy(SRACacheIndex* self);

typedef struct SRACacheUsage {
    /* config */
    uint32_t soft_threshold;
    uint32_t hard_threshold;    
    /* usage stats */ 
    uint32_t elements;
    uint64_t requests;
    uint64_t hits;
    uint64_t misses;
    uint64_t busy;
} SRACacheUsage;

typedef struct SRACache
{
    BSTree indexes; /* grows as needed */

    DLList lru; /* DLList<SRACacheElement*>;  head is the oldest */
    
    struct KLock* mutex; 

    SRACacheMetrics softThreshold;
    SRACacheMetrics hardThreshold;
    SRACacheMetrics current;
    
    uint64_t requests;
    uint64_t hits;
    uint64_t misses;
    uint64_t busy;
} SRACache;

SRA_EXTERN rc_t CC SRACacheInit(SRACache**, struct KConfig*);

SRA_EXTERN rc_t CC SRACacheGetSoftThreshold(SRACache* self, SRACacheMetrics* metrics);
SRA_EXTERN rc_t CC SRACacheSetSoftThreshold(SRACache* self, const SRACacheMetrics* metrics);

SRA_EXTERN rc_t CC SRACacheGetHardThreshold(SRACache* self, SRACacheMetrics* metrics);
SRA_EXTERN rc_t CC SRACacheSetHardThreshold(SRACache* self, const SRACacheMetrics* metrics);

SRA_EXTERN rc_t CC SRACacheGetUsage(SRACache* self, SRACacheUsage* usage);

/* flush tables until usage is lower than specified in self->softThreshold */
SRA_EXTERN rc_t CC SRACacheFlush(SRACache* self); 

/* 
    if found, moves element to the back of the list; return NULL object if not in the cache 
    if found but the refcount is not 1, returns RC( rcSRA, rcData, rcAccessing, rcParam, rcBusy)
*/
SRA_EXTERN rc_t CC SRACacheGetTable(SRACache* self, const char* acc, const SRATable** object); 

/* 
 * fails if table is already in the cache.
 * Does not affect usage stats.
 */
SRA_EXTERN rc_t CC SRACacheAddTable(SRACache* self, const char* acc, SRATable*); 

SRA_EXTERN rc_t CC SRACacheWhack(SRACache* self);

#ifdef __cplusplus
}
#endif

#endif /* _h_sra_sradb_priv_ */
