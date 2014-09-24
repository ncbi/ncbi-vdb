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

#ifndef _hpp_sra_sradb_
#define _hpp_sra_sradb_

#ifdef _hpp_sra_wsradb_
#error "read and update libraries are mutually exclusive"
#endif

#ifndef _h_sra_sradb_
#include <sra/sradb.h>
#endif


/*--------------------------------------------------------------------------
 * SRAMgr
 *  opaque handle to SRA library
 */
struct SRAMgr
{
    /* AddRef
     * Release
     */
    inline rc_t AddRef () const throw ()
    { return SRAMgrAddRef ( this ); }

    inline rc_t Release () const throw ()
    { return SRAMgrRelease ( this ); }


    /* Make
     *  create library handle for read-only access
     *
     *  NB - not implemented in read/write library,
     *  and the read-only library may not be mixed with read/write
     */
    static inline rc_t Make ( const SRAMgr **mgr ) throw ()
    { return SRAMgrMakeRead ( mgr ); }
    static inline rc_t Make ( const SRAMgr **mgr, const KDirectory *wd ) throw ()
    { return SRAMgrMakeReadWithDir ( mgr, wd ); }


    /* Version
     *  returns the library version
     */
    inline rc_t Version ( uint32_t *version ) const throw ()
    { return SRAMgrVersion ( this, version ); }


    /* OpenDatatypes
     *  open datatype registry object for requested access
     *  see <vdb/types.hpp> for VDatatypes
     *
     *  "dt" [ OUT ] - return parameter for datatypes object
     */
    inline rc_t OpenDatatypes ( const VDatatypes **dt ) const throw ()
    { return SRAMgrOpenDatatypesRead ( this, dt ); }


    /* GetSchema
     *  get a reference to the schema used to create new tables
     *
     *  "schema" [ OUT ] - return parameter for reference to SRA schema
     *
     * NB - returns a new reference that must be released via VSchemaRelease
     */
    inline rc_t GetSchemaRead ( const VSchema **schema ) const throw ()
    { return SRAMgrGetSchemaRead ( this, schema ); }


    /* UseSchema
     *  allows SRA schema to be entirely replaced
     *  affects creation/update of all subsequent tables
     *
     *  "schema" [ IN ] - new schema to be applied
     *
     * NB - manager will attach a new reference to schema object,
     *  you are still responsible for releasing it via VSchemaRelease
     */
    inline rc_t UseSchemaRead ( const VSchema *schema ) const throw ()
    { return SRAMgrUseSchemaRead ( this, schema ); }


    /* Writable
     *  returns 0 if table is writable
     *  rcLocked if locked, rcReadonly if read-only
     *  other code upon error
     *
     *  "path" [ IN ] - NUL terminated table path
     */
    inline bool Writable ( const char *path, ... ) const throw ()
    {
        va_list args;
        va_start ( args, path );
        bool locked = SRAMgrVWritable ( this, path, args );
        va_end ( args );
        return locked;
    }
    inline bool Writable ( const char *path, va_list args ) const throw ()
    { return SRAMgrVWritable ( this, path, args ); }


    /* OpenTable
     *  open an existing table for read
     *
     *  "tbl" [ OUT ] - return parameter for table
     *
     *  "path" [ IN ] - NUL terminated run name
     */
    inline rc_t OpenTable ( const SRATable **tbl,
        const char *path, ... ) const throw ()
    {
        va_list args;
        va_start ( args, path );
        rc_t rc = SRAMgrVOpenTableRead ( this, tbl, path, args );
        va_end ( args );
        return rc;
    }
    inline rc_t OpenTable ( const SRATable **tbl, const char *path,
        va_list args ) const throw ()
    { return SRAMgrVOpenTableRead ( this, tbl, path, args ); }
    
private:
    SRAMgr ();
    ~ SRAMgr ();
    SRAMgr ( const SRAMgr& );
    SRAMgr &operator = ( const SRAMgr& );
};


/*--------------------------------------------------------------------------
 * SRATable
 *  a collection of spots with several data series, minimally including
 *  base or color calls and their quality ( confidence ) values, and
 *  optionally signal-related values ( signal, intensity, noise, ... ).
 */
struct SRATable
{
    /* AddRef
     * Release
     */
    inline rc_t AddRef () const throw ()
    { return SRATableAddRef ( this ); }

    inline rc_t Release () const throw ()
    { return SRATableRelease ( this ); }


    /* Locked
     *  returns true if locked
     */
    inline bool Locked () const throw ()
    { return SRATableLocked ( this ); }


    /* BaseCount
     *  get the number of stored bases
     *
     *  "num_bases" [ OUT ] - return parameter for base count
     */
    inline rc_t BaseCount ( uint64_t *num_bases ) const throw ()
    { return SRATableBaseCount ( this, num_bases ); }


    /* SpotCount
     *  get the number of stored spots
     *
     *  "spot_count" [ OUT ] - return parameter for spot count
     */
    inline rc_t SpotCount ( uint64_t *spot_count ) const throw ()
    { return SRATableSpotCount ( this, spot_count ); }


    /* MaxSpotId
     *  returns the maximum spot id
     *
     *  a table will contain a collection of spots with ids from
     *  1 to max ( spot_id ).
     *
     *  "id" [ OUT ] - return parameter of last spot id
     */
    inline rc_t MaxSpotId ( spotid_t *id ) const throw ()
    { return SRATableMaxSpotId ( this, id ); }


    /* MinSpotId
     *  returns the minimum spot id
     *
     *  a table will contain a collection of spots with ids from
     *  min to max ( spot_id ).
     *
     *  "id" [ OUT ] - return parameter of last spot id
     */
    inline rc_t MinSpotId ( spotid_t *id ) const throw ()
    { return SRATableMinSpotId ( this, id ); }

    /* GetSpotId
     *  project a spot id from a spot name
     *
     *  "id" [ OUT ] - return parameter for 1-based spot id
     *
     *  "spot_name" [ IN ] - external spot name string
     */
    inline rc_t GetSpotId ( spotid_t *id, const char *spot_name ) const throw ()
    { return SRATableGetSpotId ( this, id, spot_name ); }


    /* ListCol
     *  returns a list of column names
     *
     *  "names" [ out ] - return parameter for names list
     */
    inline rc_t ListCol ( SRANamelist **names ) const throw ()
    { return SRATableListCol ( this, names ); }


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
    inline rc_t ColDatatypes ( const char *col,
        uint32_t *dflt_idx, SRANamelist **typedecls ) const throw ()
    { return SRATableColDatatypes ( this, col, dflt_idx, typedecls ); }


    /* OpenColumn
     *  open a column for read
     *
     *  "col" [ OUT ] - return parameter for newly opened column
     *
     *  "name" [ IN ] - NUL terminated string in UTF-8 giving column name
     *
     *  "datatype" [ IN - NULL OKAY ] - optional NUL terminated
     *   string describing fully qualified column data type in ASCII
     */
    inline rc_t OpenColumn ( const SRAColumn **col,
        const char *name, const char *datatype = 0 ) const throw ()
    { return SRATableOpenColumnRead ( this, col, name, datatype ); }


    /* MetaRevision
     *  returns current revision number
     *  where 0 ( zero ) means tip
     */
    inline rc_t MetaRevision ( uint32_t *revision ) const throw ()
    { return SRATableMetaRevision ( this, revision ); }


    /* MaxRevision
     *  returns the maximum revision available
     */
    inline rc_t MaxMetaRevision ( uint32_t *revision ) const throw ()
    { return SRATableMaxMetaRevision ( this, revision ); }


    /* UseMetaRevision
     *  opens indicated revision of metadata
     *  all non-zero revisions are read-only
     */
    inline rc_t UseMetaRevision ( uint32_t revision ) const throw ()
    { return SRATableUseMetaRevision ( this, revision ); }


    /* OpenMDataNode
     *  open a metadata node
     *
     *  "node" [ OUT ] - return parameter for metadata node
     *
     *  "path" [ IN ] - simple or hierarchical NUL terminated
     *  path to node
     */
    inline rc_t OpenMDataNode ( const KMDataNode **node,
        const char *path, ... ) const throw ()
    {
        va_list args;
        va_start ( args, path );
        rc_t rc = SRATableVOpenMDataNodeRead ( this, node, path, args );
        va_end ( args );
        return rc;
    }
    inline rc_t OpenMDataNode ( const KMDataNode **node,
        const char *path, va_list args ) const throw ()
    { return SRATableVOpenMDataNodeRead ( this, node, path, args ); }


    /*  GetSchema
    *  returns current schema of the open cursor
    */
    inline struct VSchema const* GetSchema () const throw ()
    { return SRATableGetSchema ( this ); }

private:
    SRATable ();
    ~ SRATable ();
    SRATable ( const SRATable& );
    SRATable &operator = ( const SRATable& );
};


/*--------------------------------------------------------------------------
 * SRAColumn
 *  represents a spot data column, where the column is configured as
 *  a sequence of blobs, and each blob is a sequence of records,
 *  indexed by spot id.
 */
struct SRAColumn
{
    /* AddRef
     * Release
     */
    inline rc_t AddRef () const throw ()
    { return SRAColumnAddRef ( this ); }

    inline rc_t Release () const throw ()
    { return SRAColumnRelease ( this ); }


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
    inline int Datatype ( struct VTypedecl *type, struct VTypedef *def ) const throw ()
    { return SRAColumnDatatype ( this, type, def ); }


    /* GetRange
     *  get a contiguous range around a spot id, e.g. tile for Illumina
     *
     *  "id" [ IN ] - return parameter for 1-based spot id
     *
     *  "first" [ OUT, NULL OKAY ] and "last" [ OUT, NULL OKAY ] -
     *  id range is returned in these output parameters, where
     *  at least ONE must be NOT-NULL
     */
    inline rc_t GetRange ( spotid_t id, spotid_t *first, spotid_t *last ) const throw ()
    { return SRAColumnGetRange ( this, id, first, last ); }


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
    inline rc_t Read ( spotid_t id,
        const void **base, bitsz_t *offset, bitsz_t *size ) const throw ()
    { return SRAColumnRead ( this, id, base, offset, size ); }


private:
    SRAColumn ();
    ~ SRAColumn ();
    SRAColumn ( const SRAColumn& );
    SRAColumn &operator = ( const SRAColumn& );
};


/*--------------------------------------------------------------------------
 * SRANamelist
 *  a list of names, types or other strings
 */
struct SRANamelist
{
    /* AddRef
     * Release
     */
    inline rc_t AddRef () const throw ()
    { return SRANamelistAddRef ( this ); }

    inline rc_t Release () const throw ()
    { return SRANamelistRelease ( this ); }


    /* Count
     *  gets the number of names
     *
     *  "count" [ OUT ] - return value
     */
    inline rc_t Count ( uint32_t *count ) const throw ()
    { return SRANamelistCount ( this, count ); }


    /* Get
     *  gets an indexed name
     *
     *  "idx" [ IN ] - zero-based name index
     *
     *  "name" [ OUT ] - return parameter for NUL terminated name
     */
    inline rc_t Get ( uint32_t idx, const char **name ) const throw ()
    { return SRANamelistGet ( this, idx, name ); }


private:
    SRANamelist ();
    ~ SRANamelist ();
    SRANamelist ( const SRANamelist& );
    SRANamelist &operator = ( const SRANamelist& );
};

#endif // _hpp_sra_sradb_
