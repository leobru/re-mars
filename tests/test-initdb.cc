#include <cstdio>
#include <cstdlib>
#include "mars.h"

int main(int argc, char **argv) {
    int len = 3;
    if (argc > 1)
        len = strtol(argv[1], nullptr, 8);
    if (len < 1 || len > 0777) {
        fprintf(stderr, "Usage: test-initdb [octal]\n"
                        "\toctal - total DB length in zones, 1 <= octal <= 777\n"
                        "\tdefault - 3 zones\n");
        exit(1);
    }

    mars_flags.zero_date = true;
    InitDB(052, 0, len);
}
