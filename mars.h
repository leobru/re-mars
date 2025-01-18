#ifndef MARS_H
#define MARS_H

#include <cstdint>
#include <string>
#include <algorithm>
#include <memory>

class Mars {
    struct MarsImpl & impl;
    friend struct MarsImpl;
  public:
    enum Error {
        ERR_SUCCESS = 0,
        ERR_ZEROKEY = 1,            // unclear if user-induced or internal
        ERR_BAD_PAGE = 2,           // data corruption
        ERR_NO_RECORD = 3,          // no record to operate on
        ERR_INV_NAME = 4,           // 0 or with high bit set
        ERR_BAD_CATALOG = 5,        // data corruption
        ERR_OVERFLOW = 6,           // self-explanatory
        ERR_SEEK = 7,               // offset into datum too large
        ERR_NO_NAME = 8,            // requested key name not found
        ERR_EXISTS = 9,             // key already exists
        ERR_NO_END_MARK = 10,       // end mark not found within specified range
        ERR_INTERNAL = 11,          // formerly "no end word"
        ERR_TOO_LONG = 12,          // record is longer than th user space for it
        ERR_LOCKED = 13,            // attempt to lock already locked DB
        ERR_NO_CURR = 14,           // no current record to step from
        ERR_NO_PREV = 15,           // no previous record (not triggered by BEGIN PREV)
        ERR_NO_NEXT = 16,           // no next record
        ERR_WRONG_PASSWORD = 17     // the saved password does not match the provided one
    };

    enum Op : uint64_t {
        OP_NOP = 0,
        OP_COND = 0,
        OP_BEGIN = 1,
        OP_LAST = 2,
        OP_PREV = 3,
        OP_NEXT = 4,
        OP_INSMETA = 5,
        OP_SETMETA = 6,
        OP_SEEK = 7,
        OP_INIT = 010,
        OP_FIND = 011,
        OP_SETCTL = 012,
        OP_AVAIL = 013,
        OP_MATCH = 014,
        OP_NOMATCH = 015,
        OP_STRLEN = 016,
        OP_WORDLEN = 017,
        OP_UPDATE = 020,
        OP_ALLOC = 021,
        OP_GET = 022,
        OP_FREE = 023,
        OP_PASSWD = 024,
        OP_OPEN = 025,
        OP_ADDKEY = 026,
        OP_DELKEY = 027,
        OP_LOOP = 030,
        OP_ROOT = 031,
        OP_INSERT = 032,
        OP_LENGTH = 033,
        OP_DESCR = 034,
        OP_SAVE = 035,
        OP_REPLACE = 036,
        OP_SKIP = 037,
        OP_STOP = 040,
        OP_IFEQ = 041,
        OP_WRITE = 042,
        OP_READ = 043,
        OP_USE = 044,
        OP_LOCK = 045,
        OP_UNPACK = 046,
        OP_CALL = 047,
        OP_CHAIN = 050,
        OP_SEGMENT = 051,
        OP_LDNEXT = 052,
        OP_ASSIGN = 053,
        OP_STALLOC = 054,
        OP_EXIT = 055
    };

    // Helper functions

    // Constructs a microprogram word.
    static inline uint64_t
    mcprog(Op o1, Op o2=OP_NOP, Op o3=OP_NOP, Op o4=OP_NOP,
           Op o5=OP_NOP, Op o6=OP_NOP, Op o7=OP_NOP, Op o8=OP_NOP) {
        return (o8 << 42) | (o7 << 36) | (o6 << 30) | (o5 << 24) |
            (o4 << 18) | (o3 << 12) | (o2 << 6) | o1;
    }
    // Converts a single-word string to the BESM-6 compatible format
    // for ease of comparison of binary dumps.
    static uint64_t tobesm(std::string s) {
        s += "     ";
        s.resize(6);
        std::reverse(s.begin(), s.end());
        s.resize(8);
        return *(uint64_t*)s.c_str();
    }

    struct segment {
        uint64_t *loc;
        unsigned pos, len;
        segment(uint64_t *l = nullptr, unsigned p = 0, unsigned s = 0) :
            loc(l), pos(p), len(s) { }
    };

    struct word {
        union { uint64_t d; uint64_t *u; };
        word(uint64_t x = 0) : d(x) { }
        uint64_t& operator=(long unsigned int x) { return d = x; }
        uint64_t& operator=(int x) { return d = x; }
        uint64_t& operator=(long x) { return d = x; }
        uint64_t& operator=(long long int x) { return d = x; }
        uint64_t& operator=(long long unsigned int x) { return d = x; }
        bool operator==(const word & x) const { return d == x.d; }
        bool operator!=(const word & x) const { return d != x.d; }
    };

    union bdvect_t {
        static const size_t SIZE = 168;
        uint64_t w[SIZE];
        uint64_t *u[SIZE];
        struct {                // offset(8)
            uint64_t *ip;       // 0
            void     (*call)(); // 1
            uint64_t *loc2;     // 2
            uint64_t orgcmd;    // 3
            uint64_t loc4;      // 4
            uint64_t curcmd;    // 5
            uint64_t disableSync; // 6
            uint64_t givenp;      // 7
            uint64_t key;         // 10
            void     (*erhndl)(); // 11
            uint64_t allocHandle; // 12
            uint64_t *myloc;      // 13
            uint64_t loc14;       // 14
            uint64_t mylen;       // 15
            uint64_t *bdbuf;      // 16
            uint64_t *bdtab;      // 17
            uint64_t Cursor[8];   // 20-27
            uint64_t dbdesc;      // 30
            uint64_t DBkey;       // 31
            uint64_t savedp;      // 32
            uint64_t doNotUse1;   // 33
            uint64_t *freeSpace;  // 34
            uint64_t workHandle;  // 35
            uint64_t doNotUse2;   // 36
            uint64_t curkey;      // 37
            uint64_t endmrk;      // 40
            uint64_t curWord;     // 41
            uint64_t offset;      // 42
            uint64_t loc43;       // 43
            uint64_t IOpat;       // 44
            uint64_t extLength;   // 45
            uint64_t *extPtr;     // 46
            uint64_t datumLen;    // 47
            uint64_t curMetaBlock; // 50
            uint64_t dblen;        // 51
            uint64_t loc52;        // 52
            uint64_t curPos;       // 53
            uint64_t RootBlk[042]; // 54-115
            uint64_t SecBlk[0102]; // 116-217
            uint64_t curExtent;    // 220
            uint64_t unknown[020]; // 221-240
            uint64_t *curbuf;      // 241
            uint64_t idx;          // 242
            uint64_t loc243;       // 243
            uint64_t curZone;      // 244
            uint64_t loc245;       // 245
            uint64_t blockHandle;  // 246
            uint64_t dirty;        // 247
        };
        bdvect_t() : w{} { }
    };

    static const Op KEY = Op(010);
    static const Op ALLOC = Op(012);
    static const Op HANDLE = Op(035);

    typedef uint64_t& uintref;
    typedef word& wordref;
    typedef uint64_t*& puintref;
    // BDVECT fields used by the API
    puintref cont = bdv.ip;
    uintref disableSync = bdv.disableSync;
    uintref key = bdv.key;
    puintref myloc = bdv.myloc;
    uintref mylen = bdv.mylen;
    uintref handle = bdv.workHandle;
    uintref curkey = bdv.curkey;
    uintref offset = bdv.offset;
    uintref datumLen = bdv.datumLen;

    // Page size (02000 words) minus 3.
    static const int MAXCHUNK = 01775;

    Mars();
    Mars(bool persistent);
    ~Mars();

    // Initialize a database with 0 <= lun <= 077,
    // 0 <= start_zone <= 01777, 1 <= length <= 01731
    Error InitDB(int lun, int start_zone, int length);

    // The same arguments, including root catalog length, as used in InitDB
    // must be given when opening the database.
    Error SetDB(int lun, int start_zone, int length);

    Error newd(const char * k, int lun, int start_zone, int len, uint64_t passwd = 0);

    Error opend(const char * k, uint64_t passwd = 0);

    Error putd(uint64_t k, uint64_t *loc, int len);
    Error putd(const char * k, const char * v);
    Error putd(const char * k, int loc, int len);

    Error modd(const char * k, uint64_t *loc, int len);
    Error modd(uint64_t k, uint64_t *loc, int len);
    Error getd(const char * k, uint64_t *loc, int len);
    Error getd(uint64_t k, uint64_t *loc, int len);

    Error deld(const char * k);
    Error deld(uint64_t k);

    Error root(), cleard(bool forward), eval();

    Error eval(uint64_t microcode);

    uint64_t first(), last(), prev(), next();

    uint64_t find(const char * k), find(uint64_t k);

    int getlen(), avail();

    bdvect_t & bdvect() { return bdv; }

    bool dump_txt_zones = false;
    bool verbose = false;
    bool zero_date = false;
    bool dump_diffs = false;
    Error status;
    const char * errmsg;        // nullptr when status is ERR_SUCCESS
private:
    bool flush = true;
    bdvect_t bdv, sav;
    void dump();
};

#endif
