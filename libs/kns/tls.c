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

struct KTLSStream;
#define KSTREAM_IMPL struct KTLSStream

#include <kns/extern.h>
#include <kns/tls.h>
#include <kns/impl.h>
#include <klib/rc.h>
#include <kproc/timeout.h>

#include <os-native.h>

#include <sysalloc.h>

#include "stream-priv.h"

#include <assert.h>

/*--------------------------------------------------------------------------
 * KTLSStream
 */

struct KTLSStream
{
    KStream dad;

    /* hold a reference to invalidated ciphertext object */
    KSocket * ciphertext;

    /* add library specific data here */
};

static
rc_t CC * KTLSStreamWhack ( KTLSStream *self )
{
    /* tear down all of the stuff created during Make */
    /* TBD */

    /* release the ciphertext object */
    KSocketRelease ( self -> ciphertext );
    self -> ciphertext = NULL;

    /* done */
    free ( self );
    return 0;
}

static
rc_t CC * KTLSStreamRead ( const KTLSStream * self,
    void * buffer, size_t bsize, size_t * num_read )
{
    /* TBD */
}

static
rc_t CC * KTLSStreamWrite ( KTLSStream * self,
    const void * buffer, size_t size, size_t * num_writ )
{
    /* TBD */
}

static
rc_t CC * KTLSStreamTimedRead ( const KTLSStream *self,
    void * buffer, size_t bsize, size_t * num_read, struct timeout_t * tm )
{
    /* TBD */
    return KTLSStreamRead ( self, buffer, bsize, num_read );
}

static
rc_t CC * KTLSStreamTimedWrite ( KTLSStream *self,
    const void * buffer, size_t size, size_t * num_writ, struct timeout_t * tm )
{
    /* TBD */
    return KTLSStreamWrite ( self, buffer, size, num_writ );
}


static KStream_vt_v1 vtKTLSStream =
{
    1, 1,

    KTLSStreamWhack,
    KTLSStreamRead,
    KTLSStreamWrite,
    KTLSStreamTimedRead,
    KTLSStreamTimedWrite
};

/* MakeTLSStream
 *  create a TLS wrapper upon an existing KStream
 *
 *  data read from or written to TLS stream is in plaintext
 *  data from to or wrapped stream is in ciphertext
 *
 *  "plaintext" [ OUT ] - return parameter for TLS stream
 *
 *  "ciphertext" [ IN ] - existing stream of ciphertext
 *
 *  "host" [ IN ] - host dns name
 */
LIB_EXPORT rc_t CC KNSManagerMakeTLSStream ( const KNSManager * self,
    KTLSStream ** plaintext, const KSocket * ciphertext, const String * host )
{
    rc_t rc;

    if ( plaintext == NULL )
        rc = RC ();
    else
    {
        if ( self == NULL )
            rc = RC ();
        else if ( ciphertext == NULL )
            rc = RC ();
        else
        {
            /* use private function to steal socket from ciphertext,
               which should invalidate it from being directly used
               thereafter; but this is necessary anyway */

            /* create KTLSStream object and initialize with existing
               connection. Will need to #ifdef for Windows vs. Unix */

            /* perform all initialization possible with information given */
        }

        * plaintext = NULL;
    }

    return rc;
}


/* AddRef
 * Release
 *  ignores NULL references
 */
LIB_EXPORT rc_t CC KTLSStreamAddRef ( const KTLSStream * self )
{
    /* works because address of self == address of self -> dad,
       and if self == NULL, & self -> dad == NULL */
    return KStreamAddRef ( & self -> dad );
}

LIB_EXPORT rc_t CC KTLSStreamRelease ( const KTLSStream * self )
{
    /* works because address of self == address of self -> dad,
       and if self == NULL, & self -> dad == NULL */
    return KStreamRelease ( & self -> dad );
}

/* VerifyCACert
 */
LIB_EXPORT rc_t CC KTLSStreamVerifyCACert ( const KTLSStream * self )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ();
    else
    {
        /* TBD */
        rc = 0;
    }

    return rc;
}


/* GetStream
 *  reference-counted cast operation
 *  creates a reference to a KStream from a KTLSStream
 */
LIB_EXPORT rc_t CC KTLSStreamGetStream ( const KTLSStream * self,
    struct KStream ** strm )
{
    rc_t rc;

    if ( strm == NULL )
        rc = RC ();
    else
    {
        if ( self == NULL )
            rc = RC ();
        else if ( self -> dad . vt != & vtKTLSStream )
            rc = RC ();
        else
        {
            /* at this point, the object must be in a fully
               constructed and validated state. If not, it
               is an error to access it as a stream. */

            rc = KStreamAddRef ( & self -> dad );
            if ( rc == 0 )
            {
                * strm = & ( ( KTLSStream * ) self ) -> dad;
                return 0;
            }
        }

        * strm = NULL;
    }

    return rc;
}

