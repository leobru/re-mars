#include <gtest/gtest.h>

#include "mars.h"

TEST(mars, initdb_stores)
{
    mars_flags.zero_date = true;
    mars_flags.trace_stores = true;
    mars_flags.verbose = true;

    InitDB(052, 0, 3);
    IOflush();

    EXPECT_EQ(get_store(01417), 0'0000'0000'0000'4000);
    EXPECT_EQ(get_store(01416), 0'0000'0000'0000'6000);
    EXPECT_EQ(get_store(02042), 0'2300'0150'0000'0000);
    EXPECT_EQ(get_store(02043), 0'2000'0027'2300'0160);
    EXPECT_EQ(get_store(02044), 0'2010'1532'2300'0156);
    EXPECT_EQ(get_store(02045), 0'2010'1533'0000'0000);
    EXPECT_EQ(get_store(02046), 0'0040'0016'2300'0410);
    EXPECT_EQ(get_store(02047), 0'2000'0011'2300'0375);
    EXPECT_EQ(get_store(02050), 0'5400'0242'2300'0404);
    EXPECT_EQ(get_store(02003), 0'0000'0000'0000'1400);
    EXPECT_EQ(get_store(01430), 0'0000'0000'0352'0000);
    EXPECT_EQ(get_store(01431), 0'0000'0000'0000'4000);
    EXPECT_EQ(get_store(01403), 0'0000'0000'0000'0010);
    EXPECT_EQ(get_store(02007), 0'0000'0000'0000'4700);
    EXPECT_EQ(get_store(02013), 0'0000'0000'0000'1654);
    EXPECT_EQ(get_store(02041), 0'0340'0000'0000'0000);
    EXPECT_EQ(get_store(02017), 0'0000'0000'0000'1603);
    EXPECT_EQ(get_store(02021), 0'0000'0000'0007'7770);
    EXPECT_EQ(get_store(02020), 0'0000'0000'0002'2731);
    EXPECT_EQ(get_store(01405), 0'0000'0000'0000'0010);
    EXPECT_EQ(get_store(02014), 0'0000'0000'0000'0000);

    // Executing microcode 10
    EXPECT_EQ(get_store(01644), 0'0000'0000'0000'0000);
    EXPECT_EQ(get_store(01451), 0'0000'0000'0000'0003);
    EXPECT_EQ(get_store(01444), 0'0000'0000'0052'0000);
    EXPECT_EQ(get_store(01641), 0'0000'0000'0000'4000);
//  EXPECT_EQ(get_store(02036), 0'0000'0200'0052'0000); // overwritten
//  EXPECT_EQ(get_store(04001), 0'0000'0000'0000'1777); // overwritten
    EXPECT_EQ(get_store(06002), 0'0000'0000'0000'1776);
//  EXPECT_EQ(get_store(04000), 0'0000'0000'0000'4002); // overwritten
//  EXPECT_EQ(get_store(02037), 0'0000'0200'0052'0002); // overwritten

    // Writing 520002 from address 4000
    EXPECT_EQ(get_store(06001), 0'0000'0000'0000'1776);
//  EXPECT_EQ(get_store(04000), 0'0000'0000'0000'4001); // overwritten
//  EXPECT_EQ(get_store(02037), 0'0000'0200'0052'0001); // overwritten

    // Writing 520001 from address 4000
//  EXPECT_EQ(get_store(06000), 0'0000'0000'0000'1776); // overwritten
    EXPECT_EQ(get_store(04000), 0'0000'0000'0000'4000); // latest
//  EXPECT_EQ(get_store(02037), 0'0000'0200'0052'0000); // overwritten

    // Writing 520000 from address 4000
    EXPECT_EQ(get_store(01434), 0'0000'0000'0000'6000);
    EXPECT_EQ(get_store(01413), 0'0000'0000'0000'6000);
    EXPECT_EQ(get_store(01420), 0'0000'0000'0000'2000);
    EXPECT_EQ(get_store(02011), 0'0000'0000'0000'0000);
    EXPECT_EQ(get_store(02012), 0'0000'0000'0000'2000);
//  EXPECT_EQ(get_store(01415), 0'0000'0000'0000'0041); // overwritten
    EXPECT_EQ(get_store(02010), 0'0000'0000'0000'0002);
//  EXPECT_EQ(get_store(02016), 0'0000'0000'0000'2010); // overwritten
//  EXPECT_EQ(get_store(02035), 0'0000'0000'0000'0000); // overwritten
//  EXPECT_EQ(get_store(02023), 0'0000'0000'0000'0041); // overwritten
//  EXPECT_EQ(get_store(02030), 0'0000'0000'0000'0000); // overwritten
//  EXPECT_EQ(get_store(02035), 0'0000'0000'0000'1776); // overwritten
//  EXPECT_EQ(get_store(01415), 0'0000'0000'0000'0042); // overwritten
    EXPECT_EQ(get_store(01644), 0'0000'0000'0000'0000);
    EXPECT_EQ(get_store(02015), 0'0000'0000'0000'4000);
    EXPECT_EQ(get_store(01641), 0'0000'0000'0000'4000);
//  EXPECT_EQ(get_store(01516), 0'0000'0000'0000'0001); // overwritten
//  EXPECT_EQ(get_store(02030), 0'0010'0000'0000'0000); // overwritten
//  EXPECT_EQ(get_store(02024), 0'0000'0000'0000'2000); // overwritten
//  EXPECT_EQ(get_store(04001), 0'0000'0000'0000'3735); // overwritten
//  EXPECT_EQ(get_store(02035), 0'0000'0000'0000'1735); // overwritten
//  EXPECT_EQ(get_store(02036), 0'0000'0000'0000'5736); // overwritten
//  EXPECT_EQ(get_store(02036), 0'0000'0000'0000'5737); // overwritten

    // To DB: 41(8) words from 20 to 5737
    EXPECT_EQ(get_store(05777), 0'1234'5670'0765'4321);
    EXPECT_EQ(get_store(05776), 0'1234'5670'0765'4321);
    EXPECT_EQ(get_store(05775), 0'1234'5670'0765'4321);
    EXPECT_EQ(get_store(05774), 0'1234'5670'0765'4321);
    EXPECT_EQ(get_store(05773), 0'1234'5670'0765'4321);
    EXPECT_EQ(get_store(05772), 0'1234'5670'0765'4321);
    EXPECT_EQ(get_store(05771), 0'1234'5670'0765'4321);
    EXPECT_EQ(get_store(05770), 0'1234'5670'0765'4321);
    EXPECT_EQ(get_store(05767), 0'1234'5670'0765'4321);
    EXPECT_EQ(get_store(05766), 0'1234'5670'0765'4321);
    EXPECT_EQ(get_store(05765), 0'1234'5670'0765'4321);
    EXPECT_EQ(get_store(05764), 0'1234'5670'0765'4321);
    EXPECT_EQ(get_store(05763), 0'1234'5670'0765'4321);
    EXPECT_EQ(get_store(05762), 0'1234'5670'0765'4321);
    EXPECT_EQ(get_store(05761), 0'1234'5670'0765'4321);
    EXPECT_EQ(get_store(05760), 0'1234'5670'0765'4321);
    EXPECT_EQ(get_store(05757), 0'1234'5670'0765'4321);
    EXPECT_EQ(get_store(05756), 0'1234'5670'0765'4321);
    EXPECT_EQ(get_store(05755), 0'1234'5670'0765'4321);
    EXPECT_EQ(get_store(05754), 0'1234'5670'0765'4321);
    EXPECT_EQ(get_store(05753), 0'1234'5670'0765'4321);
    EXPECT_EQ(get_store(05752), 0'1234'5670'0765'4321);
    EXPECT_EQ(get_store(05751), 0'1234'5670'0765'4321);
    EXPECT_EQ(get_store(05750), 0'1234'5670'0765'4321);
    EXPECT_EQ(get_store(05747), 0'1234'5670'0765'4321);
    EXPECT_EQ(get_store(05746), 0'1234'5670'0765'4321);
    EXPECT_EQ(get_store(05745), 0'1234'5670'0765'4321);
    EXPECT_EQ(get_store(05744), 0'1234'5670'0765'4321);
    EXPECT_EQ(get_store(05743), 0'1234'5670'0765'4321);
    EXPECT_EQ(get_store(05742), 0'1234'5670'0765'4321);
    EXPECT_EQ(get_store(05741), 0'0000'0000'0000'2000);
    EXPECT_EQ(get_store(05740), 0'0000'0000'0000'0000);
    EXPECT_EQ(get_store(05737), 0'0000'0000'0000'0002);
    EXPECT_EQ(get_store(04002), 0'0010'0000'0010'5735);

    // Reducing free 1776 by len 42 + 1
//  EXPECT_EQ(get_store(06000), 0'0000'0000'0000'1733); // overwritten
    // Got 1733

    EXPECT_EQ(get_store(05736), 0'0000'0000'0000'0041);
//  EXPECT_EQ(get_store(01647), 0'0000'0000'0000'0001); // overwritten
//  EXPECT_EQ(get_store(01415), 0'0000'0000'0000'0003); // overwritten
//  EXPECT_EQ(get_store(06000), 0'0000'0000'0000'1726); // overwritten
    EXPECT_EQ(get_store(02014), 0'0000'0000'0000'0000);

    // Executing microcode 21
    EXPECT_EQ(get_store(02016), 0'0000'0000'0000'6000); // latest
//  EXPECT_EQ(get_store(02035), 0'0000'0000'0000'0000); // overwritten
    EXPECT_EQ(get_store(02023), 0'0000'0000'0000'0003); // latest
//  EXPECT_EQ(get_store(02030), 0'0000'0000'0000'0000); // overwritten
//  EXPECT_EQ(get_store(02035), 0'0000'0000'0000'1726); // overwritten
    EXPECT_EQ(get_store(01415), 0'0000'0000'0000'0004); // latest
    EXPECT_EQ(get_store(01644), 0'0000'0000'0000'0000);
    EXPECT_EQ(get_store(02015), 0'0000'0000'0000'4000);
    EXPECT_EQ(get_store(01641), 0'0000'0000'0000'4000);
    EXPECT_EQ(get_store(01516), 0'0000'0000'0000'0002); // latest
    EXPECT_EQ(get_store(02030), 0'0020'0000'0000'0000); // latest
    EXPECT_EQ(get_store(02024), 0'0000'0000'0000'4000); // latest
    EXPECT_EQ(get_store(04001), 0'0000'0000'0000'5731); // latest
    EXPECT_EQ(get_store(02035), 0'0000'0000'0000'1731); // latest
//  EXPECT_EQ(get_store(02036), 0'0000'0000'0000'5732); // overwritten
    EXPECT_EQ(get_store(02036), 0'0000'0000'0000'5733); // latest

    // To DB: 3(8) words from 6000 to 5733
    EXPECT_EQ(get_store(05735), 0'0000'0000'0000'1776);
    EXPECT_EQ(get_store(05734), 0'0000'0000'0000'1776);
    EXPECT_EQ(get_store(05733), 0'0000'0000'0000'1726);
    EXPECT_EQ(get_store(04003), 0'0020'0000'0001'1731);

    // Reducing free 1726 by len 4 + 1
    EXPECT_EQ(get_store(06000), 0'0000'0000'0000'1721); // latest
    // Got 1721

    EXPECT_EQ(get_store(05732), 0'0000'0000'0000'0003);
//  EXPECT_EQ(get_store(01647), 0'0000'0000'0000'0001); // overwritten
    EXPECT_EQ(get_store(01412), 0'0000'0000'0000'4000);
    EXPECT_EQ(get_store(02011), 0'0000'0000'0000'0000);
    EXPECT_EQ(get_store(02037), 0'0000'0200'0052'0000); // latest

    // Writing 520000 from address 4000
    EXPECT_EQ(get_store(01647), 0'0000'0000'0000'0000); // latest
}
