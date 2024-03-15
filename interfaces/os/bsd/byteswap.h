
#ifndef _bsd_byteswap_h_
#define _bsd_byteswap_h_

#if defined(__FreeBSD__) || defined(__NetBSD__)

#include <sys/endian.h>

#define	__BYTE_ORDER	_BYTE_ORDER
#define	__BIG_ENDIAN	_BIG_ENDIAN
#define	__LITTLE_ENDIAN	_LITTLE_ENDIAN

#define bswap_16(x) bswap16 (x)
#define bswap_32(x) bswap32 (x)
#define bswap_64(x) bswap64 (x)

#else

#error "You must define macros for this BSD variant."

#endif

#endif
