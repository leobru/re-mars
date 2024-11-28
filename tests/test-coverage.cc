#include <iostream>
#include <iomanip>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <algorithm>
#include <ctime>
#include <getopt.h>

#include "mars.h"

void init(int start, int len) {
    for (int i = 0; i < len; ++i) {
      data[start+i] = (064LL << 42) + i;
    }
}

bool compare(int start1, int start2, int len) {
    bool match = true;
    for (int i = 0; i < len; ++i) {
        if (data[start1+i] != data[start2+i]) {
            match = false;
            std::cerr << "Element " << std::dec << i << " does not match ("
                      << data[start1+i].d << " vs " << data[start2+i].d << ")\n";
        }
    }
    if (match && mars_flags.verbose)
        std::cerr << std::dec << len << " elements match between "
                  << std::oct << start1 << " and " << start2 << '\n';
    return match;
}

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
    int file_len = 2;
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

    const int PAGE1 = 010000;
    const int PAGE2 = 012000;

    init(PAGE1, 1024);

    // Initializing an array of 1024 words

    std::string elt = "A";
    // Putting one half of it to the DB
    modd(elt.c_str(), PAGE1, 512);
    // Putting all of it (will be split: max usable words in a zone = 01775)
    modd(elt.c_str(), PAGE1, 1024);
    // Again (exact match of length)
    modd(elt.c_str(), PAGE1, 1024);
    // With smaller length (reusing the block)
    modd(elt.c_str(), PAGE1+1, 1023);
    // Getting back
    getd(elt.c_str(), PAGE2, 1023);
    if (!compare(PAGE2, PAGE1+1, 1023))
        exit(1);
    // Done with it
    deld(elt.c_str());

    // Putting 59 elements of varying sizes with numerical keys (key 0 is invalid)
    for (int i = 0; i <= 59; ++i) {
        if (mars_flags.verbose)
            std::cerr << "Putting " << i << '\n';
        if (putd(i+1, PAGE1+1, i)) {
            // An overflow error is expected at the last iteration
            std::cerr << "\twhile putting " << std::dec << i << '\n';
        }
    }

    uint64_t k = last();
    while (k) {
        int len = getlen();
        if (mars_flags.verbose)
            std::cout << "Found " << std::oct << k << ' ' << len << '\n';
        getd(k, PAGE2, 100);
        if (!compare(PAGE2, PAGE1+1, len))
            exit(1);
        k = prev();
    }

    cleard();                   // A termination error is expected
    IOflush();
}
