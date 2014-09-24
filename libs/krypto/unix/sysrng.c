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

#include <krypto/extern.h>
#include <krypto/rng.h>
#include "rng-priv.h"

#include <kfs/directory.h>
#include <kfs/file.h>

#include <klib/rc.h>
#include <klib/text.h>
#include <klib/printf.h>
#include <klib/checksum.h>


#include <time.h>

#include <unistd.h> /* unix specific so unix specific header is ok too */
#include <string.h>

rc_t KRngSysEntropy (KRng * self, uint8_t * buffer, size_t buff_size)
{
    MD5State state;
    KDirectory * dir;
    const KFile * file;
    uint64_t file_pos;
    rc_t rc;

    if (self == NULL)
        return RC (rcKrypto, rcRng, rcWriting, rcSelf, rcNull);
    if (buffer == NULL)
        return RC (rcKrypto, rcRng, rcWriting, rcParam, rcNull);

    rc = KDirectoryNativeDir (&dir);
    if (rc == 0)
    {
        rc = KDirectoryOpenFileRead (dir, &file, "/dev/random");
        if (rc)
            file = NULL;
    }

    file_pos = 0;
    MD5StateInit (&state);

    while (buff_size > 0)
    {

        if (file)
        {
            size_t to_read;
            size_t num_read;

            to_read = (buff_size > 16) ? 16 : buff_size;

            rc = KFileRead (file, file_pos, buffer, to_read, &num_read);
            if (rc == 0)
            {
                buffer += num_read;
                buff_size -= num_read;
                file_pos += num_read; /* superfluous? */
            }
        }
        if (buff_size > 0)
        {
            uint8_t digest [16];
            char buff [1024];
            size_t ii;

            string_printf (buff, sizeof (buff), &ii,
                           "%p%zu%lu%lu%lu%lu%s%s%lu",
                           buffer, buff_size, (uint64_t)clock(),
                           (uint64_t)time(NULL), (uint64_t)getpid(), 
                           (uint64_t)getuid(), getlogin(), ttyname(0),
                           (uint64_t)getgid());

            MD5StateAppend (&state, buff, sizeof buff);
            MD5StateFinish (&state, digest);

            ii = (buff_size < sizeof (digest)) ? buff_size : sizeof (digest);
            memcpy (buffer, digest, ii);
            buff_size -= ii;
            buffer += ii;
        }
    }

    KFileRelease (file);
    KDirectoryRelease (dir);
    
    return 0;
}
