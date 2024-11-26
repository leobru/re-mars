#include <iostream>
#include <iomanip>
#include <cstdio>
#include <cstdlib>
#include <string>
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
      "\t-l <n>\tFile length, octal (default 2)\n"
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
    int file_len = 0750;
    std::string fname;

    mars_flags.zero_date = true;

    for (;;) {
        c = getopt (argc, argv, "inhVtsidL:l:f:");
        if (c < 0)
            break;
        switch (c) {
        default:
        case 'h':
            usage ();
            return 0;
        case 'V':
            mars_flags.verbose = true;
            break;
        case 'i':
            do_init = false;
            break;
        case 'n':
            newfile = false;
            break;
        case 't':
            mars_flags.dump_txt_zones = true;
            break;
        case 's':
            mars_flags.trace_stores = true;
            break;
        case 'd':
            mars_flags.zero_date = false;
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
        InitDB(052, 0, catalog_len);
    }
    // Setting the root location
    SetDB(052, 0, catalog_len);

    if (mars_flags.verbose) {
        int space = avail();
        std::cerr << "Usable space in root catalog: "
                  << std::oct << space << '\n';
    }
    // Making a new array of 'len' zones, starting after the root catalog
    if (newfile) {
        newd(fname.c_str(), 052, catalog_len, file_len);
    }
    // Opening it
    opend(fname.c_str());
    if (mars_flags.verbose) {
        int space = avail();
        std::cerr << "Usable space in the file: "
                  << std::oct << space << '\n';
    }

    // Putting elements of size 0 until the DB overflows
    for (int i = 1; ; ++i) {
      std::cerr << "Putting " << std::dec << i << '\n';
        if (putd(i, 0, 0)) {
            // An overflow error is expected at the last iteration
            std::cerr << "\twhile putting " << std::dec << i << '\n';
            break;
        }
        int space = avail();
        std::cerr << "Remaining: "
                  << std::oct << space << '\n';
    }

    if (mars_flags.verbose) {
        int space = avail();
        std::cerr << "Remaining space in the file: "
                  << std::oct << space << '\n';
    }

    cleard();                   // A termination error is expected

    if (mars_flags.verbose) {
        int space = avail();
        std::cerr << "After clearing: "
                  << std::oct << space << '\n';
    }

}
