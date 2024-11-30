#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <format>
#include <gtest/gtest.h>

#include "fixture.h"
#include "mars.h"

TEST(mars, thrash)
{
    const int repeats = 2;
    const int numrec = 1888;
    const int maxsize = 100;
    std::string result;
    InitDB(0, 0, 0100);
    SetDB(0, 0, 0100);
    root();
    int space = avail();
    result = std::format("Initially: {}\n", space);

    for (int rep = 0; rep < repeats; rep++) {
        // Putting elements with random keys and arbitrary data, of random sizes,
        // up to numrec or until the DB overflows
        for (int i = 1; i <= numrec; ++i) {
            int k = (random() % numrec) + 1;
            int size = random() % maxsize;
            if (modd(k | 024LL << 42, 010000, size)) {
                // An overflow error is possible
                result += std::format("Overflow when attempting to put record #{}\n", i);
                break;
            }
        }

        space = avail();
        result += std::format("Remaining: {}\n", space);

        while (uint64_t k = last()) {
            if (deld(k)) {
                std::cerr << std::format("Unexpected error at {:o}, stopping clearing\n", k);
                break;
            }
        }
        space = avail();
        result += std::format("After clearing: {}\n", space);
    }
    const std::string expect =
        R"(Initially: 65243
Overflow when attempting to put record #1886
Remaining: 8
After clearing: 65233
Overflow when attempting to put record #1880
Remaining: 1
After clearing: 65184
)";
    EXPECT_EQ(result, expect);
    IOdiscard();
}
