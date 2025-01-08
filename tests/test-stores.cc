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

static bool compare(const Mars & mars, int start1, int start2, int len) {
    bool match = true;
    auto const &data = mars.data;
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


    // Executing microcode 10
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(mars.get_store(04001), 0'0000'0000'0000'1777u); // overwritten
    EXPECT_EQ(mars.get_store(06002), 0'0000'0000'0000'1776u);
//  EXPECT_EQ(mars.get_store(04000), 0'0000'0000'0000'4002u); // overwritten

    // Writing 520002 from address 4000
    EXPECT_EQ(mars.get_store(06001), 0'0000'0000'0000'1776u);
//  EXPECT_EQ(mars.get_store(04000), 0'0000'0000'0000'4001u); // overwritten

    // Writing 520001 from address 4000
//  EXPECT_EQ(mars.get_store(06000), 0'0000'0000'0000'1776u); // overwritten
    EXPECT_EQ(mars.get_store(04000), 0'0000'0000'0000'4000u); // latest

    // Writing 520000 from address 4000
    EXPECT_EQ(mars.get_store(01420), 0'0000'0000'0000'2000u);
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(mars.get_store(02024), 0'0000'0000'0000'2000u); // overwritten
//  EXPECT_EQ(mars.get_store(04001), 0'0000'0000'0000'3735u); // overwritten

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
    EXPECT_EQ(mars.get_store(06000), 0'0000'0000'0000'1726u); // fixed: kept

    // Executing microcode 21
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(04001), 0'0000'0000'0000'5731u); // latest

    // To DB: 3(8) words from 6000 to 5733
    EXPECT_EQ(mars.get_store(05735), 0'0000'0000'0000'1776u);
    EXPECT_EQ(mars.get_store(05734), 0'0000'0000'0000'1776u);
    EXPECT_EQ(mars.get_store(05733), 0'0000'0000'0000'1726u); // eventual value in the DB
    EXPECT_EQ(mars.get_store(04003), 0'0020'0000'0001'1731u);

    // Reducing free 1726 by len 4 + 1
    // EXPECT_EQ(mars.get_store(06000), 0'0000'0000'0000'1721u); // corrected in 05733
    // Got 1721

    EXPECT_EQ(mars.get_store(05732), 0'0000'0000'0000'0003u);
//  EXPECT_EQ(mars.get_store(01647), 0'0000'0000'0000'0001u); // overwritten

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


    // Executing microcode 31

    // Reading 520000 to address 4000
    EXPECT_EQ(mars.get_store(01642), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(01420), 0'0000'0000'0000'2000u);
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);

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
    EXPECT_EQ(mars.get_store(077776),0'0000'0000'0252'0001u);
    EXPECT_EQ(mars.get_store(077777),0'0524'6520'4010'0000u);
//  EXPECT_EQ(mars.get_store(02007), 0'0000'0000'0000'0000u); // overwritten
//  EXPECT_EQ(mars.get_store(02020), 0'0000'0417'6274'3662u); // random address, don't check

    // Executing microcode 31

    // Reading 520000 to address 4000
    EXPECT_EQ(mars.get_store(01642), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(01420), 0'0000'0000'0000'2000u);
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);

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

    // Executing microcode 11
    EXPECT_EQ(mars.get_store(01642), 0'0000'0000'0000'0000u);

    // Comparing 1 elements
//  EXPECT_EQ(mars.get_store(01421), 0'0000'0100'0000'0000u); // overwritten

    // Executing microcode 15

    // Executing microcode 21
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(mars.get_store(02024), 0'0000'0000'0000'6000u); // overwritten
//  EXPECT_EQ(mars.get_store(04001), 0'0000'0000'0000'7730u); // overwritten

    // To DB: 2(8) words from 77776 to 5732
//  EXPECT_EQ(mars.get_store(05733), 0'0524'6520'4010'0000u); // overwritten
    EXPECT_EQ(mars.get_store(05732), 0'0000'0000'0252'0001u);
    EXPECT_EQ(mars.get_store(04004), 0'0030'0000'0000'7730u);

    // Reducing free 1730 by len 3 + 1
//  EXPECT_EQ(mars.get_store(05735), 0'0000'0000'0000'1724u); // overwritten

    // Got 1724
    EXPECT_EQ(mars.get_store(05731), 0'0000'0000'0000'0002u);
//  EXPECT_EQ(mars.get_store(01647), 0'0000'0000'0000'0001u); // overwritten

    // Executing microcode 26
//  EXPECT_EQ(mars.get_store(02024), 0'0000'0000'0000'6000u); // overwritten

    // Expanding 0 elements
    EXPECT_EQ(mars.get_store(01460), 0'2504'2523'2502'0040u); // latest
    EXPECT_EQ(mars.get_store(01461), 0'0000'0000'0000'6000u); // latest
    EXPECT_EQ(mars.get_store(01455), 0'0000'0000'0000'0004u); // latest
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);
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
    EXPECT_EQ(mars.get_store(05736), 0'0000'0000'0000'0041u);
//  EXPECT_EQ(mars.get_store(01647), 0'0000'0000'0000'0001u); // overwritten

    // Writing 520000 from address 4000
//  EXPECT_EQ(mars.get_store(01647), 0'0000'0000'0000'0000u); // overwritten
//  EXPECT_EQ(mars.get_store(02020), 0'0000'0417'6274'3474u); // random address, don't check

    // Executing microcode 11
    EXPECT_EQ(mars.get_store(01642), 0'0000'0000'0000'0000u);

    // Comparing 2 elements
    EXPECT_EQ(mars.get_store(01421), 0'0000'0100'0000'0002u); // latest

    // Executing microcode 14

    // Executing microcode 12
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(01453), 0'0000'0000'0000'0000u);

    // From DB: 2(8) words from 5732 to 1430

    // Executing microcode 10
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(mars.get_store(04001), 0'0000'0000'0000'1777u); // overwritten
    EXPECT_EQ(mars.get_store(06001), 0'0000'0000'0000'1776u);
//  EXPECT_EQ(mars.get_store(04000), 0'0524'6520'4010'0001u); // overwritten

    // Writing 520002 from address 4000
//  EXPECT_EQ(mars.get_store(06000), 0'0000'0000'0000'1776u); // overwritten
    EXPECT_EQ(mars.get_store(04000), 0'0524'6520'4010'0000u); // latest

    // Writing 520001 from address 4000
    EXPECT_EQ(mars.get_store(01420), 0'0000'0000'0000'2000u);
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(mars.get_store(02024), 0'0000'0000'0000'2000u); // overwritten
//  EXPECT_EQ(mars.get_store(04001), 0'0000'0000'0000'3735u); // overwritten

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
    EXPECT_EQ(mars.get_store(06000), 0'0000'0000'0000'1727u); // fixed: kept

    // Executing microcode 21
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(04001), 0'0000'0000'0000'5732u); // latest

    // To DB: 2(8) words from 6000 to 5734
    EXPECT_EQ(mars.get_store(05735), 0'0000'0000'0000'1776u); // latest
    EXPECT_EQ(mars.get_store(05734), 0'0000'0000'0000'1727u);
    EXPECT_EQ(mars.get_store(04003), 0'0020'0000'0000'7732u);

    // Reducing free 1727 by len 3 + 1
    // EXPECT_EQ(mars.get_store(06000), 0'0000'0000'0000'1723u); // corrected in place
    // Got 1723

    EXPECT_EQ(mars.get_store(05733), 0'0000'0000'0000'0002u); // latest
//  EXPECT_EQ(mars.get_store(01647), 0'0000'0000'0000'0001u); // overwritten

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
//  EXPECT_EQ(mars.get_store(02020), 0'0000'0406'1017'7065u); // random address, don't check

    // Executing microcode 31

    // Reading 520000 to address 4000
    EXPECT_EQ(mars.get_store(01642), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(01420), 0'0000'0000'0000'2000u);
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);

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

    // Executing microcode 11
    EXPECT_EQ(mars.get_store(01642), 0'0000'0000'0000'0000u);

    // Comparing 2 elements
    EXPECT_EQ(mars.get_store(01421), 0'0000'0100'0000'0002u);

    // Executing microcode 14

    // Executing microcode 12
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(01453), 0'0000'0000'0000'0000u);

    // From DB: 2(8) words from 5732 to 1430

    // Executing microcode 25

    // Reading 520001 to address 4000
    EXPECT_EQ(mars.get_store(01642), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(01420), 0'0000'0000'0000'2000u);
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);

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


    // Executing microcode 11
    EXPECT_EQ(mars.get_store(01642), 0'0000'0000'0000'0000u);

    // Comparing 2 elements
    EXPECT_EQ(mars.get_store(01421), 0'0000'0100'0000'0002u);

    // Executing microcode 14

    // Executing microcode 22
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(01453), 0'0000'0000'0000'0000u);

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


    // Executing microcode 11
    EXPECT_EQ(mars.get_store(01642), 0'0000'0000'0000'0000u);

    // Comparing 2 elements
    EXPECT_EQ(mars.get_store(01421), 0'0000'0100'0000'0002u);

    // Executing microcode 14

    // Executing microcode 23
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(04001), 0'0000'0000'0000'5732u);
    EXPECT_EQ(mars.get_store(05734), 0'0000'0000'0000'1727u);
//  EXPECT_EQ(mars.get_store(01647), 0'0000'0000'0000'0001u); // overwritten
//  EXPECT_EQ(mars.get_store(01647), 0'0000'0000'0000'0001u); // overwritten

    // Executing microcode 27
    EXPECT_EQ(mars.get_store(01460), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01461), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01455), 0'0000'0000'0000'0002u);
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);
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
    EXPECT_EQ(mars.get_store(05736), 0'0000'0000'0000'0041u);
    EXPECT_EQ(mars.get_store(01647), 0'0000'0000'0000'0000u); // latest
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

    // Executing microcode 11
    EXPECT_EQ(mars.get_store(01642), 0'0000'0000'0000'0000u);

    // Comparing 1 elements
    EXPECT_EQ(mars.get_store(01421), 0'0000'0100'0000'0000u);

    // Executing microcode 15

    // Executing microcode 21
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(04001), 0'0000'0000'0000'7717u);

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

    // Executing microcode 26

    // Expanding 0 elements
    EXPECT_EQ(mars.get_store(01460), 0'0000'0000'0000'0173u);
    EXPECT_EQ(mars.get_store(01461), 0'0000'0000'0000'6000u);
    EXPECT_EQ(mars.get_store(01455), 0'0000'0000'0000'0004u);
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);
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
    EXPECT_EQ(mars.get_store(05736), 0'0000'0000'0000'0041u);
//  EXPECT_EQ(mars.get_store(01647), 0'0000'0000'0000'0001u); // overwritten

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


    // Executing microcode 11
    EXPECT_EQ(mars.get_store(01642), 0'0000'0000'0000'0000u);

    // Comparing 2 elements
    EXPECT_EQ(mars.get_store(01421), 0'0000'0100'0000'0002u);

    // Executing microcode 15

    // Executing microcode 20
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(mars.get_store(04001), 0'0000'0000'0000'5732u); // overwritten
//  EXPECT_EQ(mars.get_store(05734), 0'0000'0000'0000'1727u); // overwritten
//  EXPECT_EQ(mars.get_store(01647), 0'0000'0000'0000'0001u); // overwritten
//  EXPECT_EQ(mars.get_store(01647), 0'0000'0000'0000'0001u); // overwritten
//  EXPECT_EQ(mars.get_store(04001), 0'0000'0000'0000'7722u); // overwritten

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

    // Executing microcode 11
    EXPECT_EQ(mars.get_store(01642), 0'0000'0000'0000'0000u);

    // Comparing 2 elements
    EXPECT_EQ(mars.get_store(01421), 0'0000'0100'0000'0002u);

    // Executing microcode 14

    // Executing microcode 22
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(01453), 0'0000'0000'0000'0000u);

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


    // Executing microcode 2
    EXPECT_EQ(mars.get_store(01642), 0'0000'0000'0000'0000u);

    // Comparing 2 elements
//  EXPECT_EQ(mars.get_store(01421), 0'0000'0100'0000'0002u); // overwritten

    // Executing microcode 23
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(mars.get_store(04001), 0'0000'0000'0000'5732u); // overwritten
//  EXPECT_EQ(mars.get_store(05734), 0'0000'0000'0000'1727u); // overwritten
//  EXPECT_EQ(mars.get_store(01647), 0'0000'0000'0000'0001u); // overwritten
//  EXPECT_EQ(mars.get_store(01647), 0'0000'0000'0000'0001u); // overwritten

    // Executing microcode 27
    EXPECT_EQ(mars.get_store(01460), 0'1234'5670'0765'4321u);
    EXPECT_EQ(mars.get_store(01461), 0'1234'5670'0765'4321u);
//  EXPECT_EQ(mars.get_store(01455), 0'0000'0000'0000'0002u); // overwritten
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);
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
    EXPECT_EQ(mars.get_store(05736), 0'0000'0000'0000'0041u);
//  EXPECT_EQ(mars.get_store(01647), 0'0000'0000'0000'0001u); // overwritten

    // Executing microcode 30

    // Executing microcode 2
    EXPECT_EQ(mars.get_store(01642), 0'0000'0000'0000'0000u);

    // Comparing 1 elements
    EXPECT_EQ(mars.get_store(01421), 0'0000'0100'0000'0000u); // latest

    // Executing microcode 23
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);
//  EXPECT_EQ(mars.get_store(04002), 0'0020'0000'0000'7732u); // overwritten
    EXPECT_EQ(mars.get_store(04002), 0'0000'0000'0000'0000u); // latest
    EXPECT_EQ(mars.get_store(05777), 0'1234'5670'0765'4321u); // latest
    EXPECT_EQ(mars.get_store(05776), 0'1234'5670'0765'4321u); // latest
    EXPECT_EQ(mars.get_store(05775), 0'1234'5670'0765'4321u); // latest
    EXPECT_EQ(mars.get_store(04001), 0'0000'0000'0000'5732u); // latest
    EXPECT_EQ(mars.get_store(05734), 0'0000'0000'0000'1727u); // latest
//  EXPECT_EQ(mars.get_store(01647), 0'0000'0000'0000'0001u); // overwritten
//  EXPECT_EQ(mars.get_store(01647), 0'0000'0000'0000'0001u); // overwritten

    // Executing microcode 27
    EXPECT_EQ(mars.get_store(01456), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(01457), 0'0000'0000'0000'0000u);
    EXPECT_EQ(mars.get_store(01455), 0'0000'0000'0000'0002u); // latest
    EXPECT_EQ(mars.get_store(01644), 0'0000'0000'0000'0000u);


    // Writing 520001 from address 4000
    EXPECT_EQ(mars.get_store(01647), 0'0000'0000'0000'0000u); // latest
}
