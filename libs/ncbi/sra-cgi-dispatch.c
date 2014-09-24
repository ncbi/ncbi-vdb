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
 * =============================================================================
 *
 */

#include <ncbi/kimpl-sra-cgi.h>

/******************************************************************************/

ssize_t SraCgiRequestRead
    (SraCgiRequest *self, void *buffer, size_t buffer_size)
{
    if (self == NULL || buffer == NULL || buffer_size == 0) {
        return 0;
    }

    switch (self->vt->v1.maj) {
        case 1:
            return (*self->vt->v1.read) (self, buffer, buffer_size);
    }

    self->err = 1002;
    return -2;
}

int SraCgiResponseStatus(SraCgiResponse *self, unsigned int code)
{
    if (self == NULL) {
        return 0;
    }

    switch (self->vt->v1.maj) {
        case 1:
            (*self->vt->v1.status) (self, code);
            return 0;
    }

    self->err = 1002;
    return -2;
}

int SraCgiResponseHeader(SraCgiResponse *self, const char *name,
    const void *buffer, size_t bytes)
{
    if (self == NULL || name == NULL || name[0] == '\0'
        || buffer == NULL || bytes == 0)
    {
        return 0;
    }

    switch (self->vt->v1.maj) {
        case 1:
            (*self->vt->v1.header) (self, name, buffer, bytes);
            return 0;
    }

    self->err = 1002;
    return -2;
}

ssize_t SraCgiResponseWrite
    (SraCgiResponse *self, const void *buffer, size_t buffer_size)
{
    if (self == NULL || buffer == NULL || buffer_size == 0) {
        return 0;
    }

    switch (self->vt->v1.maj) {
        case 1:
            return (*self->vt->v1.write) (self, buffer, buffer_size);
    }

    self->err = 1002;
    return -2;
}
