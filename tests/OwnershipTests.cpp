#include <gtest/gtest.h>
#include <type_traits>

#include "Network.h"
#include "TestEventLogger.h"

static_assert(!std::is_copy_constructible_v<Network>);
static_assert(!std::is_copy_assignable_v<Network>);

static_assert(std::is_move_constructible_v<Network>);
static_assert(std::is_move_assignable_v<Network>);

TEST(OwnershipTests, NetworkTakesOwnershipOfLogger)
{
    auto logger = std::make_unique<RecordingEventLogger>();
    RecordingEventLogger *loggerObserver = logger.get();
    Network network(std::move(logger));
    EXPECT_EQ(logger, nullptr);
    User newUser{UserId{1}, "Jane Doe", "123-456-0000"};
    EXPECT_TRUE(network.addUser(newUser));
    EXPECT_FALSE(network.addUser(newUser));
    const auto &messages = loggerObserver->getMessages();
    ASSERT_EQ(messages.size(), 2);

    EXPECT_EQ(messages[0], "User registered");
    EXPECT_EQ(messages[1], "User registration rejected");
}

TEST(OwnershipTests, RejectsNullLogger) { EXPECT_THROW(Network network(std::unique_ptr<EventLogger>{}), std::invalid_argument); }