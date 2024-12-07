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
      "\t-d\tDump extent descriptors\n"
      "\t-r\tRecurse into metablocks\n"
      ;
}

uint64_t tobesm(std::string s) {
    // Convert to the BESM-6 compatible format for ease of comparison
    s += "     ";
    s.resize(6);
    std::reverse(s.begin(), s.end());
    s.resize(8);
    return *(uint64_t*)s.c_str();
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

static std::string print_id(uint64_t id) {
    return std::format("{:04o}.{:03o}", id & 01777, id >> 10);
}

struct Page {
    Page() { }
    uint64_t w[1024];
};

typedef const Page & pp;

typedef std::vector<uint64_t> Block;
typedef std::pair<Block, uint64_t> Extent;

class Analyzer {
    std::unordered_map<std::string, Page*> DiskImage;
    bool verbose;
    uint64_t arch, dbdesc, DBkey, dblen, IOpat;
    const uint64_t * freeSpace;
    Block root;
    struct BlockTreeLevel {
        uint64_t id;
        const uint64_t * location;
    };
    pp IOcall(uint64_t);
    pp get_zone(uint64_t);
    void setctl(uint64_t);
    void check_zone(int), dump_zone(int);
    Block get_root_block();
    Block get_block(uint64_t);
    Extent find_item(uint64_t);
    Extent find(uint64_t key);

public:
    Analyzer(uint64_t key, int lun, int start, int len, bool v) : verbose(v) {
       dbdesc = arch = to_lnuzzzz(lun, start, len);
       DBkey = key;
       setctl(dbdesc);
    }
    void check_leaks();
    void avail(), dir(), dump();
    void metablocks(bool recurse) {
        metablock(root, recurse, 0);
    }
    void metablock(Block &, bool, int = 0);
    Analyzer * open(const std::string &);
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
    uint64_t length = freeSpace[-1] & 01777;
    if (length != dblen) {
        std::cerr << std::format("Wrong catalog length: specified {:o}, seen {:o}\n",
                                 dblen, length);
        exit(1);
    }
    root = get_root_block();
    if (verbose) {
        std::cerr << std::format("The root block header is {:016o}\n", root[0]);
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

Extent Analyzer::find_item(uint64_t id) {
    auto zone = id & 01777;
    auto recnum = (id >> 10) & 0777;
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
    return std::make_pair(Block(aitem, aitem + curExtLength), next_extent(curExtent));
}

// Returns the payload of the block
Block Analyzer::get_block(uint64_t id) {
    auto extent = find_item(id);
    while (extent.second) {
        auto cont = find_item(extent.second);
        extent.first.insert(extent.first.end(), cont.first.begin(), cont.first.end());
        extent.second = cont.second;
    }
    if (extent.first.size() == extent.first[0] + 1) {
        return Block(extent.first.begin() + 1, extent.first.end());
    }
    
    std::cerr <<
        std::format("Block length discrepancy: {:o} total extents, {:o} in header\n",
                    extent.first.size(), extent.first[0]);
    return extent.first;
}

Block Analyzer::get_root_block() {
     return get_block(ROOT_BLOCK);
}

void Analyzer::avail() {
    std::cout << "Free space per zone (zones not mentioned are fully free):\n"
        "    ZONE   WORDS\n";
    for (size_t i = 0; i < dblen; ++i) {
        if (freeSpace[i] == 01776) {
            continue;
        }
        std::cout << std::format("    {:04o}    {:04o}\n", i, freeSpace[i]);
    }
}

void Analyzer::dir() {
    if (root[0] == 0) {
        std::cerr << "An empty block does not make sense\n";
        return;
    }
    if (root[0] & 1) {
        std::cerr << "Bad block with odd length " << root[0] << '\n';
    }
    if (root[1] != 0) {
        std::cerr << "Zero key is not zero?\n";
    }
    if (!(root[2] & ONEBIT(48)) && root[2] != ROOT_BLOCK) {
        std::cerr << "The zero key terminator is corrupted\n";
    }
    if (root[0] < 3)
        return;
    std::cout << "FILE NAME LENGTH LOCATION\n";
    for (size_t i = 3; i <= root[0]; i += 2) {
        std::string fname = frombesm(root[i]);
        auto fdescr = find_item(root[i+1]).first;
        int length = fdescr[1] >> 18;
        int location = fdescr[1] & 0777777;
        bool has_passwd = fdescr[0] == 3;
        std::cout << std::format("  {:6}    {:04o}  {:6o} {}\n", fname, length, location,
                                 has_passwd ? frombesm(fdescr[3]) : "");
    }
}

void Analyzer::metablock(Block & block, bool recurse, int indent) {
    size_t limit = block.size()-1;
    auto head = block[0];
    size_t used = head & 01777;
    if (used > limit) {
        std::cout << "Corrupted block: size " << limit << " claims " << used << '\n';
    } else {
        limit = used;
    }
    if (head >> 10) {
        std::cout << std::format("Linked: {} {}\n",
                                 print_id(head >> 29), print_id((head >> 10) & BITS(19)));
    }
    if (indent == 0)
        std::cout << "      KEY         RAW DESCRIPTOR  LOCATION\n";
    for (size_t i = 1; i <= limit; i += 2) {
        uint64_t key = block[i];
        uint64_t first_extent = block[i+1];
        std::cout << std::format("{:{}}{:016o} {:016o} {}\n", ' ', indent+1,
                                 key, first_extent, print_id(first_extent & BITS(19)));
        if (first_extent & ONEBIT(48)) {
            // Nested metablock
            auto nested = get_block(first_extent & 01777777);
            if (recurse)
                metablock(nested, true, indent+1);
        }
    }
}

void Analyzer::check_zone(int i) {
    pp page = get_zone(i);
    size_t last_free = page.w[1] & 01777;
    size_t extents = (page.w[1] >> 10) & 01777;
    size_t used = 0;               // header
    for (size_t i = 0; i < extents; ++i)
        used += get_extlength(page.w[i+2]);
    if (extents + 2 + freeSpace[i] != last_free + 1)
        std::cerr << std::format("Zone {:04o}: front {:04o} + free space {:04o} - 1 != last free {:o}\n",
                                 i, extents + 2, freeSpace[i], last_free);
    if (used + last_free != 01777) {
        std::cerr << std::format("Zone {:04o}: used {:04o} + last free {:04o} != 01777\n",
                                 i, used, last_free);
    }
}

void Analyzer::check_leaks() {
    for (size_t i = 0; i < dblen; ++i) {
        check_zone(i);
    }
    std::cout << std::format("Checked {:04o} zones\n", dblen);
}

// At the moment searches only in the root metablock
Extent Analyzer::find(uint64_t key) {
    for (size_t i = 3; i <= root[0]; i += 2) {
        if (root[i] == key) {
            return find_item(root[i+1]);
        }
    }
    return Extent();
}

Analyzer * Analyzer::open(const std::string & fname) {
    auto key = tobesm(fname);
    auto fdescr = find(key).first;
    if (fdescr.empty()) {
        std::cerr << std::format("File {} ({:016o}) not found\n", fname, key);
        exit(1);
    }
    int length = fdescr[1] >> 18;
    int location = fdescr[1] & 0777777;
    auto an = new Analyzer(fdescr[2], location >> 12, location & 07777, length, verbose);
    an->check_leaks();
    return an;
}

void Analyzer::dump_zone(int i) {
    pp page = get_zone(i);
    size_t extents = (page.w[1] >> 10) & 01777;
    size_t free_space = page.w[1] & 01777;
    if (!extents)
        return;
    std::cout << std::format("ZONE {:04o} EXTENTS {:o} FREE {:o}\n", i, extents, free_space);
    std::cout << "EXTENT START LENGTH  NEXT\n";
    for (size_t i = 0; i < extents; ++i) {
        int id = get_id(page.w[i+2]);
        size_t start = get_extstart(page.w[i+2]) + 1;
        size_t length = get_extlength(page.w[i+2]);
        size_t next = next_extent(page.w[i+2]);
        auto aitem = page.w + start;
        char flag = (length != (*aitem & 017777) + 1) ? '*' : ' ';
        if (next)
            std::cout << std::format("{:4o}   {:04o}  {}{:04o}  {}\n",
                                     id, start, flag, length, print_id(next));
        else
            std::cout << std::format("{:4o}   {:04o}  {}{:04o}\n",
                                     id, start, flag, length);
    }
}

void Analyzer::dump() {
    for (size_t i = 0; i < dblen; ++i) {
        dump_zone(i);
    }
}

int main(int argc, char ** argv) {
    int c;
    std::string fname;
    bool verbose = false;
    bool dump_descrs = false;
    bool recurse = false;
    int catalog_len = 1;
    for (;;) {
        c = getopt (argc, argv, "hVdrL:f:");
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
        case 'd':
            dump_descrs = true;
            break;
        case 'r':
            recurse = true;
            break;
        case 'L':
            catalog_len = strtol(optarg, nullptr, 8);
            break;
        case 'f':
            fname = optarg;
            break;
        }
    }

    Analyzer an(ROOTKEY, 052, 0, catalog_len, verbose);
    an.avail();
    an.check_leaks();
    an.dir();
    if (dump_descrs)
        an.dump();
    if (!fname.empty()) {
        std::cout << "Opening file " << fname << '\n';
        auto an2 = an.open(fname);
        an2->avail();
        std::cout << "Dumping file " << fname << '\n';
        an2->metablocks(recurse);
        if (dump_descrs)
            an2->dump();
        delete an2;
    }
}
