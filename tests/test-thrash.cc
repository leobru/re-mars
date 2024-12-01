#include <iostream>
#include <iomanip>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <format>
#include <algorithm>
#include <ctime>
#include <getopt.h>

#include "mars.h"

void usage() {
    std::cerr <<
      "Usage: test-thrash [options]\n"
      "\t-V\tVerbose\n"
      "\t-i\tDo not initialize the main catalog\n"
      "\t-L <n>\tCatalog length, octal (default 1)\n"
      "\t\t- must be always specified if created with a non-default value\n"
      "\t-f <n>\tOperate on a file with the given name (default TEST)\n"
      "\t-n\tDo not create the file before operating\n"
      "\t-l <n>\tFile length, octal (default 100)\n"
      "\t-r <n>\tNumber of records (default 32K)\n"
      "\t-R <n>\tNumber of repetitions of the create/delete cycle\n"
      "\t-m <n>\tMax record size (default 1K)\n"
      "\t-c\tDo not clear the DB at the end\n"
      "\t-d\tInclude date stamps in descriptors\n"
      "\t-t\tDump zones in text format as well\n"
      "\t-s\tTrace store operations\n"
      ;
}

std::string tobesm(std::string s) {
    // Convert to the BESM-6 compatible format for ease of comparison
    s += "     ";
    s.resize(6);
    std::reverse(s.begin(), s.end());
    s.resize(8);
    return s;
}

int main(int argc, char ** argv) {
    int c;
    bool do_init = true, newfile = true;
    int catalog_len = 1;
    int file_len = 0100;
    int numrec = 32768, maxsize = 1024, repeats = 1;
    bool clear = true;
    std::string fname;

    Mars mars;
    mars.zero_date = true;

    for (;;) {
        c = getopt (argc, argv, "inhVtsicdm:L:l:f:r:R:");
        if (c < 0)
            break;
        switch (c) {
        default:
        case 'h':
            usage ();
            return 0;
        case 'V':
            mars.verbose = true;
            break;
        case 'i':
            do_init = false;
            break;
        case 'n':
            newfile = false;
            break;
        case 't':
            mars.dump_txt_zones = true;
            break;
        case 's':
            mars.trace_stores = true;
            break;
        case 'd':
            mars.zero_date = false;
            break;
        case 'L':
            catalog_len = strtol(optarg, nullptr, 8);
            break;
        case 'l':
            file_len = strtol(optarg, nullptr, 8);
            break;
        case 'f':
            fname = optarg;
            break;
        case 'c':
            clear = false;
            break;
        case 'r':
            numrec = atoi(optarg);
            break;
        case 'm':
            maxsize = atoi(optarg);
            break;
        case 'R':
            repeats = atoi(optarg);
            break;
        }
    }

    if (!catalog_len || !file_len) {
        usage();
        exit(1);
    }

    if (fname.empty())
        fname = "TEST";
    fname = tobesm(fname);
    // Initializing the database catalog: 1 zone, starting from zone 0 on LUN 52 (arbitrary)
    if (do_init) {
        mars.InitDB(052, 0, catalog_len);
    }
    // Setting the root location
    mars.SetDB(052, 0, catalog_len);

    if (mars.verbose) {
        int space = mars.avail();
        std::cerr << std::format("Usable space in root catalog: {:o}\n", space);
    }
    // Making a new array of 'len' zones, starting after the root catalog
    if (newfile) {
        mars.newd(fname.c_str(), 052, catalog_len, file_len);
    }
    // Opening it
    mars.opend(fname.c_str());
    int space = mars.avail();
    std::cout << std::format("Usable space in the file: {}\n", space);

    for (int rep = 0; rep < repeats; rep++) {
        // Putting elements with random keys and arbitrary data, of random sizes,
        // up to numrec or until the DB overflows
        for (int i = 1; i <= numrec; ++i) {
            int k = (random() % numrec) + 1;
            int size = random() % maxsize;
            std::cout << k << ' ' << size << '\n';
            if (mars.verbose)
                std::cerr << std::format("Putting '{}' of size {}\n", k, size);
            // Memory location 010000 and up are not used, will be 0
            if (mars.modd(k | 024LL << 42, 010000, size)) {
                // An overflow error is possible
                std::cout << std::format("Overflow when attempting to put record #{}\n", i);
                break;
            }
        }

        space = mars.avail();
        std::cout << std::format("Remaining space in the file: {}\n", space);
        std::cout << "0 0\n";

        if (clear || rep+1 < repeats) {
            while (uint64_t k = mars.last()) {
                if (mars.deld(k)) {
                    std::cerr << std::format("Unexpected error at {:o}, stopping clearing\n", k);
                    break;
                }
            }
            space = mars.avail();
            std::cout << std::format("After clearing: {}\n", space);
        }
    }
}
