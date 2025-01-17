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
                                     int(j), sav.w[j], bdvect().w[j]);
        }
    }
    sav = bdvect();
}

#define ONEBIT(n) (1ULL << ((n)-1))   // one bit set, from 1 to 48
#define BITS(n)   (~0ULL >> (64 - n)) // bitmask of n bits from LSB

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
    typedef uint64_t &uintref;
    typedef uint64_t* &puintref;

    // Cursor is a non-persistent structure
    struct CursorElt {
        uint64_t block_id;
        int pos: 15;
        unsigned steps: 16;
    };

    union Handle {
        uint64_t word;
        uint64_t full: 19;
        struct { uint64_t zone: 10, ext: 9; };
        Handle() : word(0) { }
        Handle(uint64_t w) : word(w) { }
        operator uint64_t() { return word; }
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
            struct { uint64_t len: 10, next: 19, prev: 19, filler: 16; };
            Header() : word(0) { }
            Header(uint64_t w) : word(w) { }
            Header(int p, int n, int l) : len(l), next(n), prev(p), filler(0) { }
            operator uint64_t() { return word; }
        } header;
        BlockElt element[];
    };

    union ExtentHeader {
        uint64_t word;
        struct { uint64_t len: 15, unknown: 18, timestamp: 15; };
        ExtentHeader() : word(0) { }
        ExtentHeader(uint64_t w) : word(w) { }
        operator uint64_t() { return word; }
    };

    union Extent {
        uint64_t word;
        struct { uint64_t start: 10, len: 10, next: 19, id: 9; };
        Extent() : word(0) { }
        Extent(uint64_t w) : word(w) { }
        operator uint64_t() { return word; }
    };

    Mars & mars;
    bool & verbose;

    uint64_t bufpage[1024], tabpage[1024];

    // Fields of BDVECT

    puintref myloc, bdbuf, bdtab, curbuf, freeSpace, extPtr;
    Metablock * &curMetaBlock;
    uintref orgcmd, curcmd,
        disableSync, givenp, key,
        allocHandle,
        loc14, mylen,
        dbdesc, DBkey, savedp,
        workHandle,
        curkey,
        endmrk,
        curWord, offset,
        IOpat,
        extLength, datumLen, dblen, curPos,
        handlePtr;
    uintref idx, curZone, dirty;

    void (*&erhndl)();
    Extent curExtent;
    Handle blockHandle;

    uint64_t cont;              // continuation instruction word
    CursorElt * const Cursor;
    Metablock * const RootBlock{reinterpret_cast<Metablock*>(mars.bdv.RootBlk+1)};
    uint64_t * const Secondary{mars.bdv.SecBlk+1};

    // Fields of BDSYS
    uint64_t arch;
    uint64_t *abdv;
    std::unordered_map<std::string, Page> DiskImage;

    MarsImpl(Mars & up) :
        mars(up), verbose(up.verbose),

        myloc(up.bdv.myloc),
        bdbuf(up.bdv.bdbuf),
        bdtab(up.bdv.bdtab),
        curbuf(up.bdv.curbuf),
        freeSpace(up.bdv.freeSpace),
        extPtr(up.bdv.extPtr),

        curMetaBlock(reinterpret_cast<Metablock*&>(up.bdv.curMetaBlock)),

        orgcmd(up.bdv.orgcmd),
        curcmd(up.bdv.curcmd),
        disableSync(up.bdv.disableSync),
        givenp(up.bdv.givenp),
        key(up.bdv.key),
        allocHandle(up.bdv.allocHandle),
        loc14(up.bdv.loc14),
        mylen(up.bdv.mylen),
        dbdesc(up.bdv.dbdesc),
        DBkey(up.bdv.DBkey),
        savedp(up.bdv.savedp),
        workHandle(up.bdv.workHandle),
        curkey(up.bdv.curkey),
        endmrk(up.bdv.endmrk),
        curWord(up.bdv.curWord),
        offset(up.bdv.offset),
        IOpat(up.bdv.IOpat),
        extLength(up.bdv.extLength),
        datumLen(up.bdv.datumLen),
        dblen(up.bdv.dblen),
        curPos(up.bdv.curPos),
        handlePtr(up.bdv.SecBlk[0]),
        idx(up.bdv.idx),
        curZone(up.bdv.curZone),
        dirty(up.bdv.dirty),

        erhndl(up.bdv.erhndl),
        Cursor(reinterpret_cast<CursorElt*>(up.bdv.Cursor))

        { }
    void IOflush();
    void IOcall(uint64_t, uint64_t *);
    void get_zone(uint64_t);
    void save(bool);
    void finalize(const char *);
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
    void assign_and_incr(uint64_t&);
    void setup();
    Error eval();
    uint64_t one_insn();
    void overflow(uint64_t);
    int prepare_chunk(int, int &remlen, uint64_t* &usrloc);
    uint64_t handle_chunk(int zone, uint64_t head);
    uint64_t allocator(uint64_t *usrloc), allocator1023(uint64_t firstWord, uint64_t *usrloc);
    uint64_t allocator1047(int loc, uint64_t head, uint64_t firstWord, int remlen, uint64_t* &usrloc);
    struct BtreeArgs {
        uint64_t key;
        bool recurse;
        BtreeArgs() : key(ARBITRARY_NONZERO), recurse(false) { }
    };
    uint64_t update_btree(BtreeArgs = BtreeArgs());
    void add_key(uint64_t key, uint64_t toAdd, bool indirect);
    BtreeArgs del_key();
    void update_prev(uint64_t chain, uint64_t newHeader);
    void propagate_steps();
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

inline uint64_t prev_block(uint64_t x) {
    return MarsImpl::Metablock::Header(x).prev;
}

inline uint64_t next_block(uint64_t x) {
    return MarsImpl::Metablock::Header (x).next;
}

inline uint64_t block_len(uint64_t x) {
    return MarsImpl::Metablock::Header(x).len;
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
    nuzzzz = std::format("{:06o}", op & BITS(18));
    if (op & ONEBIT(40)) {
        // read
        auto it = DiskImage.find(nuzzzz);
        if (verbose)
            std::cerr << std::format("Reading {} to {}\n", nuzzzz,
                                     buf == bdbuf ? "buf" : "tab");
        if (it == DiskImage.end()) {
            std::ifstream f(nuzzzz);
            if (!f) {
                std::cerr << "\tZone " << nuzzzz << " does not exist yet\n";
                for (auto p = buf; p < buf+1024; ++p)
                    *p = ARBITRARY_NONZERO;
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
            std::cerr << std::format("Writing {} from {}\n", nuzzzz,
                                     buf == bdbuf ? "buf" : "tab");
        DiskImage[nuzzzz] = buf;
    }
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

void MarsImpl::finalize(const char * msg) {
    bool force_tab = false;
    mars.errmsg = msg;
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
    static uint64_t init[META_SIZE];
    if (init[0] != 2) {
        for (int i = 3; i < META_SIZE; ++i)
            init[i] = ARBITRARY_NONZERO;
    }
    mylen = META_SIZE;
    init[0] = 2;
    init[1] = key;
    init[2] = Cursor[0].block_id;
    return init;
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
// Sets 'extPtr', also returns the extent length in 'extLength'
uint64_t MarsImpl::find_item(uint64_t arg) {
    Handle h(arg);
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
        curExtent = curbuf[handlePtr + 1];
        if (curExtent.id == id) {
            break;
        }
        if (--handlePtr == 0) {
            throw Mars::ERR_NO_RECORD;
        }
    }
    extPtr = curbuf + curExtent.start + 1;
    extLength =  curExtent.len;
    return extLength;
}

// Assumes that arg points to a datum with the standard header.
// Puts the full header to curWord, and its length to datumLen.
void MarsImpl::info(uint64_t arg) {
    find_item(arg);
    curWord = *extPtr;
    datumLen = curWord & 077777;
    curPos = 0;
}

void MarsImpl::totext() {
    uint64_t v = curWord;
    endmrk = Mars::tobesm(std::format("\017{:05o}", v & 077777));
    curWord = Mars::tobesm(std::format("\017{:c}{:c}{:c}{:c}{:c}",
                                     (v >> 46) & 3, (v >> 42) & 15,
                                     (v >> 41) & 1, (v >> 37) & 15,
                                     (v >> 33) & 15));
    offset = Mars::tobesm(std::format("\172\033\0\0\0\0"));
}

void MarsImpl::set_header(uint64_t arg) {
    *extPtr = arg;
    set_dirty_data();
}

std::string inPage(uint64_t *ptr, uint64_t* page, const char * name) {
    if (ptr >= page && ptr < page + 1024)
        return std::format("{}[{:04o}]", name, ptr-page);
    return std::string();
}

void MarsImpl::copy_words(uint64_t *dst, uint64_t* src, int len) {
    std::string srcStr, dstStr;
    srcStr = inPage(src, bufpage, "buf");
    if (srcStr.empty())
        srcStr = inPage(src, tabpage, "tab");
    if (srcStr.empty())
        srcStr = "user memory";
    dstStr = inPage(dst, bufpage, "buf");
    if (dstStr.empty())
        dstStr = inPage(dst, tabpage, "tab");
    if (dstStr.empty())
        dstStr = "user memory";
    if (verbose)
        std::cerr << std::format("{:o}(8) words from {} to {}\n", len,
                                 srcStr, dstStr);
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
    auto usrloc = myloc;
    if (mylen != 0 && mylen < datumLen)
        throw Mars::ERR_TOO_LONG;
    // Skip the first word (the header) of the found item
    ++extPtr;
    --extLength;
    copy_chained(extLength, usrloc);
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
// into memory pointed to by 'dest', if 'blockHandle' does
// not have the same ID already.
void MarsImpl::get_block(uint64_t descr, uint64_t* dest) {
    Cursor[idx].block_id = descr;
    Handle needed;
    needed.full = descr;
    curMetaBlock = reinterpret_cast<Metablock*>(dest);
    if (needed.full != blockHandle.full) {
        blockHandle = needed;
        auto ptr = dest - 1;
        copy_chained(find_item(blockHandle.full), ptr);
    }
}

// Requests the root block into the main RootBlock array
void MarsImpl::get_root_block() {
    get_block(ROOT_METABLOCK, &RootBlock->header.word);
}

// Requests a block into the Secondary metadata array
void MarsImpl::get_secondary_block(uint64_t descr) {
    get_block(descr, Secondary);
}

// If the micro-instruction after the current one is OP_COND (00)
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
    blockHandle = 0;
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
            ext.start += extLength;
        }
    }
    unsigned freeIdx = curbuf[1] & 01777;
    if (freeIdx != locIdx) {
        if (locIdx < freeIdx)         // extent cannot be located in the free area
            throw Mars::ERR_INTERNAL;
        int diff = locIdx - freeIdx;
        auto freeLoc = curbuf + freeIdx;
        auto loc = freeLoc + extLength;
        // Move the extent data up to make the free area
        // contiguous
        do {
            loc[diff] = freeLoc[diff];
        } while(--diff);
    }
    // Correct the free area location and the number of extents at once
    curbuf[1] = curbuf[1] - 02000 + extLength;
    freeSpace[curZone] = freeSpace[curZone] + extLength + 1;
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
    uint64_t * limit = myloc + mylen;
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
    if (tail >= extLength) {
        // The desired length is too long; adjust it
        tail -= extLength;
        len = extLength;
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
    usrloc = usrloc + extLength;
    return false;
}

// Returns true if skipping of micro-instructions is needed.
// dir == 0: forward, otherwise back.
bool MarsImpl::step(uint64_t dir) {
    CursorElt & cur = Cursor[idx];
    if (!cur.steps && !cur.pos)
        throw Mars::ERR_NO_CURR;
    int pos = cur.pos;
    if (!dir) {
        // Step forward
        pos++;
        if (pos == curMetaBlock->header.len/2) {
            auto next = curMetaBlock->header.next;
            if (!next) {
                cont = skip(Mars::ERR_NO_NEXT);
                return true;
            }
            get_secondary_block(next);
            ++Cursor[idx-1].steps;
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
            --Cursor[idx-1].steps;
            pos = block_len(Secondary[0]) / 2;
        }
        pos--;
    }
    cur.pos = pos;
    cur.steps = ONEBIT(16);
    curkey = curMetaBlock->element[pos].key;
    workHandle = curMetaBlock->element[pos].id;
    return false;
}

// For READ and WRITE, arg = data length; for SEEK, arg = skip length
bool MarsImpl::access_data(Ops op, uint64_t arg) {
    bool done;
    unsigned tail = arg;
    curPos = curPos + arg;
    uint64_t * usrloc = myloc;
    for (;;) {
        done = true;
        if (perform(op, done, tail, usrloc)) {
            // COMPARE failed, skip an instruction
            cont = curcmd >> 12;
            return true;
        }
        if (done) {
            extPtr += tail;
            extLength -= tail;
            // Side effect: SEEK returns the word at position in curWord
            curWord = *extPtr;
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
    find_item(arg);
    // Not expecting it to fail
    access_data(SEEK, 1);
    return Metablock::Header(curWord);
}

void MarsImpl::assign_and_incr(uint64_t& dst) {
    auto src = (curcmd >> 6) & 077;
    curcmd = curcmd >> 6;
    dst = *mars.bdv.u[src]++;
}

// Unpacks "myloc" (absolute offset, length, data address)
// into offset to be skipped and mylen.
// Was:
// | 48-47 | 46-34 | 33-19 | 18-16 | 15-1 |
// |-------|-------|-------|-------|------|
// |   ?   | length| offset|   ?   |  loc |
// From the code is appears that the max expected
// length of the datum segment was 017777.
// Now myloc is treated as a pointer to
// a possibly multi-word structure with 3 fields.
// To be used with OP_SEEK and OP_READ/OP_WRITE.
// Conditionally skips the next 4 instructions.
bool MarsImpl::unpack() {
    Mars::segment * s = reinterpret_cast<Mars::segment*>(myloc);
    auto seekPos = s->pos;
    offset = seekPos - curPos;
    auto len = s->len;
    myloc = s->loc;
    if (!len) {
        cont = curcmd >> 30;
        return true;
    }
    mylen = len;
    return false;
}

// Throws overflow after freeing pending extents.
void MarsImpl::overflow(uint64_t word) {
    Extent ext(word);
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
    return allocator1023(make_extent_header(), usrloc);
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
    myloc = bdbuf;
    Cursor[0].block_id = ROOT_METABLOCK;
    allocator(make_metablock(0));
    mylen = dblen;              // length of the free space array
    // This trick results in max possible DB length = 753 zones.
    // bdbuf[0] = 01731 - mylen;
    // Invoking OP_ALLOC for the freeSpace array
    allocHandle = allocator(myloc);
    bdtab[01736-dblen] = 01731 - dblen; // This is the right way
}

void MarsImpl::update_by_reallocation(int limit, ExtentHeader extentHeader, uint64_t* &usrloc) {
    auto old = curExtent;
    curExtent.next = 0;
    free_from_current_extent(limit);
    ++mylen;
    allocator1047(0, uint64_t(old.id) << 39, extentHeader, 0, usrloc);
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
    uint64_t * limit = myloc + mylen;
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
    int i = block->header.len / 2;
    if (verbose)
        std::cerr << std::format("Comparing {} elements\n", i);
    for (; i; i--) {
        if (block->element[i-1].key <= what)
            break;
    }
    i--;
    Cursor[idx].pos = i;
    Cursor[idx].steps = ONEBIT(16);
    if (!indirect) {
        curkey = curMetaBlock->element[i].key;
        workHandle = curMetaBlock->element[i].id;
        return;
    }
    idx++;
    if (idx > 3) {
        std::cerr << "Idx = " << idx << ": DB will be corrupted\n";
    }
    get_secondary_block(block->element[i].id);
    search_in_block(reinterpret_cast<Metablock*>(Secondary), what);
}

void MarsImpl::find(uint64_t k) {
    idx = 0;
    if (!Cursor[0].block_id) {
        // There was an overflow, re-reading is needed
        get_root_block();
    }
    search_in_block(RootBlock, k);
}

void MarsImpl::update(uint64_t arg, uint64_t* usrloc) {
    uint64_t len = find_item(arg) - 1;
    if (mylen == len) {
        // The new length of data matches the first extent length
        for (; len--;)
            extPtr[len+1] = usrloc[len];
        *extPtr = make_extent_header(); // refresh the timestamp
        set_dirty_data();
        if (auto next = curExtent.next) {
            curExtent.next = 0;
            curbuf[handlePtr+1] = curExtent; // dropping the extent chain
            free(next);         // Free remaining extents
            set_dirty_tab();
        }
        return;
    }
    auto extentHeader = make_extent_header();
    auto numExtents = curbuf[1] >> 10;
    // Compute the max extent length to avoid fragmentation
    auto curFree = (curbuf[1] - numExtents) & 01777;
    curFree += extLength - 2;
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
    curbuf[handlePtr+1] = curExtent;
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
    finalize(nullptr);
    if (mars.dump_diffs)
        mars.dump();
    return mars.status = Mars::ERR_SUCCESS;
} catch (Error e) {
    if (e != Mars::ERR_SUCCESS) {
        if (erhndl) {
            // returning to erhndl instead of the point of call
        }
        std::cerr << std::format("ERROR {} ({})\n", int(e), msg[e-1]);
        finalize(msg[e-1]);
    }
    if (mars.dump_diffs)
        mars.dump();
    return mars.status = e;
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
    case Mars::OP_INSMETA:
        allocHandle = allocator(make_metablock(0));
        break;
    case Mars::OP_SETMETA:
        idx = 0;
        datumLen = META_SIZE;
        get_block(workHandle, &RootBlock->header.word);
        break;
    case Mars::OP_SEEK:               // also reads word at the reached position
        access_data(SEEK, offset);    // of the current datum into 'curWord'
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
        myloc = &dbdesc;
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
        update(workHandle, myloc);
        break;
    case Mars::OP_ALLOC:
        allocHandle = allocator(myloc);
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
        add_key(key, workHandle, false);
        return update_btree();
    case Mars::OP_DELKEY:
        return update_btree(del_key());
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
    case Mars::OP_REPLACE:
        curMetaBlock->element[Cursor[idx].pos].id = allocHandle;
        return update_btree();
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
        workHandle = curWord;
        break;
    case Mars::OP_LOCK:
        lock();
        break;
    case Mars::OP_UNPACK:
        if (unpack())
            return cont;
        break;
    case Mars::OP_CALL:
        // acc = curWord.d;
        // Then an indirect jump to outadr;
        // expected to return to enter2?
        break;
    case Mars::OP_CHAIN:
        // orgcmd := mem[bdvect[src]++]
        // curcmd is ..... src 50
        assign_and_incr(orgcmd);
        return orgcmd;
    case Mars::OP_SEGMENT:      // macro for ... src 13 52
        // myloc := mem[bdvect[src]++]
        // curcmd is ..... src 51
        assign_and_incr(*reinterpret_cast<uint64_t*>(&myloc));
        break;
    case Mars::OP_LDNEXT: {
        // bdvect[dst] := mem[bdvect[src]++]
        // curcmd is ..... src dst 52
        int dst = (curcmd >>= 6) & 077;
        assign_and_incr(mars.bdv.w[dst]);
    } break;
    case Mars::OP_ASSIGN: {
        // bdvect[dst] = bdvect[src]
        // curcmd is ..... src dst 53
        int dst = (curcmd >>= 6) & 077;
        int src = (curcmd >>= 6) & 077;
        abdv[dst] = abdv[src];
    } break;
    case Mars::OP_STALLOC: {
        // mem[bdvect[dst]] = bdvect[012]
        // curcmd is ..... dst 54
        int dst = (curcmd >>= 6) & 077;
        *mars.bdv.u[dst] = allocHandle;
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

void MarsImpl::update_prev(uint64_t chain, uint64_t prev) {
    if (auto next = next_block(chain)) {
        auto head = get_block_header(next);
        head.prev = prev;
        set_header(head);
    }
}

#define parent_block                                    \
    idx--;                                              \
    if (idx != 0) {                                     \
        get_secondary_block(Cursor[idx].block_id);      \
    } else     curMetaBlock = RootBlock;


void MarsImpl::propagate_steps() {
    curMetaBlock = RootBlock;
    if (idx == 0) {
        set_dirty_tab();
        cont = curcmd >> 6;
        return;
    }
    idx--;
    if (idx != 0) {
        get_secondary_block(Cursor[idx].block_id);
    }
    int cnt = Cursor[idx].steps - ONEBIT(16);
    // The following code is triggered, e.g. by using DELKEY
    // after stepping backwards or forwards.
    for (; cnt != 0; cnt > 0 ? --cnt : ++cnt) {
        if (step(cnt < 0)) {
            throw Mars::ERR_INTERNAL;
        }
    }
}

void MarsImpl::add_key(uint64_t key, uint64_t toAdd, bool indirect) {
    BlockElt * newelt = &curMetaBlock->element[Cursor[idx].pos + 1];
    if (verbose) {
        size_t total = curMetaBlock->header.len/2;
        size_t downto = Cursor[idx].pos + 1;
        std::cerr << std::format("Expanding {} elements\n", total-downto);
    }
    for (int i = curMetaBlock->header.len/2; i > Cursor[idx].pos; i--) {
        curMetaBlock->element[i] = curMetaBlock->element[i-1];
    }
    newelt->key = key;
    newelt->id = toAdd;
    newelt->indirect = indirect;
    curMetaBlock->header.len += 2;
}

auto MarsImpl::del_key() -> BtreeArgs {
    BtreeArgs bta;
    uint64_t chain, link, &key = bta.key;
    bool &recurse = bta.recurse;
    bool first;
    for(;;) {
        first = Cursor[idx].pos == 0;
        size_t len = curMetaBlock->header.len;
        if (len < 2)
            std::cerr << "len @ delkey < 2\n";
        // Removing the element pointed by the iterator from the metablock
        // The off-by-1 error here prevents from clearing the root metablock properly
        for (size_t i = Cursor[idx].pos; i < len/2; ++i) {
            curMetaBlock->element[i] = curMetaBlock->element[i+1];
        }
        curMetaBlock->header.len -= 2; // This should be before setting up len
        if (curMetaBlock->header.len != 0)
            break;
        // The current metablock became empty
        if (!idx)               // That was the root metablock?
            return bta;         // Do not free it
        free(blockHandle);
        chain = Secondary[0];   // Save the block chain
        link = prev_block(Secondary[0]);
        if (link) {
            auto head = get_block_header(link);  // Read the previous block if it exists ?
            Secondary[0] = Metablock::Header(head.prev, 0, head.len);
            // Exclude the deleted block from the chain
            set_header(Metablock::Header(head.prev, next_block(chain), head.len));
        }
        update_prev(chain, link);
        propagate_steps();
    }
    if (first) {
        key = curMetaBlock->element[0].key; // the new key at position 0
        recurse = true;
    }
    return bta;
}

// Updates the BTree after insertion or deletion.
uint64_t MarsImpl::update_btree(BtreeArgs bta) {
    uint64_t &key = bta.key;
    bool &recurse = bta.recurse;
    unsigned need;
    if (idx == 0) {
        mylen = META_SIZE;
        if (curMetaBlock->header.len == META_SIZE-1) {
            // The root metablock is full
            check_space(0101);
            // Copy it somewhere and make a new root metablock
            // With the 0 key pointing to the newly made block
            BlockElt &first = RootBlock->element[0];
            first.id = allocator((uint64_t*)curMetaBlock);
            first.indirect = true;
            RootBlock->header.len = 2;
            mylen = META_SIZE;
        }
        update(Cursor[0].block_id, (uint64_t*)curMetaBlock); // update the root metablock
        set_dirty_tab();        // likely unnecessary, update() sets the flags
        return curcmd >> 6;
    }
    if (curMetaBlock->header.len != 0100) {
        // The current metablock has not reached the max allowed length
        mylen = curMetaBlock->header.len + 1;
        update(blockHandle, (uint64_t*)curMetaBlock);
        if (recurse) {
            propagate_steps();
            curMetaBlock->element[Cursor[idx].pos].key = key;
            key = curMetaBlock->element[0].key; // the new key at position 0
            return update_btree(bta);
        }
        return curcmd >> 6;
    }
    need = (idx * 4) + META_SIZE;
    if (RootBlock->header.len == 036) {
        // The root metadata block is full, account for potentially adding a level
        need = need + 044;
    }
    check_space(need);
    // Split the block in two
    auto old = Secondary[040];
    Secondary[040] = Metablock::Header(blockHandle, next_block(Secondary[0]), 040);
    mylen = META_SIZE;
    uint64_t newDescr = allocator((uint64_t*)curMetaBlock + 040);
    Secondary[040] = old;
    key = Secondary[META_SIZE];
    auto origHeader = Secondary[0];
    Secondary[0] = Metablock::Header(prev_block(origHeader), newDescr, 040);
    uint64_t link = newDescr;
    mylen = block_len(Secondary[0]) + 1;
    update(blockHandle, Secondary);
    update_prev(origHeader, link);
    propagate_steps();
    add_key(key, link, true);
    return update_btree(bta);
}

void MarsImpl::setup() {
    for (size_t i = 0; i < 1024; ++i) {
        bufpage[i] = tabpage[i] = 0;
    }
    // Faking MARS words
    bdtab = tabpage;
    bdbuf = bufpage;
    abdv = mars.bdv.w;
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
Error Mars::newd(const char * k, int lun, int start, int len, uint64_t passwd) {
    static uint64_t descr[3];
    int lnuzzzz = to_lnuzzzz(lun, start, len);
    if (verbose)
        std::cerr << std::format("Running newd('{}', {:o})\n", k, lnuzzzz);
    impl.key = *reinterpret_cast<const uint64_t*>(k);
    descr[0] = lnuzzzz;
    descr[1] = (impl.key << 10) & BITS(48);
    descr[2] = passwd;
    impl.myloc = descr;
    impl.mylen = passwd ? 3 : 2;
    impl.orgcmd = mcprog(OP_ROOT, OP_FIND, OP_NOMATCH, OP_ALLOC, OP_ADDKEY);
    impl.eval();
    // OP_FIND and OP_MATCH do not seem to be necessary
    impl.orgcmd = mcprog(OP_FIND, OP_MATCH, OP_SETCTL, OP_INIT);
    return impl.eval();
}

Error Mars::opend(const char * k, uint64_t passwd) {
    if (verbose)
        std::cerr << "Running opend('" << k << "')\n";
    impl.key = *reinterpret_cast<const uint64_t*>(k);
    impl.givenp = impl.savedp = passwd;
    impl.orgcmd = mcprog(OP_ROOT, OP_FIND, OP_MATCH,
                         OP_SETCTL, OP_OPEN, OP_PASSWD);
    return impl.eval();
}

Error Mars::putd(uint64_t k, uint64_t *loc, int len) {
    if (verbose) {
        std::cerr << std::format("Running putd({:016o}, {}:{})\n", k, (void*)loc, len);
    }
    impl.key = k;
    impl.mylen = len;
    impl.myloc = loc;
    impl.orgcmd = mcprog(OP_FIND, OP_NOMATCH, OP_ALLOC, OP_ADDKEY);
    return impl.eval();
}

Error Mars::modd(const char * k, uint64_t *loc, int len) {
    impl.key = *reinterpret_cast<const uint64_t*>(k);
    impl.mylen = len;
    impl.myloc = loc;
    impl.orgcmd = mcprog(OP_FIND, OP_NOMATCH, OP_COND,
                         OP_ALLOC, OP_ADDKEY, OP_STOP,
                         OP_UPDATE);
    return impl.eval();
}

Error Mars::modd(uint64_t k, uint64_t *loc, int len) {
    impl.key = k;
    impl.mylen = len;
    impl.myloc = loc;
    impl.orgcmd = mcprog(OP_FIND, OP_NOMATCH, OP_COND,
                         OP_ALLOC, OP_ADDKEY, OP_STOP,
                         OP_UPDATE);
    return impl.eval();
}

Error Mars::getd(const char * k, uint64_t *loc, int len) {
    impl.key = *reinterpret_cast<const uint64_t*>(k);
    impl.mylen = len;
    impl.myloc = loc;
    impl.orgcmd = mcprog(OP_FIND, OP_MATCH, OP_GET);
    return impl.eval();
}

Error Mars::getd(uint64_t k, uint64_t *loc, int len) {
    if (verbose) {
        std::cerr << std::format("Running getd({:016o}, {}:{})\n", k, (void*)loc, len);
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
