#include <algorithm>
#include <gtest/gtest.h>
#include <memory>
#include <string>

#include "CallStatistics.h"
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
    CreateCallError expectedError;
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
    const auto result = mNetwork.createCall(CallId{1}, UserId{1}, UserId{2});

    EXPECT_TRUE(std::holds_alternative<std::monostate>(result));
    EXPECT_EQ(1, mNetwork.getCallCount());
}

TEST_P(InvalidCallCreationTest, InvalidCallIsRejected)
{
    const InvalidCallCreationCase &testCase = GetParam();
    const auto result = mNetwork.createCall(testCase.callId, testCase.callerId, testCase.receiverId);

    ASSERT_TRUE(std::holds_alternative<CreateCallError>(result));
    EXPECT_EQ(testCase.expectedError, std::get<CreateCallError>(result));
    EXPECT_EQ(0, mNetwork.getCallCount());
}

INSTANTIATE_TEST_SUITE_P(
    InvalidCallCases, InvalidCallCreationTest,
    ::testing::Values(InvalidCallCreationCase{CallId{1}, UserId{99}, UserId{2}, CreateCallError::CallerNotFound, "MissingCaller"},
                      InvalidCallCreationCase{CallId{2}, UserId{1}, UserId{99}, CreateCallError::ReceiverNotFound, "MissingReceiver"},
                      InvalidCallCreationCase{CallId{3}, UserId{1}, UserId{1}, CreateCallError::SameUser, "SameUser"}),
    [](const ::testing::TestParamInfo<InvalidCallCreationCase> &info) { return info.param.name; });

TEST_F(NetworkTest, DuplicateCallIdIsRejectedAndCallCountDoesNotChange)
{
    EXPECT_EQ(0, mNetwork.getCallCount());
    ASSERT_TRUE(std::holds_alternative<std::monostate>(mNetwork.createCall(CallId{10}, UserId{1}, UserId{2})));
    EXPECT_EQ(1, mNetwork.getCallCount());
    const auto result = mNetwork.createCall(CallId{10}, UserId{3}, UserId{4});

    ASSERT_TRUE(std::holds_alternative<CreateCallError>(result));
    EXPECT_EQ(CreateCallError::CallAlreadyExists, std::get<CreateCallError>(result));
    EXPECT_EQ(1, mNetwork.getCallCount());
}

TEST_F(NetworkTest, BusyCallerIsRejected)
{
    ASSERT_TRUE(std::holds_alternative<std::monostate>(mNetwork.createCall(CallId{10}, UserId{1}, UserId{2})));
    ASSERT_TRUE(std::holds_alternative<std::monostate>(mNetwork.startCall(CallId{10})));

    const auto result = mNetwork.createCall(CallId{11}, UserId{1}, UserId{3});

    ASSERT_TRUE(std::holds_alternative<CreateCallError>(result));
    EXPECT_EQ(CreateCallError::CallerBusy, std::get<CreateCallError>(result));
    EXPECT_EQ(1, mNetwork.getCallCount());
}

TEST_F(NetworkTest, IndependentCallsCanWorkConcurrently)
{
    EXPECT_EQ(0, mNetwork.getCallCount());
    ASSERT_TRUE(std::holds_alternative<std::monostate>(mNetwork.createCall(CallId{10}, UserId{1}, UserId{2})));
    ASSERT_TRUE(std::holds_alternative<std::monostate>(mNetwork.createCall(CallId{11}, UserId{3}, UserId{4})));
    ASSERT_TRUE(std::holds_alternative<std::monostate>(mNetwork.startCall(CallId{10})));
    ASSERT_TRUE(std::holds_alternative<std::monostate>(mNetwork.startCall(CallId{11})));
    EXPECT_EQ(2, mNetwork.getCallCount());
}
TEST_F(NetworkTest, UserBecomesFreeAfterEndingCall)
{
    EXPECT_EQ(0, mNetwork.getCallCount());
    ASSERT_TRUE(std::holds_alternative<std::monostate>(mNetwork.createCall(CallId{10}, UserId{1}, UserId{2})));
    ASSERT_TRUE(std::holds_alternative<std::monostate>(mNetwork.createCall(CallId{11}, UserId{2}, UserId{3})));
    {
        SCOPED_TRACE("Start call 10");
        ASSERT_TRUE(std::holds_alternative<std::monostate>(mNetwork.startCall(CallId{10})));
    }
    {
        SCOPED_TRACE("Call 11 is rejected");
        const auto result = mNetwork.startCall(CallId{11});
        ASSERT_FALSE(std::holds_alternative<std::monostate>(result));
    }
    {
        SCOPED_TRACE("End call 10");
        ASSERT_TRUE(mNetwork.endCall(CallId{10}));
    }
    {
        SCOPED_TRACE("Start call 11 after user 2 becomes free");
        EXPECT_TRUE(std::holds_alternative<std::monostate>(mNetwork.startCall(CallId{11})));
    }
}

TEST_F(NetworkTest, CallRemainsBlockedUntilBothUsersAreFree)
{
    ASSERT_TRUE(std::holds_alternative<std::monostate>(mNetwork.createCall(CallId{10}, UserId{1}, UserId{2})));
    ASSERT_TRUE(std::holds_alternative<std::monostate>(mNetwork.createCall(CallId{11}, UserId{2}, UserId{3})));
    ASSERT_TRUE(std::holds_alternative<std::monostate>(mNetwork.createCall(CallId{12}, UserId{3}, UserId{4})));
    ASSERT_TRUE(std::holds_alternative<std::monostate>(mNetwork.startCall(CallId{10})));
    ASSERT_TRUE(std::holds_alternative<std::monostate>(mNetwork.startCall(CallId{12})));
    ASSERT_TRUE(mNetwork.endCall(CallId{10}));

    const auto result2 = mNetwork.startCall(CallId{11});

    EXPECT_FALSE(std::holds_alternative<std::monostate>(result2));

    ASSERT_TRUE(mNetwork.endCall(CallId{12}));
    EXPECT_TRUE(std::holds_alternative<std::monostate>(mNetwork.startCall(CallId{11})));
}

TEST_F(NetworkTest, NonexistentCallIdIsRejected)
{
    EXPECT_EQ(0, mNetwork.getCallCount());
    const auto result = mNetwork.startCall(CallId{11});
    EXPECT_EQ(std::get<StartCallError>(result), StartCallError::CallNotFound);
    EXPECT_FALSE(mNetwork.endCall(CallId{999}));
    EXPECT_EQ(0, mNetwork.getCallCount());
}

TEST_F(NetworkTest, CallLifecycleRejectsInvalidStateTransitions)
{
    EXPECT_EQ(0, mNetwork.getCallCount());
    ASSERT_TRUE(std::holds_alternative<std::monostate>(mNetwork.createCall(CallId{10}, UserId{1}, UserId{2})));
    ASSERT_TRUE(std::holds_alternative<std::monostate>(mNetwork.startCall(CallId{10})));
    const auto result = mNetwork.startCall(CallId{10});
    EXPECT_EQ(std::get<StartCallError>(result), StartCallError::CallAlreadyStarted);
    ASSERT_TRUE(mNetwork.endCall(CallId{10}));
    EXPECT_FALSE(mNetwork.endCall(CallId{10}));
    const auto result2 = mNetwork.startCall(CallId{10});
    EXPECT_EQ(std::get<StartCallError>(result2), StartCallError::CallAlreadyEnded);
    EXPECT_EQ(1, mNetwork.getCallCount());
}

TEST(NetworkTests, PublishesEventWhenUserIsRegistered)
{
    auto subscriber = std::make_shared<RecordingEventSubscriber>();

    Network network;

    network.subscribe(subscriber);

    const User user{UserId{1}, "Jane Doe", "123-456-0000"};

    ASSERT_TRUE(network.addUser(user));

    const auto &messages = subscriber->getMessages();

    ASSERT_EQ(messages.size(), 1);
    EXPECT_EQ(messages[0], EventType::UserRegistered);
}

TEST(NetworkTests, PublishesEventWhenUserRegistrationIsRejected)
{
    auto subscriber = std::make_shared<RecordingEventSubscriber>();

    Network network;

    network.subscribe(subscriber);

    const User user{UserId{1}, "Jane Doe", "123-456-0000"};

    ASSERT_TRUE(network.addUser(user));
    ASSERT_FALSE(network.addUser(user));

    const auto &messages = subscriber->getMessages();

    ASSERT_EQ(messages.size(), 2);
    EXPECT_EQ(messages[0], EventType::UserRegistered);
    EXPECT_EQ(messages[1], EventType::UserRegistrationRejected);
}

TEST(NetworkTests, PublishesEventWhenCallIsCreated)
{
    auto subscriber = std::make_shared<RecordingEventSubscriber>();
    Network network;
    network.subscribe(subscriber);

    ASSERT_TRUE(network.addUser(User{UserId{1}, "Caller", "111-111-1111"}));
    ASSERT_TRUE(network.addUser(User{UserId{2}, "Receiver", "222-222-2222"}));
    const auto messageCountBeforeOperation = subscriber->getMessages().size();

    ASSERT_TRUE(std::holds_alternative<std::monostate>(network.createCall(CallId{1}, UserId{1}, UserId{2})));

    const auto messages = subscriber->getMessages();
    ASSERT_EQ(messages.size(), messageCountBeforeOperation + 1);
    EXPECT_EQ(messages.back(), EventType::CallCreated);
}

TEST(NetworkTests, PublishesEventWhenCallIsStarted)
{
    auto subscriber = std::make_shared<RecordingEventSubscriber>();
    Network network;
    network.subscribe(subscriber);

    ASSERT_TRUE(network.addUser(User{UserId{1}, "Caller", "111-111-1111"}));
    ASSERT_TRUE(network.addUser(User{UserId{2}, "Receiver", "222-222-2222"}));
    ASSERT_TRUE(std::holds_alternative<std::monostate>(network.createCall(CallId{1}, UserId{1}, UserId{2})));
    const auto messageCountBeforeOperation = subscriber->getMessages().size();

    ASSERT_TRUE(std::holds_alternative<std::monostate>(network.startCall(CallId{1})));

    const auto messages = subscriber->getMessages();
    ASSERT_EQ(messages.size(), messageCountBeforeOperation + 1);
    EXPECT_EQ(messages.back(), EventType::CallStarted);
}

TEST(NetworkTests, PublishesEventWhenCallIsEnded)
{
    auto subscriber = std::make_shared<RecordingEventSubscriber>();
    Network network;
    network.subscribe(subscriber);

    ASSERT_TRUE(network.addUser(User{UserId{1}, "Caller", "111-111-1111"}));
    ASSERT_TRUE(network.addUser(User{UserId{2}, "Receiver", "222-222-2222"}));
    ASSERT_TRUE(std::holds_alternative<std::monostate>(network.createCall(CallId{1}, UserId{1}, UserId{2})));
    ASSERT_TRUE(std::holds_alternative<std::monostate>(network.startCall(CallId{1})));
    const auto messageCountBeforeOperation = subscriber->getMessages().size();

    ASSERT_TRUE(network.endCall(CallId{1}));

    const auto messages = subscriber->getMessages();
    ASSERT_EQ(messages.size(), messageCountBeforeOperation + 1);
    EXPECT_EQ(messages.back(), EventType::CallEnded);
}

TEST(NetworkTests, PublishesEventWhenCallCreationIsRejected)
{
    auto subscriber = std::make_shared<RecordingEventSubscriber>();
    Network network;
    network.subscribe(subscriber);

    ASSERT_TRUE(network.addUser(User{UserId{1}, "Caller", "111-111-1111"}));
    const auto messageCountBeforeOperation = subscriber->getMessages().size();

    const auto result = network.createCall(CallId{1}, UserId{1}, UserId{999});
    ASSERT_TRUE(std::holds_alternative<CreateCallError>(result));
    EXPECT_EQ(CreateCallError::ReceiverNotFound, std::get<CreateCallError>(result));

    const auto messages = subscriber->getMessages();
    ASSERT_EQ(messages.size(), messageCountBeforeOperation + 1);
    EXPECT_EQ(messages.back(), EventType::CallCreationRejected);
}

TEST(NetworkTests, CallStatisticsCountsPublishedCallEvents)
{
    Network network;
    auto statistics = std::make_shared<CallStatistics>();
    network.subscribe(statistics);

    ASSERT_TRUE(network.addUser(User{UserId{1}, "Caller", "111-111-1111"}));
    ASSERT_TRUE(network.addUser(User{UserId{2}, "Receiver", "222-222-2222"}));
    ASSERT_TRUE(std::holds_alternative<std::monostate>(network.createCall(CallId{1}, UserId{1}, UserId{2})));
    ASSERT_TRUE(std::holds_alternative<std::monostate>(network.startCall(CallId{1})));
    ASSERT_TRUE(network.endCall(CallId{1}));

    EXPECT_EQ(statistics->createdCount(), 1);
    EXPECT_EQ(statistics->startedCount(), 1);
    EXPECT_EQ(statistics->endedCount(), 1);
}

TEST(StartCallErrorTests, ConvertsErrorToString)
{
    EXPECT_EQ(toString(StartCallError::CallNotFound), "Call not found");

    EXPECT_EQ(toString(StartCallError::UserBusy), "User busy");
}

TEST_F(NetworkTest, AmountOfCalls)
{
    ASSERT_TRUE(std::holds_alternative<std::monostate>(mNetwork.createCall(CallId{10}, UserId{1}, UserId{2})));
    mNetwork.startCall(CallId{10});
    EXPECT_EQ(mNetwork.getActiveCallCount(), 1);
}

TEST_F(NetworkTest, RemovesOnlyEndedCalls)
{
    ASSERT_TRUE(std::holds_alternative<std::monostate>(mNetwork.createCall(CallId{10}, UserId{1}, UserId{2})));
    ASSERT_TRUE(std::holds_alternative<std::monostate>(mNetwork.createCall(CallId{11}, UserId{1}, UserId{2})));
    ASSERT_TRUE(std::holds_alternative<std::monostate>(mNetwork.createCall(CallId{12}, UserId{1}, UserId{2})));

    ASSERT_TRUE(std::holds_alternative<std::monostate>(mNetwork.startCall(CallId{10})));

    ASSERT_TRUE(mNetwork.endCall(CallId{10}));

    ASSERT_TRUE(std::holds_alternative<std::monostate>(mNetwork.startCall(CallId{11})));

    EXPECT_EQ(mNetwork.removeEndedCalls(), 1);
    EXPECT_EQ(mNetwork.getCallCount(), 2);

    const auto callIds = mNetwork.getCallIds();

    EXPECT_EQ(std::find(callIds.begin(), callIds.end(), CallId{10}), callIds.end());

    EXPECT_NE(std::find(callIds.begin(), callIds.end(), CallId{11}), callIds.end());

    EXPECT_NE(std::find(callIds.begin(), callIds.end(), CallId{12}), callIds.end());
}

TEST_F(NetworkTest, ReturnsUserIdsSortedByName)
{
    ASSERT_TRUE(mNetwork.addUser(User{UserId{100}, "Charlie", "+48 500 100 100"}));

    ASSERT_TRUE(mNetwork.addUser(User{UserId{101}, "Alice", "+48 500 200 200"}));

    ASSERT_TRUE(mNetwork.addUser(User{UserId{102}, "Bob", "+48 500 300 300"}));

    const auto userIds = mNetwork.getUserIdsSortedByName();

    const auto aliceIt = std::find(userIds.begin(), userIds.end(), UserId{101});

    const auto bobIt = std::find(userIds.begin(), userIds.end(), UserId{102});

    const auto charlieIt = std::find(userIds.begin(), userIds.end(), UserId{100});

    ASSERT_NE(aliceIt, userIds.end());
    ASSERT_NE(bobIt, userIds.end());
    ASSERT_NE(charlieIt, userIds.end());

    EXPECT_LT(aliceIt, bobIt);
    EXPECT_LT(bobIt, charlieIt);
}

TEST_F(NetworkTest, ReturnsUsersWithoutActiveCallsSortedById)
{
    ASSERT_TRUE(mNetwork.addUser(User{UserId{100}, "Alice", "+48 500 100 100"}));
    ASSERT_TRUE(mNetwork.addUser(User{UserId{101}, "Bob", "+48 500 200 200"}));
    ASSERT_TRUE(mNetwork.addUser(User{UserId{102}, "Charlie", "+48 500 300 300"}));
    ASSERT_TRUE(mNetwork.addUser(User{UserId{103}, "David", "+48 500 400 400"}));

    ASSERT_TRUE(std::holds_alternative<std::monostate>(mNetwork.createCall(CallId{1000}, UserId{100}, UserId{101})));
    ASSERT_TRUE(std::holds_alternative<std::monostate>(mNetwork.startCall(CallId{1000})));
    ASSERT_TRUE(std::holds_alternative<std::monostate>(mNetwork.createCall(CallId{1001}, UserId{102}, UserId{103})));
    ASSERT_TRUE(std::holds_alternative<std::monostate>(mNetwork.startCall(CallId{1001})));
    ASSERT_TRUE(mNetwork.endCall(CallId{1001}));

    const auto result = mNetwork.getUsersWithoutActiveCalls();

    EXPECT_EQ(std::find(result.begin(), result.end(), UserId{100}), result.end());
    EXPECT_EQ(std::find(result.begin(), result.end(), UserId{101}), result.end());
    EXPECT_NE(std::find(result.begin(), result.end(), UserId{102}), result.end());
    EXPECT_NE(std::find(result.begin(), result.end(), UserId{103}), result.end());
    EXPECT_TRUE(std::is_sorted(result.begin(), result.end()));
}

TEST(NetworkTests, GetCallResultReturnsCall)
{
    Network network;

    ASSERT_TRUE(network.addUser(User(UserId{1}, "Alice", "111")));
    ASSERT_TRUE(network.addUser(User(UserId{2}, "Bob", "222")));

    const auto createResult = network.createCall(CallId{100}, UserId{1}, UserId{2});

    ASSERT_TRUE(std::holds_alternative<std::monostate>(createResult));

    const auto result = network.getCallResult(CallId{100});

    ASSERT_TRUE(std::holds_alternative<std::reference_wrapper<const Call>>(result));

    const Call &call = std::get<std::reference_wrapper<const Call>>(result).get();

    EXPECT_EQ(call.getId(), CallId{100});
}

TEST(NetworkTests, GetCallResultReturnsNotFound)
{
    Network network;

    const auto result = network.getCallResult(CallId{999});

    ASSERT_TRUE(std::holds_alternative<GetCallError>(result));

    EXPECT_EQ(std::get<GetCallError>(result), GetCallError::NotFound);
}
