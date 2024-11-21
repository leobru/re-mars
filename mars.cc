#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <error.h>
#include <cstdlib>
#include <cassert>
#include <string>
#include <ctime>
#include <getopt.h>

union word {
    uint64_t d;
    void*    p;
    word(uint64_t x = 0) : d(x) { }
    uint64_t& operator=(uint64_t x);
    word& operator=(word x);
    inline word& operator*();
    inline uint64_t operator&();
    inline word& operator[](word x);
    word operator+(int x) const { return (d+x) & 077777; }
    word operator-(int x) const { return (d-x) & 077777; }
    uint64_t operator>>(int x) const { return d >> x; }
    bool operator==(const word & x) { return d == x.d; }
    bool operator!=(const word & x) { return d != x.d; }
    word& operator++() { (*this) = d + 1; return *this; }
    word& operator--() { (*this) = d - 1; return *this; }
};

bool trace_stores = false;
bool verbose = false;

word data[1024*6];

uint64_t rk = 7LL << 41;

// For convenience of detection which locations not to trace
word & m5 = data[5];
word & m16 = data[016];
word & m6 = data[6];
word & m7 = data[7];
word & m13 = data[13];

uint64_t& word::operator=(uint64_t x) {
    size_t index = this-data;
    if (index < 16) x &= 077777;
    if (trace_stores && index > 16 && index < 1024*6)
        printf("       %05lo: store %016lo\n", index, x);
    d=x;
    return d;
}
word& word::operator=(word x) {
    size_t index = this-data;
    if (index < 16) x.d &= 077777;
    if (trace_stores && index > 16 && index < 1024*6)
        printf("       %05lo: store %016lo\n", index, x.d);
    d=x.d;
    return *this;
}


struct bdvect_t {
    static const size_t SIZE = 168;
    word w[SIZE];
};

bdvect_t sav;

#define BDSYS 06000
#define BDTAB 010000
#define BDBUF 012000
#define BDVECT 01001
#define FAKEBLK 020

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
    ERR_NO_CURRENT = 14,
    ERR_NO_PREV = 15,
    ERR_NO_NEXT = 16,
    ERR_WRONG_PASSWORD = 17
};

#define bdvect (*reinterpret_cast<bdvect_t*>(data+BDVECT))

void dump() {
    for (int j = 0; j < bdvect_t::SIZE; ++j) {
        if (bdvect.w[j] != sav.w[j]) {
            printf(" WORD %03o CHANGED FROM  %016lo TO  %016lo\n",
                   j, sav.w[j].d, bdvect.w[j].d);
        }
    }
    sav = bdvect;
}

word& word::operator*() {
    unsigned a = d & 077777;
    if (a >= sizeof(data)/sizeof(*data)) {
        std::cerr << "Address " << std::oct << a << " out of range\n";
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
#define call(addr,link) do { link.p = &&ret; goto addr; ret:; } while (0)
#define dirty(x) dirty = dirty.d | (which.d ? x : 0) + 1
#define cycle(addr,reg) if (--reg.d) goto addr
#define utm(x,v) x.d = (x.d + (v)) & 077777;
// #define jump(x) do { std::cerr << "Jump to " #x "\n"; goto x; } while(0)
#define jump(x) goto x

// struct Bdvect
word & outadr = data[BDVECT+1];
word & orgcmd = data[BDVECT+3];
word & curcmd = data[BDVECT+5];
word & sync = data[BDVECT+6];
uint64_t & givenp = data[BDVECT+7].d;
word & key = data[BDVECT+010];
word & erhndl = data[BDVECT+011];
word & loc12 = data[BDVECT+012];
word & myloc = data[BDVECT+013];
word & loc14 = data[BDVECT+014];
word & mylen = data[BDVECT+015];
word & bdbuf = data[BDVECT+016];
word & bdtab = data[BDVECT+017];
word & loc20 = data[BDVECT+020];
word * array = data+BDVECT+021;
word & dbdesc = data[BDVECT+030];
word & rootfl = data[BDVECT+031];
word & savedp = data[BDVECT+032];
word & frebas = data[BDVECT+034];
word & adescr = data[BDVECT+035];
word & limit = data[BDVECT+036];
word & curkey = data[BDVECT+037];
word & endmrk = data[BDVECT+040];
word & desc1 = data[BDVECT+041];
word & desc2 = data[BDVECT+042];
word & e70msk = data[BDVECT+044];
word & curpos = data[BDVECT+045];
word & aitem = data[BDVECT+046];
word & itmlen = data[BDVECT+047];
word & loc50 = data[BDVECT+050];
word & dblen = data[BDVECT+051];
word & loc53 = data[BDVECT+053];
word & loc54 = data[BDVECT+054];
word & loc55 = data[BDVECT+055];
word & loc56 = data[BDVECT+056];
word & loc116 = data[BDVECT+0116];
word & loc117 = data[BDVECT+0117];
word & loc120 = data[BDVECT+0120];
word & loc157 = data[BDVECT+0157];
word & loc160 = data[BDVECT+0160];
word & loc220 = data[BDVECT+0220];
word & curbuf = data[BDVECT+0241];
word & idx = data[BDVECT+0242];
word & which = data[BDVECT+0244];
word & loc246 = data[BDVECT+0246];
word & dirty = data[BDVECT+0247];

union Accumulator {    
    uint64_t d;
    struct {
        unsigned offset : 10;
        unsigned dummy  : 23;
        unsigned stamp  : 15;
    };
    struct {
        unsigned addr   : 15;
    };
    operator uint64_t() { return d; }
    uint64_t& operator=(uint64_t x) { d = x; return d; }
    uint64_t& operator&=(uint64_t x) { d &= x; return d; }
    uint64_t& operator|=(uint64_t x) { d |= x; return d; }
    uint64_t& operator^=(uint64_t x) { d ^= x; return d; }
    uint64_t& operator+=(uint64_t x) { d += x; return d; }
    uint64_t& operator-=(uint64_t x) { d -= x; return d; }
    uint64_t& operator<<=(int x) { d <<= x; return d; }
    uint64_t& operator>>=(int x) { d >>= x; return d; }
    uint64_t& operator++() { ++d; return d; }
    uint64_t& operator--() { --d; return d; }
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
word & negkey = data[BDSYS+027];
word & d00030 = data[BDSYS+030];
word & d00031 = data[BDSYS+031];
word & d00032 = data[BDSYS+032];
word & d00033 = data[BDSYS+033];
word & remlen = data[BDSYS+034];
word & work = data[BDSYS+035];
word & temp = data[BDSYS+036];
word & ise70 = data[BDSYS+037];
word & d00040 = data[BDSYS+040];
word & savrk = data[BDSYS+041];

const char * msg[] = {
    "Zero key", "Page corrupted", "No such record", "Invalid name",
    "1st page corrupted", "Overflow", "Step too big", "No such name",
    "Name already exists", "No end symbol", "Internal error", "Record too long",
    "DB is locked", "No current record", "No prev. record", "No next record",
    "Wrong password" };

void e70(word is) {
    int page = (is >> 30) & BITS(5);
    char nuzzzz[7];
    sprintf(nuzzzz, "%06o", int(is.d & BITS(18)));
    if (is.d & ONEBIT(40)) {
        // read
        FILE * f = fopen(nuzzzz, "r");
        if (!f) {
            std::cerr << "Reading zone " << nuzzzz << " which does not exist yet\n";
            return;
        }
        if (verbose)
            std::cerr << "Reading " << nuzzzz
                      << " to address " << std::oct << page*1024 << '\n';
        fread(data+page*1024, 1024, sizeof(uint64_t), f);
        fclose(f);
    } else {
        // write
        FILE * f = fopen(nuzzzz, "w");
        if (!f) {
            std::cerr << "Could not open " << nuzzzz << '(' << strerror(errno) << ")\n";
            exit(1);
        }
        if (verbose)
            std::cerr << "Writing " << nuzzzz
                      << " from address " << std::oct << page*1024 << '\n';
        fwrite(data+page*1024, 1024, sizeof(uint64_t), f);
        fclose(f);

        std::string txt(nuzzzz);
        txt += ".txt";
        FILE * t = fopen(txt.c_str(), "w");
        if (!t) {
            std::cerr << "Could not open " << txt << '(' << strerror(errno) << ")\n";
            exit(1);
        }
        int zone = is.d & 07777;
        for (int i = 0; i < 1024; ++i) {
            fprintf(t, "%04o.%04o:  %04o %04o %04o %04o\n",
                    zone, i, int(data[page*1024+i] >> 36),
                    int((data[page*1024+i] >> 24) & 07777),
                    int((data[page*1024+i] >> 12) & 07777),
                    int((data[page*1024+i] >> 0) & 07777));
        }
        fclose(t);
    }
}

void getzon() {
    which = acc;
    acc |= rootfl.d;
    zonkey = acc;
    acc &= 01777;
    acc = acc ? bdbuf.d : bdtab.d;
    m16 = curbuf = acc;
    if (m16[0] == zonkey)
        return;
    if (which.d && (dirty.d & 2)) {
        dirty = dirty.d & ~2;
        ise70 = (e70msk.d | bdbuf.d << 20) + m16[0].d;
        e70(ise70);
    }
    ise70 = (curbuf.d << 20 | which.d | ONEBIT(40)) + e70msk.d;
    e70(ise70);
    m16 = curbuf;
    if (m16[0] == zonkey)
        return;
    std::cerr << "Corruption: zonkey = " << std::oct << zonkey.d
              << ", data = " << (*m16).d <<"\n";
    throw ERR_BAD_PAGE;
}

void save(bool force_tab = false) {
    if (force_tab || (dirty.d & 1)) {
        ise70 = e70msk.d | bdtab.d << 20;
        e70(ise70);
    }
    if (dirty.d & 2) {
        ise70 = (e70msk.d | bdbuf.d << 20) + (bdbuf[0].d & 01777);
        e70(ise70);
    }
    dirty = 0;
}

void finalize() {
    d00011 = acc;
    bool force_tab = false;
    if (sync != 0) {
        if (sync != 1)
            return;
        m16 = bdtab;
        if (m16[1].d & (1 << 31)) {
            m16[1].d &= ~(1 << 31);
            force_tab = true;
        }
    }
    save(force_tab);
}

void date() {
    time_t t = time(nullptr);
    struct tm & l = *localtime(&t);
    int d = l.tm_mday, m = l.tm_mon + 1, y = l.tm_year + 1900;
    uint64_t stamp = (d/10*16+d%10) << 9 |
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
    usrloc = &d00010;
    data[FAKEBLK].d = 2;
    data[FAKEBLK+1].d = d00011.d;
    data[FAKEBLK+2].d = loc20.d;
    m16 = usrloc.d = FAKEBLK;
}

// Returns result in itmlen and acc
void usable_space() {
    itmlen = 0;
    for (int i = dblen.d - 1; i >= 0; --i) {
        if (frebas[i].d != 0)
            itmlen = itmlen.d + frebas[i].d - 1;
    }
    acc = itmlen.d;
    m16 = 0;
}

void find_item() {
    d00040 = acc;
    acc &= 01777;
    getzon();
    acc = d00040.d & 03776000;
    if (!acc)
        throw ERR_ZEROKEY;      // Attempting to compute length of a placeholder?
    work = acc;
    acc <<= 29;
    acc &= BITS(48);
    temp = acc;
    acc = m16[1].d;
    acc &= 03776000;
    if (!acc)
        throw ERR_NO_RECORD;    // Attempting to compute length of a deleted record?
    if (acc >= work.d)
        acc = work.d;
    acc >>= 10;
    loc116 = acc;
    do {
        acc = loc116[m16+1].d;
        loc220 = acc;
        acc ^= temp.d;
        acc &= 0777LL << 39;
        if (acc) {
            if (--loc116.d)
                continue;
            throw ERR_NO_RECORD;
        }
    } while (false);
    acc = loc220.d & 01777;
    ++acc;
    acc += curbuf.d;
    aitem = acc;
    acc = loc220.d & 03776000;
    acc >>= 10;
    curpos = acc;
}

void set_header() {
    *aitem = acc;
    dirty(1);
}

void eval() try {
    acc = m16.d;
    jump(enter);
    // The switch is entered with m6 = cmd0 and acc = 0
  switch_:
    if (verbose)
        std::cerr << "Executing microcode " << std::oct << m5.d << '\n';
    acc = 0;
    switch (m5.d) {
    case 0:
        jump(cmd0);
    case 1:
        negkey = 0;
        jump(cmd1);
    case 2:
        acc = BITS(47);
        jump(find);
    case 3:
        acc = 1;
        // FALL THROUGH
    case 4:
        m16 = acc;
        jump(cmd4);
    case 5:
        d00011 = 0;
        make_metablock();
        jump(alloc);
    case 6:
        idx = 0;
        jump(cmd6);
    case 7: k07:
        acc = (char*)&&a00317-(char*)&&frombase;
        jump(cmd7);
    case 010:
        which = 0;
        jump(mkctl);
    case 011:
        acc = key.d;
        jump(find);
    case 012:
        acc = m13.d;
        jump(cmd12);
    case 013:
        itmlen = 0;
        usable_space();
        jump(*m6.p);
    case 014:
        acc = curkey.d;
        jump(ckexst);
    case 015:
        acc = curkey.d;
        jump(ckdnex);
    case 016:
        acc = -mylen.d;
        jump(cmd16);
    case 017:
        acc = -mylen.d;
        jump(cmd17);
    case 020:
        acc = myloc.d;
        jump(cmd20);
    case 021:
        acc = myloc.d;
        jump(alloc);
    case 022:
        acc = adescr.d;
        jump(cpyout);
    case 023:
        acc = adescr.d;
        jump(free);
    case 024:
        acc = givenp;
        jump(chkpsw);
    case 025:
        acc = dbdesc.d;
        jump(setctl);
    case 026:
        acc = key.d;
        jump(inskey);
    case 027:
        limit = 0;
        jump(delkey);
    case 030:
        acc = orgcmd.d;
        jump(next);
    case 031:
        acc = 04000;
        jump(root);
    case 032:
        acc = loc12.d;
        jump(cmd32);
    case 033:
        acc = adescr.d;
        jump(pr342);
    case 034:
        acc = desc1.d;
        jump(cmd34);
    case 035:
        acc = dirty.d;
        save();
        return;
    case 036:
        acc = loc12.d;
        jump(cmd36);
    case 037: k37:
        acc = curcmd >> 12;
        // FALL THROUGH
    case 040:
        jump(next);
    case 041:
        acc = (char*)&&compar-(char*)&&frombase;
        jump(k43);
    case 042:
        acc = (char*)&&tobase-(char*)&&frombase;
        // FALL THROUGH
    case 043: k43:
        jmpoff = acc;
        jump(cmd43);
    case 044:
        acc = desc1.d;
        jump(cmd44);
    case 045:
        acc = bdtab.d;
        jump(lock);
    case 046:
        acc = myloc.d;
        jump(cmd46);
    case 047:
        acc = desc1.d;
        jump(*outadr.p);
    case 050:
        acc = &orgcmd-BDVECT;
        jump(cmd50);
    case 051:
        acc = &myloc-BDVECT;
        jump(cmd51);
    case 052:
        acc = curcmd.d;
        jump(cmd52);
    case 053:
        acc = curcmd.d;
        jump(move);
    case 054:
        acc = curcmd.d;
        jump(cmd54);
    case 055:
        return;
    default:
        std::cerr << std::oct << m5.d << " NYI\n";
        abort();
    }
  cmd32:
    *aitem = acc;
    dirty(2);
    jump(*m6.p);
  enter:
    savm16 = acc;
    savm13 = m13;
    savrk = rk;
    m13 = abdv;
    savm7 = m7;
    savm5 = m5;
    if (bdtab[0] != rootfl && e70msk.d) {
        ise70 = bdtab.d << 20 | ONEBIT(40) | e70msk.d;
        e70(ise70);
    }
    savm6 = m6;
  enter2:
    acc = orgcmd.d;
    jump(next);
  drtnxt:
    dirty = dirty.d | 1;
  rtnext:
    if (goto_.p) {
        void * to = goto_.p;
        goto_ = 0;
        jump(*to);
    }
    goto_ = 0;
  cmd0:
    acc = curcmd >> 6;
  next:
    if (!acc) {
        finalize();
        return;
    }
    curcmd = acc;
    m5 = acc & BITS(6);
  a00153:
    m6.p = &&cmd0;
    goto_ = 0;
    jump(switch_);
  find:
    negkey = acc;
    if (!acc || acc & ONEBIT(48))
        throw ERR_INV_NAME;
    acc = 0;
  cmd1:
    idx = acc;
    negkey = negkey.d ^ BITS(47);
  cmd1a:
    m16 = &loc56;
    acc = loc20.d;
    if (!acc) {
        if (verbose)
            std::cerr << "Loc20 == 0, nothing to compare\n";
        jump(a00212);
    }
  a00164:
    acc = m16[-1].d & 01777;
    m5 = acc;
    if (verbose)
        std::cerr << "Comparing " << std::dec << acc/2 << " elements\n";
    while (m5.d) {
        acc = m5[m16-2].d;
        acc += negkey.d;
        acc = (acc + (acc >> 48)) & BITS(48);
        if (!(acc & ONEBIT(48)))
            break;
        m5.d -= 2;
    }
    m5.d -= 2;
    acc = m5.d;
    acc |= ONEBIT(31);
    array[idx.d] = acc;
    acc = m16[1].d;
    acc &= ONEBIT(48);
    if (!acc)
        jump(a00371);
    idx = idx.d + 2;
    acc = m16[m5+1].d;
    m6.p = &&a00215;
  pr202:
    m16 = &loc116;
  a00203:
    array[idx.d-1] = acc;
    acc &= 01777777;
    loc116 = acc;
    acc = m16.d;
    usrloc = acc;
    acc += 2;
    loc50 = acc;
    acc = loc116.d ^ loc246.d;
    if (!acc)
        jump(*m6.p);
    acc = loc116.d;
    loc246 = acc;
    find_item();
    jump(a01423);
  a00212:
    m6.p = &&cmd1a;
  a00213:
    acc = 02000;
    m16 = &loc54;
    jump(a00203);
  a00215:
    m16 = &loc120;
    jump(a00164);
  cmd54:                        // mem[bdvec[next_insn]] = bdvec[012] ?
    curcmd = acc >>= 6;
    acc &= 077;
    m16 = acc;
    *m13[m16] = loc12;
    jump(*m6.p);
  move:                         // bdvec[next_insn] = bdvec[next_next_insn]
    curcmd = acc >>= 6;
    acc &= 077;
    m16 = acc;
    curcmd = acc = curcmd >> 6;
    acc &= 077;
    m5 = acc;
    acc = abdv[m5].d;
    jump(stbdv);
  cmd50:                        // cmd := bdvec[bdvec[next_insn]++]
    m6.p = &&next;
  cmd51:                        // myloc := bdvec[bdvec[next_insn]++]
    m16 = acc;
    curcmd = acc = curcmd >> 6;
    acc &= 077;
    m5 = acc;
    m5 = m5.d + m13.d;
    ++*m5;
    acc = (*m5)[-1].d;
  stbdv:
    m13[m16] = acc;
    jump(*m6.p);
  cmd52:                        // bdvec[next_insn] := bdvec[bdvec[next_next_insn]++]
    curcmd = acc >>= 6;
    acc &= 077;
    jump(cmd51);
  cmd46:
    acc >>= 18;
    acc &= 077777;
    desc2 = acc -= loc53.d;
    acc = (myloc >> 33) & 017777;
    if (!acc)
        jump(skip5);
    mylen = acc;
    jump(*m6.p);
  lock:
    acc <<= 20;
    work = acc |= e70msk.d;
    ise70 = acc |= 1LL << 39;
    e70(ise70);
    bdtab[1] = acc = bdtab[1].d ^ (1 << 31);
    if (!(acc & (1 << 31)))
        throw ERR_LOCKED;
    bdbuf[0] = acc;
    e70(work);
    jump(*m6.p);
  cmd44:
    adescr = acc;
    jump(*m6.p);
  cmd36:
    m16 = loc50;
    m16[array[idx.d]+1] = acc;
    jump(a01160);
  a00267:
    find_item();
  a00270:
    acc = negkey.d;
    m16 = acc;
    if (acc < curpos.d)
        jump(a00274);
    negkey = acc-curpos.d;
    m16 = curpos;
    m5 = 0;
  a00274:
    jump(*((char*)&&frombase+jmpoff.d));
  frombase:
    if (!m16.d)
        jump(done);
    usrloc[m16-1] = aitem[m16-1];
    --m16;
    jump(frombase);
  tobase:
    dirty(1);
    if (!m16.d)
        jump(done);
    aitem[m16-1] = usrloc[m16-1];
    --m16;
    jump(tobase);
  compar:
    // Check for 0 length is missing?
    if (aitem[m16-1] != usrloc[m16-1])
        jump(k37);
    cycle(compar,m16);
  done:
    usrloc = usrloc.d + curpos.d;
  a00317:
    if (!m5.d)
        jump(a00325);
    aitem = aitem.d + negkey.d;
    curpos = curpos.d - negkey.d;
    acc = (*aitem).d;
    desc1 = acc;
    jump(*m6.p);
  a00325:
    acc = loc220.d;
    acc >>= 20;
    acc &= 01777777;
    if (acc)
        jump(a00267);
    acc = jmpoff.d;
    acc ^= (char*)&&done-(char*)&&frombase;
    if (!acc)
        jump(a00332);  // impossible? should compare with &&a00317 ?
    acc = negkey.d;
    if (!acc)
        jump(*m6.p);
  a00332:
    m16 = 7;
    jump(skip);
  cmd7:
    jmpoff = acc;
    acc = desc2.d;
  a00334:
    negkey = acc;
    loc53.d += acc;
    usrloc = myloc;
    jump(a00270);
  cmd43:
    acc = mylen.d;
    jump(a00334);
  a00340:
    find_item();
    jump(k07);
  pr342:
    find_item();
    acc = (*aitem).d;
    desc1 = acc;
    acc &= 077777;
    itmlen = acc;
    loc53 = 0;
    jump(*m6.p);
  chkpsw:
    if (acc == savedp.d)
        jump(cmd0);
    throw ERR_WRONG_PASSWORD;
  ckdnex:
    if (acc != key.d)
        jump(cmd0);
    m16 = ERR_EXISTS;
    jump(skip);
  ckexst:
    if (acc == key.d)
        jump(cmd0);
    m16 = ERR_NO_NAME;
    jump(skip);
  a00355:
    acc += 2;
    m5 = acc;
    acc ^= loc50[-1].d & 01777;
    if (acc)
        jump(a00367);
    acc = loc50[-1].d;
    acc &= 01777777LL << 10;
    if (!acc)
        jump(nonext);
    acc >>= 10;
    call(pr202,m6);
    array[idx.d-2] = array[idx.d-2].d + (1<<15);
    m5 = 0;
  a00367:
    acc = m5.d | 1 << 30;
    array[idx.d] = acc;
  a00371:
    acc = loc50[m5].d;
    curkey = acc;
    acc = loc50[m5+1].d;
    adescr = acc;
    jump(rtnext);
    // pr376 (make_metablock) was here
  cmd6:
    itmlen = 041;
    m16 = &loc54;
    acc = adescr.d;
    jump(a00203);
  step:
    goto_ = m6;
  cmd4:
    acc = array[idx.d].d;
    if (!acc)
        throw ERR_NO_CURRENT;
    m5 = acc;
    if (!m16.d)
        jump(a00355);
    if (!m5.d)
        jump(a00415);
  a00414:
    utm(m5,-2);
    jump(a00367);
  a00415:
    acc = loc50[-1] >> 29;
    if (!acc)
        jump(nopred);
    call(pr202,m6);
    acc = array[idx.d-2].d;
    acc -= 1<<15;
    array[idx.d-2] = acc;
    acc = loc117.d & 01777;
    m5 = acc;
    jump(a00414);
  cmd16: {
    // Searching for the end mark (originally 1 or 2 bytes)
    m16 = acc;
    temp = myloc.d + mylen.d;
    // Handling only single byte mark, contemporary style
    char * start = (char*)&(temp[m16].d);
    char * end = (char*)&(temp[0].d);
    char * found = (char*)memchr(start, endmrk.d & 0xFF, end-start);
    if (!found)
        throw ERR_NO_END_MARK;
    mylen = (found - start) / 8 + 1;
    jump(cmd0);
    }
  cmd17:    
    // Searching for the end word
    m16 = acc;
    temp = myloc.d + mylen.d;
  loop_here:
    acc = temp[m16].d ^ endmrk.d;
    if (!acc) {
        m16 = m16.d + mylen.d + 1;
        mylen = m16;
        jump(cmd0);
    }
    if (m16 != 0) {
        ++m16;
        jump(cmd17);            // Possibly a typo/oversight, ought to be loop_here
    }
    throw ERR_INTERNAL;         // Originally "no end word"
  skip:
    acc = curcmd.d;
    acc >>= 6;
    if (!acc || (acc & 077))
        throw m16.d;
  skip5:
    acc = curcmd.d;
    acc >>= 30;
    jump(next);
  overfl: throw ERR_OVERFLOW;
  nopred: m16 = 15; jump(skip);
  nonext: m16 = 16; jump(skip);
  root:
    rootfl = acc;
    dbdesc = acc = arch;
    jump(setctl);
  cmd12:
    acc += &dbdesc-BDVECT;
    myloc = acc;
    mylen = 3;
    acc = adescr.d;
    jump(cpyout);
  setctl:
    e70msk = acc & 0777777;
    dblen = dbdesc >> 18;
    ise70 = bdtab.d << 20 | e70msk.d | ONEBIT(40);
    e70(ise70);
    m16 = curbuf = bdtab;
    acc = (*m16).d;
    if (acc != rootfl.d)
        throw ERR_BAD_CATALOG;
    idx = 0;
    loc246 = 0;
    which = 0;
    acc = m16[3].d & 01777;
    acc += bdtab.d + 2;
    frebas = acc;
    jump(a00213);
  a00667:
    acc = idx.d;
    if (!acc)
        jump(a01160);
    acc = loc246.d;
    call(free,m6);
    desc2 = 1;
    acc = loc117.d;
    d00025 = acc;
    acc &= 01777777LL << 29;
    d00032 = acc;
    acc >>= 29;
    if (acc) {
        call(a00340,m6);
        acc &= ~(01777777LL << 10);
        loc117 = acc;
        acc = d00025.d;
        acc &= 01777777LL << 10;
        acc |= loc117.d;
        set_header();
    }
    call(pr1232,m5);
  delkey:
    acc = loc50.d;
    m16 = acc;
    acc += array[idx.d].d;
    m5 = acc;
    acc ^= loc50.d;
    loc116 = acc;
    acc = m16[-1].d;
    acc &= 01777;
    temp = acc;
    acc += loc50.d;
    work = acc;
    do {
        acc = m5[2].d;
        *m5 = acc;
        ++m5;
        acc = m5.d;
        acc ^= work.d;
    } while (acc);
    acc = temp.d - 2;
    acc ^= temp.d;
    acc ^= m16[-1].d;
    m16[-1] = acc;
    acc &= 01777;
    if (!acc)
        jump(a00667);
    acc = loc116.d;
  a00721:
    acc &= 077777;
    if (acc)
        jump(a01160);
    d00011 = m16[0];
    goto_ = size_t(&&a00730);
    d00033 = size_t(&&a00731);
    jump(a01160);
  a00730:
    jump(a01242);
  a00731:
    acc = d00011.d;
    m16 = loc50;
    m16[array[idx.d]] = acc;
    acc = array[idx.d].d;
    jump(a00721);
  free:
    find_item();
    acc = m16[1].d;
    acc = (acc >> 10) & 077777;
    temp = acc;
    acc = loc116.d;
    m5 = acc;
    do {
        acc ^= temp.d;
        if (!acc)
            break;
        m16[m5+1] = m16[m5+2].d;
        ++m5;
        acc = m5.d;
    } while (true);
  a00747:
    acc = loc220.d & 01777;
    d00031 = acc;
    do {
        utm(m5,-1);
        if (!m5.d)
            break;
        acc = m16[m5+1].d & 01777;
        if (acc >= d00031.d)
            continue;
        acc = m16[m5+1].d + curpos.d;
        m16[m5+1] = acc;
    } while (true);
    work = m16[1].d & 01777;
    if (work == d00031)
        jump(a00773);
    if (d00031.d < work.d)
        throw ERR_INTERNAL;
    m5 = (d00031.d - work.d) & 077777;
    work = work.d + curbuf.d;
    d00031 = work.d + curpos.d;
  a00767:
    d00031[m5] = work[m5];
    cycle(a00767,m5);
  a00773:
    acc = m16[1].d;
    acc -= 02000;
    acc += curpos.d;
    m16[1] = acc;
    m16 = frebas;
    acc = which[m16].d;
    acc += curpos.d + 1;
    which[m16] = acc;
    dirty(1);
    acc = loc220.d;
  a01005:
    acc >>= 20;
    acc &= 01777777;
    if (acc)
        jump(free);
    dirty = dirty.d | 1;
    jump(*m6.p);
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
    work = which[m16];
    ++mylen;
    if (mylen.d >= work.d) {
        if (verbose)
            std::cerr << "mylen = " << mylen.d << " work = " << work.d << '\n';
        jump(nospac);
    }
    acc = which.d;
  chunk:
    getzon();
    m7 = m16;
    acc = m7[1].d;
    acc >>= 10;
    m16 = acc;
  a01035:
    acc <<= 39;
    acc ^= curbuf[m16+1].d;
    acc &= 0777LL << 39;
    if (acc) {
        curbuf[m16+2] = curbuf[m16+1];
        utm(m16, -1);
        acc = m16.d;
        if (m16.d)
            jump(a01035);
    }
  a01044:
    utm(m16, 1);
    acc = m16.d;
    loc116 = acc;
    acc <<= 39;
    acc &= BITS(48);
    acc |= d00030.d;
  pr1047:
    d00030 = acc;
    acc >>= 39;
    acc <<= 10;
    acc |= which.d;
    d00024 = acc;
    m16 = mylen;
    acc = m7[1].d - m16.d;
    acc += 02000;
    m7[1] = acc;
    acc &= 01777;
    work = acc;
    acc += 1 + curbuf.d;
    temp = acc;
    if (!m5.d) {
        utm(m16,-1);
        ++temp;
    }
  a01062:
    if (m16.d) {
        if (verbose)
            std::cerr << "To DB: "  << std::oct << m16.d << "(8) words from "
                      << usrloc.d << " to " << temp.d << '\n';
      a01063:
        temp[m16-1] = usrloc[m16-1];
        cycle(a01063, m16);
    }
  a01067:
    loc116[m7+1] = (mylen.d << 10) | work.d | d00030.d;
    m16 = frebas;
    if (verbose)
        std::cerr << "Reducing free " << std::oct << which[m16].d
                  << " by len " << mylen.d << " + 1\n";
    which[m16] = which[m16].d - (mylen.d+1);
    if (verbose)
        std::cerr << "Got " << which[m16].d << '\n';
    if (!m5.d)
        jump(a01111);
    dirty(1);
    acc = d00024.d;
    acc <<= 20;
    acc &= BITS(48);
    d00030 = acc;
    mylen = remlen;
  a01105:
    cycle(a01264, m5);
    m6.p = &&overfl;
    acc = d00030.d;
    jump(a01005);
  a01111:
    temp[-1] = datlen;
    dirty(2);
    jump(*m6.p);
  a01116:
    loc20 = 0;
    acc = loc12.d;
    m6.p = &&overfl;
    jump(free);
  inskey:
    d00011 = acc;
    acc = loc12.d;
    adescr = acc;
    jump(a01140);
  a01123:
    acc = array[idx.d].d;
    acc >>= 15;
  a01125:
    d00012 = acc;
    acc >>= 15;
    m16 = acc;
    if (!(d00012.d & 077777))
        jump(*d00033.p);
    acc = d00012.d;
    utm(m16, -1);
    if (!m16.d)
        jump(a01133);
    acc += 2;
  a01133:
    --acc;
    d00012 = acc;
    call(step,m6);
    acc = d00012.d;
    jump(a01125);
  a01136:
    acc = d00032.d;
    acc >>= 29;
    acc |= ONEBIT(48);
  a01140:
    d00024 = acc;
    acc = loc50.d;
    m16 = acc & 077777;
    acc += array[idx.d].d + 2;
    acc &= 077777;
    m5 = acc;
    limit = acc;
    acc = m16[-1].d;
    acc &= 01777;
    m6 = acc;
    utm(m6, m16.d);
    if (verbose)
        std::cerr << "Expanding " << std::dec << (m6.d-limit.d)/2 << " elements\n";
    while (m6 != limit) {
        m6[0] = m6[-2];
        m6[1] = m6[-1];
        utm(m6, -2);
    }
    m5[0] = d00011;
    m5[1] = d00024;
    m16[-1] = m16[-1].d + 2;
  a01160:
    acc = m16.d;
    usrloc = --acc;
    acc = idx.d;
    if (acc)
        jump(a01167);
  a01163:
    mylen = 041;
    if (m16[-1].d == 040)
        jump(a01252);
    acc = loc20.d;
    jump(a01311);
  a01167:
    acc = m16[-1].d & 01777;
    acc ^= 0100;
    if (!acc)
        jump(a01174);
  a01171:
    acc = m16[-1].d & 01777;
    mylen = ++acc;
    acc = loc246.d;
    jump(a01311);
  a01174:
    work = (idx.d * 2) + 041;
    acc = loc55.d ^ 036;
    if (acc)
        jump(a01201);
    acc = work.d + 044;
    work = acc;
  a01201:
    usable_space();
    if (acc < work.d)
        jump(a01116);
    d00025 = loc157;
    loc157 = (loc246.d << 29) & BITS(48);
    acc = loc117.d;
    acc &= 01777777LL << 10;
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
    acc &= 01777777LL << 10;
    acc <<= 19;
    acc &= BITS(48);
    d00032 = acc;
    m16 = &loc120;
    m5.p = &&a01231;
    goto_ = m5;
    usrloc = m16.d - 1;
    jump(a01171);
  a01231:
    m5.p = &&a01136;
  pr1232:
    d00033 = m5.d;
    desc2 = 1;
    acc = (d00025 >> 10) & BITS(19);
    if (!acc)
        jump(a01242);
    call(a00340,m6);
    acc &= BITS(29);
    acc |= d00032.d;
    set_header();
  a01242:
    m16 = &loc56;
    loc50 = m16;
    acc = idx.d;
    if (!acc)
        jump(drtnxt);
    acc -= 2;
    idx = acc;
    if (!acc)
        jump(a01123);
    m6.p = &&a01123;
    acc = array[idx.d-1].d;
    jump(pr202);
  a01252:
    usable_space();
    if (acc < 0101)
        jump(a01116);
    acc -= 0101;
    call(pr1022,m6);
    m16 = &loc56;
    acc = d00024.d;
    acc |= ONEBIT(48);
    m16[1] = acc;
    m16[-1] = 2;
    jump(a01163);
  long_:
    acc = usrloc.d;
    acc += mylen.d - 1;
    usrloc = acc;
    m5 = dblen;
  a01264:
    if (frebas[m5-1].d < 2)
        jump(a01105);
    acc = frebas[m5-1].d - 2 + 1;
    work = acc;
    --m5;
    if (acc < mylen.d)
        jump(a01274);
    acc = usrloc.d;
    acc -= mylen.d;
    acc += 1;
    usrloc = acc;
    acc = m5.d;
    m5 = 0;
    jump(chunk);
  a01274:
    remlen = mylen.d - work.d;
    mylen = work.d;
    usrloc = usrloc.d - work.d;
    acc = m5.d;
    ++m5;
    jump(chunk);
  nospac:
    if (mylen.d >= 01775)
        jump(long_);
    m7 = 077777;
  a01303:
    ++m7;
    if (m7.d == dblen.d)
        jump(long_);
    if (mylen.d >= frebas[m7].d)
        jump(a01303);
    acc = m7.d;
    jump(chunk);
  cmd20:
    usrloc = acc;
    acc = adescr.d;
  a01311:
    d00012 = acc;
    m6.p = &&drtnxt;
    find_item();
    --acc;
    m5 = acc;
    acc ^= mylen.d;
    if (acc)
        jump(a01336);
    if (!m5.d)
        jump(a01323);
  a01317:
    acc = usrloc[m5-1].d;
    aitem[m5] = acc;
    cycle(a01317, m5);
  a01323:
    m5 = m16;
    date();
    *aitem = acc;
    dirty(1);
    acc = loc220.d;
    acc &= 01777777LL << 20;
    if (!acc)
        jump(rtnext);
    acc ^= loc220.d;
    loc116[m5+1] = acc;
    acc = loc220.d;
    jump(a01005);
  a01336:
    date();
    m16 = curbuf;
    acc = m16[1] >> 10;
    m5 = acc;
    utm(m5, 1);
    acc = m16[1].d - acc;
    acc &= 01777;
    acc += curpos.d;
    acc -= 2;
    work = acc;
    if (acc < mylen.d)
        jump(a01361);
  a01346:
    d00010 = m6;
    d00026 = loc220;
    loc220 = loc220.d & 01777;
    call(a00747,m6);
    ++mylen;
    m5 = 0;
    m7 = curbuf;
    acc = d00026.d & (0777LL << 39);
    call(pr1047,m6);
    acc = d00026.d;
    m6.p = d00010.p;
    jump(a01005);
  a01361:
    usable_space();
  a01362:
    acc = (*aitem).d & 077777;
    acc += itmlen.d;
    if (acc < mylen.d)
        jump(overfl);
    m16 = curbuf;
    mylen = work.d;
    call(a01346,m6);
    usrloc = usrloc.d + mylen.d;
    mylen = (datlen.d & 077777) - mylen.d;
    datlen = usrloc[-1];
    call(a01023,m6);
    acc = d00012.d;
    find_item();
    acc = (d00024.d << 20) & BITS(48);
    acc |= loc220.d;
    loc116[m16+1] = acc;
    dirty(2);
    jump(rtnext);
  cmd34:
    sprintf((char*)&endmrk.d, " %05o", int(acc & 077777));
    acc = desc1.d;
    // Timestamp; OK to spill to desc2
    sprintf((char*)&desc1.d, " %d%d.%d%d.X%d", int((acc >> 46) & 3),
            int((acc >> 42) & 15), int((acc >> 41) & 1),
            int((acc >> 37) & 15), int((acc >> 33) & 15));
    jump(cmd0);
  cpyout:
    call(pr342,m6);
  a01415:
    m6.p = &&cmd0;
    usrloc = myloc;
    acc = mylen.d;
    if (acc) jump(chklen);
  enough:
    ++aitem;
    --curpos;
    acc = curpos.d;
  a01423:
    m16 = acc;
    acc &= 077777;
    if (!acc)
        jump(a01431);
    if (verbose)
        std::cerr << "From DB: " << std::oct << m16.d << "(8) words from "
                  << aitem.d << " to " << usrloc.d << '\n';
  coloop:
    acc = aitem[m16-1].d;
    usrloc[m16-1] = acc;
    cycle(coloop, m16);
  a01431:
    acc = usrloc.d;
    acc += curpos.d;
    usrloc = acc;
    acc = loc220.d;
    acc &= 01777777LL << 20;
    if (!acc)
        jump(*m6.p);
    acc >>= 20;
    find_item();
    jump(a01423);
  chklen:
    if (acc >= itmlen.d)
        jump(enough);
    throw ERR_TOO_LONG;
  mkctl: {
        m5 = dblen = dbdesc >> 18;
        e70msk = dbdesc.d & 0777777;
        m6 = bdtab;
        curbuf = m6;
        temp = e70msk.d | bdtab.d << 20;
        bdtab[1] = 01777;
      a01447:
        utm(m5, -1);
        bdbuf[m5] = 01776;
        bdtab[0] = rootfl.d | m5.d;
        ise70 = temp.d + m5.d;
        e70(ise70);
        if (m5.d)
            jump(a01447);
        myloc = frebas = bdbuf;
        loc20 = 02000;
        d00011 = 0;
        make_metablock();
        call(pr1022, m6);
        mylen = dblen.d;
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
    std::cerr << "ERROR " << std::dec << m16.d
              << " (" << msg[m16.d-1] << ")\n";
    d00010 = *(uint64_t*)msg[m16.d-1];
    acc = *((uint64_t*)msg[m16.d-1]+1);
    finalize();
}

void pasbdi() {
    // Faking MARS words
    m16 = 04700;
    m13 = 01654;
    m7 = 01603;
    m5 = 077770;
    m6 = 022731;
    // Page locations of P/BDTAB and P/BDBUF in a mid-sized test program
    bdtab = 010000;
    bdbuf = 012000;
    data[BDSYS+0042] = 02300'0150'0000'0000;
    data[BDSYS+0043] = 02000'0027'2300'0160;
    data[BDSYS+0044] = 02010'1532'2300'0156;
    data[BDSYS+0045] = 02010'1533'0000'0000;
    data[BDSYS+0046] = 00040'0016'2300'0410;
    data[BDSYS+0047] = 02000'0011'2300'0375;
    data[BDSYS+0050] = 05400'0242'2300'0404;
    abdv = BDVECT;
    for (int i = 0; i < 041; ++i)
      data[FAKEBLK+i].d = 01234567007654321LL;
}

void SetDB(int lnuzzzz) {
    pasbdi();
    arch = lnuzzzz;
}

void InitDB(int lnuzzzz) {
    pasbdi();
    dbdesc = lnuzzzz;
    rootfl = 04000;
    orgcmd = 010;
    eval();
}

// Replicating what the NEWD option does in the PAIB Pascal API function
void newd(const char * k, int lnuzzzz) {
    if (verbose)
        std::cerr << "Running newd('" << k << "', " << std::oct << lnuzzzz << ")\n";
    key = *reinterpret_cast<const uint64_t*>(k);
    data[02121] = lnuzzzz;
    data[02122] = (key.d << 10) & BITS(48);
    key = *reinterpret_cast<const uint64_t*>(k);
    mylen = 1;
    myloc = 01654;
    orgcmd = 0;
    myloc = 02121;
    mylen = 2;
    orgcmd = 02621151131LL;
    sav = bdvect;
    eval();
    orgcmd = 010121411;
    eval();
}

void opend(const char * k) {
    if (verbose)
        std::cerr << "Running opend('" << k << "')\n";
    key = *reinterpret_cast<const uint64_t*>(k);
    orgcmd = 02512141131;
    eval();
}

void putd(const char * k, int loc, int len) {
    if (verbose)
        std::cerr << "Running putd('" << k << "', '"
                  << reinterpret_cast<char*>(data+loc) << "':" << len << ")\n";
    key = *reinterpret_cast<const uint64_t*>(k);
    mylen = len;
    myloc = loc;
    orgcmd = 026211511;
    eval();
}

void putd(uint64_t k, int loc, int len) {
    if (verbose) {
        std::cerr << "Running putd(" << std::oct << std::setw(16) << std::setfill('0') << k << ", ";
        std::cerr << std::setw(5) << loc << ":" << std::setw(0) << std::dec << len << ")\n";
        std::cerr.copyfmt(std::ios(nullptr));
    }      
    key = k;
    mylen = len;
    myloc = loc;
    orgcmd = 026211511;
    eval();
}

void putd(const char * k, const char * v) {
    size_t len = (strlen(v)+7)/8;
    if (verbose)
        std::cerr << "Running putd('" << k << "', '"
                  << v << "':" << len << ")\n";
    key = *reinterpret_cast<const uint64_t*>(k);
    strcpy((char*)(data+02000), v);
    mylen = len;
    myloc = 02000;
    orgcmd = 026211511;
    eval();
}

void modd(const char * k, int loc, int len) {
    key = *reinterpret_cast<const uint64_t*>(k);
    mylen = len;
    myloc = loc;
    orgcmd = 020402621001511;
    eval();
}

void getd(const char * k, int loc, int len) {
    key = *reinterpret_cast<const uint64_t*>(k);
    mylen = len;
    myloc = loc;
    orgcmd = 0221411;
    eval();
}

void getd(uint64_t k, int loc, int len) {
    key = k;
    mylen = len;
    myloc = loc;
    orgcmd = 0221411;
    eval();
}

void deld(const char * k) {
    key = *reinterpret_cast<const uint64_t*>(k);
    orgcmd = 027231411;
    eval();
}

void root() {
    orgcmd = 031;
    eval();
}

void init(int start, int len) {
    for (int i = 0; i < len; ++i) {
      data[start+i] = (064LL << 42) + i;
    }
}

void compare(int start1, int start2, int len) {
    bool match = true;
    for (int i = 0; i < len; ++i) {
        if (data[start1+i] != data[start2+i]) {
            match = false;
            std::cerr << "Element " << std::dec << i << " does not match\n";
        }
    }
    if (match)
        std::cerr << std::dec << len << " elements match between "
                  << std::oct << start1 << " and " << start2 << '\n';
}

uint64_t first() {
    orgcmd = 01;
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
    orgcmd = 0400004;
    eval();
    return curkey.d;
}

uint64_t find(const char * k) {
    key = *reinterpret_cast<const uint64_t*>(k);
    orgcmd = 011;
    eval();
    return curkey.d;
}

void printn(const char * k, const char * v) {
    size_t len = itmlen.d;
    std::cout << k << " (" << len << "): ";
    for (size_t i = 0; i < len*8; ++i)
        if (!v[i]) break;
        else std::cout << v[i];
    std::cout << '\n';
}

void usage() {
    std::cerr << "Usage: mars [-hV]\n\t-V\tverbose\n\t-h\thelp\n";
}

int main(int argc, char ** argv) {
    int c;
    bool do_init = false, newfile = false;
    int len = 2;
    for (;;) {
        c = getopt (argc, argv, "inhVtil:");
        if (c < 0)
            break;
        switch (c) {
        default:
        case 'h':
            usage ();
            return 0;
        case 'V':
            verbose = true;
            break;
        case 'i':
            do_init = true;
            break;
        case 'n':
            newfile = true;
            break;
        case 't':
            trace_stores = true;
            break;
        case 'l':
            len = atoi(optarg);
            break;
        }
    }

    // Initializing the database catalog: 1 zone, starting from zone 0 on LUN 52 (arbitrary)
    if (do_init) {
        InitDB(01520000);
    }
    // Setting the root location
    SetDB(01520000);
    // Making a new array of 'len' zones, starting from zone 1
    if (newfile) {
      newd("  TSET\0", 0520001 + (len << 18));
    }
    // Opening it
    opend("  TSET\0\0");
    // Initializing an array of 1024 words
    init(04000, 1024);
    // Putting one half of it to the DB
    modd("     A\0\0", 04000, 512);
    // Putting all of it (will be split: max usable words in a zone = 01775)
    modd("     A\0\0", 04000, 1024);
    // Again (exact match of length)
    modd("     A\0\0", 04000, 1024);
    // With smaller length (reusing the block)
    modd("     A\0\0", 04001, 1023);
    // Getting back
    getd("     A\0\0", 02000, 1023);
    compare(02000, 04001, 1023);
    // Done with it
    deld("     A\0\0");
    // Putting 100 elements of varying sizes
    for (int i = 0; i <= 60; ++i) {
        std::cerr << "Putting " << i << '\n';
        putd(i+1, 04000, i);
        if (d00011.d) {
            std::cerr << "\twhile putting " << std::dec << i << '\n';
        }
    }
    exit(0);
    uint64_t key = last();
    while (key) {
        getd(key, 02000, 100);
        std::cout << (char*)&key << ' ' << itmlen.d << '\n';
        compare(02000, 04000+itmlen.d, itmlen.d);
        key = prev();
    }
#if 0
    uint64_t prevkey = 0;
    first();
    uint64_t key = next();
    while (key && key != prevkey) {
        std::cout << "Getting " << (char*)&key <<  " itemlen = " << itmlen.d << ", addr = " << aitem.d << '\n';
        getd(key, 02000, 100);
        printn((char*)&key, (char*)(data + 02000));
        prevkey = key;
        key = next();
    }
#endif
}
