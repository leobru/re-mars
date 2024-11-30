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

static bool compare(int start1, int start2, int len) {
    bool match = true;
    for (int i = 0; i < len; ++i) {
        if (data[start1+i] != data[start2+i]) {
            match = false;
            std::cout << "Element " << std::dec << i << " does not match ("
                      << data[start1+i].d << " vs " << data[start2+i].d << ")\n";
        }
    }
    if (match && mars_flags.verbose)
        std::cout << std::dec << len << " elements match between "
                  << std::oct << start1 << " and " << start2 << '\n';
    return match;
}

TEST(mars, initdb_stores)
{
    mars_flags.zero_date = true;
    mars_flags.memoize_stores = true;
    //mars_flags.verbose = true;

    InitDB(052, 0, 3);

    EXPECT_EQ(get_store(01417), 0'0000'0000'0000'4000u);
    EXPECT_EQ(get_store(01416), 0'0000'0000'0000'6000u);
    EXPECT_EQ(get_store(02042), 0'2300'0150'0000'0000u);
    EXPECT_EQ(get_store(02043), 0'2000'0027'2300'0160u);
    EXPECT_EQ(get_store(02044), 0'2010'1532'2300'0156u);
    EXPECT_EQ(get_store(02045), 0'2010'1533'0000'0000u);
    EXPECT_EQ(get_store(02046), 0'0040'0016'2300'0410u);
    EXPECT_EQ(get_store(02047), 0'2000'0011'2300'0375u);
    EXPECT_EQ(get_store(02050), 0'5400'0242'2300'0404u);
    EXPECT_EQ(get_store(02003), 0'0000'0000'0000'1400u);
    EXPECT_EQ(get_store(01430), 0'0000'0000'0352'0000u);
    EXPECT_EQ(get_store(01431), 0'0000'0000'0000'4000u);
    EXPECT_EQ(get_store(01403), 0'0000'0000'0000'0010u);
    EXPECT_EQ(get_store(02007), 0'0000'0000'0000'4700u);
    EXPECT_EQ(get_store(02013), 0'0000'0000'0000'1654u);
    EXPECT_EQ(get_store(02041), 0'0340'0000'0000'0000u);
    EXPECT_EQ(get_store(02017), 0'0000'0000'0000'1603u);
    EXPECT_EQ(get_store(02021), 0'0000'0000'0007'7770u);
    EXPECT_EQ(get_store(02020), 0'0000'0000'0002'2731u);
    EXPECT_EQ(get_store(01405), 0'0000'0000'0000'0010u);
    EXPECT_EQ(get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 10
    EXPECT_EQ(get_store(01644), 0'0000'0000'0000'0000u);
    EXPECT_EQ(get_store(01451), 0'0000'0000'0000'0003u);
    EXPECT_EQ(get_store(01444), 0'0000'0000'0052'0000u);
    EXPECT_EQ(get_store(01641), 0'0000'0000'0000'4000u);
//  EXPECT_EQ(get_store(02036), 0'0000'0200'0052'0000u); // overwritten
//  EXPECT_EQ(get_store(04001), 0'0000'0000'0000'1777u); // overwritten
    EXPECT_EQ(get_store(06002), 0'0000'0000'0000'1776u);
//  EXPECT_EQ(get_store(04000), 0'0000'0000'0000'4002u); // overwritten
//  EXPECT_EQ(get_store(02037), 0'0000'0200'0052'0002u); // overwritten

    // Writing 520002 from address 4000
    EXPECT_EQ(get_store(06001), 0'0000'0000'0000'1776u);
//  EXPECT_EQ(get_store(04000), 0'0000'0000'0000'4001u); // overwritten
//  EXPECT_EQ(get_store(02037), 0'0000'0200'0052'0001u); // overwritten

    // Writing 520001 from address 4000
//  EXPECT_EQ(get_store(06000), 0'0000'0000'0000'1776u); // overwritten
    EXPECT_EQ(get_store(04000), 0'0000'0000'0000'4000u); // latest
//  EXPECT_EQ(get_store(02037), 0'0000'0200'0052'0000u); // overwritten

    // Writing 520000 from address 4000
    EXPECT_EQ(get_store(01434), 0'0000'0000'0000'6000u);
    EXPECT_EQ(get_store(01413), 0'0000'0000'0000'6000u);
    EXPECT_EQ(get_store(01420), 0'0000'0000'0000'2000u);
    EXPECT_EQ(get_store(02011), 0'0000'0000'0000'0000u);
    EXPECT_EQ(get_store(02012), 0'0000'0000'0000'2000u);
//  EXPECT_EQ(get_store(01415), 0'0000'0000'0000'0041u); // overwritten
    EXPECT_EQ(get_store(02010), 0'0000'0000'0000'0002u);
//  EXPECT_EQ(get_store(02016), 0'0000'0000'0000'2010u); // overwritten
//  EXPECT_EQ(get_store(02035), 0'0000'0000'0000'0000u); // overwritten
//  EXPECT_EQ(get_store(02023), 0'0000'0000'0000'0041u); // overwritten
//  EXPECT_EQ(get_store(02030), 0'0000'0000'0000'0000u); // overwritten
//  EXPECT_EQ(get_store(02035), 0'0000'0000'0000'1776u); // overwritten
//  EXPECT_EQ(get_store(01415), 0'0000'0000'0000'0042u); // overwritten
    EXPECT_EQ(get_store(01644), 0'0000'0000'0000'0000u);
    EXPECT_EQ(get_store(02015), 0'0000'0000'0000'4000u);
    EXPECT_EQ(get_store(01641), 0'0000'0000'0000'4000u);
//  EXPECT_EQ(get_store(01516), 0'0000'0000'0000'0001u); // overwritten
//  EXPECT_EQ(get_store(02030), 0'0010'0000'0000'0000u); // overwritten
//  EXPECT_EQ(get_store(02024), 0'0000'0000'0000'2000u); // overwritten
//  EXPECT_EQ(get_store(04001), 0'0000'0000'0000'3735u); // overwritten
//  EXPECT_EQ(get_store(02035), 0'0000'0000'0000'1735u); // overwritten
//  EXPECT_EQ(get_store(02036), 0'0000'0000'0000'5736u); // overwritten
//  EXPECT_EQ(get_store(02036), 0'0000'0000'0000'5737u); // overwritten

    // To DB: 41(8) words from 20 to 5737
    EXPECT_EQ(get_store(05777), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05776), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05775), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05774), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05773), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05772), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05771), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05770), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05767), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05766), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05765), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05764), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05763), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05762), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05761), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05760), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05757), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05756), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05755), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05754), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05753), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05752), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05751), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05750), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05747), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05746), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05745), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05744), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05743), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05742), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05741), 0'0000'0000'0000'2000u);
    EXPECT_EQ(get_store(05740), 0'0000'0000'0000'0000u);
    EXPECT_EQ(get_store(05737), 0'0000'0000'0000'0002u);
    EXPECT_EQ(get_store(04002), 0'0010'0000'0010'5735u);

    // Reducing free 1776 by len 42 + 1
//  EXPECT_EQ(get_store(06000), 0'0000'0000'0000'1733u); // overwritten
    // Got 1733

    EXPECT_EQ(get_store(05736), 0'0000'0000'0000'0041u);
//  EXPECT_EQ(get_store(01647), 0'0000'0000'0000'0001u); // overwritten
//  EXPECT_EQ(get_store(01415), 0'0000'0000'0000'0003u); // overwritten
//  EXPECT_EQ(get_store(06000), 0'0000'0000'0000'1726u); // overwritten
    EXPECT_EQ(get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 21
    EXPECT_EQ(get_store(02016), 0'0000'0000'0000'6000u); // latest
//  EXPECT_EQ(get_store(02035), 0'0000'0000'0000'0000u); // overwritten
    EXPECT_EQ(get_store(02023), 0'0000'0000'0000'0003u); // latest
//  EXPECT_EQ(get_store(02030), 0'0000'0000'0000'0000u); // overwritten
//  EXPECT_EQ(get_store(02035), 0'0000'0000'0000'1726u); // overwritten
    EXPECT_EQ(get_store(01415), 0'0000'0000'0000'0004u); // latest
    EXPECT_EQ(get_store(01644), 0'0000'0000'0000'0000u);
    EXPECT_EQ(get_store(02015), 0'0000'0000'0000'4000u);
    EXPECT_EQ(get_store(01641), 0'0000'0000'0000'4000u);
    EXPECT_EQ(get_store(01516), 0'0000'0000'0000'0002u); // latest
    EXPECT_EQ(get_store(02030), 0'0020'0000'0000'0000u); // latest
    EXPECT_EQ(get_store(02024), 0'0000'0000'0000'4000u); // latest
    EXPECT_EQ(get_store(04001), 0'0000'0000'0000'5731u); // latest
    EXPECT_EQ(get_store(02035), 0'0000'0000'0000'1731u); // latest
//  EXPECT_EQ(get_store(02036), 0'0000'0000'0000'5732u); // overwritten
    EXPECT_EQ(get_store(02036), 0'0000'0000'0000'5733u); // latest

    // To DB: 3(8) words from 6000 to 5733
    EXPECT_EQ(get_store(05735), 0'0000'0000'0000'1776u);
    EXPECT_EQ(get_store(05734), 0'0000'0000'0000'1776u);
    EXPECT_EQ(get_store(05733), 0'0000'0000'0000'1726u);
    EXPECT_EQ(get_store(04003), 0'0020'0000'0001'1731u);

    // Reducing free 1726 by len 4 + 1
    EXPECT_EQ(get_store(06000), 0'0000'0000'0000'1721u); // latest
    // Got 1721

    EXPECT_EQ(get_store(05732), 0'0000'0000'0000'0003u);
//  EXPECT_EQ(get_store(01647), 0'0000'0000'0000'0001u); // overwritten
    EXPECT_EQ(get_store(01412), 0'0000'0000'0000'4000u);
    EXPECT_EQ(get_store(02011), 0'0000'0000'0000'0000u);
    EXPECT_EQ(get_store(02037), 0'0000'0200'0052'0000u); // latest

    // Writing 520000 from address 4000
    EXPECT_EQ(get_store(01647), 0'0000'0000'0000'0000u); // latest
}

TEST(mars, setdb_stores)
{
    const int catalog_len = 1;

    mars_flags.zero_date = true;
    //mars_flags.verbose = true;

    // Initializing the database catalog: 1 zone, starting from zone 0 on LUN 52 (arbitrary)
    InitDB(052, 0, catalog_len);

    // Setting the root location
    mars_flags.memoize_stores = true;
    std::cout << "SetDB()\n";
    SetDB(052, 0, catalog_len);

    EXPECT_EQ(get_store(01417), 0'0000'0000'0000'4000u);
    EXPECT_EQ(get_store(01416), 0'0000'0000'0000'6000u);
    EXPECT_EQ(get_store(02042), 0'2300'0150'0000'0000u);
    EXPECT_EQ(get_store(02043), 0'2000'0027'2300'0160u);
    EXPECT_EQ(get_store(02044), 0'2010'1532'2300'0156u);
    EXPECT_EQ(get_store(02045), 0'2010'1533'0000'0000u);
    EXPECT_EQ(get_store(02046), 0'0040'0016'2300'0410u);
    EXPECT_EQ(get_store(02047), 0'2000'0011'2300'0375u);
    EXPECT_EQ(get_store(02050), 0'5400'0242'2300'0404u);
    EXPECT_EQ(get_store(02003), 0'0000'0000'0000'1400u);
    EXPECT_EQ(get_store(01403), 0'0000'0000'0000'0031u);
    EXPECT_EQ(get_store(02007), 0'0000'0000'0000'4700u);
    EXPECT_EQ(get_store(02013), 0'0000'0000'0000'1654u);
    EXPECT_EQ(get_store(02041), 0'0340'0000'0000'0000u);
    EXPECT_EQ(get_store(02017), 0'0000'0000'0000'1603u);
    EXPECT_EQ(get_store(02021), 0'0000'0000'0007'7770u);
    EXPECT_EQ(get_store(02020), 0'0000'0000'0002'2731u);
    EXPECT_EQ(get_store(01405), 0'0000'0000'0000'0031u);
    EXPECT_EQ(get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 31
    EXPECT_EQ(get_store(01431), 0'0000'0000'0000'4000u);
    EXPECT_EQ(get_store(01430), 0'0000'0000'0152'0000u);
    EXPECT_EQ(get_store(01444), 0'0000'0000'0052'0000u);
    EXPECT_EQ(get_store(01451), 0'0000'0000'0000'0001u);
    EXPECT_EQ(get_store(02037), 0'0010'0200'0052'0000u);

    // Reading 520000 to address 4000
    EXPECT_EQ(get_store(01641), 0'0000'0000'0000'4000u);
    EXPECT_EQ(get_store(01642), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(get_store(01646), 0'0000'0000'0000'0000u); // overwritten
    EXPECT_EQ(get_store(01644), 0'0000'0000'0000'0000u);
    EXPECT_EQ(get_store(01434), 0'0000'0000'0000'5735u);
    EXPECT_EQ(get_store(01420), 0'0000'0000'0000'2000u);
//  EXPECT_EQ(get_store(01516), 0'0000'0000'0000'2000u); // overwritten
//  EXPECT_EQ(get_store(02016), 0'0000'0000'0000'1454u); // overwritten
    EXPECT_EQ(get_store(01450), 0'0000'0000'0000'1456u);
    EXPECT_EQ(get_store(01646), 0'0000'0000'0000'2000u); // latest
    EXPECT_EQ(get_store(02040), 0'0000'0000'0000'2000u);
    EXPECT_EQ(get_store(01644), 0'0000'0000'0000'0000u);
    EXPECT_EQ(get_store(02015), 0'0000'0000'0000'4000u);
    EXPECT_EQ(get_store(01641), 0'0000'0000'0000'4000u);
    EXPECT_EQ(get_store(02035), 0'0000'0000'0000'2000u);
    EXPECT_EQ(get_store(02036), 0'0010'0000'0000'0000u);
    EXPECT_EQ(get_store(01516), 0'0000'0000'0000'0001u); // latest
    EXPECT_EQ(get_store(01620), 0'0010'0000'0010'5735u);
    EXPECT_EQ(get_store(01446), 0'0000'0000'0000'5736u);
    EXPECT_EQ(get_store(01445), 0'0000'0000'0000'0042u);

    // From DB: 42(8) words from 5736 to 1454
    EXPECT_EQ(get_store(01515), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01514), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01513), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01512), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01511), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01510), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01507), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01506), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01505), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01504), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01503), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01502), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01501), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01500), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01477), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01476), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01475), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01474), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01473), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01472), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01471), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01470), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01467), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01466), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01465), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01464), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01463), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01462), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01461), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01460), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01457), 0'0000'0000'0000'2000u);
    EXPECT_EQ(get_store(01456), 0'0000'0000'0000'0000u);
    EXPECT_EQ(get_store(01455), 0'0000'0000'0000'0002u);
    EXPECT_EQ(get_store(01454), 0'0000'0000'0000'0041u);
    EXPECT_EQ(get_store(02016), 0'0000'0000'0000'1516u); // latest
    EXPECT_EQ(get_store(02011), 0'0000'0000'0000'0000u);
    EXPECT_EQ(get_store(01647), 0'0000'0000'0000'0000u);
}

TEST(mars, newd_stores)
{
    const int catalog_len = 1;
    const int file_len = 2;
    const std::string fname = tobesm("TEST");

    mars_flags.zero_date = true;
    //mars_flags.verbose = true;

    // Initializing the database catalog: 1 zone, starting from zone 0 on LUN 52 (arbitrary)
    InitDB(052, 0, catalog_len);

    // Setting the root location
    SetDB(052, 0, catalog_len);

    // Making a new array of 'len' zones, starting after the root catalog
    //mars_flags.trace_stores = true;
    mars_flags.memoize_stores = true;
    std::cout << "newd()\n";
    newd(fname.c_str(), 052, catalog_len, file_len);

    // Running newd('  TSET', 2520001)
    EXPECT_EQ(get_store(01410), 0'2504'2523'2502'0040u);
    EXPECT_EQ(get_store(077776),0'0000'0000'0252'0001u);
    EXPECT_EQ(get_store(077777),0'0524'6520'4010'0000u);
    EXPECT_EQ(get_store(01410), 0'2504'2523'2502'0040u);
//  EXPECT_EQ(get_store(01415), 0'0000'0000'0000'0001u); // overwritten
//  EXPECT_EQ(get_store(01413), 0'1234'5670'0765'4321u); // overwritten
//  EXPECT_EQ(get_store(01403), 0'0000'0000'0000'0000u); // overwritten
//  EXPECT_EQ(get_store(01413), 0'0000'0000'0007'7776u); // overwritten
//  EXPECT_EQ(get_store(01415), 0'0000'0000'0000'0002u); // overwritten
//  EXPECT_EQ(get_store(01403), 0'0000'0026'2115'1131u); // overwritten
//  EXPECT_EQ(get_store(02007), 0'0000'0000'0000'0000u); // overwritten
    EXPECT_EQ(get_store(02013), 0'0000'0000'0000'1400u);
    EXPECT_EQ(get_store(02041), 0'0340'0000'0000'0000u);
//  EXPECT_EQ(get_store(02017), 0'0000'0000'0000'1603u); // overwritten
//  EXPECT_EQ(get_store(02021), 0'0000'0000'0005'4321u); // overwritten
//  EXPECT_EQ(get_store(02020), 0'0000'0417'6274'3662u); // random address, don't check
//  EXPECT_EQ(get_store(01405), 0'0000'0026'2115'1131u); // overwritten
    EXPECT_EQ(get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 31
//  EXPECT_EQ(get_store(01431), 0'0000'0000'0000'4000u); // overwritten
//  EXPECT_EQ(get_store(01430), 0'0000'0000'0152'0000u); // overwritten
//  EXPECT_EQ(get_store(01444), 0'0000'0000'0052'0000u); // overwritten
//  EXPECT_EQ(get_store(01451), 0'0000'0000'0000'0001u); // overwritten
//  EXPECT_EQ(get_store(02037), 0'0010'0200'0052'0000u); // overwritten

    // Reading 520000 to address 4000
    EXPECT_EQ(get_store(01641), 0'0000'0000'0000'4000u);
    EXPECT_EQ(get_store(01642), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(get_store(01646), 0'0000'0000'0000'0000u); // overwritten
    EXPECT_EQ(get_store(01644), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(get_store(01434), 0'0000'0000'0000'5735u); // overwritten
    EXPECT_EQ(get_store(01420), 0'0000'0000'0000'2000u);
//  EXPECT_EQ(get_store(01516), 0'0000'0000'0000'2000u); // overwritten
//  EXPECT_EQ(get_store(02016), 0'0000'0000'0000'1454u); // overwritten
    EXPECT_EQ(get_store(01450), 0'0000'0000'0000'1456u);
    EXPECT_EQ(get_store(01646), 0'0000'0000'0000'2000u); // latest
//  EXPECT_EQ(get_store(02040), 0'0000'0000'0000'2000u); // overwritten
    EXPECT_EQ(get_store(01644), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(get_store(02015), 0'0000'0000'0000'4000u); // overwritten
    EXPECT_EQ(get_store(01641), 0'0000'0000'0000'4000u);
//  EXPECT_EQ(get_store(02035), 0'0000'0000'0000'2000u); // overwritten
//  EXPECT_EQ(get_store(02036), 0'0010'0000'0000'0000u); // overwritten
//  EXPECT_EQ(get_store(01516), 0'0000'0000'0000'0001u); // overwritten
//  EXPECT_EQ(get_store(01620), 0'0010'0000'0010'5735u); // overwritten
//  EXPECT_EQ(get_store(01446), 0'0000'0000'0000'5736u); // overwritten
//  EXPECT_EQ(get_store(01445), 0'0000'0000'0000'0042u); // overwritten

    // From DB: 42(8) words from 5736 to 1454   5
    EXPECT_EQ(get_store(01515), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01514), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01513), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01512), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01511), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01510), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01507), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01506), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01505), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01504), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01503), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01502), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01501), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01500), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01477), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01476), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01475), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01474), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01473), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01472), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01471), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01470), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01467), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01466), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01465), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01464), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01463), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01462), 0'1234'5670'0765'4321u);
//  EXPECT_EQ(get_store(01461), 0'1234'5670'0765'4321u); // overwritten
//  EXPECT_EQ(get_store(01460), 0'1234'5670'0765'4321u); // overwritten
    EXPECT_EQ(get_store(01457), 0'0000'0000'0000'2000u);
    EXPECT_EQ(get_store(01456), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(get_store(01455), 0'0000'0000'0000'0002u); // overwritten
    EXPECT_EQ(get_store(01454), 0'0000'0000'0000'0041u);
//  EXPECT_EQ(get_store(02016), 0'0000'0000'0000'1516u); // overwritten
//  EXPECT_EQ(get_store(01405), 0'0000'0000'2621'1511u); // overwritten
    EXPECT_EQ(get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 11
//  EXPECT_EQ(get_store(02027), 0'2504'2523'2502'0040u); // overwritten
    EXPECT_EQ(get_store(01642), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(get_store(02027), 0'1273'5254'5275'7737u); // overwritten

    // Comparing 1 elements
//  EXPECT_EQ(get_store(01421), 0'0000'0100'0000'0000u); // overwritten
//  EXPECT_EQ(get_store(01437), 0'0000'0000'0000'0000u); // overwritten
//  EXPECT_EQ(get_store(01435), 0'0000'0000'0000'2000u); // overwritten
    EXPECT_EQ(get_store(02014), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(get_store(01405), 0'0000'0000'0026'2115u); // overwritten
    EXPECT_EQ(get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 15
//  EXPECT_EQ(get_store(01405), 0'0000'0000'0000'2621u); // overwritten
    EXPECT_EQ(get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 21
//  EXPECT_EQ(get_store(02016), 0'0000'0000'0007'7776u); // overwritten
//  EXPECT_EQ(get_store(02035), 0'0000'0000'0000'0000u); // overwritten
//  EXPECT_EQ(get_store(02023), 0'0000'0000'0000'0002u); // overwritten
//  EXPECT_EQ(get_store(02030), 0'0000'0000'0000'0000u); // overwritten
//  EXPECT_EQ(get_store(02035), 0'0000'0000'0000'1730u); // overwritten
//  EXPECT_EQ(get_store(01415), 0'0000'0000'0000'0003u); // overwritten
    EXPECT_EQ(get_store(01644), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(get_store(02015), 0'0000'0000'0000'4000u); // overwritten
    EXPECT_EQ(get_store(01641), 0'0000'0000'0000'4000u);
//  EXPECT_EQ(get_store(01516), 0'0000'0000'0000'0003u); // overwritten
//  EXPECT_EQ(get_store(02030), 0'0030'0000'0000'0000u); // overwritten
//  EXPECT_EQ(get_store(02024), 0'0000'0000'0000'6000u); // overwritten
//  EXPECT_EQ(get_store(04001), 0'0000'0000'0000'7730u); // overwritten
//  EXPECT_EQ(get_store(02035), 0'0000'0000'0000'1730u); // overwritten
//  EXPECT_EQ(get_store(02036), 0'0000'0000'0000'5731u); // overwritten
//  EXPECT_EQ(get_store(02036), 0'0000'0000'0000'5732u); // overwritten

    // To DB: 2(8) words from 77776 to 5732
//  EXPECT_EQ(get_store(05733), 0'0524'6520'4010'0000u); // overwritten
    EXPECT_EQ(get_store(05732), 0'0000'0000'0252'0001u);
    EXPECT_EQ(get_store(04004), 0'0030'0000'0000'7730u);

    // Reducing free 1730 by len 3 + 1
//  EXPECT_EQ(get_store(05735), 0'0000'0000'0000'1724u); // overwritten

    // Got 1724
    EXPECT_EQ(get_store(05731), 0'0000'0000'0000'0002u);
//  EXPECT_EQ(get_store(01647), 0'0000'0000'0000'0001u); // overwritten
//  EXPECT_EQ(get_store(01412), 0'0000'0000'0000'6000u); // overwritten
//  EXPECT_EQ(get_store(01405), 0'0000'0000'0000'0026u); // overwritten
    EXPECT_EQ(get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 26
//  EXPECT_EQ(get_store(02011), 0'2504'2523'2502'0040u); // overwritten
//  EXPECT_EQ(get_store(01435), 0'0000'0000'0000'6000u); // overwritten
//  EXPECT_EQ(get_store(02024), 0'0000'0000'0000'6000u); // overwritten
    EXPECT_EQ(get_store(01436), 0'0000'0000'0000'1460u);

    // Expanding 0 elements
    EXPECT_EQ(get_store(01460), 0'2504'2523'2502'0040u); // latest
    EXPECT_EQ(get_store(01461), 0'0000'0000'0000'6000u); // latest
    EXPECT_EQ(get_store(01455), 0'0000'0000'0000'0004u); // latest
//  EXPECT_EQ(get_store(02016), 0'0000'0000'0000'1455u); // overwritten
//  EXPECT_EQ(get_store(01415), 0'0000'0000'0000'0041u); // overwritten
    EXPECT_EQ(get_store(02012), 0'0000'0000'0000'2000u);
//  EXPECT_EQ(get_store(02040), 0'0000'0000'0000'2000u); // overwritten
    EXPECT_EQ(get_store(01644), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(get_store(02015), 0'0000'0000'0000'4000u); // overwritten
    EXPECT_EQ(get_store(01641), 0'0000'0000'0000'4000u);
//  EXPECT_EQ(get_store(02035), 0'0000'0000'0000'2000u); // overwritten
//  EXPECT_EQ(get_store(02036), 0'0010'0000'0000'0000u); // overwritten
//  EXPECT_EQ(get_store(01516), 0'0000'0000'0000'0001u); // overwritten
//  EXPECT_EQ(get_store(01620), 0'0010'0000'0010'5735u); // overwritten
//  EXPECT_EQ(get_store(01446), 0'0000'0000'0000'5736u); // overwritten
//  EXPECT_EQ(get_store(01445), 0'0000'0000'0000'0042u); // overwritten
    EXPECT_EQ(get_store(05777), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05776), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05775), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05774), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05773), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05772), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05771), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05770), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05767), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05766), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05765), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05764), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05763), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05762), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05761), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05760), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05757), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05756), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05755), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05754), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05753), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05752), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05751), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05750), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05747), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05746), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05745), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05744), 0'1234'5670'0765'4321u);
//  EXPECT_EQ(get_store(05743), 0'0000'0000'0000'6000u); // overwritten
//  EXPECT_EQ(get_store(05742), 0'2504'2523'2502'0040u); // overwritten
    EXPECT_EQ(get_store(05741), 0'0000'0000'0000'2000u);
    EXPECT_EQ(get_store(05740), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(get_store(05737), 0'0000'0000'0000'0004u); // overwritten
//  EXPECT_EQ(get_store(02035), 0'0000'0000'0000'0000u); // overwritten
//  EXPECT_EQ(get_store(02023), 0'0000'0000'0000'0041u); // overwritten
    EXPECT_EQ(get_store(05736), 0'0000'0000'0000'0041u);
//  EXPECT_EQ(get_store(01647), 0'0000'0000'0000'0001u); // overwritten
    EXPECT_EQ(get_store(02014), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(get_store(02011), 0'0000'0000'0000'0000u); // overwritten
//  EXPECT_EQ(get_store(02037), 0'0000'0200'0052'0000u); // overwritten

    // Writing 520000 from address 4000
//  EXPECT_EQ(get_store(01647), 0'0000'0000'0000'0000u); // overwritten
    EXPECT_EQ(get_store(01403), 0'0000'0000'1012'1411u); // latest
    EXPECT_EQ(get_store(02007), 0'0000'0000'0000'4000u); // latest
    EXPECT_EQ(get_store(02013), 0'0000'0000'0000'1400u);
    EXPECT_EQ(get_store(02041), 0'0340'0000'0000'0000u);
    EXPECT_EQ(get_store(02017), 0'0000'0000'0000'4000u); // latest
    EXPECT_EQ(get_store(02021), 0'0000'0000'0000'4000u); // latest
//  EXPECT_EQ(get_store(02020), 0'0000'0417'6274'3474u); // random address, don't check
//  EXPECT_EQ(get_store(01405), 0'0000'0000'1012'1411u); // overwritten
    EXPECT_EQ(get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 11
//  EXPECT_EQ(get_store(02027), 0'2504'2523'2502'0040u); // overwritten
    EXPECT_EQ(get_store(01642), 0'0000'0000'0000'0000u);
    EXPECT_EQ(get_store(02027), 0'1273'5254'5275'7737u); // latest

    // Comparing 2 elements
    EXPECT_EQ(get_store(01421), 0'0000'0100'0000'0002u); // latest
    EXPECT_EQ(get_store(01437), 0'2504'2523'2502'0040u); // latest
    EXPECT_EQ(get_store(01435), 0'0000'0000'0000'6000u); // latest
    EXPECT_EQ(get_store(02014), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(get_store(01405), 0'0000'0000'0010'1214u); // overwritten
    EXPECT_EQ(get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 14
//  EXPECT_EQ(get_store(01405), 0'0000'0000'0000'1012u); // overwritten
    EXPECT_EQ(get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 12
//  EXPECT_EQ(get_store(01413), 0'0000'0000'0000'1430u); // overwritten
//  EXPECT_EQ(get_store(01415), 0'0000'0000'0000'0003u); // overwritten
    EXPECT_EQ(get_store(02040), 0'0000'0000'0000'6000u); // latest
    EXPECT_EQ(get_store(01644), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(get_store(02015), 0'0000'0000'0000'4000u); // overwritten
    EXPECT_EQ(get_store(01641), 0'0000'0000'0000'4000u);
//  EXPECT_EQ(get_store(02035), 0'0000'0000'0000'6000u); // overwritten
//  EXPECT_EQ(get_store(02036), 0'0030'0000'0000'0000u); // overwritten
//  EXPECT_EQ(get_store(01516), 0'0000'0000'0000'0003u); // overwritten
    EXPECT_EQ(get_store(01620), 0'0030'0000'0000'7730u); // latest
//  EXPECT_EQ(get_store(01446), 0'0000'0000'0000'5731u); // overwritten
//  EXPECT_EQ(get_store(01445), 0'0000'0000'0000'0003u); // overwritten
    EXPECT_EQ(get_store(01441), 0'0000'0000'0000'0002u);
    EXPECT_EQ(get_store(01447), 0'0000'0000'0000'0002u);
    EXPECT_EQ(get_store(01453), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(get_store(02016), 0'0000'0000'0000'1430u); // overwritten
    EXPECT_EQ(get_store(01446), 0'0000'0000'0000'5732u); // latest
    EXPECT_EQ(get_store(01445), 0'0000'0000'0000'0002u); // latest

    // From DB: 2(8) words from 5732 to 1430
    EXPECT_EQ(get_store(01431), 0'0524'6520'4010'0000u); // latest
    EXPECT_EQ(get_store(01430), 0'0000'0000'0252'0001u); // latest
//  EXPECT_EQ(get_store(02016), 0'0000'0000'0000'1432u); // overwritten
    EXPECT_EQ(get_store(01405), 0'0000'0000'0000'0010u); // latest
    EXPECT_EQ(get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 10
    EXPECT_EQ(get_store(01644), 0'0000'0000'0000'0000u);
    EXPECT_EQ(get_store(01451), 0'0000'0000'0000'0002u); // latest
    EXPECT_EQ(get_store(01444), 0'0000'0000'0052'0001u); // latest
    EXPECT_EQ(get_store(01641), 0'0000'0000'0000'4000u);
//  EXPECT_EQ(get_store(02036), 0'0000'0200'0052'0001u); // overwritten
//  EXPECT_EQ(get_store(04001), 0'0000'0000'0000'1777u); // overwritten
    EXPECT_EQ(get_store(06001), 0'0000'0000'0000'1776u);
//  EXPECT_EQ(get_store(04000), 0'0524'6520'4010'0001u); // overwritten
//  EXPECT_EQ(get_store(02037), 0'0000'0200'0052'0002u); // overwritten

    // Writing 520002 from address 4000
//  EXPECT_EQ(get_store(06000), 0'0000'0000'0000'1776u); // overwritten
    EXPECT_EQ(get_store(04000), 0'0524'6520'4010'0000u); // latest
//  EXPECT_EQ(get_store(02037), 0'0000'0200'0052'0001u); // overwritten

    // Writing 520001 from address 4000
    EXPECT_EQ(get_store(01434), 0'0000'0000'0000'6000u); // latest
    EXPECT_EQ(get_store(01413), 0'0000'0000'0000'6000u); // latest
    EXPECT_EQ(get_store(01420), 0'0000'0000'0000'2000u);
//  EXPECT_EQ(get_store(02011), 0'0000'0000'0000'0000u); // overwritten
    EXPECT_EQ(get_store(02012), 0'0000'0000'0000'2000u);
//  EXPECT_EQ(get_store(01415), 0'0000'0000'0000'0041u); // overwritten
    EXPECT_EQ(get_store(02010), 0'0000'0000'0000'0002u);
//  EXPECT_EQ(get_store(02016), 0'0000'0000'0000'2010u); // overwritten
//  EXPECT_EQ(get_store(02035), 0'0000'0000'0000'0000u); // overwritten
//  EXPECT_EQ(get_store(02023), 0'0000'0000'0000'0041u); // overwritten
//  EXPECT_EQ(get_store(02030), 0'0000'0000'0000'0000u); // overwritten
//  EXPECT_EQ(get_store(02035), 0'0000'0000'0000'1776u); // overwritten
//  EXPECT_EQ(get_store(01415), 0'0000'0000'0000'0042u); // overwritten
    EXPECT_EQ(get_store(01644), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(get_store(02015), 0'0524'6520'4010'0000u); // overwritten
    EXPECT_EQ(get_store(01641), 0'0000'0000'0000'4000u);
//  EXPECT_EQ(get_store(01516), 0'0000'0000'0000'0001u); // overwritten
//  EXPECT_EQ(get_store(02030), 0'0010'0000'0000'0000u); // overwritten
//  EXPECT_EQ(get_store(02024), 0'0000'0000'0000'2000u); // overwritten
//  EXPECT_EQ(get_store(04001), 0'0000'0000'0000'3735u); // overwritten
//  EXPECT_EQ(get_store(02035), 0'0000'0000'0000'1735u); // overwritten
//  EXPECT_EQ(get_store(02036), 0'0000'0000'0000'5736u); // overwritten
//  EXPECT_EQ(get_store(02036), 0'0000'0000'0000'5737u); // overwritten

    // To DB: 41(8) words from 20 to 5737
    EXPECT_EQ(get_store(05777), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05776), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05775), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05774), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05773), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05772), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05771), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05770), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05767), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05766), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05765), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05764), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05763), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05762), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05761), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05760), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05757), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05756), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05755), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05754), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05753), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05752), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05751), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05750), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05747), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05746), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05745), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05744), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(05743), 0'1234'5670'0765'4321u); // latest
    EXPECT_EQ(get_store(05742), 0'1234'5670'0765'4321u); // latest
    EXPECT_EQ(get_store(05741), 0'0000'0000'0000'2000u);
    EXPECT_EQ(get_store(05740), 0'0000'0000'0000'0000u);
    EXPECT_EQ(get_store(05737), 0'0000'0000'0000'0002u); // latest
    EXPECT_EQ(get_store(04002), 0'0010'0000'0010'5735u);

    // Reducing free 1776 by len 42 + 1
//  EXPECT_EQ(get_store(06000), 0'0000'0000'0000'1733u); // overwritten
    // Got 1733

    EXPECT_EQ(get_store(05736), 0'0000'0000'0000'0041u);
//  EXPECT_EQ(get_store(01647), 0'0000'0000'0000'0001u); // overwritten
//  EXPECT_EQ(get_store(01415), 0'0000'0000'0000'0002u); // overwritten
//  EXPECT_EQ(get_store(06000), 0'0000'0000'0000'1727u); // overwritten
    EXPECT_EQ(get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 21
    EXPECT_EQ(get_store(02016), 0'0000'0000'0000'6000u); // latest
//  EXPECT_EQ(get_store(02035), 0'0000'0000'0000'0000u); // overwritten
    EXPECT_EQ(get_store(02023), 0'0000'0000'0000'0002u); // latest
//  EXPECT_EQ(get_store(02030), 0'0000'0000'0000'0000u); // overwritten
//  EXPECT_EQ(get_store(02035), 0'0000'0000'0000'1727u); // overwritten
    EXPECT_EQ(get_store(01415), 0'0000'0000'0000'0003u); // latest
    EXPECT_EQ(get_store(01644), 0'0000'0000'0000'0000u);
    EXPECT_EQ(get_store(02015), 0'0524'6520'4010'0000u); // latest
    EXPECT_EQ(get_store(01641), 0'0000'0000'0000'4000u);
    EXPECT_EQ(get_store(01516), 0'0000'0000'0000'0002u); // latest
    EXPECT_EQ(get_store(02030), 0'0020'0000'0000'0000u); // latest
    EXPECT_EQ(get_store(02024), 0'0000'0000'0000'4000u); // latest
    EXPECT_EQ(get_store(04001), 0'0000'0000'0000'5732u); // latest
    EXPECT_EQ(get_store(02035), 0'0000'0000'0000'1732u); // latest
//  EXPECT_EQ(get_store(02036), 0'0000'0000'0000'5733u); // overwritten
    EXPECT_EQ(get_store(02036), 0'0000'0000'0000'5734u); // latest

    // To DB: 2(8) words from 6000 to 5734
    EXPECT_EQ(get_store(05735), 0'0000'0000'0000'1776u); // latest
    EXPECT_EQ(get_store(05734), 0'0000'0000'0000'1727u);
    EXPECT_EQ(get_store(04003), 0'0020'0000'0000'7732u);

    // Reducing free 1727 by len 3 + 1
    EXPECT_EQ(get_store(06000), 0'0000'0000'0000'1723u); // latest
    // Got 1723

    EXPECT_EQ(get_store(05733), 0'0000'0000'0000'0002u); // latest
//  EXPECT_EQ(get_store(01647), 0'0000'0000'0000'0001u); // overwritten
    EXPECT_EQ(get_store(01412), 0'0000'0000'0000'4000u); // latest
    EXPECT_EQ(get_store(02011), 0'0000'0000'0000'0000u); // latest
    EXPECT_EQ(get_store(02037), 0'0000'0200'0052'0001u); // latest

    // Writing 520001 from address 4000
    EXPECT_EQ(get_store(01647), 0'0000'0000'0000'0000u); // latest
}

TEST(mars, opend_stores)
{
    const int catalog_len = 1;
    const int file_len = 2;
    const std::string fname = tobesm("TEST");

    mars_flags.zero_date = true;
    //mars_flags.verbose = true;

    // Initializing the database catalog: 1 zone, starting from zone 0 on LUN 52 (arbitrary)
    InitDB(052, 0, catalog_len);

    // Setting the root location
    SetDB(052, 0, catalog_len);

    // Making a new array of 'len' zones, starting after the root catalog
    newd(fname.c_str(), 052, catalog_len, file_len);

    // Opening it
    //mars_flags.trace_stores = true;
    mars_flags.memoize_stores = true;
    std::cout << "opend()\n";
    opend(fname.c_str());

    // Running opend('  TSET')
    EXPECT_EQ(get_store(01410), 0'2504'2523'2502'0040u);
    EXPECT_EQ(get_store(01403), 0'0000'0025'1214'1131u);
    EXPECT_EQ(get_store(02007), 0'0000'0000'0000'6000u);
    EXPECT_EQ(get_store(02013), 0'0000'0000'0000'1400u);
    EXPECT_EQ(get_store(02041), 0'0340'0000'0000'0000u);
    EXPECT_EQ(get_store(02017), 0'0000'0000'0000'4000u);
    EXPECT_EQ(get_store(02021), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(get_store(02020), 0'0000'0406'1017'7065u); // random address, don't check
//  EXPECT_EQ(get_store(01405), 0'0000'0025'1214'1131u); // overwritten
    EXPECT_EQ(get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 31
//  EXPECT_EQ(get_store(01431), 0'0000'0000'0000'4000u); // overwritten
//  EXPECT_EQ(get_store(01430), 0'0000'0000'0152'0000u); // overwritten
//  EXPECT_EQ(get_store(01444), 0'0000'0000'0052'0000u); // overwritten
//  EXPECT_EQ(get_store(01451), 0'0000'0000'0000'0001u); // overwritten
//  EXPECT_EQ(get_store(02037), 0'0010'0200'0052'0000u); // overwritten

    // Reading 520000 to address 4000
    EXPECT_EQ(get_store(01641), 0'0000'0000'0000'4000u);
    EXPECT_EQ(get_store(01642), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(get_store(01646), 0'0000'0000'0000'0000u); // overwritten
    EXPECT_EQ(get_store(01644), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(get_store(01434), 0'0000'0000'0000'5735u); // overwritten
    EXPECT_EQ(get_store(01420), 0'0000'0000'0000'2000u);
//  EXPECT_EQ(get_store(01516), 0'0000'0000'0000'2000u); // overwritten
//  EXPECT_EQ(get_store(02016), 0'0000'0000'0000'1454u); // overwritten
    EXPECT_EQ(get_store(01450), 0'0000'0000'0000'1456u);
//  EXPECT_EQ(get_store(01646), 0'0000'0000'0000'2000u); // overwritten
//  EXPECT_EQ(get_store(02040), 0'0000'0000'0000'2000u); // overwritten
    EXPECT_EQ(get_store(01644), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(get_store(02015), 0'0000'0000'0000'4000u); // overwritten
    EXPECT_EQ(get_store(01641), 0'0000'0000'0000'4000u);
//  EXPECT_EQ(get_store(02035), 0'0000'0000'0000'2000u); // overwritten
//  EXPECT_EQ(get_store(02036), 0'0010'0000'0000'0000u); // overwritten
//  EXPECT_EQ(get_store(01516), 0'0000'0000'0000'0001u); // overwritten
//  EXPECT_EQ(get_store(01620), 0'0010'0000'0010'5735u); // overwritten
//  EXPECT_EQ(get_store(01446), 0'0000'0000'0000'5736u); // overwritten
//  EXPECT_EQ(get_store(01445), 0'0000'0000'0000'0042u); // overwritten

    // From DB: 42(8) words from 5736 to 1454
    EXPECT_EQ(get_store(01515), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01514), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01513), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01512), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01511), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01510), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01507), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01506), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01505), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01504), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01503), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01502), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01501), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01500), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01477), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01476), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01475), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01474), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01473), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01472), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01471), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01470), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01467), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01466), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01465), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01464), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01463), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01462), 0'1234'5670'0765'4321u);
//  EXPECT_EQ(get_store(01461), 0'0000'0000'0000'6000u); // overwritten
//  EXPECT_EQ(get_store(01460), 0'2504'2523'2502'0040u); // overwritten
    EXPECT_EQ(get_store(01457), 0'0000'0000'0000'2000u);
    EXPECT_EQ(get_store(01456), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(get_store(01455), 0'0000'0000'0000'0004u); // overwritten
    EXPECT_EQ(get_store(01454), 0'0000'0000'0000'0041u);
//  EXPECT_EQ(get_store(02016), 0'0000'0000'0000'1516u); // overwritten
//  EXPECT_EQ(get_store(01405), 0'0000'0000'2512'1411u); // overwritten
    EXPECT_EQ(get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 11
//  EXPECT_EQ(get_store(02027), 0'2504'2523'2502'0040u); // overwritten
    EXPECT_EQ(get_store(01642), 0'0000'0000'0000'0000u);
    EXPECT_EQ(get_store(02027), 0'1273'5254'5275'7737u); // latest

    // Comparing 2 elements
    EXPECT_EQ(get_store(01421), 0'0000'0100'0000'0002u);
    EXPECT_EQ(get_store(01437), 0'2504'2523'2502'0040u);
    EXPECT_EQ(get_store(01435), 0'0000'0000'0000'6000u);
    EXPECT_EQ(get_store(02014), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(get_store(01405), 0'0000'0000'0025'1214u); // overwritten
    EXPECT_EQ(get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 14
//  EXPECT_EQ(get_store(01405), 0'0000'0000'0000'2512u); // overwritten
    EXPECT_EQ(get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 12
    EXPECT_EQ(get_store(01413), 0'0000'0000'0000'1430u);
    EXPECT_EQ(get_store(01415), 0'0000'0000'0000'0003u);
//  EXPECT_EQ(get_store(02040), 0'0000'0000'0000'6000u); // overwritten
    EXPECT_EQ(get_store(01644), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(get_store(02015), 0'0000'0000'0000'4000u); // overwritten
    EXPECT_EQ(get_store(01641), 0'0000'0000'0000'4000u);
//  EXPECT_EQ(get_store(02035), 0'0000'0000'0000'6000u); // overwritten
//  EXPECT_EQ(get_store(02036), 0'0030'0000'0000'0000u); // overwritten
//  EXPECT_EQ(get_store(01516), 0'0000'0000'0000'0003u); // overwritten
//  EXPECT_EQ(get_store(01620), 0'0030'0000'0000'7730u); // overwritten
//  EXPECT_EQ(get_store(01446), 0'0000'0000'0000'5731u); // overwritten
//  EXPECT_EQ(get_store(01445), 0'0000'0000'0000'0003u); // overwritten
    EXPECT_EQ(get_store(01441), 0'0000'0000'0000'0002u);
    EXPECT_EQ(get_store(01447), 0'0000'0000'0000'0002u);
    EXPECT_EQ(get_store(01453), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(get_store(02016), 0'0000'0000'0000'1430u); // overwritten
//  EXPECT_EQ(get_store(01446), 0'0000'0000'0000'5732u); // overwritten
//  EXPECT_EQ(get_store(01445), 0'0000'0000'0000'0002u); // overwritten

    // From DB: 2(8) words from 5732 to 1430
    EXPECT_EQ(get_store(01431), 0'0524'6520'4010'0000u); // latest
    EXPECT_EQ(get_store(01430), 0'0000'0000'0252'0001u); // latest
//  EXPECT_EQ(get_store(02016), 0'0000'0000'0000'1432u); // overwritten
    EXPECT_EQ(get_store(01405), 0'0000'0000'0000'0025u); // latest
    EXPECT_EQ(get_store(02014), 0'0000'0000'0000'0000u);

    // Executing microcode 25
    EXPECT_EQ(get_store(01444), 0'0000'0000'0052'0001u); // latest
    EXPECT_EQ(get_store(01451), 0'0000'0000'0000'0002u); // latest
    EXPECT_EQ(get_store(02037), 0'0010'0200'0052'0001u); // latest

    // Reading 520001 to address 4000
    EXPECT_EQ(get_store(01641), 0'0000'0000'0000'4000u);
    EXPECT_EQ(get_store(01642), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(get_store(01646), 0'0000'0000'0000'0000u); // overwritten
    EXPECT_EQ(get_store(01644), 0'0000'0000'0000'0000u);
    EXPECT_EQ(get_store(01434), 0'0000'0000'0000'5734u); // latest
    EXPECT_EQ(get_store(01420), 0'0000'0000'0000'2000u);
//  EXPECT_EQ(get_store(01516), 0'0000'0000'0000'2000u); // overwritten
//  EXPECT_EQ(get_store(02016), 0'0000'0000'0000'1454u); // overwritten
    EXPECT_EQ(get_store(01450), 0'0000'0000'0000'1456u);
    EXPECT_EQ(get_store(01646), 0'0000'0000'0000'2000u); // latest
    EXPECT_EQ(get_store(02040), 0'0000'0000'0000'2000u); // latest
    EXPECT_EQ(get_store(01644), 0'0000'0000'0000'0000u);
    EXPECT_EQ(get_store(02015), 0'0524'6520'4010'0000u); // latest
    EXPECT_EQ(get_store(01641), 0'0000'0000'0000'4000u);
    EXPECT_EQ(get_store(02035), 0'0000'0000'0000'2000u); // latest
    EXPECT_EQ(get_store(02036), 0'0010'0000'0000'0000u); // latest
    EXPECT_EQ(get_store(01516), 0'0000'0000'0000'0001u); // latest
    EXPECT_EQ(get_store(01620), 0'0010'0000'0010'5735u); // latest
    EXPECT_EQ(get_store(01446), 0'0000'0000'0000'5736u); // latest
    EXPECT_EQ(get_store(01445), 0'0000'0000'0000'0042u); // latest

    // From DB: 42(8) words from 5736 to 1454
    EXPECT_EQ(get_store(01515), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01514), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01513), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01512), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01511), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01510), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01507), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01506), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01505), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01504), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01503), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01502), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01501), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01500), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01477), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01476), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01475), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01474), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01473), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01472), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01471), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01470), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01467), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01466), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01465), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01464), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01463), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01462), 0'1234'5670'0765'4321u);
    EXPECT_EQ(get_store(01461), 0'1234'5670'0765'4321u); // latest
    EXPECT_EQ(get_store(01460), 0'1234'5670'0765'4321u); // latest
    EXPECT_EQ(get_store(01457), 0'0000'0000'0000'2000u);
    EXPECT_EQ(get_store(01456), 0'0000'0000'0000'0000u);
    EXPECT_EQ(get_store(01455), 0'0000'0000'0000'0002u); // latest
    EXPECT_EQ(get_store(01454), 0'0000'0000'0000'0041u);
    EXPECT_EQ(get_store(02016), 0'0000'0000'0000'1516u); // latest
    EXPECT_EQ(get_store(02011), 0'0000'0000'0000'0000u);
    EXPECT_EQ(get_store(01647), 0'0000'0000'0000'0000u);
}

TEST(mars, cleard_stores)
{
    const int catalog_len = 1;
    const int file_len = 2;
    const std::string fname = tobesm("TEST");

    mars_flags.zero_date = true;
    //mars_flags.verbose = true;

    // Initializing the database catalog: 1 zone, starting from zone 0 on LUN 52 (arbitrary)
    InitDB(052, 0, catalog_len);

    // Setting the root location
    SetDB(052, 0, catalog_len);

    // Making a new array of 'len' zones, starting after the root catalog
    newd(fname.c_str(), 052, catalog_len, file_len);

    // Opening it
    opend(fname.c_str());

    // Initializing an array of 1024 words
    const int PAGE1 = 010000;
    const int PAGE2 = 012000;
    for (int i = 0; i < 1024; ++i) {
        data[PAGE1 + i] = (064LL << 42) + i;
    }

    std::string elt = "A";
    elt.resize(8);

    // Putting one half of it to the DB
    modd(elt.c_str(), PAGE1, 512);

    // Putting all of it (will be split: max usable words in a zone = 01775)
    modd(elt.c_str(), PAGE1, 1024);

    // Again (exact match of length)
    modd(elt.c_str(), PAGE1, 1024);

    // With smaller length (reusing the block)
    modd(elt.c_str(), PAGE1+1, 1023);

    // Getting back
    getd(elt.c_str(), PAGE2, 1023);
    ASSERT_TRUE(compare(PAGE2, PAGE1+1, 1023));

    // Done with it
    deld(elt.c_str());

    // Putting 59 elements of varying sizes with numerical keys (key 0 is invalid)
    for (int i = 0; i <= 59; ++i) {
        if (i < 59) {
            EXPECT_EQ(putd(i+1, PAGE1+1, i), 0)
                << "while putting " << std::dec << i;
        } else {
            // An overflow error is expected at the last iteration
            EXPECT_NE(putd(i+1, PAGE1+1, i), 0)
                << "while putting " << std::dec << i;
        }
    }

    uint64_t k = last();
    while (k) {
        int len = getlen();
        getd(k, PAGE2, 100);
        ASSERT_TRUE(compare(PAGE2, PAGE1+1, len))
            << "Found " << std::oct << k << ' ' << len;
        k = prev();
    }

    // A termination error is expected
    mars_flags.memoize_stores = true;
    std::cout << "cleard()\n";
    cleard();
    // TODO: more stores
    EXPECT_EQ(get_store(01450), 0'0000'0000'0000'1456u);

    IOflush();
}
