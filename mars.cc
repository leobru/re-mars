#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <error.h>
#include <cstdlib>

typedef uint64_t word;

#define ONEBIT(n) (1ULL << ((n)-1))   // one bit set, from 1 to 48
#define BITS(n)   (~0ULL >> (64 - n)) // bitmask of bits from 0 to n

struct Bdvect {
    word data[168];
    word & outadr = data[1];
    word & orgcmd = data[3];
    word & curcmd = data[5];
    word & givenp = data[7];
    word & key = data[010];
    word & myloc = data[013];
    word & mylen = data[015];
    word & bdbuf = data[016];
    word & bdtab = data[017];
    word & array = data[021];
    word & dbdesc = data[030];
    word & rootfl = data[031];
    word & savedp = data[032];
    word & adescr = data[035];
    word & curkey = data[037];
    word & e70msk = data[044];
    word & curpos = data[045];
    word & aitem = data[046];
    word & dblen = data[051];
    word & idx = data[0242];
    word & which = data[0244];
    word & dirty = data[0247];
};

Bdvect bdv;

word bdtab[1024], bdbuf[1024];
const word TAB = 031LL, BUF = 032LL; // arbitrary
word acc;

struct Mars {
    word data[042];
    word & abdv = data[3];
    word & arch = data[4];
    word & goto_ = data[014];
    word & usrloc = data[016];
    word & jmpoff = data[022];
    word & temp = data[027];
    word & work = data[035];
    word & ise70 = data[037];

    void e70(word is) {
        int page = (is >> 30) & BITS(6);
        char nuzzzz[7];
        sprintf(nuzzzz, "%06o", int(is & BITS(18)));
        if (is & ONEBIT(40)) {
            // read
            FILE * f = fopen(nuzzzz, "r");
            if (!f) {
                std::cerr << "Reading zone " << nuzzzz << " which does not exist yet\n";
                return;
            }
            fread(page == TAB ? bdtab : bdbuf, 1024, sizeof(uint64_t), f);
            fclose(f);
        } else {
            // write
            FILE * f = fopen(nuzzzz, "w");
            if (!f) {
                std::cerr << "Count not open " << nuzzzz << '(' << strerror(errno) << ")\n";
                exit(1);
            }
            fwrite(page == TAB ? bdtab : bdbuf, 1024, sizeof(uint64_t), f);
            fclose(f);
        }
    }
    void eval() {
        int m5;
        int m16;
        void * m6;
        if (bdtab[0] != bdv.rootfl || bdv.e70msk) {
            e70(TAB << 30 | ONEBIT(40) | bdv.e70msk);
        }
      enter2:
        acc = bdv.orgcmd;
        goto next;
      drtnxt:
        bdv.dirty |= 1;
      rtnext:
        if (goto_) {
            std::cerr << "GOTO " << goto_ << '\n';
            goto_ = 0;
            return;
        }
      cmd0:
        acc = bdv.curcmd >> 6;
      next:
        if (!acc) goto a00626;
        bdv.curcmd = acc;
        m5 = acc & BITS(6);
      a00153:
        m6 = &&cmd0;
        goto_ = 0;
        switch (m5) {
        case 0:
            goto cmd0;
        case 1:
            temp = 0;
            // TODO goto cmd1;
        case 2:
            find(BITS(47));
            goto cmd0;
        case 3:
            m16 = 1;
            // TODO goto cmd4;
        case 4:
            m16 = 0;
            // TODO goto cmd4;
        case 5:
            data[011] = 0;
            // TODO goto cmd5;
        case 6:
            bdv.idx = 0;
            // TODO goto cmd6;
        case 7:
            cmd7(7);
        case 010:
            bdv.which = 0;
            goto mkctl;
        }
      cmd32:
        bdv.aitem = acc;
        bdv.dirty |= (bdv.which ? 2 : 0) + 1;
        goto *m6;
      pr376:
        d00012 = bdv.loc20;
        bdv.mylen = 041;
        d00010 = 2;
        usrloc = &d00010;
        goto *m6;
      pr1022:
        m7 = &&a01023;
        goto date;
      a01023:
        m5 = 0;
        d00030 = 0;
        m16 = bdv.loc34;
        work = M16[bdv.which];
        ++bdv.mylen;
        if (bdv.mylen >= work) goto nospac;
        acc = bdv.which;
      chunk:
        m7 = &&a01033;
        goto getzon;
      a01033:
        m7 = m16;
        acc = *(m7+1);
        acc >>= 10;
        m16 = acc;
      a01035:
        acc ^= bdv.curbuf[m16+1];
        if (acc >> 39) {
            bdv.curbuf[m16+2] = bdv.curbuf[m16+1];
            --m16;
            acc = m16;
            if (m16) goto a01035;
        }
        ++m16;
        acc = m16;
        bdv.loc116 = acc;
        acc <<= 39;
        acc |= d00030;
      pr1047:
        d00030 = acc;
        acc >>= 39;
        acc <<= 10;
        acc |= bdv.which;
        d00024 = acc;
        m16 = bdv.mylen;
        acc = *(m7+1) - m16;
        acc += 02000;
        *(m7+1) = acc;
        acc &= 01777;
        work = acc;
        acc += 1 + bdv.curbuf;
        temp = acc;
        if (!m5) { --m16; ++temp; }
        if (m16) {
          x:
            temp[m16-1] = usrloc[m16-1];
            if (m16) { ++m16; goto x; }
        }
        *(bdv.loc116+m7+1) = (bdv.mylen << 10) | work | d00030;
        m16 = bdv.frebas;
        *(bdv.which+m16) -= bdv.mylen+1;

    }
    void flush(word x) {
        usrloc = x;
        pr1022();
        bdv.loc12 = d00024;
    }
    void mkctl() {
        int M5 = bdv.dblen = bdv.dbdesc >> 18;
        bdv.e70msk = bdv.dbdesc & 0777777;
        word * M6 = bdtab;
        bdv.loc241 = M6;
        work2 = bdv.e70msk | TAB << 30;
        bdtab[1] = 01777;
    a01447:
        --M5;
        bdbuf[M5] = 01776;
        bdtab = bdv.rootfl | M5;
        e70(work2 + M5);
        if (M5) goto a01447;
        bdv.myloc = bdv.loc34 = bdbuf;
        bdv.loc20 = 02000;
        d00011 = 0;
        pr376();
        pr1022();
        bdv.mylen = bdv.dblen;
        bdbuf[0] = 01731 - bdv.mylen;
        flush(bdv.myloc);
    }
};

