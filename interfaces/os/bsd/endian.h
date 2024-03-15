
#if defined(__FreeBSD__) || defined(__NetBSD__)

#include <sys/endian.h>

#else

#error "You must add support for this BSD variant"

#endif
