#include <gtest/gtest.h>
#include <algorithm>

#include "mars.h"

static std::string tobesm(std::string s) {
    // Convert to the BESM-6 compatible format for ease of comparison
    s += "     ";
    s.resize(6);
    std::reverse(s.begin(), s.end());
    s.resize(8);
    return s;
}

static bool compare(Mars & mars, int start1, int start2, int len) {
    bool match = true;
    auto &data = mars.data;
    for (int i = 0; i < len; ++i) {
        if (data[start1+i] != data[start2+i]) {
            match = false;
            std::cout << "Element " << std::dec << i << " does not match ("
                      << data[start1+i].d << " vs " << data[start2+i].d << ")\n";
        }
    }
    if (match && mars.verbose)
        std::cout << std::dec << len << " elements match between "
                  << std::oct << start1 << " and " << start2 << '\n';
    return match;
}

TEST(mars, initdb_stores)
{
    Mars mars(false);
    mars.zero_date = true;
    mars.memoize_stores = true;
    //mars.verbose = true;

    mars.InitDB(052, 0, 3);

    EXPECT_EQ(mars.get_store(01417), 0'0000'0000'0000'4000u);
    EXPECT_EQ(mars.get_store(01416), 0'0000'0000'0000'6000u);
    EXPECT_EQ(mars.get_store(02003), 0'0000'0000'0000'1400u);
    EXPECT_EQ(mars.get_store(01430), 0'0000'0000'0352'0000u);
    EXPECT_EQ(mars.get_store(01431), 0'0000'0000'0000'4000u);
    EXPECT_EQ(mars.get_store(01403), 0'0000'0000'0000'0010u);
    EXPECT_EQ(mars.get_store(02007), 0'0000'0000'0000'4700u);
    EXPECT_EQ(mars.get_store(02013), 0'0000'0000'0000'1654u);
    EXPECT_EQ(mars.get_store(02041), 0'0340'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(02017), 0'0000'0000'0000'1603u);
    EXPECT_EQ(mars.get_store(02021), 0'0000'0000'0007'7770u);
    EXPECT_EQ(mars.get_store(02020), 0'0000'0000'0002'2731u);
    EXPECT_EQ(mars.get_store(01405), 0'0000'0000'0000'0010u);
    EXPECT_EQ(mars.get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 10
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(01451), 0'0000'0000'0000'0003u);
    EXPECT_EQ(mars.get_store(01444), 0'0000'0000'0052'0000u);
    EXPECT_EQ(mars.get_store(01641), 0'0000'0000'0000'4000u);
//  EXPECT_EQ(mars.get_store(02036), 0'0000'0200'0052'0000u); // overwritten
//  EXPECT_EQ(mars.get_store(04001), 0'0000'0000'0000'1777u); // overwritten
    EXPECT_EQ(mars.get_store(06002), 0'0000'0000'0000'1776u);
//  EXPECT_EQ(mars.get_store(04000), 0'0000'0000'0000'4002u); // overwritten
//  EXPECT_EQ(mars.get_store(02037), 0'0000'0200'0052'0002u); // overwritten

    // Writing 520002 from address 4000
    EXPECT_EQ(mars.get_store(06001), 0'0000'0000'0000'1776u);
//  EXPECT_EQ(mars.get_store(04000), 0'0000'0000'0000'4001u); // overwritten
//  EXPECT_EQ(mars.get_store(02037), 0'0000'0200'0052'0001u); // overwritten

    // Writing 520001 from address 4000
//  EXPECT_EQ(mars.get_store(06000), 0'0000'0000'0000'1776u); // overwritten
    EXPECT_EQ(mars.get_store(04000), 0'0000'0000'0000'4000u); // latest
//  EXPECT_EQ(mars.get_store(02037), 0'0000'0200'0052'0000u); // overwritten

    // Writing 520000 from address 4000
    EXPECT_EQ(mars.get_store(01434), 0'0000'0000'0000'6000u);
    EXPECT_EQ(mars.get_store(01413), 0'0000'0000'0000'6000u);
    EXPECT_EQ(mars.get_store(01420), 0'0000'0000'0000'2000u);
    EXPECT_EQ(mars.get_store(02011), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(02012), 0'0000'0000'0000'2000u);
//  EXPECT_EQ(mars.get_store(01415), 0'0000'0000'0000'0041u); // overwritten
    EXPECT_EQ(mars.get_store(02010), 0'0000'0000'0000'0002u);
//  EXPECT_EQ(mars.get_store(02016), 0'0000'0000'0000'2010u); // overwritten
//  EXPECT_EQ(mars.get_store(02035), 0'0000'0000'0000'0000u); // overwritten
//  EXPECT_EQ(mars.get_store(02023), 0'0000'0000'0000'0041u); // overwritten
//  EXPECT_EQ(mars.get_store(02030), 0'0000'0000'0000'0000u); // overwritten
//  EXPECT_EQ(mars.get_store(02035), 0'0000'0000'0000'1776u); // overwritten
//  EXPECT_EQ(mars.get_store(01415), 0'0000'0000'0000'0042u); // overwritten
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(02015), 0'0000'0000'0000'4000u);
    EXPECT_EQ(mars.get_store(01641), 0'0000'0000'0000'4000u);
//  EXPECT_EQ(mars.get_store(01516), 0'0000'0000'0000'0001u); // overwritten
//  EXPECT_EQ(mars.get_store(02030), 0'0010'0000'0000'0000u); // overwritten
//  EXPECT_EQ(mars.get_store(02024), 0'0000'0000'0000'2000u); // overwritten
//  EXPECT_EQ(mars.get_store(04001), 0'0000'0000'0000'3735u); // overwritten
//  EXPECT_EQ(mars.get_store(02035), 0'0000'0000'0000'1735u); // overwritten
//  EXPECT_EQ(mars.get_store(02036), 0'0000'0000'0000'5736u); // overwritten
//  EXPECT_EQ(mars.get_store(02036), 0'0000'0000'0000'5737u); // overwritten

    // To DB: 41(8) words from 20 to 5737
    EXPECT_EQ(mars.get_store(05777), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05776), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05775), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05774), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05773), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05772), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05771), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05770), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05767), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05766), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05765), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05764), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05763), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05762), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05761), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05760), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05757), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05756), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05755), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05754), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05753), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05752), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05751), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05750), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05747), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05746), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05745), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05744), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05743), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05742), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05741), 0'0000'0000'0000'2000u);
    EXPECT_EQ(mars.get_store(05740), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(05737), 0'0000'0000'0000'0002u);
    EXPECT_EQ(mars.get_store(04002), 0'0010'0000'0010'5735u);

    // Reducing free 1776 by len 42 + 1
//  EXPECT_EQ(mars.get_store(06000), 0'0000'0000'0000'1733u); // overwritten
    // Got 1733

    EXPECT_EQ(mars.get_store(05736), 0'0000'0000'0000'0041u);
//  EXPECT_EQ(mars.get_store(01647), 0'0000'0000'0000'0001u); // overwritten
//  EXPECT_EQ(mars.get_store(01415), 0'0000'0000'0000'0003u); // overwritten
//  EXPECT_EQ(mars.get_store(06000), 0'0000'0000'0000'1726u); // overwritten
    EXPECT_EQ(mars.get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 21
    EXPECT_EQ(mars.get_store(02016), 0'0000'0000'0000'6000u); // latest
//  EXPECT_EQ(mars.get_store(02035), 0'0000'0000'0000'0000u); // overwritten
    EXPECT_EQ(mars.get_store(02023), 0'0000'0000'0000'0003u); // latest
//  EXPECT_EQ(mars.get_store(02030), 0'0000'0000'0000'0000u); // overwritten
//  EXPECT_EQ(mars.get_store(02035), 0'0000'0000'0000'1726u); // overwritten
    EXPECT_EQ(mars.get_store(01415), 0'0000'0000'0000'0004u); // latest
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(02015), 0'0000'0000'0000'4000u);
    EXPECT_EQ(mars.get_store(01641), 0'0000'0000'0000'4000u);
    EXPECT_EQ(mars.get_store(01516), 0'0000'0000'0000'0002u); // latest
    EXPECT_EQ(mars.get_store(02030), 0'0020'0000'0000'0000u); // latest
    EXPECT_EQ(mars.get_store(02024), 0'0000'0000'0000'4000u); // latest
    EXPECT_EQ(mars.get_store(04001), 0'0000'0000'0000'5731u); // latest
    EXPECT_EQ(mars.get_store(02035), 0'0000'0000'0000'1731u); // latest
//  EXPECT_EQ(mars.get_store(02036), 0'0000'0000'0000'5732u); // overwritten
    EXPECT_EQ(mars.get_store(02036), 0'0000'0000'0000'5733u); // latest

    // To DB: 3(8) words from 6000 to 5733
    EXPECT_EQ(mars.get_store(05735), 0'0000'0000'0000'1776u);
    EXPECT_EQ(mars.get_store(05734), 0'0000'0000'0000'1776u);
    EXPECT_EQ(mars.get_store(05733), 0'0000'0000'0000'1726u);
    EXPECT_EQ(mars.get_store(04003), 0'0020'0000'0001'1731u);

    // Reducing free 1726 by len 4 + 1
    EXPECT_EQ(mars.get_store(06000), 0'0000'0000'0000'1721u); // latest
    // Got 1721

    EXPECT_EQ(mars.get_store(05732), 0'0000'0000'0000'0003u);
//  EXPECT_EQ(mars.get_store(01647), 0'0000'0000'0000'0001u); // overwritten
    EXPECT_EQ(mars.get_store(01412), 0'0000'0000'0000'4000u);
    EXPECT_EQ(mars.get_store(02011), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(02037), 0'0000'0200'0052'0000u); // latest

    // Writing 520000 from address 4000
    EXPECT_EQ(mars.get_store(01647), 0'0000'0000'0000'0000u); // latest
}

TEST(mars, setdb_stores)
{
    Mars mars(false);
    const int catalog_len = 1;

    mars.zero_date = true;
    //mars.verbose = true;

    // Initializing the database catalog: 1 zone, starting from zone 0 on LUN 52 (arbitrary)
    mars.InitDB(052, 0, catalog_len);

    // Setting the root location
    mars.memoize_stores = true;
    std::cout << "SetDB()\n";
    mars.SetDB(052, 0, catalog_len);

    EXPECT_EQ(mars.get_store(01417), 0'0000'0000'0000'4000u);
    EXPECT_EQ(mars.get_store(01416), 0'0000'0000'0000'6000u);
    EXPECT_EQ(mars.get_store(02003), 0'0000'0000'0000'1400u);
    EXPECT_EQ(mars.get_store(01403), 0'0000'0000'0000'0031u);
    EXPECT_EQ(mars.get_store(02007), 0'0000'0000'0000'4700u);
    EXPECT_EQ(mars.get_store(02013), 0'0000'0000'0000'1654u);
    EXPECT_EQ(mars.get_store(02041), 0'0340'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(02017), 0'0000'0000'0000'1603u);
    EXPECT_EQ(mars.get_store(02021), 0'0000'0000'0007'7770u);
    EXPECT_EQ(mars.get_store(02020), 0'0000'0000'0002'2731u);
    EXPECT_EQ(mars.get_store(01405), 0'0000'0000'0000'0031u);
    EXPECT_EQ(mars.get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 31
    EXPECT_EQ(mars.get_store(01431), 0'0000'0000'0000'4000u);
    EXPECT_EQ(mars.get_store(01430), 0'0000'0000'0152'0000u);
    EXPECT_EQ(mars.get_store(01444), 0'0000'0000'0052'0000u);
    EXPECT_EQ(mars.get_store(01451), 0'0000'0000'0000'0001u);
    EXPECT_EQ(mars.get_store(02037), 0'0010'0200'0052'0000u);

    // Reading 520000 to address 4000
    EXPECT_EQ(mars.get_store(01641), 0'0000'0000'0000'4000u);
    EXPECT_EQ(mars.get_store(01642), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(mars.get_store(01646), 0'0000'0000'0000'0000u); // overwritten
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(01434), 0'0000'0000'0000'5735u);
    EXPECT_EQ(mars.get_store(01420), 0'0000'0000'0000'2000u);
//  EXPECT_EQ(mars.get_store(01516), 0'0000'0000'0000'2000u); // overwritten
//  EXPECT_EQ(mars.get_store(02016), 0'0000'0000'0000'1454u); // overwritten
    EXPECT_EQ(mars.get_store(01450), 0'0000'0000'0000'1456u);
    EXPECT_EQ(mars.get_store(01646), 0'0000'0000'0000'2000u); // latest
    EXPECT_EQ(mars.get_store(02040), 0'0000'0000'0000'2000u);
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(02015), 0'0000'0000'0000'4000u);
    EXPECT_EQ(mars.get_store(01641), 0'0000'0000'0000'4000u);
    EXPECT_EQ(mars.get_store(02035), 0'0000'0000'0000'2000u);
    EXPECT_EQ(mars.get_store(02036), 0'0010'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(01516), 0'0000'0000'0000'0001u); // latest
    EXPECT_EQ(mars.get_store(01620), 0'0010'0000'0010'5735u);
    EXPECT_EQ(mars.get_store(01446), 0'0000'0000'0000'5736u);
    EXPECT_EQ(mars.get_store(01445), 0'0000'0000'0000'0042u);

    // From DB: 42(8) words from 5736 to 1454
    EXPECT_EQ(mars.get_store(01515), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01514), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01513), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01512), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01511), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01510), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01507), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01506), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01505), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01504), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01503), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01502), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01501), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01500), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01477), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01476), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01475), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01474), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01473), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01472), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01471), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01470), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01467), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01466), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01465), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01464), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01463), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01462), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01461), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01460), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01457), 0'0000'0000'0000'2000u);
    EXPECT_EQ(mars.get_store(01456), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(01455), 0'0000'0000'0000'0002u);
    EXPECT_EQ(mars.get_store(01454), 0'0000'0000'0000'0041u);
    EXPECT_EQ(mars.get_store(02016), 0'0000'0000'0000'1516u); // latest
    EXPECT_EQ(mars.get_store(02011), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(01647), 0'0000'0000'0000'0000u);
}

TEST(mars, newd_stores)
{
    Mars mars(false);
    const int catalog_len = 1;
    const int file_len = 2;
    const std::string fname = tobesm("TEST");

    mars.zero_date = true;
    //mars.verbose = true;

    // Initializing the database catalog: 1 zone, starting from zone 0 on LUN 52 (arbitrary)
    mars.InitDB(052, 0, catalog_len);

    // Setting the root location
    mars.SetDB(052, 0, catalog_len);

    // Making a new array of 'len' zones, starting after the root catalog
    //mars.trace_stores = true;
    mars.memoize_stores = true;
    std::cout << "newd()\n";
    mars.newd(fname.c_str(), 052, catalog_len, file_len);

    // Running newd('  TSET', 2520001)
    EXPECT_EQ(mars.get_store(01410), 0'2504'2523'2502'0040u);
    EXPECT_EQ(mars.get_store(077776),0'0000'0000'0252'0001u);
    EXPECT_EQ(mars.get_store(077777),0'0524'6520'4010'0000u);
    EXPECT_EQ(mars.get_store(01410), 0'2504'2523'2502'0040u);
//  EXPECT_EQ(mars.get_store(01415), 0'0000'0000'0000'0001u); // overwritten
//  EXPECT_EQ(mars.get_store(01413), 0'1234'5670'0765'4321u); // overwritten
//  EXPECT_EQ(mars.get_store(01403), 0'0000'0000'0000'0000u); // overwritten
//  EXPECT_EQ(mars.get_store(01413), 0'0000'0000'0007'7776u); // overwritten
//  EXPECT_EQ(mars.get_store(01415), 0'0000'0000'0000'0002u); // overwritten
//  EXPECT_EQ(mars.get_store(01403), 0'0000'0026'2115'1131u); // overwritten
//  EXPECT_EQ(mars.get_store(02007), 0'0000'0000'0000'0000u); // overwritten
    EXPECT_EQ(mars.get_store(02013), 0'0000'0000'0000'1400u);
    EXPECT_EQ(mars.get_store(02041), 0'0340'0000'0000'0000u);
//  EXPECT_EQ(mars.get_store(02017), 0'0000'0000'0000'1603u); // overwritten
//  EXPECT_EQ(mars.get_store(02021), 0'0000'0000'0005'4321u); // overwritten
//  EXPECT_EQ(mars.get_store(02020), 0'0000'0417'6274'3662u); // random address, don't check
//  EXPECT_EQ(mars.get_store(01405), 0'0000'0026'2115'1131u); // overwritten
    EXPECT_EQ(mars.get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 31
//  EXPECT_EQ(mars.get_store(01431), 0'0000'0000'0000'4000u); // overwritten
//  EXPECT_EQ(mars.get_store(01430), 0'0000'0000'0152'0000u); // overwritten
//  EXPECT_EQ(mars.get_store(01444), 0'0000'0000'0052'0000u); // overwritten
//  EXPECT_EQ(mars.get_store(01451), 0'0000'0000'0000'0001u); // overwritten
//  EXPECT_EQ(mars.get_store(02037), 0'0010'0200'0052'0000u); // overwritten

    // Reading 520000 to address 4000
    EXPECT_EQ(mars.get_store(01641), 0'0000'0000'0000'4000u);
    EXPECT_EQ(mars.get_store(01642), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(mars.get_store(01646), 0'0000'0000'0000'0000u); // overwritten
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(mars.get_store(01434), 0'0000'0000'0000'5735u); // overwritten
    EXPECT_EQ(mars.get_store(01420), 0'0000'0000'0000'2000u);
//  EXPECT_EQ(mars.get_store(01516), 0'0000'0000'0000'2000u); // overwritten
//  EXPECT_EQ(mars.get_store(02016), 0'0000'0000'0000'1454u); // overwritten
    EXPECT_EQ(mars.get_store(01450), 0'0000'0000'0000'1456u);
    EXPECT_EQ(mars.get_store(01646), 0'0000'0000'0000'2000u); // latest
//  EXPECT_EQ(mars.get_store(02040), 0'0000'0000'0000'2000u); // overwritten
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(mars.get_store(02015), 0'0000'0000'0000'4000u); // overwritten
    EXPECT_EQ(mars.get_store(01641), 0'0000'0000'0000'4000u);
//  EXPECT_EQ(mars.get_store(02035), 0'0000'0000'0000'2000u); // overwritten
//  EXPECT_EQ(mars.get_store(02036), 0'0010'0000'0000'0000u); // overwritten
//  EXPECT_EQ(mars.get_store(01516), 0'0000'0000'0000'0001u); // overwritten
//  EXPECT_EQ(mars.get_store(01620), 0'0010'0000'0010'5735u); // overwritten
//  EXPECT_EQ(mars.get_store(01446), 0'0000'0000'0000'5736u); // overwritten
//  EXPECT_EQ(mars.get_store(01445), 0'0000'0000'0000'0042u); // overwritten

    // From DB: 42(8) words from 5736 to 1454   5
    EXPECT_EQ(mars.get_store(01515), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01514), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01513), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01512), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01511), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01510), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01507), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01506), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01505), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01504), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01503), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01502), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01501), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01500), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01477), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01476), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01475), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01474), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01473), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01472), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01471), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01470), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01467), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01466), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01465), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01464), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01463), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01462), 0'1234'5670'0765'4321u);
//  EXPECT_EQ(mars.get_store(01461), 0'1234'5670'0765'4321u); // overwritten
//  EXPECT_EQ(mars.get_store(01460), 0'1234'5670'0765'4321u); // overwritten
    EXPECT_EQ(mars.get_store(01457), 0'0000'0000'0000'2000u);
    EXPECT_EQ(mars.get_store(01456), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(mars.get_store(01455), 0'0000'0000'0000'0002u); // overwritten
    EXPECT_EQ(mars.get_store(01454), 0'0000'0000'0000'0041u);
//  EXPECT_EQ(mars.get_store(02016), 0'0000'0000'0000'1516u); // overwritten
//  EXPECT_EQ(mars.get_store(01405), 0'0000'0000'2621'1511u); // overwritten
    EXPECT_EQ(mars.get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 11
//  EXPECT_EQ(mars.get_store(02027), 0'2504'2523'2502'0040u); // overwritten
    EXPECT_EQ(mars.get_store(01642), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(mars.get_store(02027), 0'1273'5254'5275'7737u); // overwritten

    // Comparing 1 elements
//  EXPECT_EQ(mars.get_store(01421), 0'0000'0100'0000'0000u); // overwritten
//  EXPECT_EQ(mars.get_store(01437), 0'0000'0000'0000'0000u); // overwritten
//  EXPECT_EQ(mars.get_store(01435), 0'0000'0000'0000'2000u); // overwritten
    EXPECT_EQ(mars.get_store(02014), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(mars.get_store(01405), 0'0000'0000'0026'2115u); // overwritten
    EXPECT_EQ(mars.get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 15
//  EXPECT_EQ(mars.get_store(01405), 0'0000'0000'0000'2621u); // overwritten
    EXPECT_EQ(mars.get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 21
//  EXPECT_EQ(mars.get_store(02016), 0'0000'0000'0007'7776u); // overwritten
//  EXPECT_EQ(mars.get_store(02035), 0'0000'0000'0000'0000u); // overwritten
//  EXPECT_EQ(mars.get_store(02023), 0'0000'0000'0000'0002u); // overwritten
//  EXPECT_EQ(mars.get_store(02030), 0'0000'0000'0000'0000u); // overwritten
//  EXPECT_EQ(mars.get_store(02035), 0'0000'0000'0000'1730u); // overwritten
//  EXPECT_EQ(mars.get_store(01415), 0'0000'0000'0000'0003u); // overwritten
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(mars.get_store(02015), 0'0000'0000'0000'4000u); // overwritten
    EXPECT_EQ(mars.get_store(01641), 0'0000'0000'0000'4000u);
//  EXPECT_EQ(mars.get_store(01516), 0'0000'0000'0000'0003u); // overwritten
//  EXPECT_EQ(mars.get_store(02030), 0'0030'0000'0000'0000u); // overwritten
//  EXPECT_EQ(mars.get_store(02024), 0'0000'0000'0000'6000u); // overwritten
//  EXPECT_EQ(mars.get_store(04001), 0'0000'0000'0000'7730u); // overwritten
//  EXPECT_EQ(mars.get_store(02035), 0'0000'0000'0000'1730u); // overwritten
//  EXPECT_EQ(mars.get_store(02036), 0'0000'0000'0000'5731u); // overwritten
//  EXPECT_EQ(mars.get_store(02036), 0'0000'0000'0000'5732u); // overwritten

    // To DB: 2(8) words from 77776 to 5732
//  EXPECT_EQ(mars.get_store(05733), 0'0524'6520'4010'0000u); // overwritten
    EXPECT_EQ(mars.get_store(05732), 0'0000'0000'0252'0001u);
    EXPECT_EQ(mars.get_store(04004), 0'0030'0000'0000'7730u);

    // Reducing free 1730 by len 3 + 1
//  EXPECT_EQ(mars.get_store(05735), 0'0000'0000'0000'1724u); // overwritten

    // Got 1724
    EXPECT_EQ(mars.get_store(05731), 0'0000'0000'0000'0002u);
//  EXPECT_EQ(mars.get_store(01647), 0'0000'0000'0000'0001u); // overwritten
//  EXPECT_EQ(mars.get_store(01412), 0'0000'0000'0000'6000u); // overwritten
//  EXPECT_EQ(mars.get_store(01405), 0'0000'0000'0000'0026u); // overwritten
    EXPECT_EQ(mars.get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 26
//  EXPECT_EQ(mars.get_store(02011), 0'2504'2523'2502'0040u); // overwritten
//  EXPECT_EQ(mars.get_store(01435), 0'0000'0000'0000'6000u); // overwritten
//  EXPECT_EQ(mars.get_store(02024), 0'0000'0000'0000'6000u); // overwritten
    EXPECT_EQ(mars.get_store(01436), 0'0000'0000'0000'1460u);

    // Expanding 0 elements
    EXPECT_EQ(mars.get_store(01460), 0'2504'2523'2502'0040u); // latest
    EXPECT_EQ(mars.get_store(01461), 0'0000'0000'0000'6000u); // latest
    EXPECT_EQ(mars.get_store(01455), 0'0000'0000'0000'0004u); // latest
//  EXPECT_EQ(mars.get_store(02016), 0'0000'0000'0000'1455u); // overwritten
//  EXPECT_EQ(mars.get_store(01415), 0'0000'0000'0000'0041u); // overwritten
    EXPECT_EQ(mars.get_store(02012), 0'0000'0000'0000'2000u);
//  EXPECT_EQ(mars.get_store(02040), 0'0000'0000'0000'2000u); // overwritten
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(mars.get_store(02015), 0'0000'0000'0000'4000u); // overwritten
    EXPECT_EQ(mars.get_store(01641), 0'0000'0000'0000'4000u);
//  EXPECT_EQ(mars.get_store(02035), 0'0000'0000'0000'2000u); // overwritten
//  EXPECT_EQ(mars.get_store(02036), 0'0010'0000'0000'0000u); // overwritten
//  EXPECT_EQ(mars.get_store(01516), 0'0000'0000'0000'0001u); // overwritten
//  EXPECT_EQ(mars.get_store(01620), 0'0010'0000'0010'5735u); // overwritten
//  EXPECT_EQ(mars.get_store(01446), 0'0000'0000'0000'5736u); // overwritten
//  EXPECT_EQ(mars.get_store(01445), 0'0000'0000'0000'0042u); // overwritten
    EXPECT_EQ(mars.get_store(05777), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05776), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05775), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05774), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05773), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05772), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05771), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05770), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05767), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05766), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05765), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05764), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05763), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05762), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05761), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05760), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05757), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05756), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05755), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05754), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05753), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05752), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05751), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05750), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05747), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05746), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05745), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05744), 0'1234'5670'0765'4321u);
//  EXPECT_EQ(mars.get_store(05743), 0'0000'0000'0000'6000u); // overwritten
//  EXPECT_EQ(mars.get_store(05742), 0'2504'2523'2502'0040u); // overwritten
    EXPECT_EQ(mars.get_store(05741), 0'0000'0000'0000'2000u);
    EXPECT_EQ(mars.get_store(05740), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(mars.get_store(05737), 0'0000'0000'0000'0004u); // overwritten
//  EXPECT_EQ(mars.get_store(02035), 0'0000'0000'0000'0000u); // overwritten
//  EXPECT_EQ(mars.get_store(02023), 0'0000'0000'0000'0041u); // overwritten
    EXPECT_EQ(mars.get_store(05736), 0'0000'0000'0000'0041u);
//  EXPECT_EQ(mars.get_store(01647), 0'0000'0000'0000'0001u); // overwritten
    EXPECT_EQ(mars.get_store(02014), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(mars.get_store(02011), 0'0000'0000'0000'0000u); // overwritten
//  EXPECT_EQ(mars.get_store(02037), 0'0000'0200'0052'0000u); // overwritten

    // Writing 520000 from address 4000
//  EXPECT_EQ(mars.get_store(01647), 0'0000'0000'0000'0000u); // overwritten
    EXPECT_EQ(mars.get_store(01403), 0'0000'0000'1012'1411u); // latest
    EXPECT_EQ(mars.get_store(02007), 0'0000'0000'0000'4000u); // latest
    EXPECT_EQ(mars.get_store(02013), 0'0000'0000'0000'1400u);
    EXPECT_EQ(mars.get_store(02041), 0'0340'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(02017), 0'0000'0000'0000'4000u); // latest
    EXPECT_EQ(mars.get_store(02021), 0'0000'0000'0000'4000u); // latest
//  EXPECT_EQ(mars.get_store(02020), 0'0000'0417'6274'3474u); // random address, don't check
//  EXPECT_EQ(mars.get_store(01405), 0'0000'0000'1012'1411u); // overwritten
    EXPECT_EQ(mars.get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 11
//  EXPECT_EQ(mars.get_store(02027), 0'2504'2523'2502'0040u); // overwritten
    EXPECT_EQ(mars.get_store(01642), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(02027), 0'1273'5254'5275'7737u); // latest

    // Comparing 2 elements
    EXPECT_EQ(mars.get_store(01421), 0'0000'0100'0000'0002u); // latest
    EXPECT_EQ(mars.get_store(01437), 0'2504'2523'2502'0040u); // latest
    EXPECT_EQ(mars.get_store(01435), 0'0000'0000'0000'6000u); // latest
    EXPECT_EQ(mars.get_store(02014), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(mars.get_store(01405), 0'0000'0000'0010'1214u); // overwritten
    EXPECT_EQ(mars.get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 14
//  EXPECT_EQ(mars.get_store(01405), 0'0000'0000'0000'1012u); // overwritten
    EXPECT_EQ(mars.get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 12
//  EXPECT_EQ(mars.get_store(01413), 0'0000'0000'0000'1430u); // overwritten
//  EXPECT_EQ(mars.get_store(01415), 0'0000'0000'0000'0003u); // overwritten
    EXPECT_EQ(mars.get_store(02040), 0'0000'0000'0000'6000u); // latest
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(mars.get_store(02015), 0'0000'0000'0000'4000u); // overwritten
    EXPECT_EQ(mars.get_store(01641), 0'0000'0000'0000'4000u);
//  EXPECT_EQ(mars.get_store(02035), 0'0000'0000'0000'6000u); // overwritten
//  EXPECT_EQ(mars.get_store(02036), 0'0030'0000'0000'0000u); // overwritten
//  EXPECT_EQ(mars.get_store(01516), 0'0000'0000'0000'0003u); // overwritten
    EXPECT_EQ(mars.get_store(01620), 0'0030'0000'0000'7730u); // latest
//  EXPECT_EQ(mars.get_store(01446), 0'0000'0000'0000'5731u); // overwritten
//  EXPECT_EQ(mars.get_store(01445), 0'0000'0000'0000'0003u); // overwritten
    EXPECT_EQ(mars.get_store(01441), 0'0000'0000'0000'0002u);
    EXPECT_EQ(mars.get_store(01447), 0'0000'0000'0000'0002u);
    EXPECT_EQ(mars.get_store(01453), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(mars.get_store(02016), 0'0000'0000'0000'1430u); // overwritten
    EXPECT_EQ(mars.get_store(01446), 0'0000'0000'0000'5732u); // latest
    EXPECT_EQ(mars.get_store(01445), 0'0000'0000'0000'0002u); // latest

    // From DB: 2(8) words from 5732 to 1430
    EXPECT_EQ(mars.get_store(01431), 0'0524'6520'4010'0000u); // latest
    EXPECT_EQ(mars.get_store(01430), 0'0000'0000'0252'0001u); // latest
//  EXPECT_EQ(mars.get_store(02016), 0'0000'0000'0000'1432u); // overwritten
    EXPECT_EQ(mars.get_store(01405), 0'0000'0000'0000'0010u); // latest
    EXPECT_EQ(mars.get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 10
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(01451), 0'0000'0000'0000'0002u); // latest
    EXPECT_EQ(mars.get_store(01444), 0'0000'0000'0052'0001u); // latest
    EXPECT_EQ(mars.get_store(01641), 0'0000'0000'0000'4000u);
//  EXPECT_EQ(mars.get_store(02036), 0'0000'0200'0052'0001u); // overwritten
//  EXPECT_EQ(mars.get_store(04001), 0'0000'0000'0000'1777u); // overwritten
    EXPECT_EQ(mars.get_store(06001), 0'0000'0000'0000'1776u);
//  EXPECT_EQ(mars.get_store(04000), 0'0524'6520'4010'0001u); // overwritten
//  EXPECT_EQ(mars.get_store(02037), 0'0000'0200'0052'0002u); // overwritten

    // Writing 520002 from address 4000
//  EXPECT_EQ(mars.get_store(06000), 0'0000'0000'0000'1776u); // overwritten
    EXPECT_EQ(mars.get_store(04000), 0'0524'6520'4010'0000u); // latest
//  EXPECT_EQ(mars.get_store(02037), 0'0000'0200'0052'0001u); // overwritten

    // Writing 520001 from address 4000
    EXPECT_EQ(mars.get_store(01434), 0'0000'0000'0000'6000u); // latest
    EXPECT_EQ(mars.get_store(01413), 0'0000'0000'0000'6000u); // latest
    EXPECT_EQ(mars.get_store(01420), 0'0000'0000'0000'2000u);
//  EXPECT_EQ(mars.get_store(02011), 0'0000'0000'0000'0000u); // overwritten
    EXPECT_EQ(mars.get_store(02012), 0'0000'0000'0000'2000u);
//  EXPECT_EQ(mars.get_store(01415), 0'0000'0000'0000'0041u); // overwritten
    EXPECT_EQ(mars.get_store(02010), 0'0000'0000'0000'0002u);
//  EXPECT_EQ(mars.get_store(02016), 0'0000'0000'0000'2010u); // overwritten
//  EXPECT_EQ(mars.get_store(02035), 0'0000'0000'0000'0000u); // overwritten
//  EXPECT_EQ(mars.get_store(02023), 0'0000'0000'0000'0041u); // overwritten
//  EXPECT_EQ(mars.get_store(02030), 0'0000'0000'0000'0000u); // overwritten
//  EXPECT_EQ(mars.get_store(02035), 0'0000'0000'0000'1776u); // overwritten
//  EXPECT_EQ(mars.get_store(01415), 0'0000'0000'0000'0042u); // overwritten
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(mars.get_store(02015), 0'0524'6520'4010'0000u); // overwritten
    EXPECT_EQ(mars.get_store(01641), 0'0000'0000'0000'4000u);
//  EXPECT_EQ(mars.get_store(01516), 0'0000'0000'0000'0001u); // overwritten
//  EXPECT_EQ(mars.get_store(02030), 0'0010'0000'0000'0000u); // overwritten
//  EXPECT_EQ(mars.get_store(02024), 0'0000'0000'0000'2000u); // overwritten
//  EXPECT_EQ(mars.get_store(04001), 0'0000'0000'0000'3735u); // overwritten
//  EXPECT_EQ(mars.get_store(02035), 0'0000'0000'0000'1735u); // overwritten
//  EXPECT_EQ(mars.get_store(02036), 0'0000'0000'0000'5736u); // overwritten
//  EXPECT_EQ(mars.get_store(02036), 0'0000'0000'0000'5737u); // overwritten

    // To DB: 41(8) words from 20 to 5737
    EXPECT_EQ(mars.get_store(05777), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05776), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05775), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05774), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05773), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05772), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05771), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05770), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05767), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05766), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05765), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05764), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05763), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05762), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05761), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05760), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05757), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05756), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05755), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05754), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05753), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05752), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05751), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05750), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05747), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05746), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05745), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05744), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05743), 0'1234'5670'0765'4321u); // latest
    EXPECT_EQ(mars.get_store(05742), 0'1234'5670'0765'4321u); // latest
    EXPECT_EQ(mars.get_store(05741), 0'0000'0000'0000'2000u);
    EXPECT_EQ(mars.get_store(05740), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(05737), 0'0000'0000'0000'0002u); // latest
    EXPECT_EQ(mars.get_store(04002), 0'0010'0000'0010'5735u);

    // Reducing free 1776 by len 42 + 1
//  EXPECT_EQ(mars.get_store(06000), 0'0000'0000'0000'1733u); // overwritten
    // Got 1733

    EXPECT_EQ(mars.get_store(05736), 0'0000'0000'0000'0041u);
//  EXPECT_EQ(mars.get_store(01647), 0'0000'0000'0000'0001u); // overwritten
//  EXPECT_EQ(mars.get_store(01415), 0'0000'0000'0000'0002u); // overwritten
//  EXPECT_EQ(mars.get_store(06000), 0'0000'0000'0000'1727u); // overwritten
    EXPECT_EQ(mars.get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 21
    EXPECT_EQ(mars.get_store(02016), 0'0000'0000'0000'6000u); // latest
//  EXPECT_EQ(mars.get_store(02035), 0'0000'0000'0000'0000u); // overwritten
    EXPECT_EQ(mars.get_store(02023), 0'0000'0000'0000'0002u); // latest
//  EXPECT_EQ(mars.get_store(02030), 0'0000'0000'0000'0000u); // overwritten
//  EXPECT_EQ(mars.get_store(02035), 0'0000'0000'0000'1727u); // overwritten
    EXPECT_EQ(mars.get_store(01415), 0'0000'0000'0000'0003u); // latest
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(02015), 0'0524'6520'4010'0000u); // latest
    EXPECT_EQ(mars.get_store(01641), 0'0000'0000'0000'4000u);
    EXPECT_EQ(mars.get_store(01516), 0'0000'0000'0000'0002u); // latest
    EXPECT_EQ(mars.get_store(02030), 0'0020'0000'0000'0000u); // latest
    EXPECT_EQ(mars.get_store(02024), 0'0000'0000'0000'4000u); // latest
    EXPECT_EQ(mars.get_store(04001), 0'0000'0000'0000'5732u); // latest
    EXPECT_EQ(mars.get_store(02035), 0'0000'0000'0000'1732u); // latest
//  EXPECT_EQ(mars.get_store(02036), 0'0000'0000'0000'5733u); // overwritten
    EXPECT_EQ(mars.get_store(02036), 0'0000'0000'0000'5734u); // latest

    // To DB: 2(8) words from 6000 to 5734
    EXPECT_EQ(mars.get_store(05735), 0'0000'0000'0000'1776u); // latest
    EXPECT_EQ(mars.get_store(05734), 0'0000'0000'0000'1727u);
    EXPECT_EQ(mars.get_store(04003), 0'0020'0000'0000'7732u);

    // Reducing free 1727 by len 3 + 1
    EXPECT_EQ(mars.get_store(06000), 0'0000'0000'0000'1723u); // latest
    // Got 1723

    EXPECT_EQ(mars.get_store(05733), 0'0000'0000'0000'0002u); // latest
//  EXPECT_EQ(mars.get_store(01647), 0'0000'0000'0000'0001u); // overwritten
    EXPECT_EQ(mars.get_store(01412), 0'0000'0000'0000'4000u); // latest
    EXPECT_EQ(mars.get_store(02011), 0'0000'0000'0000'0000u); // latest
    EXPECT_EQ(mars.get_store(02037), 0'0000'0200'0052'0001u); // latest

    // Writing 520001 from address 4000
    EXPECT_EQ(mars.get_store(01647), 0'0000'0000'0000'0000u); // latest
}

TEST(mars, opend_stores)
{
    Mars mars(false);
    const int catalog_len = 1;
    const int file_len = 2;
    const std::string fname = tobesm("TEST");

    mars.zero_date = true;
    //mars.verbose = true;

    // Initializing the database catalog: 1 zone, starting from zone 0 on LUN 52 (arbitrary)
    mars.InitDB(052, 0, catalog_len);

    // Setting the root location
    mars.SetDB(052, 0, catalog_len);

    // Making a new array of 'len' zones, starting after the root catalog
    mars.newd(fname.c_str(), 052, catalog_len, file_len);

    // Opening it
    //mars.trace_stores = true;
    mars.memoize_stores = true;
    std::cout << "opend()\n";
    mars.opend(fname.c_str());

    // Running opend('  TSET')
    EXPECT_EQ(mars.get_store(01410), 0'2504'2523'2502'0040u);
    EXPECT_EQ(mars.get_store(01403), 0'0000'0025'1214'1131u);
    EXPECT_EQ(mars.get_store(02007), 0'0000'0000'0000'6000u);
    EXPECT_EQ(mars.get_store(02013), 0'0000'0000'0000'1400u);
    EXPECT_EQ(mars.get_store(02041), 0'0340'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(02017), 0'0000'0000'0000'4000u);
    EXPECT_EQ(mars.get_store(02021), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(mars.get_store(02020), 0'0000'0406'1017'7065u); // random address, don't check
//  EXPECT_EQ(mars.get_store(01405), 0'0000'0025'1214'1131u); // overwritten
    EXPECT_EQ(mars.get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 31
//  EXPECT_EQ(mars.get_store(01431), 0'0000'0000'0000'4000u); // overwritten
//  EXPECT_EQ(mars.get_store(01430), 0'0000'0000'0152'0000u); // overwritten
//  EXPECT_EQ(mars.get_store(01444), 0'0000'0000'0052'0000u); // overwritten
//  EXPECT_EQ(mars.get_store(01451), 0'0000'0000'0000'0001u); // overwritten
//  EXPECT_EQ(mars.get_store(02037), 0'0010'0200'0052'0000u); // overwritten

    // Reading 520000 to address 4000
    EXPECT_EQ(mars.get_store(01641), 0'0000'0000'0000'4000u);
    EXPECT_EQ(mars.get_store(01642), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(mars.get_store(01646), 0'0000'0000'0000'0000u); // overwritten
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(mars.get_store(01434), 0'0000'0000'0000'5735u); // overwritten
    EXPECT_EQ(mars.get_store(01420), 0'0000'0000'0000'2000u);
//  EXPECT_EQ(mars.get_store(01516), 0'0000'0000'0000'2000u); // overwritten
//  EXPECT_EQ(mars.get_store(02016), 0'0000'0000'0000'1454u); // overwritten
    EXPECT_EQ(mars.get_store(01450), 0'0000'0000'0000'1456u);
//  EXPECT_EQ(mars.get_store(01646), 0'0000'0000'0000'2000u); // overwritten
//  EXPECT_EQ(mars.get_store(02040), 0'0000'0000'0000'2000u); // overwritten
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(mars.get_store(02015), 0'0000'0000'0000'4000u); // overwritten
    EXPECT_EQ(mars.get_store(01641), 0'0000'0000'0000'4000u);
//  EXPECT_EQ(mars.get_store(02035), 0'0000'0000'0000'2000u); // overwritten
//  EXPECT_EQ(mars.get_store(02036), 0'0010'0000'0000'0000u); // overwritten
//  EXPECT_EQ(mars.get_store(01516), 0'0000'0000'0000'0001u); // overwritten
//  EXPECT_EQ(mars.get_store(01620), 0'0010'0000'0010'5735u); // overwritten
//  EXPECT_EQ(mars.get_store(01446), 0'0000'0000'0000'5736u); // overwritten
//  EXPECT_EQ(mars.get_store(01445), 0'0000'0000'0000'0042u); // overwritten

    // From DB: 42(8) words from 5736 to 1454
    EXPECT_EQ(mars.get_store(01515), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01514), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01513), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01512), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01511), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01510), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01507), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01506), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01505), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01504), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01503), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01502), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01501), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01500), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01477), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01476), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01475), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01474), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01473), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01472), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01471), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01470), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01467), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01466), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01465), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01464), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01463), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01462), 0'1234'5670'0765'4321u);
//  EXPECT_EQ(mars.get_store(01461), 0'0000'0000'0000'6000u); // overwritten
//  EXPECT_EQ(mars.get_store(01460), 0'2504'2523'2502'0040u); // overwritten
    EXPECT_EQ(mars.get_store(01457), 0'0000'0000'0000'2000u);
    EXPECT_EQ(mars.get_store(01456), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(mars.get_store(01455), 0'0000'0000'0000'0004u); // overwritten
    EXPECT_EQ(mars.get_store(01454), 0'0000'0000'0000'0041u);
//  EXPECT_EQ(mars.get_store(02016), 0'0000'0000'0000'1516u); // overwritten
//  EXPECT_EQ(mars.get_store(01405), 0'0000'0000'2512'1411u); // overwritten
    EXPECT_EQ(mars.get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 11
//  EXPECT_EQ(mars.get_store(02027), 0'2504'2523'2502'0040u); // overwritten
    EXPECT_EQ(mars.get_store(01642), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(02027), 0'1273'5254'5275'7737u); // latest

    // Comparing 2 elements
    EXPECT_EQ(mars.get_store(01421), 0'0000'0100'0000'0002u);
    EXPECT_EQ(mars.get_store(01437), 0'2504'2523'2502'0040u);
    EXPECT_EQ(mars.get_store(01435), 0'0000'0000'0000'6000u);
    EXPECT_EQ(mars.get_store(02014), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(mars.get_store(01405), 0'0000'0000'0025'1214u); // overwritten
    EXPECT_EQ(mars.get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 14
//  EXPECT_EQ(mars.get_store(01405), 0'0000'0000'0000'2512u); // overwritten
    EXPECT_EQ(mars.get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 12
    EXPECT_EQ(mars.get_store(01413), 0'0000'0000'0000'1430u);
    EXPECT_EQ(mars.get_store(01415), 0'0000'0000'0000'0003u);
//  EXPECT_EQ(mars.get_store(02040), 0'0000'0000'0000'6000u); // overwritten
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(mars.get_store(02015), 0'0000'0000'0000'4000u); // overwritten
    EXPECT_EQ(mars.get_store(01641), 0'0000'0000'0000'4000u);
//  EXPECT_EQ(mars.get_store(02035), 0'0000'0000'0000'6000u); // overwritten
//  EXPECT_EQ(mars.get_store(02036), 0'0030'0000'0000'0000u); // overwritten
//  EXPECT_EQ(mars.get_store(01516), 0'0000'0000'0000'0003u); // overwritten
//  EXPECT_EQ(mars.get_store(01620), 0'0030'0000'0000'7730u); // overwritten
//  EXPECT_EQ(mars.get_store(01446), 0'0000'0000'0000'5731u); // overwritten
//  EXPECT_EQ(mars.get_store(01445), 0'0000'0000'0000'0003u); // overwritten
    EXPECT_EQ(mars.get_store(01441), 0'0000'0000'0000'0002u);
    EXPECT_EQ(mars.get_store(01447), 0'0000'0000'0000'0002u);
    EXPECT_EQ(mars.get_store(01453), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(mars.get_store(02016), 0'0000'0000'0000'1430u); // overwritten
//  EXPECT_EQ(mars.get_store(01446), 0'0000'0000'0000'5732u); // overwritten
//  EXPECT_EQ(mars.get_store(01445), 0'0000'0000'0000'0002u); // overwritten

    // From DB: 2(8) words from 5732 to 1430
    EXPECT_EQ(mars.get_store(01431), 0'0524'6520'4010'0000u); // latest
    EXPECT_EQ(mars.get_store(01430), 0'0000'0000'0252'0001u); // latest
//  EXPECT_EQ(mars.get_store(02016), 0'0000'0000'0000'1432u); // overwritten
    EXPECT_EQ(mars.get_store(01405), 0'0000'0000'0000'0025u); // latest
    EXPECT_EQ(mars.get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 25
    EXPECT_EQ(mars.get_store(01444), 0'0000'0000'0052'0001u); // latest
    EXPECT_EQ(mars.get_store(01451), 0'0000'0000'0000'0002u); // latest
    EXPECT_EQ(mars.get_store(02037), 0'0010'0200'0052'0001u); // latest

    // Reading 520001 to address 4000
    EXPECT_EQ(mars.get_store(01641), 0'0000'0000'0000'4000u);
    EXPECT_EQ(mars.get_store(01642), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(mars.get_store(01646), 0'0000'0000'0000'0000u); // overwritten
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(01434), 0'0000'0000'0000'5734u); // latest
    EXPECT_EQ(mars.get_store(01420), 0'0000'0000'0000'2000u);
//  EXPECT_EQ(mars.get_store(01516), 0'0000'0000'0000'2000u); // overwritten
//  EXPECT_EQ(mars.get_store(02016), 0'0000'0000'0000'1454u); // overwritten
    EXPECT_EQ(mars.get_store(01450), 0'0000'0000'0000'1456u);
    EXPECT_EQ(mars.get_store(01646), 0'0000'0000'0000'2000u); // latest
    EXPECT_EQ(mars.get_store(02040), 0'0000'0000'0000'2000u); // latest
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(02015), 0'0524'6520'4010'0000u); // latest
    EXPECT_EQ(mars.get_store(01641), 0'0000'0000'0000'4000u);
    EXPECT_EQ(mars.get_store(02035), 0'0000'0000'0000'2000u); // latest
    EXPECT_EQ(mars.get_store(02036), 0'0010'0000'0000'0000u); // latest
    EXPECT_EQ(mars.get_store(01516), 0'0000'0000'0000'0001u); // latest
    EXPECT_EQ(mars.get_store(01620), 0'0010'0000'0010'5735u); // latest
    EXPECT_EQ(mars.get_store(01446), 0'0000'0000'0000'5736u); // latest
    EXPECT_EQ(mars.get_store(01445), 0'0000'0000'0000'0042u); // latest

    // From DB: 42(8) words from 5736 to 1454
    EXPECT_EQ(mars.get_store(01515), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01514), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01513), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01512), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01511), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01510), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01507), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01506), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01505), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01504), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01503), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01502), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01501), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01500), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01477), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01476), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01475), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01474), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01473), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01472), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01471), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01470), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01467), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01466), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01465), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01464), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01463), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01462), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01461), 0'1234'5670'0765'4321u); // latest
    EXPECT_EQ(mars.get_store(01460), 0'1234'5670'0765'4321u); // latest
    EXPECT_EQ(mars.get_store(01457), 0'0000'0000'0000'2000u);
    EXPECT_EQ(mars.get_store(01456), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(01455), 0'0000'0000'0000'0002u); // latest
    EXPECT_EQ(mars.get_store(01454), 0'0000'0000'0000'0041u);
    EXPECT_EQ(mars.get_store(02016), 0'0000'0000'0000'1516u); // latest
    EXPECT_EQ(mars.get_store(02011), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(01647), 0'0000'0000'0000'0000u);
}

TEST(mars, getd_str_stores)
{
    Mars mars(false);
    const int catalog_len = 1;
    const int file_len = 2;
    const std::string fname = tobesm("TEST");

    mars.zero_date = true;
    //mars.verbose = true;

    // Initializing the database catalog: 1 zone, starting from zone 0 on LUN 52 (arbitrary)
    mars.InitDB(052, 0, catalog_len);

    // Setting the root location
    mars.SetDB(052, 0, catalog_len);

    // Making a new array of 'len' zones, starting after the root catalog
    mars.newd(fname.c_str(), 052, catalog_len, file_len);

    // Opening it
    mars.opend(fname.c_str());

    // Initializing an array of 10 words
    const int PAGE1 = 010000;
    const int PAGE2 = 012000;
    for (int i = 0; i < 10; ++i) {
        mars.data[PAGE1 + i] = (064LL << 42) + i;
    }

    std::string elt = "A";
    elt.resize(8);

    // Put data to the DB
    mars.modd(elt.c_str(), PAGE1, 10);

    // Getting back
    //mars.trace_stores = true;
    mars.memoize_stores = true;
    std::cout << "getd()\n";
    mars.getd(elt.c_str(), PAGE2, 10);
    ASSERT_TRUE(compare(mars, PAGE2, PAGE1, 10));

    EXPECT_EQ(mars.get_store(01410), 0'0000'0000'0000'0101u);
    EXPECT_EQ(mars.get_store(01415), 0'0000'0000'0000'0012u);
    EXPECT_EQ(mars.get_store(01413), 0'0000'0000'0001'2000u);
    EXPECT_EQ(mars.get_store(01403), 0'0000'0000'0022'1411u);
    EXPECT_EQ(mars.get_store(02007), 0'0000'0000'0000'4000u);
    EXPECT_EQ(mars.get_store(02013), 0'0000'0000'0000'1400u);
    EXPECT_EQ(mars.get_store(02041), 0'0340'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(02017), 0'0000'0000'0000'4000u);
    EXPECT_EQ(mars.get_store(02021), 0'0000'0000'0000'0040u);
    EXPECT_EQ(mars.get_store(02020), 0'0000'0000'0007'6500u);
//  EXPECT_EQ(mars.get_store(01405), 0'0000'0000'0022'1411u); // overwritten
    EXPECT_EQ(mars.get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 11
//  EXPECT_EQ(mars.get_store(02027), 0'0000'0000'0000'0101u); // overwritten
    EXPECT_EQ(mars.get_store(01642), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(02027), 0'3777'7777'7777'7676u); // latest

    // Comparing 2 elements
    EXPECT_EQ(mars.get_store(01421), 0'0000'0100'0000'0002u);
    EXPECT_EQ(mars.get_store(01437), 0'0000'0000'0000'0101u);
    EXPECT_EQ(mars.get_store(01435), 0'0000'0000'0000'6000u);
//  EXPECT_EQ(mars.get_store(01405), 0'0000'0000'0000'2214u); // overwritten
    EXPECT_EQ(mars.get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 14
    EXPECT_EQ(mars.get_store(01405), 0'0000'0000'0000'0022u); // latest
    EXPECT_EQ(mars.get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 22
    EXPECT_EQ(mars.get_store(02040), 0'0000'0000'0000'6000u);
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(02015), 0'0524'6520'4010'0000u);
    EXPECT_EQ(mars.get_store(01641), 0'0000'0000'0000'4000u);
    EXPECT_EQ(mars.get_store(02035), 0'0000'0000'0000'6000u);
    EXPECT_EQ(mars.get_store(02036), 0'0030'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(01516), 0'0000'0000'0000'0003u);
    EXPECT_EQ(mars.get_store(01620), 0'0030'0000'0002'7717u);
//  EXPECT_EQ(mars.get_store(01446), 0'0000'0000'0000'5720u); // overwritten
//  EXPECT_EQ(mars.get_store(01445), 0'0000'0000'0000'0013u); // overwritten
    EXPECT_EQ(mars.get_store(01441), 0'0000'0000'0000'0012u);
    EXPECT_EQ(mars.get_store(01447), 0'0000'0000'0000'0012u);
    EXPECT_EQ(mars.get_store(01453), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(mars.get_store(02016), 0'0000'0000'0001'2000u); // overwritten
    EXPECT_EQ(mars.get_store(01446), 0'0000'0000'0000'5721u); // latest
    EXPECT_EQ(mars.get_store(01445), 0'0000'0000'0000'0012u); // latest

    // From DB: 12(8) words from 5721 to 12000
    EXPECT_EQ(mars.get_store(012011),0'6400'0000'0000'0011u);
    EXPECT_EQ(mars.get_store(012010),0'6400'0000'0000'0010u);
    EXPECT_EQ(mars.get_store(012007),0'6400'0000'0000'0007u);
    EXPECT_EQ(mars.get_store(012006),0'6400'0000'0000'0006u);
    EXPECT_EQ(mars.get_store(012005),0'6400'0000'0000'0005u);
    EXPECT_EQ(mars.get_store(012004),0'6400'0000'0000'0004u);
    EXPECT_EQ(mars.get_store(012003),0'6400'0000'0000'0003u);
    EXPECT_EQ(mars.get_store(012002),0'6400'0000'0000'0002u);
    EXPECT_EQ(mars.get_store(012001),0'6400'0000'0000'0001u);
    EXPECT_EQ(mars.get_store(012000),0'6400'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(02016), 0'0000'0000'0001'2012u); // latest
    EXPECT_EQ(mars.get_store(02011), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(01647), 0'0000'0000'0000'0000u);
}

TEST(mars, deld_stores)
{
    Mars mars(false);
    const int catalog_len = 1;
    const int file_len = 2;
    const std::string fname = tobesm("TEST");

    mars.zero_date = true;
    //mars.verbose = true;

    // Initializing the database catalog: 1 zone, starting from zone 0 on LUN 52 (arbitrary)
    mars.InitDB(052, 0, catalog_len);

    // Setting the root location
    mars.SetDB(052, 0, catalog_len);

    // Making a new array of 'len' zones, starting after the root catalog
    mars.newd(fname.c_str(), 052, catalog_len, file_len);

    // Opening it
    mars.opend(fname.c_str());

    // Initializing an array of 10 words
    const int PAGE1 = 010000;
    for (int i = 0; i < 10; ++i) {
        mars.data[PAGE1 + i] = (064LL << 42) + i;
    }

    std::string elt = "A";
    elt.resize(8);

    // Put data to the DB
    mars.modd(elt.c_str(), PAGE1, 10);

    // Delete it
    //mars.trace_stores = true;
    mars.memoize_stores = true;
    std::cout << "deld()\n";
    mars.deld(elt.c_str());

    EXPECT_EQ(mars.get_store(01410), 0'0000'0000'0000'0101u);
    EXPECT_EQ(mars.get_store(01403), 0'0000'0000'2723'1411u);
    EXPECT_EQ(mars.get_store(02007), 0'0000'0000'0000'4000u);
    EXPECT_EQ(mars.get_store(02013), 0'0000'0000'0000'1400u);
    EXPECT_EQ(mars.get_store(02041), 0'0340'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(02017), 0'0000'0000'0000'4000u);
    EXPECT_EQ(mars.get_store(02021), 0'0000'0000'0000'0040u);
    EXPECT_EQ(mars.get_store(02020), 0'0000'0000'0007'6500u);
//  EXPECT_EQ(mars.get_store(01405), 0'0000'0000'2723'1411u); // overwritten
    EXPECT_EQ(mars.get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 11
//  EXPECT_EQ(mars.get_store(02027), 0'0000'0000'0000'0101u); // overwritten
    EXPECT_EQ(mars.get_store(01642), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(02027), 0'3777'7777'7777'7676u); // latest

    // Comparing 2 elements
    EXPECT_EQ(mars.get_store(01421), 0'0000'0100'0000'0002u);
    EXPECT_EQ(mars.get_store(01437), 0'0000'0000'0000'0101u);
    EXPECT_EQ(mars.get_store(01435), 0'0000'0000'0000'6000u);
//  EXPECT_EQ(mars.get_store(01405), 0'0000'0000'0027'2314u); // overwritten
    EXPECT_EQ(mars.get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 14
//  EXPECT_EQ(mars.get_store(01405), 0'0000'0000'0000'2723u); // overwritten
    EXPECT_EQ(mars.get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 23
//  EXPECT_EQ(mars.get_store(02040), 0'0000'0000'0000'6000u); // overwritten
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(02015), 0'0524'6520'4010'0000u);
    EXPECT_EQ(mars.get_store(01641), 0'0000'0000'0000'4000u);
//  EXPECT_EQ(mars.get_store(02035), 0'0000'0000'0000'6000u); // overwritten
//  EXPECT_EQ(mars.get_store(02036), 0'0030'0000'0000'0000u); // overwritten
//  EXPECT_EQ(mars.get_store(01516), 0'0000'0000'0000'0003u); // overwritten
//  EXPECT_EQ(mars.get_store(01620), 0'0030'0000'0002'7717u); // overwritten
//  EXPECT_EQ(mars.get_store(01446), 0'0000'0000'0000'5720u); // overwritten
//  EXPECT_EQ(mars.get_store(01445), 0'0000'0000'0000'0013u); // overwritten
//  EXPECT_EQ(mars.get_store(02036), 0'0000'0000'0000'0003u); // overwritten
    EXPECT_EQ(mars.get_store(02031), 0'0000'0000'0000'1717u);
//  EXPECT_EQ(mars.get_store(02035), 0'0000'0000'0000'1717u); // overwritten
    EXPECT_EQ(mars.get_store(04001), 0'0000'0000'0000'5732u);
    EXPECT_EQ(mars.get_store(05734), 0'0000'0000'0000'1727u);
//  EXPECT_EQ(mars.get_store(01647), 0'0000'0000'0000'0001u); // overwritten
//  EXPECT_EQ(mars.get_store(01647), 0'0000'0000'0000'0001u); // overwritten
    EXPECT_EQ(mars.get_store(01405), 0'0000'0000'0000'0027u); // latest
    EXPECT_EQ(mars.get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 27
    EXPECT_EQ(mars.get_store(01436), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(mars.get_store(01516), 0'0000'0100'0000'0036u); // overwritten
//  EXPECT_EQ(mars.get_store(02036), 0'0000'0000'0000'0004u); // overwritten
//  EXPECT_EQ(mars.get_store(02035), 0'0000'0000'0000'1462u); // overwritten
    EXPECT_EQ(mars.get_store(01460), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01461), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01455), 0'0000'0000'0000'0002u);
    EXPECT_EQ(mars.get_store(02016), 0'0000'0000'0000'1455u);
    EXPECT_EQ(mars.get_store(01415), 0'0000'0000'0000'0041u);
    EXPECT_EQ(mars.get_store(02012), 0'0000'0000'0000'2000u);
    EXPECT_EQ(mars.get_store(02040), 0'0000'0000'0000'2000u); // latest
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(02015), 0'0524'6520'4010'0000u);
    EXPECT_EQ(mars.get_store(01641), 0'0000'0000'0000'4000u);
//  EXPECT_EQ(mars.get_store(02035), 0'0000'0000'0000'2000u); // overwritten
    EXPECT_EQ(mars.get_store(02036), 0'0010'0000'0000'0000u); // latest
    EXPECT_EQ(mars.get_store(01516), 0'0000'0000'0000'0001u); // latest
    EXPECT_EQ(mars.get_store(01620), 0'0010'0000'0010'5735u); // latest
    EXPECT_EQ(mars.get_store(01446), 0'0000'0000'0000'5736u); // latest
    EXPECT_EQ(mars.get_store(01445), 0'0000'0000'0000'0042u); // latest
    EXPECT_EQ(mars.get_store(05777), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05776), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05775), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05774), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05773), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05772), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05771), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05770), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05767), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05766), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05765), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05764), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05763), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05762), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05761), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05760), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05757), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05756), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05755), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05754), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05753), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05752), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05751), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05750), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05747), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05746), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05745), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05744), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05743), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05742), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05741), 0'0000'0000'0000'2000u);
    EXPECT_EQ(mars.get_store(05740), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(05737), 0'0000'0000'0000'0002u);
    EXPECT_EQ(mars.get_store(02035), 0'0000'0000'0000'0000u); // latest
    EXPECT_EQ(mars.get_store(02023), 0'0000'0000'0000'0041u);
    EXPECT_EQ(mars.get_store(05736), 0'0000'0000'0000'0041u);
    EXPECT_EQ(mars.get_store(01647), 0'0000'0000'0000'0000u); // latest
    EXPECT_EQ(mars.get_store(02011), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(02037), 0'0000'0200'0052'0001u);
}

TEST(mars, putd_stores)
{
    Mars mars(false);
    const int catalog_len = 1;
    const int file_len = 2;
    const std::string fname = tobesm("TEST");

    mars.zero_date = true;
    //mars.verbose = true;

    // Initializing the database catalog: 1 zone, starting from zone 0 on LUN 52 (arbitrary)
    mars.InitDB(052, 0, catalog_len);

    // Setting the root location
    mars.SetDB(052, 0, catalog_len);

    // Making a new array of 'len' zones, starting after the root catalog
    mars.newd(fname.c_str(), 052, catalog_len, file_len);

    // Opening it
    mars.opend(fname.c_str());

    // Initializing an array of 1024 words
    const int PAGE1 = 010000;
    for (int i = 0; i < 1024; ++i) {
        mars.data[PAGE1 + i] = (064LL << 42) + i;
    }

    // Put element with numerical key
    //mars.trace_stores = true;
    mars.memoize_stores = true;
    std::cout << "putd()\n";
    mars.putd(123, PAGE1, 10);

    // Running putd(0000000000000173, 10000, 10)
    EXPECT_EQ(mars.get_store(01410), 0'0000'0000'0000'0173u);
//  EXPECT_EQ(mars.get_store(01415), 0'0000'0000'0000'0012u); // overwritten
    EXPECT_EQ(mars.get_store(01413), 0'0000'0000'0001'0000u);
    EXPECT_EQ(mars.get_store(01403), 0'0000'0000'2621'1511u);
    EXPECT_EQ(mars.get_store(02007), 0'0000'0000'0000'4000u);
    EXPECT_EQ(mars.get_store(02013), 0'0000'0000'0000'1400u);
    EXPECT_EQ(mars.get_store(02041), 0'0340'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(02017), 0'0000'0000'0000'4000u);
    EXPECT_EQ(mars.get_store(02021), 0'0000'0000'0005'4321u);
    EXPECT_EQ(mars.get_store(02020), 0'0000'0000'0007'6500u);
//  EXPECT_EQ(mars.get_store(01405), 0'0000'0000'2621'1511u); // overwritten
    EXPECT_EQ(mars.get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 11
//  EXPECT_EQ(mars.get_store(02027), 0'0000'0000'0000'0173u); // overwritten
    EXPECT_EQ(mars.get_store(01642), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(02027), 0'3777'7777'7777'7604u); // latest

    // Comparing 1 elements
    EXPECT_EQ(mars.get_store(01421), 0'0000'0100'0000'0000u);
    EXPECT_EQ(mars.get_store(01437), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(mars.get_store(01435), 0'0000'0000'0000'2000u); // overwritten
//  EXPECT_EQ(mars.get_store(01405), 0'0000'0000'0026'2115u); // overwritten
    EXPECT_EQ(mars.get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 15
//  EXPECT_EQ(mars.get_store(01405), 0'0000'0000'0000'2621u); // overwritten
    EXPECT_EQ(mars.get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 21
//  EXPECT_EQ(mars.get_store(02016), 0'0000'0000'0001'0000u); // overwritten
//  EXPECT_EQ(mars.get_store(02035), 0'0000'0000'0000'0000u); // overwritten
//  EXPECT_EQ(mars.get_store(02023), 0'0000'0000'0000'0012u); // overwritten
//  EXPECT_EQ(mars.get_store(02030), 0'0000'0000'0000'0000u); // overwritten
//  EXPECT_EQ(mars.get_store(02035), 0'0000'0000'0000'1727u); // overwritten
//  EXPECT_EQ(mars.get_store(01415), 0'0000'0000'0000'0013u); // overwritten
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(02015), 0'0524'6520'4010'0000u);
    EXPECT_EQ(mars.get_store(01641), 0'0000'0000'0000'4000u);
//  EXPECT_EQ(mars.get_store(01516), 0'0000'0000'0000'0003u); // overwritten
    EXPECT_EQ(mars.get_store(02030), 0'0030'0000'0000'0000u); // latest
    EXPECT_EQ(mars.get_store(02024), 0'0000'0000'0000'6000u);
    EXPECT_EQ(mars.get_store(04001), 0'0000'0000'0000'7717u);
//  EXPECT_EQ(mars.get_store(02035), 0'0000'0000'0000'1717u); // overwritten
//  EXPECT_EQ(mars.get_store(02036), 0'0000'0000'0000'5720u); // overwritten
//  EXPECT_EQ(mars.get_store(02036), 0'0000'0000'0000'5721u); // overwritten

    // To DB: 12(8) words from 10000 to 5721
    EXPECT_EQ(mars.get_store(05732), 0'6400'0000'0000'0011u);
    EXPECT_EQ(mars.get_store(05731), 0'6400'0000'0000'0010u);
    EXPECT_EQ(mars.get_store(05730), 0'6400'0000'0000'0007u);
    EXPECT_EQ(mars.get_store(05727), 0'6400'0000'0000'0006u);
    EXPECT_EQ(mars.get_store(05726), 0'6400'0000'0000'0005u);
    EXPECT_EQ(mars.get_store(05725), 0'6400'0000'0000'0004u);
    EXPECT_EQ(mars.get_store(05724), 0'6400'0000'0000'0003u);
    EXPECT_EQ(mars.get_store(05723), 0'6400'0000'0000'0002u);
    EXPECT_EQ(mars.get_store(05722), 0'6400'0000'0000'0001u);
    EXPECT_EQ(mars.get_store(05721), 0'6400'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(04004), 0'0030'0000'0002'7717u);

    // Reducing free 1727 by len 13 + 1
    EXPECT_EQ(mars.get_store(05734), 0'0000'0000'0000'1713u);
    // Got 1713

    EXPECT_EQ(mars.get_store(05720), 0'0000'0000'0000'0012u);
//  EXPECT_EQ(mars.get_store(01647), 0'0000'0000'0000'0001u); // overwritten
    EXPECT_EQ(mars.get_store(01412), 0'0000'0000'0000'6000u);
    EXPECT_EQ(mars.get_store(01405), 0'0000'0000'0000'0026u); // latest
    EXPECT_EQ(mars.get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 26
//  EXPECT_EQ(mars.get_store(02011), 0'0000'0000'0000'0173u); // overwritten
    EXPECT_EQ(mars.get_store(01435), 0'0000'0000'0000'6000u); // latest
    EXPECT_EQ(mars.get_store(02024), 0'0000'0000'0000'6000u);
    EXPECT_EQ(mars.get_store(01436), 0'0000'0000'0000'1460u);

    // Expanding 0 elements
    EXPECT_EQ(mars.get_store(01460), 0'0000'0000'0000'0173u);
    EXPECT_EQ(mars.get_store(01461), 0'0000'0000'0000'6000u);
    EXPECT_EQ(mars.get_store(01455), 0'0000'0000'0000'0004u);
    EXPECT_EQ(mars.get_store(02016), 0'0000'0000'0000'1455u); // latest
    EXPECT_EQ(mars.get_store(01415), 0'0000'0000'0000'0041u); // latest
    EXPECT_EQ(mars.get_store(02012), 0'0000'0000'0000'2000u);
    EXPECT_EQ(mars.get_store(02040), 0'0000'0000'0000'2000u);
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(02015), 0'0524'6520'4010'0000u);
    EXPECT_EQ(mars.get_store(01641), 0'0000'0000'0000'4000u);
//  EXPECT_EQ(mars.get_store(02035), 0'0000'0000'0000'2000u); // overwritten
    EXPECT_EQ(mars.get_store(02036), 0'0010'0000'0000'0000u); // latest
    EXPECT_EQ(mars.get_store(01516), 0'0000'0000'0000'0001u); // latest
    EXPECT_EQ(mars.get_store(01620), 0'0010'0000'0010'5735u);
    EXPECT_EQ(mars.get_store(01446), 0'0000'0000'0000'5736u);
    EXPECT_EQ(mars.get_store(01445), 0'0000'0000'0000'0042u);
    EXPECT_EQ(mars.get_store(05777), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05776), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05775), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05774), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05773), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05772), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05771), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05770), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05767), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05766), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05765), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05764), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05763), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05762), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05761), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05760), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05757), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05756), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05755), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05754), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05753), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05752), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05751), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05750), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05747), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05746), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05745), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05744), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05743), 0'0000'0000'0000'6000u);
    EXPECT_EQ(mars.get_store(05742), 0'0000'0000'0000'0173u);
    EXPECT_EQ(mars.get_store(05741), 0'0000'0000'0000'2000u);
    EXPECT_EQ(mars.get_store(05740), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(05737), 0'0000'0000'0000'0004u);
    EXPECT_EQ(mars.get_store(02035), 0'0000'0000'0000'0000u); // latest
    EXPECT_EQ(mars.get_store(02023), 0'0000'0000'0000'0041u); // latest
    EXPECT_EQ(mars.get_store(05736), 0'0000'0000'0000'0041u);
//  EXPECT_EQ(mars.get_store(01647), 0'0000'0000'0000'0001u); // overwritten
    EXPECT_EQ(mars.get_store(02011), 0'0000'0000'0000'0000u); // latest
    EXPECT_EQ(mars.get_store(02037), 0'0000'0200'0052'0001u);

    // Writing 520001 from address 4000
    EXPECT_EQ(mars.get_store(01647), 0'0000'0000'0000'0000u); // latest
}

TEST(mars, modd_stores)
{
    Mars mars(false);
    const int catalog_len = 1;
    const int file_len = 2;
    const std::string fname = tobesm("TEST");

    mars.zero_date = true;
    //mars.verbose = true;

    // Initializing the database catalog: 1 zone, starting from zone 0 on LUN 52 (arbitrary)
    mars.InitDB(052, 0, catalog_len);

    // Setting the root location
    mars.SetDB(052, 0, catalog_len);

    // Making a new array of 'len' zones, starting after the root catalog
    mars.newd(fname.c_str(), 052, catalog_len, file_len);

    // Opening it
    mars.opend(fname.c_str());

    // Initializing an array of 10 words
    const int PAGE1 = 010000;
    for (int i = 0; i < 10; ++i) {
        mars.data[PAGE1 + i] = (064LL << 42) + i;
    }

    std::string elt = "A";
    elt.resize(8);

    // Put data to the DB
    mars.putd(elt.c_str(), PAGE1, 10);

    // Update data with smaller size
    //mars.trace_stores = true;
    mars.memoize_stores = true;
    std::cout << "modd()\n";
    mars.modd(elt.c_str(), PAGE1 + 1, 7);

    EXPECT_EQ(mars.get_store(01410), 0'0000'0000'0000'0101u);
//  EXPECT_EQ(mars.get_store(01415), 0'0000'0000'0000'0007u); // overwritten
    EXPECT_EQ(mars.get_store(01413), 0'0000'0000'0001'0001u);
    EXPECT_EQ(mars.get_store(01403), 0'0020'4026'2100'1511u);
    EXPECT_EQ(mars.get_store(02007), 0'0000'0000'0000'4000u);
    EXPECT_EQ(mars.get_store(02013), 0'0000'0000'0000'1400u);
    EXPECT_EQ(mars.get_store(02041), 0'0340'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(02017), 0'0000'0000'0000'4000u);
    EXPECT_EQ(mars.get_store(02021), 0'0000'0000'0000'4000u);
//  EXPECT_EQ(mars.get_store(02020), 0'0000'0000'0007'6502u); // not needed
//  EXPECT_EQ(mars.get_store(01405), 0'0020'4026'2100'1511u); // overwritten
    EXPECT_EQ(mars.get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 11
//  EXPECT_EQ(mars.get_store(02027), 0'0000'0000'0000'0101u); // overwritten
    EXPECT_EQ(mars.get_store(01642), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(02027), 0'3777'7777'7777'7676u); // latest

    // Comparing 2 elements
    EXPECT_EQ(mars.get_store(01421), 0'0000'0100'0000'0002u);
    EXPECT_EQ(mars.get_store(01437), 0'0000'0000'0000'0101u);
    EXPECT_EQ(mars.get_store(01435), 0'0000'0000'0000'6000u);
//  EXPECT_EQ(mars.get_store(01405), 0'0000'2040'2621'0015u); // overwritten
    EXPECT_EQ(mars.get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 15
    EXPECT_EQ(mars.get_store(01405), 0'0000'0000'0000'0020u); // latest
    EXPECT_EQ(mars.get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 20
    EXPECT_EQ(mars.get_store(02016), 0'0000'0000'0001'0001u);
    EXPECT_EQ(mars.get_store(02012), 0'0000'0000'0000'6000u);
    EXPECT_EQ(mars.get_store(02040), 0'0000'0000'0000'6000u);
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(02015), 0'0524'6520'4010'0000u);
    EXPECT_EQ(mars.get_store(01641), 0'0000'0000'0000'4000u);
//  EXPECT_EQ(mars.get_store(02035), 0'0000'0000'0000'6000u); // overwritten
//  EXPECT_EQ(mars.get_store(02036), 0'0030'0000'0000'0000u); // overwritten
    EXPECT_EQ(mars.get_store(01516), 0'0000'0000'0000'0003u);
//  EXPECT_EQ(mars.get_store(01620), 0'0030'0000'0002'7717u); // overwritten
    EXPECT_EQ(mars.get_store(01446), 0'0000'0000'0000'5720u);
    EXPECT_EQ(mars.get_store(01445), 0'0000'0000'0000'0013u);
//  EXPECT_EQ(mars.get_store(02035), 0'0000'0000'0000'0000u); // overwritten
    EXPECT_EQ(mars.get_store(02023), 0'0000'0000'0000'0007u);
//  EXPECT_EQ(mars.get_store(02035), 0'0000'0000'0000'1725u); // overwritten
//  EXPECT_EQ(mars.get_store(02010), 0'0000'0000'0007'6501u); // not needed
    EXPECT_EQ(mars.get_store(02026), 0'0030'0000'0002'7717u);
//  EXPECT_EQ(mars.get_store(01620), 0'0000'0000'0000'1717u); // overwritten
    EXPECT_EQ(mars.get_store(02031), 0'0000'0000'0000'1717u);
//  EXPECT_EQ(mars.get_store(02035), 0'0000'0000'0000'1717u); // overwritten
//  EXPECT_EQ(mars.get_store(04001), 0'0000'0000'0000'5732u); // overwritten
//  EXPECT_EQ(mars.get_store(05734), 0'0000'0000'0000'1727u); // overwritten
//  EXPECT_EQ(mars.get_store(01647), 0'0000'0000'0000'0001u); // overwritten
//  EXPECT_EQ(mars.get_store(01647), 0'0000'0000'0000'0001u); // overwritten
    EXPECT_EQ(mars.get_store(01415), 0'0000'0000'0000'0010u); // latest
    EXPECT_EQ(mars.get_store(02030), 0'0030'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(02024), 0'0000'0000'0000'6000u);
//  EXPECT_EQ(mars.get_store(04001), 0'0000'0000'0000'7722u); // overwritten
    EXPECT_EQ(mars.get_store(02035), 0'0000'0000'0000'1722u); // latest
//  EXPECT_EQ(mars.get_store(02036), 0'0000'0000'0000'5723u); // overwritten
    EXPECT_EQ(mars.get_store(02036), 0'0000'0000'0000'5724u); // latest

    // To DB: 7(8) words from 10001 to 5724
    EXPECT_EQ(mars.get_store(05732), 0'6400'0000'0000'0007u);
    EXPECT_EQ(mars.get_store(05731), 0'6400'0000'0000'0006u);
    EXPECT_EQ(mars.get_store(05730), 0'6400'0000'0000'0005u);
    EXPECT_EQ(mars.get_store(05727), 0'6400'0000'0000'0004u);
    EXPECT_EQ(mars.get_store(05726), 0'6400'0000'0000'0003u);
    EXPECT_EQ(mars.get_store(05725), 0'6400'0000'0000'0002u);
    EXPECT_EQ(mars.get_store(05724), 0'6400'0000'0000'0001u);
    EXPECT_EQ(mars.get_store(04004), 0'0030'0000'0002'1722u);

    // Reducing free 1727 by len 10 + 1
    EXPECT_EQ(mars.get_store(05734), 0'0000'0000'0000'1716u); // latest
    // Got 1716

    EXPECT_EQ(mars.get_store(05723), 0'0000'0000'0000'0007u);
//  EXPECT_EQ(mars.get_store(01647), 0'0000'0000'0000'0001u); // overwritten
//  EXPECT_EQ(mars.get_store(01647), 0'0000'0000'0000'0001u); // overwritten
//  EXPECT_EQ(mars.get_store(01647), 0'0000'0000'0000'0001u); // overwritten
    EXPECT_EQ(mars.get_store(02011), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(02037), 0'0000'0200'0052'0001u);

    // Writing 520001 from address 4000
    EXPECT_EQ(mars.get_store(01647), 0'0000'0000'0000'0000u); // latest
}

TEST(mars, getd_num_stores)
{
    Mars mars(false);
    const int catalog_len = 1;
    const int file_len = 2;
    const std::string fname = tobesm("TEST");

    mars.zero_date = true;
    //mars.verbose = true;

    // Initializing the database catalog: 1 zone, starting from zone 0 on LUN 52 (arbitrary)
    mars.InitDB(052, 0, catalog_len);

    // Setting the root location
    mars.SetDB(052, 0, catalog_len);

    // Making a new array of 'len' zones, starting after the root catalog
    mars.newd(fname.c_str(), 052, catalog_len, file_len);

    // Opening it
    mars.opend(fname.c_str());

    // Initializing an array of 1024 words
    const int PAGE1 = 010000;
    const int PAGE2 = 012000;
    for (int i = 0; i < 1024; ++i) {
        mars.data[PAGE1 + i] = (064LL << 42) + i;
    }

    // Put element with numerical key
    mars.putd(123, PAGE1, 10);

    // Get it back
    //mars.trace_stores = true;
    mars.memoize_stores = true;
    std::cout << "getd()\n";
    mars.getd(123, PAGE2, 10);
    ASSERT_TRUE(compare(mars, PAGE2, PAGE1, 10));

    // Running getd(0000000000000173, 12000:10)
    EXPECT_EQ(mars.get_store(01410), 0'0000'0000'0000'0173u);
    EXPECT_EQ(mars.get_store(01415), 0'0000'0000'0000'0012u);
    EXPECT_EQ(mars.get_store(01413), 0'0000'0000'0001'2000u);
    EXPECT_EQ(mars.get_store(01403), 0'0000'0000'0022'1411u);
    EXPECT_EQ(mars.get_store(02007), 0'0000'0000'0000'4000u);
    EXPECT_EQ(mars.get_store(02013), 0'0000'0000'0000'1400u);
    EXPECT_EQ(mars.get_store(02041), 0'0340'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(02017), 0'0000'0000'0000'4000u);
    EXPECT_EQ(mars.get_store(02021), 0'0000'0000'0000'4000u);
//  EXPECT_EQ(mars.get_store(02020), 0'0000'0000'0007'6502u); // not needed
//  EXPECT_EQ(mars.get_store(01405), 0'0000'0000'0022'1411u); // overwritten
    EXPECT_EQ(mars.get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 11
//  EXPECT_EQ(mars.get_store(02027), 0'0000'0000'0000'0173u); // overwritten
    EXPECT_EQ(mars.get_store(01642), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(02027), 0'3777'7777'7777'7604u); // latest

    // Comparing 2 elements
    EXPECT_EQ(mars.get_store(01421), 0'0000'0100'0000'0002u);
    EXPECT_EQ(mars.get_store(01437), 0'0000'0000'0000'0173u);
    EXPECT_EQ(mars.get_store(01435), 0'0000'0000'0000'6000u);
//  EXPECT_EQ(mars.get_store(01405), 0'0000'0000'0000'2214u); // overwritten
    EXPECT_EQ(mars.get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 14
    EXPECT_EQ(mars.get_store(01405), 0'0000'0000'0000'0022u); // latest
    EXPECT_EQ(mars.get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 22
    EXPECT_EQ(mars.get_store(02040), 0'0000'0000'0000'6000u);
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(02015), 0'0524'6520'4010'0000u);
    EXPECT_EQ(mars.get_store(01641), 0'0000'0000'0000'4000u);
    EXPECT_EQ(mars.get_store(02035), 0'0000'0000'0000'6000u);
    EXPECT_EQ(mars.get_store(02036), 0'0030'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(01516), 0'0000'0000'0000'0003u);
    EXPECT_EQ(mars.get_store(01620), 0'0030'0000'0002'7717u);
//  EXPECT_EQ(mars.get_store(01446), 0'0000'0000'0000'5720u); // overwritten
//  EXPECT_EQ(mars.get_store(01445), 0'0000'0000'0000'0013u); // overwritten
    EXPECT_EQ(mars.get_store(01441), 0'0000'0000'0000'0012u);
    EXPECT_EQ(mars.get_store(01447), 0'0000'0000'0000'0012u);
    EXPECT_EQ(mars.get_store(01453), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(mars.get_store(02016), 0'0000'0000'0001'2000u); // overwritten
    EXPECT_EQ(mars.get_store(01446), 0'0000'0000'0000'5721u); // latest
    EXPECT_EQ(mars.get_store(01445), 0'0000'0000'0000'0012u); // latest

    // From DB: 12(8) words from 5721 to 12000
    EXPECT_EQ(mars.get_store(012011),0'6400'0000'0000'0011u);
    EXPECT_EQ(mars.get_store(012010),0'6400'0000'0000'0010u);
    EXPECT_EQ(mars.get_store(012007),0'6400'0000'0000'0007u);
    EXPECT_EQ(mars.get_store(012006),0'6400'0000'0000'0006u);
    EXPECT_EQ(mars.get_store(012005),0'6400'0000'0000'0005u);
    EXPECT_EQ(mars.get_store(012004),0'6400'0000'0000'0004u);
    EXPECT_EQ(mars.get_store(012003),0'6400'0000'0000'0003u);
    EXPECT_EQ(mars.get_store(012002),0'6400'0000'0000'0002u);
    EXPECT_EQ(mars.get_store(012001),0'6400'0000'0000'0001u);
    EXPECT_EQ(mars.get_store(012000),0'6400'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(02016), 0'0000'0000'0001'2012u); // latest
    EXPECT_EQ(mars.get_store(02011), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(01647), 0'0000'0000'0000'0000u);
}

TEST(mars, cleard_stores)
{
    Mars mars(false);
    const int catalog_len = 1;
    const int file_len = 2;
    const std::string fname = tobesm("TEST");

    mars.zero_date = true;
    //mars.verbose = true;

    // Initializing the database catalog: 1 zone, starting from zone 0 on LUN 52 (arbitrary)
    mars.InitDB(052, 0, catalog_len);

    // Setting the root location
    mars.SetDB(052, 0, catalog_len);

    // Making a new array of 'len' zones, starting after the root catalog
    mars.newd(fname.c_str(), 052, catalog_len, file_len);

    // Opening it
    mars.opend(fname.c_str());

    // Initializing an array of 1024 words
    const int PAGE1 = 010000;
    for (int i = 0; i < 1024; ++i) {
        mars.data[PAGE1 + i] = (064LL << 42) + i;
    }

    // Put element with numerical key
    mars.putd(123, PAGE1, 10);

    // Clear database
    //mars.trace_stores = true;
    mars.memoize_stores = true;
    std::cout << "cleard()\n";
    mars.cleard(false);

    EXPECT_EQ(mars.get_store(01410), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(01403), 0'0000'0000'3027'2302u);
    EXPECT_EQ(mars.get_store(02007), 0'0000'0000'0000'4000u);
    EXPECT_EQ(mars.get_store(02013), 0'0000'0000'0000'1400u);
    EXPECT_EQ(mars.get_store(02041), 0'0340'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(02017), 0'0000'0000'0000'4000u);
    EXPECT_EQ(mars.get_store(02021), 0'0000'0000'0000'4000u);
//    EXPECT_EQ(mars.get_store(02020), 0'0000'0000'0007'6502u); // fluctuates
//  EXPECT_EQ(mars.get_store(01405), 0'0000'0000'3027'2302u); // overwritten
    EXPECT_EQ(mars.get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 2
//  EXPECT_EQ(mars.get_store(02027), 0'3777'7777'7777'7777u); // overwritten
    EXPECT_EQ(mars.get_store(01642), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(mars.get_store(02027), 0'0000'0000'0000'0000u); // overwritten

    // Comparing 2 elements
//  EXPECT_EQ(mars.get_store(01421), 0'0000'0100'0000'0002u); // overwritten
//  EXPECT_EQ(mars.get_store(01437), 0'0000'0000'0000'0173u); // overwritten
//  EXPECT_EQ(mars.get_store(01435), 0'0000'0000'0000'6000u); // overwritten
//  EXPECT_EQ(mars.get_store(01405), 0'0000'0000'0030'2723u); // overwritten
    EXPECT_EQ(mars.get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 23
//  EXPECT_EQ(mars.get_store(02040), 0'0000'0000'0000'6000u); // overwritten
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(02015), 0'0524'6520'4010'0000u);
    EXPECT_EQ(mars.get_store(01641), 0'0000'0000'0000'4000u);
//  EXPECT_EQ(mars.get_store(02035), 0'0000'0000'0000'6000u); // overwritten
//  EXPECT_EQ(mars.get_store(02036), 0'0030'0000'0000'0000u); // overwritten
//  EXPECT_EQ(mars.get_store(01516), 0'0000'0000'0000'0003u); // overwritten
//  EXPECT_EQ(mars.get_store(01620), 0'0030'0000'0002'7717u); // overwritten
//  EXPECT_EQ(mars.get_store(01446), 0'0000'0000'0000'5720u); // overwritten
//  EXPECT_EQ(mars.get_store(01445), 0'0000'0000'0000'0013u); // overwritten
//  EXPECT_EQ(mars.get_store(02036), 0'0000'0000'0000'0003u); // overwritten
//  EXPECT_EQ(mars.get_store(02031), 0'0000'0000'0000'1717u); // overwritten
//  EXPECT_EQ(mars.get_store(02035), 0'0000'0000'0000'1717u); // overwritten
//  EXPECT_EQ(mars.get_store(04001), 0'0000'0000'0000'5732u); // overwritten
//  EXPECT_EQ(mars.get_store(05734), 0'0000'0000'0000'1727u); // overwritten
//  EXPECT_EQ(mars.get_store(01647), 0'0000'0000'0000'0001u); // overwritten
//  EXPECT_EQ(mars.get_store(01647), 0'0000'0000'0000'0001u); // overwritten
//  EXPECT_EQ(mars.get_store(01405), 0'0000'0000'0000'3027u); // overwritten
    EXPECT_EQ(mars.get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 27
    EXPECT_EQ(mars.get_store(01436), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(mars.get_store(01516), 0'0000'0100'0000'0036u); // overwritten
//  EXPECT_EQ(mars.get_store(02036), 0'0000'0000'0000'0004u); // overwritten
//  EXPECT_EQ(mars.get_store(02035), 0'0000'0000'0000'1462u); // overwritten
    EXPECT_EQ(mars.get_store(01460), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01461), 0'1234'5670'0765'4321u);
//  EXPECT_EQ(mars.get_store(01455), 0'0000'0000'0000'0002u); // overwritten
    EXPECT_EQ(mars.get_store(02016), 0'0000'0000'0000'1455u);
    EXPECT_EQ(mars.get_store(01415), 0'0000'0000'0000'0041u);
    EXPECT_EQ(mars.get_store(02012), 0'0000'0000'0000'2000u);
//  EXPECT_EQ(mars.get_store(02040), 0'0000'0000'0000'2000u); // overwritten
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(02015), 0'0524'6520'4010'0000u);
    EXPECT_EQ(mars.get_store(01641), 0'0000'0000'0000'4000u);
//  EXPECT_EQ(mars.get_store(02035), 0'0000'0000'0000'2000u); // overwritten
//  EXPECT_EQ(mars.get_store(02036), 0'0010'0000'0000'0000u); // overwritten
//  EXPECT_EQ(mars.get_store(01516), 0'0000'0000'0000'0001u); // overwritten
//  EXPECT_EQ(mars.get_store(01620), 0'0010'0000'0010'5735u); // overwritten
//  EXPECT_EQ(mars.get_store(01446), 0'0000'0000'0000'5736u); // overwritten
//  EXPECT_EQ(mars.get_store(01445), 0'0000'0000'0000'0042u); // overwritten
//  EXPECT_EQ(mars.get_store(05777), 0'1234'5670'0765'4321u); // overwritten
//  EXPECT_EQ(mars.get_store(05776), 0'1234'5670'0765'4321u); // overwritten
//  EXPECT_EQ(mars.get_store(05775), 0'1234'5670'0765'4321u); // overwritten
    EXPECT_EQ(mars.get_store(05774), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05773), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05772), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05771), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05770), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05767), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05766), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05765), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05764), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05763), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05762), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05761), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05760), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05757), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05756), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05755), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05754), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05753), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05752), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05751), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05750), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05747), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05746), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05745), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05744), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05743), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05742), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(05741), 0'0000'0000'0000'2000u);
    EXPECT_EQ(mars.get_store(05740), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(05737), 0'0000'0000'0000'0002u);
//  EXPECT_EQ(mars.get_store(02035), 0'0000'0000'0000'0000u); // overwritten
    EXPECT_EQ(mars.get_store(02023), 0'0000'0000'0000'0041u);
    EXPECT_EQ(mars.get_store(05736), 0'0000'0000'0000'0041u);
//  EXPECT_EQ(mars.get_store(01647), 0'0000'0000'0000'0001u); // overwritten
//  EXPECT_EQ(mars.get_store(01405), 0'0000'0000'0000'0030u); // overwritten
    EXPECT_EQ(mars.get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 30
//  EXPECT_EQ(mars.get_store(01405), 0'0000'0000'3027'2302u); // overwritten
    EXPECT_EQ(mars.get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 2
//  EXPECT_EQ(mars.get_store(02027), 0'3777'7777'7777'7777u); // overwritten
    EXPECT_EQ(mars.get_store(01642), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(02027), 0'0000'0000'0000'0000u); // latest

    // Comparing 1 elements
    EXPECT_EQ(mars.get_store(01421), 0'0000'0100'0000'0000u); // latest
    EXPECT_EQ(mars.get_store(01437), 0'0000'0000'0000'0000u); // latest
    EXPECT_EQ(mars.get_store(01435), 0'0000'0000'0000'2000u); // latest
//  EXPECT_EQ(mars.get_store(01405), 0'0000'0000'0030'2723u); // overwritten
    EXPECT_EQ(mars.get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 23
//  EXPECT_EQ(mars.get_store(02040), 0'0000'0000'0000'2000u); // overwritten
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(02015), 0'0524'6520'4010'0000u);
    EXPECT_EQ(mars.get_store(01641), 0'0000'0000'0000'4000u);
//  EXPECT_EQ(mars.get_store(02035), 0'0000'0000'0000'2000u); // overwritten
//  EXPECT_EQ(mars.get_store(02036), 0'0010'0000'0000'0000u); // overwritten
//  EXPECT_EQ(mars.get_store(01516), 0'0000'0000'0000'0001u); // overwritten
//  EXPECT_EQ(mars.get_store(01620), 0'0010'0000'0010'5735u); // overwritten
    EXPECT_EQ(mars.get_store(01446), 0'0000'0000'0000'5736u); // latest
    EXPECT_EQ(mars.get_store(01445), 0'0000'0000'0000'0042u); // latest
//  EXPECT_EQ(mars.get_store(02036), 0'0000'0000'0000'0002u); // overwritten
//  EXPECT_EQ(mars.get_store(04002), 0'0020'0000'0000'7732u); // overwritten
//  EXPECT_EQ(mars.get_store(02031), 0'0000'0000'0000'1735u); // overwritten
    EXPECT_EQ(mars.get_store(04002), 0'0020'0000'0000'7774u); // latest
//  EXPECT_EQ(mars.get_store(02035), 0'0000'0000'0000'1732u); // overwritten
//  EXPECT_EQ(mars.get_store(02035), 0'0000'0000'0000'5732u); // overwritten
    EXPECT_EQ(mars.get_store(02031), 0'0000'0000'0000'5774u); // latest
    EXPECT_EQ(mars.get_store(05777), 0'0000'0000'0000'1776u); // latest
    EXPECT_EQ(mars.get_store(05776), 0'0000'0000'0000'1727u); // latest
    EXPECT_EQ(mars.get_store(05775), 0'0000'0000'0000'0002u); // latest
    EXPECT_EQ(mars.get_store(04001), 0'0000'0000'0000'3774u); // latest
    EXPECT_EQ(mars.get_store(05734), 0'0000'0000'0000'1772u); // latest
//  EXPECT_EQ(mars.get_store(01647), 0'0000'0000'0000'0001u); // overwritten
//  EXPECT_EQ(mars.get_store(01647), 0'0000'0000'0000'0001u); // overwritten
    EXPECT_EQ(mars.get_store(01405), 0'0000'0000'0000'3027u); // latest
    EXPECT_EQ(mars.get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 27
    EXPECT_EQ(mars.get_store(01436), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(mars.get_store(01516), 0'0000'0100'0000'0000u); // overwritten
//  EXPECT_EQ(mars.get_store(02036), 0'0000'0000'0000'0002u); // overwritten
//  EXPECT_EQ(mars.get_store(02035), 0'0000'0000'0000'1460u); // overwritten
    EXPECT_EQ(mars.get_store(01456), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01457), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01455), 0'0000'0000'0000'0000u); // latest
    EXPECT_EQ(mars.get_store(02016), 0'0000'0000'0000'1455u);
    EXPECT_EQ(mars.get_store(01415), 0'0000'0000'0000'0041u);
    EXPECT_EQ(mars.get_store(02012), 0'0000'0000'0000'2000u);
    EXPECT_EQ(mars.get_store(02040), 0'0000'0000'0000'2000u); // latest
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(02015), 0'0524'6520'4010'0000u);
    EXPECT_EQ(mars.get_store(01641), 0'0000'0000'0000'4000u);
    EXPECT_EQ(mars.get_store(02035), 0'0000'0000'0000'2000u); // latest
    EXPECT_EQ(mars.get_store(02036), 0'0010'0000'0000'0000u); // latest
    EXPECT_EQ(mars.get_store(01516), 0'0000'0000'0000'0001u); // latest
    EXPECT_EQ(mars.get_store(01620), 0'0020'0000'0000'7774u); // latest

    // ERROR 3 (No such record)
    EXPECT_EQ(mars.get_store(02010), 0x20'68'63'75'73'20'6f'4eu); // text message 'No such '
    EXPECT_EQ(mars.get_store(02011), 0x49'00'64'72'6f'63'65'72u); // text message 'record\0I'
    EXPECT_EQ(mars.get_store(02037), 0'0000'0200'0052'0001u);

    // Writing 520001 from address 4000
    EXPECT_EQ(mars.get_store(01647), 0'0000'0000'0000'0000u); // latest
}
