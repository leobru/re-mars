#include <cstdio>
#include <cstdlib>
#include <getopt.h>

#include "mars.h"

void usage() {
    fprintf(stderr, "Usage: test-initdb [-t] [octal]\n"
            "\t-t - dump zones in text format as well\n"
            "\t-s - trace stores\n"
            "\toctal - total DB length in zones, 1 <= octal <= 1777\n"
            "\tdefault - 3 zones\n");
}

int main(int argc, char **argv) {
    int c, len = 3;
    for (;;) {
        c = getopt (argc, argv, "stV");
        if (c < 0)
            break;
        switch (c) {
        case 't':
            mars_flags.dump_txt_zones = true;
            break;
        case 's':
            mars_flags.trace_stores = true;
            break;
        case 'V':
            mars_flags.verbose = true;
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

    mars_flags.zero_date = true;
    InitDB(052, 0, len);
    IOflush();
}
