#include <gtest/gtest.h>
#include <string>

#include "Network.h"

class NetworkTest : public ::testing::Test
{
protected:
    Network mNetwork;
    User mFirstUser{1, "John Doe", "123-456-7890"};
    User mSecondUser{2, "Johnny Doesony", "123-456-7777"};
    User mThirdUser{3, "Johnini Doesini", "123-456-9999"};
    User mFourthUser{4, "Johniasty Doesiniasty", "999-999-9999"};

    void SetUp() override
    {
        ASSERT_TRUE(mNetwork.addUser(mFirstUser));
        ASSERT_TRUE(mNetwork.addUser(mSecondUser));
        ASSERT_TRUE(mNetwork.addUser(mThirdUser));
        ASSERT_TRUE(mNetwork.addUser(mFourthUser));
    }
};

struct InvalidCallCreationCase
{
    int callId;
    int callerId;
    int receiverId;
    std::string name;
};

class InvalidCallCreationTest : public NetworkTest, public ::testing::WithParamInterface<InvalidCallCreationCase>
{
};

TEST(NetworkStandaloneTests, NewlyConstructedNetworkHasNoUsers)
{
    Network emptyNetwork;

    EXPECT_EQ(0, emptyNetwork.getUserCount());
}

TEST_F(NetworkTest, AddingUniqueUserIncreasesUserCount)
{
    User newUser{5, "Jane Doe", "123-456-0000"};

    EXPECT_TRUE(mNetwork.addUser(newUser));
    EXPECT_EQ(5, mNetwork.getUserCount());
}

TEST_F(NetworkTest, AddingDuplicateUserIdIsRejectedAndKeepsUserCount)
{
    User userWithDuplicateId{
        mFirstUser.getId(),
        "Completely Different Name",
        "000-000-0000"};

    EXPECT_FALSE(mNetwork.addUser(userWithDuplicateId));
    EXPECT_EQ(4, mNetwork.getUserCount());
}

TEST_F(NetworkTest, RemovingExistingUserDecreasesUserCount)
{
    EXPECT_TRUE(mNetwork.removeUser(mFourthUser.getId()));
    EXPECT_EQ(3, mNetwork.getUserCount());
}

TEST_F(NetworkTest, RemovingNonexistentUserIsRejectedAndKeepsUserCount)
{
    EXPECT_FALSE(mNetwork.removeUser(999));
    EXPECT_EQ(4, mNetwork.getUserCount());
}

TEST_F(NetworkTest, ValidCallCreationIncreasesCallCount)
{
    EXPECT_EQ(0, mNetwork.getCallCount());
    ASSERT_TRUE(mNetwork.createCall(1, 1, 2));
    EXPECT_EQ(1, mNetwork.getCallCount());
}

TEST_P(InvalidCallCreationTest, InvalidCallIsRejected)
{
    const InvalidCallCreationCase &testCase = GetParam();
    EXPECT_FALSE(mNetwork.createCall(testCase.callId, testCase.callerId, testCase.receiverId));
    EXPECT_EQ(0, mNetwork.getCallCount());
}

INSTANTIATE_TEST_SUITE_P(
    InvalidCallCases,
    InvalidCallCreationTest,
    ::testing::Values(
        InvalidCallCreationCase{1, 99, 2, "MissingCaller"},
        InvalidCallCreationCase{2, 1, 99, "MissingReceiver"},
        InvalidCallCreationCase{3, 1, 1, "SameUser"}),
    [](const ::testing::TestParamInfo<InvalidCallCreationCase> &info)
    {
        return info.param.name;
    });

TEST_F(NetworkTest, DuplicateCallIdIsRejectedAndCallCountDoesNotChange)
{
    EXPECT_EQ(0, mNetwork.getCallCount());
    ASSERT_TRUE(mNetwork.createCall(10, 1, 2));
    EXPECT_EQ(1, mNetwork.getCallCount());
    EXPECT_FALSE(mNetwork.createCall(10, 3, 4));
    EXPECT_EQ(1, mNetwork.getCallCount());
}

TEST_F(NetworkTest, IndependentCallsCanWorkConcurrently)
{
    EXPECT_EQ(0, mNetwork.getCallCount());
    ASSERT_TRUE(mNetwork.createCall(10, 1, 2));
    ASSERT_TRUE(mNetwork.createCall(11, 3, 4));
    ASSERT_TRUE(mNetwork.startCall(10));
    ASSERT_TRUE(mNetwork.startCall(11));
    EXPECT_EQ(2, mNetwork.getCallCount());
}
TEST_F(NetworkTest, UserBecomesFreeAfterEndingCall)
{
    EXPECT_EQ(0, mNetwork.getCallCount());
    ASSERT_TRUE(mNetwork.createCall(10, 1, 2));
    ASSERT_TRUE(mNetwork.createCall(11, 2, 3));
    {
        SCOPED_TRACE("Start call 10");
        ASSERT_TRUE(mNetwork.startCall(10));
    }
    {
        SCOPED_TRACE("Call 11 is rejected");
        ASSERT_FALSE(mNetwork.startCall(11));
    }
    {
        SCOPED_TRACE("End call 10");
        ASSERT_TRUE(mNetwork.endCall(10));
    }
    {
        SCOPED_TRACE("Start call 11 after user 2 becomes free");
        EXPECT_TRUE(mNetwork.startCall(11));
    }
}

TEST_F(NetworkTest, CallRemainsBlockedUntilBothUsersAreFree)
{
    ASSERT_TRUE(mNetwork.createCall(10, 1, 2));
    ASSERT_TRUE(mNetwork.createCall(11, 2, 3));
    ASSERT_TRUE(mNetwork.createCall(12, 3, 4));
    ASSERT_TRUE(mNetwork.startCall(10));
    ASSERT_TRUE(mNetwork.startCall(12));
    ASSERT_TRUE(mNetwork.endCall(10));

    EXPECT_FALSE(mNetwork.startCall(11));

    ASSERT_TRUE(mNetwork.endCall(12));
    EXPECT_TRUE(mNetwork.startCall(11));
}

TEST_F(NetworkTest, NonexistentCallIdIsRejected)
{
    EXPECT_EQ(0, mNetwork.getCallCount());
    EXPECT_FALSE(mNetwork.startCall(999));
    EXPECT_FALSE(mNetwork.endCall(999));
    EXPECT_EQ(0, mNetwork.getCallCount());
}

TEST_F(NetworkTest, CallLifecycleRejectsInvalidStateTransitions)
{
    EXPECT_EQ(0, mNetwork.getCallCount());
    ASSERT_TRUE(mNetwork.createCall(10, 1, 2));
    ASSERT_TRUE(mNetwork.startCall(10));
    EXPECT_FALSE(mNetwork.startCall(10));
    ASSERT_TRUE(mNetwork.endCall(10));
    EXPECT_FALSE(mNetwork.endCall(10));
    EXPECT_FALSE(mNetwork.startCall(10));
    EXPECT_EQ(1, mNetwork.getCallCount());
}