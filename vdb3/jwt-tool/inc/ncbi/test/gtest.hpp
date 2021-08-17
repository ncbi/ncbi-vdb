/*==============================================================================
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
 *  Author: Kurt Rodarmer
 *
 * ===========================================================================
 *
 */

#pragma once

#include <ncbi/secure/except.hpp>

#include <gtest/gtest.h>


#define SEE_EXCEPTION_INFO 1
#define SEE_ANY_EXCEPTION_INFO 0

#if SEE_EXCEPTION_INFO

#undef EXPECT_THROW
#define EXPECT_THROW( stmt, except )                                    \
    try { stmt; } catch ( except & x ) {} catch ( Exception & x )       \
    { std :: cerr << __func__ << ": " << typeid ( x ) . name ()         \
                  << " at " << x . file () << ':' << x . line ()        \
                  << '(' << x . function () << ") - " << x . what ()    \
                  << '\n' << XBackTrace ( x ); throw; } ( void ) 0

#if SEE_ANY_EXCEPTION_INFO

#undef EXPECT_ANY_THROW
#define EXPECT_ANY_THROW( stmt )                                        \
    try { stmt; } catch ( Exception & x )                               \
    { std :: cerr << __func__ << ": " << typeid ( x ) . name ()         \
                  << " at " << x . file () << ':' << x . line ()        \
                  << '(' << x . function () << ") - " << x . what ()    \
                  << '\n'; } catch ( ... ) {} ( void ) 0
#endif

#undef EXPECT_NO_THROW
#define EXPECT_NO_THROW( stmt )                                         \
    try { stmt; } catch ( Exception & x )                               \
    { std :: cerr << __func__ << ": " << typeid ( x ) . name ()         \
                  << " at " << x . file () << ':' << x . line ()        \
                  << '(' << x . function () << ") - " << x . what ()    \
                  << '\n' << XBackTrace ( x ); throw; } ( void ) 0

#endif
