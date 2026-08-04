#include <gtest/gtest.h>

#include "User.h"

TEST(UserTests, ConstructorStoresId)
{
    User user(UserId{1}, "John Doe", "123-456-7890");

    EXPECT_EQ(UserId{1}, user.getId());
}

TEST(UserTests, ConstructorStoresName)
{
    User user(UserId{1}, "John Doe", "123-456-7890");

    EXPECT_EQ("John Doe", user.getName());
}

TEST(UserTests, ConstructorStoresPhoneNumber)
{
    User user(UserId{1}, "John Doe", "123-456-7890");

    EXPECT_EQ("123-456-7890", user.getPhoneNumber());
}
