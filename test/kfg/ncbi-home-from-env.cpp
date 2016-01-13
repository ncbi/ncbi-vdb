#include <climits> /* PATH_MAX */
#include <cstdlib> // system
#include <cstring> // strrchr
 #include <stdio.h>
int main(int argc, char **argv) {
    char *last = strrchr(argv[0], '/');
    if (!last)
        return 1;
    char *n = strstr(argv[0], "ncbi-vdb");
    int s = 8;
    if (!n)
        return 1;
    int l = strlen(argv[0]);
    l -= n - argv[0];
    int tail = strlen(argv[0]);
    tail -= last - argv[0];
    size_t sPfx = n - argv[0];
    char command[PATH_MAX] = "";
    sprintf(command,
        "./ncbi-home-from-env.sh %.*ssra-tools%.*s/../bin/vdb-config",
        sPfx, argv[0], l - tail - s, n + s);
    return system(command) != 0;
}
