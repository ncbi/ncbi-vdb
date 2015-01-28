// ngs-test.cpp
//

#include <iostream>

#include <ngs/ncbi/NGS.hpp>
/*#include <ngs/ErrorMsg.hpp>
#include <ngs/ReadCollection.hpp>
#include <ngs/AlignmentIterator.hpp>
#include <ngs/Alignment.hpp>*/

#include "my_utils.h"

#include <vdb/manager.h>
#include <vdb/database.h>
#include <vdb/table.h>
#include <vdb/cursor.h>

#include "../../libs/ngs/NGS_Reference.h"
#include "../../libs/ngs/NGS_Pileup.h"

#include <vector>
#include <algorithm>

extern "C" uint64_t DEBUG_CSRA1_Pileup_GetRefPos (void const* self);

#if 0
extern "C" int64_t DEBUG_CSRA1_Reference_GetRowBegin ( void const* self );
extern "C" int64_t DEBUG_CSRA1_Reference_GetRowEnd ( void const* self );

extern "C" PileupIteratorState* global_get_pileup_state ();

extern "C" rc_t init_vdb_objects (
    VDBManager const** mgr,
    VDatabase const** db,
    VTable const** table_ref,
    VTable const** table_pa,
    VCursor const** cursor_ref,
    VCursor const** cursor_pa,
    char const* db_path,
    char const* const* column_names_ref, uint32_t* column_index_ref, size_t column_count_ref,
    char const* const* column_names_pa, uint32_t* column_index_pa, size_t column_count_pa,
    char* error_buf,
    size_t error_buf_size);

extern "C" void release_vdb_objects (
    VDBManager const* mgr,
    VDatabase const* db,
    VTable const* table_ref,
    VTable const* table_pa,
    VCursor const* cursor_ref,
    VCursor const* cursor_pa);

extern "C" bool nextPileup (
    PileupIteratorState* pileup_state,
    VCursor const* cursor_ref, VCursor const* cursor_pa,
    char const* const* column_names_ref, uint32_t* column_index_ref, size_t column_count_ref,
    char const* const* column_names_pa, uint32_t* column_index_pa, size_t column_count_pa,
    char* error_buf,
    size_t error_buf_size);

extern "C" rc_t initialize_ref_pos (
    PileupIteratorState* pileup_state,
    VCursor const* cursor_ref, VCursor const* cursor_pa,
    char const* const* column_names_ref, uint32_t* column_index_ref, size_t column_count_ref,
    char const* const* column_names_pa, uint32_t* column_index_pa, size_t column_count_pa,
    char* error_buf,
    size_t error_buf_size);

#endif
namespace PileupTest
{
#if 0
    class ExperimantalPileupIterator
    {
    public:
        ExperimantalPileupIterator();
        ~ExperimantalPileupIterator();

        void init_vdb_objects (char const* db_path);
        void release_vdb_objects ();
        void init_state (int64_t reference_start_id,
            int64_t slice_start, uint64_t slice_length);

        bool nextPileup ();
        uint64_t getCurrentRefPos () const { return pileup_state.ref_pos; }
        uint64_t getDepth () const;

    private:

        ::PileupIteratorState pileup_state;

        char error_buf[256];

        VDBManager const* mgr;
        VDatabase const* db;

        VTable const* table_ref;
        VTable const* table_pa; 

        VCursor const* cursor_ref;
        VCursor const* cursor_pa;

        static char const* const column_names_ref[4];
        static char const* const column_names_pa[2];

        uint32_t column_index_ref [ countof (column_names_ref) ];
        uint32_t column_index_pa [ countof (column_names_pa) ];

    };

    char const* const ExperimantalPileupIterator::column_names_ref[] =
    {
        "MAX_SEQ_LEN",
        "PRIMARY_ALIGNMENT_IDS",
        "SEQ_START",
        "NAME"
    };
    char const* const ExperimantalPileupIterator::column_names_pa[] =
    {
        "REF_START",
        "REF_LEN"
    };

    ExperimantalPileupIterator::ExperimantalPileupIterator ()
        : mgr(NULL), db(NULL), table_ref(NULL), table_pa(NULL),
        cursor_ref(NULL), cursor_pa(NULL)
    {
        error_buf[0] = '\0';
#if CACHE_IMPL_AS_LIST == 1
        pileup_state.cache_alignment.size = 0;
        ::Alignment_InitCacheWithNull ( & pileup_state.cache_alignment );
#else
        pileup_state.cache_alignment.data = NULL;
#endif
        pileup_state.alignment_ids = NULL;
#if USE_SINGLE_BLOB_FOR_ALIGNMENT_IDS == 1
        pileup_state.blob_alignment_ids = NULL;
#endif
#if USE_SINGLE_BLOB_FOR_ALIGNMENTS == 1
        pileup_state.blob_alignments_ref_start = NULL;
        pileup_state.blob_alignments_ref_len = NULL;
#endif
    }

    ExperimantalPileupIterator::~ExperimantalPileupIterator()
    {
        release_vdb_objects ();
        ::Alignment_Release ( & pileup_state.cache_alignment );
        ::PileupIteratorState_Release ( & pileup_state );
    }

    void ExperimantalPileupIterator::init_vdb_objects (char const* db_path)
    {
        rc_t rc = :: init_vdb_objects (
            &mgr, &db, &table_ref, &table_pa, &cursor_ref, &cursor_pa,
            db_path,
            column_names_ref, column_index_ref, countof (column_names_ref),
            column_names_pa, column_index_pa, countof (column_names_pa),
            error_buf, countof (error_buf) );

        if ( rc != 0 )
        {
            release_vdb_objects ();
            throw ngs::ErrorMsg ( error_buf );// std::exception ( error_buf );
        }
    }

    void ExperimantalPileupIterator::init_state (
        int64_t reference_start_id,
        int64_t slice_start, uint64_t slice_length )
    {
        ::Alignment_Release ( & pileup_state.cache_alignment );
        ::PileupIteratorState_Release ( & pileup_state );
        ::Alignment_Init ( & pileup_state.cache_alignment );
        ::PileupIteratorState_Init ( & pileup_state );

        pileup_state.ref_name [0] = '\0'; // we might copy ri.getCanonicalName()
        pileup_state.reference_start_id = reference_start_id;
        //pileup_state.reference_end_id = 0; // anyway we don't know the end id here
        //pileup_state.reference_length = reference_length;
        pileup_state.slice_start = 0;//20000-10;
        pileup_state.slice_length = 0;//32;

        pileup_state.ref_pos = pileup_state.slice_start;

        rc_t rc = ::initialize_ref_pos ( &pileup_state,
            cursor_ref, cursor_pa,
            column_names_ref, column_index_ref, countof (column_names_ref),
            column_names_pa, column_index_pa, countof (column_names_pa),
            error_buf, countof(error_buf));

        if ( rc != 0 )
            throw ngs::ErrorMsg ( error_buf ); //std::exception (error_buf);
    }

    void ExperimantalPileupIterator::release_vdb_objects ()
    {
        ::release_vdb_objects (mgr, db, table_ref, table_pa, cursor_ref, cursor_pa);

        mgr = NULL;
        db = NULL;
        table_ref = NULL;
        table_pa = NULL;
        cursor_ref = NULL;
        cursor_pa = NULL;
    }

    bool ExperimantalPileupIterator::nextPileup ()
    {
        bool has_next = ::nextPileup ( & pileup_state, cursor_ref, cursor_pa,
                            column_names_ref, column_index_ref, countof (column_index_ref),
                            column_names_pa, column_index_pa, countof (column_index_pa),
                            error_buf, countof (error_buf) );

        if ( !has_next && error_buf[0] )
            throw ngs::ErrorMsg ( error_buf ); //std::exception ( error_buf );

        return has_next;
    }

    uint64_t ExperimantalPileupIterator::getDepth () const
    {
        return pileup_state.cache_alignment.size;
        /*uint64_t depth = 0;
        for (size_t i = 0; i < pileup_state.cache_alignment.size; ++i)
        {
            Alignment_CacheItem const* cache_item = Alignment_Get ( & pileup_state.cache_alignment, i );
            uint64_t local_ref_pos = pileup_state.ref_pos - cache_item->seq_start;

            if ( cache_item->start <= (int64_t)local_ref_pos &&
                 cache_item->start + (int64_t)cache_item->len >= (int64_t)local_ref_pos)
            {
                ++ depth;
            }
        }
        return depth;*/
    }
#endif

    void Test_SliceVsEntireRef ()
    {
        std::cout << "Running Test_SliceVsEntireRef " << std::endl;
        char const db_path[] = "SRR341578";

        std::vector <uint32_t> vecDepthSlice, vecDepthEntire, vecRef;

        int64_t const pos_start = 20017;
        uint64_t const len = 5;

        vecRef.push_back(1); // 20017
        vecRef.push_back(0); // 20018
        vecRef.push_back(1); // 20019
        vecRef.push_back(1); // 20020
        vecRef.push_back(3); // 20021

        {
            ngs::ReadCollection run = ncbi::NGS::openReadCollection (db_path);
            ngs::ReferenceIterator ri = run.getReferences ();

            ri.nextReference ();
            ri.nextReference ();

            std::cout
                << ri.getCanonicalName()
                << ": len="  << ri.getLength()
                << std::endl;


            ngs::PileupIterator pi = ri.getPileups ( ngs::Alignment::primaryAlignment );

            uint64_t ref_pos = 0;
            for (; pi.nextPileup (); ++ ref_pos)
            {
                if ( ref_pos >= (uint64_t)pos_start && ref_pos < (uint64_t)pos_start + len )
                {
                    std::cout
                        << " pos=" << ref_pos
                        << ", depth=" << pi.getPileupDepth ()
                        << std::endl;
                    vecDepthEntire.push_back ( pi.getPileupDepth () );
                }
            }
        }
        {
            ngs::ReadCollection run = ncbi::NGS::openReadCollection (db_path);
            ngs::ReferenceIterator ri = run.getReferences ();

            ri.nextReference ();
            ri.nextReference ();

            std::cout
                << ri.getCanonicalName()
                << ": len="  << ri.getLength()
                << std::endl;


            ngs::PileupIterator pi = ri.getPileupSlice ( pos_start, len, ngs::Alignment::primaryAlignment );

            uint64_t ref_pos = 0;
            for (; pi.nextPileup (); ++ ref_pos)
            {
                if ( ref_pos >= (uint64_t)pos_start && ref_pos < (uint64_t)pos_start + len )
                {
                    std::cout
                        << " pos=" << ref_pos
                        << ", depth=" << pi.getPileupDepth ()
                        << std::endl;
                    vecDepthSlice.push_back ( pi.getPileupDepth () );
                }
            }
        }
    }

    void RunUnsafe (char const* db_path)
    {
        ngs::ReadCollection run = ncbi::NGS::openReadCollection (db_path);
        ngs::ReferenceIterator ri = run.getReferences ();

        //ExperimantalPileupIterator pi;
        //pi.init_vdb_objects (db_path);

        for (; ri.nextReference () ;)
        {
            std::cout
                << ri.getCanonicalName()
                << ": len="  << ri.getLength()
                //<< " row_begin=" << DEBUG_CSRA1_Reference_GetRowBegin ( ri.self )
                //<< " row_end=" << DEBUG_CSRA1_Reference_GetRowEnd ( ri.self )
                << std::endl;

            ngs::PileupIterator pi = ri.getPileups ( ngs::Alignment::primaryAlignment /*all*/ );

            uint64_t ref_pos = 0;
            for (; pi.nextPileup (); ++ ref_pos)
            {
                if ( !(ref_pos % 5113) /*|| ref_pos >= 19900 && ref_pos <= 20022*/)
                {
                    std::cout
                        << " pos=" << ref_pos
                        << ", depth=" << pi.getPileupDepth ()
                        << std::endl;
                }
            }
            std::cout << "position count=" << ref_pos << std::endl;

#if 0
            for (; ;)
            {
                if ( !(pi.getCurrentRefPos() % 5113))
                {
                    std::cout
                        << " pos=" << pi.getCurrentRefPos()
                        << ", depth=" << pi.getDepth()
                        << std::endl;
                }

                if ( ! pi.nextPileup() )
                    break;
            }
#endif
        }

    }

    void Run (char const* db_path)
    {
        std::cout << "Hello from ngs_test" << std::endl;

        try
        {
            //RunUnsafe (db_path);
            Test_SliceVsEntireRef ();
        }
        catch ( ngs::ErrorMsg & x )
        {
            std::cerr <<  x.toString () << std::endl;
        }
        catch ( std::exception & x )
        {
            std::cerr <<  x.what () << std::endl;
        }
        catch ( ... )
        {
            std::cerr <<  "unknown exception\n";
        }
    }
}


extern "C"
{
    void NGS_Test (char const* db_path)
    {
        PileupTest::Run (db_path);
    }
}
