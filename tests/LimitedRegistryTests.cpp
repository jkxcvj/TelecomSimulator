#include <gtest/gtest.h>
#include <string>

#include "LimitedRegistry.h"

TEST(LimitedRegistryTests, RejectsDuplicatesAndValuesAboveTheSizeLimit)
{
    LimitedRegistry<int, std::string, 2> registry;

    EXPECT_TRUE(registry.add(1, "Alice"));
    EXPECT_TRUE(registry.add(2, "Bob"));
    EXPECT_FALSE(registry.add(3, "Tom"));
    EXPECT_FALSE(registry.add(1, "XYZ"));

    EXPECT_EQ(registry.size(), 2);
    EXPECT_TRUE(registry.contains(1));
    EXPECT_FALSE(registry.contains(3));
}
