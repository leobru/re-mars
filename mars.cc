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

#define ONEBIT(n) (1ULL << ((n)-1))   // one bit set, from 1 to 48
#define BITS(n)   (~0ULL >> (64 - n)) // bitmask of n bits from LSB
// #define jump(x) do { std::cerr << "Jump to " #x "\n"; goto x; } while(0)
#define jump(x) goto x

struct Page {
    Page() { }
    Page(uint64_t * page) {
        for (int i = 0; i < 1024; ++i)
            w[i] = page[i];
    }
    void to_memory(uint64_t * page) {
        for (int i = 0; i < 1024; ++i)
            page[i] = w[i];
    }
    uint64_t w[1024];
};

struct MarsImpl {
    static const int BDVECT = Mars::BDVECT;
    static const int BDSYS = Mars::BDSYS;
    typedef uint64_t &uintref;
    typedef uint64_t* &puintref;

    struct CursorElt {
        uint64_t block_id;
        unsigned pos: 15;
        unsigned steps: 16;
    };

    union Handle {
        uint64_t word;
        uint64_t full: 19;
        struct { uint64_t zone: 10, ext: 9; };
    };
    struct BlockElt {
        uint64_t key;
        union {
            Handle handle;
            struct {
                uint64_t id: 47;
                bool indirect:1;
            };
        };
    };
    struct Metablock {
        union Header {
            uint64_t word;
            struct { uint64_t len: 10, next: 19, prev: 19; };
            Header() = default;
            Header(int p, int n, int l) : len(l), next(n), prev(p) { }
        } header;
        BlockElt element[];
    };

    union ExtentHeader {
        uint64_t word;
        struct { uint64_t len: 15, unknown: 18, timestamp: 15; };
    };

    union Extent {
        uint64_t word;
        struct { uint64_t start: 10, len: 10, next: 19, id: 9; };
    };

    Mars & mars;
    word * const data;
    bool & verbose;

    // Fields of BDVECT
    // uintref outadr;
    puintref bdbuf, bdtab, freeSpace, newkey, extPtr, curbuf;
    Metablock * &curMetaBlock;
    uintref orgcmd, curcmd,
        disableSync, givenp, key, erhndl,
        allocHandle,
        myloc, loc14, mylen,
        dbdesc, DBkey, savedp,
        workHandle,
        curkey,
        endmrk,
        desc1, offset,
        IOpat,
        curExtLength, datumLen, dblen, curPos,
        handlePtr,
        idx, curZone,
        dirty;

    Extent & curExtent;
    Handle & curBlockDescr;

    uint64_t cont;              // continuation instruction word
    CursorElt * const Cursor;
    uint64_t * const RootBlock;
    uint64_t * const Secondary;

    // Fields of BDSYS
    uint64_t & arch;
    puintref abdv;
    std::unordered_map<std::string, Page> DiskImage;

    MarsImpl(Mars & up) :
        mars(up), data(up.data), verbose(up.verbose),

        // outadr(data[BDVECT+1].u),

        bdbuf(data[BDVECT+016].u),
        bdtab(data[BDVECT+017].u),
        freeSpace(data[BDVECT+034].u),
        newkey(data[BDVECT+036].u),
        extPtr(data[BDVECT+046].u),
        curbuf(data[BDVECT+0241].u),

        curMetaBlock(reinterpret_cast<Metablock*&>(data[BDVECT+050].u)),

        orgcmd(data[BDVECT+3].d),
        curcmd(data[BDVECT+5].d),
        disableSync(data[BDVECT+6].d),
        givenp(data[BDVECT+7].d),
        key(data[BDVECT+010].d),
        erhndl(data[BDVECT+011].d),
        allocHandle(data[BDVECT+012].d),
        myloc(data[BDVECT+013].d),
        loc14(data[BDVECT+014].d),
        mylen(data[BDVECT+015].d),
        dbdesc(data[BDVECT+030].d),
        DBkey(data[BDVECT+031].d),
        savedp(data[BDVECT+032].d),
        workHandle(data[BDVECT+035].d),
        curkey(data[BDVECT+037].d),
        endmrk(data[BDVECT+040].d),
        desc1(data[BDVECT+041].d),
        offset(data[BDVECT+042].d),
        IOpat(data[BDVECT+044].d),
        curExtLength(data[BDVECT+045].d),
        datumLen(data[BDVECT+047].d),
        dblen(data[BDVECT+051].d),
        curPos(data[BDVECT+053].d),
        handlePtr(data[BDVECT+0116].d),
        idx(data[BDVECT+0242].d),
        curZone(data[BDVECT+0244].d),
        dirty(data[BDVECT+0247].d),

        curExtent(reinterpret_cast<Extent&>(data[BDVECT+0220].d)),
        curBlockDescr(reinterpret_cast<Handle&>(data[BDVECT+0246].d)),

        Cursor(reinterpret_cast<CursorElt*>(&data[BDVECT+020].d)),
        // RootBlock[-1] (loc54) is also used
        RootBlock(&data[BDVECT+055].d), // spans 041 words, up to 0115
        // Secondary[-1] handlePtr is also used
        Secondary(&data[BDVECT+0117].d), // spans 0101 words, up to 0217

        // Fields of BDSYS
        arch(data[BDSYS+4].d),
        abdv(data[BDSYS+3].u)
        { }
    void IOflush();
    void IOcall(uint64_t, uint64_t *);
    void get_zone(uint64_t);
    void save(bool);
    void finalize(uint64_t);
    ExtentHeader make_extent_header();
    uint64_t* make_metablock(uint64_t key);
    uint64_t usable_space();
    uint64_t find_item(uint64_t);
    void info(uint64_t);
    void totext();
    void set_header(uint64_t);
    void copy_words(uint64_t* dst, uint64_t* src, int len);
    void copy_chained(int len, uint64_t* &usrloc);
    void cpyout(uint64_t);
    void lock();
    void get_block(uint64_t, uint64_t*), get_root_block(), get_secondary_block(uint64_t);
    void free_from_current_extent(int);
    Metablock::Header get_block_header(uint64_t arg);
    uint64_t skip(Error);
    void setctl(uint64_t);
    void free_extent(int), free(uint64_t);
    void find_end_mark(), find_end_word();
    enum Ops {
        FROMBASE, TOBASE, COMPARE, SEEK, DONE
    };
    bool perform(Ops, bool &, unsigned &, uint64_t* &);
    bool step(uint64_t), access_data(Ops, uint64_t);
    bool unpack();
    void assign_and_incr(uint64_t);
    void setup();
    Error eval();
    uint64_t one_insn();
    void overflow(uint64_t);
    int prepare_chunk(int, int &remlen, uint64_t* &usrloc);
    uint64_t handle_chunk(int zone, uint64_t head);
    uint64_t allocator(uint64_t *usrloc), allocator1023(uint64_t firstWord, uint64_t *usrloc);
    uint64_t allocator1047(int loc, uint64_t head, uint64_t firstWord, int remlen, uint64_t* &usrloc);
    enum KeyOp { ADDKEY, DELKEY, A01160 };
    uint64_t key_manager(KeyOp, uint64_t key = ARBITRARY_NONZERO);
    bool pr12x2(bool withHeader, uint64_t chain, uint64_t newHeader);
    void check_space(unsigned need);
    void mkctl(), find(uint64_t);
    void update_by_reallocation(int, ExtentHeader, uint64_t* &usrloc);
    void search_in_block(Metablock*, uint64_t);
    void update(uint64_t, uint64_t *usrloc);
    void setDirty(int x) {
        dirty |= curZone ? x+1 : 1;
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
        dirty |= 1;
    }
    bool is_dirty_tab() const {
        return dirty & 1;
    }
    bool is_dirty_buf() {
        return dirty & 2;
    }

};

Mars::Mars() : impl(*new MarsImpl(*this)) { }
Mars::Mars(bool persistent) : impl(*new MarsImpl(*this)), flush(persistent) { }

Mars::~Mars() {
    if (flush) {
        impl.IOflush();
    }
    delete &impl;
}

uint64_t prev_block(uint64_t x) {
    MarsImpl::Metablock::Header h;
    h.word = x;
    return h.prev;
}

uint64_t next_block(uint64_t x) {
    MarsImpl::Metablock::Header h;
    h.word = x;
    return h.next;
}

uint64_t block_len(uint64_t x) {
    MarsImpl::Metablock::Header h;
    h.word = x;
    return h.len;
}

inline MarsImpl::Metablock::Header make_block_header(uint64_t prev, uint64_t next, uint64_t len) {
    MarsImpl::Metablock::Header ret;
    ret.prev = prev;
    ret.next = next;
    ret.len = len;
    return ret;
}

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

void MarsImpl::IOcall(uint64_t op, uint64_t *buf) {
    std::string nuzzzz;
    bool stores = mars.trace_stores;
    mars.trace_stores = false;
    nuzzzz = std::format("{:06o}", op & BITS(18));
    if (op & ONEBIT(40)) {
        // read
        auto it = DiskImage.find(nuzzzz);
        if (verbose)
            std::cerr << std::format("Reading {} to address {:o}\n", nuzzzz, buf - &data[0].d);
        if (it == DiskImage.end()) {
            std::ifstream f(nuzzzz);
            if (!f) {
                std::cerr << "\tZone " << nuzzzz << " does not exist yet\n";
                for (auto p = buf; p < buf+1024; ++p)
                    *p = ARBITRARY_NONZERO;
                mars.trace_stores = stores;
                return;
            }
            if (verbose)
                std::cerr << "\tFirst time - reading from disk\n";
            f.read(reinterpret_cast<char*>(&DiskImage[nuzzzz]), sizeof(Page));
            DiskImage[nuzzzz].to_memory(buf);
        } else
            it->second.to_memory(buf);
    } else {
        // write
        if (verbose)
            std::cerr << std::format("Writing {} from address {:o}\n", nuzzzz, buf - &data[0].d);
        DiskImage[nuzzzz] = buf;
    }
    mars.trace_stores = stores;
}

// Takes the zone number as the argument,
// returns the pointer to the zone read in curbuf
void MarsImpl::get_zone(uint64_t arg) {
    curZone = arg;
    uint64_t zoneKey = arg | DBkey;
    curbuf = (arg & 01777) ? bdbuf : bdtab;
    if (curbuf[0] == zoneKey)
        return;
    if (curZone && is_dirty_buf()) {
        dirty &= ~2;
        IOcall(IOpat + (curbuf[0] & 01777), bdbuf);
    }
    IOcall((curZone | ONEBIT(40)) + IOpat, curbuf);
    if (curbuf[0] == zoneKey)
        return;
    std::cerr << std::format("Corruption: zoneKey = {:o}, data = {:o}\n", zoneKey, curbuf[0]);
    throw Mars::ERR_BAD_PAGE;
}

void MarsImpl::save(bool force_tab = false) {
    if (force_tab || is_dirty_tab()) {
        IOcall(IOpat, bdtab);
    }
    if (is_dirty_buf()) {
        IOcall(IOpat + (bdbuf[0] & 01777), bdbuf);
    }
    dirty = 0;
}

// The argument is nonzero (a part of the error message) if there is an error
void MarsImpl::finalize(uint64_t err) {
    data[BDSYS+011] = err;
    bool force_tab = false;
    if (disableSync != 0) {
        if (disableSync != 1)
            return;
        if (bdtab[1] & LOCKKEY) {
            bdtab[1] = bdtab[1] & ~LOCKKEY;
            force_tab = true;
        }
    }
    save(force_tab);
}

// Returns the constructed extent header
auto MarsImpl::make_extent_header() -> ExtentHeader {
    using namespace std::chrono;
    const year_month_day ymd{floor<days>(system_clock::now())};
    unsigned d{ymd.day()}, m{ymd.month()};
    int y{ymd.year()};

    ExtentHeader ret;
    ret.timestamp = mars.zero_date ? 0 : (d/10*16+d%10) << 9 |
        (m/10*16+m%10) << 4 |
        y % 10;
    // This is the only use of loc14
    // and of this bit range of the extent header structure
    ret.unknown = loc14;
    ret.len = mylen;
    return ret;
}

// Prepares a metadata block, retuirns its address
uint64_t* MarsImpl::make_metablock(uint64_t key) {
    mylen = META_SIZE;
    data[FAKEBLK] = 2;
    data[FAKEBLK+1] = key;
    data[FAKEBLK+2] = Cursor[0].block_id;
    return &data[FAKEBLK].d;
}

// Usable space is one word (extent handle) less than free space
uint64_t MarsImpl::usable_space() {
    uint64_t l = 0;
    for (int i = dblen - 1; i >= 0; --i) {
        if (freeSpace[i] != 0)
            l += freeSpace[i] - 1;
    }
    return l;
}

// Finds item indicated by the zone number in bits 10-1
// and by the number within the zone in bits 19-11
// Sets 'extPtr', also returns the extent length in 'curExtLength'
uint64_t MarsImpl::find_item(uint64_t arg) {
    Handle h;
    h.word = arg;
    get_zone(h.zone);
    if (!h.ext)
        throw Mars::ERR_ZEROKEY;      // Attempting to find a placeholder?
    unsigned id = h.ext;
    uint64_t record = curbuf[1] >> 10;
    if (!record)
        throw Mars::ERR_NO_RECORD;    // Attempting to find a deleted record?
    // Records within the zone may be non-contiguous but sorted;
    // need to search only from the indicated position toward lower addresses.
    if (record >= h.ext)
        record = h.ext;
    handlePtr = record;
    for (;;) {
        curExtent.word = curbuf[handlePtr + 1];
        if (curExtent.id == id) {
            break;
        }
        if (--handlePtr == 0) {
            throw Mars::ERR_NO_RECORD;
        }
    }
    extPtr = curbuf + curExtent.start + 1;
    curExtLength =  curExtent.len;
    return curExtLength;
}

// Assumes that arg points to a datum with the standard header.
// Puts the full header to desc1, and its length to datumLen.
void MarsImpl::info(uint64_t arg) {
    find_item(arg);
    desc1 = *extPtr;
    datumLen = desc1 & 077777;
    curPos = 0;
}

void MarsImpl::totext() {
    uint64_t v = desc1;
    endmrk = Mars::tobesm(std::format("\017{:05o}", v & 077777));
    desc1 = Mars::tobesm(std::format("\017{:c}{:c}{:c}{:c}{:c}",
                                     (v >> 46) & 3, (v >> 42) & 15,
                                     (v >> 41) & 1, (v >> 37) & 15,
                                     (v >> 33) & 15));
    offset = Mars::tobesm(std::format("\172\033\0\0\0\0"));
}

void MarsImpl::set_header(uint64_t arg) {
    *extPtr = arg;
    set_dirty_data();
}

void MarsImpl::copy_words(uint64_t *dst, uint64_t* src, int len) {
    if (verbose)
        std::cerr << std::format("{:o}(8) words from {:o} to {:o}\n", len, src - &data[0].d, dst - &data[0].d);
    // Using backwards store order to match the original binary for ease of debugging.
    while (len) {
        dst[len-1] = src[len-1];
        --len;
    }
}

// Copies chained extents to user memory (len = length of the first extent)
void MarsImpl::copy_chained(int len, uint64_t* &usrloc) { // a01423
    for (;;) {
        if (len) {
            if (verbose)
                std::cerr << "From DB: ";
            copy_words(usrloc, extPtr, len);
        }
        usrloc = usrloc + len;
        if (!curExtent.next)
            return;
        len = find_item(curExtent.next);
    }
}

void MarsImpl::cpyout(uint64_t descr) {
    info(descr);
    auto usrloc = &data[myloc].d;
    if (mylen != 0 && mylen < datumLen)
        throw Mars::ERR_TOO_LONG;
    // Skip the first word (the header) of the found item
    ++extPtr;
    --curExtLength;
    copy_chained(curExtLength, usrloc);
}

// Not really a mutex
void MarsImpl::lock() {
    IOcall(IOpat | ONEBIT(40), bdtab);
    bdtab[1] = bdtab[1] ^ LOCKKEY;
    if (!(bdtab[1] & LOCKKEY))
        throw Mars::ERR_LOCKED;
    bdbuf[0] = LOCKKEY;         // What is the purpose of this?
    IOcall(IOpat, bdtab);
}

// Finds and reads a block of metadata identified by 'descr'
// into memory pointed to by 'dest', if 'curBlockDescr' does
// not have the same ID already.
void MarsImpl::get_block(uint64_t descr, uint64_t* dest) {
    Cursor[idx/2].block_id = descr;
    Handle needed;
    needed.full = descr;
    curMetaBlock = reinterpret_cast<Metablock*>(dest);
    if (needed.full != curBlockDescr.full) {
        curBlockDescr = needed;
        auto ptr = dest - 1;
        copy_chained(find_item(curBlockDescr.full), ptr);
    }
}

// Requests the root block into the main RootBlock array
void MarsImpl::get_root_block() {
    get_block(ROOT_METABLOCK, RootBlock);
}

// Requests a block into the Secondary metadata array
void MarsImpl::get_secondary_block(uint64_t descr) {
    get_block(descr, Secondary);
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
    IOcall(IOpat | ONEBIT(40), bdtab);
    curbuf = bdtab;
    if (bdtab[0] != DBkey)
        throw Mars::ERR_BAD_CATALOG;
    idx = 0;
    curBlockDescr.word = 0;
    curZone = 0;
    freeSpace = bdtab + (bdtab[3] & 01777) + 2;
    get_root_block();
}

// Frees space occupied by curExtent
void MarsImpl::free_extent(int pos) {
    unsigned locIdx = curExtent.start;
    // Correct the data pointers of the extents
    // below the one being freed
    for (;--pos != 0;) {
        Extent &ext = *reinterpret_cast<Extent*>(&curbuf[pos+1]);
        if (ext.start < locIdx) {
            ext.start += curExtLength;
        }
    }
    unsigned freeIdx = curbuf[1] & 01777;
    if (freeIdx != locIdx) {
        if (locIdx < freeIdx)         // extent cannot be located in the free area
            throw Mars::ERR_INTERNAL;
        int diff = locIdx - freeIdx;
        auto freeLoc = curbuf + freeIdx;
        auto loc = freeLoc + curExtLength;
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
        // position of the extent to be freed
        for (int i = handlePtr; i != extCount; ++i) {
            // shrink the extent handle array
            curbuf[i+1] = curbuf[i+2];
        };
        free_extent(extCount);
        arg = curExtent.next;
    } while (arg);
    set_dirty_tab();
}

// When an extent chain must be freed but not the descriptor
// (for OP_UPDATE)
void MarsImpl::free_from_current_extent(int limit) {
    free_extent(limit);
    if (curExtent.next) {
        free(curExtent.next);
    } else {
        set_dirty_tab();
    }
}

void MarsImpl::find_end_mark() {
    // Searching for the end mark (originally 1 or 2 bytes)
    int i = -mylen;
    uint64_t * limit = &data[myloc + mylen].d;
    // Handling only single byte mark, contemporary style
    char * start = (char*)&(limit[i]);
    char * end = (char*)&(limit[0]);
    char * found = (char*)memchr(start, endmrk & 0xFF, end-start);
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
bool MarsImpl::perform(Ops op, bool &done, unsigned &tail, uint64_t* &usrloc) {
    int len = tail;             // data length to operate upon
    if (tail >= curExtLength) {
        // The desired length is too long; adjust it
        tail -= curExtLength;
        len = curExtLength;
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
bool MarsImpl::step(uint64_t dir) {
    CursorElt & cur = Cursor[idx/2];
    if (!cur.steps && !cur.pos)
        throw Mars::ERR_NO_CURR;
    int pos = cur.pos;
    if (!dir) {
        // Step forward
        pos += 2;
        if ((pos & 01777) == curMetaBlock->header.len) {
            auto next = curMetaBlock->header.next;
            if (!next) {
                cont = skip(Mars::ERR_NO_NEXT);
                return true;
            }
            get_secondary_block(next);
            ++Cursor[idx/2-1].steps;
            pos = 0;
        }
    } else {
        // Step back
        if (!pos) {
            auto prev = curMetaBlock->header.prev;
            if (!prev) {
                cont = skip(Mars::ERR_NO_PREV);
                return true;
            }
            get_secondary_block(prev);
            --Cursor[idx/2-1].steps;
            pos = block_len(Secondary[0]);
        }
        pos = pos - 2;
    }
    cur.pos = pos;
    cur.steps = ONEBIT(16);
    curkey = curMetaBlock->element[pos/2].key;
    workHandle = curMetaBlock->element[pos/2].id;
    return false;
}

// For READ and WRITE, arg = data length; for SEEK, arg = skip length
bool MarsImpl::access_data(Ops op, uint64_t arg) {
    bool done;
    unsigned tail = arg;
    curPos = curPos + arg;
    uint64_t * usrloc = &data[myloc & BITS(15)].d;
    for (;;) {
        done = true;
        if (perform(op, done, tail, usrloc)) {
            // COMPARE failed, skip an instruction
            cont = curcmd >> 12;
            return true;
        }
        if (done) {
            extPtr += tail;
            curExtLength -= tail;
            // Side effect: SEEK returns the word at position in desc1
            desc1 = *extPtr;
        } else {
            auto next = curExtent.next;
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

// Reads word at offset 1 of the datum (i. e. the first payload word)
auto MarsImpl::get_block_header(uint64_t arg) -> Metablock::Header {
    Metablock::Header ret;
    find_item(arg);
    // Not expecting it to fail
    access_data(SEEK, 1);
    ret.word = desc1;
    return ret;
}

void MarsImpl::assign_and_incr(uint64_t dst) {
    auto src = (curcmd >> 6) & 077;
    curcmd = curcmd >> 6;
    abdv[dst] = data[abdv[src] & 077777].d;
    ++abdv[src];
}

// Unpacks "myloc" (absolute offset, length, data address)
// into offset to be skipped and mylen.
// To be used with OP_SEEK and OP_READ/OP_WRITE.
// Conditionally skips the next 4 instructions.
// From the code is appears that the max expected
// length of the datum segment was 017777.
bool MarsImpl::unpack() {
    auto seekPos = (myloc >> 18) & BITS(15);
    offset = seekPos - curPos;
    auto len = (myloc >> 33) & 017777;
    if (!len) {
        cont = curcmd >> 30;
        return true;
    }
    mylen = len;
    return false;
}

// Throws overflow after freeing pending extents.
void MarsImpl::overflow(uint64_t word) {
    Extent ext;
    ext.word = word;
    if (ext.next) {
        free(ext.next);
        throw Mars::ERR_OVERFLOW;
    }
    set_dirty_tab();
    throw Mars::ERR_OVERFLOW;
}

// Input: z = zone number + 1
// returns 0 if no more chunks remain
// otherwise z to continue finding
// free space for the remaining data.
int MarsImpl::prepare_chunk(int z, int &remlen, uint64_t* &usrloc) {
    auto free = freeSpace[z-1] - 1;
    if (free < mylen) {
        remlen = mylen - free;
        mylen = free;
        usrloc -= free;
    } else {
        usrloc -= mylen - 1;
        z = 0;
    }
    return z;
}

uint64_t MarsImpl::handle_chunk(int zone, uint64_t head) {
    get_zone(zone);
    uint64_t id = curbuf[1] >> 10;
    do {
        Extent &ext = reinterpret_cast<Extent&>(curbuf[id+1]);
        if (id == ext.id)
            break;
        curbuf[id+2] = curbuf[id+1];
        --id;
    } while (id);
    ++id;
    handlePtr = id;
    return (id << 39) | head;
}

uint64_t MarsImpl::allocator(uint64_t *usrloc) {
    return allocator1023(make_extent_header().word, usrloc);
}

// The 2 allocator helpers are disambiguated by the original code addresses
// for lack of a better understanding of their semantics.
uint64_t MarsImpl::allocator1023(uint64_t firstWord, uint64_t *usrloc) {
    int i = 0;
    auto free = freeSpace[curZone];
    int zone, remlen = 0;
    ++mylen;
    if (mylen < free) {
        // The datum fits in the current zone!
        zone = curZone;
    } else {
        if (verbose)
            std::cerr << "mylen = " << mylen << " free = " << free << '\n';
        // If the datum is larger than MAXCHUNK, it will have to be split
        if (mylen < Mars::MAXCHUNK) {
            // Find a zone with enough free space
            for (size_t i = 0; i < dblen; ++i) {
                if (mylen < freeSpace[i]) {
                    return allocator1047(0, handle_chunk(i, 0), firstWord, 0, usrloc);
                }
            }
        }
        // End reached, or the length is too large: must split
        usrloc += mylen - 1;
        for (i = dblen; freeSpace[i-1] < 2 && --i;);
        if (i == 0)
            overflow(0);
        zone = i - 1;
        i = prepare_chunk(i, remlen, usrloc);
    }
    return allocator1047(i, handle_chunk(zone, 0), firstWord, remlen, usrloc);
}

uint64_t MarsImpl::allocator1047(int loc, uint64_t head, uint64_t firstWord, int remlen, uint64_t* &usrloc) {
    uint64_t newDescr;
    for (;;) {               // head has the extent id in bits 48-40
        Extent & ext = reinterpret_cast<Extent&>(head);
        newDescr = (ext.id << 10) | curZone;
        int len = mylen;
        // Correcting the free space pointer and incrementing extent count
        curbuf[1] = curbuf[1] - mylen + 02000;
        int destLoc = curbuf[1] & 01777;
        auto dest = &curbuf[destLoc+1];
        if (!loc) {
            --len;
            ++dest;
        }
        if (len) {
            if (verbose)
                std::cerr << "To DB: ";
            copy_words(dest, usrloc, len);
        }
        curbuf[handlePtr+1] = (mylen << 10) | destLoc | head;
        if (verbose)
          std::cerr << std::format("Reducing free {:o} by len {:o} + 1\n",
                                   freeSpace[curZone], mylen);
        freeSpace[curZone] -= mylen+1;
        if (verbose)
          std::cerr << std::format("Got {:o}\n", freeSpace[curZone]);
        if (!loc) {
            dest[-1] = firstWord;
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
    uint64_t writeWord = IOpat;
    bdtab[1] = 01777;           // last free location in zone
    do {
        --nz;
        bdbuf[nz] = 01776;      // free words in zone
        bdtab[0] = DBkey | nz;  // zone id
        IOcall(writeWord + nz, bdtab);
    } while (nz);
    freeSpace = bdbuf;     // freeSpace[0] is now the same as bdbuf[0]
    myloc = bdbuf - &data[0].d;
    Cursor[0].block_id = ROOT_METABLOCK;
    allocator(make_metablock(0));
    mylen = dblen;              // length of the free space array
    // This trick results in max possible DB length = 753 zones.
    // bdbuf[0] = 01731 - mylen;
    // Invoking OP_ALLOC for the freeSpace array
    allocHandle = allocator(&data[myloc].d);
    bdtab[01736-dblen] = 01731 - dblen; // This is the right way
}

void MarsImpl::update_by_reallocation(int limit, ExtentHeader extentHeader, uint64_t* &usrloc) {
    auto old = curExtent;
    curExtent.next = 0;
    free_from_current_extent(limit);
    ++mylen;
    allocator1047(0, uint64_t(old.id) << 39, extentHeader.word, 0, usrloc);
    auto next = old.next;
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
    i = -mylen;
    uint64_t * limit = &data[myloc + mylen].d;
    do {
        if (limit[i] == endmrk) {
            mylen = i + mylen + 1;
            return;
        }
    } while (++i != 0); // "i++" in the binary
    throw Mars::ERR_INTERNAL;
}

void MarsImpl::search_in_block(Metablock *block, uint64_t what) {
    bool indirect = block->element[0].indirect;
    int i = block->header.len;
    if (verbose)
        std::cerr << std::format("Comparing {} elements\n", i/2);
    for (; i; i -= 2) {
        if (block->element[i/2-1].key <= what)
            break;
    }
    i -= 2;
    Cursor[idx/2].pos = i;
    Cursor[idx/2].steps = ONEBIT(16);
    if (!indirect) {
        curkey = curMetaBlock->element[i/2].key;
        workHandle = curMetaBlock->element[i/2].id;
        return;
    }
    idx += 2;
    if (idx > 7) {
        std::cerr << "Idx = " << idx << ": DB will be corrupted\n";
    }
    get_secondary_block(block->element[i/2].id);
    search_in_block(reinterpret_cast<Metablock*>(Secondary), what);
}

void MarsImpl::find(uint64_t k) {
    idx = 0;
    if (!Cursor[0].block_id) {
        // There was an overflow, re-reading is needed
        get_root_block();
    }
    search_in_block(reinterpret_cast<Metablock*>(RootBlock), k);
}

void MarsImpl::update(uint64_t arg, uint64_t* usrloc) {
    uint64_t len = find_item(arg) - 1;
    if (mylen == len) {
        // The new length of data matches the first extent length
        for (; len--;)
            extPtr[len+1] = usrloc[len];
        *extPtr = make_extent_header().word;
        set_dirty_data();
        if (auto next = curExtent.next) {
            curExtent.next = 0;
            curbuf[handlePtr+1] = curExtent.word; // dropping the extent chain
            free(next);         // Free remaining extents
            set_dirty_tab();
        }
        return;
    }
    auto extentHeader = make_extent_header();
    auto numExtents = curbuf[1] >> 10;
    // Compute the max extent length to avoid fragmentation
    auto curFree = (curbuf[1] - numExtents) & 01777;
    curFree += curExtLength - 2;
    if (curFree >= mylen) {
        // The new length will fit in one zone,
        // reallocation is guaranteed to succeed
        update_by_reallocation(numExtents + 1, extentHeader, usrloc);
        set_dirty_tab();
        return;
    }
    if ((*extPtr & BITS(15)) + usable_space() < mylen) {
        throw Mars::ERR_OVERFLOW;
    }
    // Put as much as possible in the current zone
    mylen = curFree;
    update_by_reallocation(numExtents + 1, extentHeader, usrloc);
    usrloc += mylen;
    mylen = extentHeader.len - mylen;
    auto newDescr = allocator1023(usrloc[-1], usrloc);
    find_item(arg);
    // Chain the first extent and the tail
    curExtent.next = newDescr;
    curbuf[handlePtr+1] = curExtent.word;
    set_dirty_both();
}

Error MarsImpl::eval() try {
    if (bdtab[0] != DBkey && IOpat) {
        IOcall(ONEBIT(40) | IOpat, bdtab);
    }
    // enter2:                       // continue execution after a callback?
    uint64_t next = orgcmd;
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
        if (erhndl) {
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
    if (verbose)
        std::cerr << std::format("Executing microcode {:02o}\n", curcmd & 077);
    switch (curcmd & 077) {
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
        allocHandle = allocator(make_metablock(0));
        break;
    case Mars::OP_SETMETA:
        idx = 0;
        datumLen = META_SIZE;
        get_block(workHandle, RootBlock);
        break;
    case Mars::OP_SEEK:               // also reads word at the reached position
        access_data(SEEK, offset);    // of the current datum into 'desc1'
        break;
    case Mars::OP_INIT:
        mkctl();
        break;
    case Mars::OP_FIND:
        if (key == 0 || key & ONEBIT(48))
            throw Mars::ERR_INV_NAME;
        find(key);
        break;
    case Mars::OP_SETCTL:
        myloc = BDVECT+030;     // &dbdesc
        mylen = 3;              // accounting for a possible password
        cpyout(workHandle);
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
        update(workHandle, &data[myloc].d);
        break;
    case Mars::OP_ALLOC:
        allocHandle = allocator(&data[myloc].d);
        break;
    case Mars::OP_GET:
        cpyout(workHandle);
        break;
    case Mars::OP_FREE:
        free(workHandle);
        break;
    case Mars::OP_PASSWD:
        if (givenp != savedp)
            throw Mars::ERR_WRONG_PASSWORD;
        break;
    case Mars::OP_OPEN:
        setctl(dbdesc);
        break;
    case Mars::OP_ADDKEY:
        workHandle = allocHandle;
        return key_manager(ADDKEY, key);
    case Mars::OP_DELKEY:
        newkey = 0;              // looks dead
        return key_manager(DELKEY);
    case Mars::OP_LOOP:
        return orgcmd;
    case Mars::OP_ROOT:
        DBkey = ROOTKEY;
        dbdesc = arch;
        setctl(dbdesc);
        break;
    case Mars::OP_INSERT:
        *extPtr = allocHandle;
        set_dirty_both();
        break;
    case Mars::OP_LENGTH:
        info(workHandle);
        break;
    case Mars::OP_DESCR:
        totext();
        break;
    case Mars::OP_SAVE:
        save();
        throw Mars::ERR_SUCCESS;
    case Mars::OP_ADDMETA:
        curMetaBlock->element[Cursor[idx/2].pos/2].id = allocHandle;
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
    case  Mars::OP_USE:         // macro for OP_ASSIGN 35 41
        workHandle = desc1;
        break;
    case Mars::OP_LOCK:
        lock();
        break;
    case Mars::OP_UNPACK:
        if (unpack())
            return cont;
        break;
    case Mars::OP_CALL:
        // acc = desc1.d;
        // Then an indirect jump to outadr;
        // expected to return to enter2?
        break;
    case Mars::OP_CHAIN:
        // orgcmd := mem[bdvect[src]++]
        // curcmd is ..... src 50
        assign_and_incr(3);     // offset of orgcmd within BDVECT
        return orgcmd;
    case Mars::OP_SEGMENT:      // macro for ... src 13 52
        // myloc := mem[bdvect[src]++]
        // curcmd is ..... src 51
        assign_and_incr(013);   // offset of myloc within BDVECT
        break;
    case Mars::OP_LDNEXT: {
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
    case Mars::OP_STALLOC: {
        // mem[bdvect[dst]] = bdvect[012]
        // curcmd is ..... dst 54
        int dst = (curcmd >> 6) & 077;
        curcmd = curcmd >> 6;
        data[abdv[dst] & 077777] = allocHandle;
    } break;
    case Mars::OP_EXIT:
        throw Mars::ERR_SUCCESS;
    default:
        // In the original binary, loss of control ensued.
        std::cerr << std::format("Invalid micro-operation {:o} encountered\n",
                                 curcmd & 077);
        abort();
    }
    return curcmd >> 6;
}

void MarsImpl::check_space(unsigned need) {
    if (usable_space() < need) {
        Cursor[0].block_id = 0;
        free(allocHandle);
        throw Mars::ERR_OVERFLOW;
    }
}

bool MarsImpl::pr12x2(bool withHeader, uint64_t chain, uint64_t newHeader) {
    if (withHeader) {
        if (auto next = next_block(chain)) {
            auto head = get_block_header(next);
            head.prev = 0;
            set_header(newHeader | head.word);
        }
    }
    curMetaBlock = reinterpret_cast<Metablock*>(RootBlock);
    if (idx == 0) {
        set_dirty_tab();
        cont = curcmd >> 6;
        return true;
    }
    idx -= 2;
    if (idx != 0) {
        get_secondary_block(Cursor[idx/2].block_id);
    }
    uint64_t cnt = Cursor[idx/2].steps;
    bool pos = cnt & ONEBIT(16);
    // The following code is triggered, e.g. by using DELKEY
    // after stepping backwards or forwards.
    for (;cnt & BITS(15); pos ? --cnt : ++cnt) {
        if (step(!pos))
            return true;         // never happens?
    }
    return false;
}

// Performs key insertion and deletion in the BTree
uint64_t MarsImpl::key_manager(KeyOp op, uint64_t key) {
    bool recurse = false;
    int len;
    bool first;
    BlockElt * newelt;
    unsigned need;
    uint64_t chain, newHeader;
    uint64_t newDescr;
    uint64_t toAdd = workHandle;
    bool indirect = false;
    switch (op) {
    case ADDKEY: jump(addkey);
    case DELKEY: jump(delkey);
    case A01160: jump(a01160);
    }
  delkey:
    first = Cursor[idx/2].pos == 0;
    len = curMetaBlock->header.len;
    if (len < 2)
        std::cerr << "len @ delkey < 2\n";
    // Removing the element pointed by the iterator from the metablock
    for (size_t i = Cursor[idx/2].pos; i < curMetaBlock->header.len; ++i) {
        curMetaBlock->element[i/2] = curMetaBlock->element[i/2+1];
    }
    curMetaBlock->header.len -= 2;
    if (curMetaBlock->header.len == 0) {
        // The current metablock became empty
        if (!idx)               // That was the root metablock?
            jump(a01160);       // Do not free it
        free(curBlockDescr.word);
        chain = Secondary[0];   // Save the block chain
        newHeader = make_block_header(prev_block(Secondary[0]), 0, 0).word;
        if (auto prev = prev_block(Secondary[0])) {
            auto head = get_block_header(prev);  // Read the previous block if it exists ?
            Secondary[0] = make_block_header(head.prev, 0, head.len).word;
            // Exclude the deleted block from the chain
            set_header(make_block_header(head.prev, next_block(chain), head.len).word);
        }
        pr12x2(true, chain, newHeader);
        jump(delkey);           // Do something and go to delkey again
    }

    if (!first)
        jump(a01160);
    // The first entry in a metablock has been deleted
    while (true) {
        key = curMetaBlock->element[0].key; // the new key at position 0
        recurse = true;
        jump(a01160);
      a00731:
        curMetaBlock->element[Cursor[idx/2].pos/2].key = key;
    }
  addkey:
    newelt = &curMetaBlock->element[Cursor[idx/2].pos/2 + 1];
    if (verbose) {
        size_t total = curMetaBlock->header.len;
        size_t downto = Cursor[idx/2].pos + 2;
        std::cerr << std::format("Expanding {} elements\n", (total-downto)/2);
    }
    for (int i = curMetaBlock->header.len; i != Cursor[idx/2].pos + 2; i -= 2) {
        curMetaBlock->element[i/2] = curMetaBlock->element[i/2-1];
    }
    newelt->key = key;
    newelt->id = toAdd;
    newelt->indirect = indirect;
    curMetaBlock->header.len += 2;
  a01160:                       // generic metablock update ???
    if (idx == 0) {
        mylen = META_SIZE;
        if (curMetaBlock->header.len == META_SIZE-1) {
            // The root metablock is full
            check_space(0101);
            // Copy it somewhere and make a new root metablock
            // With the 0 key pointing to the newly made block
            BlockElt &first = *reinterpret_cast<BlockElt*>(RootBlock+1);
            first.id = allocator((uint64_t*)curMetaBlock);
            first.indirect = true;
            RootBlock[0] = 2;
            mylen = META_SIZE;
        }
        update(Cursor[0].block_id, (uint64_t*)curMetaBlock); // update the root metablock
        if (recurse) {
            recurse = false;
            if (pr12x2(false, chain, newHeader))
                return cont;
            jump(a00731);
        }
        return curcmd >> 6;
    }
    if (curMetaBlock->header.len != 0100) {
        // The current metablock has not reached the max allowed length
        mylen = curMetaBlock->header.len + 1;
        update(curBlockDescr.word, (uint64_t*)curMetaBlock);
        if (recurse) {
            recurse = false;
            if (pr12x2(false, chain, newHeader))
                return cont;
            jump(a00731);
        }
        return curcmd >> 6;
    }
    need = (idx * 2) + META_SIZE;
    if (RootBlock[0] == 036) {
        // The root metadata block is full, account for potentially adding a level
        need = need + 044;
    }
    check_space(need);
    // Split the block in two
    auto old = Secondary[040];
    Secondary[040] = make_block_header(curBlockDescr.word, next_block(Secondary[0]), 040).word;
    mylen = META_SIZE;
    newDescr = allocator((uint64_t*)curMetaBlock + 040);
    Secondary[040] = old;
    key = Secondary[META_SIZE];
    auto header = Secondary[0];
    Secondary[0] = make_block_header(prev_block(header), newDescr, 040).word;
    newHeader = make_block_header(newDescr, 0, 0).word;
    mylen = block_len(Secondary[0]) + 1;
    update(curBlockDescr.word, Secondary);
    if (pr12x2(true, header, newHeader))
        return cont;
    toAdd = newHeader >> 29;
    indirect = true;
    jump(addkey);
}

void MarsImpl::setup() {
    for (size_t i = 0; i < Mars::RAM_LENGTH; ++i) {
        new (data+i) word(0ul);
    }
    // Faking MARS words
    bdtab = &data[Mars::BDTAB].d;
    bdbuf = &data[Mars::BDBUF].d;
    abdv = &data[BDVECT].d;
    for (int i = 0; i < META_SIZE; ++i)
        data[FAKEBLK+i].d = ARBITRARY_NONZERO;
}

static int to_lnuzzzz(int lun, int start, int len) {
    if (lun > 077 || start > 01777 || len > 01731)
        std::cerr << std::format("{:o} {:o} {:o} out of valid range, truncated\n", lun, start, len);
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
    data[077777] = (impl.key << 10) & BITS(48);
    impl.myloc = 077776;
    impl.mylen = 2;
    impl.orgcmd = mcprog(OP_ROOT, OP_FIND, OP_NOMATCH, OP_ALLOC, OP_ADDKEY);
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
    impl.orgcmd = mcprog(OP_FIND, OP_NOMATCH, OP_ALLOC, OP_ADDKEY);
    return impl.eval();
}

Error Mars::putd(uint64_t k, int loc, int len) {
    if (verbose) {
        std::cerr << std::format("Running putd({:016o}, {:05o}:{})\n", k, loc, len);
    }
    impl.key = k;
    impl.mylen = len;
    impl.myloc = loc;
    impl.orgcmd = mcprog(OP_FIND, OP_NOMATCH, OP_ALLOC, OP_ADDKEY);
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
    impl.orgcmd = mcprog(OP_FIND, OP_NOMATCH, OP_ALLOC, OP_ADDKEY);
    return impl.eval();
}

Error Mars::modd(const char * k, int loc, int len) {
    impl.key = *reinterpret_cast<const uint64_t*>(k);
    impl.mylen = len;
    impl.myloc = loc;
    impl.orgcmd = mcprog(OP_FIND, OP_NOMATCH, OP_COND,
                         OP_ALLOC, OP_ADDKEY, OP_STOP,
                         OP_UPDATE);
    return impl.eval();
}

Error Mars::modd(uint64_t k, int loc, int len) {
    impl.key = k;
    impl.mylen = len;
    impl.myloc = loc;
    impl.orgcmd = mcprog(OP_FIND, OP_NOMATCH, OP_COND,
                         OP_ALLOC, OP_ADDKEY, OP_STOP,
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
    return impl.curkey;
}

uint64_t Mars::last() {
    impl.orgcmd = OP_LAST;
    impl.eval();
    return impl.curkey;
}

uint64_t Mars::prev() {
    impl.orgcmd = OP_PREV;
    impl.eval();
    return impl.curkey;
}

uint64_t Mars::next() {
    impl.orgcmd = OP_NEXT;
    impl.eval();
    return impl.curkey;
}

uint64_t Mars::find(const char * k) {
    impl.key = *reinterpret_cast<const uint64_t*>(k);
    impl.orgcmd = OP_FIND;
    impl.eval();
    return impl.curkey;
}

uint64_t Mars::find(uint64_t k) {
    impl.key = k;
    impl.orgcmd = OP_FIND;
    impl.eval();
    return impl.curkey;
}

int Mars::getlen() {
    impl.orgcmd = OP_LENGTH;
    if (impl.eval())
        return -1;
    return impl.datumLen;
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
    return impl.datumLen;
}

Error Mars::eval(uint64_t microcode) {
    impl.orgcmd = microcode;
    return impl.eval();
}
