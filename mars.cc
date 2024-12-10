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

#define FAKEBLK 020
#define ARBITRARY_NONZERO 01234567007654321LL
#define ROOTKEY 04000
#define ROOT_METABLOCK 02000    // ID 1 in zone 0
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
        metaflag, dbdesc, DBkey, savedp, freeSpace, adescr, limit,
        curkey, endmrk, desc1, desc2, IOpat, curExtLength, aitem, datumLen,
        curMetaBlock, dblen, curlen, loc116,
        curExtent, curbuf, curZone, curBlockDescr,
        dirty;

    uint64_t & idx;
// Metadata[-1] (loc54) is also used
    word * const Array;
    word * const Metadata;
    word * const Secondary;

    // Fields of BDSYS
    uint64_t & arch;
    wordref abdv, d00010, d00011, d00012,
        goto_, zoneKey, usrloc,
        jmpoff, extentHeader, newDescr, d00025, d00026,
        temp, chainHead, d00031, d00032, d00033, remlen,
        work, work2, IOword, d00040;

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
        metaflag(data[BDVECT+020]), // Can only be 0 or 02000 ???
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
        datumLen(data[BDVECT+047]),
        curMetaBlock(data[BDVECT+050]),
        dblen(data[BDVECT+051]),
        curlen(data[BDVECT+053]),
        loc116(data[BDVECT+0116]),
        curExtent(data[BDVECT+0220]),
        curbuf(data[BDVECT+0241]),
        curZone(data[BDVECT+0244]),
        curBlockDescr(data[BDVECT+0246]),
        dirty(data[BDVECT+0247]),

        idx(data[BDVECT+0242].d),
        Array(data+BDVECT+021),
        // Metadata[-1] (loc54) is also used
        Metadata(data+BDVECT+055), // spans 041 words, up to 0115
        // Secondary[-1] loc116 is also used
        Secondary(data+BDVECT+0117), // spans 0101 words, up to 0217

        // Fields of BDSYS
        arch(data[BDSYS+4].d),
        abdv(data[BDSYS+3]),
        d00010(data[BDSYS+010]),
        d00011(data[BDSYS+011]),
        d00012(data[BDSYS+012]),
        goto_(data[BDSYS+014]),
        zoneKey(data[BDSYS+015]),
        usrloc(data[BDSYS+016]),
        jmpoff(data[BDSYS+022]),
        extentHeader(data[BDSYS+023]),
        newDescr(data[BDSYS+024]),
        d00025(data[BDSYS+025]),
        d00026(data[BDSYS+026]),
        temp(data[BDSYS+027]),
        chainHead(data[BDSYS+030]),
        d00031(data[BDSYS+031]),
        d00032(data[BDSYS+032]),
        d00033(data[BDSYS+033]),
        remlen(data[BDSYS+034]),
        work(data[BDSYS+035]),
        work2(data[BDSYS+036]),
        IOword(data[BDSYS+037]),
        d00040(data[BDSYS+040])
        { }
    void IOflush();
    void IOcall(word);
    void get_zone(uint64_t);
    void save(bool);
    void finalize(word);
    void make_extent_header();
    void make_metablock();
    uint64_t usable_space();
    uint64_t find_item(uint64_t);
    void info(uint64_t);
    void totext();
    void set_header(word);
    void copy_words(word dst, word src, int len);
    void copy_chained(int len);
    void cpyout(uint64_t);
    void lock();
    void get_block(word), get_root_block(), get_secondary_block(word);
    void free_from_current_extent();
    uint64_t a00340(uint64_t);
    void skip(Error);
    void setctl(uint64_t);
    void free_extent(), free(uint64_t);
    void find_end_mark(), find_end_word();
    bool proc270(), step(), a00334(word);
    bool cmd46();
    void assign_and_incr(uint64_t);
    void pasbdi();
    Error eval();
    void overflow(word);
    void prepare_chunk();
    void allocator(int);
    void mkctl(), find(word);
    void update_by_reallocation(), search_in_block(), update(word);
    void setDirty(int x) {
        dirty = dirty.d | (curZone.d ? x+1 : 1);
    }
    void set_dirty_data() {
        // If the current data zone is 0, sets dirty tab,
        // otherwise dirty buf
        setDirty(1);
    }
    void set_dirty_both() {
        // If the current data zone is 0, sets dirty tab,
        // otherwise dirty buf and tab
        setDirty(2);
    }
    void set_dirty_tab() {
        dirty = dirty | 1;
    }
    bool is_dirty_tab() const {
        return dirty.d & 1;
    }
    bool is_dirty_buf() {
        return dirty.d & 2;
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
    store(x.d);
    return *this;
}
word& word::operator=(word * x) {
    assert(mars);
    ptrdiff_t offset = x-mars->data;
    if (offset < 0 || offset >= 077777) {
        std::cerr << std::format("Cannot point to words outside of RAM, offset = {:o}\n", offset);
        abort();
    }
    store(offset);
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

template<class T> uint64_t get_extstart(const T & x) {
        return x.d & BITS(10);
}

template<class T> uint64_t get_id(const T & x) {
        return x.d >> 39;
}

template<class T> uint64_t prev_block(const T & x) {
        return x.d >> 29;
}

template<class T> uint64_t next_block(const T & x) {
    return (x.d >> 10) & BITS(19);
}

template<class T> uint64_t block_len(const T & x) {
        return x.d & BITS(10);
}

inline uint64_t make_block_header(uint64_t prev, uint64_t next, uint64_t len) {
    return prev << 29 | next << 10 | len;
}

union Accumulator {
    static const uint64_t MASK = BITS(48);
    uint64_t d;
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
    if (curZone.d && is_dirty_buf()) {
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
    if (force_tab || is_dirty_tab()) {
        IOword = IOpat.d | bdtab.d << 20;
        IOcall(IOword);
    }
    if (is_dirty_buf()) {
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
void MarsImpl::make_extent_header() {
    using namespace std::chrono;
    const year_month_day ymd{floor<days>(system_clock::now())};
    unsigned d{ymd.day()}, m{ymd.month()};
    int y{ymd.year()};

    uint64_t stamp = mars.zero_date ? 0 : (d/10*16+d%10) << 9 |
        (m/10*16+m%10) << 4 |
        y % 10;
    // This is the only use of loc14
    // and of this bit range of the extent header structure
    work = loc14 & (BITS(18) << 15);
    extentHeader = work | mylen | (stamp << 33);
}

// Prepare a metadata block, set m16 and usrloc to its address
void MarsImpl::make_metablock() {
    d00012 = metaflag;
    mylen = 041;
    d00010 = 2;
    usrloc = &d00010;           // to match traced stores with the original
    data[FAKEBLK].d = 2;
    data[FAKEBLK+1].d = d00011.d; // appears to be always 0
    data[FAKEBLK+2].d = metaflag.d;
    m16 = usrloc.d = FAKEBLK;
}

// Returns result in datumLen and acc
uint64_t MarsImpl::usable_space() {
    datumLen = 0;
    for (int i = dblen.d - 1; i >= 0; --i) {
        if (freeSpace[i] != 0)
            datumLen = datumLen + freeSpace[i] - 1;
    }
    m16 = 0;
    return acc = datumLen.d;
}

// Finds item indicated by the zone number in bits 10-1
// and by the number within the zone in bits 19-11
// Sets 'aitem', also returns the extent length in 'curExtLength'
uint64_t MarsImpl::find_item(uint64_t arg) {
    d00040 = arg;               // To pacify test-stores, not really needed anymore
    get_zone(arg & 01777);
    // now m16 points to the current page
    work = arg & (BITS(10) << 10);
    if (!work.d)
        throw Mars::ERR_ZEROKEY;      // Attempting to find a placeholder?
    work2 = work.d << 29;
    acc = m16[1] & (BITS(10) << 10);
    if (!acc)
        throw Mars::ERR_NO_RECORD;    // Attempting to find a deleted record?
    // Records within the zone may be non-contiguous but sorted;
    // need to search only from the indicated position toward lower addresses.
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
    aitem = curbuf + get_extstart(curExtent) + 1;
    curExtLength =  get_extlength(curExtent);
    m5 = ARBITRARY_NONZERO;     // clobbering the link register
    return curExtLength.d;
}

// Assumes that arg points to a datum with the standard header.
// Puts the full header to desc1, and its length to datumLen.
void MarsImpl::info(uint64_t arg) {
    find_item(arg);
    desc1 = *aitem;
    datumLen = desc1 & 077777;
    curlen = 0;
}

void MarsImpl::totext() {
    uint64_t v = desc1.d;
    endmrk = Mars::tobesm(std::format("\017{:05o}", v & 077777));
    desc1 = Mars::tobesm(std::format("\017{:c}{:c}{:c}{:c}{:c}",
                                     (v >> 46) & 3, (v >> 42) & 15,
                                     (v >> 41) & 1, (v >> 37) & 15,
                                     (v >> 33) & 15));
    desc2 = Mars::tobesm(std::format("\172\033\0\0\0\0"));
}

void MarsImpl::set_header(word arg) {
    *aitem = arg;
    set_dirty_data();
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

// Copies chained extents to user memory (len = length of the first extent)
void MarsImpl::copy_chained(int len) { // a01423
    for (;;) {
        if (len) {
            if (verbose)
                std::cerr << "From DB: ";
            copy_words(usrloc, aitem, len);
        }
        usrloc = usrloc + len;
        if (!next_extent(curExtent))
            return;
        len = find_item(next_extent(curExtent));
    }
}

// After cpyout must go to cmd0
void MarsImpl::cpyout(uint64_t descr) {
    info(descr);
    usrloc = myloc;
    if (mylen.d && mylen.d < datumLen.d)
        throw Mars::ERR_TOO_LONG;
    // Skip the first word ot the found item
    ++aitem;
    --curExtLength;
    copy_chained(curExtLength.d);
}

// Not really a mutex
void MarsImpl::lock() {
    work = (bdtab.d << 20) | IOpat.d;
    IOword = work | ONEBIT(40);
    IOcall(IOword);
    bdtab[1] = bdtab[1].d ^ LOCKKEY;
    if (!(bdtab[1].d & LOCKKEY))
        throw Mars::ERR_LOCKED;
    bdbuf[0] = LOCKKEY;         // What is the purpose of this?
    IOcall(work);
}

// Finds and reads a block of metadata identified by 'descr'
// into memory pointed to by m16, if 'curBlockDescr' does
// not have the same ID already.
void MarsImpl::get_block(word descr) {
    Array[idx-1] = descr;
    loc116 = descr & BITS(19);
    usrloc = m16;
    curMetaBlock = m16 + 2;
    if (loc116 == curBlockDescr)
        return;
    curBlockDescr = loc116;
    copy_chained(find_item(curBlockDescr.d));
}

// Requests the root block into the main Metadata array
void MarsImpl::get_root_block() {
    m16 = Metadata-1;
    get_block(ROOT_METABLOCK);
}

// Requests a block into the Secondary metadata array
void MarsImpl::get_secondary_block(word descr) {
    m16 = Secondary-1;
    get_block(descr);
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
    if (bdtab[0] != DBkey)
        throw Mars::ERR_BAD_CATALOG;
    idx = 0;
    curBlockDescr = 0;
    curZone = 0;
    freeSpace = bdtab + (bdtab[3] & 01777) + 2;
    get_root_block();
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
        m16[m5+1] = m16[m5+1] + curExtLength;
    } while (true);
    work = m16[1] & 01777;
    if (work != d00031) {
        if (d00031.d < work.d)
            throw Mars::ERR_INTERNAL;
        m5 = (d00031 - work) & 077777;
        work = work + curbuf;
        d00031 = work + curExtLength;
        do {
            d00031[m5] = work[m5];
        } while(--m5.d);
    }
    m16[1] = m16[1] - 02000 + curExtLength;
    freeSpace[curZone] = freeSpace[curZone] + curExtLength + 1;
    set_dirty_data();
    acc = curExtent.d;
}

// DOes not seem to
void MarsImpl::free(uint64_t arg) {
    do {
        find_item(arg);
        work2 = (m16[1].d >> 10) & 077777;
        m5 = loc116;
        while (m5 != work2) {
            m16[m5+1] = m16[m5+2];
            ++m5;
        };
        free_extent();
        arg = next_extent(curExtent);
    } while (arg);
    set_dirty_tab();
}

// When an extent chain must be freed but not the descriptor
// (for OP_UPDATE)
void MarsImpl::free_from_current_extent() {
    free_extent();
    if (next_extent(curExtent)) {
        free(next_extent(curExtent));
    } else {
        set_dirty_tab();
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
        set_dirty_data();
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

// Returns true if skipping of micro-instructions is needed.
bool MarsImpl::step() {
    acc = Array[idx].d;
    if (!acc)
        throw Mars::ERR_NO_CURR;
    m5 = acc;
    if (!m16.d) {
        // Step forward
        acc += 2;
        m5 = acc;
        if ((acc & 01777) == block_len(curMetaBlock[-1])) {
            auto next = next_block(curMetaBlock[-1]);
            if (!next) {
                skip(Mars::ERR_NO_NEXT);
                return true;
            }
            get_secondary_block(next);
            Array[idx-2] = Array[idx-2] + (1<<15);
            m5 = 0;
        }
    } else {
        // Step back
        if (!m5.d) {
            auto prev = prev_block(curMetaBlock[-1]);
            if (!prev) {
                skip(Mars::ERR_NO_PREV);
                return true;
            }
            get_secondary_block(prev);
            Array[idx-2] = Array[idx-2] - (1<<15);
            m5 = block_len(Secondary[0]);
        }
        m5 = m5 - 2;
    }
    Array[idx] = m5 | ONEBIT(31);
    curkey = curMetaBlock[m5];
    adescr = curMetaBlock[m5+1];
    return false;
}

bool MarsImpl::a00334(word arg) {
    temp = arg;
    curlen = curlen + arg;
    usrloc = myloc;
  a00270:
    if (proc270()) {
        // COMPARE failed, skip an instruction
        return true;
    }
    if (m5.d) {
        aitem = aitem + temp;
        curExtLength = curExtLength - temp;
        acc = (*aitem).d;
        desc1 = acc;
    } else {
        acc = next_extent(curExtent);
        if (acc) {
            find_item(acc);
            jump(a00270);
        }
        if (jmpoff == DONE
            // impossible? should compare with A00317, or deliberate as a binary patch?
            || temp.d) {
            skip(Mars::ERR_STEP);
            return true;
        }
    }
    return false;
}

uint64_t MarsImpl::a00340(uint64_t arg) {
    find_item(arg);
    jmpoff = A00317;
    a00334(desc2);
    return acc;
}

void MarsImpl::assign_and_incr(uint64_t arg) {
    m16 = arg;
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

// Throws overflow after freeing pending extents.
void MarsImpl::overflow(word ext) {
    auto next = next_extent(ext);
    if (next) {
        free(next);
        throw Mars::ERR_OVERFLOW;
    }
    set_dirty_tab();
    throw Mars::ERR_OVERFLOW;
}

void MarsImpl::prepare_chunk() {
    work = freeSpace[m5-1].d - 1;
    --m5;
    if (work.d < mylen.d) {
        remlen = mylen - work;
        mylen = work;
        usrloc = usrloc - work;
        acc = m5.d;
        ++m5;
    } else {
        usrloc = usrloc - mylen + 1;
        acc = m5.d;
        m5 = 0;
    }
}

// The 3 entry points are disambiguated by the original code addresses
// for lack of a better understanding of their semantics.
void MarsImpl::allocator(int addr) {
    switch (addr) {
    case 01022:
        make_extent_header();
        // FALL THROUGH
    case 01023:
        m5 = 0;
        chainHead = 0;
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
            usrloc = usrloc + mylen - 1;
            m5 = dblen;
            while (freeSpace[m5-1].d < 2) {
                if (--m5.d)
                    continue;
                overflow(chainHead);
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
        loc116 = m16;
        acc = (m16.d << 39) | chainHead.d;
        // FALL THROUGH
    case 01047:                 // acc has the extent id in bits 48-40
        chainHead = acc;
        newDescr = (get_id(acc) << 10) | curZone.d;
        m16 = mylen;
        acc = m7[1].d - m16.d + 02000;
        m7[1] = acc;
        acc &= 01777;
        work = acc;
        work2 = curbuf + work + 1;
        if (!m5.d) {
            --m16;
            ++work2;
        }
        if (m16.d) {
            if (verbose)
                std::cerr << "To DB: ";
            copy_words(work2, usrloc, m16.d);
        }
        m7[loc116+1] = (mylen.d << 10) | work.d | chainHead.d;
        m16 = freeSpace;
        if (verbose)
            std::cerr << "Reducing free " << std::oct << freeSpace[curZone].d
                      << " by len " << mylen.d << " + 1\n";
        freeSpace[curZone] = freeSpace[curZone].d - (mylen.d+1);
        if (verbose)
            std::cerr << "Got " << freeSpace[curZone].d << '\n';
        if (!m5.d) {
            work2[-1] = extentHeader;
            set_dirty_both();
            return;
        }
        set_dirty_data();
        acc = newDescr.d;
        acc <<= 20;
        acc &= BITS(48);
        chainHead = acc;
        mylen = remlen;
        if (--m5.d) {
            while (freeSpace[m5-1].d < 2) {
                if (--m5.d)
                    continue;
                overflow(chainHead);
            }
            prepare_chunk();
            jump(chunk);
        }
        overflow(chainHead);       // will throw
    }
}

// There was no check for dblen == 0
void MarsImpl::mkctl() {
    curZone = 0;
    int nz = dblen = dbdesc >> 18;
    IOpat = dbdesc & 0777777;
    curbuf = bdtab;
    work2 = IOpat | bdtab.d << 20;
    bdtab[1] = 01777;           // last free location in zone
    do {
        --nz;
        bdbuf[nz] = 01776;      // free countwords in zone
        bdtab[0] = DBkey | nz;
        IOword = work2 + nz;
        IOcall(IOword);
    } while (nz);
    myloc = freeSpace = bdbuf; // freeSpace[0] is now the same as bdbuf[0]
    metaflag = ROOT_METABLOCK;
    d00011 = 0;
    make_metablock();
    allocator(01022);
    mylen = dblen;              // length of the free space array
    // This trick results in max possible DB length = 753 zones.
    bdbuf[0] = 01731 - mylen.d;
    // Invoking OP_INSERT for the freeSpace array
    usrloc = myloc;
    allocator(01022);
    curDescr = newDescr;          // unused
}

void MarsImpl::update_by_reallocation() {
    // d00026 is used only in this function
    d00026 = curExtent;
    curExtent = curExtent & 01777;
    free_from_current_extent();
    ++mylen;
    m5 = 0;
    m7 = curbuf;
    acc = get_id(d00026) << 39;
    allocator(01047);
    acc = next_extent(d00026);
    if (acc) {
        free(acc);
    } else {
        set_dirty_tab();
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

// m16 points to the payload of a metadata block
void MarsImpl::search_in_block() {
    bool parent = m16[1].d & ONEBIT(48);
    m5 = m16[-1] & 01777;
    if (verbose)
        std::cerr << "Comparing " << std::dec << m5.d/2 << " elements\n";
    while (m5.d) {
        if (m16[m5-2].d <= (temp.d ^ BITS(47)))
            break;
        m5.d -= 2;
    }
    m5.d -= 2;
    Array[idx] = m5.d | ONEBIT(31);
    if (!parent) {
        curkey = curMetaBlock[m5];
        adescr = curMetaBlock[m5+1];
        return;
    }
    idx = idx + 2;
    if (idx > 7) {
        std::cerr << "Idx = " << idx << ": DB will be corrupted\n";
    }
    get_secondary_block(m16[m5+1]);
    m16 = Secondary+1;
    search_in_block();
}

void MarsImpl::find(word k) {
    temp = k;          // the key to find, not necessarily bdvect[010]
    idx = 0;
    temp = temp.d ^ BITS(47);
    m16 = Metadata+1;
    if (!metaflag.d) {
        get_root_block();
    }
    m16 = Metadata+1;
    search_in_block();
}

void MarsImpl::update(word arg) {
    d00012 = arg;
    m5 = find_item(d00012.d) - 1;
    if (m5 == mylen) {
        // The new length of data matches the first extent length
        if (m5.d) {
            do {
                aitem[m5] = usrloc[m5-1];
            } while (--m5.d);
        }
        m5 = m16;
        make_extent_header();
        *aitem = extentHeader;
        set_dirty_data();
        if (!next_extent(curExtent))
            return;       // No extents to free: done
        m5[loc116+1] = curExtent & ~(BITS(19) << 20); // dropping the extent chain
        free(next_extent(curExtent)); // Free remaining extents
        set_dirty_tab();
        return;
    }
    make_extent_header();
    m16 = curbuf;
    acc = m16[1] >> 10;
    m5 = acc + 1;
    acc = (m16[1].d - acc) & 01777;
    acc += curExtLength.d - 2;
    work = acc;
    if (acc >= mylen.d) {
        update_by_reallocation();
        set_dirty_tab();
        return;
    }
    usable_space();
    acc = (*aitem).d & 077777;
    acc += datumLen.d;
    if (acc < mylen.d) {
        throw Mars::ERR_OVERFLOW;
    }
    m16 = curbuf;
    mylen = work;
    update_by_reallocation();
    usrloc = usrloc + mylen;
    mylen = (extentHeader & 077777) - mylen;
    extentHeader = usrloc[-1];
    allocator(01023);
    find_item(d00012.d);
    // This is likely chaining the extents
    acc = (newDescr.d << 20) & BITS(48);
    m16[loc116+1] = acc | curExtent.d;
    set_dirty_both();
    return;
}

Error MarsImpl::eval() try {
    std::unordered_map<void*,int> jumptab;
    std::vector<void*> targets;
    m13 = abdv;
    if (bdtab[0] != DBkey && IOpat.d) {
        IOword = bdtab.d << 20 | ONEBIT(40) | IOpat.d;
        IOcall(IOword);
    }
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
    // The switch is entered with m6 = cmd0 and acc = 0
    if (verbose)
        std::cerr << "Executing microcode " << std::oct << m5.d << '\n';
    acc = 0;
    switch (m5.d) {
    case 0:
        break;
    case Mars::OP_BEGIN:
        find(0);
        break;
    case Mars::OP_LAST:
        find(BITS(47));
        break;
    case Mars::OP_PREV:
        m16 = 1;
        if (step())
            jump(next);
        break;
    case Mars::OP_NEXT:
        m16 = 0;
        if (step())
            jump(next);
        break;
    case Mars::OP_INSMETA:
        d00011 = 0;
        make_metablock();
        allocator(01022);
        curDescr = newDescr;
        break;
    case Mars::OP_SETMETA:
        idx = 0;
        datumLen = 041;
        m16 = Metadata-1;
        get_block(adescr);
        break;
    case 7:
        jmpoff = A00317;
        a00334(desc2);
        break;
    case Mars::OP_INIT:
        mkctl();
        break;
    case Mars::OP_FIND:
        if (key == 0 || key.d & ONEBIT(48))
            throw Mars::ERR_INV_NAME;
        find(key);
        break;
    case Mars::OP_SETCTL:
        myloc = &dbdesc;
        mylen = 3;              // accounting for a possible password
        cpyout(adescr.d);
        break;
    case Mars::OP_AVAIL:
        usable_space();
        break;
    case Mars::OP_MATCH:
        if (curkey == key)
            break;
        skip(Mars::ERR_NO_NAME);
        jump(next);
    case Mars::OP_NOMATCH:
        if (curkey != key)
            break;
        skip(Mars::ERR_EXISTS);
        jump(next);
    case Mars::OP_STRLEN:
        find_end_mark();
        break;
    case Mars::OP_WORDLEN:
        find_end_word();
        break;
    case Mars::OP_UPDATE:
        usrloc = myloc.d;
        update(adescr);
        break;
    case Mars::OP_INSERT:
        usrloc = myloc;
        allocator(01022);
        curDescr = newDescr;
        break;
    case Mars::OP_GET:
        cpyout(adescr.d);
        break;
    case Mars::OP_FREE:
        free(adescr.d);
        break;
    case Mars::OP_PASSWD:
        if (givenp != savedp)
            throw Mars::ERR_WRONG_PASSWORD;
        break;
    case Mars::OP_OPEN:
        setctl(dbdesc.d);
        break;
    case Mars::OP_ADDKEY:
        d00011 = key;
        adescr = acc = curDescr.d;
        jump(addkey);
    case Mars::OP_DELKEY:
        limit = 0;              // looks dead
        jump(delkey);
    case Mars::OP_LOOP:
        acc = orgcmd.d;
        jump(next);
    case Mars::OP_ROOT:
        DBkey = ROOTKEY;
        dbdesc = arch;
        setctl(dbdesc.d);
        break;
    case 032:
        *aitem = curDescr;
        set_dirty_both();
        break;
    case Mars::OP_LENGTH:
        info(adescr.d);
        break;
    case Mars::OP_DESCR:
        totext();
        break;
    case Mars::OP_SAVE:
        acc = dirty.d;
        save();
        if (mars.dump_diffs)
            mars.dump();
        return Mars::ERR_SUCCESS;
    case Mars::OP_ADDMETA:
        m16 = curMetaBlock;
        m16[Array[idx]+1] = curDescr;
        jump(a01160);
    case Mars::OP_SKIP:
        acc = curcmd >> 12;
        jump(next);
    case Mars::OP_STOP:
        jump(next);
    case Mars::OP_IFEQ:
        jmpoff = COMPARE;
        if (a00334(mylen))
            jump(next);
        break;
    case Mars::OP_MODIFY:
        jmpoff = TOBASE;
        if (a00334(mylen))
            jump(next);
        break;
    case Mars::OP_COPY:
        jmpoff = FROMBASE;
        if(a00334(mylen))
            jump(next);
        break;
    case 044:
        adescr = desc1;
        break;
    case Mars::OP_LOCK:
        lock();
        break;
    case 046:
        if (cmd46())
            jump(next);
        break;
    case Mars::OP_CALL:
        acc = desc1.d;
        indjump(outadr);
    case Mars::OP_CHAIN:
        // cmd := mem[bdvect[src]++]
        // curcmd is ..... src 50
        assign_and_incr(&orgcmd-BDVECT);
        jump(next);
    case 051:
        // myloc := mem[bdvect[src]++]
        // curcmd is ..... src 51
        assign_and_incr(&myloc-BDVECT);
        break;
    case 052:
        // bdvect[dst] := mem[bdvect[src]++]
        // curcmd is ..... src dst 52
        acc = (curcmd >> 6) & 077;
        curcmd = curcmd >> 6;
        assign_and_incr(acc);
        break;
    case Mars::OP_ASSIGN:
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
    case Mars::OP_EXIT:
        if (mars.dump_diffs)
            mars.dump();
        return Mars::ERR_SUCCESS;
    default:
        // In the original binary, loss of control ensued.
        std::cerr << std::format("Invalid micro-operation {:o} encountered\n", m5.d);
        abort();
    }
    jump(cmd0);
  delkey:
    m16 = curMetaBlock;
    m5 = m16 + Array[idx];
    loc116 = m5.d ^ curMetaBlock.d;
    work2 = m16[-1] & 01777;
    if (work2.d < 2) std::cerr << "work2 @ delkey < 2\n";
    work = work2 + curMetaBlock;
    do {
        *m5 = m5[2];
        ++m5;
    } while (m5 != work);
    m16[-1] = m16[-1] - 2;
    acc = m16[-1].d & 01777;
    if (!acc) {
        acc = idx;
        if (!acc)
            jump(a01160);
        free(curBlockDescr.d);
        desc2 = 1;
        d00025 = Secondary[0];
        d00032 = make_block_header(prev_block(Secondary[0]), 0, 0);
        acc = prev_block(d00032);
        if (acc) {
            acc = a00340(acc);
            Secondary[0] = make_block_header(prev_block(acc), 0, block_len(acc));
            set_header(make_block_header(prev_block(acc), next_block(d00025), block_len(acc)));
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
        m16 = curMetaBlock;
        m16[Array[idx]] = acc;
        acc = Array[idx].d;
    }
  a01136:
    acc = d00032.d;
    acc >>= 29;
    acc |= ONEBIT(48);
  addkey:
    newDescr = acc;
    m16 = curMetaBlock;
    m5 = curMetaBlock + Array[idx] + 2;
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
    m5[1] = newDescr;
    m16[-1] = m16[-1] + 2;
  a01160:                       // generic metablock update ???
    usrloc = m16 - 1;
    if (idx == 0) {
        mylen = 041;
        if (m16[-1] == 040) {
            if (usable_space() < 0101) {
                metaflag = 0;
                free(curDescr.d);
                throw Mars::ERR_OVERFLOW;
            }
            acc -= 0101;
            allocator(01022);
            Metadata[2] = newDescr.d | ONEBIT(48);
            Metadata[0] = 2;
            mylen = 041;
        }
        update(metaflag);
        jump(rtnext);
    }
    acc = m16[-1].d & 01777;
    if (acc != 0100) {
        acc = m16[-1].d & 01777;
        mylen = ++acc;
        update(curBlockDescr);
        jump(rtnext);
    }
    work = (idx * 2) + 041;
    if (Metadata[0] == 036) {
        // The current metadata block is full, account for another one
        work = work + 044;
    }
    if (usable_space() < work.d) {
        metaflag = 0;
        free(curDescr.d);
        throw Mars::ERR_OVERFLOW;
    }
    d00025 = Secondary[040];
    Secondary[040] = make_block_header(curBlockDescr.d, next_block(Secondary[0]), 040);
    usrloc.d += 040;
    mylen = 041;
    allocator(01022);
    Secondary[040] = d00025;
    d00011 = Secondary[041];
    d00025 = Secondary[0];
    Secondary[0] = make_block_header(prev_block(d00025), newDescr.d, 040);
    d00032 = make_block_header(newDescr.d, 0, 0);
    m16 = Secondary+1;
    usrloc = Secondary;
    acc = Secondary[0].d & 01777;
    mylen = ++acc;
    update(curBlockDescr);
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
    acc = next_block(d00025);
    if (acc) {
        a00340(acc);
        set_header(d00032 | (acc & BITS(29)));
    }
  a01242:
    curMetaBlock = Metadata+1;
    if (idx == 0) {
        if (goto_.d)
            std::cerr << "Returning from pr1232 with non-0 goto_\n";
        set_dirty_tab();
        jump(rtnext);
    }
    idx = idx - 2;
    if (idx != 0) {
        get_secondary_block(Array[idx-1]);
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
        d00012 = acc - 1;
        if (step())
            jump(next);
        acc = d00012.d;
    }
    if (usable_space() < 0101) {
        metaflag = 0;
        free(curDescr.d);
        throw Mars::ERR_OVERFLOW;
    }
    acc -= 0101;
    allocator(01022);
    Metadata[2] = newDescr | ONEBIT(48);
    Metadata[0] = 2;
    mylen = 041;
    update(metaflag);
    jump(rtnext);
} catch (Error e) {
    if (erhndl.d) {
        // savm16 = erhndl;  // returning to erhndl instead of the point of call
    }
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
    impl.orgcmd = OP_INIT;
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
    impl.orgcmd = mcprog(OP_ROOT, OP_FIND, OP_NOMATCH, OP_INSERT, OP_ADDKEY);
    impl.eval();
    // OP_FIND and OP_MATCH do not seem to be necessary
    impl.orgcmd = mcprog(OP_FIND, OP_MATCH, OP_SETCTL, OP_INIT);
    return impl.eval();
}

Error Mars::opend(const char * k) {
    if (verbose)
        std::cerr << "Running opend('" << k << "')\n";
    impl.key = *reinterpret_cast<const uint64_t*>(k);
    impl.orgcmd = mcprog(OP_ROOT, OP_FIND, OP_MATCH, OP_SETCTL, OP_OPEN);
    return impl.eval();
}

Error Mars::putd(const char * k, int loc, int len) {
    if (verbose)
        std::cerr << std::format("Running putd('{}', '{}':{})\n", k,
                                 reinterpret_cast<char*>(data+loc), len);
    impl.key = *reinterpret_cast<const uint64_t*>(k);
    impl.mylen = len;
    impl.myloc = loc;
    impl.orgcmd = mcprog(OP_FIND, OP_NOMATCH, OP_INSERT, OP_ADDKEY);
    return impl.eval();
}

Error Mars::putd(uint64_t k, int loc, int len) {
    if (verbose) {
        std::cerr << std::format("Running putd({:016o}, {:05o}:{})\n", k, loc, len);
    }
    impl.key = k;
    impl.mylen = len;
    impl.myloc = loc;
    impl.orgcmd = mcprog(OP_FIND, OP_NOMATCH, OP_INSERT, OP_ADDKEY);
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
    impl.orgcmd = mcprog(OP_FIND, OP_NOMATCH, OP_INSERT, OP_ADDKEY);
    return impl.eval();
}

Error Mars::modd(const char * k, int loc, int len) {
    impl.key = *reinterpret_cast<const uint64_t*>(k);
    impl.mylen = len;
    impl.myloc = loc;
    impl.orgcmd = mcprog(OP_FIND, OP_NOMATCH, OP_COND,
                         OP_INSERT, OP_ADDKEY, OP_STOP,
                         OP_UPDATE);
    return impl.eval();
}

Error Mars::modd(uint64_t k, int loc, int len) {
    impl.key = k;
    impl.mylen = len;
    impl.myloc = loc;
    impl.orgcmd = mcprog(OP_FIND, OP_NOMATCH, OP_COND,
                         OP_INSERT, OP_ADDKEY, OP_STOP,
                         OP_UPDATE);
    return impl.eval();
}

Error Mars::getd(const char * k, int loc, int len) {
    impl.key = *reinterpret_cast<const uint64_t*>(k);
    impl.mylen = len;
    impl.myloc = loc;
    impl.orgcmd = mcprog(OP_FIND, OP_MATCH, OP_GET);
    return impl.eval();
}

Error Mars::getd(uint64_t k, int loc, int len) {
    if (verbose) {
        std::cerr << std::format("Running getd({:016o}, {:05o}:{})\n", k, loc, len);
    }
    impl.key = k;
    impl.mylen = len;
    impl.myloc = loc;
    impl.orgcmd = mcprog(OP_FIND, OP_MATCH, OP_GET);
    return impl.eval();
}

Error Mars::deld(const char * k) {
    impl.key = *reinterpret_cast<const uint64_t*>(k);
    impl.orgcmd = mcprog(OP_FIND, OP_MATCH, OP_FREE, OP_DELKEY);
    return impl.eval();
}

Error Mars::deld(uint64_t k) {
    impl.idx = 0;
    impl.key = k;
    impl.orgcmd = mcprog(OP_FIND, OP_MATCH, OP_FREE, OP_DELKEY);
    return impl.eval();
}

Error Mars::root() {
    impl.orgcmd = OP_ROOT;
    return impl.eval();
}

uint64_t Mars::first() {
    impl.orgcmd = mcprog(OP_BEGIN, OP_NEXT);
    impl.eval();
    return impl.curkey.d;
}

uint64_t Mars::last() {
    impl.orgcmd = OP_LAST;
    impl.eval();
    return impl.curkey.d;
}

uint64_t Mars::prev() {
    impl.orgcmd = OP_PREV;
    impl.eval();
    return impl.curkey.d;
}

uint64_t Mars::next() {
    impl.orgcmd = OP_NEXT;
    impl.eval();
    return impl.curkey.d;
}

uint64_t Mars::find(const char * k) {
    impl.key = *reinterpret_cast<const uint64_t*>(k);
    impl.orgcmd = OP_FIND;
    impl.eval();
    return impl.curkey.d;
}

uint64_t Mars::find(uint64_t k) {
    impl.key = k;
    impl.orgcmd = OP_FIND;
    impl.eval();
    return impl.curkey.d;
}

int Mars::getlen() {
    impl.orgcmd = OP_LENGTH;
    if (impl.eval())
        return -1;
    return impl.datumLen.d;
}

Error Mars::cleard(bool forward) {
    impl.key = 0;
    if (forward)
        impl.orgcmd = mcprog(OP_BEGIN, OP_NEXT, OP_COND,
                             OP_FREE, OP_DELKEY, OP_LOOP);
    else
        impl.orgcmd = mcprog(OP_LAST, OP_NOMATCH, OP_COND,
                             OP_FREE, OP_DELKEY, OP_LOOP);
    return impl.eval();
}

int Mars::avail() {
    impl.orgcmd = OP_AVAIL;
    impl.datumLen = 0;
    impl.eval();
    return impl.datumLen.d;
}

Error Mars::eval(uint64_t microcode) {
    impl.orgcmd = microcode;
    return impl.eval();
}
