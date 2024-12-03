#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <gtest/gtest.h>

#include "fixture.h"
#include "mars.h"

static void run_command(std::string &result, const std::string &cmd)
{
    // Run a standalone test executable.
    FILE *pipe = popen(cmd.c_str(), "r");
    ASSERT_TRUE(pipe != nullptr);

    // Capture output.
    result = stream_contents(pipe);
    std::cout << result;

    // Check exit code.
    int exit_status = pclose(pipe);
    int exit_code   = WEXITSTATUS(exit_status);
    ASSERT_NE(exit_status, -1);
    ASSERT_EQ(exit_code, 0);
}

TEST(mars, diagnostics)
{
    Mars mars(false);
    uint64_t zero = 0, one = 1;
    mars.InitDB(0, 0, 1);
    mars.root();
    ASSERT_EQ(mars.getd(zero, 0, 0), Mars::ERR_INV_NAME);
    ASSERT_EQ(mars.getd(one, 0, 0), Mars::ERR_NO_NAME);
    ASSERT_EQ(mars.putd(one, 0, 0), Mars::ERR_SUCCESS);
    ASSERT_EQ(mars.putd(one, 0, 0), Mars::ERR_EXISTS);
    ASSERT_EQ(mars.getd(one, 0, 0), Mars::ERR_SUCCESS);
    ASSERT_EQ(mars.deld(one), Mars::ERR_SUCCESS);
}

static void init(Mars & mars, int start, int len) {
    for (int i = 0; i < len; ++i) {
      mars.data[start+i] = (064LL << 42) + i;
    }
}

static bool compare(Mars & mars, int start1, int start2, int len) {
    for (int i = 0; i < len; ++i) {
        if (mars.data[start1+i] != mars.data[start2+i]) {
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
    run_command(result, "sha1sum 52000[0-2]");
    const std::string expect = R"(76948c7d9cf5a68f7ba3b9c804c317d2ce904424  520000
3ada2ada7dc333451e34e7641d8190d55bbcca46  520001
2ec94808d03eea091b0548f4f42b85ad1a734814  520002
)";
    EXPECT_EQ(result, expect);
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

    const int PAGE1 = 010000;
    const int PAGE2 = 012000;

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
    EXPECT_TRUE(compare(mars, PAGE2, PAGE1+1, 1023));
    EXPECT_EQ(mars.cleard(true), Mars::ERR_SUCCESS);

    // Putting 59 elements of varying sizes with numerical keys (key 0 is invalid)
    for (int i = 0; i <= 59; ++i) {
        auto e = mars.putd(i+1, PAGE1+1, i);
        EXPECT_EQ(e, i < 59 ? Mars::ERR_SUCCESS : Mars::ERR_OVERFLOW);
    }

    uint64_t k = mars.last();
    while (k) {
        int len = mars.getlen();
        EXPECT_EQ(mars.getd(k, PAGE2, 100), Mars::ERR_SUCCESS);
        EXPECT_TRUE(compare(mars, PAGE2, PAGE1+1, len));
        k = mars.prev();
    }

    EXPECT_EQ(mars.cleard(false), Mars::ERR_NO_RECORD);
    delete &mars;

    run_command(result, "sha1sum 52000[0-2]");
    const std::string expect =
        R"(484f08dd503bb5a4a2757ce6be3e18dfdb23ca99  520000
eddc535b966c84800da223dbe086df9024f52014  520001
0ab4c4f30aae195bc33691b6465dd3f2f9ddce61  520002
)";
    EXPECT_EQ(result, expect);
}
