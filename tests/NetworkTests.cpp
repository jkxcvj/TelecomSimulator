#include <gtest/gtest.h>
#include <memory>
#include <string>

#include "Network.h"
#include "TestEventLogger.h"
#include "TestEventSubscriber.h"

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

TEST(NetworkTests, PublishesEventWhenUserIsRegistered)
{
    auto subscriber = std::make_shared<RecordingEventSubscriber>();

    Network network(std::make_unique<SilentEventLogger>());

    network.subscribe(subscriber);

    const User user{UserId{1}, "Jane Doe", "123-456-0000"};

    ASSERT_TRUE(network.addUser(user));

    const auto &messages = subscriber->getMessages();

    ASSERT_EQ(messages.size(), 1);
    EXPECT_EQ(messages[0], "User registered");
}

TEST(NetworkTests, PublishesEventWhenUserRegistrationIsRejected)
{
    auto subscriber = std::make_shared<RecordingEventSubscriber>();

    Network network(std::make_unique<SilentEventLogger>());

    network.subscribe(subscriber);

    const User user{UserId{1}, "Jane Doe", "123-456-0000"};

    ASSERT_TRUE(network.addUser(user));
    ASSERT_FALSE(network.addUser(user));

    const auto &messages = subscriber->getMessages();

    ASSERT_EQ(messages.size(), 2);
    EXPECT_EQ(messages[0], "User registered");
    EXPECT_EQ(messages[1], "User registration rejected");
}

TEST(NetworkTests, PublishesEventWhenCallIsCreated)
{
    auto subscriber = std::make_shared<RecordingEventSubscriber>();
    Network network(std::make_unique<SilentEventLogger>());
    network.subscribe(subscriber);

    ASSERT_TRUE(network.addUser(User{UserId{1}, "Caller", "111-111-1111"}));
    ASSERT_TRUE(network.addUser(User{UserId{2}, "Receiver", "222-222-2222"}));
    const auto messageCountBeforeOperation = subscriber->getMessages().size();

    ASSERT_TRUE(network.createCall(CallId{1}, UserId{1}, UserId{2}));

    const auto messages = subscriber->getMessages();
    ASSERT_EQ(messages.size(), messageCountBeforeOperation + 1);
    EXPECT_EQ(messages.back(), "Call created");
}

TEST(NetworkTests, PublishesEventWhenCallIsStarted)
{
    auto subscriber = std::make_shared<RecordingEventSubscriber>();
    Network network(std::make_unique<SilentEventLogger>());
    network.subscribe(subscriber);

    ASSERT_TRUE(network.addUser(User{UserId{1}, "Caller", "111-111-1111"}));
    ASSERT_TRUE(network.addUser(User{UserId{2}, "Receiver", "222-222-2222"}));
    ASSERT_TRUE(network.createCall(CallId{1}, UserId{1}, UserId{2}));
    const auto messageCountBeforeOperation = subscriber->getMessages().size();

    ASSERT_TRUE(network.startCall(CallId{1}));

    const auto messages = subscriber->getMessages();
    ASSERT_EQ(messages.size(), messageCountBeforeOperation + 1);
    EXPECT_EQ(messages.back(), "Call started");
}

TEST(NetworkTests, PublishesEventWhenCallIsEnded)
{
    auto subscriber = std::make_shared<RecordingEventSubscriber>();
    Network network(std::make_unique<SilentEventLogger>());
    network.subscribe(subscriber);

    ASSERT_TRUE(network.addUser(User{UserId{1}, "Caller", "111-111-1111"}));
    ASSERT_TRUE(network.addUser(User{UserId{2}, "Receiver", "222-222-2222"}));
    ASSERT_TRUE(network.createCall(CallId{1}, UserId{1}, UserId{2}));
    ASSERT_TRUE(network.startCall(CallId{1}));
    const auto messageCountBeforeOperation = subscriber->getMessages().size();

    ASSERT_TRUE(network.endCall(CallId{1}));

    const auto messages = subscriber->getMessages();
    ASSERT_EQ(messages.size(), messageCountBeforeOperation + 1);
    EXPECT_EQ(messages.back(), "Call ended");
}

TEST(NetworkTests, PublishesEventWhenCallCreationIsRejected)
{
    auto subscriber = std::make_shared<RecordingEventSubscriber>();
    Network network(std::make_unique<SilentEventLogger>());
    network.subscribe(subscriber);

    ASSERT_TRUE(network.addUser(User{UserId{1}, "Caller", "111-111-1111"}));
    const auto messageCountBeforeOperation = subscriber->getMessages().size();

    ASSERT_FALSE(network.createCall(CallId{1}, UserId{1}, UserId{999}));

    const auto messages = subscriber->getMessages();
    ASSERT_EQ(messages.size(), messageCountBeforeOperation + 1);
    EXPECT_EQ(messages.back(), "Call creation rejected");
}
