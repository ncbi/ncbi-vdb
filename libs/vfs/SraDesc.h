#include <klib/defs.h> /* rc_t */
struct KDirectory;
rc_t SraDescConvert(struct KDirectory * dir, const char * path,
    bool * recognized);
