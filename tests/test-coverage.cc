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

void init(uint64_t *start, int len) {
    for (int i = 0; i < len; ++i) {
        start[i] = (064LL << 42) + i;
    }
}

bool compare(Mars & mars, uint64_t *start1, uint64_t *start2, int len) {
    bool match = true;
    for (int i = 0; i < len; ++i) {
        if (start1[i] != start2[i]) {
            match = false;
            std::cerr << std::format("Element {} does not match ({:016o} vs {:016o})\n",
                                     i, start1[i], start2[i]);
        }
    }
    if (match && mars.verbose)
        std::cerr << std::format("{} elements match between {} and {}\n", len, (void*)start1, (void*)start2);
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

    Mars mars;
    mars.zero_date = true;

    for (;;) {
        c = getopt (argc, argv, "inhVtidL:l:f:");
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

    uint64_t PAGE1[1024], PAGE2[1024];

    init(PAGE1, 1024);

    // Initializing an array of 1024 words

    std::string elt = "A";
    elt.resize(8);
    // Putting one half of it to the DB
    mars.modd(elt.c_str(), PAGE1, 512);
    // Putting all of it (will be split: max usable words in a zone = 01775)
    mars.modd(elt.c_str(), PAGE1, 1024);
    // Again (exact match of length)
    mars.modd(elt.c_str(), PAGE1, 1024);
    // With smaller length (reusing the block)
    mars.modd(elt.c_str(), PAGE1+1, 1023);
    // Getting back
    mars.getd(elt.c_str(), PAGE2, 1023);
    if (!compare(mars, PAGE2, PAGE1+1, 1023))
        exit(1);
    // Done with it
    mars.deld(elt.c_str());

    // Putting 59 elements of varying sizes with numerical keys (key 0 is invalid)
    for (int i = 0; i <= 59; ++i) {
        if (mars.verbose)
            std::cerr << "Putting " << i << '\n';
        if (mars.putd(i+1, PAGE1+1, i)) {
            // An overflow error is expected at the last iteration
            std::cerr << "\twhile putting " << i << '\n';
        }
    }

    uint64_t k = mars.last();
    while (k) {
        int len = mars.getlen();
        if (mars.verbose)
            std::cout << std::format("Found {:o} {:o}\n", k, len);
        mars.getd(k, PAGE2, 100);
        if (!compare(mars, PAGE2, PAGE1+1, len))
            exit(1);
        k = mars.prev();
    }

    mars.cleard(false);              // A termination error is expected
}
