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
    uint64_t zero = 0, one = 1;
    InitDB(0, 0, 1);
    root();
    ASSERT_EQ(getd(zero, 0, 0), ERR_INV_NAME);
    ASSERT_EQ(getd(one, 0, 0), ERR_NO_NAME);
    ASSERT_EQ(putd(one, 0, 0), ERR_SUCCESS);
    ASSERT_EQ(putd(one, 0, 0), ERR_EXISTS);
    ASSERT_EQ(getd(one, 0, 0), ERR_SUCCESS);
    ASSERT_EQ(deld(one), ERR_SUCCESS);
    IOdiscard();
}

static void zeromem() {
    for (int i = 0; i < RAM_LENGTH; ++i) {
        data[i] = 0;
    }
}

static void init(int start, int len) {
    for (int i = 0; i < len; ++i) {
      data[start+i] = (064LL << 42) + i;
    }
}

static bool compare(int start1, int start2, int len) {
    for (int i = 0; i < len; ++i) {
        if (data[start1+i] != data[start2+i]) {
            return false;
        }
    }
    return true;
}

TEST(mars, initdb)
{
    std::string result;
    run_command(result, "rm -f 52000[0-2]");
    ASSERT_EQ(result, "");
    mars_flags.zero_date = true;
    zeromem();
    InitDB(052, 0, 3);
    IOflush();
    run_command(result, "sha1sum 52000[0-2]");
    const std::string expect = R"(76948c7d9cf5a68f7ba3b9c804c317d2ce904424  520000
3ada2ada7dc333451e34e7641d8190d55bbcca46  520001
2ec94808d03eea091b0548f4f42b85ad1a734814  520002
)";
    EXPECT_EQ(result, expect);
}

TEST(mars, coverage)
{
    std::string result;
    run_command(result, "rm -f 52000[0-2]");
    ASSERT_EQ(result, "");
    mars_flags.zero_date = true;
    zeromem();
    InitDB(052, 0, 1);
    SetDB(052, 0, 1);
    std::string fname = tobesm("TEST");
    ASSERT_EQ(newd(fname.c_str(), 052, 1, 2), ERR_SUCCESS);
    ASSERT_EQ(opend(fname.c_str()), ERR_SUCCESS);

    const int PAGE1 = 010000;
    const int PAGE2 = 012000;

    // Initializing an array of 1024 words
    init(PAGE1, 1024);

    std::string elt = "A";
    elt.resize(8);
    // Putting one half of it to the DB
    ASSERT_EQ(modd(elt.c_str(), PAGE1, 512), ERR_SUCCESS);
    // Putting all of it (will be split: max usable words in a zone = 01775)
    ASSERT_EQ(modd(elt.c_str(), PAGE1, 1024), ERR_SUCCESS);
    // Again (exact match of length)
    ASSERT_EQ(modd(elt.c_str(), PAGE1, 1024), ERR_SUCCESS);
    // With smaller length (reusing the block)
    ASSERT_EQ(modd(elt.c_str(), PAGE1+1, 1023), ERR_SUCCESS);
    // Getting back
    ASSERT_EQ(getd(elt.c_str(), PAGE2, 1023), ERR_SUCCESS);
    EXPECT_TRUE(compare(PAGE2, PAGE1+1, 1023));
    EXPECT_EQ(deld(elt.c_str()), ERR_SUCCESS);

    // Putting 59 elements of varying sizes with numerical keys (key 0 is invalid)
    for (int i = 0; i <= 59; ++i) {
        auto e = putd(i+1, PAGE1+1, i);
        EXPECT_EQ(e, i < 59 ? ERR_SUCCESS : ERR_OVERFLOW);
    }

    uint64_t k = last();
    while (k) {
        int len = getlen();
        EXPECT_EQ(getd(k, PAGE2, 100), ERR_SUCCESS);
        EXPECT_TRUE(compare(PAGE2, PAGE1+1, len));
        k = prev();
    }

    EXPECT_EQ(cleard(), ERR_NO_RECORD);
    IOflush();

    run_command(result, "sha1sum 52000[0-2]");
    const std::string expect =
        R"(484f08dd503bb5a4a2757ce6be3e18dfdb23ca99  520000
eddc535b966c84800da223dbe086df9024f52014  520001
0ab4c4f30aae195bc33691b6465dd3f2f9ddce61  520002
)";
    EXPECT_EQ(result, expect);
}
