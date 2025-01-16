#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <format>
#include <gtest/gtest.h>

#include "fixture.h"
#include "mars.h"

static Mars::Error mkdir(Mars & mars, uint64_t dir) {
    mars.key = dir;
    return mars.eval(Mars::mcprog(Mars::OP_FIND, Mars::OP_NOMATCH, Mars::OP_INSMETA, Mars::OP_ADDKEY));
}

static Mars::Error chdir(Mars & mars, uint64_t dir) {
    mars.key = dir;
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

// Before calling this rmdir, the directory to be removed must be the current directory
static Mars::Error rmdir(Mars & mars, uint64_t dir) {
    uint64_t cont[] = {
        Mars::mcprog(Mars::OP_LAST, Mars::OP_NOMATCH,
                     Mars::OP_COND, Mars::OP_FREE, Mars::OP_DELKEY, Mars::OP_LOOP,
                     Mars::OP_DELKEY, Mars::OP_CHAIN),
        Mars::mcprog(Mars::OP_ASSIGN, Mars::Op(010), Mars::Op(2), Mars::OP_CHAIN),
        Mars::mcprog(Mars::OP_ASSIGN, Mars::HANDLE, Mars::Op(014),
                     Mars::OP_SETMETA,
                     Mars::OP_FIND, Mars::OP_FREE, Mars::OP_DELKEY)
    };
    mars.cont = cont;
    mars.key = 0;
    mars.data[Mars::BDVECT+2] = dir;
    return mars.eval(Mars::mcprog(Mars::OP_BEGIN,
                                  Mars::OP_ASSIGN, Mars::Op(014), Mars::HANDLE,
                                  Mars::OP_CHAIN));
}

TEST(mars, cleardir)
{
    Mars mars(false);
    std::string result;
    mars.InitDB(0, 0, 20);
    mars.SetDB(0, 0, 20);
    mars.root();
    int before = mars.avail();
    EXPECT_EQ(mkdir(mars, 12345), Mars::ERR_SUCCESS);
    EXPECT_EQ(chdir(mars, 12345), Mars::ERR_SUCCESS);
    // Results in secondary metablocks for the directory
    for (int i = 1; i < 1000; ++i)
        ASSERT_EQ(mars.putd(i, 0, 0), Mars::ERR_SUCCESS);
    // This will not discard secondary metablocks
    EXPECT_EQ(mars.cleard(false), Mars::ERR_SUCCESS);
    EXPECT_EQ(dotdot(mars),  Mars::ERR_SUCCESS);
    EXPECT_EQ(mars.deld(12345), Mars::ERR_SUCCESS);
    int after = mars.avail();
    // There will be a memory leak of 10 words
    EXPECT_EQ(before - after, 10);
}

TEST(mars, rmdir)
{
    Mars mars(false);
    std::string result;
    mars.InitDB(0, 0, 20);
    mars.SetDB(0, 0, 20);
    mars.root();
    int before = mars.avail();
    EXPECT_EQ(mkdir(mars, 12345), Mars::ERR_SUCCESS);
    EXPECT_EQ(chdir(mars, 12345), Mars::ERR_SUCCESS);
    // Results in secondary metablocks for the directory
    for (int i = 1; i < 1000; ++i)
        ASSERT_EQ(mars.putd(i, 0, 0), Mars::ERR_SUCCESS);
    // This should discard all secondary metablocks
    EXPECT_EQ(rmdir(mars, 12345), Mars::ERR_SUCCESS);
    int after = mars.avail();
    // Therefore reclaiming all used memory
    EXPECT_EQ(before, after);
    // Checking that the directory had been deleted, and that the DB is not corrupt.
    ASSERT_EQ(mars.putd(12345, 0, 0), Mars::ERR_SUCCESS);
}
