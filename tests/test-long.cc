#include <cstdio>
#include <cstdlib>
#include <set>
#include <numeric>
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
            ostr << "Error " << mars.status << " while putting " << i << '\n';
            break;
        }
    }
    delete &mars;
    EXPECT_EQ(ostr.str(), "Error 2 while putting 65536\n");
    run_command(result, "sha1sum --quiet -c test-maxsize.gold");
}

TEST(mars, clear)
{
  Mars mars(false);
    mars.InitDB(052, 0, 0427);
    mars.SetDB(052, 0, 0427);
    mars.root();
    int before = mars.avail();
    // One more element would cause a failure.
    for (int i = 1; i < 65535; ++i) {
        ASSERT_EQ (mars.putd(i, 0, 0), Mars::ERR_SUCCESS);
    }
    for (int i = 65534; i >= 1; --i) {
        if (i % 4096 != 0) {
            ASSERT_EQ (mars.deld(i), Mars::ERR_SUCCESS);
        }
    }
    mars.first();
    for (int i = 0; i < 8; ++i)
        mars.next();
    ASSERT_EQ(mars.eval(Mars::mcprog(Mars::OP_FREE, Mars::OP_DELKEY)), Mars::ERR_SUCCESS);
    EXPECT_EQ(mars.cleard(false), Mars::ERR_SUCCESS);
    int after = mars.avail();
    // Expect 3 sentinel blocks (extent descriptor + header, length, zero key, handle)
    EXPECT_EQ(before - after, 15);
}

static void del_forward(Mars & mars, std::set<int> &gold) {
    mars.first();
    auto it = gold.begin();
    for (int i = 0; i < 10000; ++i) {
        mars.next();
        ++it;
    }
    mars.eval(Mars::mcprog(Mars::OP_FREE, Mars::OP_DELKEY));
    gold.erase(it);
}

static void del_back(Mars & mars, std::set<int> &gold) {
    mars.last();
    auto it = gold.rbegin();
    for (int i = 0; i < 10000; ++i) {
        mars.prev();
        ++it;
    }
    mars.eval(Mars::mcprog(Mars::OP_FREE, Mars::OP_DELKEY));
    gold.erase(*it);
}

TEST(mars, stepdel)
{
    Mars mars(false);
    mars.InitDB(052, 0, 0427);
    mars.SetDB(052, 0, 0427);
    mars.root();
    std::set<int> gold;
    for (int i = 1; i <= 65530; ++i) {
        gold.insert(i);
        ASSERT_EQ(mars.putd(i, 0, 0), Mars::ERR_SUCCESS);
    }
    for (int i = 0; i < 2048; ++i)
        del_forward(mars, gold);

    for (int i = 0; i < 2048; ++i)
        del_back(mars, gold);

    int sum = 0;
    for (auto cur = mars.first(); cur != 65530; cur = mars.next())
        sum += cur;
    EXPECT_EQ(sum, std::accumulate(gold.begin(), --gold.end(), 0));
}
