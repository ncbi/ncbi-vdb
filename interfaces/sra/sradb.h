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

#ifndef _h_sra_sradb_
#define _h_sra_sradb_

#ifndef _h_sra_extern_
#include <sra/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifndef _h_insdc_insdc_
#include <insdc/insdc.h>
#endif

#ifndef _h_insdc_sra_
#include <insdc/sra.h>
#endif

#include <stdarg.h>


#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct VSchema;
struct VTypedef;
struct VTypedecl;
struct VDatatypes;
struct KMDataNode;
struct KDirectory;
struct SRANamelist;


/*--------------------------------------------------------------------------
 * version
 *  current API version is 3.1
 */
#define SRADB_CURRENT_VERSION 0x03010000


/*--------------------------------------------------------------------------
 * spotid_t
 *  a 1-based integer spot identifier
 *  unique within table
 *  see <insdc/sra.h>
 */
typedef INSDC_SRA_spotid_t spotid_t;


/*--------------------------------------------------------------------------
 * REFERENCE COUNTING
 *  most objects used in the API are reference counted.
 *
 *  all functions that return an object return an original reference
 *  to that object ( unless otherwise noted ).
 *
 *  *Release functions are used to discard a reference. under normal
 *  operation objects will be returned by an API function and freed via
 *  the corresponding release.
 *
 *  *AddRef functions are used to attach an additional reference to
 *  an object, which increases the number of *Release messages needed
 *  to actually free an object. manipulating the reference count of an
 *  object is not normally required.
 */


/*--------------------------------------------------------------------------
 * SRAMgr
 *  opaque handle to SRA library
 */
typedef struct SRAMgr SRAMgr;


/* MakeRead
 *  create library reference for read-only access
 *
 *  "wd" [ IN, NULL OKAY ] - optional working directory for
 *  accessing the file system. mgr will attach its own reference
 *
 *  NB - not implemented in update libary
 *  and the read-only library may not be mixed with read/write
 */
SRA_EXTERN rc_t CC SRAMgrMakeRead ( const SRAMgr **mgr );
SRA_EXTERN rc_t CC SRAMgrMakeReadWithDir ( const SRAMgr **mgr, struct KDirectory const *wd );


/* Release
 *  releases reference to object
 *  obtained from MakeRead, MakeUpdate,
 *  or AddRef ( see above )
 */
SRA_EXTERN rc_t CC SRAMgrRelease ( const SRAMgr *self );


/* AddRef
 *  attach a new reference to an existing object
 *  ( see above )
 */
SRA_EXTERN rc_t CC SRAMgrAddRef ( const SRAMgr *self );


/* Version
 *  returns the library version
 */
SRA_EXTERN rc_t CC SRAMgrVersion ( const SRAMgr *self, uint32_t *version );


/* OpenDatatypes - DEPRECATED
 *  open datatype registry object for requested access
 *
 *  "dt" [ OUT ] - return parameter for datatypes object
 */
SRA_EXTERN rc_t CC SRAMgrOpenDatatypesRead ( const SRAMgr *self,
    struct VDatatypes const **dt );


/* GetSchema
 *  get a reference to the schema used to create new tables
 *
 *  "schema" [ OUT ] - return parameter for reference to SRA schema
 *
 * NB - returns a new reference that must be released via VSchemaRelease
 */
SRA_EXTERN rc_t CC SRAMgrGetSchemaRead ( const SRAMgr *self, struct VSchema const **schema );


/* UseSchema
 *  allows SRA schema to be entirely replaced
 *  affects creation/update of all subsequent tables
 *
 *  "schema" [ IN ] - new schema to be applied
 *
 * NB - manager will attach a new reference to schema object,
 *  you are still responsible for releasing it via VSchemaRelease
 */
SRA_EXTERN rc_t CC SRAMgrUseSchemaRead ( const SRAMgr *self, struct VSchema const *schema );


/* Writable
 *  returns 0 if table is writable
 *  rcLocked if locked, rcReadonly if read-only
 *  other code upon error
 *
 *  "path" [ IN ] - NUL terminated table path
 */
SRA_EXTERN rc_t CC SRAMgrWritable ( const SRAMgr *self, const char *path, ... );
SRA_EXTERN rc_t CC SRAMgrVWritable ( const SRAMgr *self, const char *path, va_list args );


/*--------------------------------------------------------------------------
 * SRANamelist
 */
typedef struct SRANamelist SRANamelist;

/* AddRef
 * Release
 *  see REFERENCE COUNTING, above
 */
SRA_EXTERN rc_t CC SRANamelistAddRef ( const SRANamelist *self );
SRA_EXTERN rc_t CC SRANamelistRelease ( const SRANamelist *self );


/* Count
 *  gets the number of names
 *
 *  "count" [ OUT ] - return value
 */
SRA_EXTERN rc_t CC SRANamelistCount ( const SRANamelist *self, uint32_t *count );


/* Get
 *  gets an indexed name
 *
 *  "idx" [ IN ] - zero-based name index
 *
 *  "name" [ OUT ] - return parameter for NUL terminated name
 */
SRA_EXTERN rc_t CC SRANamelistGet ( const SRANamelist *self,
    uint32_t idx, const char **name );
    
    
/*--------------------------------------------------------------------------
 * SRATable
 *  a collection of spots with several data series, minimally including
 *  base or color calls and their quality ( confidence ) values, and
 *  optionally signal-related values ( signal, intensity, noise, ... ).
 *
 *  to discover the series available, use "SRATableListCol" to get
 *  the names, and "SRATableColDatatypes" to list data types available
 *  for each name.
 *
 *  see below for standard readable columns
 */
typedef struct SRATable SRATable;

/* AddRef
 * Release
 *  see REFERENCE COUNTING, above
 */
SRA_EXTERN rc_t CC SRATableAddRef ( const SRATable *self );
SRA_EXTERN rc_t CC SRATableRelease ( const SRATable *self );


/* OpenRead
 *  open an existing table
 *
 *  "tbl" [ OUT ] - return parameter for table
 *
 *  "spec" [ IN ] - NUL terminated UTF-8 string giving path
 *  to table.
 */
SRA_EXTERN rc_t CC SRAMgrOpenTableRead ( const SRAMgr *self,
    const SRATable **tbl, const char *spec, ... );
SRA_EXTERN rc_t CC SRAMgrVOpenTableRead ( const SRAMgr *self,
    const SRATable **tbl, const char *spec, va_list args );

/* Locked
 *  returns true if locked
 */
SRA_EXTERN bool CC SRATableLocked ( const SRATable *self );


/* BaseCount
 *  get the number of stored bases
 *
 *  "num_bases" [ OUT ] - return parameter for base count
 */
SRA_EXTERN rc_t CC SRATableBaseCount ( const SRATable *self, uint64_t *num_bases );


/* SpotCount
 *  get the number of stored spots
 *
 *  "spot_count" [ OUT ] - return parameter for spot count
 */
SRA_EXTERN rc_t CC SRATableSpotCount ( const SRATable *self, uint64_t *spot_count );


/* MinSpotId
 *  returns the minimum spot id
 *
 *  a table will contain a collection of spots with ids from
 *  min(spot_id) to max(spot_id) unless empty.
 *
 *  "id" [ OUT ] - return parameter of last spot id
 *  or zero if the table is empty.
 */
SRA_EXTERN rc_t CC SRATableMinSpotId ( const SRATable *self, spotid_t *id );

/* MaxSpotId
 *  returns the maximum spot id
 *
 *  a table will contain a collection of spots with ids from
 *  1 to max ( spot_id ) unless empty.
 *
 *  "id" [ OUT ] - return parameter of last spot id
 *  or zero if the table is empty.
 */
SRA_EXTERN rc_t CC SRATableMaxSpotId ( const SRATable *self, spotid_t *id );


/* GetSpotId
 *  convert spot name to spot id
 *  may fail if original textual spot name is not present
 *
 *  "id" [ OUT ] - return parameter for 1-based spot id
 *
 *  "spot_name" [ IN ] - external spot name string
 *  in platform canonical format.
 */
SRA_EXTERN rc_t CC SRATableGetSpotId ( const SRATable *self,
    spotid_t *id, const char *spot_name );


/* ListCol
 *  returns a list of simple column names
 *  each name represents at least one typed column
 *
 *  "names" [ out ] - return parameter for names list
 */
SRA_EXTERN rc_t CC SRATableListCol ( const SRATable *self, SRANamelist **names );


/* ColDatatypes
 *  returns list of typedecls for named column
 *
 *  "col" [ IN ] - column name
 *
 *  "dflt_idx" [ OUT, NULL OKAY ] - returns the zero-based index
 *  into "typedecls" of the default datatype for the named column
 *
 *  "typedecls" [ OUT ] - list of datatypes available for named column
 */
SRA_EXTERN rc_t CC SRATableColDatatypes ( const SRATable *self, const char *col,
    uint32_t *dflt_idx, SRANamelist **typedecls );


/* MetaRevision
 *  returns current revision number
 *  where 0 ( zero ) means tip
 */
SRA_EXTERN rc_t CC SRATableMetaRevision ( const SRATable *self, uint32_t *revision );


/* MaxRevision
 *  returns the maximum revision available
 */
SRA_EXTERN rc_t CC SRATableMaxMetaRevision ( const SRATable *self, uint32_t *revision );


/* UseMetaRevision
 *  opens indicated revision of metadata
 *  all non-zero revisions are read-only
 */
SRA_EXTERN rc_t CC SRATableUseMetaRevision ( const SRATable *self, uint32_t revision );


/* OpenMDataNode
 *  open a metadata node
 *
 *  "node" [ OUT ] - return parameter for metadata node
 *
 *  "path" [ IN ] - simple or hierarchical NUL terminated
 *  path to node
 */
SRA_EXTERN rc_t CC SRATableOpenMDataNodeRead ( const SRATable *self,
    struct KMDataNode const **node, const char *path, ... );
SRA_EXTERN rc_t CC SRATableVOpenMDataNodeRead ( const SRATable *self,
    struct KMDataNode const **node, const char *path, va_list args );


/*  SRATableGetSchema
 *  returns current schema of the open cursor
 */
SRA_EXTERN struct VSchema const* CC SRATableGetSchema ( const SRATable *self );



/*--------------------------------------------------------------------------
 * SRAColumn
 *  represents a spot data column, where the column is configured as
 *  a sequence of blobs, and each blob is a sequence of records,
 *  indexed by spot id.
 */
typedef struct SRAColumn SRAColumn;

/* AddRef
 * Release
 *  see REFERENCE COUNTING, above
 */
SRA_EXTERN rc_t CC SRAColumnAddRef ( const SRAColumn *self );
SRA_EXTERN rc_t CC SRAColumnRelease ( const SRAColumn *self );


/* OpenColumnRead
 *  open a column for read
 *
 *  "col" [ OUT ] - return parameter for newly opened column
 *
 *  "name" [ IN ] - NUL terminated string in UTF-8 giving column name
 *
 *  "datatype" [ IN, NULL OKAY ] - optional NUL terminated typedecl
 *  string describing fully qualified column data type, or if NULL
 *  the default type for column.
 */
SRA_EXTERN rc_t CC SRATableOpenColumnRead ( const SRATable *self,
    const SRAColumn **col, const char *name, const char *datatype );


/* Datatype
 *  access data type
 *
 *  "type" [ OUT, NULL OKAY ] - returns the column type declaration
 *
 *  "def" [ OUT, NULL OKAY ] - returns the definition of the type
 *  returned in "type_decl"
 *
 * NB - one of "type" and "def" must be non-NULL
 */
SRA_EXTERN rc_t CC SRAColumnDatatype ( const SRAColumn *self,
    struct VTypedecl *type, struct VTypedef *def );


/* GetRange
 *  get a contiguous range around a spot id, e.g. tile for Illumina
 *
 *  "id" [ IN ] - return parameter for 1-based spot id
 *
 *  "first" [ OUT, NULL OKAY ] and "last" [ OUT, NULL OKAY ] -
 *  id range is returned in these output parameters, where
 *  at least ONE must be NOT-NULL
 */
SRA_EXTERN rc_t CC SRAColumnGetRange ( const SRAColumn *self,
    spotid_t id, spotid_t *first, spotid_t *last );


/* Read
 *  read row data
 *
 *  "id" [ IN ] - spot row id between 1 and max ( spot id )
 *
 *  "base" [ OUT ] and "offset" [ OUT ] - pointer and bit offset
 *  to start of spot row data.
 *
 *  "size" [ OUT ] - size in bits of row data
 */
SRA_EXTERN rc_t CC SRAColumnRead ( const SRAColumn *self, spotid_t id,
    const void **base, bitsz_t *offset, bitsz_t *size );



/*--------------------------------------------------------------------------
 * SRA column formatted type descriptions
 *
 *  most datatypes are constructed as vectors, rather than structures.
 *  this permits flexibility in data transformations and removes compiler
 *  dependency in layout.
 */


/* SRAPlatforms
 *  see <insdc/sra.h>
 */
typedef INSDC_SRA_platform_id SRAPlatforms;


/* SRAReadTypes
 *  read type bits
 *  see <insdc/sra.h>
 */
typedef INSDC_SRA_xread_type SRAReadTypes;


/* SRAReadFilter
 *  read filter attribute bits
 *  see <insdc/sra.h>
 */
typedef INSDC_SRA_read_filter SRAReadFilter;


/* SRASpotDesc - DEPRECATED
 *  a structure returned by SPOT_DESC column
 */
#define sra_spot_desc_t "NCBI:SRA:SpotDesc"
typedef struct SRASpotDesc SRASpotDesc;
struct SRASpotDesc
{
    /* lengths in bases */
    uint16_t spot_len;
    uint16_t fixed_len;
    uint16_t signal_len;

    /* quality clips */
    uint16_t clip_qual_right;

    /* number of reads per spot */
    uint8_t num_reads;

    /* pad out to have 16 byte structure size */
    uint8_t align [ 7 ];
};

/* SRASegment - DEPRECATED
 *  a paired starting coordinate and length
 */
#define sra_segment_t "NCBI:SRA:Segment"
typedef struct SRASegment SRASegment;
struct SRASegment
{
    uint16_t start;
    uint16_t len;
};

/* SRAReadDesc - DEPRECATED
 *  a structure 
 */
#define sra_read_desc_t "NCBI:SRA:ReadDesc"
typedef struct SRAReadDesc SRAReadDesc;
struct SRAReadDesc
{
    /* location and length in bases of read
       within READ and QUALITY columns */
    SRASegment seg;

    /* read type from SRAReadTypes */
    uint8_t type;

    /* optional color space key */
    char cs_key;

    /* NUL terminated read label */
    char label [ 74 ];
};

/* SRAPhredPair
 *  pairs an event character with a phred score
 *  event characters are "ACGT0123B+-"
 */
#define sra_phred_pair_t "NCBI:SRA:PhredPair"
typedef struct SRAPhredPair SRAPhredPair;
struct SRAPhredPair
{
    char event;
    INSDC_quality_phred phred;
};


#ifdef __cplusplus
}
#endif

#endif /* _h_sra_sradb_ */
