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

#include "ColumnBlob.hpp"

extern "C"
{
    #include "rcolumnblob.h"
    #include "rcolumn.h"
}

struct KColumnBlob
{
    /* holds existing blob loc */
    KColBlobLoc loc;

    /* owning column */
    const struct KRColumn *col;

    /* captured from idx1 for CRC32 validation */
    bool bswap;

    KRColumnPageMap pmorig;
};

class R_ColumnBlob: public TColumnBlob<KColumnBlob>
{
private:
    using Super = TColumnBlob<KColumnBlob>;

public:
    R_ColumnBlob()  {}
    virtual ~R_ColumnBlob() { release(); }

    virtual rc_t whack()
    {
        KColumnSever ( m_blob -> col );
        return Super::whack();
    }

    rc_t Init(bool bswap)
    {
        m_blob = (KColumnBlob*)malloc ( sizeof * m_blob );
        if ( m_blob == nullptr )
            return RC ( rcDB, rcBlob, rcConstructing, rcMemory, rcExhausted );

        memset ( m_blob, 0, sizeof * m_blob );
        m_blob -> bswap = bswap;

        return 0;
    }

    // cf. W_ColumnBlob::openRead
    // depends on KRColumn, cannot be easily moved to TColumnBlob
    rc_t openRead ( const KRColumn *col, int64_t id )
    {
        /* locate blob */
        rc_t rc = KRColumnIdxLocateBlob ( & col -> idx, & m_blob -> loc, id, id );
        if ( rc == 0 )
        {
            /* open page map to blob */
            rc = KRColumnPageMapOpen ( & m_blob -> pmorig,
                ( KRColumnData* ) & col -> df, m_blob -> loc . pg, m_blob -> loc . u . blob . size );
            if ( rc == 0 )
            {
                /* existing blob must have proper checksum bytes */
                if ( m_blob -> loc . u . blob .  size >= col -> csbytes )
                {
                    /* remove them from apparent blob size */
                    m_blob -> loc . u . blob . size -= col -> csbytes;
                    m_blob -> col = KColumnAttach ( col );
                    return 0;
                }

                /* the blob is corrupt */
                rc = RC ( rcDB, rcColumn, rcOpening, rcBlob, rcCorrupt );
            }
        }

        return rc;
    }

    virtual rc_t dataRead( size_t offset, void *buffer, size_t bsize, size_t *num_read ) const
    {
        return KColumnDataRead (
                m_blob -> col -> df . pgsize,
                m_blob -> col -> df . f,
                m_blob -> pmorig . pg,
                offset,
                buffer,
                bsize,
                num_read );
    }

    virtual int32_t checksumType() const
    {
        return m_blob -> col -> checksum;
    }

    virtual const KColBlobLoc& getLoc() const
    {
        return m_blob -> loc;
    }

};

/* Make
 */
rc_t KRColumnBlobMake ( const KColumnBlob **blobp, bool bswap )
{
    R_ColumnBlob *blob = new R_ColumnBlob;
    * blobp = (const KColumnBlob *)blob;
    return blob -> Init( bswap );
}

/* OpenRead
 */
rc_t KRColumnBlobOpenRead ( const KColumnBlob *self, const KRColumn *col, int64_t id )
{
    return ((R_ColumnBlob*)self) -> openRead( col, id );
}
