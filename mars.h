#ifndef MARS_H
#define MARS_H

#include <cstdint>

enum Error {
    ERR_SUCCESS = 0,
    ERR_ZEROKEY = 1,            // unclear if user-induced or internal
    ERR_BAD_PAGE = 2,
    ERR_NO_RECORD = 3,
    ERR_INV_NAME = 4,           // 0 or with high bit set
    ERR_BAD_CATALOG = 5,
    ERR_OVERFLOW = 6,
    ERR_STEP = 7,               // unclear
    ERR_NO_NAME = 8,
    ERR_EXISTS = 9,
    ERR_NO_END_MARK = 10,
    ERR_INTERNAL = 11,          // formerly "no end word"
    ERR_TOO_LONG = 12,
    ERR_LOCKED = 13,
    ERR_NO_CURR = 14,
    ERR_NO_PREV = 15,
    ERR_NO_NEXT = 16,
    ERR_WRONG_PASSWORD = 17
};

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
    inline word& operator*();
    inline uint64_t operator&();
    inline word& operator[](word x);
    word operator+(word x) const { return d + x.d; }
    word operator-(word x) const { return d - x.d; }
    word operator&(word x) const { return d & x.d; }
    word operator|(word x) const { return d | x.d; }
    uint64_t operator>>(int x) const { return d >> x; }
    bool operator==(const word & x) { return d == x.d; }
    bool operator!=(const word & x) { return d != x.d; }
    word& operator++() { (*this) = d + 1; return *this; }
    word& operator--() { (*this) = d - 1; return *this; }
};

class Mars {
    class MarsImpl & impl;
    friend class word;
    friend class MarsImpl;
  public:
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

    // Page size (02000 words) minus 3.
    static const int MAXCHUNK = 01775;

    Mars();
    Mars(bool persistent);
    ~Mars();

    // Initialize a database with 0 <= lun <= 077,
    // 0 <= start_zone <= 01777, 1 <= length <= 01777
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
