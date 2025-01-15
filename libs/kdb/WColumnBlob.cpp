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

#include <kdb/extern.h>

#include "ColumnBlob.hpp"

extern "C"
{
    #include "wcolumnblob.h"
    #include "wcolumn.h"
    #include "werror.h"
}

struct KColumnBlob
{
    /* this part is shared with the read-side structure, consider inheritance */

    /* holds either an existing blob loc
       or new blob index range */
    KColBlobLoc loc;

    /* owning column */
    KWColumn *col;

    /* for validation */
    bool bswap;

    /*************************************************************************/

    /* write-side only fields: */

    /* holds old and new page maps */
    KWColumnPageMap pmorig;
    KWColumnPageMap pmnew;

    /* number of bytes written to blob */
    uint32_t num_writ;

    /* checksums */
    uint32_t crc32;
    MD5State md5;

    /* open mode */
    uint8_t read_only;
};

class W_ColumnBlob: public TColumnBlob<KColumnBlob>
{
private:
    using Super = TColumnBlob<KColumnBlob>;

public:
    W_ColumnBlob()  {}
    virtual ~W_ColumnBlob() { release(); }

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

    // cf. R_ColumnBlob::openRead
    // depends on KRColumn, cannot be easily moved to TColumnBlob
    rc_t openRead ( const KWColumn *col, int64_t id )
    {
        /* locate blob */
        rc_t rc = KWColumnIdxLocateBlob ( & col -> idx, & m_blob -> loc, id, id );
        if ( rc == 0 )
        {
            /* open page map to blob */
            rc = KWColumnPageMapOpen ( & m_blob -> pmorig,
                ( KWColumnData* ) & col -> df, m_blob -> loc . pg, m_blob -> loc . u . blob . size );
            if ( rc == 0 )
            {
                /* existing blob must have proper checksum bytes */
                if ( m_blob -> loc . u . blob .  size >= col -> csbytes )
                {
                    /* remove them from apparent blob size */
                    m_blob -> loc . u . blob . size -= col -> csbytes;
                    m_blob -> col = KColumnAttach ( col );
                    m_blob -> read_only = true; // only on the write side
                    return 0;
                }

                /* the blob is corrupt */
                rc = RC ( rcDB, rcColumn, rcOpening, rcBlob, rcCorrupt );
            }
        }

        return rc;
    }

    virtual rc_t validate() const
    {
        if ( m_blob -> num_writ != 0 )
            return RC ( rcDB, rcBlob, rcValidating, rcBlob, rcBusy );
        return Super::validate();
    }

    virtual rc_t validateBuffer ( struct KDataBuffer const * buffer, const KColumnBlobCSData * cs_data, size_t cs_data_size ) const
    {
        rc_t rc = Super::validateBuffer( buffer, cs_data, cs_data_size );
        if ( rc == SILENT_RC ( rcDB, rcBlob, rcValidating, rcChecksum, rcNotFound ) ) // this is allowed on the write side
        {
            rc = 0;
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
rc_t KWColumnBlobMake ( KColumnBlob **blobp, bool bswap )
{
    W_ColumnBlob *blob = new W_ColumnBlob;
    * blobp = (KColumnBlob *)blob;
    return blob -> Init( bswap );
}

/* OpenRead
 */
rc_t KWColumnBlobOpenRead ( KColumnBlob *self, const KWColumn *col, int64_t id )
{
    return ((W_ColumnBlob*)self) -> openRead( col, id );
}

rc_t KWColumnBlobOpenUpdate ( KColumnBlob *bself, KWColumn *col, int64_t id )
{
    KColumnBlob *self = ((W_ColumnBlob*)bself) -> getBlob();
    /* open existing blob */
    rc_t rc = KWColumnBlobOpenRead ( self, col, id );
    if ( rc == 0 )
    {
        /* create a new page map for replacement */
        rc = KWColumnPageMapCreate ( & self -> pmnew, & col -> df );
        if ( rc == 0 )
        {
            /* initialize for writing checksums */
            switch ( col -> checksum )
            {
            case kcsCRC32:
                self -> crc32 = 0;
                break;
            case kcsMD5:
                MD5StateInit ( & self -> md5 );
                break;
            }

            self -> col = KColumnAttach ( col );
        }
    }

    return rc;
}

#define CAST() KColumnBlob * self = ((W_ColumnBlob*)bself) -> getBlob();

/* Create
 */
rc_t KWColumnBlobCreate ( KColumnBlob *bself, KWColumn *col )
{
    CAST();

    rc_t rc;

    if ( col -> md5 != NULL )
    {
	/* this perhaps should be split into wcoldat.c and wcolidx.c */
	rc = KMD5FileBeginTransaction ( col -> df . fmd5 );
	if ( rc == 0 )
	{
	    rc = KMD5FileBeginTransaction ( col -> idx . idx2 . fmd5 );
	    if ( rc == 0 )
	    {
		rc = KMD5FileBeginTransaction ( col -> idx . idx0 . fmd5 );
		if ( rc == 0 )
		{
		    rc = KMD5FileBeginTransaction ( col -> idx . idx1 . fmd5 );
		    if ( rc == 0 )
		    {
			rc = KMD5FileBeginTransaction ( col -> idx . idx1 . fidxmd5 );
			if ( rc != 0 )
			    KMD5FileCommit ( col -> idx . idx1 .  fmd5 );
		    }
		    if ( rc != 0 )
			KMD5FileCommit ( col -> idx . idx0 .  fmd5 );
		}
		if ( rc != 0 )
		    KMD5FileCommit ( col -> idx . idx2 .  fmd5 );
	    }
	    if ( rc != 0 )
		KMD5FileCommit ( col -> df . fmd5 );
	}
	if ( rc != 0 )
	    return rc;
    }

    /* no location yet */
    memset ( & self -> loc, 0, sizeof self -> loc );

    /* invalid existing page map */
    memset ( & self -> pmorig, 0, sizeof self -> pmorig );

    /* create a new page map */
    rc = KWColumnPageMapCreate ( & self -> pmnew, & col -> df );
    if ( rc == 0 )
    {
        /* initialize for writing checksums */
        switch ( col -> checksum )
        {
        case kcsCRC32:
            self -> crc32 = 0;
            break;
        case kcsMD5:
            MD5StateInit ( & self -> md5 );
            break;
        }

        self -> col = KColumnAttach ( col );
    }

    return rc;
}

/* KColumnBlobAppend
 *  append data to open blob
 *
 *  "buffer" [ IN ] and "size" [ IN ] - blob data
 */
LIB_EXPORT rc_t CC KColumnBlobAppend ( KColumnBlob *bself, const void *buffer, size_t size )
{
    KColumnBlob *self = ((W_ColumnBlob*)bself) -> getBlob();

    KWColumn *col;
    size_t total, num_writ;

    if ( self == NULL )
        return RC ( rcDB, rcBlob, rcWriting, rcSelf, rcNull );

    /* data fork will refuse write anyway, but... */
    if ( self -> read_only )
        return RC ( rcDB, rcBlob, rcWriting, rcBlob, rcReadonly );

    col = self -> col;

    for ( total = 0; total < size; total += num_writ )
    {
        rc_t rc = KWColumnDataWrite ( & col -> df,
            & self -> pmnew, self -> num_writ + total,
            ( const char* ) buffer + total, size - total, & num_writ );
        if ( rc != 0 )
            return rc;
        if ( num_writ == 0 )
            return RC ( rcDB, rcBlob, rcWriting, rcTransfer, rcIncomplete );
    }

    self -> num_writ += size;

    switch ( col -> checksum )
    {
    case kcsCRC32:
        self -> crc32 = CRC32 ( self -> crc32, buffer, size );
        break;
    case kcsMD5:
        MD5StateAppend ( & self -> md5, buffer, size );
        break;
    }

    return 0;
}

/* KColumnBlobAssignRange
 *  assign a total id range to blob at any time before commit
 *
 *  "first" [ IN ] and "count" [ IN ] - range parameters for assign
 */
LIB_EXPORT rc_t CC KColumnBlobAssignRange ( KColumnBlob *bself, int64_t first, uint32_t count )
{
    KColumnBlob *self = ((W_ColumnBlob*)bself) -> getBlob();

    rc_t rc;
    const KWColumn *col;

    if ( self == NULL )
        return RC ( rcDB, rcBlob, rcUpdating, rcSelf, rcNull );

    if ( count == 0 )
        return RC ( rcDB, rcBlob, rcUpdating, rcRange, rcEmpty );

    if ( self -> read_only )
        return RC ( rcDB, rcBlob, rcUpdating, rcBlob, rcReadonly );

    if ( self -> loc . id_range != 0 )
    {
        /* allow benign reassignment */
        if ( self -> loc . start_id == first &&
             self -> loc . id_range == count )
            return 0;

        /* can't be reset */
        return RC ( rcDB, rcBlob, rcUpdating, rcBlob, rcBusy );
    }

    col = self -> col;

    /* try to open read map */
    rc = KWColumnBlobOpenRead ( self, col, first );
    if ( rc == 0 )
    {
        /* blob already exists
           again, allow benign reassignment */
        if ( self -> loc . start_id == first &&
             self -> loc . id_range == count )
            return 0;

        /* conflicting assignment */
        memset ( & self -> loc, 0, sizeof self -> loc );
        memset ( & self -> pmorig, 0, sizeof self -> pmorig );
        return RC ( rcDB, rcBlob, rcUpdating, rcRange, rcIncorrect );
    }

    /* expect status of not found */
    if ( GetRCState ( rc ) != rcNotFound )
        return rc;

    /* assign the range */
    self -> loc . pg = 0;
    self -> loc . u . blob . remove = 0;
    self -> loc . start_id = first;
    self -> loc . id_range = count;
    return 0;
}

/* KColumnBlobCommit
 *  commit changes to blob
 *  close to further updates
 */
static
char zero [ 4096 ];

static
rc_t KColumnBlobZeroPad ( KColumnBlob *self )
{
    KWColumn *col = self -> col;
    size_t pad_bytes = self -> num_writ % col -> df . pgsize;
    if ( pad_bytes != 0 )
    {
        size_t total, num_writ;

        pad_bytes = col -> df . pgsize - pad_bytes;
        for ( total = 0; total < pad_bytes; total += num_writ )
        {
            rc_t rc;

            size_t to_write = pad_bytes - total;
            if ( to_write > sizeof zero )
                to_write = sizeof zero;

            rc = KWColumnDataWrite ( & col -> df,
                & self -> pmnew, self -> num_writ + total,
                zero, to_write, & num_writ );
            if ( rc != 0 )
                return rc;
            if ( num_writ == 0 )
                return RC ( rcDB, rcBlob, rcCommitting, rcTransfer, rcIncomplete );
        }
    }
    return 0;
}

static
rc_t KColumnBlobDoCommit ( KColumnBlob *self )
{
    rc_t rc;
    KColBlobLoc loc;
    KWColumn *col = self -> col;

    /* finish checksum */
    if ( col -> csbytes != 0 )
    {
        MD5State md5;
        uint32_t crc32;
        uint8_t digest [ 16 ];

        void *cs;
        size_t num_writ;

        switch ( col -> checksum )
        {
        case kcsCRC32:
            crc32 = self -> crc32;
            if ( self -> bswap )
                crc32 = bswap_32 ( crc32 );
            cs = & crc32;
            break;
        case kcsMD5:
        default: /* to quiet compiler warnings */
            /* work on copy in case of failure */
            md5 = self -> md5;
            MD5StateFinish ( & md5, digest );
            cs = digest;
            break;
        }

        rc = KWColumnDataWrite ( & col -> df,
            & self -> pmnew, self -> num_writ,
            cs, col -> csbytes, & num_writ );
        if ( rc != 0 )
            return rc;
        if ( num_writ != col -> csbytes )
            return RC ( rcDB, rcBlob, rcCommitting, rcTransfer, rcIncomplete );

        self -> num_writ += num_writ;
    }

    /* extract index information */
    rc = KWColumnPageMapId ( & self -> pmnew, & col -> df, & loc . pg );
    if ( rc == 0 )
    {
        loc . u . blob . size = ( uint32_t ) self -> num_writ;
        loc . u . blob . remove = 0;
        loc . start_id = self -> loc . start_id;
        loc . id_range = self -> loc . id_range;

        /* pad out rest of page */
        if ( col -> df . pgsize > 1 )
            rc = KColumnBlobZeroPad ( self );
        if ( rc == 0 )
        {
            /* commit data fork */
            rc = KWColumnDataCommit ( & col -> df,
                & self -> pmnew, self -> num_writ );
            if ( rc == 0 )
            {
                /* commit index fork */
                rc = KWColumnIdxCommit ( & col -> idx, col -> md5,
                    & loc, col -> commit_freq, col -> df . eof,
                    col -> df . pgsize, col -> checksum );
                if ( rc == 0 || rc == kdbReindex )
                {
                    rc_t status = rc;

                    /* release old pages */
                    if ( self -> loc . u . blob . size == 0 )
                        rc = 0;
                    else
                    {
                        rc = KWColumnDataFree ( & col -> df,
                            & self -> pmorig, self -> loc . u . blob . size + col -> csbytes );
                        if ( GetRCState ( rc ) == rcInvalid )
                            rc = 0;
                    }

                    if ( rc == 0 )
                    {
                        /* transfer new map */
                        self -> pmorig = self -> pmnew;
                        memset ( & self -> pmnew, 0, sizeof self -> pmnew );

                        /* fill out location */
                        loc . u . blob . size -= col -> csbytes;
                        self -> loc = loc;

                        /* HACK - should open new pm on demand
                           but since the code does not yet support it,
                           disallow further writes */
                        self -> read_only = true;

                        /* mark blob as clean */
                        self -> num_writ = 0;

			/* these must not be a point of failure
			   The only failure from the KMD5FileCommit
			   behind these is on NULL parameter */
			rc = KWColumnDataCommitDone ( & col -> df );
			if ( rc == 0 )
			    rc = KWColumnIdxCommitDone ( & col -> idx );

                        return status;
                    }
                }

                /* revert data fork */
                KWColumnDataFree ( & col -> df,
                    & self -> pmnew, self -> num_writ );
		KMD5FileRevert ( self -> col -> df . fmd5 );
            }
        }
    }

    /* remove checksum bytes */
    self -> num_writ -= col -> csbytes;
    return rc;
}

LIB_EXPORT rc_t CC KColumnBlobCommit ( KColumnBlob *bself )
{
    KColumnBlob *self = ((W_ColumnBlob*)bself) -> getBlob();

    rc_t rc;

    if ( self == NULL )
        return RC ( rcDB, rcBlob, rcCommitting, rcSelf, rcNull );

    if ( self -> loc . id_range == 0 )
        return RC ( rcDB, rcBlob, rcCommitting, rcRange, rcInvalid );

    if ( self -> num_writ == 0 && self -> loc . u . blob . size != 0 )
        return 0;

    assert ( self -> read_only == false );

    rc = KColumnBlobDoCommit ( self );

    if ( rc == kdbReindex )
        rc = KColumnReindex ( & self -> col -> dad );

    return rc;
}
