#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <numeric>
#include <format>
#include <array>
#include <gtest/gtest.h>

#include "fixture.h"
#include "mars.h"

static bool compare(uint64_t *start1, uint64_t *start2, int len) {
    for (int i = 0; i < len; ++i) {
        if (start1[i] != start2[i]) {
            return false;
        }
    }
    return true;
}

static void setup(Mars &mars, uint64_t *orig, uint64_t *base, uint64_t key, size_t len, size_t offset)
{
    mars.zero_date = true;
    mars.InitDB(052, 0, 2);
    mars.SetDB(052, 0, 2);
    mars.root();
    for (size_t i = 0; i < len; ++i)
        orig[i] = i + 12345;
    mars.putd(key, orig, len);
    mars.key = key;
    mars.offset = offset;       // offset 0 - header, 1 - first word of data, etc.
    mars.mylen = 3;             // length the the segment to be read/written/compared
    mars.myloc = base;          // source/destination
}

TEST(mars, read)
{
    Mars mars(false);
    auto key = Mars::tobesm("foobar");
    uint64_t orig[1024], base[1024];
    size_t len = 1024;
    size_t offset = 1000;
    setup(mars, orig, base, key, len, offset);
    Mars::Error e =
        mars.eval(Mars::mcprog(Mars::OP_FIND, Mars::OP_LENGTH, Mars::OP_SEEK, Mars::OP_READ));
    ASSERT_EQ(e, Mars::ERR_SUCCESS);
    EXPECT_EQ(mars.datumLen.d, len);
    EXPECT_TRUE(compare(base, orig + offset - 1, 3));
}

TEST(mars, write)
{
    Mars mars(false);
    auto key = Mars::tobesm("foobar");
    uint64_t orig[1024], base[1024];
    size_t len = 1024;
    size_t offset = 1000;
    setup(mars, orig, base, key, len, offset);
    base[0] = 0;
    base[1] = 1;
    base[2] = 2;
    Mars::Error e =
        mars.eval(Mars::mcprog(Mars::OP_FIND, Mars::OP_LENGTH, Mars::OP_SEEK, Mars::OP_WRITE));
    ASSERT_EQ(e, Mars::ERR_SUCCESS);
    EXPECT_EQ(mars.datumLen.d, len);
    ASSERT_EQ(mars.getd(key, base, 0), Mars::ERR_SUCCESS);
    auto expect = std::accumulate(orig, orig+len, 0) -
        12345*3 - 999 - 1000 - 1001 + 3;
    EXPECT_EQ(std::accumulate(base, base+len, 0), expect);
}

TEST(mars, compare_match)
{
    Mars mars(false);
    auto key = Mars::tobesm("foobar");
    uint64_t orig[1024], base[1024];
    size_t len = 1024;
    size_t offset = 1000;
    setup(mars, orig, base, key, len, offset);
    base[0] = 12345+999;
    base[1] = 12345+1000;
    base[2] = 12345+1001;
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
    uint64_t orig[1024], base[1024];
    size_t len = 1024;
    size_t offset = 1000;
    setup(mars, orig, base, key, len, offset);
    base[0] = 12345+999;
    base[1] = 12345+1000;
    base[2] = 12345+1002;       // will not match
    Mars::Error e =
        mars.eval(Mars::mcprog(Mars::OP_FIND, Mars::OP_LENGTH, Mars::OP_SEEK, Mars::OP_IFEQ, Mars::OP_DELKEY));
    ASSERT_EQ(e, Mars::ERR_SUCCESS);
    EXPECT_EQ(mars.datumLen.d, len);
    // comparison failed -> the next op (OP_DELKEY) was not executed, getd will succeed
    ASSERT_EQ(mars.getd(key, base, 0), Mars::ERR_SUCCESS);
}

TEST(mars, scatter)
{
    Mars mars(false);
    auto key = Mars::tobesm("foobar");
    uint64_t orig[1024], base[1024];
    size_t len = 1024;
    size_t offset = 1000;
    setup(mars, orig, base, key, len, offset);
    uint64_t zero = 0;
    Mars::Error e =
      mars.eval(Mars::mcprog(Mars::OP_FIND, Mars::OP_LENGTH));
    ASSERT_EQ(e, Mars::ERR_SUCCESS);
    // Will be scattering 0 at mutiple-of-128 positions
    std::array<std::unique_ptr<Mars::segment>, 9> scatter;
    for (int i = 0; i < 8; ++i) {
        scatter[i] = std::make_unique<Mars::segment>(&zero, (i+1)*128, 1);
    }
    scatter[8] = std::make_unique<Mars::segment>(nullptr);
    mars.data[Mars::BDVECT+014].u = reinterpret_cast<uint64_t*>(&scatter[0]);
    e = mars.eval(Mars::mcprog(Mars::OP_SEGMENT, Mars::Op(014), Mars::OP_UNPACK, Mars::OP_SEEK, Mars::OP_WRITE, Mars::OP_LOOP));
    ASSERT_EQ(e, Mars::ERR_SUCCESS);
    ASSERT_EQ(mars.getd(key, base, 0), Mars::ERR_SUCCESS);
    auto expect = std::accumulate(orig, orig+len, 0) -
             12345*8 - (1+2+3+4+5+6+7+8)*128 + 8;
    EXPECT_EQ(std::accumulate(base, base+len, 0), expect);
}

static std::string lengths(Mars & mars) {
    std::string ret;
    mars.find(12345);
    ret += "Length of 12345 = ";
    ret += std::to_string(mars.getlen());
    ret += '\n';
    mars.find(23456);
    ret += "Length of 23456 = ";
    ret += std::to_string(mars.getlen());
    ret += '\n';
    return ret;
}

TEST(mars, exchange)
{
    Mars mars(false);
    std::string result;
    uint64_t dummy[2];
    mars.InitDB(0, 0, 20);
    mars.SetDB(0, 0, 20);
    mars.root();
    mars.putd(12345, dummy, 1);
    mars.putd(23456, dummy, 2);
    result = lengths(mars);
    mars.find(12345);
    mars.eval(Mars::mcprog(Mars::OP_ASSIGN, Mars::Op(014), Mars::HANDLE));
    mars.find(23456);
    mars.eval(Mars::mcprog(Mars::OP_ASSIGN, Mars::Op(02), Mars::HANDLE));
    mars.eval(Mars::mcprog(Mars::OP_ASSIGN, Mars::ALLOC, Mars::Op(014)));
    mars.eval(Mars::mcprog(Mars::OP_REPLACE));
    mars.find(12345);
    mars.eval(Mars::mcprog(Mars::OP_ASSIGN, Mars::ALLOC, Mars::Op(02)));
    mars.eval(Mars::mcprog(Mars::OP_REPLACE));
    result += lengths(mars);
    const std::string expect = R"(Length of 12345 = 1
Length of 23456 = 2
Length of 12345 = 2
Length of 23456 = 1
)";
}
