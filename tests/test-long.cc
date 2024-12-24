#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <format>
#include <gtest/gtest.h>

#include "fixture.h"
#include "mars.h"

TEST(mars, maxsize)
{
    Mars & mars = *new Mars;
    std::string result;
    run_command(result, "rm -f 520[0-4]??");
    ASSERT_EQ(result, "");
    mars.zero_date = true;
    mars.InitDB(052, 0, 1);
    mars.SetDB(052, 0, 1);
    std::string fname = tobesm("TEST");
    mars.newd(fname.c_str(), 052, 1, 0427);
    mars.opend(fname.c_str());

    std::ostringstream ostr;
    // Putting elements of size 0 until fails
    for (int i = 1; i <= 65536; ++i) {
        if (mars.putd(i | 024LL << 42, 0, 0)) {
            // A DB corruption error is expected at the last iteration
            ostr << "Error " << mars.data[7].d << " while putting " << i << '\n';
            break;
        }
    }
    delete &mars;
    EXPECT_EQ(ostr.str(), "Error 2 while putting 65536\n");
    run_command(result, "sha1sum 520[0-4]??");
    EXPECT_EQ(result, file_contents("test-maxsize.gold"));
}
