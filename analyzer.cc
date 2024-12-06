#include <iostream>
#include <iomanip>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
#include <unordered_map>
#include <format>
#include <algorithm>
#include <ctime>
#include <getopt.h>

#include "mars.h"

#define ARBITRARY_NONZERO 01234567007654321LL
#define ROOTKEY 04000
#define ROOT_BLOCK 02000
#define LOCKKEY ONEBIT(32)

#define ONEBIT(n) (1ULL << ((n)-1))   // one bit set, from 1 to 48
#define BITS(n)   (~0ULL >> (64 - n)) // bitmask of bits from 0 to n

void usage() {
    std::cerr <<
      "Usage: analyzer [options]\n"
      "\t-V\tVerbose\n"
      "\t-L <n>\tCatalog length, octal (default 1)\n"
      "\t-f <n>\tOperate on a file with the given name\n"
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

std::string frombesm(uint64_t w) {
    std::string s((char*)&w, (char*)&w+6);
    std::reverse(s.begin(), s.end());
    return s;
}

template<class T> uint64_t next_extent(const T & x) {
        return (x >> 20) & BITS(19);
}

template<class T> uint64_t get_extlength(const T & x) {
        return (x >> 10) & BITS(10);
}

template<class T> uint64_t get_extstart(const T & x) {
        return x & BITS(10);
}

template<class T> uint64_t get_id(const T & x) {
        return x >> 39;
}

static int to_lnuzzzz(int lun, int start, int len) {
    if (lun > 077 || start > 01777 || len > 0777)
        std::cerr << std::oct << lun << ' ' << start << ' ' << len
                  << "out of valid range, truncated\n";
    return ((lun & 077) << 12) | (start & 01777) | ((len & 0777) << 18);
}

struct Page {
    Page() { }
    uint64_t w[1024];
};

typedef const Page & pp;

class Analyzer {
    std::unordered_map<std::string, Page*> DiskImage;
    bool verbose;
    uint64_t arch, dbdesc, DBkey, dblen, IOpat;
    const uint64_t * freeSpace;
    const uint64_t * root;
    struct BlockTreeLevel {
        uint64_t id;
        const uint64_t * location;
    };
    pp IOcall(uint64_t);
    pp get_zone(uint64_t);
    void setctl(uint64_t);
    void check_zone(int);
    const uint64_t * get_root_block();
    const uint64_t * get_block(uint64_t);
    const uint64_t * find_item(uint64_t);

public:
    Analyzer(int lun, int start, int len, bool v) : verbose(v) {
       dbdesc = arch = to_lnuzzzz(lun, start, len);
       DBkey = ROOTKEY;
       setctl(dbdesc);
    }
    void check_leaks();
    void dir(), open(const std::string &), dump();
};

void Analyzer::setctl(uint64_t location) {
    IOpat = location & 0777777;
    dblen = dbdesc >> 18;
    pp page = IOcall(IOpat);
    if (page.w[0] != DBkey) {
        std::cerr << std::format("Bad DBkey: seen {:016o}, want {:016o}\n",
                                 page.w[0], DBkey);
        exit(1);
    }
    freeSpace = page.w + (page.w[3] & 01777) + 2;
    int length = freeSpace[-1] & 01777;
    if (length != dblen) {
        std::cerr << std::format("Wrong catalog length: specified {:o}, seen {:o}\n",
                                 dblen, length);
        exit(1);
    }
    // Can the root block be fragmented?
    root = get_root_block();
    if (verbose) {
        std::cerr << std::format("The root block header is {:016o}\n", *root);
    }
}

pp Analyzer::IOcall(uint64_t is) {
    std::string nuzzzz;
    nuzzzz = std::format("{:06o}", is & BITS(18));
    // read only operation
    auto it = DiskImage.find(nuzzzz);
    if (verbose)
        std::cerr << std::format("Reading {}\n", nuzzzz);
    if (it == DiskImage.end()) {
        FILE * f = fopen(nuzzzz.c_str(), "r");
        if (!f) {
            std::cerr << "\tZone " << nuzzzz << " does not exist yet\n";
            DiskImage[nuzzzz] = new Page();  // create default
            return *DiskImage[nuzzzz];
        }
        if (verbose)
            std::cerr << "\tFirst time - reading from disk\n";
        DiskImage[nuzzzz] = new Page;
        fread(DiskImage[nuzzzz], 1, sizeof(Page), f);
        fclose(f);
    }
    return *DiskImage[nuzzzz];
}

// Takes the zone number as the argument,
// returns the pointer to the zone read in m16 and curbuf
pp Analyzer::get_zone(uint64_t arg) {
    auto zoneKey = arg | DBkey;
    auto IOword = arg + IOpat;
    pp page = IOcall(IOword);
    if (page.w[0] == zoneKey)
        return page;
    std::cerr << std::format("Corruption: zoneKey = {:o}, data = {:o}\n", zoneKey, page.w[0]);
    throw Mars::ERR_BAD_PAGE;
}

const uint64_t * Analyzer::find_item(uint64_t id) {
    auto zone = id & 01777;
    auto recnum = id >> 10;
    if (verbose)
        std::cerr << std::format("Need item {:o} in zone {:o}\n", recnum, zone);
    pp page = get_zone(zone);
    if ((id >> 10) == 0)
        throw Mars::ERR_ZEROKEY;
    auto nrec = (page.w[1] >> 10) & 01777;
    if (verbose)
        std::cerr << std::format("There are {:o} items\n", nrec);
    if (!nrec)
        throw Mars::ERR_NO_RECORD;
    if (nrec > recnum)
        nrec = recnum;
    uint64_t curExtent;
    for (int i = nrec; ;) {
        curExtent = page.w[i+1];
        if (get_id(curExtent) == recnum)
            break;
        if (!--i)
            throw Mars::ERR_NO_RECORD;
    }

    auto aitem = page.w + get_extstart(curExtent) + 1;
    if (verbose) {
        std::cerr << std::format("Extent descriptor: {:016o}\n",
                                 curExtent);
        std::cerr << std::format("The extent header is @{:04o}\n",
                                 get_extstart(curExtent));
        std::cerr << std::format("Header contents: {:016o}\n", *aitem);
    }

    auto curExtLength = get_extlength(curExtent);
    if (verbose) {
        std::cerr << std::format("Found extent of length {:o}\n", curExtLength);
    }
    if (curExtLength != (*aitem & 01777) + 1) {
        std::cerr <<
            std::format("Extent length discrepancy: {:o} in descriptor, {:o} in header\n",
            curExtLength, (*aitem & 01777));
    }
    return aitem;
}

// Returns the payload of the block
const uint64_t * Analyzer::get_block(uint64_t id) {
    auto extent = find_item(id);
    // TODO: check for chaining
    return extent + 1;
}

const uint64_t * Analyzer::get_root_block() {
     return get_block(ROOT_BLOCK);
}

void Analyzer::dir() {
    std::cout << "Free space per zone:\n    ZONE   WORDS\n";
    for (int i = 0; i < dblen; ++i) {
        std::cout << std::format("    {:04o}    {:04o}\n", i, freeSpace[i]);
    }
    if (root[1] != 0 || root[2] != ROOT_BLOCK) {
        std::cerr << "Bad self-reference of the root block\n";
    }
    std::cout << "FILE NAME LENGTH LOCATION\n";
    for (int i = 3; i <= root[0]; i += 2) {
        std::string fname = frombesm(root[i]);
        auto fdescr = find_item(root[i+1]);
        int length = fdescr[1] >> 18;
        int location = fdescr[1] & 0777777;
        bool has_passwd = fdescr[0] == 3;
        std::cout << std::format("  {:6}    {:04o}  {:6o} {}\n", fname, length, location,
                                 has_passwd ? frombesm(fdescr[3]) : "");
    }
}

void Analyzer::check_zone(int i) {
    pp page = get_zone(i);
    int last_free = page.w[1] & 01777;
    int extents = (page.w[1] >> 10) & 01777;
    int used = 0;               // header
    for (int i = 0; i < extents; ++i)
        used += get_extlength(page.w[i+2]);
    if (used + last_free != 01777) {
        std::cerr << std::format("Zone {:04o}: used {:04o} + free {:04o} != 02000\n",
                                 i, used, last_free);
    }    
}

void Analyzer::check_leaks() {
    for (int i = 0; i < dblen; ++i) {
        check_zone(i);
    }
    std::cout << std::format("Checked {:04o} zones\n", dblen); 
}

void Analyzer::open(const std::string &) {
    std::cerr << __FUNCTION__ << " NYI\n";
}

void Analyzer::dump() {
    std::cerr << __FUNCTION__ << " NYI\n";
}

int main(int argc, char ** argv) {
    int c;
    std::string fname;
    bool verbose = false;
    int catalog_len = 1;
    for (;;) {
        c = getopt (argc, argv, "hVL:f:");
        if (c < 0)
            break;
        switch (c) {
        default:
        case 'h':
            usage ();
            return 0;
        case 'V':
            verbose = true;
            break;
        case 'L':
            catalog_len = strtol(optarg, nullptr, 8);
            break;
        case 'f':
            fname = optarg;
            break;
        }
    }

    Analyzer an(052, 0, catalog_len, verbose);
    an.check_leaks();
    an.dir();
    if (!fname.empty()) {
        an.open(fname);
        an.dump();
    }
}
