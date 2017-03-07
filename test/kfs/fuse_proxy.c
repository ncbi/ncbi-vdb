/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>
  Copyright (C) 2011       Sebastian Pipping <sebastian@pipping.org>

  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.

  gcc -Wall fusexmp.c `pkg-config fuse --cflags --libs` -o fusexmp
*/

#define FUSE_USE_VERSION 26

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef linux
/* For pread()/pwrite()/utimensat() */
#define _XOPEN_SOURCE 700
#endif

#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#endif

#define PREFIX_SIZE 4096
char prefix[ PREFIX_SIZE ];
int prefix_len;
off_t prefix_truncate_limit = 0;
off_t prefix_write_limit = 0;

static void set_prefix_truncate_limit( off_t new_limit )
{
    prefix_truncate_limit = new_limit;
}

static bool above_truncate_limit( off_t value )
{
    return ( ( prefix_truncate_limit > 0 ) && ( value >= prefix_truncate_limit ) );
}

static void set_prefix_write_limit( off_t new_limit )
{
    prefix_write_limit = new_limit;
}

static bool above_write_limit( off_t value )
{
    return ( ( prefix_write_limit > 0 ) && ( value >= prefix_write_limit ) );
}

static int set_prefix( const char * new_prefix )
{
    char buffer[ PREFIX_SIZE ];
    char *abs_path = realpath( new_prefix, buffer );

    prefix[ 0 ] = 0;
    prefix_len = 0;
    if ( abs_path != NULL )
    {
        int new_prefix_len = strlen( abs_path );
        if ( new_prefix_len < PREFIX_SIZE )
        {
            strncpy( prefix, new_prefix, new_prefix_len );
            prefix[ new_prefix_len ] = 0;
            prefix_len = new_prefix_len;
        }
    }
    return prefix_len;
}

static const char * make_prefixed_path( const char * path )
{
    int path_len = strlen( path );
    int prefixed_len = prefix_len + path_len;
    char * res = malloc( prefixed_len + 1 );
    strncpy( res, prefix, prefix_len );
    strncpy( &( res[ prefix_len ] ), path, path_len );
    res[ prefixed_len ] = 0;
    return res;
}

static void release_prefixed_path( const char * path )
{
    free( ( void * ) path );
}

static size_t str_2_size( const char * s )
{
    size_t res = 0;
    if ( s != NULL )
    {
        size_t l = strlen( s );
        if ( l > 0 )
        {
            size_t multipl = 1;
            switch( s[ l - 1 ] )
            {
                case 'k' :
                case 'K' : multipl = 1024; break;
                case 'm' :
                case 'M' : multipl = 1024 * 1024; break;
                case 'g' :
                case 'G' : multipl = 1024 * 1024 * 1024; break;
            }

            {
                char * endptr;
                res = strtol( s, &endptr, 0 ) * multipl;
            }
        }
    }
    return res;
}

static size_t used_space_of_file( const char * path )
{
    size_t res = 0;
    struct stat file_status;
    int stat_res = stat( path, &file_status );
    if ( stat_res == 0 )
        res = ( file_status.st_blocks * 512 );
    return res;
}

/* -------------------------------------------------------------------------- */
static int xmp_getattr( const char *path, struct stat *stbuf )
{
	int res = lstat( path, stbuf );
	if ( res == -1 ) return -errno;
	return 0;
}

static int xmp_prefixed_getattr( const char *path, struct stat *stbuf )
{
    const char *prefixed_path = make_prefixed_path( path );
	int res = xmp_getattr( prefixed_path, stbuf );
    release_prefixed_path( prefixed_path );
	return res;
}

/* -------------------------------------------------------------------------- */
static int xmp_access( const char * path, int mask )
{
	int res = access( path, mask );
	if ( res == -1 ) return -errno;
	return 0;
}

static int xmp_prefixed_access( const char * path, int mask )
{
    const char *prefixed_path = make_prefixed_path( path );
	int res = xmp_access( prefixed_path, mask );
    release_prefixed_path( prefixed_path );
	return res;
}

/* -------------------------------------------------------------------------- */
static int xmp_readlink( const char *path, char *buf, size_t size )
{
	int res = readlink( path, buf, size - 1 );
	if ( res == -1 ) return -errno;
	buf[ res ] = '\0';
	return 0;
}

static int xmp_prefixed_readlink( const char *path, char *buf, size_t size )
{
    const char *prefixed_path = make_prefixed_path( path );
	int res = xmp_readlink( prefixed_path, buf, size );
    release_prefixed_path( prefixed_path );
	return res;
}

/* -------------------------------------------------------------------------- */
static int xmp_readdir( const char *path, void *buf, fuse_fill_dir_t filler,
                        off_t offset, struct fuse_file_info *fi )
{
	DIR *dp;
	struct dirent *de;

	(void) offset;
	(void) fi;

	dp = opendir( path );
	if ( dp == NULL ) return -errno;

	while ( ( de = readdir( dp ) ) != NULL )
    {
		struct stat st;
		memset( &st, 0, sizeof( st ) );
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;
		if ( filler( buf, de->d_name, &st, 0 ) )
			break;
	}

	closedir( dp );
	return 0;
}

static int xmp_prefixed_readdir( const char *path, void *buf, fuse_fill_dir_t filler,
                        off_t offset, struct fuse_file_info *fi )
{
    const char *prefixed_path = make_prefixed_path( path );
	int res = xmp_readdir( prefixed_path, buf, filler, offset, fi );
    release_prefixed_path( prefixed_path );
	return res;
}

/* -------------------------------------------------------------------------- */
static int xmp_mknod( const char *path, mode_t mode, dev_t rdev )
{
	int res;

	/* On Linux this could just be 'mknod(path, mode, rdev)' but this
	   is more portable */
	if ( S_ISREG( mode ) )
    {
		res = open( path, O_CREAT | O_EXCL | O_WRONLY, mode );
		if ( res >= 0 ) res = close( res );
	}
    else if ( S_ISFIFO( mode ) )
    {
		res = mkfifo( path, mode );
    }
	else
    {
		res = mknod( path, mode, rdev );
    }
	if ( res == -1 ) return -errno;

	return 0;
}

static int xmp_prefixed_mknod( const char *path, mode_t mode, dev_t rdev )
{
    const char *prefixed_path = make_prefixed_path( path );
	int res = xmp_mknod( prefixed_path, mode, rdev );
    release_prefixed_path( prefixed_path );
	return res;
}

/* -------------------------------------------------------------------------- */
static int xmp_mkdir( const char *path, mode_t mode )
{
	int res;
	res = mkdir( path, mode );
	if ( res == -1 ) return -errno;
	return 0;
}

static int xmp_prefixed_mkdir( const char *path, mode_t mode )
{
    const char *prefixed_path = make_prefixed_path( path );
	int res = xmp_mkdir( prefixed_path, mode );
    release_prefixed_path( prefixed_path );
	return res;
}

/* -------------------------------------------------------------------------- */
static int xmp_unlink( const char *path )
{
	int res;
	res = unlink( path );
	if ( res == -1 ) return -errno;
	return 0;
}

static int xmp_prefixed_unlink( const char *path )
{
    const char *prefixed_path = make_prefixed_path( path );
	int res = xmp_unlink( prefixed_path );
    release_prefixed_path( prefixed_path );
	return res;
}

/* -------------------------------------------------------------------------- */
static int xmp_rmdir( const char *path )
{
	int res;
	res = rmdir( path );
	if ( res == -1 ) return -errno;
	return 0;
}

static int xmp_prefixed_rmdir( const char *path )
{
    const char *prefixed_path = make_prefixed_path( path );
	int res = xmp_rmdir( prefixed_path );
    release_prefixed_path( prefixed_path );
	return res;
}

/* -------------------------------------------------------------------------- */
static int xmp_symlink( const char *from, const char *to )
{
	int res;
	res = symlink( from, to );
	if ( res == -1 ) return -errno;
	return 0;
}

static int xmp_prefixed_symlink( const char *from, const char *to )
{
    const char *prefixed_from = make_prefixed_path( from );
    const char *prefixed_to = make_prefixed_path( to );
	int res = xmp_symlink( prefixed_from, prefixed_to );
    release_prefixed_path( prefixed_to );    
    release_prefixed_path( prefixed_from );
	return res;
}

/* -------------------------------------------------------------------------- */
static int xmp_rename( const char *from, const char *to )
{
	int res;
	res = rename( from, to );
	if ( res == -1 ) return -errno;
	return 0;
}

static int xmp_prefixed_rename( const char *from, const char *to )
{
    const char *prefixed_from = make_prefixed_path( from );
    const char *prefixed_to = make_prefixed_path( to );
	int res = xmp_rename( prefixed_from, prefixed_to );
    release_prefixed_path( prefixed_to );    
    release_prefixed_path( prefixed_from );
	return res;
}

/* -------------------------------------------------------------------------- */
static int xmp_link( const char *from, const char *to )
{
	int res = link( from, to );
	if ( res == -1 ) return -errno;
	return 0;
}

static int xmp_prefixed_link( const char *from, const char *to )
{
    const char *prefixed_from = make_prefixed_path( from );
    const char *prefixed_to = make_prefixed_path( to );
	int res = xmp_link( prefixed_from, prefixed_to );
    release_prefixed_path( prefixed_to );    
    release_prefixed_path( prefixed_from );
	return res;
}

/* -------------------------------------------------------------------------- */
static int xmp_chmod( const char *path, mode_t mode )
{
	int res = chmod( path, mode );
	if ( res == -1 ) return -errno;
	return 0;
}

static int xmp_prefixed_chmod( const char *path, mode_t mode )
{
    const char *prefixed_path = make_prefixed_path( path );
	int res = xmp_chmod( prefixed_path, mode );
    release_prefixed_path( prefixed_path );
	return res;
}

/* -------------------------------------------------------------------------- */
static int xmp_chown( const char *path, uid_t uid, gid_t gid )
{
	int res = lchown( path, uid, gid );
	if ( res == -1 ) return -errno;
	return 0;
}

static int xmp_prefixed_chown( const char *path, uid_t uid, gid_t gid )
{
    const char *prefixed_path = make_prefixed_path( path );
	int res = xmp_chown( prefixed_path, uid, gid );
    release_prefixed_path( prefixed_path );
	return res;
}

/* -------------------------------------------------------------------------- */
static int xmp_truncate( const char *path, off_t size )
{
	int res = truncate( path, size );
	if ( res == -1 ) return -errno;
	return 0;
}

static int xmp_prefixed_truncate( const char *path, off_t size )
{
    int res = 0;
    if ( above_truncate_limit( size ) )
        res = -EFBIG;
    else
    {
        const char *prefixed_path = make_prefixed_path( path );
        res = xmp_truncate( prefixed_path, size );
        release_prefixed_path( prefixed_path );
    }
	return res;
}

/* -------------------------------------------------------------------------- */
#ifdef HAVE_UTIMENSAT
static int xmp_utimens( const char *path, const struct timespec ts[2] )
{
	/* don't use utime/utimes since they follow symlinks */
	int res = utimensat( 0, path, ts, AT_SYMLINK_NOFOLLOW );
	if ( res == -1 ) return -errno;
	return 0;
}

static int xmp_prefixed_utimens( const char *path, const struct timespec ts[2] )
{
    const char *prefixed_path = make_prefixed_path( path );
	int res = xmp_utimens( prefixed_path, size );
    release_prefixed_path( prefixed_path );
	return res;
}

#endif

/* -------------------------------------------------------------------------- */
static int xmp_open( const char *path, struct fuse_file_info *fi )
{
	int res = open( path, fi->flags );
	if ( res == -1 ) return -errno;
	close( res );
	return 0;
}

static int xmp_prefixed_open( const char *path, struct fuse_file_info *fi )
{
    const char *prefixed_path = make_prefixed_path( path );
	int res = xmp_open( prefixed_path, fi );
    release_prefixed_path( prefixed_path );
	return res;
}

/* -------------------------------------------------------------------------- */
static int xmp_read( const char *path, char *buf, size_t size, off_t offset,
                     struct fuse_file_info *fi )
{
	int fd;
	int res;

	( void ) fi;
	fd = open( path, O_RDONLY );
	if ( fd == -1 ) return -errno;

	res = pread( fd, buf, size, offset );
	if ( res == -1 ) res = -errno;
	close( fd );
    
	return res;
}

static int xmp_prefixed_read( const char *path, char *buf, size_t size, off_t offset,
                              struct fuse_file_info *fi )
{
    const char *prefixed_path = make_prefixed_path( path );
	int res = xmp_read( prefixed_path, buf, size, offset, fi );
    release_prefixed_path( prefixed_path );
	return res;
}

/* -------------------------------------------------------------------------- */
static int xmp_write( const char *path, const char *buf, size_t size,
                      off_t offset, struct fuse_file_info *fi )
{
	int res;

	( void ) fi;
	int fd = open( path, O_WRONLY );
	if ( fd == -1 ) return -errno;

	res = pwrite( fd, buf, size, offset );
	if ( res == -1 ) res = -errno;

	close( fd );
	return res;
}

static int xmp_prefixed_write( const char *path, const char *buf, size_t size,
                               off_t offset, struct fuse_file_info *fi )
{
    int res;
    const char *prefixed_path = make_prefixed_path( path );
    if ( above_write_limit( used_space_of_file( prefixed_path ) ) )
        res = -EFBIG;
    else
        res = xmp_write( prefixed_path, buf, size, offset, fi );
    release_prefixed_path( prefixed_path );
    return res; 
}

/* -------------------------------------------------------------------------- */
static int xmp_statfs( const char *path, struct statvfs *stbuf )
{
	int res = statvfs( path, stbuf );
	if ( res == -1 ) return -errno;
	return 0;
}

static int xmp_prefixed_statfs( const char *path, struct statvfs *stbuf )
{
    const char *prefixed_path = make_prefixed_path( path );
	int res = xmp_statfs( prefixed_path, stbuf );
    release_prefixed_path( prefixed_path );
	return res;
}

/* -------------------------------------------------------------------------- */
static int xmp_release( const char *path, struct fuse_file_info *fi )
{
	/* Just a stub.	 This method is optional and can safely be left
	   unimplemented */

	(void) path;
	(void) fi;
	return 0;
}

static int xmp_fsync( const char *path, int isdatasync,
                      struct fuse_file_info *fi )
{
	/* Just a stub.	 This method is optional and can safely be left
	   unimplemented */

	(void) path;
	(void) isdatasync;
	(void) fi;
	return 0;
}

#ifdef HAVE_POSIX_FALLOCATE
static int xmp_fallocate( const char *path, int mode,
			off_t offset, off_t length, struct fuse_file_info *fi )
{
	int fd;
	int res;

	( void ) fi;

	if ( mode ) return -EOPNOTSUPP;

	fd = open( path, O_WRONLY );
	if ( fd == -1 ) return -errno;

	res = -posix_fallocate( fd, offset, length );

	close( fd );
	return res;
}
#endif

#ifdef HAVE_SETXATTR
/* xattr operations are optional and can safely be left unimplemented */
static int xmp_setxattr( const char *path, const char *name, const char *value,
                         size_t size, int flags )
{
	int res = lsetxattr( path, name, value, size, flags );
	if ( res == -1 ) return -errno;
	return 0;
}

static int xmp_getxattr( const char *path, const char *name, char *value, size_t size )
{
	int res = lgetxattr( path, name, value, size );
	if ( res == -1 ) return -errno;
	return res;
}

static int xmp_listxattr( const char *path, char *list, size_t size )
{
	int res = llistxattr( path, list, size );
	if ( res == -1 ) return -errno;
	return res;
}

static int xmp_removexattr( const char *path, const char *name )
{
	int res = lremovexattr( path, name );
	if ( res == -1 ) return -errno;
	return 0;
}
#endif /* HAVE_SETXATTR */

static struct fuse_operations xmp_oper = {
	.getattr	= xmp_prefixed_getattr,
	.access		= xmp_prefixed_access,
	.readlink	= xmp_prefixed_readlink,
	.readdir	= xmp_prefixed_readdir,
	.mknod		= xmp_prefixed_mknod,
	.mkdir		= xmp_prefixed_mkdir,
	.symlink	= xmp_prefixed_symlink,
	.unlink		= xmp_prefixed_unlink,
	.rmdir		= xmp_prefixed_rmdir,
	.rename		= xmp_prefixed_rename,
	.link		= xmp_prefixed_link,
	.chmod		= xmp_prefixed_chmod,
	.chown		= xmp_prefixed_chown,
	.truncate	= xmp_prefixed_truncate,
#ifdef HAVE_UTIMENSAT
	.utimens	= xmp_prefixed_utimens,
#endif
	.open		= xmp_prefixed_open,
	.read		= xmp_prefixed_read,
	.write		= xmp_prefixed_write,
	.statfs		= xmp_prefixed_statfs,
	.release	= xmp_release,
	.fsync		= xmp_fsync,
#ifdef HAVE_POSIX_FALLOCATE
	.fallocate	= xmp_fallocate,
#endif
#ifdef HAVE_SETXATTR
	.setxattr	= xmp_setxattr,
	.getxattr	= xmp_getxattr,
	.listxattr	= xmp_listxattr,
	.removexattr	= xmp_removexattr,
#endif
};


int main( int argc, char *argv[] )
{
    int res = 0;
    if ( argc < 4 )
        printf( "usage: fuse-proxy [options] mount-point proxy-dir truncate_limit write_limit\n" );
    else
    {
        set_prefix_write_limit( str_2_size( argv[ argc - 1 ] ) );        
        set_prefix_truncate_limit( str_2_size( argv[ argc - 2 ] ) );
        if ( set_prefix( argv[ argc - 3 ] ) == 0 )
        {
            printf( "cannot find absolute path of '%s'\n", argv[ argc - 3 ] );
            res = 3;
        }
        else
        {
            printf( "establishing proxy '%s' -> '%s'\n", argv[ argc - 4 ], argv[ argc - 3 ] );
            if ( prefix_write_limit > 0 )
                printf( "with write-limit of %lu bytes\n", prefix_write_limit );
            if ( prefix_truncate_limit > 0 )
                printf( "with truncate-limit of %lu bytes\n", prefix_truncate_limit );

            argc -= 3;
            umask( 0 );
            /* this function does not return! */
            fuse_main( argc, argv, &xmp_oper, NULL );
        }
    }
    return res;
}
