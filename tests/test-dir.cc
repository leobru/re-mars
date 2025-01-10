#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <format>
#include <gtest/gtest.h>

#include "fixture.h"
#include "mars.h"

static Mars::Error mkdir(Mars & mars, uint64_t dir) {
    mars.data[Mars::BDVECT+010].d = dir;
    return mars.eval(Mars::mcprog(Mars::OP_FIND, Mars::OP_NOMATCH, Mars::OP_INSMETA, Mars::OP_ADDKEY));
}

static Mars::Error chdir(Mars & mars, uint64_t dir) {
    mars.data[Mars::BDVECT+010].d = dir;
    return mars.eval(Mars::mcprog(Mars::OP_FIND, Mars::OP_MATCH, Mars::OP_SETMETA));
}

static Mars::Error dotdot(Mars & mars) {
    return mars.eval(Mars::mcprog(Mars::OP_BEGIN, Mars::OP_SETMETA));
}

TEST(mars, dir)
{
    Mars mars(false);
    std::string result;
    mars.InitDB(0, 0, 1);
    mars.root();
    EXPECT_EQ(mkdir(mars, 12345), Mars::ERR_SUCCESS);
    ASSERT_EQ(mars.putd(1, 0, 0), Mars::ERR_SUCCESS); // adding 1 to root dir
    ASSERT_EQ(mars.putd(10, 0, 0), Mars::ERR_SUCCESS);
    EXPECT_EQ(chdir(mars, 12345), Mars::ERR_SUCCESS);
    EXPECT_EQ(mars.putd(1, 0, 0), Mars::ERR_SUCCESS); // adding 1 to 12345/
    ASSERT_EQ(mars.putd(2, 0, 0), Mars::ERR_SUCCESS); // adding 2 to 12345/
    EXPECT_EQ(mkdir(mars, 54321), Mars::ERR_SUCCESS);
    EXPECT_EQ(chdir(mars, 54321), Mars::ERR_SUCCESS);
    EXPECT_EQ(mars.putd(2, 0, 0), Mars::ERR_SUCCESS); // adding 2 to 12345/54321/
    EXPECT_EQ(dotdot(mars), Mars::ERR_SUCCESS);       // back to 12345/
    EXPECT_EQ(mars.getd(1, 0, 0), Mars::ERR_SUCCESS);  // 1 is there
    EXPECT_EQ(mars.putd(10, 0, 0), Mars::ERR_SUCCESS); // but not 10
    EXPECT_EQ(dotdot(mars), Mars::ERR_SUCCESS);        // back to root
    EXPECT_EQ(mars.putd(2, 0, 0), Mars::ERR_SUCCESS);  // 2 is not there
    EXPECT_EQ(mars.getd(10, 0, 0), Mars::ERR_SUCCESS); // but 10 is
}
