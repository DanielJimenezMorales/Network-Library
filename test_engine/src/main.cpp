#include "gtest/gtest.h"

int main(int, char**)
{
    testing::InitGoogleTest();
    // testing::GTEST_FLAG( filter ) =  "FileTests.CheckNavigateFolder";
    RUN_ALL_TESTS();
    return 0;
}
