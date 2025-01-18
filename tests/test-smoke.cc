#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <gtest/gtest.h>

#include "fixture.h"
#include "mars.h"

TEST(mars, diagnostics)
{
    Mars mars(false);
    uint64_t zero = 0, one = 1;
    uint64_t dummy[2];
    mars.SetDB(0, 0, 1);
    ASSERT_EQ(mars.status, Mars::ERR_BAD_CATALOG);
    mars.InitDB(0, 0, 1);
    mars.root();
    mars.next();
    ASSERT_EQ(mars.status, Mars::ERR_NO_CURR);
    ASSERT_EQ(mars.eval(Mars::mcprog(Mars::OP_BEGIN, Mars::OP_PREV)),
              Mars::ERR_NO_PREV);
    ASSERT_EQ(mars.eval(Mars::mcprog(Mars::OP_BEGIN, Mars::OP_NEXT)),
              Mars::ERR_NO_NEXT);
    ASSERT_EQ(mars.getd(zero, 0, 0), Mars::ERR_INV_NAME);
    ASSERT_EQ(mars.getd(one, 0, 0), Mars::ERR_NO_NAME);
    ASSERT_EQ(mars.putd(one, dummy, 2), Mars::ERR_SUCCESS);
    ASSERT_EQ(mars.putd(one, 0, 0), Mars::ERR_EXISTS);
    ASSERT_EQ(mars.getd(one, dummy, 0), Mars::ERR_SUCCESS);
    ASSERT_EQ(mars.getd(one, dummy, 1), Mars::ERR_TOO_LONG);
    mars.offset = 2;            // within the datum of length 2
    ASSERT_EQ(mars.eval(Mars::mcprog(Mars::OP_SEEK)), Mars::ERR_SUCCESS);
    mars.offset = 5;            // outside of datum
    ASSERT_EQ(mars.eval(Mars::mcprog(Mars::OP_SEEK)), Mars::ERR_SEEK);
    // The deld(one); operation is implemented using micro-instruction word chaining
    uint64_t cont[] = {
        Mars::mcprog(Mars::OP_MATCH, Mars::OP_CHAIN),
        Mars::mcprog(Mars::OP_FREE, Mars::OP_CHAIN),
        Mars::mcprog(Mars::OP_DELKEY)
    };
    mars.cont = cont;
    ASSERT_EQ(mars.eval(Mars::mcprog(Mars::OP_FIND, Mars::OP_CHAIN)), Mars::ERR_SUCCESS);
    ASSERT_EQ(mars.eval(Mars::mcprog(Mars::OP_BEGIN, Mars::OP_FREE, Mars::OP_DELKEY)),
              Mars::ERR_NO_RECORD);
    ASSERT_EQ(mars.eval(Mars::mcprog(Mars::OP_BEGIN, Mars::OP_SETMETA)),
              Mars::ERR_ZEROKEY);
    ASSERT_EQ(mars.eval(Mars::mcprog(Mars::OP_LOCK, Mars::OP_LOCK)), Mars::ERR_LOCKED);
}

static void init(Mars & mars, uint64_t *start, int len) {
    for (int i = 0; i < len; ++i) {
      start[i] = (064LL << 42) + i;
    }
}

static bool compare(uint64_t *start1, uint64_t *start2, int len) {
    for (int i = 0; i < len; ++i) {
        if (start1[i] != start2[i]) {
            return false;
        }
    }
    return true;
}

TEST(mars, initdb)
{
    Mars & mars = *new Mars;
    std::string result;
    run_command(result, "rm -f 52000[0-2]");
    ASSERT_EQ(result, "");
    mars.zero_date = true;
    mars.InitDB(052, 0, 3);
    delete &mars;
    run_command(result, R"(sha1sum --quiet -c - 2>&1 << EOF
76948c7d9cf5a68f7ba3b9c804c317d2ce904424  520000
3ada2ada7dc333451e34e7641d8190d55bbcca46  520001
2ec94808d03eea091b0548f4f42b85ad1a734814  520002
EOF
)");
    EXPECT_TRUE(result.empty());
}

TEST(mars, coverage)
{
    Mars & mars = *new Mars;
    std::string result;
    run_command(result, "rm -f 52000[0-2]");
    ASSERT_EQ(result, "");
    mars.zero_date = true;
    mars.InitDB(052, 0, 1);
    mars.SetDB(052, 0, 1);
    std::string fname = tobesm("TEST");
    ASSERT_EQ(mars.newd(fname.c_str(), 052, 1, 2), Mars::ERR_SUCCESS);
    ASSERT_EQ(mars.opend(fname.c_str()), Mars::ERR_SUCCESS);
    mars.disableSync = 3;

    uint64_t PAGE1[1024];
    uint64_t PAGE2[1024];

    // Initializing an array of 1024 words
    init(mars, PAGE1, 1024);

    std::string elt = "A";
    elt.resize(8);
    // Putting one half of it to the DB
    ASSERT_EQ(mars.modd(elt.c_str(), PAGE1, 512), Mars::ERR_SUCCESS);
    // Putting all of it (will be split: max usable words in a zone = 01775)
    ASSERT_EQ(mars.modd(elt.c_str(), PAGE1, 1024), Mars::ERR_SUCCESS);
    // Again (exact match of length)
    ASSERT_EQ(mars.modd(elt.c_str(), PAGE1, 1024), Mars::ERR_SUCCESS);
    // With smaller length (reusing the block)
    ASSERT_EQ(mars.modd(elt.c_str(), PAGE1+1, 1023), Mars::ERR_SUCCESS);
    // Getting back
    ASSERT_EQ(mars.getd(elt.c_str(), PAGE2, 1023), Mars::ERR_SUCCESS);
    EXPECT_TRUE(compare(PAGE2, PAGE1+1, 1023));
    EXPECT_EQ(mars.cleard(true), Mars::ERR_SUCCESS);

    // Putting 59 elements of varying sizes with numerical keys (key 0 is invalid)
    for (int i = 0; i <= 59; ++i) {
        auto e = mars.putd(i+1, PAGE1+1, i);
        EXPECT_EQ(e, i < 59 ? Mars::ERR_SUCCESS : Mars::ERR_OVERFLOW);
    }
    // Functionally a no-op
    mars.eval(Mars::mcprog(Mars::OP_BEGIN, Mars::OP_SETMETA));
    uint64_t k = mars.last();
    while (k) {
        int len = mars.getlen();
        EXPECT_EQ(mars.getd(k, PAGE2, 100), Mars::ERR_SUCCESS);
        EXPECT_TRUE(compare(PAGE2, PAGE1+1, len));
        k = mars.prev();
    }

    EXPECT_EQ(mars.cleard(false), Mars::ERR_SUCCESS);
    mars.eval(Mars::OP_SAVE);
    delete &mars;

    run_command(result, R"(sha1sum --quiet -c - 2>&1 << EOF
484f08dd503bb5a4a2757ce6be3e18dfdb23ca99  520000
038c042a23b75e82e659ab806b4f92089a7b6991  520001
0ab4c4f30aae195bc33691b6465dd3f2f9ddce61  520002
)");
}
