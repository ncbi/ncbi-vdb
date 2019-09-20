/*===========================================================================
*
*                            Public Domain Notice
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

#ifndef _h_vfs_path_priv_
#define _h_vfs_path_priv_

#ifndef _h_vfs_extern_
#include <vfs/extern.h>
#endif

#ifndef _h_vfs_path_
#include <vfs/path.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------
 * forwards
 */
struct String;


#define NCBI_FILE_SCHEME       "ncbi-file"
#define NCBI_ACCESSION_SCHEME  "ncbi-acc"
#define HTTP_SCHEME            "http"
#define FTP_SCHEME             "ftp"
#define FASP_SCHEME            "fasp"
#define NCBI_LEGREFSEQ_SCHEME  "x-ncbi-legrefseq"
#define NCBI_OBJECT_SCHEME     "ncbi-obj"

/* options for a VPath possibly obtained from a query string on an URI */
typedef uint32_t VPOption_t;
enum eVPOption_t
{
    vpopt_encrypted,
    vpopt_pwpath,
    vpopt_pwfd,
    vpopt_readgroup,
#if 0    
    vpopt_temporary_pw_hack,
#endif    
    vpopt_vdb_ctx,
    vpopt_gap_ticket,
    vpopt_gap_prjId,
    vpopt_count
};


/* =====
 * Much of what follows is expected to move into the interface vfs/path.h
 * once it becomes supported.
 */

/* MakeFmt
 *  make a path object from a format string plus arguments
 *
 *  "new_path" [ OUT ] - a reference to the new object.
 *
 *  "fmt" [ IN ] and "args" [ IN ] - arguments to string_printf
 *  ( see <klib/text.h> ) to build a NUL-terminated string
 *  that conforms with the rules for "posix_path"
 *
 * NB - SECURITY RISK IF USED DIRECTLY FROM EXTERNAL STRINGS.
 *      ALSO, FMT IS **NOT** PRINTF COMPATIBLE - see string_printf.
 */

#if 0
VFS_EXTERN rc_t CC VPathMakeRelative ( VPath ** new_path, const VPath * base_path,
                                       const char * relative_path );
VFS_EXTERN rc_t CC VPathMakeRelativeFmt ( VPath ** new_path, const VPath * base_path,
                                          const char * fmt, ... );
VFS_EXTERN rc_t CC VPathVMakeRelativeFmt ( VPath ** new_path, const VPath * base_path,
                                           const char * fmt, va_list args );
VFS_EXTERN rc_t CC VPathMakeCurrentPath ( VPath ** new_path );

VFS_EXTERN rc_t CC VPathMakeURI ( VPath ** new_path, const char * uri );
#endif

VFS_EXTERN bool CC VPathHasRefseqContext ( const VPath * accession );


/* Option
 *  rc == 0 if the option has been specified
 *  for options with a parameter, the value of the parameter is copied to buffer
 */
VFS_EXTERN rc_t CC VPathOption ( const VPath * self, VPOption_t option,
                                char * buffer, size_t buffer_size,
                                size_t * num_read);

/* SetParentAcc
 * set accession of parent DB for refseqs
 */
rc_t VPathSetAccOfParentDb(VPath * self, const struct String * acc);

#ifdef __cplusplus
}
#endif

#endif /* _h_vfs_path_priv_ */

/*--------------------------------------------------------------------------
 *
 * The ABNF form described in RFC5234 will be used to define URLS and paths
 * with some handwaving instead of complete character definitions for case 
 * insensitivity.  CORE definitions from RFC5234 will be used again handwaving
 * case insensitivity.
 *
 * --------------------
 *
 * URL for a VPath
 *
 *  as per RFC 3986, an URI consists of:
 *
 *    URI           = scheme ":" heir-part [ "?" query ] [ "#" fragment ]
 *
 *  We will support however something intbetween an URI and an IRI in that we'll
 *  all UTF-8 rather than limit certain characters to ASCII.
 *
 *  For the NCBI path  URL:  The hier-part is intended to be compatible with the 
 *                    "file:" scheme  Authority on any of the Unix-like operating
 *                    systems must either be empty or "localhost".  For Windows
 *                    it must  be a host that can be used if the "file" url is
 *                    translated into a UNC style Windows path.  Also in a
 *                    Windows vfs: URL a single colon ':' will be allowed only at
 *                    the end of a single character rive letter for the first
 *                    sub-part of the path.
 *
 *    scheme        = "ncbi-file" ; ( case insensitive )
 *
 *    hier-part     = "//" authority path-abempty
 *                  / path-absolute
 *                  / path-relative
 *                  / path-empty
 *
 * by RFC 3986 authority is
 *    authority     = [ userinfo "@" ] host [ ":" port]
 * but at this point we only recognize
 *    authority     = host
 *
 * by RFC 3986 host is
 *    host          = IP-literal / IPv4address / reg-name
 * but at this point we only recognize
 *    host          = reg-name
 *
 *    reg-name      = *( unreserved / pct-encoded / sub-delims )
 *
 *    path          = path-abempty
 *                  / path-absolute
 *                  / path-noscheme
 *                  / path-rootless
 *                  / path-empty
 *
 *    path-abempty  = * ( "/" segment )
 *
 *    path-absolute = "/" segment-nz *( "/" segment )
 *
 *    path-noscheme = segment-nz-nc *( "/" segment )
 *
 *    path-rootless = segment-nz *( "/" segment )
 *
 *    path-empty    = ""
 *
 *    segment       = *pchar       ; can be empty
 *    segment-nz    = 1*pchar      ; can't be empty
 *    segment-nz-nc = 1*pchar-nz   ; can't be empty
 *
 *    pchar         = ":" / pchar-nc
 *
 *    pchar-nc      = unreserved / pct-encoded / sub-delims ? "@"
 *
 *    pct-encoded   = "%" HEXDIG HEXDIG ; hex digits are 0-9, a-f or A-F
 *
 *    authority   = "localhost" / host-name ; the host name is an O/S specific 
 *                                          ; name of a remote host 
 *
 *    query       = query_entry [ * ( "&" query_entry ) ]
 *
 *    query_entry = "encrypt" / "enc" / ( "pwfile=" hier-part ) / ( "pwfd=" fd )
 *
 *    fd          = 1* DIGIT
 *
 *    unreserved  = ALPHA / DIGIT / "-" / "." / "_" / "~" / UTF-8
 *
 *    reserved    = gen-delims / subdelims
 *
 *    gen-delims  = ":" / "/" / "?" / "#" / "[" / "]" / "@"
 *
 *    sub-delims  = "!" / "$" / "&" / "'" / "(" / ")" / "*"
 *                / "+" / "," / ";" / "="
 *
 * Handwaving on the UTF-8.  We'll accept it rather than requiring percent encoding
 * in most cases.

 * Examples:
 *         "ncbi-file://home/my-name/data-files"
 *         "ncbi-file://win-server/archive/secure/read12345?encrypted"
 *         "ncbi-file:///c:/scanned-data/0001/file.sra?enc?pwd-file=c:/Users/JamesMcCoy/ncbi.pwd"
 *
 *  We allow an implied scheme of "vfs" if none present.
 *  'host-name' is partially implemented for Windows.  Not currently for any the
 *  of the supported Unix systems.  We do not support IP addresses instead of 
 *  host names.
 *
 *  'path' can be either absolute or relative and must be posix style as per 
 *  RFC 3986 and matches the "file" scheme.
 *
 *  'fragment' is not yet implemented or defined.
 *
 *  'fd' is a system specific file handle.  Not yet supported on Windows.
 *
 * --------------------
 *
 * VFS posix_path representation:
 *
 * The internal representation of a path for VFS resembles very closely the
 * POSIX pathname crossed with the Windows UNC path.  Only the Unix "/" 
 * separator is allowed not the "\" for paths.
 *
 * posix-path     = full-path / relative-path
 *
 * full-path      = ["//" host] "/" [ directory-path "/" ] resource
 *
 * relative-path  = [ directory-path "/" ] resource
 *
 * directory-path = resource [ "/" directory-path ]
 *
 * resource       = string ; (UTF-8 O/S specific name for a directory or a file)
 *
 * host           = string ; (UTF-8 O/S specific name for the local or remote host)
 *
 * For a host "localhost" is a synonym for the current host.
 * The "//host" is not guaranteed to work for all systems except "//localhost"
 *
 * The resource "." is assumed to mean the local directory and will be stripped
 * when a path is made canonical.
 *
 * The resource ".." is assumed to mean the containing directory and when made 
 * canonical it will be left in at the begining for relative paths or removed along 
 * with the preceding resource name.
 *
 * For Windows the device is handled in a unique way with some potential for ambiguity.
 * The UNC approach for a named drive "C:" becomes "/c".  So "C:\" becomes "/c/".
 *
 * In the future the complex approach from the "file:" URL should be adopted 
 * with the ":" allowed only in the first directory name in a path.  RFC 3986 
 * notes though that this complex approach puts some five variants into the ABNF
 * to describe the path portion of an URL.
 */
