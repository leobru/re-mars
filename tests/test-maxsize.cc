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
      "Usage: mars [-h] [-V] [-i] [-L <n>] [-n] [-l <n>] [-t]\n"
      "\t-V\tVerbose\n"
      "\t-i\tDo not initialize the main catalog\n"
      "\t-L <n>\tCatalog length, octal (default 1)\n"
      "\t\t- must be always specified if created with a non-default value\n"
      "\t-f <n>\tOperate on a file with the given name (default TEST)\n"
      "\t-n\tDo not create the file before operating\n"
      "\t-l <n>\tFile length, octal (default 427)\n"
      "\t-R <n>\tStart record number (default 1)\n"
      "\t-r <n>\tEnd record number (default 65536)\n"
      "\t-d\tInclude date stamps in descriptors\n"
      "\t-t - dump zones in text format as well\n"
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
    int file_len = 0427;
    int startrec = 1, numrec = 65536;
    int space;
    int clear = 0;
    std::string fname;

    Mars mars;
    mars.zero_date = true;

    for (;;) {
        c = getopt (argc, argv, "inhVtsiCcdL:l:f:R:r:");
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
        case 'R':
            startrec = atoi(optarg);
            break;
        case 'r':
            numrec = atoi(optarg);
            break;
        case 'f':
            fname = optarg;
            break;
        case 'c':
            clear = -1;
            break;
        case 'C':
            clear = 1;
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
        space = mars.avail();
        std::cerr << std::format("Usable space in root catalog: {:o}\n", space);
    }
    // Making a new array of 'len' zones, starting after the root catalog
    if (newfile) {
        mars.newd(fname.c_str(), 052, catalog_len, file_len);
    }
    // Opening it
    // mars.trace_stores = true;
    mars.opend(fname.c_str());
    mars.trace_stores = false;
    space = mars.avail();
    std::cout << std::format("Usable space in the file: {:o}\n", space);

    // Putting elements of size 0 until the DB overflows
    for (int i = startrec; i <= numrec; ++i) {
      // std::cerr << "Putting " << std::dec << i << '\n';
      // if (i == numrec) mars_flags.trace_stores = true;
      if (mars.putd(i | 024LL << 42, 0, 0)) {
            // An overflow error is expected at the last iteration
            std::cerr << "\twhile putting " << i << '\n';
            break;
        }
    }
    mars.trace_stores = false;
    space = mars.avail();
    std::cout << std::format("Remaining space in the file: {:o}\n", space);

    if (clear) {
        mars.cleard(clear > 0);

        space = mars.avail();
        std::cout << std::format("After clearing: {:o}\n", space);
    }
}
