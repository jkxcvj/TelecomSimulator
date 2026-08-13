#include <gtest/gtest.h>

#include "Identifiers.h"
#include "Registry.h"
#include "User.h"

TEST(RegistryTests, AddsAndFindsValues)
{
    Registry<int, std::string> registry;

    EXPECT_TRUE(registry.add(1, "Alice"));
    EXPECT_FALSE(registry.add(1, "Bob"));

    EXPECT_TRUE(registry.contains(1));
    EXPECT_FALSE(registry.contains(2));

    EXPECT_EQ(registry.size(), 1);

    auto *value = registry.find(1);

    ASSERT_NE(value, nullptr);
    EXPECT_EQ(*value, "Alice");

    EXPECT_EQ(registry.find(999), nullptr);
}

TEST(RegistryTests, WorksWithUserDomainTypes)
{
    Registry<UserId, User, UserIdHash> registry;

    const UserId aliceId{1};
    const UserId bobId{2};

    EXPECT_TRUE(registry.add(aliceId, User{aliceId, "Alice", "111"}));
    EXPECT_TRUE(registry.add(bobId, User{bobId, "Bob", "112"}));

    EXPECT_FALSE(registry.add(aliceId, User{aliceId, "Another Alice", "113"}));

    EXPECT_EQ(registry.size(), 2);

    EXPECT_TRUE(registry.contains(aliceId));
    EXPECT_TRUE(registry.contains(bobId));
    EXPECT_FALSE(registry.contains(UserId{99}));

    User *alice = registry.find(aliceId);

    ASSERT_NE(alice, nullptr);
    EXPECT_EQ(alice->getId(), aliceId);
    EXPECT_EQ(alice->getName(), "Alice");

    EXPECT_EQ(registry.find(UserId{99}), nullptr);
}