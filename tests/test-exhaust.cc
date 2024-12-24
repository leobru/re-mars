#include <iostream>
#include <iomanip>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <format>
#include <algorithm>
#include <ctime>
#include <getopt.h>
#include <vector>
#include "mars.h"

void usage() {
    std::cerr <<
      "Usage: mars [-h] [-V] [-i] [-L <n>] [-n] [-l <n>] [-t]\n"
      "\t-V\tVerbose\n"
      "\t-r <n>\tEnd record number (default 200000)\n"
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

int experiment(Mars & mars, int step, size_t numrec);
    
int main(int argc, char ** argv) {
    int c;
    int numrec = 200000;
    int start = 0, finish = 100;
    Mars mars(false);
    mars.zero_date = true;

    for (;;) {
        c = getopt (argc, argv, "hVtsidr:n:N:");
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
        case 't':
            mars.dump_txt_zones = true;
            break;
        case 's':
            mars.trace_stores = true;
            break;
        case 'd':
            mars.zero_date = false;
            break;
        case 'r':
            numrec = atoi(optarg);
            break;
        case 'n':
          finish = atoi(optarg);
          break;
        case 'N':
          start = atoi(optarg);
          break;
        }
    }

    int best = 0, with = 0;
    for (int i = start; i <= finish; ++i) {
        int cur = experiment(mars, i, numrec);
        if (cur > best) {
            std::cout << std::format("Using {}, got {} elements\n", i, cur);
            best = cur;
            with = i;
        }        
    }
    std::cout << std::format("At the end, using {}, got {} elements\n",
                             with, best);
}

int experiment(Mars & mars, int step, size_t numrec) {
    std::cerr << "Step " << step << ':';
    // Initializing the database catalog: 1 zone, starting from zone 0 on LUN 52 (arbitrary)
    mars.InitDB(052, 0, 01731);
    // Setting the root location
    mars.SetDB(052, 0, 01731);

    mars.trace_stores = false;
    std::vector<int> a(numrec);
    srandom(step);
    for (size_t i = 0; i < a.size(); ++i)
        a[i] = random();
    std::sort(a.begin(), a.end());
    a.erase( std::unique( a.begin(), a.end() ), a.end() );
    std::cerr << a.size() << " remain, ";
    for (size_t i = 0; i < a.size(); ++i)
        std::swap(a[i], a[random() % a.size()]);
    // Putting elements of size 0 until the DB overflows
    for (int i = 0; i < std::min(numrec, a.size()); ++i) {
      // std::cerr << "Putting " << std::dec << i << '\n';
      // if (i == numrec) mars_flags.trace_stores = true;
      if (mars.putd(a[i] | 024LL << 42, 0, 0)) {
            // An overflow error is expected at the last iteration
          std::cerr << "Step " << step << "\twhile putting " << i << '\n';
            return i;
        }
    }
    return numrec;
}
