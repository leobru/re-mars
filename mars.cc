#include <cstring>
#include <fstream>
#include <iostream>
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
#define INDIRECT ONEBIT(48)
#define META_SIZE (2*16+1)

// Field offsets of interest within BDVECT
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

// Base/index inversion check was to ascertain which of the constituents of an indexing
// operation was the "base", and which was the "index", using simple heuristics.
word& word::operator[](word x) const {
    if ((x.d & 077770) != 077770 &&
        ((x.d & 077777) >= 02000 ||
         ((x.d & 077777) >= 01400 && (d & 077777) < 0400)))
        std::cerr << std::format("Base/index inversion: base = {}, index = {}\n",
                                 d, x.d & 077777);
    return *word(*mars, d+x.d);
}

#define ONEBIT(n) (1ULL << ((n)-1))   // one bit set, from 1 to 48
#define BITS(n)   (~0ULL >> (64 - n)) // bitmask of n bits from LSB
// #define jump(x) do { std::cerr << "Jump to " #x "\n"; goto x; } while(0)
#define jump(x) goto x

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

    // For convenience of detection which locations not to trace,
    // the registers are mapped to the lower addresses of the RAM.

    // Fields of BDVECT
    wordref outadr, orgcmd, curcmd, disableSync, givenp, key,
        erhndl, curDescr, myloc, loc14, mylen, bdbuf, bdtab,
        dbdesc, DBkey, savedp, freeSpace, adescr, newkey,
        curkey, endmrk, desc1, offset, IOpat, curExtLength, extPtr, datumLen,
        curMetaBlock, dblen, curlen, loc116,
        curExtent, curbuf, curZone, curBlockDescr,
        dirty;

    uint64_t & idx;
    uint64_t cont;              // continuation instruction word
// RootBlock[-1] (loc54) is also used
    word * const Cursor;
    word * const RootBlock;
    word * const Secondary;

    // Fields of BDSYS
    uint64_t & arch;
    wordref abdv;
    std::unordered_map<std::string, Page> DiskImage;

    // History of all stores, for debug.
    std::unordered_map<size_t, uint64_t> all_stores;

    MarsImpl(Mars & up) :
        mars(up), data(up.data), verbose(up.verbose),
        outadr(data[BDVECT+1]),
        orgcmd(data[BDVECT+3]),
        curcmd(data[BDVECT+5]),
        disableSync(data[BDVECT+6]),
        givenp(data[BDVECT+7]),
        key(data[BDVECT+010]),
        erhndl(data[BDVECT+011]),
        curDescr(data[BDVECT+012]),
        myloc(data[BDVECT+013]),
        loc14(data[BDVECT+014]),
        mylen(data[BDVECT+015]),
        bdbuf(data[BDVECT+016]),
        bdtab(data[BDVECT+017]),
        // Cursor is BDVECT+020 to BDVECT+027
        dbdesc(data[BDVECT+030]),
        DBkey(data[BDVECT+031]),
        savedp(data[BDVECT+032]),
        freeSpace(data[BDVECT+034]),
        adescr(data[BDVECT+035]),
        newkey(data[BDVECT+036]),
        curkey(data[BDVECT+037]),
        endmrk(data[BDVECT+040]),
        desc1(data[BDVECT+041]),
        offset(data[BDVECT+042]),
        IOpat(data[BDVECT+044]),
        curExtLength(data[BDVECT+045]),
        extPtr(data[BDVECT+046]),
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
        Cursor(data+BDVECT+021), // normally used to access Cursor[-1] to Cursor[6]
        // RootBlock[-1] (loc54) is also used
        RootBlock(data+BDVECT+055), // spans 041 words, up to 0115
        // Secondary[-1] loc116 is also used
        Secondary(data+BDVECT+0117), // spans 0101 words, up to 0217

        // Fields of BDSYS
        arch(data[BDSYS+4].d),
        abdv(data[BDSYS+3])
        { }
    void IOflush();
    void IOcall(word);
    void get_zone(uint64_t);
    void save(bool);
    void finalize(word);
    uint64_t make_extent_header();
    word make_metablock(uint64_t key);
    uint64_t usable_space();
    uint64_t find_item(uint64_t);
    void info(uint64_t);
    void totext();
    void set_header(word);
    void copy_words(word dst, word src, int len);
    void copy_chained(int len, word &usrloc);
    void cpyout(uint64_t);
    void lock();
    void get_block(word, word*), get_root_block(), get_secondary_block(word);
    void free_from_current_extent(int);
    uint64_t get_word(uint64_t arg, uint64_t offset);
    uint64_t skip(Error);
    void setctl(uint64_t);
    void free_extent(int), free(uint64_t);
    void find_end_mark(), find_end_word();
    enum Ops {
        FROMBASE, TOBASE, COMPARE, SEEK, DONE
    };
    bool perform(Ops, bool &, unsigned &, word &);
    bool step(word), access_data(Ops, word);
    bool cmd46();
    void assign_and_incr(uint64_t);
    void setup();
    Error eval();
    uint64_t one_insn();
    void overflow(word);
    int prepare_chunk(int, int &remlen, word &usrloc);
    uint64_t handle_chunk(int zone, word head);
    uint64_t allocator(word usrloc), allocator1023(uint64_t extentHeader, word usrloc);
    uint64_t allocator1047(int loc, uint64_t head, uint64_t extentHeader, int remlen, word &usrloc);
    enum KeyOp { ADDKEY, DELKEY, A01160 };
    uint64_t key_manager(KeyOp, uint64_t key = ARBITRARY_NONZERO);
    void check_space(unsigned need);
    void mkctl(), find(word);
    void update_by_reallocation(int, uint64_t, word &usrloc), search_in_block(word*, word), update(word, word usrloc);
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

uint64_t get_id(const word & x) {
        return x.d >> 39;
}

uint64_t get_id(uint64_t x) {
        return x >> 39;
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

struct Accumulator {
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

const char * msg[] = {
    "Zero key", "Page corrupted", "No such record", "Invalid name",
    "1st page corrupted", "Overflow", "Out of bounds", "No such name",
    "Name already exists", "No end symbol", "Internal error", "Record too long",
    "DB is locked", "No current record", "No prev. record", "No next record",
    "Wrong password" };

void MarsImpl::IOflush() {
    for (auto & it : DiskImage) {
        const std::string& nuzzzz = it.first;
        std::ofstream f(nuzzzz);
        if (!f) {
            std::cerr << std::format("Could not open {} ({})\n", nuzzzz, strerror(errno));
            exit(1);
        }
        f.write(reinterpret_cast<char*>(&it.second), sizeof(Page));

        if (mars.dump_txt_zones) {
            std::string txt(nuzzzz+".txt");
            std::ofstream t(txt);
            if (!t) {
                std::cerr << std::format("Could not open {} ({})\n", txt, strerror(errno));
                exit(1);
            }
            const char * zone = nuzzzz.c_str()+2;
            for (int i = 0; i < 1024; ++i) {
                t << std::format("{}.{:04o}:  {:04o} {:04o} {:04o} {:04o}\n",
                        zone, i, it.second.w[i] >> 36,
                        (it.second.w[i] >> 24) & 07777,
                        (it.second.w[i] >> 12) & 07777,
                        (it.second.w[i] >> 0) & 07777);
            }
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
            std::ifstream f(nuzzzz);
            if (!f) {
                std::cerr << "\tZone " << nuzzzz << " does not exist yet\n";
                for (int i = page*1024; i < (page+1)*1024; ++i)
                    data[i] = ARBITRARY_NONZERO;
                mars.trace_stores = stores;
                return;
            }
            if (verbose)
                std::cerr << "\tFirst time - reading from disk\n";
            f.read(reinterpret_cast<char*>(&DiskImage[nuzzzz]), sizeof(Page));
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
// returns the pointer to the zone read in curbuf
void MarsImpl::get_zone(uint64_t arg) {
    curZone = arg;
    uint64_t zoneKey = arg | DBkey.d;
    curbuf = (arg & 01777) ? bdbuf : bdtab;
    if (curbuf[0] == zoneKey)
        return;
    if (curZone.d && is_dirty_buf()) {
        dirty = dirty.d & ~2;
        IOcall((IOpat.d | bdbuf.d << 20) + (curbuf[0].d & 01777));
    }
    IOcall((curbuf.d << 20 | curZone.d | ONEBIT(40)) + IOpat.d);
    if (curbuf[0] == zoneKey)
        return;
    std::cerr << std::format("Corruption: zoneKey = {:o}, data = {:o}\n", zoneKey, curbuf[0].d);
    throw Mars::ERR_BAD_PAGE;
}

void MarsImpl::save(bool force_tab = false) {
    if (force_tab || is_dirty_tab()) {
        IOcall(IOpat.d | bdtab.d << 20);
    }
    if (is_dirty_buf()) {
        IOcall((IOpat.d | bdbuf.d << 20) + (bdbuf[0].d & 01777));
    }
    dirty = 0;
}

// The argument is nonzero (a part of the error message) if there is an error
void MarsImpl::finalize(word err) {
    data[BDSYS+011] = err;
    bool force_tab = false;
    if (disableSync != 0) {
        if (disableSync != 1)
            return;
        if (bdtab[1].d & LOCKKEY) {
            bdtab[1].d &= ~LOCKKEY;
            force_tab = true;
        }
    }
    save(force_tab);
}

// Returns the constructed extent header
uint64_t MarsImpl::make_extent_header() {
    using namespace std::chrono;
    const year_month_day ymd{floor<days>(system_clock::now())};
    unsigned d{ymd.day()}, m{ymd.month()};
    int y{ymd.year()};

    uint64_t stamp = mars.zero_date ? 0 : (d/10*16+d%10) << 9 |
        (m/10*16+m%10) << 4 |
        y % 10;
    // This is the only use of loc14
    // and of this bit range of the extent header structure
    return (loc14.d & (BITS(18) << 15)) | mylen.d | (stamp << 33);
}

// Prepares a metadata block, retuirns its address
word MarsImpl::make_metablock(uint64_t key) {
    mylen = META_SIZE;
    data[FAKEBLK] = 2;
    data[FAKEBLK+1] = key;
    data[FAKEBLK+2] = Cursor[-1];
    return word(mars, FAKEBLK);
}

// Usable space is one word (extent handle) less than free space
uint64_t MarsImpl::usable_space() {
    uint64_t l = 0;
    for (int i = dblen.d - 1; i >= 0; --i) {
        if (freeSpace[i] != 0)
            l += freeSpace[i].d - 1;
    }
    return l;
}

// Finds item indicated by the zone number in bits 10-1
// and by the number within the zone in bits 19-11
// Sets 'extPtr', also returns the extent length in 'curExtLength'
uint64_t MarsImpl::find_item(uint64_t arg) {
    get_zone(arg & 01777);
    auto extNum = arg & (BITS(10) << 10);
    if (!extNum)
        throw Mars::ERR_ZEROKEY;      // Attempting to find a placeholder?
    unsigned id = (extNum >> 10) & 0777;
    uint64_t record = curbuf[1].d & (BITS(10) << 10);
    if (!record)
        throw Mars::ERR_NO_RECORD;    // Attempting to find a deleted record?
    // Records within the zone may be non-contiguous but sorted;
    // need to search only from the indicated position toward lower addresses.
    if (record >= extNum)
        record = extNum;
    loc116 = record >> 10;
    for (;;) {
        curExtent = curbuf[loc116+1];
        if (get_id(curExtent) == id) {
            break;
        }
        if (--loc116 == 0) {
            throw Mars::ERR_NO_RECORD;
        }
    }
    extPtr = curbuf + get_extstart(curExtent) + 1;
    curExtLength =  get_extlength(curExtent);
    return curExtLength.d;
}

// Assumes that arg points to a datum with the standard header.
// Puts the full header to desc1, and its length to datumLen.
void MarsImpl::info(uint64_t arg) {
    find_item(arg);
    desc1 = *extPtr;
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
    offset = Mars::tobesm(std::format("\172\033\0\0\0\0"));
}

void MarsImpl::set_header(word arg) {
    *extPtr = arg;
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
}

// Copies chained extents to user memory (len = length of the first extent)
void MarsImpl::copy_chained(int len, word &usrloc) { // a01423
    for (;;) {
        if (len) {
            if (verbose)
                std::cerr << "From DB: ";
            copy_words(usrloc, extPtr, len);
        }
        usrloc = usrloc + len;
        if (!next_extent(curExtent))
            return;
        len = find_item(next_extent(curExtent));
    }
}

void MarsImpl::cpyout(uint64_t descr) {
    info(descr);
    auto usrloc = myloc;
    if (mylen != 0 && mylen < datumLen)
        throw Mars::ERR_TOO_LONG;
    // Skip the first word (the header) of the found item
    ++extPtr;
    --curExtLength;
    copy_chained(curExtLength.d, usrloc);
}

// Not really a mutex
void MarsImpl::lock() {
    uint64_t writeWord = (bdtab.d << 20) | IOpat.d;
    IOcall(writeWord | ONEBIT(40));
    bdtab[1] = bdtab[1].d ^ LOCKKEY;
    if (!(bdtab[1].d & LOCKKEY))
        throw Mars::ERR_LOCKED;
    bdbuf[0] = LOCKKEY;         // What is the purpose of this?
    IOcall(writeWord);
}

// Finds and reads a block of metadata identified by 'descr'
// into memory pointed to by 'dest', if 'curBlockDescr' does
// not have the same ID already.
void MarsImpl::get_block(word descr, word * dest) {
    Cursor[idx-1] = descr;
    loc116 = descr & BITS(19);
    word usrloc(mars, 0);
    usrloc = dest;
    curMetaBlock = dest + 2;
    if (loc116 == curBlockDescr)
        return;
    curBlockDescr = loc116;
    copy_chained(find_item(curBlockDescr.d), usrloc);
}

// Requests the root block into the main RootBlock array
void MarsImpl::get_root_block() {
    get_block(ROOT_METABLOCK, RootBlock-1);
}

// Requests a block into the Secondary metadata array
void MarsImpl::get_secondary_block(word descr) {
    get_block(descr, Secondary-1);
}

// If the micro-instruction after tue current one is OP_COND (00)
// and there are instructions after it to be skipped, skip them;
// otherwise, throw an error.
uint64_t MarsImpl::skip(Error e) {
    auto next = curcmd >> 6;
    if (!next || (next & 077))
        throw e;
    // Removes the current micro-instruction,
    // the 00 after it, and the next 3 micro-instructions,
    // totaling 5. 5 x 6 bit = 30
    return curcmd >> 30;
}

void MarsImpl::setctl(uint64_t location) {
    IOpat = location & 0777777;
    dblen = dbdesc >> 18;
    IOcall(bdtab.d << 20 | IOpat.d | ONEBIT(40));
    curbuf = bdtab;
    if (bdtab[0] != DBkey)
        throw Mars::ERR_BAD_CATALOG;
    idx = 0;
    curBlockDescr = 0;
    curZone = 0;
    freeSpace = bdtab + (bdtab[3] & 01777) + 2;
    get_root_block();
}

// Frees curExtent, which position is given as the argument
void MarsImpl::free_extent(int pos) {
    word loc(mars,get_extstart(curExtent));
    // Correct the data pointers of the extents
    // below the one being freed
    for (;--pos != 0;) {
        if ((curbuf[pos+1].d & 01777) >= loc.d)
            continue;
        curbuf[pos+1] = curbuf[pos+1] + curExtLength;
    }
    word freeLoc(mars, curbuf[1].d & 01777);
    if (freeLoc != loc) {
        if (loc < freeLoc)         // extent cannot be located in the free area
            throw Mars::ERR_INTERNAL;
        int diff = (loc.d - freeLoc.d) & 077777;
        freeLoc = freeLoc + curbuf;
        loc = freeLoc + curExtLength;
        // Move the extent data up to make the free area
        // contiguous
        do {
            loc[diff] = freeLoc[diff];
        } while(--diff);
    }
    // Correct the free area location and the number of extents at once
    curbuf[1] = curbuf[1] - 02000 + curExtLength;
    freeSpace[curZone] = freeSpace[curZone] + curExtLength + 1;
    set_dirty_data();
}

void MarsImpl::free(uint64_t arg) {
    do {
        find_item(arg);
        int extCount = (curbuf[1] >> 10) & 077777;
        int i = loc116.d;       // position of the extent to be freed
        // shrink the extent handle array
        while (i != extCount) {
            curbuf[i+1] = curbuf[i+2];
            ++i;
        };
        free_extent(i);
        arg = next_extent(curExtent);
    } while (arg);
    set_dirty_tab();
}

// When an extent chain must be freed but not the descriptor
// (for OP_UPDATE)
void MarsImpl::free_from_current_extent(int pos) {
    free_extent(pos);
    if (next_extent(curExtent)) {
        free(next_extent(curExtent));
    } else {
        set_dirty_tab();
    }
}

void MarsImpl::find_end_mark() {
    // Searching for the end mark (originally 1 or 2 bytes)
    int i = -mylen.d;
    word limit(mars, myloc.d + mylen.d);
    // Handling only single byte mark, contemporary style
    char * start = (char*)&(limit[i].d);
    char * end = (char*)&(limit[0].d);
    char * found = (char*)memchr(start, endmrk.d & 0xFF, end-start);
    if (!found)
        throw Mars::ERR_NO_END_MARK;
    mylen = (found - start) / 8 + 1;
}

// Performs the operation requested (op)
// on the current extent and adjusts remaining
// length and the user location.
// If the operation was not complete, set done = false
// for SEEK, usrloc is not updated (extent traversal only).
// DONE is unused.
bool MarsImpl::perform(Ops op, bool &done, unsigned &tail, word &usrloc) {
    int len = tail;             // data length to operate upon
    if (tail >= curExtLength.d) {
        // The desired length is too long; adjust it
        tail -= curExtLength.d;
        len = curExtLength.d;
        done = false;
    }
    // Now len = current extent length or less, tail = remaining length (may be 0)
    switch (op) {
    case FROMBASE:
        copy_words(usrloc, extPtr, len);
        break;
    case TOBASE:
        set_dirty_data();
        copy_words(extPtr, usrloc, len);
        break;
    case COMPARE:
    // Check for 0 length is not needed, perhaps because the key part of datum
    // which is being compared, is never empty?
        do {
            if (extPtr[len-1] != usrloc[len-1]) {
                return true;
            }
        } while (--len);
        break;
    case SEEK:
        return false;
    case DONE:
        break;
    }
    usrloc = usrloc + curExtLength;
    return false;
}

// Returns true if skipping of micro-instructions is needed.
// dir == 0: forward, otherwise back.
bool MarsImpl::step(word dir) {
    uint64_t cur = Cursor[idx].d;
    if (!cur)
        throw Mars::ERR_NO_CURR;
    int pos = cur & BITS(15);
    if (!dir.d) {
        // Step forward
        cur += 2;
        pos = cur;
        if ((cur & 01777) == block_len(curMetaBlock[-1])) {
            auto next = next_block(curMetaBlock[-1]);
            if (!next) {
                cont = skip(Mars::ERR_NO_NEXT);
                return true;
            }
            get_secondary_block(next);
            Cursor[idx-2] = Cursor[idx-2] + (1<<15);
            pos = 0;
        }
    } else {
        // Step back
        if (!pos) {
            auto prev = prev_block(curMetaBlock[-1]);
            if (!prev) {
                cont = skip(Mars::ERR_NO_PREV);
                return true;
            }
            get_secondary_block(prev);
            Cursor[idx-2] = Cursor[idx-2] - (1<<15);
            pos = block_len(Secondary[0]);
        }
        pos = pos - 2;
    }
    Cursor[idx] = pos | ONEBIT(31);
    curkey = curMetaBlock[pos];
    adescr = curMetaBlock[pos+1];
    return false;
}

bool MarsImpl::access_data(Ops op, word arg) {
    bool done;
    unsigned tail = arg.d;
    curlen = curlen + arg;
    auto usrloc = myloc;
    for (;;) {
        done = true;
        if (perform(op, done, tail, usrloc)) {
            // COMPARE failed, skip an instruction
            cont = curcmd >> 12;
            return true;
        }
        if (done) {
            extPtr = extPtr + tail;
            curExtLength = curExtLength - tail;
            // Side effect: SEEK returns the word at offset in desc1
            desc1 = *extPtr;
        } else {
            auto next = next_extent(curExtent);
            if (next) {
                find_item(next);
                continue;
            }
            if (op == DONE
                // impossible? should compare with SEEK, or deliberate as a binary patch?
                || tail) {
                cont = skip(Mars::ERR_SEEK);
                return true;
            }
        }
        return false;
    }
}

// Read word at offset 'offset' of the datum 'arg'
uint64_t MarsImpl::get_word(uint64_t arg, uint64_t offset) {
    find_item(arg);
    // Not expecting it to fail
    access_data(SEEK, offset);
    return desc1.d;
}

void MarsImpl::assign_and_incr(uint64_t dst) {
    auto src = (curcmd >> 6) & 077;
    curcmd = curcmd >> 6;
    abdv[dst] = *abdv[src];
    ++abdv[src];
}

// Performs operations on upper bits in myloc
// using value in curlen, assigning to offset and mylen.
// Conditionally skips the next 4 instructions.
// From the code is appears that the max expected
// length of the datum was 017777.
// Untested: usage unknown.
bool MarsImpl::cmd46() {
    auto len = (myloc >> 18) & BITS(15);
    offset = len -= curlen.d;
    len = (myloc >> 33) & 017777;
    if (!len) {
        cont = curcmd >> 30;
        return true;
    }
    mylen = len;
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

// Input: z = zone number + 1
// returns 0 if no more chunks remain
// otherwise z to continue finding
// free space for the remaining data.
int MarsImpl::prepare_chunk(int z, int &remlen, word &usrloc) {
    auto free = freeSpace[z-1].d - 1;
    if (free < mylen.d) {
        remlen = mylen.d - free;
        mylen = free;
        usrloc = usrloc - free;
    } else {
        usrloc = usrloc - mylen + 1;
        z = 0;
    }
    return z;
}

uint64_t MarsImpl::handle_chunk(int zone, word head) {
    get_zone(zone);
    uint64_t id = curbuf[1] >> 10;
    do {
        if (id == get_id(curbuf[id+1]))
            break;
        curbuf[id+2] = curbuf[id+1];
        --id;
    } while (id);
    ++id;
    loc116 = id;
    return (id << 39) | head.d;
}

uint64_t MarsImpl::allocator(word usrloc) {
    return allocator1023(make_extent_header(), usrloc);
}

// The 2 allocator helpers are disambiguated by the original code addresses
// for lack of a better understanding of their semantics.
uint64_t MarsImpl::allocator1023(uint64_t extentHeader, word usrloc) {
    int i = 0;
    auto free = freeSpace[curZone].d;
    int zone, remlen;
    assert(usrloc.mars);
    ++mylen;
    if (mylen.d < free) {
        // The datum fits in the current zone!
        zone = curZone.d;
    } else {
        if (verbose)
            std::cerr << "mylen = " << mylen.d << " free = " << free << '\n';
        // If the datum is larger than MAXCHUNK, it will have to be split
        if (mylen < Mars::MAXCHUNK) {
            // Find a zone with enough free space
            for (size_t i = 0; i < dblen.d; ++i) {
                if (mylen < freeSpace[i]) {
                    return allocator1047(0, handle_chunk(i, 0), extentHeader, 0, usrloc);
                }
            }
        }
        // End reached, or the length is too large: must split
        usrloc = usrloc + mylen - 1;
        i = dblen.d;
        while (freeSpace[i-1] < 2) {
            if (--i)
                continue;
            overflow(0);
        }
        zone = i - 1;
        i = prepare_chunk(i, remlen, usrloc);
    }
    return allocator1047(i, handle_chunk(zone, 0), extentHeader, remlen, usrloc);
}

uint64_t MarsImpl::allocator1047(int loc, uint64_t head, uint64_t extentHeader, int remlen, word &usrloc) {
    uint64_t newDescr;
    for (;;) {               // head has the extent id in bits 48-40
        newDescr = (get_id(head) << 10) | curZone.d;
        int len = mylen.d;
        // Correcting the free space pointer and incrementing extent count
        curbuf[1] = curbuf[1] - mylen + 02000;
        int destLoc = curbuf[1].d & 01777;
        word dest(mars, curbuf.d + destLoc + 1);
        if (!loc) {
            --len;
            ++dest;
        }
        if (len) {
            if (verbose)
                std::cerr << "To DB: ";
            copy_words(dest, usrloc, len);
        }
        curbuf[loc116+1] = (mylen.d << 10) | destLoc | head;
        if (verbose)
          std::cerr << std::format("Reducing free {:o} by len {:o} + 1\n",
                                   freeSpace[curZone].d, mylen.d);
        freeSpace[curZone] = freeSpace[curZone] - (mylen+1);
        if (verbose)
          std::cerr << std::format("Got {:o}\n", freeSpace[curZone].d);
        if (!loc) {
            dest[-1] = extentHeader;
            set_dirty_both();
            return newDescr;
        }
        set_dirty_data();
        head = newDescr << 20;
        mylen = remlen;
        if (--loc) {
            while (freeSpace[loc-1] < 2) {
                if (--loc)
                    continue;
                overflow(head);
            }
            int zone = loc - 1;
            loc = prepare_chunk(loc, remlen, usrloc);
            head = handle_chunk(zone, head);
            continue;
        }
        overflow(head);       // will throw
    }
}

// There was no check for dblen == 0
void MarsImpl::mkctl() {
    curZone = 0;
    int nz = dblen = dbdesc >> 18;
    IOpat = dbdesc & 0777777;
    curbuf = bdtab;
    uint64_t writeWord = IOpat.d | bdtab.d << 20;
    bdtab[1] = 01777;           // last free location in zone
    do {
        --nz;
        bdbuf[nz] = 01776;      // free words in zone
        bdtab[0] = DBkey | nz;  // zone id
        IOcall(writeWord + nz);
    } while (nz);
    myloc = freeSpace = bdbuf; // freeSpace[0] is now the same as bdbuf[0]
    Cursor[-1] = ROOT_METABLOCK;
    allocator(make_metablock(0));
    mylen = dblen;              // length of the free space array
    // This trick results in max possible DB length = 753 zones.
    // bdbuf[0] = 01731 - mylen.d;
    // Invoking OP_INSERT for the freeSpace array
    curDescr = allocator(myloc);
    bdtab[01736-dblen.d] = 01731 - dblen.d; // This is the right way
}

void MarsImpl::update_by_reallocation(int pos, uint64_t extentHeader, word &usrloc) {
    word old = curExtent.d;
    curExtent = curExtent & 01777;
    free_from_current_extent(pos);
    ++mylen;
    /* newDescr = */ allocator1047(0, get_id(old) << 39, extentHeader, 0, usrloc);
    auto next = next_extent(old);
    if (next) {
        free(next);
    } else {
        set_dirty_tab();
    }
}

// Looked like an off-by-one bug; however, in the binary code,
// the procedure was not functional at all.
// The loop enclosed the whole body of the procedure, so it would not terminate.
// Likely intended for completeness along with find_end_mark() but never used.
void MarsImpl::find_end_word() {
    int i;
    i = -mylen.d;
    word limit(mars, myloc.d + mylen.d);
    do {
        if (limit[i] == endmrk) {
            mylen = i + mylen.d + 1;
            return;
        }
    } while (++i != 0); // "i++" in the binary
    throw Mars::ERR_INTERNAL;
}

// base points to the payload of a metadata block
void MarsImpl::search_in_block(word *base, word what) {
    bool parent = base[1].d & INDIRECT;
    int i = block_len(base[-1]);
    if (verbose)
        std::cerr << "Comparing " << std::dec << i/2 << " elements\n";
    while (i) {
        if (base[i-2].d <= what.d)
            break;
        i -= 2;
    }
    i -= 2;
    Cursor[idx] = i | ONEBIT(31);
    if (!parent) {
        curkey = curMetaBlock[i];
        adescr = curMetaBlock[i+1];
        return;
    }
    idx = idx + 2;
    if (idx > 7) {
        std::cerr << "Idx = " << idx << ": DB will be corrupted\n";
    }
    get_secondary_block(base[i+1]);
    search_in_block(Secondary+1, what);
}

void MarsImpl::find(word k) {
    idx = 0;
    if (!Cursor[-1].d) {
        // There was an overflow, re-reading is needed
        get_root_block();
    }
    search_in_block(RootBlock+1, k);
}

void MarsImpl::update(word arg, word usrloc) {
    word saved(arg);
    int len = find_item(arg.d) - 1;
    assert(usrloc.mars);
    if (mylen == len) {
        // The new length of data matches the first extent length
        if (len != 0) {
            do {
                extPtr[len] = usrloc[len-1];
            } while (--len);
        }
        *extPtr = make_extent_header();
        set_dirty_data();
        if (next_extent(curExtent)) {
            curbuf[loc116+1] = curExtent & ~(BITS(19) << 20); // dropping the extent chain
            free(next_extent(curExtent)); // Free remaining extents
            set_dirty_tab();
        }
        return;
    }
    auto extentHeader = make_extent_header();
    acc = curbuf[1] >> 10;
    int pos = (acc + 1) & BITS(15);
    // Compute the max extent length to avoid fragmentation
    acc = (curbuf[1].d - acc) & 01777;
    acc += curExtLength.d - 2;
    auto curLen = acc;
    if (acc >= mylen.d) {
        // The new length will fit in one zone,
        // reallocation is guaranteed to succeed
        update_by_reallocation(pos, extentHeader, usrloc);
        set_dirty_tab();
        return;
    }
    if ((*extPtr & BITS(15)) + usable_space() < mylen) {
        throw Mars::ERR_OVERFLOW;
    }
    // Put as much as possible in the current zone
    mylen = curLen;
    update_by_reallocation(pos, extentHeader, usrloc);
    usrloc = usrloc + mylen;
    mylen = (extentHeader & 077777) - mylen.d;
    auto newDescr = allocator1023(usrloc[-1].d, usrloc);
    find_item(saved.d);
    // Chain the first extent and the tail
    curbuf[loc116+1] = (newDescr << 20) | curExtent.d;
    set_dirty_both();
}

Error MarsImpl::eval() try {
    if (bdtab[0] != DBkey && IOpat.d) {
        IOcall(bdtab.d << 20 | ONEBIT(40) | IOpat.d);
    }
    // enter2:                       // continue execution after a callback?
    uint64_t next = orgcmd.d;
    do {
        curcmd = next;
        next = one_insn();
    } while (next);
    finalize(0);
    if (mars.dump_diffs)
        mars.dump();
    return Mars::ERR_SUCCESS;
} catch (Error e) {
    if (e != Mars::ERR_SUCCESS) {
        if (erhndl.d) {
            // returning to erhndl instead of the point of call
        }
        data[7] = e;            // imitating M7 = e
        std::cerr << std::format("ERROR {} ({})\n", int(e), msg[e-1]);
        data[BDSYS+010] = *(uint64_t*)msg[e-1];
        finalize(*((uint64_t*)msg[e-1]+1));
    }
    if (mars.dump_diffs)
        mars.dump();
    return e;
}

// Returns the updated instruction word
uint64_t MarsImpl::one_insn() {
    // The switch is entered with acc = 0
    if (verbose)
        std::cerr << std::format("Executing microcode {:02o}\n", curcmd.d & 077);
    acc = 0;
    switch (curcmd.d & 077) {
    case 0:
        break;
    case Mars::OP_BEGIN:
        find(0);
        break;
    case Mars::OP_LAST:
        find(BITS(47));
        break;
    case Mars::OP_PREV:
        if (step(1))
            return cont;
        break;
    case Mars::OP_NEXT:
        if (step(0))
            return cont;
        break;
    case Mars::OP_INSMETA:      // not yet covered by tests
        curDescr = allocator(make_metablock(0));
        break;
    case Mars::OP_SETMETA:
        idx = 0;
        datumLen = META_SIZE;
        get_block(adescr, RootBlock-1);
        break;
    case Mars::OP_SEEK:               // also reads word at offset 'offset'
        access_data(SEEK, offset);    // of the current datum into 'desc1'
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
        datumLen = usable_space();
        break;
    case Mars::OP_MATCH:
        if (curkey == key)
            break;
        return skip(Mars::ERR_NO_NAME);
    case Mars::OP_NOMATCH:
        if (curkey != key)
            break;
        return skip(Mars::ERR_EXISTS);
    case Mars::OP_STRLEN:
        find_end_mark();
        break;
    case Mars::OP_WORDLEN:
        find_end_word();
        break;
    case Mars::OP_UPDATE:
        update(adescr, myloc);
        break;
    case Mars::OP_INSERT:
        curDescr = allocator(myloc);
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
        adescr = acc = curDescr.d;
        return key_manager(ADDKEY, key.d);
    case Mars::OP_DELKEY:
        newkey = 0;              // looks dead
        return key_manager(DELKEY);
    case Mars::OP_LOOP:
        return orgcmd.d;
    case Mars::OP_ROOT:
        DBkey = ROOTKEY;
        dbdesc = arch;
        setctl(dbdesc.d);
        break;
    case 032:
        *extPtr = curDescr;
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
        throw Mars::ERR_SUCCESS;
    case Mars::OP_ADDMETA:
        curMetaBlock[Cursor[idx]+1] = curDescr;
        return key_manager(A01160);
    case Mars::OP_SKIP:
        return curcmd >> 12;
    case Mars::OP_STOP:
        return 0;
    case Mars::OP_IFEQ:
        if (access_data(COMPARE, mylen))
            return cont;
        break;
    case Mars::OP_WRITE:
        if (access_data(TOBASE, mylen))
            return cont;
        break;
    case Mars::OP_READ:
        if(access_data(FROMBASE, mylen))
            return cont;
        break;
    case 044:                   // macro for OP_ASSIGN 35 41
        adescr = desc1;
        break;
    case Mars::OP_LOCK:
        lock();
        break;
    case 046:
        if (cmd46())
            return cont;
        break;
    case Mars::OP_CALL:
        acc = desc1.d;
        // Then an indirect jump to outadr;
        // expected to return to enter2?
        break;
    case Mars::OP_CHAIN:
        // cmd := mem[bdvect[src]++]
        // curcmd is ..... src 50
        assign_and_incr(&orgcmd - &data[BDVECT]);
        return orgcmd.d;
    case 051:                   // macro for ... src 13 52
        // myloc := mem[bdvect[src]++]
        // curcmd is ..... src 51
        assign_and_incr(&myloc - &data[BDVECT]);
        break;
    case 052: {
        // bdvect[dst] := mem[bdvect[src]++]
        // curcmd is ..... src dst 52
        int dst = (curcmd >> 6) & 077;
        curcmd = curcmd >> 6;
        assign_and_incr(dst);
    } break;
    case Mars::OP_ASSIGN: {
        // bdvect[dst] = bdvect[src]
        // curcmd is ..... src dst 53
        int dst = (curcmd >> 6) & 077;
        int src = (curcmd >> 12) & 077;
        curcmd = curcmd >> 12;
        abdv[dst] = abdv[src];
    } break;
    case 054: {
        // mem[bdvect[dst]] = bdvect[012]
        // curcmd is ..... dst 54
        int dst = (curcmd >> 6) & 077;
        curcmd = curcmd >> 6;
        *abdv[dst] = curDescr;
    } break;
    case Mars::OP_EXIT:
        throw Mars::ERR_SUCCESS;
    default:
        // In the original binary, loss of control ensued.
        std::cerr << std::format("Invalid micro-operation {:o} encountered\n",
                                 curcmd.d & 077);
        abort();
    }
    return curcmd >> 6;
}

void MarsImpl::check_space(unsigned need) {
    if (usable_space() < need) {
        Cursor[-1] = 0;
        free(curDescr.d);
        throw Mars::ERR_OVERFLOW;
    }
}

// Performs key insertion and deletion in the BTree
uint64_t MarsImpl::key_manager(KeyOp op, uint64_t key) {
    enum { LOC_NONE, LOC_DELKEY, LOC_A00731, LOC_ADDKEY } dest = LOC_NONE;
    bool recurse = false;
    int len;
    unsigned need;
    word chain(mars, 0), newHeader(mars, 0);
    uint64_t newDescr;
    switch (op) {
    case ADDKEY: jump(addkey);
    case DELKEY: jump(delkey);
    case A01160: jump(a01160);
    }
  delkey:
    // Was loc116 = curMetaBlock ^ (curMetaBlock + Cursor[idx])
    // using a 15-bit register to compute, which is effectively as good as ...
    loc116 = (Cursor[idx].d & 077777) ? ARBITRARY_NONZERO : 0;
    len = block_len(curMetaBlock[-1]);
    if (len < 2)
        std::cerr << "len @ delkey < 2\n";
    // work = curMetaBlock + len;
    // Removing the element pointed by the iterator from the metablock
    for (size_t i = block_len(Cursor[idx]); i < block_len(curMetaBlock[-1]); ++i) {
        curMetaBlock[i] = curMetaBlock[i+2];
    }
    curMetaBlock[-1] = curMetaBlock[-1] - 2;
    if (block_len(curMetaBlock[-1]) == 0) {
        // The current metablock became empty
        if (!idx)               // That was the root metablock?
            jump(a01160);       // Do not free it
        free(curBlockDescr.d);
        chain = Secondary[0];  // Save the block chain
        newHeader = make_block_header(prev_block(Secondary[0]), 0, 0);
        acc = prev_block(Secondary[0]);
        if (acc) {
            acc = get_word(acc, 1);  // Read the previous block if it exists ?
            Secondary[0] = make_block_header(prev_block(acc), 0, block_len(acc));
            // Exclude the deleted block from the chain
            set_header(make_block_header(prev_block(acc), next_block(chain), block_len(acc)));
        }
        dest = LOC_DELKEY;
        jump(pr1232);           // Do something and go to delkey again
    }
    acc = loc116.d;
    while (true) {
        acc &= 077777;
        if (!acc) {
            key = curMetaBlock[0].d;
            recurse = true;
            dest = LOC_A00731;
        }
        jump(a01160);
      a00731:
        curMetaBlock[Cursor[idx]] = key;
        acc = Cursor[idx].d;
    }
  addkey:
    newkey = (curMetaBlock + Cursor[idx] + 2) & 077777;
    if (verbose) {
        size_t total = block_len(curMetaBlock[-1]);
        size_t downto = block_len(Cursor[idx]) + 2;
        std::cerr << std::format("Expanding {} elements\n", (total-downto)/2);
    }
    for (size_t i = block_len(curMetaBlock[-1]); i != block_len(Cursor[idx]) + 2; i -= 2) {
        curMetaBlock[i] = curMetaBlock[i-2];
        curMetaBlock[i+1] = curMetaBlock[i-1];
    }
    newkey[0] = key;
    newkey[1] = acc;
    curMetaBlock[-1] = curMetaBlock[-1] + 2;
  a01160:                       // generic metablock update ???
    if (idx == 0) {
        mylen = META_SIZE;
        if (curMetaBlock[-1] == META_SIZE-1) {
            // The root metablock is full
            check_space(0101);
            // Copy it somewhere and make a new root metablock
            // With the 0 key pointing to the newly made block
            RootBlock[2] = allocator(curMetaBlock - 1) | INDIRECT;
            RootBlock[0] = 2;
            mylen = META_SIZE;
        }
        update(Cursor[-1], curMetaBlock - 1); // update the root metablock
        if (recurse) {
            recurse = false; jump(a01242);
        }
        return curcmd >> 6;
    }
    if (block_len(curMetaBlock[-1]) != 0100) {
        // The current metablock has not reached the max allowed length
        mylen = block_len(curMetaBlock[-1]) + 1;
        update(curBlockDescr, curMetaBlock - 1);
        if (recurse) {
            recurse = false; jump(a01242);
        }
        return curcmd >> 6;
    }
    need = (idx * 2) + META_SIZE;
    if (RootBlock[0] == 036) {
        // The current metadata block is full, account for another one
        need = need + 044;
    }
    check_space(need);
    // Split the block in two
    chain = Secondary[040];
    Secondary[040] = make_block_header(curBlockDescr.d, next_block(Secondary[0]), 040);
    mylen = META_SIZE;
    newDescr = allocator(curMetaBlock - 1 + 040);
    Secondary[040] = chain;
    key = Secondary[META_SIZE].d;
    chain = Secondary[0];
    Secondary[0] = make_block_header(prev_block(chain), newDescr, 040);
    newHeader = make_block_header(newDescr, 0, 0);
    mylen = block_len(Secondary[0]) + 1;
    update(curBlockDescr, word(mars, Secondary - data));
    dest = LOC_ADDKEY;
    // pr1232 can return in 3 ways:
    // - continue execution (following dest)
    // - terminating execution of the instruction (returning curcmd >> 6)
    // - conditionaly skipping micro-instructions afer step() (returning cont)
  pr1232:
    if (auto next = next_block(chain)) {
        set_header(newHeader | (get_word(next, 1) & BITS(29)));
    }
  a01242:
    curMetaBlock = RootBlock+1;
    if (idx == 0) {
        set_dirty_tab();
        return curcmd >> 6;
    }
    idx = idx - 2;
    if (idx != 0) {
        get_secondary_block(Cursor[idx-1]);
    }
    acc = Cursor[idx] >> 15;
    for (;;) {
        uint64_t stepCnt = acc;
        acc >>= 15;
        int i = acc & BITS(15);
        if (!(stepCnt & 077777)) {
            switch (dest) {
            case LOC_ADDKEY:
                acc = (newHeader >> 29) | INDIRECT;
                if (verbose)
                    std::cerr << "Jump target addkey\n";
                jump(addkey);
            case LOC_DELKEY:
                if (verbose)
                    std::cerr << "Jump target delkey\n";
                jump(delkey);
            case LOC_A00731:
                if (verbose)
                    std::cerr << "Jump target a00731\n";
                jump(a00731);
            default:
                abort();
            }
        }
        // The following code is triggered, e.g. by using DELKEY
        // after stepping backwards or forwards.
        acc = stepCnt;
        if (--i) {
            acc += 2;
        }
        stepCnt = acc - 1;
        if (step(i))
            return cont;         // never happens?
        acc = stepCnt;
    }
}

void MarsImpl::setup() {
    for (size_t i = 0; i < Mars::RAM_LENGTH; ++i) {
        new (data+i) word(mars, 0);
    }
    // Faking MARS words
    bdtab = Mars::BDTAB;
    bdbuf = Mars::BDBUF;
    abdv = BDVECT;
    for (int i = 0; i < META_SIZE; ++i)
        data[FAKEBLK+i].d = ARBITRARY_NONZERO;
}

static int to_lnuzzzz(int lun, int start, int len) {
    if (lun > 077 || start > 01777 || len > 01731)
        std::cerr << std::oct << lun << ' ' << start << ' ' << len
                  << "out of valid range, truncated\n";
    len = std::min(len, 01731);
    return ((lun & 077) << 12) | (start & 01777) | (len << 18);
}

Error Mars::SetDB(int lun, int start, int len) {
    impl.setup();
    impl.arch = to_lnuzzzz(lun, start, len);
    return root();
}

Error Mars::InitDB(int lun, int start, int len) {
    impl.setup();
    impl.dbdesc = to_lnuzzzz(lun, start, len);
    impl.DBkey = ROOTKEY;
    impl.orgcmd = OP_INIT;
    return impl.eval();
}

// A cleaned-up version of the original NEWD operation in the BESM-6 Pascal library
Error Mars::newd(const char * k, int lun, int start, int len) {
    int lnuzzzz = to_lnuzzzz(lun, start, len);
    if (verbose)
        std::cerr << std::format("Running newd('{}', {:o})\n", k, lnuzzzz);
    impl.key = *reinterpret_cast<const uint64_t*>(k);
    data[077776] = lnuzzzz;
    data[077777] = (impl.key.d << 10) & BITS(48);
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
