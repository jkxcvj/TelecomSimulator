#include <gtest/gtest.h>
#include <type_traits>

#include "Network.h"
#include "TestEventLogger.h"

static_assert(!std::is_copy_constructible_v<Network>);
static_assert(!std::is_copy_assignable_v<Network>);

static_assert(std::is_move_constructible_v<Network>);
static_assert(std::is_move_assignable_v<Network>);

TEST(OwnershipTests, SubscribedLoggerReceivesEvents)
{
    auto logger = std::make_shared<RecordingEventLogger>();
    Network network;
    network.subscribe(logger);
    User newUser{UserId{1}, "Jane Doe", "123-456-0000"};
    EXPECT_TRUE(network.addUser(newUser));
    EXPECT_FALSE(network.addUser(newUser));
    const auto &messages = logger->getMessages();
    ASSERT_EQ(messages.size(), 2);

    EXPECT_EQ(messages[0], toString(EventType::UserRegistered));
    EXPECT_EQ(messages[1], toString(EventType::UserRegistrationRejected));
}
