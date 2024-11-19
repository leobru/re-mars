#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <error.h>
#include <cstdlib>
#include <cassert>
#include <string>
#include <ctime>

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
    bool operator==(const word & x) { return d == x.d; }
    bool operator!=(const word & x) { return d != x.d; }
    word& operator++() { (*this) = d + 1; return *this; }
    word& operator--() { (*this) = d - 1; return *this; }
//    operator uint64_t() const { return d; }
};

bool trace_stores = false;

word data[1024*6];

uint64_t rk = 7LL << 41;

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
    d=x; return d;
}
word& word::operator=(word x) {
    size_t index = this-data;
    if (index < 16) x.d &= 077777;
    if (trace_stores && index > 16 && index < 1024*6)
        printf("       %05lo: store %016lo\n", index, x.d);
    d=x.d; return *this;
}

typedef struct {
    word w[168];
} bdvect_t;

bdvect_t sav;


#define BDSYS 06000
#define BDTAB 010000
#define BDBUF 012000
#define BDVECT 01001

#define bdvect (*reinterpret_cast<bdvect_t*>(data+BDVECT))

void dump() {
    for (int j = 0; j < 168; ++j) {
        if (bdvect.w[j] != sav.w[j]) {
            printf(" WORD %03o CHANGED FROM  %016lo TO  %016lo\n", j, sav.w[j].d, bdvect.w[j].d);
        }
    }
    sav = bdvect;
}

word& word::operator*() {
    unsigned a = d & 077777;
    if (a >= 014000) {
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
#define ati(x) x = acc & 077777
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

uint64_t acc;

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
    int page = (is.d >> 30) & BITS(5);
    char nuzzzz[7];
    sprintf(nuzzzz, "%06o", int(is.d & BITS(18)));
    if (is.d & ONEBIT(40)) {
        // read
        FILE * f = fopen(nuzzzz, "r");
        if (!f) {
            std::cerr << "Reading zone " << nuzzzz << " which does not exist yet\n";
            return;
        }
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
        fprintf(t, " Zone %d:\n", zone);
        for (int i = 0; i < 1024; ++i) {
            fprintf(t, "%04o.%04o:  %04o %04o %04o %04o\n",
                    zone, i, int(data[page*1024+i].d >> 36),
                    int((data[page*1024+i].d >> 24) & 07777),
                    int((data[page*1024+i].d >> 12) & 07777),
                    int((data[page*1024+i].d >> 0) & 07777));
        }
        fclose(t);
    }
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

void eval() {
    acc = m16.d;
    jump(enter);
  switch_:
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
        ati(m16);
        jump(cmd4);
#if 0
    case 5:
        d00011 = 0;
        jump(cmd5);
    case 6:
        idx = 0;
        jump(cmd6);
#endif
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
        jump(pr425);
    case 014:
        acc = curkey.d;
        jump(ckexst);
    case 015:
        acc = curkey.d;
        jump(ckdnex);
#if 0
    case 016:
        acc = mylen.d ? (mylen.d ^ BITS(41)) + 1 : 0;
        jump(cmd16);
    case 017:
        acc = mylen.d ? (mylen.d ^ BITS(41)) + 1 : 0;
        jump(cmd17);
#endif
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
#if 0
    case 024:
        acc = givenp;
        jump(chkpsw);
#endif
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
        jump(save);
#if 0
    case 036:
        acc = loc12.d;
        jump(cmd36);
#endif
    case 037:
        acc = curcmd.d >> 12;
        // FALL THROUGH
    case 040:
        jump(next);
#if 0
    case 041:
        acc = (char*)&&compar-(char*)&&frombase;
        jump(k43);
    case 042:
        acc = (char*)&&tobase-(char*)&&frombase;
        // FALL THROUGH
#endif
    case 043: k43:
        jmpoff = acc;
        jump(cmd43);
#if 0
    case 044:
        acc = desc1.d;
        jump(cmd44);
    case 045:
        acc = bdtab.d;
        jump(lock);
    case 046:
        acc = myloc.d;
        jump(cmd46);
#endif
    case 047:
        acc = desc1.d;
        jump(*outadr.p);
#if 0
    case 050:
        acc = 3;
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
#endif
    case 055:
        jump(*savm16.p);
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
    acc = curcmd.d >> 6;
  next:
    if (!acc) jump(err2);
    curcmd = acc;
    m5 = acc & BITS(6);
  a00153:
    m6.p = &&cmd0;
    goto_ = 0;
    jump(switch_);
  find:
    negkey = acc;
    if (!acc || acc & ONEBIT(48))
        jump(badnam);
    acc = 0;
  cmd1:
    idx = acc;
    negkey = negkey.d ^ BITS(47);
  cmd1a:
    m16 = &loc56;
    acc = loc20.d;
    if (!acc) {
        std::cerr << "Loc20 == 0, nothing to compare\n";
        jump(a00212);
    }
  a00164:
    acc = m16[-1].d & 01777;
    ati(m5);
    std::cerr << "Comparing " << std::dec << acc/2 << " elements\n";
    if (!acc)
        jump(a00172);
  a00166:
    acc = m5[m16-2].d;
    acc += negkey.d;
    acc = (acc + (acc >> 48)) & BITS(48);
    if (!(acc & ONEBIT(48)))
        jump(a00172);
    utm(m5, -2);
    if (m5.d)
        jump(a00166);
  a00172:
    utm(m5, -2);
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
    jump(a00530);
  a00212:
    m6.p = &&cmd1a;
  a00213:
    acc = 02000;
    m16 = &loc54;
    jump(a00203);
  a00215:
    m16 = &loc120;
    jump(a00164);
    // ...
  a00267:
    call(totlen,m5);
  a00270:
    acc = negkey.d;
    ati(m16);
    if (acc < curpos.d) jump(a00274);
    negkey = acc-curpos.d;
    m16 = curpos;
    m5 = 0;
  a00274:
    jump(*((char*)&&frombase+jmpoff.d));
  frombase:
    abort();
  done:
    abort();
  a00317:
    if (!m5.d)
        jump(a00325);
    acc = aitem.d;
    acc += negkey.d;
    aitem = acc;
    acc = curpos.d;
    acc -= negkey.d;
    curpos = acc;
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
        jump(a00332);  // impossible?
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
    call(totlen,m5);
    jump(k07);
  pr342:
    call(totlen, m5);
    acc = (*aitem).d;
    desc1 = acc;
    acc &= 077777;
    itmlen = acc;
    loc53 = 0;
    jump(*m6.p);
  chkpsw:
    acc ^= savedp.d;
    if (!acc)
        jump(cmd0);
    m16 = 17;
    jump(err);
  ckdnex:
    if (acc != key.d)
        jump(cmd0);
    m16 = 9;
    jump(skip);
  ckexst:
    if (acc == key.d)
        jump(cmd0);
    m16 = 8;
    jump(skip);
  a00355:
    acc += 2;
    ati(m5);
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
  cmd5:
    m6.p = &&alloc;
  pr376:
    d00012 = loc20;
    mylen = 041;
    d00010 = 2;
    usrloc = m16 = &d00010;
    jump(*m6.p);
  cmd6:
    itmlen = 041;
    m16 = loc54;
    acc = adescr.d;
    jump(a00203);
  step:
    goto_ = m6;
  cmd4:
    acc = array[idx.d].d;
    if (!acc)
        jump(empty);
    ati(m5);
    if (!m16.d)
        jump(a00355);
    if (!m5.d)
        jump(a00415);
  a00414:
    ----m5;
    jump(a00367);
  a00415:
    acc = loc50[-1].d >> 29;
    if (!acc)
        jump(nopred);
    call(pr202,m6);
    acc = array[idx.d-2].d;
    acc -= 1<<15;
    array[idx.d-2] = acc;
    acc = loc117.d & 01777;
    ati(m5);
    jump(a00414);
  pr425:
    m16 = dblen;
    itmlen = 0;
  a00427:
    acc = frebas[m16-1].d;
    if (!acc)
        jump(a00431);
    --acc;
  a00431:
    itmlen = itmlen.d + acc;
    cycle(a00427, m16);
    jump(*m6.p);
    // ...
  a00500:
    m16 = 11;
    jump(err);
  getzon:
    which = acc;
    acc |= rootfl.d;
    zonkey = acc;
    acc &= 01777;
    acc = acc ? bdbuf.d : bdtab.d;
    m16 = curbuf = acc;
    if (m16[0] == zonkey)
        jump(*m7.p);
    if (which.d && (dirty.d & 2)) {
        dirty = dirty.d & ~2;
        ise70 = (e70msk.d | bdbuf.d << 20) + m16[0].d;
        e70(ise70);
    }
    ise70 = (curbuf.d << 20 | which.d | ONEBIT(40)) + e70msk.d;
    e70(ise70);
    m16 = curbuf;
    if (m16[0] == zonkey)
        jump(*m7.p);
    std::cerr << "Corruption: zonkey = " << std::oct << zonkey.d
              << ", data = " << (*m16).d <<"\n";
  corupt:
    m16.d = 2;
    jump(err);
  a00530:
    m5.p = &&a01423;
  totlen:
    d00040 = acc;
    acc &= 01777;
    call(getzon,m7);
  a00533:
    acc = d00040.d & 03776000;
    if (!acc)
        jump(zerkey);
    work = acc;
    acc <<= 29;
    acc &= BITS(48);
    temp = acc;
    acc = m16[1].d;
    acc &= 03776000;
    if (!acc)
        jump(delrec);
    if (acc >= work.d)
        acc = work.d;
    acc >>= 10;
    loc116 = acc;
  a00542:
    acc = loc116[m16+1].d;
    loc220 = acc;
    acc ^= temp.d;
    acc &= 0777LL << 39;
    if (acc) {
        if (--loc116.d)
            jump(a00542);
        jump(delrec);
    }
    acc = loc220.d & 01777;
    ++acc;
    acc += curbuf.d;
    aitem = acc;
    acc = loc220.d & 03776000;
    acc >>= 10;
    curpos = acc;
    jump(*m5.p);
  skip:
    acc = curcmd.d;
    acc >>= 6;
    if (!acc)
        jump(err);
    if (acc & 077)
        jump(err);
  skip5:
    acc = curcmd.d;
    acc >>= 30;
    jump(next);
  zerkey: m16 = 1; jump(err);
  delrec: m16 = 3; jump(err);
  badnam: m16 = 4; jump(err);
  corr1p: m16 = 5; jump(err);
  overfl: m16 = 6; jump(err);
  clash: m16 = 13; jump(err);
  empty: m16 = 14; jump(err);
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
    dblen = dbdesc.d >> 18;
    ise70 = bdtab.d << 20 | e70msk.d | ONEBIT(40);
    e70(ise70);
    m16 = curbuf = bdtab;
    acc = (*m16).d;
    if (acc != rootfl.d)
        jump(corr1p);
    idx = 0;
    loc246 = 0;
    which = 0;
    acc = m16[3].d & 01777;
    acc += bdtab.d + 2;
    frebas = acc;
    jump(a00213);
  err:
    acc = erhndl.d;
    if (!acc)
        jump(err1);
    savm16 = acc;
  err1:
    m7 = m16;
    std::cerr << "ERROR " << std::dec << m16.d
              << " (" << msg[m16.d-1] << ")\n";
    d00010 = *(uint64_t*)msg[m16.d-1];
    acc = *((uint64_t*)msg[m16.d-1]+1);
  err2:
    d00011 = acc;
    if (sync != 0) {
        if (sync != 1)
            jump(exit1);
        m16 = bdtab;
        if (m16[1].d & (1 << 31)) {
            m16[1].d &= ~(1 << 31);
            jump(wrtab);
        }
    }
  save:
    if (!(dirty.d & 1))
        jump(wrbuf);
  wrtab:
    ise70 = e70msk.d | bdtab.d << 20;
    e70(ise70);
  wrbuf:
    if (dirty.d & 2) {
        ise70 = (e70msk.d | bdbuf.d << 20) + (bdbuf[0].d & 01777);
        e70(ise70);
    }
  exit:
    dirty = 0;
  exit1:
    return;
  pr662:
    *aitem = acc;
    dirty(1);
    jump(*m6.p);
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
    if (!acc)
        jump(a00703);
    call(a00340,m6);
    acc &= ~(01777777LL << 10);
    loc117 = acc;
    acc = d00025.d;
    acc &= 01777777LL << 10;
    acc |= loc117.d;
    call(pr662,m6);
  a00703:
    call(pr1232,m5);
  delkey:
    acc = loc50.d;
    ati(m16);
    acc += array[idx.d].d;
    ati(m5);
    acc ^= loc50.d;
    loc116 = acc;
    acc = m16[-1].d;
    acc &= 01777;
    temp = acc;
    acc += loc50.d;
    work = acc;
  a00712:
    acc = m5[2].d;
    *m5 = acc;
    ++m5;
    acc = m5.d;
    acc ^= work.d;
    if (acc)
        jump(a00712);
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
    m5.p = &&a00730;
    acc = m5.d;
    goto_ = acc;
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
    call(totlen,m5);
  a00737:
    acc = m16[1].d;
    acc = (acc >> 10) & 077777;
    temp = acc;
    acc = loc116.d;
    ati(m5);
  a00742:
    acc ^= temp.d;
    if (!acc)
        jump(a00747);
    m5[m16+1] = m5[m16+2].d;
    ++m5;
    acc = m5.d;
    jump(a00742);
  a00747:
    acc = loc220.d & 01777;
    d00031 = acc;
  a00751:
    utm(m5,-1);
    if (!m5.d)
        jump(a00760);
    acc = m16[m5+1].d & 01777;
    if (acc >= d00031.d)
        jump(a00751);
    acc = m16[m5+1].d + curpos.d;
    m16[m5+1] = acc;
    jump(a00751);
  a00760:
    work = m16[1].d & 01777;
    if (work == d00031)
        jump(a00773);
    if (d00031.d < work.d)
        jump(a00500);
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
        std::cerr << "mylen = " << mylen.d << " work = " << work.d << '\n';
        jump(nospac);
    }
    acc = which.d;
  chunk:
    call(getzon, m7);
    m7 = m16;
    acc = m7[1].d;
    acc >>= 10;
    ati(m16);
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
        std::cerr << "To DB: "  << std::oct << m16.d << "(8) words from "
                  << usrloc.d << " to " << temp.d << '\n';
      a01063:
        temp[m16-1] = usrloc[m16-1];
        cycle(a01063, m16);
    }
  a01067:
    loc116[m7+1] = (mylen.d << 10) | work.d | d00030.d;
    m16 = frebas;
    std::cerr << "Reducing free " << std::oct << which[m16].d << " by len " << mylen.d << " + 1\n";
    which[m16] = which[m16].d - (mylen.d+1);
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
    ati(m16);
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
    std::cerr << "Expanding " << std::dec << m6.d-limit.d << " elements\n";
  expand:
    if (m6 == limit)
        jump(expand2);
    m6[0] = m6[-2];
    m6[1] = m6[-1];
    utm(m6, -2);
    jump(expand);
  expand2:
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
    acc = idx.d << 1;
    acc += 041;
    work = acc;
    acc = loc55.d ^ 036;
    if (acc)
        jump(a01201);
    acc = work.d + 044;
    work = acc;
  a01201:
    call(pr425,m6);
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
    acc = (d00025.d >> 10) & BITS(19);
    if (!acc)
        jump(a01242);
    call(a00340,m6);
    acc &= BITS(29);
    acc |= d00032.d;
    call(pr662,m6);
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
    call(pr425,m6);
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
    call(totlen,m5);
    acc--;
    ati(m5);
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
    acc = m16[1].d >> 10;
    ati(m5);
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
    call(pr425,m6);
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
    call(totlen,m5);
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
    jump(a00530);
  chklen:
    if (acc >= itmlen.d)
        jump(enough);
    m16 = 12;
    jump(err);
  mkctl: {
        m5 = dblen = dbdesc.d >> 18;
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
        call(pr376, m6);
        call(pr1022, m6);
        mylen = dblen.d;
        bdbuf[0] = 01731 - mylen.d;
        m5 = 021;
        jump(a00153);
    }
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
    std::cerr << "Running newd('" << k << "', " << std::oct << lnuzzzz << ")\n";
    key = *reinterpret_cast<const uint64_t*>(k);
    data[02121] = lnuzzzz;
    data[02122] = (key.d << 10) & BITS(48);
//    trace_stores = true;
    key = *reinterpret_cast<const uint64_t*>(k);
    mylen = 1;
    myloc = 01654;
    orgcmd = 0;
    myloc = 02121;
    mylen = 2;
    orgcmd = 02621151131LL;
    trace_stores = false;
    sav = bdvect;
//    trace_stores = true;
    eval();
    orgcmd = 010121411;
    eval();
    trace_stores = false;
}

void opend(const char * k) {
    std::cerr << "Running opend('" << k << "')\n";
    key = *reinterpret_cast<const uint64_t*>(k);
    orgcmd = 0342512141131;
    trace_stores = true;
    eval();
    trace_stores = false;
    std::cerr << "Got " << (char*)&endmrk.d << (char*)&desc1.d << '\n';
}

void putd(const char * k, int loc, int len) {
    std::cerr << "Running putd('" << k << "', '"
              << reinterpret_cast<char*>(data+loc) << "':" << len << ")\n";
    key = *reinterpret_cast<const uint64_t*>(k);
    mylen = len;
    myloc = loc;
    orgcmd = 026211511;
    eval();
}

void putd(const char * k, const char * v) {
    size_t len = (strlen(v)+7)/8;
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
    for (int i = start; i < start+len; ++i) {
        std::string s = "wrd" + std::to_string(i);
        data[i] = *reinterpret_cast<const uint64_t*>(s.c_str());
    }
}

void compare(int start1, int start2, int len) {
    for (int i = start1; i < start1+len; ++i) {
        if (data[start1+i] != data[start2+i]) {
            std::cerr << "Element " << i << "does not match\n";
        }
    }
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


int main() {
    // Initializing the database catalog: 1 zone, starting from zone 0 on LUN 52 (arbitrary)
    InitDB(01520000);
    // Setting the root location
    SetDB(01520000);
    // Making a new array of 3 zones, starting from zone 1
    newd("test\0\0\0", 03520001);
    // Opening it
    opend("test\0\0\0");
    // Initializing an array of 1024 words
    init(04000, 1024);
    // Putting one half of it to the DB
    modd("array\0\0", 04000, 512);
    // Putting all of it (will be split: max usable words in a zone = 01775)
    modd("array\0\0", 04000, 1024);
    // Again (exact match of length)
    modd("array\0\0", 04000, 1024);
    // With smaller length (reusing the block)
    modd("array\0\0", 04001, 1023);
    // Getting back
    getd("array\0\0", 02000, 1023);
    compare(02000, 04001, 1023);
    // Done with it
    deld("array\0\0");
    // Putting 100 elements of varying sizes
    for (int i = 0; i < 100; ++i) {
        std::string k = std::to_string(i);
        putd(k.c_str(), 04000, i);
    }
    uint64_t key = last();
    while (key) {
        getd(key, 02000, 100);
        std::cout << (char*)&key << ' ' << itmlen.d << '\n';
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
