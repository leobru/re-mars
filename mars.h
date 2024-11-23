#ifndef MARS_H
#define MARS_H

#include <cstdint>

enum Error {
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

union word {
    uint64_t d;
    void*    p;
    struct {
        unsigned off1 : 10;
        unsigned off2 : 10;
        unsigned extent  : 19;
        unsigned upper9  : 9;
    };
    word(uint64_t x = 0) : d(x) { }
    uint64_t& operator=(uint64_t x);
    uint64_t& operator=(int x) { return this->operator=(uint64_t(x)); }
    uint64_t& operator=(int64_t x) { return this->operator=(uint64_t(x)); }
    uint64_t& operator=(long long int x) { return this->operator=(uint64_t(x)); }
    uint64_t& operator=(long long unsigned int x) { return this->operator=(uint64_t(x)); }
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

const int RAM_LENGTH = 32768;

// Locations of BDSYS, BDTAB and BDBUF match those in a mid-sized test program on the BESM-6.

const int BDVECT = 01400;
const int BDSYS = 02000;
const int BDTAB = 04000;
const int BDBUF = 06000;
const int MAXCHUNK = 01775;

// Addresses above 010000 can be used for user data
extern word data[RAM_LENGTH];

void InitDB(int lun, int start_zone, int length);

// The root catalog length must be given when opening the database
void SetDB(int lun, int start_zone, int length);

void newd(const char * k, int lun, int start_zone, int length);

void opend(const char * k);

void putd(uint64_t k, int loc, int len);

#endif