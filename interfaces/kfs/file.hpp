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

#ifndef _hpp_kfs_file_
#define _hpp_kfs_file_

#ifndef _h_kfs_file_
#include <kfs/file.h>
#endif

//#include <kfs/md5.h> // KFileMakeMd5ForReadFromString

/*--------------------------------------------------------------------------
 * KFile
 *  a file
 */
struct KFile
{
    /* AddRef
     * Release
     *  ignores NULL references
     */
    inline rc_t AddRef () const throw ()
    { return KFileAddRef ( this ); }

    inline rc_t Release () const throw ()
    { return KFileRelease ( this ); }

    /* RandomAccess
     *  ALMOST by definition, the file is random access
     *
     *  certain file types will refuse random access
     *  these include FIFO and socket based files, but also
     *  wrappers that require serial access ( e.g. compression )
     *
     *  returns 0 if random access, error code otherwise
    */
    inline rc_t RandomAccess () const throw()
    { return KFileRandomAccess ( this ); }


    /* Type
     *  returns a KFileDesc
     *  not intended to be a content type,
     *  but rather an implementation class
     */
    inline uint32_t FileType() const throw()
    { return KFileType ( this ); }


    /* Size
     *  returns size in bytes of file
     *
     *  "size" [ OUT ] - return parameter for file size
     */
    inline rc_t Size ( uint64_t *size ) const throw ()
    { return KFileSize ( this, size ); }

    /* SetSize
     *  sets size in bytes of file
     *
     *  "size" [ IN ] - new file size
     */
    inline rc_t SetSize ( uint64_t size ) throw ()
    { return KFileSetSize ( this, size ); }

    /* Read
     *  read file from known position
     *
     *  "pos" [ IN ] - starting position within file
     *
     *  "buffer" [ OUT ] and "bsize" [ IN ] - return buffer for read
     *
     *  "num_read" [ OUT ] - return parameter giving number of bytes
     *  actually read. when returned value is zero and return code is
     *  also zero, interpreted as end of file.
     */
    inline rc_t Read ( uint64_t pos,
        void *buffer, size_t bsize, size_t *num_read ) const throw ()
    { return KFileRead ( this, pos, buffer, bsize, num_read ); }

    /* Write
     *  write file at known position
     *
     *  "pos" [ IN ] - starting position within file
     *
     *  "buffer" [ IN ] and "size" [ IN ] - data to be written
     *
     *  "num_writ" [ OUT, NULL OKAY ] - optional return parameter
     *  giving number of bytes actually written
     */
    inline rc_t Write ( uint64_t pos,
        const void *buffer, size_t size, size_t *num_writ ) throw ()
    { return KFileWrite ( this, pos, buffer, size, num_writ ); }

    /* MakeStdIn
     *  creates a read-only file on stdin
     */
    static inline rc_t MakeStdIn ( const KFile **std_in ) throw ()
    { return KFileMakeStdIn ( std_in ); }

    /* MakeStdOut
     * MakeStdErr
     *  creates a write-only file on stdout or stderr
     */
    static inline rc_t MakeStdOut ( KFile **std_out ) throw ()
    { return KFileMakeStdOut ( std_out ); }
    static inline rc_t MakeStdErr ( KFile **std_err ) throw ()
    { return KFileMakeStdErr ( std_err ); }
     

    /*/ MD5
    static inline rc_t MakeMd5ForRead ( const KFile **f,
        const KFile *src, const char digest[MD5_DIGEST_SIZE] )
    { return KFileMakeMd5ForReadFromString ( f, src, digest ); }*/

private:
    KFile ();
    ~ KFile ();
    KFile ( const KFile& );
    KFile &operator = ( const KFile& );
};

#endif // _hpp_kfs_file_
