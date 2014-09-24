#include <klib/time.h> /* KSleep */

LIB_EXPORT rc_t CC KSleep(uint32_t seconds) { return KSleepMs(seconds * 1000); }
