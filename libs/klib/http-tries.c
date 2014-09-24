#include <klib/misc.h> /* SetHttpTries */

static uint32_t s_tries = 0;
static bool s_tries_set = false;

void SetHttpTries(uint32_t tries) {
    s_tries = tries;
    s_tries_set = true;
}

uint32_t GetHttpTries(bool *isTriesSet) {
    bool dummy = false;
    if (isTriesSet == NULL) {
        isTriesSet = &dummy;
    }
    *isTriesSet = s_tries_set;
    return s_tries;
}
