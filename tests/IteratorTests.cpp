#include <algorithm>
#include <gtest/gtest.h>
#include <vector>

TEST(IteratorTests, DiffBetweenConstAndNonConstIt)
{
    std::vector<int> numbers{1, 2, 3};

    auto it = numbers.begin();

    *it = 4;

    EXPECT_EQ(*it, 4);
    EXPECT_EQ(numbers[0], 4);

    const std::vector<int> numbersC{1, 2, 3};

    auto itC = numbersC.cbegin();

    EXPECT_EQ(*itC, 1);
}