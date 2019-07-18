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

#include <klib/printf.h> /* string_printf */
#include <klib/rc.h>
#include <klib/sra-release-version.h>

#include "release-vers.h"

#include <ctype.h> /* isdigit */
#include <os-native.h>
#include <string.h> /* memset */
#include <assert.h>

LIB_EXPORT rc_t CC SraReleaseVersionGet(SraReleaseVersion *version) {
    if (version == NULL) {
        return RC(rcRuntime, rcArgv, rcAccessing, rcParam, rcNull);
    }

    version->version = RELEASE_VERS;
    version->revision = RELEASE_REVISION;

    switch (RELEASE_TYPE) {
        case 'a':
            version->type = eSraReleaseVersionTypeAlpha;
            break;
        case 'b':
            version->type = eSraReleaseVersionTypeBeta;
            break;
        case 'c':
            version->type = eSraReleaseVersionTypeRC;
            break;
        case 'd':
            version->type = eSraReleaseVersionTypeDev;
            break;
        default:
            version->type = eSraReleaseVersionTypeFinal;
    }

    return 0;
}

LIB_EXPORT rc_t CC SraReleaseVersionCmp(const SraReleaseVersion *self,
    const SraReleaseVersion *version2, int32_t *result)
{
    if (result == NULL) {
        return RC(rcRuntime, rcArgv, rcAccessing, rcParam, rcNull);
    }

    *result = 0;

    if (self == NULL) {
        return RC(rcRuntime, rcArgv, rcAccessing, rcSelf, rcNull);
    }

    if (version2 == NULL) {
        return RC(rcRuntime, rcArgv, rcAccessing, rcParam, rcNull);
    }

    if (self->version < version2->version) {
        *result = 1;
    }
    else if (self->version > version2->version) {
        *result = -1;
    }
    else if (self->type < version2->type) {
        *result = 1;
    }
    else if (self->type > version2->type) {
        *result = -1;
    }
    else if (self->revision < version2->revision) {
        *result = 1;
    }
    else if (self->revision > version2->revision) {
        *result = -1;
    }
    else {
        *result = 0;
    }

    return 0;
}

static int32_t getdigit(const char **s, size_t *size, int32_t max) {
    int32_t d = 0;
    char c = 0;

    assert(s && size);

    if (*size == 0) {
        return 0;
    }

    c = **s;
    if (!isdigit(c)) {
        if (c == '\n') {
            return 0;
        }
        else {
            return -1;
        }
    }

    while (*size > 0) {
        char c = **s;
        if (isdigit(c)) {
            d = d * 10 + c - '0';
        }
        else {
            break;
        }
        ++(*s);
        --*size;
    }

    if (max > 0 && d > max) {
        return -2;
    }

    return d;
}

#define SET_RC(i) ( \
    (i) == -1 ? RC(rcRuntime, rcFile, rcReading, rcFile, rcInvalid) \
              : RC(rcRuntime, rcFile, rcReading, rcNumeral, rcExcessive) )

/* Parse
 *  Initialize SraReleaseVersion from char version[size]
 */
LIB_EXPORT rc_t CC SraReleaseVersionInit ( SraReleaseVersion *self,
    const char *version, size_t size )
{
    rc_t rc = 0;
    int32_t major = 0, minor = 0, release = 0, revision = 0, i = 0,
        type = eSraReleaseVersionTypeFinal; 
    const char *p = version;

    if (self == NULL) {
        return RC(rcRuntime, rcArgv, rcAccessing, rcSelf, rcNull);
    }

    assert(self);
    memset(self, 0, sizeof *self);

    self->type = eSraReleaseVersionTypeFinal;

    if (size == 0) {
        return 0;
    }

    /* major: 1. */
    if (size > 0 && rc == 0) {
        int32_t i = getdigit(&p, &size, 255);
        if (i >= 0) {
            major = i;
        }
        else {
            rc = SET_RC(i);
        }
    }

    /* minor: 1.2 */
    if (size > 0 && rc == 0 && *p != '\n') {
        if (*p != '.' && *p != '-') {
            rc = RC(rcRuntime, rcFile, rcReading, rcFile, rcInvalid);
        }
        if (*p == '.') {
            ++p;
            --size;
            i = getdigit(&p, &size, 255);
            if (i >= 0) {
                minor = i;
            }
            else {
                rc = SET_RC(i);
            }
        }
    }

    /* release: 1.2.3 */
    if (size > 0 && rc == 0 && *p != '\n') {
        if (*p != '.' && *p != '-') {
            rc = RC(rcRuntime, rcFile, rcReading, rcFile, rcInvalid);
        }
        if (*p == '.') {
            ++p;
            --size;
            i = getdigit(&p, &size, 0xFFFF);
            if (i >= 0) {
                release = i;
            }
            else {
                rc = SET_RC(i);
            }
        }
    }

    /* dash: 1.2.3- */
    if (size > 0 && rc == 0 && *p != '\n') {
        if (*p != '-') {
            rc = RC(rcRuntime, rcFile, rcReading, rcFile, rcInvalid);
        }
        ++p;
        --size;
    }

    /* type: 1.2.3-a 1.2.3-rc */
    if (size > 0 && rc == 0 && *p != '\n') {
        if (!isalpha(*p)) {
            if (!isdigit(*p)) {
                rc = RC(rcRuntime, rcFile, rcReading, rcFile, rcInvalid);
            }
        }
        else {
            switch (*p) {
                case 'a':
                    type = eSraReleaseVersionTypeAlpha;
                    break;
                case 'b':
                    type = eSraReleaseVersionTypeBeta;
                    break;
                case 'r':
                    type = eSraReleaseVersionTypeRC;
                    ++p;
                    --size;
                    if (size == 0) {
                        rc = RC(rcRuntime,
                            rcFile, rcReading, rcFile, rcInvalid);
                    }
                    else if (*p != 'c') {
                        rc = RC(rcRuntime,
                            rcFile, rcReading, rcFile, rcInvalid);
                    }
                    break;
            }
            ++p;
            --size;
        }
    }

    /* revision: 1.2.3-4 1.2.3-b4 1.2.3-rc4 */
    if (size > 0 && rc == 0 && *p != '\n') {
        i = getdigit(&p, &size, 0);
        if (i >= 0) {
            revision = i;
        }
        else {
            rc = SET_RC(i);
        }
    }

    if (size > 0 && rc == 0 && *p != '\n') {
        rc = RC(rcRuntime, rcFile, rcReading, rcFile, rcInvalid);
    }

    if (rc == 0) {
        self->version = (major << 24) + (minor << 16) + release;
        self->revision = revision;
        self->type = type;
    }

    return rc;
}

LIB_EXPORT rc_t CC SraReleaseVersionPrint ( const SraReleaseVersion *self,
    char *version, size_t size, size_t *num_writ )
{
    rc_t rc = 0;
    const char *type = "";

    if (self == NULL) {
        return RC(rcRuntime, rcArgv, rcAccessing, rcSelf, rcNull);
    }

    switch (self->type) {
        case eSraReleaseVersionTypeDev:
            type = "-dev";
            break;
        case eSraReleaseVersionTypeAlpha:
            type = "-a";
            break;
        case eSraReleaseVersionTypeBeta:
            type = "-b";
            break;
        case eSraReleaseVersionTypeRC:
            type = "-rc";
            break;
        case eSraReleaseVersionTypeFinal:
            if (self->revision == 0) {
                type = "";
            }
            else {
                type = "-";
            }
            break;
        default:
            assert(0);
            break;
    }

    if (self->revision == 0) {
        rc = string_printf(version, size, num_writ,
            "%.3V%s", self->version, type);
    }
    else {
        rc = string_printf(version, size, num_writ,
            "%V%s%d", self->version, type, self->revision);
    }

    return rc;
}
