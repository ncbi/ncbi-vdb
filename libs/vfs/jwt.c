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

#include <kfs/directory.h> /* KDirectoryRelease */
#include <kfs/file.h> /* KFileRelease */
#include <klib/rc.h> /* RC */
#include "jwt.h" /* JwtKartValidateFile */

#define RELEASE(type, obj) do { rc_t rc2 = type##Release(obj); \
    if (rc2 && !rc) { rc = rc2; } obj = NULL; } while (false)

#define MIN_SIZE 5
#define MAX_SIZE 20000000

static rc_t JwtKartValidateSize(uint64_t size) {
    if (size == 0)
        return RC(rcVFS, rcQuery, rcValidating, rcFile, rcEmpty);
    else if (size < MIN_SIZE)
        return RC(rcVFS, rcQuery, rcValidating, rcFile, rcInsufficient);
    else if (size > MAX_SIZE)
        return RC(rcVFS, rcQuery, rcValidating, rcFile, rcExcessive);
    else
        return 0;
}

#define VALID(c) ( ( (c) >= 'a' && (c) <= 'z' ) || \
                   ( (c) >= 'A' && (c) <= 'Z' ) || \
                   ( (c) >= '0' && (c) <= '9' ) || (c) == '-' || (c) == '_' )

rc_t JwtKartValidateString(const String * cart) {
    rc_t rc = 0;

    int MIN1 = 1, MIN2 = 1, MIN3 = 1;

    int dots = 0;

    size_t j = 0;

    if (cart == NULL || cart->addr == NULL)
        return RC(rcVFS, rcQuery, rcValidating, rcParam, rcNull);

    rc = JwtKartValidateSize(cart->size);

    if (rc == 0) {
        size_t i = 0;
        /* section I before first '.' */
        for (i = 0, j = 0; i < cart->size; ++i, ++j)
            if (!VALID(cart->addr[i])) {
                if (cart->addr[i] == '.') {
                    if (j < MIN1)
                        return RC(rcVFS,
                            rcQuery, rcValidating, rcChar, rcUnexpected);
                    ++dots;
                    break;
                }
                else
                    return RC(rcVFS, rcQuery, rcValidating, rcChar, rcInvalid);
            }

        /* section II after first '.' */
        for (j = 0, ++i; i < cart->size; ++i, ++j)
            if (!VALID(cart->addr[i])) {
                if (cart->addr[i] == '.') {
                    if (j < MIN2)
                        return RC(rcVFS,
                            rcQuery, rcValidating, rcChar, rcUnexpected);
                    ++dots;
                    break;
                }
                else
                    return RC(rcVFS, rcQuery, rcValidating, rcChar, rcInvalid);
            }

        /* section III after third '.' */
        for (j = 0, ++i; i < cart->size; ++i, ++j)
            if (!VALID(cart->addr[i]))
                break;
        /* trailing EOL-s */

        if (j < MIN3)
            return RC(rcVFS,
                rcQuery, rcValidating, rcChar, rcUnexpected);

        for (; i < cart->size; ++i)
            if (cart->addr[i] != '\r' && cart->addr[i] != '\n')
                return RC(rcVFS, rcQuery, rcValidating, rcChar, rcInvalid);
    }

    if (dots != 2)
        return RC(rcVFS, rcQuery, rcValidating, rcChar, rcNotFound);

    return rc;
}

rc_t JwtKartValidateFile(const char * path) {
    rc_t rc = 0;

    KDirectory * dir = NULL;

    const KFile * f = NULL;

    char * buffer = NULL;

    uint64_t size = ~0;

    if (path == NULL)
        return RC(rcVFS, rcQuery, rcValidating, rcParam, rcNull);

    rc = KDirectoryNativeDir(&dir);

    if (rc == 0)
        if ((KDirectoryPathType(dir, "%s", path) & ~kptAlias) != kptFile)
            rc = RC(rcVFS, rcQuery, rcValidating, rcFile, rcIncorrect);

    if (rc == 0)
        rc = KDirectoryOpenFileRead(dir, &f, "%s", path);

    if (rc == 0)
        rc = KFileSize(f, &size);

    if (rc == 0)
        rc = JwtKartValidateSize(size);

    if (rc == 0) {
        buffer = malloc(size);
        if (buffer == NULL)
            rc = RC(rcVFS, rcQuery, rcValidating, rcMemory, rcExhausted);
    }

    if (rc == 0)
        rc = KFileReadExactly(f, 0, buffer, size);

    if (rc == 0) {
        String s;
        StringInit(&s, buffer, size, size);
        rc = JwtKartValidateString(&s);
    }

    free(buffer);

    RELEASE(KFile, f);

    RELEASE(KDirectory, dir);

    return rc;
}

/******************************************************************************/
