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
    uint64_t& operator=(uint64_t x) { d=x; return d; }
    inline word& operator*();
    inline uint64_t operator&();
    inline word& operator[](word x);
    word operator+(int x) const { return (d+x) & 077777; }
    word operator-(int x) const { return (d-x) & 077777; }
    bool operator==(const word & x) { return d == x.d; }
    bool operator!=(const word & x) { return d != x.d; }
//    operator uint64_t() const { return d; }
};

// Bdvect starts from 0, MARS temporaries from 500
// bdtab is 1024-2047, brbuf is 2048-3071
word data[1024*6];

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
#define dirty(x) dirty |= (which.d ? x : 0) + 1

// struct Bdvect
    word & outadr = data[1];
    uint64_t & orgcmd = data[3].d;
    uint64_t & curcmd = data[5].d;
    word & loc6 = data[6];
    uint64_t & givenp = data[7].d;
    uint64_t & key = data[010].d;
    word & loc12 = data[012];
    word & myloc = data[013];
    word & loc14 = data[014];
    uint64_t & mylen = data[015].d;
    word & bdbuf = data[016];
    word & bdtab = data[017];
    word & loc20 = data[020];
    word & array = data[021];
    word & dbdesc = data[030];
    uint64_t & rootfl = data[031].d;
    word & savedp = data[032];
    word & frebas = data[034];
    word & adescr = data[035];
    word & limit = data[036];
    uint64_t & curkey = data[037].d;
    uint64_t & endmrk = data[040].d;
    uint64_t & desc1 = data[041].d;
    uint64_t & desc2 = data[042].d;
    uint64_t & e70msk = data[044].d;
    word & curpos = data[045];
    word & aitem = data[046];
    uint64_t & itmlen = data[047].d;
    word & loc50 = data[050];
    uint64_t & dblen = data[051].d;
    word & loc53 = data[053];
    word & loc54 = data[054];
    word & loc56 = data[056];
    word & loc116 = data[0116];
    word & loc120 = data[0120];
    word & loc220 = data[0220];
    word & curbuf = data[0241];
    word & idx = data[0242];
    word & which = data[0244];
    word & loc246 = data[0246];
    uint64_t & dirty = data[0247].d;

uint64_t acc;

// struct Mars
    word & abdv = data[500+3];
    uint64_t & arch = data[500+4].d;
    word & d00010 = data[500+010];
    word & d00011 = data[500+011];
    word & d00012 = data[500+012];
    uint64_t & savm13 = data[500+013].d;
    void*& goto_ = data[500+014].p;
    uint64_t & zonkey = data[500+015].d;
    word & usrloc = data[500+016];
    uint64_t & savm7 = data[500+017].d;
    uint64_t & savm6 = data[500+020].d;
    uint64_t & savm5 = data[500+021].d;
    word & jmpoff = data[500+022];
    word & datlen = data[500+023];
    word & d00024 = data[500+024];
    word & negkey = data[500+027];
    word & d00030 = data[500+030];
    word & d00031 = data[500+031];
    word & d00032 = data[500+032];
    uint64_t & savm4 = data[500+033].d;
    uint64_t & remlen = data[500+034].d;
    word & d00040 = data[500+040];
    word & work = data[500+035];
    word & temp = data[500+036];
    uint64_t & ise70 = data[500+037].d;
    uint64_t & savrk = data[500+041].d;

    void e70(uint64_t is) {
        int page = (is >> 30) & BITS(5);
        char nuzzzz[7];
        sprintf(nuzzzz, "%06o", int(is & BITS(18)));
        if (is & ONEBIT(40)) {
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
            int zone = is & 07777;
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
    void eval() {
        word m5;
        word m16;
        word m6;
        word m7;
        goto enter;
      switch_:
        switch (m5.d) {
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
            idx = 0;
            // TODO goto cmd6;
        case 7:
            cmd7(7);
#endif
        case 010:
            which.d = 0;
            goto mkctl;
        case 011:
            acc = key;
            goto find;
        case 021:
            acc = myloc.d;
            goto flush;
        case 031:
            acc = 04000;
            goto root;
        default:
            std::cerr << std::oct << m5.d << " NYI\n";
            abort();
        }
      cmd32:
        *aitem = acc;
        dirty(2);
        goto *m6.p;
      enter:
        if (bdtab[0].d != rootfl && e70msk) {
            e70(bdtab.d << 20 | ONEBIT(40) | e70msk);
        }
      enter2:
        acc = orgcmd;
        goto next;
      drtnxt:
        dirty |= 1;
      rtnext:
        if (goto_) {
            void * to = goto_;
            goto_ = nullptr;
            goto *to;
        }
      cmd0:
        acc = curcmd >> 6;
      next:
        if (!acc) goto err2;
        curcmd = acc;
        m5 = acc & BITS(6);
      a00153:
        m6.p = &&cmd0;
        goto_ = nullptr;
        goto switch_;
      find:
        negkey = acc;
        if (!acc || acc & ONEBIT(48))
            goto badnam;
        acc = 0;
      cmd1:
        idx = acc;
        acc = negkey.d;
        acc ^= BITS(47);
        negkey = acc;
      cmd1a:
        m16 = loc56;
        acc = loc20.d;
        if (!acc) goto a00212;
      a00164:
        acc = m16[-1].d;
        acc &= 01777;
        m5 = acc;
        if (!acc) goto a00172;
      a00166:
        acc = m5[m16-2].d;
        acc += negkey.d;
        acc = (acc + acc >> 48) & BITS(48);
        if (!(acc & ONEBIT(47))) goto a00172;
        m5.d -= 2;
        if (m5.d) goto a00166;
      a00172:
        m5.d -= 2;
        acc = m5.d;
        acc |= ONEBIT(31);
        idx[&array] = acc;
        acc = m16[1].d;
        acc &= ONEBIT(48);
        if (!acc) goto a00371;
        acc = idx.d;
        acc += 2;
        idx = acc;
        acc = (*(m16+1+m5.d)).d;
        m6.p = &&a00215;
      pr202:
        m16 = &loc116;
      a00203:
        idx[&loc20] = acc;
        acc &= 01777777;
        loc116 = acc;
        acc = m16.d;
        usrloc = acc;
        acc += 2;
        loc50 = acc;
        acc = loc116.d;
        acc ^= loc246.d;
        if (!acc) goto *m6.p;
        acc = loc116.d;
        loc246 = acc;
        goto a00530;
      a00212:
        m6.p = &&cmd1a;
      a00213:
        acc = 02000;
        m16 = &loc54;
        goto a00203;
      a00215:
        m16 = &loc120;
        goto a00164;
        // ...
      pr342:
        call(totlen, m5);
        acc = (*aitem).d;
        desc1 = acc;
        acc &= 077777;
        itmlen = acc;
        loc53 = 0;
        goto *m6.p;
        // ...
      a00371:
        acc = loc50[m5].d;
        curkey = acc;
        acc = loc50[m5+1].d;
        adescr = acc;
        goto rtnext;
      pr376:
        d00012 = loc20;
        mylen = 041;
        d00010 = 2;
        usrloc = &d00010;
        goto *m6.p;
      gzne0:
        acc = bdbuf.d;
        goto gz1;
      getzon:
        which.d = acc;
        acc |= rootfl;
        zonkey = acc;
        acc &= 01777;
        if (acc) goto gzne0;        
        acc = bdtab.d;
      gz1:
        m16.d = curbuf.d = acc;
        if ((*m16).d == zonkey) goto *m7.p;
        if (which.d == 0) goto gz2;
        if (!(dirty & 2)) goto gz2;
        dirty &= ~2;
        ise70 = e70msk | bdbuf.d << 20;
        ise70 += (*m16).d;
        e70(ise70);
      gz2:
        ise70 = curbuf.d << 20 | which.d | ONEBIT(40);
        ise70 += e70msk;
        e70(ise70);
        m16 = curbuf;
        if ((*m16).d == zonkey) goto *m7.p;
        std::cerr << "Corruption: zonkey = " << std::oct << zonkey
                  << ", data = " << (*m16).d <<"\n";
      corupt:
        m16.d = 2;
        goto err;
      a00530:
        m5.p = &&a01423;
      totlen:
        d00040 = acc;
        acc &= 01777;
        call(getzon,m7);
      a00533:
        acc = d00040.d;
        acc &= 03776000;
        if (!acc) goto delrec;
        if (acc >= work.d) goto a00555;
        acc >>= 10;
        loc116 = acc;
      a00542:
        acc = loc116[m16+1].d;
        loc220 = acc;
        acc ^= temp.d;
        acc &= 0777LL << 39;
        if (!acc) goto a00550;
        --loc116.d;
        if (loc116.d) goto a00542;
        goto delrec;
      a00550:
      a00555:
      skip:
        acc = curcmd;
        acc >>= 6;
        if (!acc) goto err;
        if (acc & 077) goto err;
      skip5:
        acc = curcmd;
        acc >>= 30;
        goto next;
      zerkey: m16 = 1; goto err;
      delrec: m16 = 3; goto err;
      badnam: m16 = 4; goto err;
      corr1p: m16 = 5; goto err;
      overfl: m16 = 6; goto err;
      clash: m16 = 13; goto err;
      empty: m16 = 14; goto err;
      nopred: m16 = 15; goto skip;
      nonext: m16 = 16; goto skip;
      root:
        rootfl = acc;
        dbdesc = acc = arch;
        goto setctl;
      cmd12:
        acc += &dbdesc;
        myloc = acc;
        mylen = 3;
        acc = adescr.d;
        goto cpyout;
      setctl:
        acc &= 0777777;
        e70msk = acc;
        acc = dbdesc.d;
        acc >>= 18;
        dblen = acc;
        ise70 = bdtab.d << 20 | e70msk | ONEBIT(40);
        e70(ise70);
        m16 = curbuf = bdtab;
        acc = (*m16).d;
        if (acc != rootfl) goto corr1p;
        idx = loc246 = which = 0;
        acc = m16[3].d & 01777;
        acc += bdtab.d + 2;
        frebas = acc;
        goto a00213;
      err:
        abort();
      err1:
      err2:
        d00011.d = acc;
        if (loc6 != 0) {
            if (loc6 != 1) goto exit1;
            m16 = bdtab;
            if ((*(m16+1)).d & (1 << 31)) {
                (*(m16+1)).d &= ~(1 << 31);
                goto wrtab;
            }
        }
      save:
        if (!(dirty & 1)) goto wrbuf;
      wrtab:
        ise70 = e70msk | bdtab.d << 20;
        e70(ise70);
      wrbuf:
        if (dirty & 2) {
            ise70 = e70msk | bdbuf.d << 20;
            ise70 += bdbuf[0].d & 01777;
            e70(ise70);
        }
      exit:
        dirty = 0;
      exit1:
        return;
      pr662:
        *aitem = acc;
        dirty(1);
        goto *m6.p;
      a00667:
      a00703:
        // ...
      free:
        // ...
      a01005:
        acc >>= 20;
        acc &= 01777777;
        if (acc) goto free;
        dirty |= 1;
        goto *m6.p;
      date:
        work.d = loc14.d & (0777777LL << 15);
        {
            time_t t = time(nullptr);
            struct tm & l = *localtime(&t);
            int d = l.tm_mday, m = l.tm_mon + 1, y = l.tm_year + 1900;
            std::cerr << std::dec << "Date " << d << '.' << m << '.' << y << '\n';
            uint64_t stamp = (d/10*16+d%10) << 9 |
                (m/10*16+m%10) << 4 |
                y % 10;
            datlen.d = work.d | mylen | (stamp << 33);
        }
        goto *m7.p;
      flush:
        std::cerr << std::oct << "Flush " << mylen << " words @ " << acc <<
            '\n';
        usrloc.d = acc;
        call(pr1022,m6);
        loc12 = d00024;
        goto cmd0;
      pr1022:
        call(date, m7);
      a01023:
        m5 = 0;
        d00030.d = 0;
        m16 = frebas;
        work = *(m16+which.d);
        ++mylen;
        if (mylen >= work.d) {
            std::cerr << "mylen = " << mylen << " work = " << work.d << '\n';
            goto nospac;
        }
        acc = which.d;
      chunk:
        call(getzon, m7);
        m7 = m16;
        acc = (*(m7+1)).d;
        acc >>= 10;
        m16 = acc;
      a01035:
        acc <<= 39;
        acc ^= curbuf[m16+1].d;
        acc &= 0777LL << 39;
        if (acc) {
            curbuf[m16+2] = curbuf[m16+1];
            --m16.d;
            acc = m16.d;
            if (m16.d) goto a01035;
        }
      a01044:
        ++m16.d;
        acc = m16.d;
        loc116 = acc;
        acc <<= 39;
        acc |= d00030.d;
      pr1047:
        d00030 = acc;
        acc >>= 39;
        acc <<= 10;
        acc |= which.d;
        d00024 = acc;
        m16 = mylen;
        acc = (*(m7+1)).d - m16.d;
        acc += 02000;
        *(m7+1) = acc;
        acc &= 01777;
        work = acc;
        acc += 1 + curbuf.d;
        temp = acc;
        if (!m5.d) { --m16.d; ++temp.d; }
      a01062:
        if (m16.d) {
          a01063:
            temp[m16.d-1] = usrloc[m16.d-1];
            if (--m16.d) { goto a01063; }
        }
      a01067:
        *(loc116+m7.d+1) = (mylen << 10) | work.d | d00030.d;
        m16 = frebas;
        (*(which+m16.d)).d -= mylen+1;
        if (!m5.d) goto a01111;
        dirty(1);
        acc = d00024.d;
        acc <<= 20;
        d00030 = acc;
        mylen = remlen;
      a01105:
        if (--m5.d) { goto a01264; }
        m6.p = &&overfl;
        acc = d00030.d;
        goto a01005;
      a01111:
        temp[-1] = datlen;
        dirty(2);
        goto *m6.p;
        // ...
      a01264:
        // ...
        /* ** */
      nospac:
        std::cerr << "Nospac\n";
      cpyout:
        call(pr342,m6);
      a01415:
        m6.p = &&cmd0;
        usrloc = myloc;
        acc = mylen;
        if (acc) goto chklen;
      enough:
        ++aitem.d;
        --curpos.d;
        acc = curpos.d;        
      a01423:
        m16 = acc;
        if (!acc) goto a01431;
      coloop:
        acc = aitem[m16-1].d;
        usrloc[m16-1] = acc;
        if (--m16.d) { goto coloop; }
      a01431:
        acc = usrloc.d;
        acc += curpos.d;
        usrloc = acc;
        acc = loc220.d;
        acc &= 01777777LL << 20;
        if (!acc) goto *m6.p;
        acc >>= 20;
        goto a00530;
      chklen:
        if (acc >= itmlen) goto enough;
        m16 = 12;
        goto err;
      mkctl: {
            m5 = dblen = dbdesc.d >> 18;
            e70msk = dbdesc.d & 0777777;
            m6 = bdtab;
            curbuf = m6;
            ise70 = e70msk | bdtab.d << 20;
            bdtab[1] = 01777;
          a01447:
            --m5.d;
            bdbuf[m5] = 01776;
            bdtab[0] = rootfl | m5.d;
            e70(ise70 + m5.d);
            if (m5.d) goto a01447;
            myloc = frebas = bdbuf;
            loc20 = 02000;
            d00011.d = 0;
            std::cerr << "Calling pr376\n";
            call(pr376, m6);
            std::cerr << "Done with pr376\n";
            call(pr1022, m6);
            std::cerr << "Done with pr1022\n";
            mylen = dblen;
            bdbuf[0] = 01731 - mylen;
            m5 = 021;
            goto a00153;
        }
}

void pasbdi() {
    // Faking MARS words
    savrk = 3LL << 42;
    // Arbitrariness indicator
    savm4 = savm5 = savm6 = savm7 = savm13 = 0765432101234567LL;
    // Page locations of P/BDTAB and P/BDBUF in a mid-sized test program
    for (int i = 0; i < 500; ++i)
        data[i] = 0;
    bdtab = 010000;
    bdbuf = 012000;
    abdv = 0;
}

void passetar(int lnuzzzz) {    
    arch = lnuzzzz;   
}

void pasacd(int lnuzzzz) {
    pasbdi();
    dbdesc = lnuzzzz;
    rootfl = 04000;
    orgcmd = 010;
    eval();
}

void newd(const char * k, int lnuzzzz) {
    pasbdi();
    orgcmd = 021101214112621151131;
    key = *reinterpret_cast<const uint64_t*>(k);
    data[0] = lnuzzzz;
    myloc = 0;
    mylen = 1;
    eval();
}

int main() {    
    pasacd(01520000);
    passetar(01520000);
    newd("test", 01520001);
}
