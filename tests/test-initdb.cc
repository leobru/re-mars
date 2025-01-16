#include <cstdio>
#include <cstdlib>
#include <getopt.h>

#include "mars.h"

void usage() {
    fprintf(stderr, "Usage: test-initdb [-t] [octal]\n"
            "\t-t - dump zones in text format as well\n"
            "\t-d - include date stamps in descriptors\n"
            "\toctal - total DB length in zones, 1 <= octal <= 1777\n"
            "\tdefault - 3 zones\n");
}

int main(int argc, char **argv) {
    int c, len = 3;
    Mars mars;
    for (;;) {
        c = getopt (argc, argv, "tV");
        if (c < 0)
            break;
        switch (c) {
        case 't':
            mars.dump_txt_zones = true;
            break;
        case 'V':
            mars.verbose = true;
            break;
        case 'd':
            mars.zero_date = false;
            break;
        default:
        case 'h':
            usage ();
            return 0;
        }
    }
    if (argv[optind])
        len = strtol(argv[optind], nullptr, 8);
    if (len < 1 || len > 01777) {
        usage();
        exit(1);
    }
    mars.InitDB(052, 0, len);
}
