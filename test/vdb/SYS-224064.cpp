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

#include <vdb/manager.h>
#include <vdb/database.h> 
#include <vdb/table.h> 
#include <vdb/cursor.h> 
#include <vdb/vdb-priv.h>
#include <sra/sraschema.h> // VDBManagerMakeSRASchema
#include <vdb/schema.h> /* VSchemaRelease */

#include <ktst/unit_test.hpp> // TEST_CASE
#include <kfg/config.h> 
#include <klib/printf.h>

#include <iostream>
#include <string>

#define NCBI_TRACES01_PATH "/panfs/traces01"

static
std :: string print_err ( const char * expr, rc_t rc )
{
    size_t num_writ;
    char buffer [ 4096 ];
    rc_t rc2 = string_printf ( buffer, sizeof buffer, & num_writ, "%s: rc = %R", expr, rc );
    if ( rc2 != 0 )
        return std :: string ( "wow!" );
    return std :: string ( buffer, num_writ );
}

#define CALL( x ) \
    if ( ( rc = x ) != 0 ) throw print_err ( #x, rc )

#define _ASSERT( x ) \
    if ( ! ( x ) ) throw std :: string ( # x )

using namespace std;

int LowLevelTest(void)
{
    rc_t rc = 0;

    cout << "LowLevelTest for memory overuse..." << endl;
    const char* file_name = NCBI_TRACES01_PATH
        "/compress/1KG/CEU/NA12249/exome.ILLUMINA.MOSAIK.csra";
    const VDBManager* mgr = 0;
    CALL(VDBManagerMakeRead(&mgr, 0));
        
    const VDatabase* db = 0;
    CALL(VDBManagerOpenDBRead(mgr, &db, 0, file_name));
        
    const VTable* ref_table = 0;
    CALL(VDatabaseOpenTableRead(db, &ref_table, "REFERENCE"));
    
    const VCursor* ref_cursor = 0;
    CALL(VTableCreateCursorRead(ref_table, &ref_cursor));
    CALL(VCursorPermitPostOpenAdd(ref_cursor));
    CALL(VCursorOpen(ref_cursor));
    
    uint32_t align_column;
    CALL(VCursorAddColumn(ref_cursor, &align_column, "PRIMARY_ALIGNMENT_IDS"));
    
    const VTable* align_table = 0;
    CALL(VDatabaseOpenTableRead(db, &align_table, "PRIMARY_ALIGNMENT"));

    const VCursor* align_cursor = 0;
    CALL(VTableCreateCursorRead(align_table, &align_cursor));
    CALL(VCursorPermitPostOpenAdd(align_cursor));
    CALL(VCursorOpen(align_cursor));
    
    uint32_t spot_id_column;
    CALL(VCursorAddColumn(align_cursor, &spot_id_column,
                          "SEQ_SPOT_ID"));
    uint32_t read_id_column;
    CALL(VCursorAddColumn(align_cursor, &read_id_column,
                          "SEQ_READ_ID"));

    const VTable* seq_table = 0;
    CALL(VDatabaseOpenTableRead(db, &seq_table, "SEQUENCE"));

    const VCursor* seq_cursor = 0;
    CALL(VTableCreateCursorRead(seq_table, &seq_cursor));
    CALL(VCursorPermitPostOpenAdd(seq_cursor));
    CALL(VCursorOpen(seq_cursor));
    
    uint32_t trim_len_column;
    CALL(VCursorAddColumn(seq_cursor, &trim_len_column, "TRIM_LEN"));
    
    for ( int64_t ref_row = 582444; ref_row <= 582444; ++ref_row ) {
        const int64_t* align_rows = 0;
        size_t align_count = 0;
        {
            const void* data;
            uint32_t bit_offset, bit_length;
            uint32_t elem_count;
            CALL(VCursorCellDataDirect(ref_cursor, ref_row,
                                       align_column,
                                       &bit_length, &data, &bit_offset,
                                       &elem_count));
            _ASSERT(bit_length == 8*sizeof(int64_t));
            _ASSERT(bit_offset == 0);
            align_rows = static_cast<const int64_t*>(data);
            align_count = elem_count;
        }
        
        for ( size_t i = 0; i < align_count; ++i ) {
            int64_t align_row = align_rows[i];
            int64_t spot_id = 0;
            uint32_t read_id = 0;
            {
                const void* data;
                uint32_t bit_offset, bit_length;
                uint32_t elem_count;
                CALL(VCursorCellDataDirect(align_cursor, align_row,
                                           spot_id_column,
                                           &bit_length, &data, &bit_offset,
                                           &elem_count));
                _ASSERT(bit_length == 8*sizeof(int64_t));
                _ASSERT(bit_offset == 0);
                _ASSERT(elem_count == 1);
                spot_id = *static_cast<const int64_t*>(data);
                _ASSERT(spot_id);
            }
            {
                const void* data;
                uint32_t bit_offset, bit_length;
                uint32_t elem_count;
                CALL(VCursorCellDataDirect(align_cursor, align_row,
                                           read_id_column,
                                           &bit_length, &data, &bit_offset,
                                           &elem_count));
                _ASSERT(bit_length == 8*sizeof(uint32_t));
                _ASSERT(bit_offset == 0);
                _ASSERT(elem_count == 1);
                read_id = *static_cast<const uint32_t*>(data);
                _ASSERT(read_id);
            }

            uint32_t trim_len = 0;
            if ( 1 )
            {
                const void* data;
                uint32_t bit_offset, bit_length;
                uint32_t elem_count;
                CALL(VCursorCellDataDirect(seq_cursor, spot_id,
                                           trim_len_column,
                                           &bit_length, &data, &bit_offset,
                                           &elem_count));
                _ASSERT(bit_length == 8*sizeof(uint32_t));
                _ASSERT(bit_offset == 0);
                _ASSERT(elem_count == 1);
                trim_len = *static_cast<const uint32_t*>(data);
            }

            cout << " " << align_row << ":" << spot_id << "." << read_id
                 << endl;
        }
    }
    CALL(VCursorRelease(seq_cursor));
    CALL(VTableRelease(seq_table));
    CALL(VCursorRelease(align_cursor));
    CALL(VTableRelease(align_table));
    CALL(VCursorRelease(ref_cursor));
    CALL(VTableRelease(ref_table));
    CALL(VDatabaseRelease(db));
    CALL(VDBManagerRelease(mgr));
    cout << "LowLevelTest done" << endl;
    return 0;
}

int main ( int argc, char * argv [] )
{
    int status = 7;

    try
    {
        status = LowLevelTest ();
    }
    catch ( std :: string & x )
    {
        std :: cerr
            << "failed: "
            << x
            << endl
            ;
    }
    catch ( const char x [] )
    {
        std :: cerr
            << "failed: "
            << x
            << endl
            ;
    }
    catch ( std :: exception & x )
    {
        std :: cerr
            << "failed: "
            << x . what ()
            << endl
            ;
    }
    catch ( ... )
    {
        std :: cerr
            << "failed: "
            << "unknown exception"
            << endl
            ;
    }

    return status;
}
