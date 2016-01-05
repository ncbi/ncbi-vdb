#include <cstdlib> // system
int main(int argc, char **argv) {
    return system("./ncbi-home-from-env.sh") != 0;
}