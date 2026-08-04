#include <gtest/gtest.h>
#include <type_traits>

#include "Identifiers.h"

static_assert(!std::is_same_v<UserId, CallId>);
static_assert(!std::is_convertible_v<UserId, CallId>);
static_assert(!std::is_convertible_v<CallId, UserId>);

static_assert(!std::is_convertible_v<int, UserId>);
static_assert(!std::is_convertible_v<int, CallId>);

TEST(IdentifierTests, EqualUserIdsCompareEqual)
{
    const UserId first{1};
    const UserId second{1};

    EXPECT_EQ(first, second);
}

TEST(IdentifierTests, DifferentUserIdsCompareNotEqual)
{
    const UserId first{1};
    const UserId second{2};

    EXPECT_NE(first, second);
}

TEST(IdentifierTests, UserIdsCanBeOrdered)
{
    const UserId smaller{1};
    const UserId larger{2};

    EXPECT_LT(smaller, larger);
}

TEST(IdentifierTests, EqualUserIdsProduceEqualHashes)
{
    const UserId first{1};
    const UserId second{1};

    const UserIdHash hasher;

    EXPECT_EQ(hasher(first), hasher(second));
}

TEST(IdentifierTests, EqualCallIdsCompareEqual)
{
    const CallId first{1};
    const CallId second{1};

    EXPECT_EQ(first, second);
}

TEST(IdentifierTests, DifferentCallIdsCompareNotEqual)
{
    const CallId first{1};
    const CallId second{2};

    EXPECT_NE(first, second);
}

TEST(IdentifierTests, CallIdsCanBeOrdered)
{
    const CallId smaller{1};
    const CallId larger{2};

    EXPECT_LT(smaller, larger);
}

TEST(IdentifierTests, EqualCallIdsProduceEqualHashes)
{
    const CallId first{1};
    const CallId second{1};

    const CallIdHash hasher;

    EXPECT_EQ(hasher(first), hasher(second));
}