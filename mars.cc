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

using word = Mars::word;
using Error = Mars::Error;

const uint64_t rk = 7LL << 41;

#define FAKEBLK 020
#define ARBITRARY_NONZERO 01234567007654321LL
#define ROOTKEY 04000
#define LOCKKEY ONEBIT(32)

static const std::vector<int> comparable{
    3,5,010,012,013,014,015,020,021,022,023,024,025,026,027,
    031,033,034,035,036,037,040,041,042,043,044,045,046,047,050,051,052,053,054,
    055,056,0116,0117,0120,0157,0160,0220,0241,0242,0243,0244,0245,0246,0247
};

void Mars::dump() {
    for (auto j : comparable) {
        if (bdvect().w[j] != sav.w[j]) {
            std::cout << std::format(" WORD {:03o} CHANGED FROM  {:016o} TO  {:016o}\n",
                                     int(j), sav.w[j].d, bdvect().w[j].d);
        }
    }
    sav = bdvect();
}

word& word::operator*() const {
    unsigned a = d & 077777;
    if (a >= Mars::RAM_LENGTH) {
      std::cerr << std::format("Address {:o} out of range\n", a);
        abort();
    }
    return mars->data[a];
}

word& word::operator[](word x) const {
    if ((x.d & 077770) != 077770 &&
        ((x.d & 077777) >= 02000 ||
         ((x.d & 077777) >= 01400 && (d & 077777) < 0400)))
        std::cerr << "Base/index inversion\n";
    return *word(*mars, d+x.d);
}

uint64_t word::operator&() const { return this-mars->data; }

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
// #define jump(x) do { std::cerr << "Jump to " #x "\n"; goto x; } while(0)
#define jump(x) goto x
#define indjump(x)                                                      \
    do {                                                                \
        assert ((x.d & ~077LL) == INDJUMP_MAGIC &&                      \
                (x.d & 077) < targets.size());                          \
        goto *(targets[x.d & 077]);                                     \
    } while (false)

struct Page {
    Page() { }
    Page(word * page) {
        for (int i = 0; i < 1024; ++i)
            w[i] = page[i].d;
    }
    void to_memory(word * page) {
        for (int i = 0; i < 1024; ++i)
            page[i].d = w[i];
    }
    uint64_t w[1024];
};

struct MarsImpl {
    static const int BDVECT = Mars::BDVECT;
    static const int BDSYS = Mars::BDSYS;
    typedef word & wordref;
    Mars & mars;
    word * const data;
    bool & verbose;
    // Registers
    wordref m5, m6, m7, m13, m16;

    // For convenience of detection which locations not to trace,
    // the registers are mapped to the lower addresses of the RAM.

    // Fields of BDVECT
    wordref outadr, orgcmd, curcmd, syncw, givenp, key,
        erhndl, curDescr, myloc, loc14, mylen, bdbuf, bdtab,
        loc20, dbdesc, DBkey, savedp, freeSpace, adescr, limit,
        curkey, endmrk, desc1, desc2, IOpat, curExtLength, aitem, itmlen,
        element, dblen, curlen, loc54, loc116, loc117,
        loc157, loc160, curExtent, curbuf, curZone, loc246,
        dirty;

    uint64_t & idx;
// Metadata[-1] (loc55) is also used
    word * const Array;
    word * const Metadata;
    word * const Loc120;

    // Fields of BDSYS
    uint64_t & arch;
    wordref abdv, savm16, d00010, d00011, d00012,
        savm13, goto_, zoneKey, usrloc, savm7, savm6,
        savm5, jmpoff, extentHeader, d00024, d00025, d00026,
        temp, d00030, d00031, d00032, d00033, remlen,
        work, work2, IOword, d00040, savrk;

    std::unordered_map<std::string, Page> DiskImage;

    // History of all stores, for debug.
    std::unordered_map<size_t, uint64_t> all_stores;

    MarsImpl(Mars & up) :
        mars(up), data(up.data), verbose(up.verbose),
        m5(data[5]), m6(data[6]), m7(data[7]), m13(data[013]), m16(data[016]),
        outadr(data[BDVECT+1]),
        orgcmd(data[BDVECT+3]),
        curcmd(data[BDVECT+5]),
        syncw(data[BDVECT+6]),
        givenp(data[BDVECT+7]),
        key(data[BDVECT+010]),
        erhndl(data[BDVECT+011]),
        curDescr(data[BDVECT+012]),
        myloc(data[BDVECT+013]),
        loc14(data[BDVECT+014]),
        mylen(data[BDVECT+015]),
        bdbuf(data[BDVECT+016]),
        bdtab(data[BDVECT+017]),
        loc20(data[BDVECT+020]),
        dbdesc(data[BDVECT+030]),
        DBkey(data[BDVECT+031]),
        savedp(data[BDVECT+032]),
        freeSpace(data[BDVECT+034]),
        adescr(data[BDVECT+035]),
        limit(data[BDVECT+036]),
        curkey(data[BDVECT+037]),
        endmrk(data[BDVECT+040]),
        desc1(data[BDVECT+041]),
        desc2(data[BDVECT+042]),
        IOpat(data[BDVECT+044]),
        curExtLength(data[BDVECT+045]),
        aitem(data[BDVECT+046]),
        itmlen(data[BDVECT+047]),
        element(data[BDVECT+050]),
        dblen(data[BDVECT+051]),
        curlen(data[BDVECT+053]),
        loc54(data[BDVECT+054]),
        loc116(data[BDVECT+0116]),
        loc117(data[BDVECT+0117]),
        loc157(data[BDVECT+0157]),
        loc160(data[BDVECT+0160]),
        curExtent(data[BDVECT+0220]),
        curbuf(data[BDVECT+0241]),
        curZone(data[BDVECT+0244]),
        loc246(data[BDVECT+0246]),
        dirty(data[BDVECT+0247]),

        idx(data[BDVECT+0242].d),
        Array(data+BDVECT+021),
        // Metadata[-1] (loc55) is also used
        Metadata(data+BDVECT+056),
        Loc120(data+BDVECT+0120),

        // Fields of BDSYS
        arch(data[BDSYS+4].d),
        abdv(data[BDSYS+3]),
        savm16(data[BDSYS+7]),
        d00010(data[BDSYS+010]),
        d00011(data[BDSYS+011]),
        d00012(data[BDSYS+012]),
        savm13(data[BDSYS+013]),
        goto_(data[BDSYS+014]),
        zoneKey(data[BDSYS+015]),
        usrloc(data[BDSYS+016]),
        savm7(data[BDSYS+017]),
        savm6(data[BDSYS+020]),
        savm5(data[BDSYS+021]),
        jmpoff(data[BDSYS+022]),
        extentHeader(data[BDSYS+023]),
        d00024(data[BDSYS+024]),
        d00025(data[BDSYS+025]),
        d00026(data[BDSYS+026]),
        temp(data[BDSYS+027]),
        d00030(data[BDSYS+030]),
        d00031(data[BDSYS+031]),
        d00032(data[BDSYS+032]),
        d00033(data[BDSYS+033]),
        remlen(data[BDSYS+034]),
        work(data[BDSYS+035]),
        work2(data[BDSYS+036]),
        IOword(data[BDSYS+037]),
        d00040(data[BDSYS+040]),
        savrk(data[BDSYS+041]) { }
    void IOflush();
    void IOcall(word);
    void get_zone(uint64_t);
    void save(bool);
    void finalize(word);
    void date();
    void make_metablock();
    uint64_t usable_space();
    void find_item(uint64_t);
    void info(uint64_t);
    void totext();
    void set_header(word);
    void copy_words(word dst, word src, int len);
    void copy_chained(int len);
    void cpyout(uint64_t);
    void lock();
    void a00203(uint64_t), a00213(), pr202(uint64_t);
    void free_from_current_extent(), a00340(uint64_t);
    void skip(Error);
    void setctl(uint64_t);
    void free_extent(), free(uint64_t);
    void find_end_mark(), find_end_word();
    bool proc270(), step(), a00334(word);
    bool cmd46();
    void assign_and_incr();
    void pasbdi();
    Error eval();
    void overflow();
    void prepare_chunk();
    void allocator(int);
    void mkctl(), find(word);
    void a01346(), a00164(), a01311(word);
    void setDirty(int x) {
        dirty = dirty.d | ((curZone.d ? x : 0) + 1);
    }

};

uint64_t Mars::get_store(size_t index) {
    return impl.all_stores[index];
}

uint64_t& word::store(uint64_t x) {
    assert(mars);
    size_t index = this-mars->data;
    if (index < 16) x &= 077777;
    if (mars->trace_stores && index > 16 && index < Mars::RAM_LENGTH) {
        std::cerr << std::format("       {:05o}: store {:016o}\n", index, x);
    }
    if (mars->memoize_stores && index > 16 && index < Mars::RAM_LENGTH) {
        mars->impl.all_stores[index] = x;
    }
    d=x;
    return d;
}
word& word::operator=(word x) {
    assert(mars);
    size_t index = this-mars->data;
    if (index < 16) x.d &= 077777;
    if (mars->trace_stores && index > 16 && index < Mars::RAM_LENGTH) {
        std::cerr << std::format("       {:05o}: store {:016o}\n", index, x.d);
    }
    if (mars->memoize_stores && index > 16 && index < Mars::RAM_LENGTH) {
        mars->impl.all_stores[index] = x.d;
    }
    d=x.d;
    return *this;
}
word& word::operator=(word * x) {
    assert(mars);
    ptrdiff_t offset = x-mars->data;
    if (offset < 0 || offset >= 077777) {
        std::cerr << std::format("Cannot point to words outside of RAM, offset = {:o}\n", offset);
        abort();
    }
    size_t index = this-mars->data;
    if (mars->trace_stores && index > 16 && index < Mars::RAM_LENGTH) {
        std::cerr << std::format("       {:05o}: store {:016o}\n", index, offset);
    }
    if (mars->memoize_stores && index > 16 && index < Mars::RAM_LENGTH) {
        mars->impl.all_stores[index] = offset;
    }
    d=offset;
    return *this;
}

Mars::Mars() : impl(*new MarsImpl(*this)) { }
Mars::Mars(bool persistent) : impl(*new MarsImpl(*this)), flush(persistent) { }

Mars::~Mars() {
    if (flush) {
        impl.IOflush();
    }
    delete &impl;
}

template<class T> uint64_t next_extent(const T & x) {
        return (x.d >> 20) & BITS(19);
}

template<class T> uint64_t get_extlength(const T & x) {
        return (x.d >> 10) & BITS(10);
}

template<class T> uint64_t get_id(const T & x) {
        return x.d >> 39;
}

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

enum Ops {
    FROMBASE, TOBASE, COMPARE, A00317, DONE
};

const char * msg[] = {
    "Zero key", "Page corrupted", "No such record", "Invalid name",
    "1st page corrupted", "Overflow", "Step too big", "No such name",
    "Name already exists", "No end symbol", "Internal error", "Record too long",
    "DB is locked", "No current record", "No prev. record", "No next record",
    "Wrong password" };

void MarsImpl::IOflush() {
    for (auto & it : DiskImage) {
        const std::string& nuzzzz = it.first;
        FILE * f = fopen(nuzzzz.c_str(), "w");
        if (!f) {
            std::cerr << std::format("Could not open {} ({})\n", nuzzzz, strerror(errno));
            exit(1);
        }
        fwrite(&it.second, 1, sizeof(Page), f);
        fclose(f);

        if (mars.dump_txt_zones) {
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
}

void MarsImpl::IOcall(word is) {
    int page = (is >> 30) & BITS(5);
    std::string nuzzzz;
    bool stores = mars.trace_stores;
    mars.trace_stores = false;
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
                for (int i = page*1024; i < (page+1)*1024; ++i)
                    data[i] = ARBITRARY_NONZERO;
                mars.trace_stores = stores;
                return;
            }
            if (verbose)
                std::cerr << "\tFirst time - reading from disk\n";
            fread(&DiskImage[nuzzzz], 1, sizeof(Page), f);
            fclose(f);
            DiskImage[nuzzzz].to_memory(data+page*1024);
        } else
            it->second.to_memory(data+page*1024);
    } else {
        // write
        if (verbose)
            std::cerr << std::format("Writing {} from address {:o}\n", nuzzzz, page*1024);
        DiskImage[nuzzzz] = data+page*1024;
    }
    mars.trace_stores = stores;
}

// Takes the zone number as the argument,
// returns the pointer to the zone read in m16 and curbuf
void MarsImpl::get_zone(uint64_t arg) {
    curZone = arg;
    zoneKey = arg | DBkey.d;
    m16 = curbuf = (arg & 01777) ? bdbuf : bdtab;
    if (m16[0] == zoneKey)
        return;
    if (curZone.d && (dirty.d & 2)) {
        dirty = dirty.d & ~2;
        IOword = (IOpat.d | bdbuf.d << 20) + (m16[0].d & 01777);
        IOcall(IOword);
    }
    IOword = (curbuf.d << 20 | curZone.d | ONEBIT(40)) + IOpat.d;
    IOcall(IOword);
    // m16 = curbuf; // not needed; emulated IOcall does not corrupt m16 as opposed to the original
    if (m16[0] == zoneKey)
        return;
    std::cerr << std::format("Corruption: zoneKey = {:o}, data = {:o}\n", zoneKey.d, (*m16).d);
    throw Mars::ERR_BAD_PAGE;
}

void MarsImpl::save(bool force_tab = false) {
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

// The argument is nonzero (a part of the error message) if there is an error
void MarsImpl::finalize(word err) {
    d00011 = err;
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

// Sets 'extentHeader'
void MarsImpl::date() {
    using namespace std::chrono;
    const year_month_day ymd{floor<days>(system_clock::now())};
    unsigned d{ymd.day()}, m{ymd.month()};
    int y{ymd.year()};

    uint64_t stamp = mars.zero_date ? 0 : (d/10*16+d%10) << 9 |
        (m/10*16+m%10) << 4 |
        y % 10;
    work = loc14.d & (0777777LL << 15);
    extentHeader = work.d | mylen.d | (stamp << 33);
}

// Prepare a metadata block
void MarsImpl::make_metablock() {
    d00012 = loc20;
    mylen = 041;
    d00010 = 2;
    usrloc = &d00010;           // to match traced stores with the original
    data[FAKEBLK].d = 2;
    data[FAKEBLK+1].d = d00011.d; // appears to be always 0
    data[FAKEBLK+2].d = loc20.d;
    m16 = usrloc.d = FAKEBLK;
}

// Returns result in itmlen and acc
uint64_t MarsImpl::usable_space() {
    itmlen = 0;
    for (int i = dblen.d - 1; i >= 0; --i) {
        if (freeSpace[i].d != 0)
            itmlen = itmlen.d + freeSpace[i].d - 1;
    }
    m16 = 0;
    return itmlen.d;
}

// Sets 'aitem', returns the extent length in 'curExtLength' and acc
void MarsImpl::find_item(uint64_t arg) {
    d00040 = arg;               // To pacify test-stores, not really needed anymore
    get_zone(arg & 01777);
    // now m16 points to the current page
    work = arg & 03776000;
    if (!work.d)
        throw Mars::ERR_ZEROKEY;      // Attempting to find a placeholder?
    work2 = work.d << 29;
    acc = m16[1] & 03776000;
    if (!acc)
        throw Mars::ERR_NO_RECORD;    // Attempting to find a deleted record?
    if (acc >= work.d)
        acc = work.d;
    loc116 = acc >> 10;
    for (;;) {
        curExtent = m16[loc116+1];
        if (get_id(curExtent) == get_id(work2)) {
            break;
        }
        if (--loc116.d == 0) {
            throw Mars::ERR_NO_RECORD;
        }
    }
    aitem = curbuf.d + (curExtent.d & 01777) + 1;
    curExtLength = acc = get_extlength(curExtent);
    m5 = ARBITRARY_NONZERO;     // clobbering the link register
}

void MarsImpl::info(uint64_t arg) {
    find_item(arg);
    desc1 = *aitem;
    itmlen = desc1 & 077777;
    curlen = 0;
}

void MarsImpl::totext() {
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

void MarsImpl::set_header(word arg) {
    *aitem = arg;
    setDirty(1);
}

void MarsImpl::copy_words(word dst, word src, int len) {
    if (verbose)
        std::cerr << std::format("{:o}(8) words from {:o} to {:o}\n", len, src.d, dst.d);
    // Using backwards store order to match the original binary for ease of debugging.
    while (len) {
        dst[len-1] = src[len-1];
        --len;
    }
    // originally m16 is 0 at return
}

// Copies chained extents to user memory (acc = length of the first extent)
void MarsImpl::copy_chained(int len) { // a01423
    for (;;) {
        if (len) {
            if (verbose)
                std::cerr << "From DB: ";
            copy_words(usrloc, aitem, len);
        }
        usrloc = usrloc + curExtLength;
        if (!next_extent(curExtent))
            return;
        find_item(next_extent(curExtent));
        len = curExtLength.d;
    }
}

// After cpyout must go to cmd0
void MarsImpl::cpyout(uint64_t descr) {
    info(descr);
    usrloc = myloc;
    if (mylen.d && mylen.d < itmlen.d)
        throw Mars::ERR_TOO_LONG;
    // Skip the first word ot the found item
    ++aitem;
    --curExtLength;
    copy_chained(curExtLength.d);
}

// Not really a mutex
void MarsImpl::lock() {
    acc = bdtab.d << 20;
    work = acc |= IOpat.d;
    IOword = acc |= ONEBIT(40);
    IOcall(IOword);
    bdtab[1] = acc = bdtab[1].d ^ LOCKKEY;
    if (!(acc & LOCKKEY))
        throw Mars::ERR_LOCKED;
    bdbuf[0] = acc;
    IOcall(work);
}

void MarsImpl::a00203(uint64_t arg) {
    Array[idx-1] = arg;
    loc116 = arg & BITS(19);
    usrloc = m16;
    element = m16 + 2;
    if (loc116 == loc246)
        return;
    loc246 = loc116;
    find_item(loc246.d);
    copy_chained(curExtLength.d);
}

void MarsImpl::a00213() {
    m16 = &loc54;
    a00203(02000);
}

void MarsImpl::pr202(uint64_t arg) {
    m16 = &loc116;
    a00203(arg);
}

void MarsImpl::skip(Error e) {
    acc = curcmd.d >> 6;
    if (!acc || (acc & 077))
        throw e;
    acc = curcmd.d >> 30;
}

void MarsImpl::setctl(uint64_t location) {
    IOpat = location & 0777777;
    dblen = dbdesc >> 18;
    IOword = bdtab.d << 20 | IOpat.d | ONEBIT(40);
    IOcall(IOword);
    m16 = curbuf = bdtab;
    acc = (*m16).d;
    if (acc != DBkey.d)
        throw Mars::ERR_BAD_CATALOG;
    idx = 0;
    loc246 = 0;
    curZone = 0;
    acc = m16[3].d & 01777;
    acc += bdtab.d + 2;
    freeSpace = acc;
    a00213();
}

// Frees curExtent
void MarsImpl::free_extent() {
    d00031 = curExtent.d & 01777;
    do {
        --m5;
        if (!m5.d)
            break;
        if ((m16[m5+1].d & 01777) >= d00031.d)
            continue;
        m16[m5+1] = m16[m5+1].d + curExtLength.d;
    } while (true);
    work = m16[1].d & 01777;
    if (work != d00031) {
        if (d00031.d < work.d)
            throw Mars::ERR_INTERNAL;
        m5 = (d00031.d - work.d) & 077777;
        work = work.d + curbuf.d;
        d00031 = work.d + curExtLength.d;
        do {
            d00031[m5] = work[m5];
        } while(--m5.d);
    }
    m16[1] = m16[1].d - 02000 + curExtLength.d;
    freeSpace[curZone] = freeSpace[curZone].d + curExtLength.d + 1;
    setDirty(1);
    acc = curExtent.d;
}

void MarsImpl::free(uint64_t arg) {
    acc = arg;
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
        acc = next_extent(curExtent);
    } while (acc);
    dirty = dirty.d | 1;
}

void MarsImpl::free_from_current_extent() {
    free_extent();
    if (next_extent(curExtent)) {
        free(next_extent(curExtent));
    } else {
        dirty = dirty.d | 1;
    }
}

void MarsImpl::find_end_mark() {
    // Searching for the end mark (originally 1 or 2 bytes)
    m16 = -mylen.d;
    work2 = myloc.d + mylen.d;
    // Handling only single byte mark, contemporary style
    char * start = (char*)&(work2[m16].d);
    char * end = (char*)&(work2[0].d);
    char * found = (char*)memchr(start, endmrk.d & 0xFF, end-start);
    if (!found)
        throw Mars::ERR_NO_END_MARK;
    mylen = (found - start) / 8 + 1;
}

bool MarsImpl::proc270() {
    acc = temp.d;
    m16 = acc;
    acc = (acc - curExtLength.d) & BITS(41);
    if (!(acc & ONEBIT(41))) {
        temp = acc;
        m16 = curExtLength;
        m5 = 0;
    }
    switch (jmpoff.d) {
    case FROMBASE:
        copy_words(usrloc, aitem, m16.d);
        break;
    case TOBASE:
        setDirty(1);
        copy_words(aitem, usrloc, m16.d);
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
    usrloc = usrloc.d + curExtLength.d;
    return false;
}

bool MarsImpl::step() {
    acc = Array[idx].d;
    if (!acc)
        throw Mars::ERR_NO_CURR;
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
                skip(Mars::ERR_NO_NEXT);
                return true;
            }
            pr202(acc >> 10);
            Array[idx-2] = Array[idx-2].d + (1<<15);
            m5 = 0;
        }
    } else {
        // Step back
        if (!m5.d) {
            acc = element[-1] >> 29;
            if (!acc) {
                skip(Mars::ERR_NO_PREV);
                return true;
            }
            pr202(acc);
            acc = Array[idx-2].d;
            acc -= 1<<15;
            Array[idx-2] = acc;
            acc = loc117.d & 01777;
            m5 = acc;
        }
        m5 = m5 - 2;
    }
    acc = m5.d | 1 << 30;
    Array[idx] = acc;
    curkey = element[m5];
    adescr = element[m5+1];
    return false;
}

bool MarsImpl::a00334(word arg) {
    temp = arg;
    curlen = curlen.d + arg.d;
    usrloc = myloc;
  a00270:
    if (proc270()) {
        // COMPARE failed, skip an instruction
        return true;
    }
    if (m5.d) {
        aitem = aitem.d + temp.d;
        curExtLength = curExtLength.d - temp.d;
        acc = (*aitem).d;
        desc1 = acc;
    } else {
        acc = next_extent(curExtent);
        if (acc) {
            find_item(acc);
            jump(a00270);
        }
        if (jmpoff.d == DONE
            // impossible? should compare with A00317, or deliberate as a binary patch?
            || temp.d) {
            skip(Mars::ERR_STEP);
            return true;
        }
    }
    return false;
}

void MarsImpl::a00340(uint64_t arg) {
    find_item(arg);
    jmpoff = A00317;
    a00334(desc2);
}

void MarsImpl::assign_and_incr() {
    m16 = acc;
    m5 = (curcmd >> 6) & 077;
    curcmd = curcmd >> 6;
    abdv[m16] = (*abdv[m5]).d;
    ++abdv[m5];
}

// Performs operations of upper bits in myloc
// using value in curlen, assigning to desc2 and mylen.
// Conditionally skips the next 4 instructions.
// From the code is appears that the max expected
// length of the datum was 017777.
bool MarsImpl::cmd46() {
    acc = (myloc >> 18) & BITS(15);
    desc2 = acc -= curlen.d;
    acc = (myloc >> 33) & 017777;
    if (!acc) {
        acc = curcmd.d >> 30;
        return true;
    }
    mylen = acc;
    return false;
}

void MarsImpl::overflow() {
    acc = next_extent(d00030);
    if (acc) {
        free(acc);
        throw Mars::ERR_OVERFLOW;
    }
    dirty = dirty.d | 1;
    throw Mars::ERR_OVERFLOW;
}

void MarsImpl::prepare_chunk() {
    acc = freeSpace[m5-1].d - 1;
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
}

// The 3 entry points are disambiguated by the original code addresses
// for lack of a better understanding of their semantics.
void MarsImpl::allocator(int addr) {
    switch (addr) {
    case 01022:
        date();
        // FALL THROUGH
    case 01023:
        m5 = 0;
        d00030 = 0;
        m16 = freeSpace;
        work = freeSpace[curZone];
        ++mylen;
        if (mylen.d >= work.d) {
            if (verbose)
                std::cerr << "mylen = " << mylen.d << " work = " << work.d << '\n';
            // If the datum is larger than MAXCHUNK, it will have to be split
            if (mylen.d >= Mars::MAXCHUNK)
                jump(split);
            // Find a zone with enough free space
            for (size_t i = 0; i < dblen.d; ++i) {
                if (mylen.d < freeSpace[i].d) {
                    acc = i;
                    jump(chunk);
                }
            }
            // End reached, must split
          split:
            usrloc = usrloc.d + mylen.d - 1;
            m5 = dblen;
            while (freeSpace[m5-1].d < 2) {
                if (--m5.d)
                    continue;
                overflow();
            }
            prepare_chunk();
            jump(chunk);
        }
        acc = curZone.d;
    chunk:
        get_zone(acc);
        m7 = m16;
        m16 = m7[1] >> 10;
        do {
            if (m16 == get_id(curbuf[m16+1]))
                break;
            curbuf[m16+2] = curbuf[m16+1];
            --m16;
        } while (m16.d);
        ++m16;
        loc116 = acc = m16.d;
        acc <<= 39;
        acc &= BITS(48);
        acc |= d00030.d;
    case 01047:                 // acc has the extent id in bits 48-40
        d00030 = acc;
        d00024 = (get_id(acc) << 10) | curZone.d;
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
            copy_words(work2, usrloc, m16.d);
        }
        m7[loc116+1] = (mylen.d << 10) | work.d | d00030.d;
        m16 = freeSpace;
        if (verbose)
            std::cerr << "Reducing free " << std::oct << freeSpace[curZone].d
                      << " by len " << mylen.d << " + 1\n";
        freeSpace[curZone] = freeSpace[curZone].d - (mylen.d+1);
        if (verbose)
            std::cerr << "Got " << freeSpace[curZone].d << '\n';
        if (!m5.d) {
            work2[-1] = extentHeader;
            setDirty(2);
            return;
        }
        setDirty(1);
        acc = d00024.d;
        acc <<= 20;
        acc &= BITS(48);
        d00030 = acc;
        mylen = remlen;
        if (--m5.d) {
            while (freeSpace[m5-1].d < 2) {
                if (--m5.d)
                    continue;
                overflow();
            }
            prepare_chunk();
            jump(chunk);
        }
        overflow();             // will throw
    }
}

void MarsImpl::mkctl() {
    curZone = 0;
    m5 = dblen = dbdesc >> 18;
    IOpat = dbdesc.d & 0777777;
    curbuf = bdtab;
    work2 = IOpat.d | bdtab.d << 20;
    bdtab[1] = 01777;
    do {
        --m5;
        bdbuf[m5] = 01776;
        bdtab[0] = DBkey.d | m5.d;
        IOword = work2.d + m5.d;
        IOcall(IOword);
    } while (m5.d);
    myloc = freeSpace = bdbuf; // freeSpace[0] is now the same as bdbuf[0]
    loc20 = 02000;
    d00011 = 0;
    make_metablock();
    allocator(01022);
    mylen = dblen.d;
    // This trick results in max possible DB length = 753 zones.
    bdbuf[0] = 01731 - mylen.d;
}

void MarsImpl::a01346() {
    d00026 = curExtent;
    curExtent = curExtent.d & 01777;
    free_from_current_extent();
    ++mylen;
    m5 = 0;
    m7 = curbuf;
    acc = d00026.d & (0777LL << 39);
    allocator(01047);
    acc = next_extent(d00026);
    if (acc) {
        free(acc);
    } else {
        dirty = dirty.d | 1;
    }
}

void MarsImpl::find_end_word() {
  cmd17:
    m16 = -mylen.d;
    work2 = myloc + mylen;
    // loop_here:
    if (work2[m16] == endmrk) {
        m16 = m16 + mylen + 1;
        mylen = m16;
        return;
    }
    if (m16 != 0) {
        ++m16;
        jump(cmd17);            // Possibly a typo/oversight, ought to be loop_here
    }
    throw Mars::ERR_INTERNAL;         // Originally "no end word"
}

void MarsImpl::a00164() {
    acc = m16[-1].d & 01777;
    m5 = acc;
    if (verbose)
        std::cerr << "Comparing " << std::dec << acc/2 << " elements\n";
    while (m5.d) {
        acc = m16[m5-2].d + temp.d;
        acc = (acc + (acc >> 48)) & BITS(48);
        if (!(acc & ONEBIT(48)))
            break;
        m5.d -= 2;
    }
    m5.d -= 2;
    Array[idx] = m5.d | ONEBIT(31);
    acc = m16[1].d & ONEBIT(48);
    if (!acc) {
        curkey = element[m5];
        adescr = element[m5+1];
        return;
    }
    idx = idx + 2;
    if (idx > 7) {
        std::cerr << "Idx = " << idx << ": DB will be corrupted\n";
    }
    pr202(m16[m5+1].d);
    m16 = Loc120;
    a00164();
}

void MarsImpl::find(word k) {
    temp = k;
    idx = 0;
    temp = temp.d ^ BITS(47);
    while (true) {
        m16 = Metadata;
        if (loc20.d)
            break;
        if (verbose)
            std::cerr << "Loc20 == 0, nothing to compare\n";
        a00213();
    }
    a00164();
}

void MarsImpl::a01311(word arg) {
    d00012 = arg;
    find_item(d00012.d);
    --acc;
    m5 = acc;
    if (acc == mylen.d) {
        // The new length of data matches the first extent length
        if (m5.d) {
            do {
                aitem[m5] = usrloc[m5-1].d;
            } while (--m5.d);
        }
        m5 = m16;
        date();
        *aitem = extentHeader;
        setDirty(1);
        if (!next_extent(curExtent))
            return;       // No extents to free: done
        m5[loc116+1] = curExtent & ~(BITS(19) << 20); // dropping the extent chain
        acc = next_extent(curExtent);
        free(acc);          // Free remaining extents
        dirty = dirty.d | 1;
        return;
    }
    date();
    m16 = curbuf;
    acc = m16[1] >> 10;
    m5 = acc + 1;
    acc = (m16[1].d - acc) & 01777;
    acc += curExtLength.d - 2;
    work = acc;
    if (acc >= mylen.d) {
        a01346();
        dirty = dirty.d | 1;
        return;
    }
    usable_space();
    acc = (*aitem).d & 077777;
    acc += itmlen.d;
    if (acc < mylen.d) {
        throw Mars::ERR_OVERFLOW;
    }
    m16 = curbuf;
    mylen = work.d;
    a01346();
    usrloc = usrloc.d + mylen.d;
    mylen = (extentHeader.d & 077777) - mylen.d;
    extentHeader = usrloc[-1];
    allocator(01023);
    find_item(d00012.d);
    // This is likely chaining the extents
    acc = (d00024.d << 20) & BITS(48);
    m16[loc116+1] = acc | curExtent.d;
    setDirty(2);
    return;
}

Error MarsImpl::eval() try {
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
    case 1:                     // FIRST
        find(0);
        break;
    case 2:                     // LAST
        find(BITS(47));
        break;
    case 3:                     // PREV
        m16 = 1;
        if (step())
            jump(next);
        break;
    case 4:                     // NEXT
        m16 = 0;
        if (step())
            jump(next);
        break;
    case 5:
        d00011 = 0;
        make_metablock();
        usrloc = acc;
        allocator(01022);
        curDescr = d00024;
        break;
    case 6:
        idx = 0;
        itmlen = 041;
        m16 = &loc54;
        a00203(adescr.d);
        break;
    case 7:
        jmpoff = A00317;
        a00334(desc2);
        break;
    case 010:
        mkctl();
        // Invoking command 21 for the freeSpace array
        usrloc = myloc;
        allocator(01022);
        curDescr = d00024;
        break;
    case 011:
        if (key == 0 || key.d & ONEBIT(48))
            throw Mars::ERR_INV_NAME;
        find(key);
        break;
    case 012:
        myloc = &dbdesc;
        mylen = 3;
        cpyout(adescr.d);
        break;
    case 013:
        usable_space();
        break;
    case 014:
        if (curkey == key)
            break;
        skip(Mars::ERR_NO_NAME);
        jump(next);
    case 015:
        if (curkey != key)
            break;
        skip(Mars::ERR_EXISTS);
        jump(next);
    case 016:
        find_end_mark();
        break;
    case 017:
        find_end_word();
        break;
    case 020:
        usrloc = myloc.d;
        a01311(adescr);
        break;
    case 021:
        usrloc = myloc;
        allocator(01022);
        curDescr = d00024;
        break;
    case 022:
        cpyout(adescr.d);
        break;
    case 023:
        free(adescr.d);
        break;
    case 024:
        if (givenp != savedp)
            throw Mars::ERR_WRONG_PASSWORD;
        break;
    case 025:
        setctl(dbdesc.d);
        break;
    case 026:
        d00011 = key;
        adescr = acc = curDescr.d;
        jump(a01140);
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
        *aitem = curDescr;
        setDirty(2);
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
        if (mars.dump_diffs)
            mars.dump();
        return Mars::ERR_SUCCESS;
    case 036:
        m16 = element;
        m16[Array[idx]+1] = curDescr;
        jump(a01160);
    case 037:
        acc = curcmd >> 12;
        jump(next);
    case 040:
        jump(next);
    case 041:
        jmpoff = COMPARE;
        if (a00334(mylen))
            jump(next);
        break;
    case 042:
        jmpoff = TOBASE;
        if (a00334(mylen))
            jump(next);
        break;
    case 043:
        jmpoff = FROMBASE;
        if(a00334(mylen))
            jump(next);
        break;
    case 044:
        adescr = desc1;
        break;
    case 045:
        lock();
        break;
    case 046:
        if (cmd46())
            jump(next);
        break;
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
        abdv[m16] = abdv[m5];
        break;
    case 054:
        // mem[bdvect[dst]] = bdvect[012]
        // curcmd is ..... dst 54
        m16 = (curcmd >> 6) & 077;
        curcmd = curcmd >> 6;
        *abdv[m16] = curDescr;
        break;
    case 055:
        if (mars.dump_diffs)
            mars.dump();
        return Mars::ERR_SUCCESS;
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
        finalize(0);
        if (mars.dump_diffs)
            mars.dump();
        return Mars::ERR_SUCCESS;
    }
    curcmd = acc;
    m5 = acc & BITS(6);
    m6 = target(cmd0);
    goto_ = 0;
    jump(switch_);
  delkey:
    acc = element.d;
    m16 = acc;
    acc += Array[idx].d;
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
    if (!acc) {
        acc = idx;
        if (!acc)
            jump(a01160);
        free(loc246.d);
        desc2 = 1;
        acc = loc117.d;
        d00025 = acc;
        acc &= BITS(19) << 29;
        d00032 = acc;
        acc >>= 29;
        if (acc) {
            a00340(acc);
            acc &= ~(BITS(19) << 10);
            loc117 = acc;
            set_header((d00025 & (BITS(19) << 10)) | loc117);
        }
        call(pr1232,m5);
        jump(delkey);
    }
    acc = loc116.d;
    while (true) {
        acc &= 077777;
        if (!acc) {
            d00011 = m16[0];
            goto_ = target(a01242);
            d00033 = target(a00731);
        }
        jump(a01160);
      a00731:
        acc = d00011.d;
        m16 = element;
        m16[Array[idx]] = acc;
        acc = Array[idx].d;
    }
  a01136:
    acc = d00032.d;
    acc >>= 29;
    acc |= ONEBIT(48);
  a01140:
    d00024 = acc;
    m16 = element;
    m5 = element + Array[idx] + 2;
    limit = m5;
    m6 = (m16[-1] & 01777) + m16;
    if (verbose)
        std::cerr << std::format("Expanding {}  elements\n", (m6.d-limit.d)/2);
    while (m6 != limit) {
        m6[0] = m6[-2];
        m6[1] = m6[-1];
        m6 = m6 - 2;
    }
    m5[0] = d00011;
    m5[1] = d00024;
    m16[-1] = m16[-1] + 2;
  a01160:
    acc = m16.d;
    usrloc = --acc;
    acc = idx;
    if (!acc) {
        mylen = 041;
        if (m16[-1] == 040) {
            if (usable_space() < 0101) {
                loc20 = 0;
                free(curDescr.d);
                throw Mars::ERR_OVERFLOW;
            }
            acc -= 0101;
            allocator(01022);
            Metadata[1] = d00024.d | ONEBIT(48);
            Metadata[-1] = 2;
            mylen = 041;
            a01311(loc20);
            jump(rtnext);
        }
        a01311(loc20);
        jump(rtnext);
    }
    acc = m16[-1].d & 01777;
    if (acc != 0100) {
        acc = m16[-1].d & 01777;
        mylen = ++acc;
        a01311(loc246);
        jump(rtnext);
    }
    work = (idx * 2) + 041;
    if (Metadata[-1].d == 036) {
        // The current metadata block is full, account for another one
        work = work + 044;
    }
    if (usable_space() < work.d) {
        loc20 = 0;
        free(curDescr.d);
        throw Mars::ERR_OVERFLOW;
    }
    d00025 = loc157;
    loc157 = (loc246.d << 29) & BITS(48);
    acc = loc117.d;
    acc &= BITS(19) << 10;
    acc |= loc157.d | 040;
    loc157 = acc;
    usrloc.d += 040;
    mylen = 041;
    allocator(01022);
    loc157 = d00025;
    d00011 = loc160;
    acc = (d00025 = loc117).d;
    acc = (acc >> 29 << 19) | d00024.d;
    acc <<= 10;
    acc &= BITS(48);
    acc |= 040;
    loc117 = acc;
    acc &= BITS(19) << 10;
    d00032 = (acc << 19) & BITS(48);
    m16 = Loc120;
    usrloc = Loc120 - 1;
    acc = Loc120[-1].d & 01777;
    mylen = ++acc;
    a01311(loc246);
    call(pr1232,m5);
    jump(a01136);
    // pr1232 can return in 3 ways:
    // - back to the immediate caller
    // - terminating execution of the instruction (next)
    // - potential termination of the execution
    //   returning to the upper level caller if goto_ is not 0 (rtnext)
  pr1232:
    d00033 = m5;
    desc2 = 1;
    acc = (d00025 >> 10) & BITS(19);
    if (acc) {
        a00340(acc);
        set_header(d00032 | (acc & BITS(29)));
    }
  a01242:
    element = Metadata;
    if (idx == 0) {
        if (goto_.d)
            std::cerr << "Returning from pr1232 with non-0 goto_\n";
        dirty = dirty.d | 1;
        jump(rtnext);
    }
    idx = idx - 2;
    if (idx != 0) {
        pr202(Array[idx-1].d);
    }
    acc = Array[idx].d;
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
    if (usable_space() < 0101) {
        loc20 = 0;
        free(curDescr.d);
        throw Mars::ERR_OVERFLOW;
    }
    acc -= 0101;
    allocator(01022);
    Metadata[1] = d00024 | ONEBIT(48);
    Metadata[-1] = 2;
    mylen = 041;
    a01311(loc20);
    jump(rtnext);
} catch (Error e) {
    if (erhndl.d)
        savm16 = erhndl;
    m7 = e;
    std::cerr << std::format("ERROR {} ({})\n", int(e), msg[e-1]);
    d00010 = *(uint64_t*)msg[e-1];
    finalize(*((uint64_t*)msg[e-1]+1));
    if (mars.dump_diffs)
        mars.dump();
    return e;
}

void MarsImpl::pasbdi() {
    for (size_t i = 0; i < Mars::RAM_LENGTH; ++i) {
        new (data+i) word(mars, 0);
    }
    // Faking MARS words
    m16 = 04700;
    m13 = 01654;
    m7 = 01603;
    m5 = 077770;
    m6 = 022731;
    bdtab = Mars::BDTAB;
    bdbuf = Mars::BDBUF;
    abdv = BDVECT;
    for (int i = 0; i < 041; ++i)
        data[FAKEBLK+i].d = ARBITRARY_NONZERO;
}

static int to_lnuzzzz(int lun, int start, int len) {
    if (lun > 077 || start > 01777 || len > 0777)
        std::cerr << std::oct << lun << ' ' << start << ' ' << len
                  << "out of valid range, truncated\n";
    return ((lun & 077) << 12) | (start & 01777) | ((len & 0777) << 18);
}

Error Mars::SetDB(int lun, int start, int len) {
    impl.pasbdi();
    impl.arch = to_lnuzzzz(lun, start, len);
    return root();
}

Error Mars::InitDB(int lun, int start, int len) {
    impl.pasbdi();
    impl.dbdesc = to_lnuzzzz(lun, start, len);
    impl.DBkey = ROOTKEY;
    impl.orgcmd = 010;
    return impl.eval();
}

// Replicating what the NEWD option does in the PAIB Pascal API function
// to match the sequence of store ops as close as possible
Error Mars::newd(const char * k, int lun, int start, int len) {
    int lnuzzzz = to_lnuzzzz(lun, start, len);
    if (verbose)
        std::cerr << std::format("Running newd('{}', {:o})\n", k, lnuzzzz);
    impl.key = *reinterpret_cast<const uint64_t*>(k);
    data[077776] = lnuzzzz;
    data[077777] = (impl.key.d << 10) & BITS(48);
    impl.key = *reinterpret_cast<const uint64_t*>(k);
    impl.mylen = 1;
    impl.myloc = ARBITRARY_NONZERO;
    impl.orgcmd = 0;
    impl.myloc = 077776;
    impl.mylen = 2;
    impl.orgcmd = 02621151131LL; // ROOT FIND NOMATCH PUT ADDKEY
    impl.eval();
    impl.orgcmd = 010121411;    // FIND MATCH SETCTL MKCTL
    return impl.eval();
}

Error Mars::opend(const char * k) {
    if (verbose)
        std::cerr << "Running opend('" << k << "')\n";
    impl.key = *reinterpret_cast<const uint64_t*>(k);
    impl.orgcmd = 02512141131;  // ROOT FIND MATCH SETCTL OPEN
    return impl.eval();
}

Error Mars::putd(const char * k, int loc, int len) {
    if (verbose)
        std::cerr << std::format("Running putd('{}', '{}':{})\n", k,
                                 reinterpret_cast<char*>(data+loc), len);
    impl.key = *reinterpret_cast<const uint64_t*>(k);
    impl.mylen = len;
    impl.myloc = loc;
    impl.orgcmd = 026211511;
    return impl.eval();
}

Error Mars::putd(uint64_t k, int loc, int len) {
    if (verbose) {
        std::cerr << std::format("Running putd({:016o}, {:05o}:{})\n", k, loc, len);
    }
    impl.key = k;
    impl.mylen = len;
    impl.myloc = loc;
    impl.orgcmd = 026211511;
    return impl.eval();
}

Error Mars::putd(const char * k, const char * v) {
    size_t len = (strlen(v)+7)/8;
    if (verbose)
        std::cerr << std::format("Running putd('{}', '{}':{})\n", k, v, len);
    impl.key = *reinterpret_cast<const uint64_t*>(k);
    strcpy((char*)(data+RAM_LENGTH-len), v);
    impl.mylen = len;
    impl.myloc = RAM_LENGTH-len;
    impl.orgcmd = 026211511;
    return impl.eval();
}

Error Mars::modd(const char * k, int loc, int len) {
    impl.key = *reinterpret_cast<const uint64_t*>(k);
    impl.mylen = len;
    impl.myloc = loc;
    impl.orgcmd = 020402621001511; // FIND NOMATCH ? (PUT ADDKEY DONE) : UPDATE
    return impl.eval();
}

Error Mars::modd(uint64_t k, int loc, int len) {
    impl.key = k;
    impl.mylen = len;
    impl.myloc = loc;
    impl.orgcmd = 020402621001511;
    return impl.eval();
}

Error Mars::getd(const char * k, int loc, int len) {
    impl.key = *reinterpret_cast<const uint64_t*>(k);
    impl.mylen = len;
    impl.myloc = loc;
    impl.orgcmd = 0221411;      // FIND MATCH GET
    return impl.eval();
}

Error Mars::getd(uint64_t k, int loc, int len) {
    if (verbose) {
        std::cerr << std::format("Running getd({:016o}, {:05o}:{})\n", k, loc, len);
    }
    impl.key = k;
    impl.mylen = len;
    impl.myloc = loc;
    impl.orgcmd = 0221411;
    return impl.eval();
}

Error Mars::deld(const char * k) {
    impl.key = *reinterpret_cast<const uint64_t*>(k);
    impl.orgcmd = 027231411;    // FIND MATCH FREE DELKEY
    return impl.eval();
}

Error Mars::deld(uint64_t k) {
    impl.idx = 0;
    impl.key = k;
    impl.orgcmd = 027231411;
    return impl.eval();
}

Error Mars::root() {
    impl.orgcmd = 031;
    return impl.eval();
}

uint64_t Mars::first() {
    impl.orgcmd = 0401;         // BEGIN NEXT
    impl.eval();
    return impl.curkey.d;
}

uint64_t Mars::last() {
    impl.orgcmd = 02;
    impl.eval();
    return impl.curkey.d;
}

uint64_t Mars::prev() {
    impl.orgcmd = 03;
    impl.eval();
    return impl.curkey.d;
}

uint64_t Mars::next() {
    impl.orgcmd = 04;
    impl.eval();
    return impl.curkey.d;
}

uint64_t Mars::find(const char * k) {
    impl.key = *reinterpret_cast<const uint64_t*>(k);
    impl.orgcmd = 011;
    impl.eval();
    return impl.curkey.d;
}

uint64_t Mars::find(uint64_t k) {
    impl.key = k;
    impl.orgcmd = 011;
    impl.eval();
    return impl.curkey.d;
}

int Mars::getlen() {
    impl.orgcmd = 033;
    if (impl.eval())
        return -1;
    return impl.itmlen.d;
}

Error Mars::cleard(bool forward) {
    impl.key = 0;
    if (forward)
        impl.orgcmd = 0302723000401; // BEGIN NEXT ? (FREE DELKEY LOOP) : empty
    else
        impl.orgcmd = 0302723001502; // LAST NOMATCH ? FREE DELKEY LOOP : empty
    return impl.eval();
}

int Mars::avail() {
    impl.orgcmd = 013;
    impl.itmlen = 0;
    impl.eval();
    return impl.itmlen.d;
}

Error Mars::eval(uint64_t microcode) {
    impl.orgcmd = microcode;
    return impl.eval();
}
