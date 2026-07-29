#include <gtest/gtest.h>

#include "User.h"

TEST(UserTests, ConstructorStoresId)
{
    User user(1, "John Doe", "123-456-7890");

    EXPECT_EQ(1, user.getId());
}

TEST(UserTests, ConstructorStoresName)
{
    User user(1, "John Doe", "123-456-7890");

    EXPECT_EQ("John Doe", user.getName());
}

TEST(UserTests, ConstructorStoresPhoneNumber)
{
    User user(1, "John Doe", "123-456-7890");

    EXPECT_EQ("123-456-7890", user.getPhoneNumber());
}
