#include <cstdio>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cassert>
#include <unordered_map>
#include <vector>
#include <string>
#include <format>
#include <algorithm>
#include <chrono>
#include <getopt.h>

#include "mars.h"

MarsFlags mars_flags;

bool & trace_stores = mars_flags.trace_stores;
bool & verbose = mars_flags.verbose;

word data[RAM_LENGTH];

uint64_t rk = 7LL << 41;

// For convenience of detection which locations not to trace
word & m5 = data[5];
word & m16 = data[016];
word & m6 = data[6];
word & m7 = data[7];
word & m13 = data[13];

// History of all stores, for debug.
std::unordered_map<size_t, uint64_t> all_stores;

uint64_t& word::store(uint64_t x) {
    size_t index = this-data;
    if (index < 16) x &= 077777;
    if (trace_stores && index > 16 && index < RAM_LENGTH) {
        std::cerr << std::format("       {:05o}: store {:016o}\n", index, x);
    }
    if (mars_flags.memoize_stores && index > 16 && index < RAM_LENGTH) {
        all_stores[index] = x;
    }
    d=x;
    return d;
}
word& word::operator=(word x) {
    size_t index = this-data;
    if (index < 16) x.d &= 077777;
    if (trace_stores && index > 16 && index < RAM_LENGTH) {
        std::cerr << std::format("       {:05o}: store {:016o}\n", index, x.d);
    }
    if (mars_flags.memoize_stores && index > 16 && index < RAM_LENGTH) {
        all_stores[index] = x.d;
    }
    d=x.d;
    return *this;
}
word& word::operator=(word * x) {
    ptrdiff_t offset = x-data;
    if (offset < 0 || offset >= 077777) {
        std::cerr << std::format("Cannot point to words outside of RAM, offset = {:o}\n", offset);
        abort();
    }
    size_t index = this-data;
    if (trace_stores && index > 16 && index < RAM_LENGTH) {
        printf("       %05lo: store %016lo\n", index, offset);
    }
    if (mars_flags.memoize_stores && index > 16 && index < RAM_LENGTH) {
        all_stores[index] = offset;
    }
    d=offset;
    return *this;
}

uint64_t get_store(size_t index) {
    return all_stores[index];
}

struct bdvect_t {
    static const size_t SIZE = 168;
    word w[SIZE];
};

bdvect_t sav;

#define FAKEBLK 020
#define ARBITRARY_NONZERO 01234567007654321LL
#define ROOTKEY 04000
#define LOCKKEY ONEBIT(32)

#define bdvect (*reinterpret_cast<bdvect_t*>(data+BDVECT))

//
std::vector<int> comparable{
    3,5,010,012,013,014,015,020,021,022,023,024,025,026,027,
    031,033,034,035,036,037,040,041,042,043,044,045,046,047,050,051,052,053,054,
    055,056,0116,0117,0120,0157,0160,0220,0241,0242,0243,0244,0245,0246,0247
};

void dump() {
    for (auto j : comparable) {
        if (bdvect.w[j] != sav.w[j]) {
            std::cout << std::format(" WORD {:03o} CHANGED FROM  {:016o} TO  {:016o}\n",
                                     int(j), sav.w[j].d, bdvect.w[j].d);
        }
    }
    sav = bdvect;
}

word& word::operator*() {
    unsigned a = d & 077777;
    if (a >= sizeof(data)/sizeof(*data)) {
      std::cerr << std::format("Address {:o} out of range\n", a);
        abort();
    }
    return data[a];
}

word& word::operator[](word x) { return *(*this + x.d); }

uint64_t word::operator&() { return this-data; }

#define ONEBIT(n) (1ULL << ((n)-1))   // one bit set, from 1 to 48
#define BITS(n)   (~0ULL >> (64 - n)) // bitmask of bits from 0 to n
#define MERGE_(a,b)  a##b
#define LABEL_(a) MERGE_(ret_, a)
#define ret LABEL_(__LINE__)
#define INDJUMP_MAGIC 076500
#define target(label) (jumptab.count(&&label) ? jumptab[&&label] :      \
                       (targets.push_back(&&label),                     \
                        jumptab[&&label] = targets.size() + INDJUMP_MAGIC-1))
#define call(addr,link) do { link = target(ret); goto addr; ret:; } while (0)
#define dirty(x) dirty = dirty.d | ((curZone.d ? x : 0) + 1)
// #define jump(x) do { std::cerr << "Jump to " #x "\n"; goto x; } while(0)
#define jump(x) goto x
#define indjump(x)                                                      \
    do {                                                                \
        assert ((x.d & ~077LL) == INDJUMP_MAGIC &&                      \
                (x.d & 077) < targets.size());                          \
        goto *(targets[x.d & 077]);                                     \
    } while (false)

// struct Bdvect
word & outadr = data[BDVECT+1];
word & orgcmd = data[BDVECT+3];
word & curcmd = data[BDVECT+5];
word & syncw = data[BDVECT+6];
word & givenp = data[BDVECT+7];
word & key = data[BDVECT+010];
word & erhndl = data[BDVECT+011];
word & loc12 = data[BDVECT+012];
word & myloc = data[BDVECT+013];
word & loc14 = data[BDVECT+014];
word & mylen = data[BDVECT+015];
word & bdbuf = data[BDVECT+016];
word & bdtab = data[BDVECT+017];
word & loc20 = data[BDVECT+020];
word * const Array = data+BDVECT+021;
word & dbdesc = data[BDVECT+030];
word & DBkey = data[BDVECT+031];
word & savedp = data[BDVECT+032];
word & frebas = data[BDVECT+034];
word & adescr = data[BDVECT+035];
word & limit = data[BDVECT+036];
word & curkey = data[BDVECT+037];
word & endmrk = data[BDVECT+040];
word & desc1 = data[BDVECT+041];
word & desc2 = data[BDVECT+042];
word & IOpat = data[BDVECT+044];
word & curpos = data[BDVECT+045];
word & aitem = data[BDVECT+046];
word & itmlen = data[BDVECT+047];
word & element = data[BDVECT+050];
word & dblen = data[BDVECT+051];
word & loc53 = data[BDVECT+053];
word & loc54 = data[BDVECT+054];
// Metadata[-1] (loc55) is also used
word * const Metadata = data+BDVECT+056;
word & loc116 = data[BDVECT+0116];
word & loc117 = data[BDVECT+0117];
word & loc120 = data[BDVECT+0120];
word & loc157 = data[BDVECT+0157];
word & loc160 = data[BDVECT+0160];
word & curExtent = data[BDVECT+0220];
word & curbuf = data[BDVECT+0241];
word & idx = data[BDVECT+0242];
word & curZone = data[BDVECT+0244];
word & loc246 = data[BDVECT+0246];
word & dirty = data[BDVECT+0247];

union Accumulator {
    static const uint64_t MASK = BITS(48);
    uint64_t d;
    struct {
        unsigned off1 : 10;
        unsigned off2 : 10;
        unsigned extent  : 19;
        unsigned upper9  : 9;
    };
    struct {
        unsigned addr   : 15;
        unsigned dummy  : 18;
        unsigned stamp  : 15;
    };
    operator uint64_t() { return d; }
    uint64_t& operator=(uint64_t x) { d = x & MASK; return d; }
    uint64_t& operator=(word x) { d = x.d & MASK; return d; }
    uint64_t& operator&=(uint64_t x) { d &= x; return d; }
    uint64_t& operator|=(uint64_t x) { d |= x; d &= MASK; return d; }
    uint64_t& operator^=(uint64_t x) { d ^= x; d &= MASK; return d; }
    uint64_t& operator+=(uint64_t x) { d += x; d &= MASK; return d; }
    uint64_t& operator-=(uint64_t x) { d -= x; d &= MASK; return d; }
    uint64_t& operator<<=(int x) { d <<= x; d &= MASK; return d; }
    uint64_t& operator>>=(int x) { d >>= x; return d; }
    uint64_t& operator++() { ++d; d &= MASK; return d; }
    uint64_t& operator--() { --d; d &= MASK; return d; }

} acc;

// struct Mars
word & abdv = data[BDSYS+3];
uint64_t & arch = data[BDSYS+4].d;
word & savm16 = data[BDSYS+7];
word & d00010 = data[BDSYS+010];
word & d00011 = data[BDSYS+011];
word & d00012 = data[BDSYS+012];
word & savm13 = data[BDSYS+013];
word & goto_ = data[BDSYS+014];
word & zonkey = data[BDSYS+015];
word & usrloc = data[BDSYS+016];
word & savm7 = data[BDSYS+017];
word & savm6 = data[BDSYS+020];
word & savm5 = data[BDSYS+021];
word & jmpoff = data[BDSYS+022];
word & datlen = data[BDSYS+023];
word & d00024 = data[BDSYS+024];
word & d00025 = data[BDSYS+025];
word & d00026 = data[BDSYS+026];
word & temp = data[BDSYS+027];
word & d00030 = data[BDSYS+030];
word & d00031 = data[BDSYS+031];
word & d00032 = data[BDSYS+032];
word & d00033 = data[BDSYS+033];
word & remlen = data[BDSYS+034];
word & work = data[BDSYS+035];
word & work2 = data[BDSYS+036];
word & IOword = data[BDSYS+037];
word & d00040 = data[BDSYS+040];
word & savrk = data[BDSYS+041];

enum Ops {
    FROMBASE, TOBASE, COMPARE, A00317, DONE
};

const char * msg[] = {
    "Zero key", "Page corrupted", "No such record", "Invalid name",
    "1st page corrupted", "Overflow", "Step too big", "No such name",
    "Name already exists", "No end symbol", "Internal error", "Record too long",
    "DB is locked", "No current record", "No prev. record", "No next record",
    "Wrong password" };

struct Page {
    word w[1024];
};

std::unordered_map<std::string, Page> DiskImage;

void IOdiscard() {
    DiskImage.clear();
}

void IOflush() {
    for (auto & it : DiskImage) {
        const std::string& nuzzzz = it.first;
        FILE * f = fopen(nuzzzz.c_str(), "w");
        if (!f) {
            std::cerr << std::format("Could not open {} ({})\n", nuzzzz, strerror(errno));
            exit(1);
        }
        fwrite(&it.second, 1, sizeof(Page), f);
        fclose(f);

        if (mars_flags.dump_txt_zones) {
            std::string txt(nuzzzz+".txt");
            FILE * t = fopen(txt.c_str(), "w");
            if (!t) {
                std::cerr << std::format("Could not open {} ({})\n", txt, strerror(errno));
                exit(1);
            }
            const char * zone = nuzzzz.c_str()+2;
            for (int i = 0; i < 1024; ++i) {
                fprintf(t, "%s.%04o:  %04o %04o %04o %04o\n",
                        zone, i, int(it.second.w[i] >> 36),
                        int((it.second.w[i] >> 24) & 07777),
                        int((it.second.w[i] >> 12) & 07777),
                        int((it.second.w[i] >> 0) & 07777));
            }
            fclose(t);
        }
    }
    IOdiscard();
}

void IOcall(word is) {
    int page = (is >> 30) & BITS(5);
    std::string nuzzzz;
    bool stores = mars_flags.trace_stores;
    mars_flags.trace_stores = false;
    nuzzzz = std::format("{:06o}", is.d & BITS(18));
    if (is.d & ONEBIT(40)) {
        // read
        auto it = DiskImage.find(nuzzzz);
        if (verbose)
            std::cerr << std::format("Reading {} to address {:o}\n", nuzzzz, page*1024);
        if (it == DiskImage.end()) {
            FILE * f = fopen(nuzzzz.c_str(), "r");
            if (!f) {
                std::cerr << "\tZone " << nuzzzz << " does not exist yet\n";
                *reinterpret_cast<Page*>(data+page*1024) = Page();
                trace_stores = stores;
                return;
            }
            if (verbose)
                std::cerr << "\tFirst time - reading from disk\n";

            fread(data+page*1024, 1024, sizeof(uint64_t), f);
            fclose(f);
            DiskImage[nuzzzz] = *reinterpret_cast<Page*>(data+page*1024);
        } else
            *reinterpret_cast<Page*>(data+page*1024) = it->second;
    } else {
        // write
        if (verbose)
            std::cerr << std::format("Writing {} from address {:o}\n", nuzzzz, page*1024);
        DiskImage[nuzzzz] = *reinterpret_cast<Page*>(data+page*1024);
    }
    mars_flags.trace_stores = stores;
}

// Takes acc as the zone number,
// returns the pointer to the zone read in m16
void getzon() {
    curZone = acc;
    acc |= DBkey.d;
    zonkey = acc;
    acc &= 01777;
    acc = acc ? bdbuf.d : bdtab.d;
    m16 = curbuf = acc;
    if (m16[0] == zonkey)
        return;
    if (curZone.d && (dirty.d & 2)) {
        dirty = dirty.d & ~2;
        IOword = (IOpat.d | bdbuf.d << 20) + (m16[0].d & 01777);
        IOcall(IOword);
    }
    IOword = (curbuf.d << 20 | curZone.d | ONEBIT(40)) + IOpat.d;
    IOcall(IOword);
    m16 = curbuf;
    if (m16[0] == zonkey)
        return;
    std::cerr << std::format("Corruption: zonkey = {:o}, data = {:o}\n", zonkey.d, (*m16).d);
    throw ERR_BAD_PAGE;
}

void save(bool force_tab = false) {
    if (force_tab || (dirty.d & 1)) {
        IOword = IOpat.d | bdtab.d << 20;
        IOcall(IOword);
    }
    if (dirty.d & 2) {
        IOword = (IOpat.d | bdbuf.d << 20) + (bdbuf[0].d & 01777);
        IOcall(IOword);
    }
    dirty = 0;
}

void finalize() {
    d00011 = acc;               // nonzero if error
    bool force_tab = false;
    if (syncw != 0) {
        if (syncw != 1)
            return;
        m16 = bdtab;
        if (m16[1].d & LOCKKEY) {
            m16[1].d &= ~LOCKKEY;
            force_tab = true;
        }
    }
    save(force_tab);
}

void date() {
    using namespace std::chrono;
    const year_month_day ymd{floor<days>(system_clock::now())};
    unsigned d{ymd.day()}, m{ymd.month()};
    int y{ymd.year()};
    uint64_t stamp = mars_flags.zero_date ? 0 : (d/10*16+d%10) << 9 |
        (m/10*16+m%10) << 4 |
        y % 10;
    work = loc14.d & (0777777LL << 15);
    datlen = acc = work.d | mylen.d | (stamp << 33);
}

// Prepare a metadata block
void make_metablock() {
    d00012 = loc20;
    mylen = 041;
    d00010 = 2;
    usrloc = &d00010;           // to match traced stores with the original
    data[FAKEBLK].d = 2;
    data[FAKEBLK+1].d = d00011.d;
    data[FAKEBLK+2].d = loc20.d;
    m16 = usrloc.d = FAKEBLK;
}

// Returns result in itmlen and acc
uint64_t usable_space() {
    itmlen = 0;
    for (int i = dblen.d - 1; i >= 0; --i) {
        if (frebas[i].d != 0)
            itmlen = itmlen.d + frebas[i].d - 1;
    }
    m16 = 0;
    return itmlen.d;
}

void find_item(uint64_t arg) {
    d00040 = acc = arg;
    acc &= 01777;
    getzon();
    acc = d00040.d & 03776000;
    if (!acc)
        throw ERR_ZEROKEY;      // Attempting to compute length of a placeholder?
    work = acc;
    acc <<= 29;
    acc &= BITS(48);
    work2 = acc;
    acc = m16[1].d;
    acc &= 03776000;
    if (!acc)
        throw ERR_NO_RECORD;    // Attempting to compute length of a deleted record?
    if (acc >= work.d)
        acc = work.d;
    acc >>= 10;
    loc116 = acc;
    for (;;) {
        acc = loc116[m16+1].d;
        curExtent = acc;
        acc ^= work2.d;
        acc &= 0777LL << 39;
        if (acc) {
            if (--loc116.d)
                continue;
            throw ERR_NO_RECORD;
        }
        break;
    }
    acc = curExtent.d & 01777;
    ++acc;
    acc += curbuf.d;
    aitem = acc;
    acc = curExtent.d & 03776000;
    acc >>= 10;
    curpos = acc;
    m5 = ARBITRARY_NONZERO;     // clobbering the link register
}

void info(uint64_t arg) {
    find_item(arg);
    acc = (*aitem).d;
    desc1 = acc;
    acc &= 077777;
    itmlen = acc;
    loc53 = 0;
}

void totext() {
    char buf[13];
    snprintf(buf, sizeof(buf), " %05o", int(acc & 077777));
    strncpy((char*)&endmrk.d, buf, 6);
    acc = desc1.d;
    snprintf(buf, sizeof(buf), " %d%d.%d%d.X%d", int((acc >> 46) & 3),
            int((acc >> 42) & 15), int((acc >> 41) & 1),
            int((acc >> 37) & 15), int((acc >> 33) & 15));
    strncpy((char*)&desc1.d, buf, 8);
    strncpy((char*)&desc2.d, buf+8, 8);
}

void set_header() {
    *aitem = acc;
    dirty(1);
}

void copy_words(word dst, word src) {
    if (verbose)
        std::cerr << std::format("{:o}(8) words from {:o} to {:o}\n", m16.d, src.d, dst.d);
    // Using backwards store order to match the original binary for ease of debugging.
    while (m16.d) {
        dst[m16-1] = src[m16-1];
        --m16.d;
    }
}

// Copies chained extents to user memory (acc = length of the first extent)
void copy_chained() {           // a01423
    for (;;) {
        m16 = acc;
        if (m16.d) {
            if (verbose)
                std::cerr << "From DB: ";
            copy_words(usrloc, aitem);
        }
        usrloc = usrloc.d + curpos.d;
        acc = curExtent.d & (BITS(19) << 20);
        if (!acc)
            return;
        find_item(acc >> 20);
    }
}

// After cpyout must go to cmd0
void cpyout() {
    info(acc);
    usrloc = myloc;
    if (mylen.d && mylen.d < itmlen.d)
        throw ERR_TOO_LONG;
    // Skip the first word ot the found item
    ++aitem;
    --curpos;
    acc = curpos.d;
    copy_chained();
}

// Not really a mutex
void lock() {
    acc = bdtab.d << 20;
    work = acc |= IOpat.d;
    IOword = acc |= ONEBIT(40);
    IOcall(IOword);
    bdtab[1] = acc = bdtab[1].d ^ LOCKKEY;
    if (!(acc & LOCKKEY))
        throw ERR_LOCKED;
    bdbuf[0] = acc;
    IOcall(work);
}

void a00203() {
    Array[idx.d-1] = acc;
    acc &= BITS(19);
    loc116 = acc;
    acc = m16.d;
    usrloc = acc;
    acc += 2;
    element = acc;
    if (loc116 == loc246)
        return;
    loc246 = loc116;
    find_item(loc246.d);
    copy_chained();
}

void a00213() {
    acc = 02000;
    m16 = &loc54;
    a00203();
}

void pr202() {
    m16 = &loc116;
    a00203();
}

void skip(Error e) {
    acc = curcmd.d >> 6;
    if (!acc || (acc & 077))
        throw e;
    acc = curcmd.d >> 30;
}

void setctl(uint64_t location) {
    IOpat = location & 0777777;
    dblen = dbdesc >> 18;
    IOword = bdtab.d << 20 | IOpat.d | ONEBIT(40);
    IOcall(IOword);
    m16 = curbuf = bdtab;
    acc = (*m16).d;
    if (acc != DBkey.d)
        throw ERR_BAD_CATALOG;
    idx = 0;
    loc246 = 0;
    curZone = 0;
    acc = m16[3].d & 01777;
    acc += bdtab.d + 2;
    frebas = acc;
    a00213();
}

void free_extent() {
    d00031 = curExtent.d & 01777;
    do {
        --m5;
        if (!m5.d)
            break;
        acc = m16[m5+1].d & 01777;
        if (acc >= d00031.d)
            continue;
        acc = m16[m5+1].d + curpos.d;
        m16[m5+1] = acc;
    } while (true);
    work = m16[1].d & 01777;
    if (work != d00031) {
        if (d00031.d < work.d)
            throw ERR_INTERNAL;
        m5 = (d00031.d - work.d) & 077777;
        work = work.d + curbuf.d;
        d00031 = work.d + curpos.d;
        do {
            d00031[m5] = work[m5];
        } while(--m5.d);
    }
    m16[1] = m16[1].d - 02000 + curpos.d;
    frebas[curZone] = frebas[curZone].d + curpos.d + 1;
    dirty(1);
    acc = curExtent.d;
}

void free() {
    do {
        find_item(acc);
        work2 = (m16[1].d >> 10) & 077777;
        acc = loc116.d;
        m5 = acc;
        while (acc != work2.d) {
            m16[m5+1] = m16[m5+2].d;
            ++m5;
            acc = m5.d;
        };
        free_extent();
        acc = (acc >> 20) & BITS(19);
    } while (acc);
    dirty = dirty.d | 1;
}

void a00747() {
    free_extent();
    acc = (acc >> 20) & BITS(19);
    if (acc) {
        free();
        return;
    }
    dirty = dirty.d | 1;
}

void find_end_mark() {
    // Searching for the end mark (originally 1 or 2 bytes)
    m16 = -mylen.d;
    work2 = myloc.d + mylen.d;
    // Handling only single byte mark, contemporary style
    char * start = (char*)&(work2[m16].d);
    char * end = (char*)&(work2[0].d);
    char * found = (char*)memchr(start, endmrk.d & 0xFF, end-start);
    if (!found)
        throw ERR_NO_END_MARK;
    mylen = (found - start) / 8 + 1;
}

bool proc270() {
    acc = temp.d;
    m16 = acc;
    acc = (acc - curpos.d) & BITS(41);
    if (!(acc & ONEBIT(41))) {
        temp = acc;
        m16 = curpos;
        m5 = 0;
    }
    switch (jmpoff.d) {
    case FROMBASE:
        copy_words(usrloc, aitem);
        break;
    case TOBASE:
        dirty(1);
        copy_words(aitem, usrloc);
        break;
    case COMPARE:
    // Check for 0 length is not needed, perhaps because the key part of datum
    // which is being compared, is never empty?
        do {
            if (aitem[m16-1] != usrloc[m16-1]) {
                acc = curcmd >> 12;
                return true;
            }
        } while (--m16.d);
        break;
    case A00317:
        return false;
    case DONE:
        break;
    }
    usrloc = usrloc.d + curpos.d;
    return false;
}

bool step() {
    acc = Array[idx.d].d;
    if (!acc)
        throw ERR_NO_CURR;
    m5 = acc;
    if (!m16.d) {
        // Step forward
        acc += 2;
        m5 = acc;
        (acc ^= element[-1].d) &= 01777;
        if (!acc) {
            acc = element[-1].d;
            acc &= BITS(19) << 10;
            if (!acc) {
                skip(ERR_NO_NEXT);
                return true;
            }
            acc >>= 10;
            pr202();
            Array[idx.d-2] = Array[idx.d-2].d + (1<<15);
            m5 = 0;
        }
    } else {
        // Step back
        if (!m5.d) {
            acc = element[-1] >> 29;
            if (!acc) {
                skip(ERR_NO_PREV);
                return true;
            }
            pr202();
            acc = Array[idx.d-2].d;
            acc -= 1<<15;
            Array[idx.d-2] = acc;
            acc = loc117.d & 01777;
            m5 = acc;
        }
        m5 = m5 - 2;
    }
    acc = m5.d | 1 << 30;
    Array[idx.d] = acc;
    curkey = element[m5];
    adescr = element[m5+1];
    return false;
}

bool a00334() {
    temp = acc;
    loc53 = loc53.d + acc;
    usrloc = myloc;
  a00270:
    if (proc270()) {
        // COMPARE failed, skip an instruction
        return true;
    }
    if (m5.d) {
        aitem = aitem.d + temp.d;
        curpos = curpos.d - temp.d;
        acc = (*aitem).d;
        desc1 = acc;
    } else {
        acc = (curExtent.d >> 20) & BITS(19);
        if (acc) {
            find_item(acc);
            jump(a00270);
        }
        if (jmpoff.d == DONE
            // impossible? should compare with A00317, or deliberate as a binary patch?
            || temp.d) {
            skip(ERR_STEP);
            return true;
        }
    }
    return false;
}

void a00340() {
    find_item(acc);
    jmpoff = A00317;
    acc = desc2.d;
    a00334();
}

void assign_and_incr() {
    m16 = acc;
    m5 = (curcmd >> 6) & 077;
    curcmd = curcmd >> 6;
    m13[m16] = (*m13[m5]).d;
    ++m13[m5];
}

Error eval() try {
    std::unordered_map<void*,int> jumptab;
    std::vector<void*> targets;
    acc = m16.d;
    jump(enter);
    // The switch is entered with m6 = cmd0 and acc = 0
  switch_:
    if (verbose)
        std::cerr << "Executing microcode " << std::oct << m5.d << '\n';
    acc = 0;
    switch (m5.d) {
    case 0:
        break;
    case 1:
        temp = 0;
        jump(cmd1);
    case 2:
        acc = BITS(47);
        jump(find);
    case 3:
        m16 = 1;
        jump(cmd4);
    case 4:
        m16 = 0;
        jump(cmd4);
    case 5:
        d00011 = 0;
        make_metablock();
        jump(alloc);
    case 6:
        idx = 0;
        itmlen = 041;
        m16 = &loc54;
        acc = adescr.d;
        a00203();
        break;
    case 7:
        jmpoff = A00317;
        acc = desc2.d;
        a00334();
        break;
    case 010:
        jump(mkctl);
    case 011:
        acc = key.d;
        jump(find);
    case 012:
        myloc = &dbdesc;
        mylen = 3;
        acc = adescr.d;
        cpyout();
        break;
    case 013:
        usable_space();
        break;
    case 014:
        if (curkey == key)
            break;
        skip(ERR_NO_NAME);
        jump(next);
    case 015:
        if (curkey != key)
            break;
        skip(ERR_EXISTS);
        jump(next);
    case 016:
        find_end_mark();
        break;
    case 017:
        jump(cmd17);
    case 020:
        acc = myloc.d;
        jump(cmd20);
    case 021:
        acc = myloc.d;
        jump(alloc);
    case 022:
        acc = adescr.d;
        cpyout();
        break;
    case 023:
        acc = adescr.d;
        free();
        break;
    case 024:
        if (givenp != savedp)
            throw ERR_WRONG_PASSWORD;
        break;
    case 025:
        setctl(dbdesc.d);
        break;
    case 026:
        jump(inskey);
    case 027:
        limit = 0;              // looks dead
        jump(delkey);
    case 030:
        acc = orgcmd.d;
        jump(next);
    case 031:
        DBkey = ROOTKEY;
        dbdesc = arch;
        setctl(dbdesc.d);
        break;
    case 032:
        *aitem = loc12;
        dirty(2);
        break;
    case 033:
        info(adescr.d);
        break;
    case 034:
        acc = desc1.d;
        totext();
        break;
    case 035:
        acc = dirty.d;
        save();
        if (mars_flags.dump_diffs)
            dump();
        return ERR_SUCCESS;
    case 036:
        jump(cmd36);
    case 037:
        acc = curcmd >> 12;
        jump(next);
    case 040:
        jump(next);
    case 041:
        jmpoff = COMPARE;
        acc = mylen.d;
        if (a00334())
            jump(next);
        break;
    case 042:
        jmpoff = TOBASE;
        acc = mylen.d;
        if (a00334())
            jump(next);
        break;
    case 043:
        jmpoff = FROMBASE;
        acc = mylen.d;
        if(a00334())
            jump(next);
        break;
    case 044:
        adescr = desc1;
        break;
    case 045:
        lock();
        break;
    case 046:
        jump(cmd46);
    case 047:
        acc = desc1.d;
        indjump(outadr);
    case 050:
        // cmd := mem[bdvect[src]++]
        // curcmd is ..... src 50
        acc = &orgcmd-BDVECT;
        assign_and_incr();
        jump(next);
    case 051:
        // myloc := mem[bdvect[src]++]
        // curcmd is ..... src 51
        acc = &myloc-BDVECT;
        assign_and_incr();
        break;
    case 052:
        // bdvect[dst] := mem[bdvect[src]++]
        // curcmd is ..... src dst 52
        acc = (curcmd >> 6) & 077;
        curcmd = curcmd >> 6;
        assign_and_incr();
        break;
    case 053:
        // bdvect[dst] = bdvect[src]
        // curcmd is ..... src dst 53
        m16 = (curcmd >> 6) & 077;
        m5 = (curcmd >> 12) & 077;
        curcmd = curcmd >> 12;
        m13[m16] = m13[m5];
        break;
    case 054:
        // mem[bdvect[dst]] = bdvect[012]
        // curcmd is ..... dst 54
        m16 = (curcmd >> 6) & 077;
        curcmd = curcmd >> 6;
        *m13[m16] = loc12;
        break;
    case 055:
        if (mars_flags.dump_diffs)
            dump();
        return ERR_SUCCESS;
    default:
        // In the original binary, loss of control ensued.
        std::cerr << std::format("Invalid micro-operation {:o} encountered\n", m5.d);
        abort();
    }
    jump(cmd0);
  enter:
    savm16 = acc;
    savm13 = m13;
    savrk = rk;
    m13 = abdv;
    savm7 = m7;
    savm5 = m5;
    if (bdtab[0] != DBkey && IOpat.d) {
        IOword = bdtab.d << 20 | ONEBIT(40) | IOpat.d;
        IOcall(IOword);
    }
    savm6 = m6;
    // enter2:                       // continue execution after a callback?
    acc = orgcmd.d;
    jump(next);
  drtnxt:
    dirty = dirty.d | 1;
  rtnext:
    if (goto_.d) {
        word to = goto_;
        goto_ = 0;
        indjump(to);
    }
  cmd0:
    acc = curcmd >> 6;
  next:
    if (!acc) {
        finalize();
        if (mars_flags.dump_diffs)
            dump();
        return ERR_SUCCESS;
    }
    curcmd = acc;
    m5 = acc & BITS(6);
  a00153:
    m6 = target(cmd0);
    goto_ = 0;
    jump(switch_);
  find:
    temp = acc;
    if (!acc || acc & ONEBIT(48))
        throw ERR_INV_NAME;
    acc = 0;
  cmd1:
    idx = acc;
    temp = temp.d ^ BITS(47);
    while (true) {
        m16 = Metadata;
        if (loc20.d)
            break;
        if (verbose)
            std::cerr << "Loc20 == 0, nothing to compare\n";
        a00213();
    }
  a00164:
    acc = m16[-1].d & 01777;
    m5 = acc;
    if (verbose)
        std::cerr << "Comparing " << std::dec << acc/2 << " elements\n";
    while (m5.d) {
        acc = m5[m16-2].d;
        acc += temp.d;
        acc = (acc + (acc >> 48)) & BITS(48);
        if (!(acc & ONEBIT(48)))
            break;
        m5.d -= 2;
    }
    m5.d -= 2;
    acc = m5.d;
    acc |= ONEBIT(31);
    Array[idx.d] = acc;
    acc = m16[1].d;
    acc &= ONEBIT(48);
    if (!acc) {
        curkey = element[m5];
        adescr = element[m5+1];
        jump(rtnext);
    }
    idx = idx.d + 2;
    if (idx.d > 7) {
        std::cerr << "Idx = " << idx.d << ": DB will be corrupted\n";
    }
    acc = m16[m5+1].d;
    pr202();
    m16 = &loc120;
    jump(a00164);
  cmd46:                        // Unpacking the descriptor (?) in myloc
    acc = myloc.d;
    acc >>= 18;
    acc &= 077777;
    desc2 = acc -= loc53.d;
    acc = (myloc >> 33) & 017777;
    if (!acc) {
        acc = curcmd.d >> 30;
        jump(next);
    }
    mylen = acc;
    indjump(m6);
  cmd36:
    m16 = element;
    m16[Array[idx.d]+1] = loc12;
    jump(a01160);
    // pr376 (make_metablock) was here
  cmd4:
    if (step())
        jump(next);
    jump(cmd0);
  cmd17:
    // Searching for the end word
    m16 = -mylen.d;
    work2 = myloc.d + mylen.d;
    // loop_here:
    if (work2[m16] == endmrk) {
        m16 = m16.d + mylen.d + 1;
        mylen = m16;
        jump(cmd0);
    }
    if (m16 != 0) {
        ++m16;
        jump(cmd17);            // Possibly a typo/oversight, ought to be loop_here
    }
    throw ERR_INTERNAL;         // Originally "no end word"
  a00667:
    acc = idx.d;
    if (!acc)
        jump(a01160);
    acc = loc246.d;
    free();
    desc2 = 1;
    acc = loc117.d;
    d00025 = acc;
    acc &= BITS(19) << 29;
    d00032 = acc;
    acc >>= 29;
    if (acc) {
        a00340();
        acc &= ~(BITS(19) << 10);
        loc117 = acc;
        acc = d00025.d;
        acc &= BITS(19) << 10;
        acc |= loc117.d;
        set_header();
    }
    call(pr1232,m5);
  delkey:
    acc = element.d;
    m16 = acc;
    acc += Array[idx.d].d;
    m5 = acc;
    loc116 = acc ^ element.d;
    acc = m16[-1].d & 01777;
    work2 = acc;
    work = acc + element.d;
    do {
        *m5 = m5[2].d;
        ++m5;
    } while (m5 != work);
    acc = work2.d - 2;
    acc ^= work2.d;
    acc ^= m16[-1].d;
    m16[-1] = acc;
    acc &= 01777;
    if (!acc)
        jump(a00667);
    acc = loc116.d;
  a00721:
    acc &= 077777;
    if (!acc) {
        d00011 = m16[0];
        goto_ = target(a00730);
        d00033 = target(a00731);
    }
    jump(a01160);
  a00730:
    jump(a01242);
  a00731:
    acc = d00011.d;
    m16 = element;
    m16[Array[idx.d]] = acc;
    acc = Array[idx.d].d;
    jump(a00721);
    // date() was here
  alloc:
    usrloc = acc;
    call(pr1022,m6);
    loc12 = d00024;
    jump(cmd0);
  pr1022:
    date();
  a01023:
    m5 = 0;
    d00030 = 0;
    m16 = frebas;
    work = frebas[curZone];
    ++mylen;
    if (mylen.d >= work.d) {
        if (verbose)
            std::cerr << "mylen = " << mylen.d << " work = " << work.d << '\n';
        // If the datum is larger than MAXCHUNK, it will have to be split
        if (mylen.d >= MAXCHUNK)
            jump(split);
        // Find a zone with enough free space
        for (size_t i = 0; i < dblen.d; ++i) {
            if (mylen.d < frebas[i].d) {
                acc = i;
                jump(chunk);
            }
        }
        jump(split); // End reached, must split
    }
    acc = curZone.d;
  chunk:
    getzon();
    m7 = m16;
    acc = m7[1].d;
    acc >>= 10;
    m16 = acc;
    do {
        acc <<= 39;
        acc ^= curbuf[m16+1].d;
        acc &= 0777LL << 39;
        if (!acc)
            break;
        curbuf[m16+2] = curbuf[m16+1];
        --m16;
        acc = m16.d;
    } while (m16.d);
    ++m16;
    acc = m16.d;
    loc116 = acc;
    acc <<= 39;
    acc &= BITS(48);
    acc |= d00030.d;
  pr1047:
    d00030 = acc;
    acc >>= 39;
    acc <<= 10;
    acc |= curZone.d;
    d00024 = acc;
    m16 = mylen;
    acc = m7[1].d - m16.d;
    acc += 02000;
    m7[1] = acc;
    acc &= 01777;
    work = acc;
    acc += 1 + curbuf.d;
    work2 = acc;
    if (!m5.d) {
        --m16;
        ++work2;
    }
    if (m16.d) {
        if (verbose)
            std::cerr << "To DB: ";
        copy_words(work2, usrloc);
    }
    loc116[m7+1] = (mylen.d << 10) | work.d | d00030.d;
    m16 = frebas;
    if (verbose)
        std::cerr << "Reducing free " << std::oct << frebas[curZone].d
                  << " by len " << mylen.d << " + 1\n";
    frebas[curZone] = frebas[curZone].d - (mylen.d+1);
    if (verbose)
        std::cerr << "Got " << frebas[curZone].d << '\n';
    if (!m5.d) {
        work2[-1] = datlen;
        dirty(2);
        indjump(m6);
    }
    dirty(1);
    acc = d00024.d;
    acc <<= 20;
    acc &= BITS(48);
    d00030 = acc;
    mylen = remlen;
  a01105:
    if (--m5.d)
        jump(a01264);
    acc = d00030.d;
    acc = (acc >> 20) & BITS(19);
    if (acc) {
        free();
        throw ERR_OVERFLOW;
    }
    dirty = dirty.d | 1;
    throw ERR_OVERFLOW;
  inskey:
    d00011 = key;
    acc = loc12.d;
    adescr = acc;
    jump(a01140);
  a01123:
    acc = Array[idx.d].d;
    acc >>= 15;
    for (;;) {
        d00012 = acc;
        acc >>= 15;
        m16 = acc;
        if (!(d00012.d & 077777)) {
            indjump(d00033);    // return from pr1232
        }
        acc = d00012.d;
        --m16;
        if (m16.d) {
            acc += 2;
        }
        --acc;
        d00012 = acc;
        if (step())
            jump(next);
        acc = d00012.d;
    }
  a01136:
    acc = d00032.d;
    acc >>= 29;
    acc |= ONEBIT(48);
  a01140:
    d00024 = acc;
    acc = element.d;
    m16 = acc & 077777;
    acc += Array[idx.d].d + 2;
    acc &= 077777;
    m5 = acc;
    limit = acc;
    acc = m16[-1].d;
    acc &= 01777;
    m6 = acc + m16.d;
    if (verbose)
        std::cerr << "Expanding " << std::dec << (m6.d-limit.d)/2 << " elements\n";
    while (m6 != limit) {
        m6[0] = m6[-2];
        m6[1] = m6[-1];
        m6 = m6.d - 2;
    }
    m5[0] = d00011;
    m5[1] = d00024;
    m16[-1] = m16[-1].d + 2;
  a01160:
    acc = m16.d;
    usrloc = --acc;
    acc = idx.d;
    if (!acc) {
        mylen = 041;
        if (m16[-1].d == 040)
            jump(a01252);
        acc = loc20.d;
        jump(a01311);
    }
    acc = m16[-1].d & 01777;
    if (acc != 0100) {
        acc = m16[-1].d & 01777;
        mylen = ++acc;
        acc = loc246.d;
        jump(a01311);
    }
    work = (idx.d * 2) + 041;
    if (Metadata[-1].d == 036) {
        // The current metadata block is full, account for another one
        work = work.d + 044;
    }
    if (usable_space() < work.d) {
        loc20 = 0;
        acc = loc12.d;
        free();
        throw ERR_OVERFLOW;
    }
    d00025 = loc157;
    loc157 = (loc246.d << 29) & BITS(48);
    acc = loc117.d;
    acc &= BITS(19) << 10;
    acc |= loc157.d | 040;
    loc157 = acc;
    usrloc.d += 040;
    mylen = 041;
    call(pr1022,m6);
    loc157 = d00025;
    d00011 = loc160;
    acc = (d00025 = loc117).d;
    acc >>= 29;
    acc <<= 19;
    acc |= d00024.d;
    acc <<= 10;
    acc &= BITS(48);
    acc |= 040;
    loc117 = acc;
    acc &= BITS(19) << 10;
    d00032 = (acc << 19) & BITS(48);
    m16 = &loc120;
    goto_ = target(a01231);
    usrloc = m16.d - 1;
    acc = m16[-1].d & 01777;
    mylen = ++acc;
    acc = loc246.d;
    jump(a01311);
  a01231:
    m5 = target(a01136);
  pr1232:
    d00033 = m5.d;
    desc2 = 1;
    acc = (d00025 >> 10) & BITS(19);
    if (acc) {
        a00340();
        acc &= BITS(29);
        acc |= d00032.d;
        set_header();
    }
  a01242:
    element = Metadata;
    acc = idx.d;
    if (!acc)
        jump(drtnxt);
    acc -= 2;
    idx = acc;
    if (idx != 0) {
        acc = Array[idx.d-1].d;
        pr202();
    }
    jump(a01123);
  a01252:
    do {
        if (usable_space() < 0101) {
            loc20 = 0;
            acc = loc12.d;
            free();
            throw ERR_OVERFLOW;
        }
        acc -= 0101;
        call(pr1022,m6);
        m16 = Metadata;
        acc = d00024.d;
        acc |= ONEBIT(48);
        m16[1] = acc;
        m16[-1] = 2;
        mylen = 041;
    } while (m16[-1].d == 040);
    acc = loc20.d;
    jump(a01311);
  split:
    usrloc = usrloc.d + mylen.d - 1;
    m5 = dblen;
  a01264:
    if (frebas[m5-1].d < 2)
        jump(a01105);
    acc = frebas[m5-1].d - 1;
    work = acc;
    --m5;
    if (acc < mylen.d) {
        remlen = mylen.d - work.d;
        mylen = work.d;
        usrloc = usrloc.d - work.d;
        acc = m5.d;
        ++m5;
    } else {
        acc = usrloc.d;
        acc -= mylen.d;
        acc += 1;
        usrloc = acc;
        acc = m5.d;
        m5 = 0;
    }
    jump(chunk);
  cmd20:
    usrloc = acc;
    acc = adescr.d;
  a01311:
    d00012 = acc;
    m6 = target(drtnxt);
    find_item(d00012.d);
    --acc;
    m5 = acc;
    acc ^= mylen.d;
    if (!acc) {
        // The new length of data matches the first extent length
        if (m5.d) {
            do {
                aitem[m5] = usrloc[m5-1].d;
            } while (--m5.d);
        }
        m5 = m16;
        date();
        *aitem = acc;
        dirty(1);
        acc = curExtent.d & (BITS(19) << 20);
        if (!acc)
            jump(rtnext);       // No extents to free: done
        acc ^= curExtent.d;
        loc116[m5+1] = acc;
        acc = curExtent.d;
        acc = (acc >> 20) & BITS(19);
        if (acc) {
            free();
            indjump(m6);
        }
        dirty = dirty.d | 1;
        indjump(m6);
    }
    date();
    m16 = curbuf;
    acc = m16[1] >> 10;
    m5 = acc + 1;
    acc = (m16[1].d - acc) & 01777;
    acc += curpos.d - 2;
    work = acc;
    if (acc >= mylen.d) {
      a01346:
        d00010 = m6;
        d00026 = curExtent;
        curExtent = curExtent.d & 01777;
        a00747();
        ++mylen;
        m5 = 0;
        m7 = curbuf;
        acc = d00026.d & (0777LL << 39);
        call(pr1047,m6);
        acc = d00026.d;
        m6 = d00010;
        acc = (acc >> 20) & BITS(19);
        if (acc) {
            free();
            indjump(m6);
        }
        dirty = dirty.d | 1;
        indjump(m6);
    }
    usable_space();
    acc = (*aitem).d & 077777;
    acc += itmlen.d;
    if (acc < mylen.d) {
        throw ERR_OVERFLOW;
    }
    m16 = curbuf;
    mylen = work.d;
    call(a01346,m6);
    usrloc = usrloc.d + mylen.d;
    mylen = (datlen.d & 077777) - mylen.d;
    datlen = usrloc[-1];
    call(a01023,m6);
    find_item(d00012.d);
    acc = (d00024.d << 20) & BITS(48);
    loc116[m16+1] = acc | curExtent.d;
    dirty(2);
    jump(rtnext);
  mkctl: {
        curZone = 0;
        m5 = dblen = dbdesc >> 18;
        IOpat = dbdesc.d & 0777777;
        m6 = bdtab;
        curbuf = m6;
        work2 = IOpat.d | bdtab.d << 20;
        bdtab[1] = 01777;
        do {
            --m5;
            bdbuf[m5] = 01776;
            bdtab[0] = DBkey.d | m5.d;
            IOword = work2.d + m5.d;
            IOcall(IOword);
        } while (m5.d);
        myloc = frebas = bdbuf; // frebas[0] is now the same as bdbuf[0]
        loc20 = 02000;
        d00011 = 0;
        make_metablock();
        call(pr1022, m6);
        mylen = dblen.d;
        // This trick results in max possible DB length = 753 zones.
        bdbuf[0] = 01731 - mylen.d;
        m5 = 021;
        jump(a00153);
    }
} catch (Error e) {
    m16 = e;
    acc = erhndl.d;
    if (acc)
        savm16 = acc;
    m7 = m16;
    std::cerr << std::format("ERROR {} ({})\n", m16.d, msg[m16.d-1]);
    d00010 = *(uint64_t*)msg[m16.d-1];
    acc = *((uint64_t*)msg[m16.d-1]+1);
    finalize();
    if (mars_flags.dump_diffs)
        dump();
    return e;
}

void pasbdi() {
    // Faking MARS words
    m16 = 04700;
    m13 = 01654;
    m7 = 01603;
    m5 = 077770;
    m6 = 022731;
    bdtab = BDTAB;
    bdbuf = BDBUF;
    data[BDSYS+0042] = 02300'0150'0000'0000;
    data[BDSYS+0043] = 02000'0027'2300'0160;
    data[BDSYS+0044] = 02010'1532'2300'0156;
    data[BDSYS+0045] = 02010'1533'0000'0000;
    data[BDSYS+0046] = 00040'0016'2300'0410;
    data[BDSYS+0047] = 02000'0011'2300'0375;
    data[BDSYS+0050] = 05400'0242'2300'0404;
    abdv = BDVECT;
    for (int i = 0; i < 041; ++i)
        data[FAKEBLK+i].d = ARBITRARY_NONZERO;
}

int to_lnuzzzz(int lun, int start, int len) {
    if (lun > 077 || start > 01777 || len > 0777)
        std::cerr << std::oct << lun << ' ' << start << ' ' << len
                  << "out of valid range, truncated\n";
    return ((lun & 077) << 12) | (start & 01777) | ((len & 0777) << 18);
}

Error SetDB(int lun, int start, int len) {
    pasbdi();
    arch = to_lnuzzzz(lun, start, len);
    return root();
}

Error InitDB(int lun, int start, int len) {
    pasbdi();
    dbdesc = to_lnuzzzz(lun, start, len);
    DBkey = ROOTKEY;
    orgcmd = 010;
    return eval();
}

// Replicating what the NEWD option does in the PAIB Pascal API function
// to match the sequence of store ops as close as possible
Error newd(const char * k, int lun, int start, int len) {
    int lnuzzzz = to_lnuzzzz(lun, start, len);
    if (verbose)
        std::cerr << "Running newd('" << k << "', " << std::oct << lnuzzzz << ")\n";
    key = *reinterpret_cast<const uint64_t*>(k);
    data[077776] = lnuzzzz;
    data[077777] = (key.d << 10) & BITS(48);
    key = *reinterpret_cast<const uint64_t*>(k);
    mylen = 1;
    myloc = ARBITRARY_NONZERO;
    orgcmd = 0;
    myloc = 077776;
    mylen = 2;
    orgcmd = 02621151131LL;
    eval();
    orgcmd = 010121411;
    return eval();
}

Error opend(const char * k) {
    if (verbose)
        std::cerr << "Running opend('" << k << "')\n";
    key = *reinterpret_cast<const uint64_t*>(k);
    orgcmd = 02512141131;
    return eval();
}

Error putd(const char * k, int loc, int len) {
    if (verbose)
        std::cerr << std::format("Running putd('{}', '{}':{})\n", k,
                                 reinterpret_cast<char*>(data+loc), len);
    key = *reinterpret_cast<const uint64_t*>(k);
    mylen = len;
    myloc = loc;
    orgcmd = 026211511;
    return eval();
}

Error putd(uint64_t k, int loc, int len) {
    if (verbose) {
        std::cerr << std::format("Running putd({:016o}, {:05o}, {})\n", k, loc, len);
    }
    key = k;
    mylen = len;
    myloc = loc;
    orgcmd = 026211511;
    return eval();
}

Error putd(const char * k, const char * v) {
    size_t len = (strlen(v)+7)/8;
    if (verbose)
        std::cerr << "Running putd('" << k << "', '"
                  << v << "':" << len << ")\n";
    key = *reinterpret_cast<const uint64_t*>(k);
    strcpy((char*)(data+02000), v);
    mylen = len;
    myloc = 02000;
    orgcmd = 026211511;
    return eval();
}

Error modd(const char * k, int loc, int len) {
    key = *reinterpret_cast<const uint64_t*>(k);
    mylen = len;
    myloc = loc;
    orgcmd = 020402621001511;
    return eval();
}

Error modd(uint64_t k, int loc, int len) {
    key = k;
    mylen = len;
    myloc = loc;
    orgcmd = 020402621001511;
    return eval();
}

Error getd(const char * k, int loc, int len) {
    key = *reinterpret_cast<const uint64_t*>(k);
    mylen = len;
    myloc = loc;
    orgcmd = 0221411;
    return eval();
}

Error getd(uint64_t k, int loc, int len) {
    if (verbose) {
        std::cerr << "Running getd(" << std::oct << std::setw(16) << std::setfill('0') << k << ", ";
        std::cerr << std::setw(5) << loc << ":" << std::setw(0) << std::dec << len << ")\n";
        std::cerr.copyfmt(std::ios(nullptr));
    }
    key = k;
    mylen = len;
    myloc = loc;
    orgcmd = 0221411;
    return eval();
}

Error deld(const char * k) {
    key = *reinterpret_cast<const uint64_t*>(k);
    orgcmd = 027231411;
    return eval();
}

Error deld(uint64_t k) {
    idx = 0;
    key = k;
    orgcmd = 027231411;
    return eval();
}

Error root() {
    orgcmd = 031;
    return eval();
}

uint64_t first() {
    orgcmd = 0401;
    eval();
    return curkey.d;
}

uint64_t last() {
    orgcmd = 02;
    eval();
    return curkey.d;
}

uint64_t prev() {
    orgcmd = 03;
    eval();
    return curkey.d;
}

uint64_t next() {
    orgcmd = 04;
    eval();
    return curkey.d;
}

uint64_t find(const char * k) {
    key = *reinterpret_cast<const uint64_t*>(k);
    orgcmd = 011;
    eval();
    return curkey.d;
}

uint64_t find(uint64_t k) {
    key = k;
    orgcmd = 011;
    eval();
    return curkey.d;
}

int getlen() {
    orgcmd = 033;
    if (eval())
        return -1;
    return itmlen.d;
}

Error cleard() {
    key = 0;
    orgcmd = 030272302;
    return eval();
}

int avail() {
    orgcmd = 013;
    itmlen = 0;
    eval();
    return itmlen.d;
}
