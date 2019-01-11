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
 * ===========================================================================
 *
 */

#ifndef _hpp_jwt_string_
#define _hpp_jwt_string_

//
// VDB String based imitation of std::string
//

#include <cstddef>

struct String;

namespace ncbi
{

class JwtString
{
public:
    JwtString();
    JwtString(const char* v);
    JwtString(const char* v, size_t size);
    JwtString(const JwtString& v);

    ~JwtString();

    JwtString& operator= (const JwtString& s);

    static const size_t npos = -1;

    bool empty() const;
    const char* data() const;
    const char* c_str() const { return data(); }
    size_t size() const;

    size_t find_first_of (char c, size_t pos = 0) const;
    size_t find_first_of (const char* s, size_t pos = 0) const;

    JwtString substr (size_t pos = 0, size_t len = npos) const;
    void clear();

    int compare (const char* s) const;
    int compare (size_t pos, size_t len, const char* s) const;
    int compare (size_t pos, size_t len, const char* s, size_t n) const;
    int compare (const JwtString& s) const;
    int compare (size_t pos, size_t len, const JwtString& s,
                 size_t subpos, size_t sublen) const;

    size_t find (const char* s, size_t pos = 0) const;

    JwtString& operator+= (const JwtString& s);
    JwtString& operator+= (char c);

    const char& operator[] (size_t pos) const;

private:
    const String * m_str;
};

bool operator<  (const JwtString& lhs, const JwtString& rhs);
JwtString operator+ (const JwtString& lhs, const char*   rhs);
JwtString operator+ (const JwtString& lhs, const JwtString&   rhs);
JwtString operator+ (const JwtString& lhs, char   rhs);

int stoi (const JwtString&  str, size_t* idx = 0, int base = 10);
long long stoll (const JwtString&  str, size_t* idx = 0, int base = 10);
long double stold (const JwtString&  str, size_t* idx = 0);

JwtString to_string( unsigned value );
JwtString to_string( long long value );

}

#endif /* _hpp_jwt_string_ */
