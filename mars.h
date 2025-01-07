#ifndef MARS_H
#define MARS_H

#include <cstdint>

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
        OP_INSERT = 021,
        OP_GET = 022,
        OP_FREE = 023,
        OP_PASSWD = 024,
        OP_OPEN = 025,
        OP_ADDKEY = 026,
        OP_DELKEY = 027,
        OP_LOOP = 030,
        OP_ROOT = 031,
        OP_LENGTH = 033,
        OP_DESCR = 034,
        OP_SAVE = 035,
        OP_ADDMETA = 036,
        OP_SKIP = 037,
        OP_STOP = 040,
        OP_IFEQ = 041,
        OP_WRITE = 042,
        OP_READ = 043,
        OP_LOCK = 045,
        OP_CALL = 047,
        OP_CHAIN = 050,
        OP_ASSIGN = 053,
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

    struct word {
        class Mars * const mars = nullptr;
        uint64_t d;
        word(Mars& base, uint64_t x) : mars(&base), d(x) { }
        word(uint64_t x = 0) : d(x) { }
        uint64_t& operator=(long unsigned int x) { return store(x); }
        uint64_t& operator=(int x) { return store(x); }
        uint64_t& operator=(long x) { return store(x); }
        uint64_t& operator=(long long int x) { return store(x); }
        uint64_t& operator=(long long unsigned int x) { return store(x); }
        uint64_t& store(uint64_t x);
        word& operator=(word x);
        word& operator=(word * x);
        inline word& operator*() const;
        inline word& operator[](word x) const;
        word operator+(word x) const { return word(*mars, d + x.d); }
        word operator-(word x) const { return word(*mars, d - x.d); }
        word operator&(word x) const { return d & x.d; }
        word operator|(word x) const { return d | x.d; }
        uint64_t operator>>(int x) const { return d >> x; }
        bool operator==(const word & x) const { return d == x.d; }
        bool operator!=(const word & x) const { return d != x.d; }
        bool operator<(const word & x) const { return d < x.d; }
        word& operator++() { (*this) = d + 1; return *this; }
        word& operator--() { (*this) = d - 1; return *this; }
    };

    struct bdvect_t {
        static const size_t SIZE = 168;
        word w[SIZE];
    };

    // The user-accessible memory size of the BESM-6 is 32K 48-bit words.
    static const int RAM_LENGTH = 32768;

    // Addresses above 010000 can be used for user data.
    word data[RAM_LENGTH];

    // Locations of BDSYS, BDTAB and BDBUF match those
    // in a mid-sized test program on the BESM-6.
    static const int BDVECT = 01400;
    static const int BDSYS = 02000;
    static const int BDTAB = 04000;
    static const int BDBUF = 06000;

    typedef word& wordref;
    // BDVECT fields used by the API
    wordref disableSync = data[BDVECT+6];
    wordref key = data[BDVECT+010];
    wordref myloc = data[BDVECT+013];
    wordref mylen = data[BDVECT+015];
    wordref offset = data[BDVECT+042];
    wordref datumLen = data[BDVECT+047];

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

    Error newd(const char * k, int lun, int start_zone, int len);

    Error opend(const char * k);

    Error putd(uint64_t k, int loc, int len);
    Error putd(const char * k, const char * v);
    Error putd(const char * k, int loc, int len);

    Error modd(const char * k, int loc, int len);
    Error modd(uint64_t k, int loc, int len);
    Error getd(const char * k, int loc, int len);
    Error getd(uint64_t k, int loc, int len);

    Error deld(const char * k);
    Error deld(uint64_t k);

    Error root(), cleard(bool forward), eval();

    Error eval(uint64_t microcode);

    uint64_t first(), last(), prev(), next();

    uint64_t find(const char * k), find(uint64_t k);

    int getlen(), avail();

    // Get result of store(), when flag memoize_stores is enabled.
    uint64_t get_store(size_t index);

    bdvect_t & bdvect() { return *reinterpret_cast<bdvect_t*>(data+BDVECT); }

    bool dump_txt_zones = false;
    bool verbose = false;
    bool trace_stores = false;
    bool memoize_stores = false;
    bool zero_date = false;
    bool dump_diffs = false;
private:
    bool flush = true;
    bdvect_t sav;
    void dump();
};

#endif
