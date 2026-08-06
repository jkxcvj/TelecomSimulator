#include <gtest/gtest.h>
#include <memory>
#include <string>

#include "Network.h"
#include "TestEventLogger.h"

class NetworkTest : public ::testing::Test
{
  protected:
    Network mNetwork = makeNetwork();
    User mFirstUser{UserId{1}, "John Doe", "123-456-7890"};
    User mSecondUser{UserId{2}, "Johnny Doesony", "123-456-7777"};
    User mThirdUser{UserId{3}, "Johnini Doesini", "123-456-9999"};
    User mFourthUser{UserId{4}, "Johniasty Doesiniasty", "999-999-9999"};

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
    CallId callId;
    UserId callerId;
    UserId receiverId;
    std::string name;
};

class InvalidCallCreationTest : public NetworkTest, public ::testing::WithParamInterface<InvalidCallCreationCase>
{
};

TEST(NetworkStandaloneTests, NewlyConstructedNetworkHasNoUsers)
{
    Network emptyNetwork = makeNetwork();

    EXPECT_EQ(0, emptyNetwork.getUserCount());
}

TEST_F(NetworkTest, AddingUniqueUserIncreasesUserCount)
{
    User newUser{UserId{5}, "Jane Doe", "123-456-0000"};

    EXPECT_TRUE(mNetwork.addUser(newUser));
    EXPECT_EQ(5, mNetwork.getUserCount());
}

TEST_F(NetworkTest, AddingDuplicateUserIdIsRejectedAndKeepsUserCount)
{
    User userWithDuplicateId{mFirstUser.getId(), "Completely Different Name", "000-000-0000"};

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
    EXPECT_FALSE(mNetwork.removeUser(UserId{999}));
    EXPECT_EQ(4, mNetwork.getUserCount());
}

TEST_F(NetworkTest, ValidCallCreationIncreasesCallCount)
{
    EXPECT_EQ(0, mNetwork.getCallCount());
    ASSERT_TRUE(mNetwork.createCall(CallId{1}, UserId{1}, UserId{2}));
    EXPECT_EQ(1, mNetwork.getCallCount());
}

TEST_P(InvalidCallCreationTest, InvalidCallIsRejected)
{
    const InvalidCallCreationCase &testCase = GetParam();
    EXPECT_FALSE(mNetwork.createCall(testCase.callId, testCase.callerId, testCase.receiverId));
    EXPECT_EQ(0, mNetwork.getCallCount());
}

INSTANTIATE_TEST_SUITE_P(InvalidCallCases, InvalidCallCreationTest,
                         ::testing::Values(InvalidCallCreationCase{CallId{1}, UserId{99}, UserId{2}, "MissingCaller"},
                                           InvalidCallCreationCase{CallId{2}, UserId{1}, UserId{99}, "MissingReceiver"},
                                           InvalidCallCreationCase{CallId{3}, UserId{1}, UserId{1}, "SameUser"}),
                         [](const ::testing::TestParamInfo<InvalidCallCreationCase> &info) { return info.param.name; });

TEST_F(NetworkTest, DuplicateCallIdIsRejectedAndCallCountDoesNotChange)
{
    EXPECT_EQ(0, mNetwork.getCallCount());
    ASSERT_TRUE(mNetwork.createCall(CallId{10}, UserId{1}, UserId{2}));
    EXPECT_EQ(1, mNetwork.getCallCount());
    EXPECT_FALSE(mNetwork.createCall(CallId{10}, UserId{3}, UserId{4}));
    EXPECT_EQ(1, mNetwork.getCallCount());
}

TEST_F(NetworkTest, IndependentCallsCanWorkConcurrently)
{
    EXPECT_EQ(0, mNetwork.getCallCount());
    ASSERT_TRUE(mNetwork.createCall(CallId{10}, UserId{1}, UserId{2}));
    ASSERT_TRUE(mNetwork.createCall(CallId{11}, UserId{3}, UserId{4}));
    ASSERT_TRUE(mNetwork.startCall(CallId{10}));
    ASSERT_TRUE(mNetwork.startCall(CallId{11}));
    EXPECT_EQ(2, mNetwork.getCallCount());
}
TEST_F(NetworkTest, UserBecomesFreeAfterEndingCall)
{
    EXPECT_EQ(0, mNetwork.getCallCount());
    ASSERT_TRUE(mNetwork.createCall(CallId{10}, UserId{1}, UserId{2}));
    ASSERT_TRUE(mNetwork.createCall(CallId{11}, UserId{2}, UserId{3}));
    {
        SCOPED_TRACE("Start call 10");
        ASSERT_TRUE(mNetwork.startCall(CallId{10}));
    }
    {
        SCOPED_TRACE("Call 11 is rejected");
        ASSERT_FALSE(mNetwork.startCall(CallId{11}));
    }
    {
        SCOPED_TRACE("End call 10");
        ASSERT_TRUE(mNetwork.endCall(CallId{10}));
    }
    {
        SCOPED_TRACE("Start call 11 after user 2 becomes free");
        EXPECT_TRUE(mNetwork.startCall(CallId{11}));
    }
}

TEST_F(NetworkTest, CallRemainsBlockedUntilBothUsersAreFree)
{
    ASSERT_TRUE(mNetwork.createCall(CallId{10}, UserId{1}, UserId{2}));
    ASSERT_TRUE(mNetwork.createCall(CallId{11}, UserId{2}, UserId{3}));
    ASSERT_TRUE(mNetwork.createCall(CallId{12}, UserId{3}, UserId{4}));
    ASSERT_TRUE(mNetwork.startCall(CallId{10}));
    ASSERT_TRUE(mNetwork.startCall(CallId{12}));
    ASSERT_TRUE(mNetwork.endCall(CallId{10}));

    EXPECT_FALSE(mNetwork.startCall(CallId{11}));

    ASSERT_TRUE(mNetwork.endCall(CallId{12}));
    EXPECT_TRUE(mNetwork.startCall(CallId{11}));
}

TEST_F(NetworkTest, NonexistentCallIdIsRejected)
{
    EXPECT_EQ(0, mNetwork.getCallCount());
    EXPECT_FALSE(mNetwork.startCall(CallId{999}));
    EXPECT_FALSE(mNetwork.endCall(CallId{999}));
    EXPECT_EQ(0, mNetwork.getCallCount());
}

TEST_F(NetworkTest, CallLifecycleRejectsInvalidStateTransitions)
{
    EXPECT_EQ(0, mNetwork.getCallCount());
    ASSERT_TRUE(mNetwork.createCall(CallId{10}, UserId{1}, UserId{2}));
    ASSERT_TRUE(mNetwork.startCall(CallId{10}));
    EXPECT_FALSE(mNetwork.startCall(CallId{10}));
    ASSERT_TRUE(mNetwork.endCall(CallId{10}));
    EXPECT_FALSE(mNetwork.endCall(CallId{10}));
    EXPECT_FALSE(mNetwork.startCall(CallId{10}));
    EXPECT_EQ(1, mNetwork.getCallCount());
}
