#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <numeric>
#include <gtest/gtest.h>

#include "fixture.h"
#include "mars.h"

static bool compare(const Mars & mars, int start1, int start2, int len) {
    for (int i = 0; i < len; ++i) {
        if (mars.data[start1+i] != mars.data[start2+i]) {
            return false;
        }
    }
    return true;
}

static void setup(Mars & mars, uint64_t key, size_t len, size_t offset)
{
    mars.zero_date = true;
    mars.InitDB(052, 0, 2);
    mars.SetDB(052, 0, 2);
    mars.root();
    for (size_t i = 0; i < len; ++i)
        mars.data[010000+i] = i + 12345;
    mars.putd(key, 010000, len);
    mars.key = key;
    mars.offset = offset;       // offset 0 - header, 1 - first word of data, etc.
    mars.mylen = 3;             // length the the segment to be read/written/compared
    mars.myloc = 020000;        // source/destination
}

TEST(mars, read)
{
    Mars mars(false);
    auto key = Mars::tobesm("foobar");
    const int orig = 010000, base = 020000;
    size_t len = 1024;
    size_t offset = 1000;
    setup(mars, key, len, offset);
    Mars::Error e =
        mars.eval(Mars::mcprog(Mars::OP_FIND, Mars::OP_LENGTH, Mars::OP_SEEK, Mars::OP_READ));
    ASSERT_EQ(e, Mars::ERR_SUCCESS);
    EXPECT_EQ(mars.datumLen.d, len);
    EXPECT_TRUE(compare(mars, base, orig + offset - 1, 3));
}

TEST(mars, write)
{
    Mars mars(false);
    auto key = Mars::tobesm("foobar");
    const int orig = 010000, base = 020000;
    size_t len = 1024;
    size_t offset = 1000;
    setup(mars, key, len, offset);
    mars.data[base] = 0;
    mars.data[base+1] = 1;
    mars.data[base+2] = 2;
    Mars::Error e =
        mars.eval(Mars::mcprog(Mars::OP_FIND, Mars::OP_LENGTH, Mars::OP_SEEK, Mars::OP_WRITE));
    ASSERT_EQ(e, Mars::ERR_SUCCESS);
    EXPECT_EQ(mars.datumLen.d, len);
    ASSERT_EQ(mars.getd(key, base, 0), Mars::ERR_SUCCESS);
    auto expect = std::accumulate(&mars.data[orig].d, &mars.data[orig+len].d, 0) -
        12345*3 - 999 - 1000 - 1001 + 3;
    EXPECT_EQ(std::accumulate(&mars.data[base].d, &mars.data[base+len].d, 0), expect);
}

TEST(mars, compare_match)
{
    Mars mars(false);
    auto key = Mars::tobesm("foobar");
    const int base = 020000;
    size_t len = 1024;
    size_t offset = 1000;
    setup(mars, key, len, offset);
    mars.data[base] = 12345+999;
    mars.data[base+1] = 12345+1000;
    mars.data[base+2] = 12345+1001;
    Mars::Error e =
        mars.eval(Mars::mcprog(Mars::OP_FIND, Mars::OP_LENGTH, Mars::OP_SEEK, Mars::OP_IFEQ, Mars::OP_DELKEY));
    ASSERT_EQ(e, Mars::ERR_SUCCESS);
    EXPECT_EQ(mars.datumLen.d, len);
    // comparison succeeded -> the next op (OP_DELKEY) has been executed, getd will fail
    ASSERT_EQ(mars.getd(key, base, 0), Mars::ERR_NO_NAME);
}

TEST(mars, compare_nomatch)
{
    Mars mars(false);
    auto key = Mars::tobesm("foobar");
    const int base = 020000;
    size_t len = 1024;
    size_t offset = 1000;
    setup(mars, key, len, offset);
    mars.data[base] = 12345+999;
    mars.data[base+1] = 12345+1000;
    mars.data[base+2] = 12345+1002;
    Mars::Error e =
        mars.eval(Mars::mcprog(Mars::OP_FIND, Mars::OP_LENGTH, Mars::OP_SEEK, Mars::OP_IFEQ, Mars::OP_DELKEY));
    ASSERT_EQ(e, Mars::ERR_SUCCESS);
    EXPECT_EQ(mars.datumLen.d, len);
    // comparison failed -> the next op (OP_DELKEY) was not executed, getd will succeed
    ASSERT_EQ(mars.getd(key, base, 0), Mars::ERR_SUCCESS);
}
