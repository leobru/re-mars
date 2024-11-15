#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <error.h>
#include <cstdlib>

union word {
    uint64_t d;
    word*    a;
    void*    p;
    operator bool() { return d; }
    uint64_t& operator=(uint64_t x) { d=x; return d; }
    word*& operator=(word* x) { a=x; return a; }
};


#define ONEBIT(n) (1ULL << ((n)-1))   // one bit set, from 1 to 48
#define BITS(n)   (~0ULL >> (64 - n)) // bitmask of bits from 0 to n
#define MERGE_(a,b)  a##b
#define LABEL_(a) MERGE_(ret_, a)
#define ret LABEL_(__LINE__)
#define call(addr,link) do { link = &&ret; goto addr; ret:; } while (0)

struct Bdvect {
    word data[168];
    word*& outadr = data[1].a;
    uint64_t & orgcmd = data[3].d;
    uint64_t & curcmd = data[5].d;
    word & loc6 = data[6];
    uint64_t & givenp = data[7].d;
    uint64_t & key = data[010].d;
    word & loc12 = data[012];
    word*& myloc = data[013].a;
    uint64_t & mylen = data[015].d;
//    word & bdbuf = data[016];
//    word & bdtab = data[017];
    word & loc20 = data[020];
    word & array = data[021];
    word & dbdesc = data[030];
    word & rootfl = data[031];
    word & savedp = data[032];
    word & frebas = data[034];
    word*& adescr = data[035].a;
    uint64_t & curkey = data[037].d;
    uint64_t & e70msk = data[044].d;
    word & curpos = data[045];
    word & aitem = data[046];
    word & dblen = data[051];
    word & loc116 = data[0116];
    word & curbuf = data[0241];
    word & idx = data[0242];
    word & which = data[0244];
    uint64_t & dirty = data[0247].d;
};

Bdvect bdv;

word bdtab[1024], bdbuf[1024];
const uint64_t TAB = 031LL, BUF = 032LL; // arbitrary
uint64_t acc;

struct Mars {
    word data[042];
    word & abdv = data[3];
    word & arch = data[4];
    word & d00010 = data[010];
    word & d00011 = data[011];
    word & d00012 = data[012];
    void*& goto_ = data[014].p;
    word & usrloc = data[016];
    word & jmpoff = data[022];
    word & d00024 = data[024];
    word & negkey = data[027];
    word & d00030 = data[030];
    word & d00031 = data[031];
    word & d00032 = data[032];
    word & work = data[035];
    word & temp = data[036];
    uint64_t & ise70 = data[037].d;

    void e70(uint64_t is) {
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
        word m16;
        void * m6;
        word m7;
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
        if (!acc) goto err2;
        bdv.curcmd = acc;
        m5 = acc & BITS(6);
      a00153:
        m6 = &&cmd0;
        goto_ = nullptr;
        switch (m5) {
        case 0:
            goto cmd0;
#if 0
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
#endif
        case 010:
            bdv.which.d = 0;
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
        usrloc.a = &d00010;
        goto *m6;
      getzon:
      err2:
        d00011.d = acc;
        if (bdv.loc6) {
            if (bdv.loc6 != 1) goto exit1;
            m16.a = bdtab;
            if (m16.a[1].d & (1 << 31)) {
                m16.a[1].d &= ~(1 << 31);
                goto wrtab;
            }
        }
      save:
        if (!(bdv.dirty & 1)) goto wrbuf;
      wrtab:
      wrbuf:
      exit:
      exit1:
      pr662:
      a00667:
      a00703:
      date:
      flush:
        usrloc.d = acc;
        call(pr1022,m6);
        bdv.loc12 = d00024;
        goto cmd0;
      pr1022:
        call(date, m7.p);
      a01023:
        m5 = 0;
        d00030.d = 0;
        m16 = bdv.frebas;
        work = m16.a[bdv.which];
        ++bdv.mylen;
        if (bdv.mylen >= work) goto nospac;
        acc = bdv.which;
      chunk:
        call(getzon, m7.p);
        m7 = m16;
        acc = *(m7.a+1);
        acc >>= 10;
        m16 = acc;
      a01035:
        acc ^= bdv.curbuf.a[m16.d+1];
        if (acc >> 39) {
            bdv.curbuf.a[m16.d+2] = bdv.curbuf.a[m16.d+1];
            --m16.d;
            acc = m16.d;
            if (m16) goto a01035;
        }
        ++m16.d;
        acc = m16.d;
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
        acc = *(m7.a+1) - m16;
        acc += 02000;
        *(m7.a+1) = acc;
        acc &= 01777;
        work = acc;
        acc += 1 + bdv.curbuf;
        temp = acc;
        if (!m5) { --m16.d; ++temp.d; }
        if (m16) {
          x:
            temp.a[m16.d-1] = usrloc.a[m16.d-1];
            if (m16) { ++m16.d; goto x; }
        }
        *(bdv.loc116+m7.a+1) = (bdv.mylen << 10) | work | d00030;
        m16 = bdv.frebas;
        (bdv.which+m16.a)->d -= bdv.mylen+1;
        /* ** */
      nospac:
      mkctl: {
            int m5 = bdv.dblen = bdv.dbdesc >> 18;
            bdv.e70msk = bdv.dbdesc & 0777777;
            word m6;
            m6.a = bdtab;
            bdv.curbuf = m6;
            ise70 = bdv.e70msk | TAB << 30;
            bdtab[1] = 01777;
          a01447:
            --m5;
            bdbuf[m5] = 01776;
            bdtab[0] = bdv.rootfl | m5;
            e70(ise70 + m5);
            if (m5) goto a01447;
            bdv.myloc = bdv.frebas = bdbuf;
            bdv.loc20 = 02000;
            d00011.d = 0;
            call(pr376, m6.p);
            call(pr1022, m6.p);
            bdv.mylen = bdv.dblen;
            bdbuf[0] = 01731 - bdv.mylen;
            goto a00153;
        }
    }
};

