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
* Test stream for mocking HTTP connection for unit testing
*/

#pragma once

#include <string>
#include <list>

class TestStream;
#define KSTREAM_IMPL TestStream
#include <kns/impl.h>

class TestStream
{
public:
    static KStream_vt_v1 vt;

    static rc_t CC Whack ( KSTREAM_IMPL *self );
    static rc_t CC Read ( const KSTREAM_IMPL *self, void *buffer, size_t bsize, size_t *num_read );
    static rc_t CC Write ( KSTREAM_IMPL *self, const void *buffer, size_t size, size_t *num_writ );
    static rc_t CC TimedRead ( const KSTREAM_IMPL *self, void *buffer, size_t bsize, size_t *num_read, struct timeout_t *tm );
    static rc_t CC TimedWrite ( KSTREAM_IMPL *self, const void *buffer, size_t size, size_t *num_writ, struct timeout_t *tm );
    static void AddResponse ( const std :: string& p_str, bool end_binary = false );

    static std :: list < std :: string > m_requests;
    static std :: list < std :: string > m_responses;
};
