#include <gtest/gtest.h>
#include <variant>

#include "CallStatistics.h"
#include "Network.h"
#include "PersistenceUtils.h"
#include "TestEventLogger.h"

TEST(PersistenceTests, CreatesDirectoryWhenItDoesNotExist)
{
    const std::filesystem::path path = "test_data/new_directory";

    std::filesystem::remove_all("test_data");

    EXPECT_TRUE(ensureDirectoryExists(path.parent_path()));
    EXPECT_TRUE(ensureDirectoryExists(path));
    EXPECT_TRUE(std::filesystem::exists(path));
    EXPECT_TRUE(std::filesystem::is_directory(path));

    std::filesystem::remove_all("test_data");
}

TEST(PersistenceTests, ReturnsTrueWhenDirectoryAlreadyExists)
{
    const std::filesystem::path path = "test_data";

    std::filesystem::remove_all(path);
    std::filesystem::create_directory(path);

    EXPECT_TRUE(ensureDirectoryExists(path));
    EXPECT_TRUE(std::filesystem::exists(path));

    std::filesystem::remove_all(path);
}

TEST(PersistenceTests, CreatesNestedDirectories)
{
    const std::filesystem::path path = "test_data/saves/2026";

    std::filesystem::remove_all("test_data");

    EXPECT_TRUE(ensureDirectoryExists(path));
    EXPECT_TRUE(std::filesystem::exists(path));
    EXPECT_TRUE(std::filesystem::is_directory(path));

    std::filesystem::remove_all("test_data");
}

TEST(PersistenceTests, SavesTextToFile)
{
    const std::filesystem::path dir = "test_data";
    const std::filesystem::path filePath = dir / "sample.txt";

    std::filesystem::remove_all(dir);
    std::filesystem::create_directories(dir);

    ASSERT_TRUE(saveText(filePath, "Telecom Simulator"));
    ASSERT_TRUE(std::filesystem::exists(filePath));

    std::ifstream file(filePath);
    std::string content;

    std::getline(file, content);

    EXPECT_EQ(content, "Telecom Simulator");

    std::filesystem::remove_all(dir);
}

TEST(PersistenceTests, SavesAndLoadsText)
{
    const std::filesystem::path dir = "test_data";
    const std::filesystem::path filePath = dir / "round_trip.txt";

    std::filesystem::remove_all(dir);
    std::filesystem::create_directories(dir);

    const std::string expected = "User|1|Alice\nCall|10|1|2|Active";

    ASSERT_TRUE(saveText(filePath, expected));

    const auto loaded = loadText(filePath);

    ASSERT_TRUE(loaded.has_value());
    EXPECT_EQ(loaded.value(), expected);

    std::filesystem::remove_all(dir);
}
TEST(PersistenceTests, ReturnsNulloptWhenFileDoesNotExist)
{
    const std::filesystem::path path = "test_data/missing.txt";

    std::filesystem::remove_all("test_data");

    const auto loaded = loadText(path);

    EXPECT_FALSE(loaded.has_value());
}

TEST(PersistenceTests, SerializesUser)
{
    User user(UserId{1}, "Alice", "123456789");

    const std::string serialized = serializeUser(user);

    EXPECT_EQ(serialized, "USER|1|Alice|123456789");
}

TEST(PersistenceTests, DeserializesUser)
{
    const auto user = deserializeUser("USER|1|Alice|123456789");

    ASSERT_TRUE(user.has_value());

    EXPECT_EQ(user->getId(), UserId{1});
    EXPECT_EQ(user->getName(), "Alice");
    EXPECT_EQ(user->getPhoneNumber(), "123456789");
}

TEST(PersistenceTests, RejectsUserWithWrongPrefix)
{
    const auto user = deserializeUser("CALL|1|Alice|123456789");

    EXPECT_FALSE(user.has_value());
}

TEST(PersistenceTests, RejectsUserWithInvalidId)
{
    const auto user = deserializeUser("USER|abc|Alice|123456789");

    EXPECT_FALSE(user.has_value());
}

TEST(PersistenceTests, RejectsUserWithMissingFields)
{
    const auto user = deserializeUser("USER|1|Alice");

    EXPECT_FALSE(user.has_value());
}

TEST(PersistenceTests, RejectsUserWithExtraFields)
{
    const auto user = deserializeUser("USER|1|Alice|123456789|EXTRA");

    EXPECT_FALSE(user.has_value());
}

TEST(PersistenceTests, ConvertsCallStatusToString)
{
    EXPECT_EQ(callStatusToString(CallStatus::Created), "Created");
    EXPECT_EQ(callStatusToString(CallStatus::Active), "Active");
    EXPECT_EQ(callStatusToString(CallStatus::Ended), "Ended");
}

TEST(PersistenceTests, ConvertsStringToCallStatus)
{
    EXPECT_EQ(callStatusFromString("Created"), CallStatus::Created);
    EXPECT_EQ(callStatusFromString("Active"), CallStatus::Active);
    EXPECT_EQ(callStatusFromString("Ended"), CallStatus::Ended);
}

TEST(PersistenceTests, RejectsInvalidCallStatus) { EXPECT_FALSE(callStatusFromString("Something").has_value()); }

TEST(PersistenceTests, SerializesCall)
{
    const CallParameters parameters{CallId{100}, UserId{1}, UserId{2}};

    const Call call(parameters);

    EXPECT_EQ(serializeCall(call), "CALL|100|1|2|Created");
}

TEST(PersistenceTests, SerializesActiveCall)
{
    const CallParameters parameters{CallId{100}, UserId{1}, UserId{2}};

    const Call call(parameters, CallStatus::Active);

    EXPECT_EQ(serializeCall(call), "CALL|100|1|2|Active");
}

TEST(PersistenceTests, DeserializesCall)
{
    const auto call = deserializeCall("CALL|100|1|2|Active");

    ASSERT_TRUE(call.has_value());

    EXPECT_EQ(call->getId(), CallId{100});
    EXPECT_EQ(call->getCallerId(), UserId{1});
    EXPECT_EQ(call->getReceiverId(), UserId{2});
    EXPECT_EQ(call->getStatusId(), CallStatus::Active);
}

TEST(PersistenceTests, RejectsCallWithInvalidStatus) { EXPECT_FALSE(deserializeCall("CALL|100|1|2|Flying").has_value()); }

TEST(PersistenceTests, RejectsCallWithInvalidId) { EXPECT_FALSE(deserializeCall("CALL|abc|1|2|Created").has_value()); }

TEST(PersistenceTests, RejectsCallWithExtraFields) { EXPECT_FALSE(deserializeCall("CALL|100|1|2|Created|EXTRA").has_value()); }

TEST(PersistenceTests, SavesNetworkToFile)
{
    const std::filesystem::path dir = "test_data";
    const std::filesystem::path filePath = dir / "network.txt";

    std::filesystem::remove_all(dir);
    std::filesystem::create_directories(dir);

    CallStatistics statistics;
    auto logger = std::make_unique<SilentEventLogger>();

    Network network(std::move(logger), statistics);

    ASSERT_TRUE(network.addUser(User(UserId{1}, "Alice", "123456789")));

    ASSERT_TRUE(network.addUser(User(UserId{2}, "Bob", "987654321")));

    ASSERT_TRUE(network.createCall(CallId{100}, UserId{1}, UserId{2}));

    ASSERT_TRUE(saveNetwork(network, filePath));

    const auto content = loadText(filePath);

    ASSERT_TRUE(content.has_value());

    EXPECT_NE(content->find("USER|1|Alice|123456789"), std::string::npos);

    EXPECT_NE(content->find("USER|2|Bob|987654321"), std::string::npos);

    EXPECT_NE(content->find("CALL|100|1|2|Created"), std::string::npos);

    std::filesystem::remove_all(dir);
}

TEST(PersistenceTests, LoadsNetworkFromFile)
{
    const std::filesystem::path dir = "test_data";
    const std::filesystem::path filePath = dir / "network.txt";

    std::filesystem::remove_all(dir);
    std::filesystem::create_directories(dir);

    ASSERT_TRUE(saveText(filePath, "USER|1|Alice|123456789\n"
                                   "USER|2|Bob|987654321\n"
                                   "CALL|100|1|2|Active\n"));

    CallStatistics statistics;
    auto logger = std::make_unique<SilentEventLogger>();

    Network network(std::move(logger), statistics);

    ASSERT_TRUE(loadNetwork(network, filePath));

    EXPECT_EQ(network.getUserCount(), 2);
    EXPECT_EQ(network.getCallCount(), 1);

    const Call *call = network.getCall(CallId{100});

    ASSERT_NE(call, nullptr);
    EXPECT_EQ(call->getStatusId(), CallStatus::Active);

    std::filesystem::remove_all(dir);
}

TEST(PersistenceTests, InvalidFileDoesNotPartiallyModifyNetwork)
{
    const std::filesystem::path dir = "test_data";
    const std::filesystem::path filePath = dir / "broken_network.txt";

    std::filesystem::remove_all(dir);
    std::filesystem::create_directories(dir);

    ASSERT_TRUE(saveText(filePath, "USER|1|Alice|123456789\n"
                                   "USER|2|Bob|987654321\n"
                                   "CALL|100|1|2|Active\n"
                                   "CALL|BROKEN|1|2|Ended\n"));

    CallStatistics statistics;
    auto logger = std::make_unique<SilentEventLogger>();
    Network network(std::move(logger), statistics);

    EXPECT_FALSE(loadNetwork(network, filePath));

    EXPECT_EQ(network.getUserCount(), 0);
    EXPECT_EQ(network.getCallCount(), 0);

    std::filesystem::remove_all(dir);
}

TEST(PersistenceTests, ExistingUserConflictDoesNotPartiallyModifyNetwork)
{
    const std::filesystem::path dir = "test_data";
    const std::filesystem::path filePath = dir / "conflict.txt";

    std::filesystem::remove_all(dir);
    std::filesystem::create_directories(dir);

    ASSERT_TRUE(saveText(filePath, "USER|1|Alice|123456789\n"
                                   "USER|2|Bob|987654321\n"));

    CallStatistics statistics;
    auto logger = std::make_unique<SilentEventLogger>();
    Network network(std::move(logger), statistics);

    ASSERT_TRUE(network.addUser(User(UserId{2}, "Existing", "111111111")));

    EXPECT_FALSE(loadNetwork(network, filePath));

    EXPECT_EQ(network.getUserCount(), 1);
    EXPECT_EQ(network.getUser(UserId{1}), nullptr);
    EXPECT_NE(network.getUser(UserId{2}), nullptr);

    std::filesystem::remove_all(dir);
}

TEST(PersistenceTests, DuplicateUserInFileDoesNotModifyNetwork)
{
    const std::filesystem::path dir = "test_data";
    const std::filesystem::path filePath = dir / "duplicates.txt";

    std::filesystem::remove_all(dir);
    std::filesystem::create_directories(dir);

    ASSERT_TRUE(saveText(filePath, "USER|1|Alice|123456789\n"
                                   "USER|1|Bob|987654321\n"));

    CallStatistics statistics;
    auto logger = std::make_unique<SilentEventLogger>();
    Network network(std::move(logger), statistics);

    EXPECT_FALSE(loadNetwork(network, filePath));

    EXPECT_EQ(network.getUserCount(), 0);

    std::filesystem::remove_all(dir);
}

TEST(PersistenceTests, RejectsCallReferencingMissingUser)
{
    const std::filesystem::path dir = "test_data";
    const std::filesystem::path filePath = dir / "missing_user.txt";

    std::filesystem::remove_all(dir);
    std::filesystem::create_directories(dir);

    ASSERT_TRUE(saveText(filePath, "USER|1|Alice|123456789\n"
                                   "CALL|100|1|999|Created\n"));

    CallStatistics statistics;
    auto logger = std::make_unique<SilentEventLogger>();
    Network network(std::move(logger), statistics);

    EXPECT_FALSE(loadNetwork(network, filePath));

    EXPECT_EQ(network.getUserCount(), 0);
    EXPECT_EQ(network.getCallCount(), 0);

    std::filesystem::remove_all(dir);
}

TEST(PersistenceTests, SavesAndLoadsWholeNetwork)
{
    const std::filesystem::path dir = "test_data";
    const std::filesystem::path filePath = dir / "network_round_trip.txt";

    std::filesystem::remove_all(dir);
    std::filesystem::create_directories(dir);

    CallStatistics statistics1;
    auto logger1 = std::make_unique<SilentEventLogger>();
    Network original(std::move(logger1), statistics1);

    ASSERT_TRUE(original.addUser(User(UserId{1}, "Alice", "123456789")));

    ASSERT_TRUE(original.addUser(User(UserId{2}, "Bob", "987654321")));

    ASSERT_TRUE(original.createCall(CallId{100}, UserId{1}, UserId{2}));

    const auto startResult = original.startCall(CallId{100});

    ASSERT_TRUE(std::holds_alternative<std::monostate>(startResult));

    ASSERT_TRUE(saveNetwork(original, filePath));

    CallStatistics statistics2;
    auto logger2 = std::make_unique<SilentEventLogger>();
    Network loaded(std::move(logger2), statistics2);

    ASSERT_TRUE(loadNetwork(loaded, filePath));

    EXPECT_EQ(loaded.getUserCount(), 2);
    EXPECT_EQ(loaded.getCallCount(), 1);

    const User *alice = loaded.getUser(UserId{1});
    const User *bob = loaded.getUser(UserId{2});
    const Call *call = loaded.getCall(CallId{100});

    ASSERT_NE(alice, nullptr);
    ASSERT_NE(bob, nullptr);
    ASSERT_NE(call, nullptr);

    EXPECT_EQ(alice->getName(), "Alice");
    EXPECT_EQ(bob->getName(), "Bob");

    EXPECT_EQ(call->getCallerId(), UserId{1});
    EXPECT_EQ(call->getReceiverId(), UserId{2});
    EXPECT_EQ(call->getStatusId(), CallStatus::Active);

    std::filesystem::remove_all(dir);
}
