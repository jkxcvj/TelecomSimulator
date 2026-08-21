#include <gtest/gtest.h>
#include <variant>

#include "CallStatistics.h"
#include "FileUtils.h"
#include "Network.h"
#include "NetworkPersistence.h"
#include "Serialization.h"
#include "TestEventLogger.h"

TEST(PersistenceErrorTests, ConvertsDeserializeErrorsToStrings)
{
    EXPECT_EQ(deserializeUserErrorToString(DeserializeUserError::InvalidFieldCount), "Invalid user field count");
    EXPECT_EQ(deserializeUserErrorToString(DeserializeUserError::InvalidPrefix), "Invalid user prefix");
    EXPECT_EQ(deserializeUserErrorToString(DeserializeUserError::InvalidId), "Invalid user ID");
    EXPECT_EQ(deserializeCallErrorToString(DeserializeCallError::InvalidFieldCount), "Invalid call field count");
    EXPECT_EQ(deserializeCallErrorToString(DeserializeCallError::InvalidPrefix), "Invalid call prefix");
    EXPECT_EQ(deserializeCallErrorToString(DeserializeCallError::InvalidId), "Invalid call ID");
    EXPECT_EQ(deserializeCallErrorToString(DeserializeCallError::InvalidStatus), "Invalid call status");
}

TEST(PersistenceErrorTests, ConvertsLoadNetworkErrorsToStrings)
{
    EXPECT_EQ(loadNetworkErrorToString(LoadNetworkError::FileOpenFailed), "Failed to open network file");
    EXPECT_EQ(loadNetworkErrorToString(LoadNetworkError::InvalidRecordType), "Invalid network record type");
    EXPECT_EQ(loadNetworkErrorToString(LoadNetworkError::InvalidUser), "Invalid user record");
    EXPECT_EQ(loadNetworkErrorToString(LoadNetworkError::InvalidCall), "Invalid call record");
    EXPECT_EQ(loadNetworkErrorToString(LoadNetworkError::DuplicateUser), "Duplicate user");
    EXPECT_EQ(loadNetworkErrorToString(LoadNetworkError::DuplicateCall), "Duplicate call");
    EXPECT_EQ(loadNetworkErrorToString(LoadNetworkError::MissingReferencedUser), "Call references a missing user");
    EXPECT_EQ(loadNetworkErrorToString(LoadNetworkError::FileReadError), "Failed to read network file");
    EXPECT_EQ(loadNetworkErrorToString(LoadNetworkError::AddUsersError), "Failed to add users to the network");
    EXPECT_EQ(loadNetworkErrorToString(LoadNetworkError::AddCallsError), "Failed to add calls to the network");
}

TEST(PersistenceErrorTests, FormatsLoadNetworkFailureWithLineAndNestedError)
{
    const LoadNetworkFailure failure{LoadNetworkError::InvalidCall, 7, std::nullopt, DeserializeCallError::InvalidStatus};

    EXPECT_EQ(loadNetworkFailureToString(failure), "Invalid call record at line 7: Invalid call status");
}

TEST(PersistenceErrorTests, OmitsUnavailableLoadNetworkFailureDetails)
{
    const LoadNetworkFailure failure{LoadNetworkError::FileOpenFailed, 0, std::nullopt, std::nullopt};

    EXPECT_EQ(loadNetworkFailureToString(failure), "Failed to open network file");
}

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

    if (!loaded.has_value())
    {
        return;
    }

    EXPECT_EQ(*loaded, expected);

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
    const auto result = deserializeUser("USER|1|Alice|123456789");

    ASSERT_TRUE(std::holds_alternative<User>(result));
    const auto &user = std::get<User>(result);

    EXPECT_EQ(user.getId(), UserId{1});
    EXPECT_EQ(user.getName(), "Alice");
    EXPECT_EQ(user.getPhoneNumber(), "123456789");
}

TEST(PersistenceTests, RejectsUserWithWrongPrefix)
{
    const auto result = deserializeUser("CALL|1|Alice|123456789");

    ASSERT_TRUE(std::holds_alternative<DeserializeUserError>(result));
    EXPECT_EQ(DeserializeUserError::InvalidPrefix, std::get<DeserializeUserError>(result));
}

TEST(PersistenceTests, RejectsUserWithInvalidId)
{
    const auto result = deserializeUser("USER|abc|Alice|123456789");

    ASSERT_TRUE(std::holds_alternative<DeserializeUserError>(result));
    EXPECT_EQ(DeserializeUserError::InvalidId, std::get<DeserializeUserError>(result));
}

TEST(PersistenceTests, RejectsUserWithMissingFields)
{
    const auto result = deserializeUser("USER|1|Alice");

    ASSERT_TRUE(std::holds_alternative<DeserializeUserError>(result));
    EXPECT_EQ(DeserializeUserError::InvalidFieldCount, std::get<DeserializeUserError>(result));
}

TEST(PersistenceTests, RejectsUserWithExtraFields)
{
    const auto result = deserializeUser("USER|1|Alice|123456789|EXTRA");

    ASSERT_TRUE(std::holds_alternative<DeserializeUserError>(result));
    EXPECT_EQ(DeserializeUserError::InvalidFieldCount, std::get<DeserializeUserError>(result));
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
    const auto result = deserializeCall("CALL|100|1|2|Active");

    ASSERT_TRUE(std::holds_alternative<Call>(result));
    const auto &call = std::get<Call>(result);

    EXPECT_EQ(call.getId(), CallId{100});
    EXPECT_EQ(call.getCallerId(), UserId{1});
    EXPECT_EQ(call.getReceiverId(), UserId{2});
    EXPECT_EQ(call.getStatusId(), CallStatus::Active);
}

TEST(PersistenceTests, RejectsCallWithInvalidStatus)
{
    const auto result = deserializeCall("CALL|100|1|2|Flying");

    ASSERT_TRUE(std::holds_alternative<DeserializeCallError>(result));
    EXPECT_EQ(DeserializeCallError::InvalidStatus, std::get<DeserializeCallError>(result));
}

TEST(PersistenceTests, RejectsCallWithInvalidId)
{
    const auto result = deserializeCall("CALL|abc|1|2|Created");

    ASSERT_TRUE(std::holds_alternative<DeserializeCallError>(result));
    EXPECT_EQ(DeserializeCallError::InvalidId, std::get<DeserializeCallError>(result));
}

TEST(PersistenceTests, RejectsCallWithExtraFields)
{
    const auto result = deserializeCall("CALL|100|1|2|Created|EXTRA");

    ASSERT_TRUE(std::holds_alternative<DeserializeCallError>(result));
    EXPECT_EQ(DeserializeCallError::InvalidFieldCount, std::get<DeserializeCallError>(result));
}

TEST(PersistenceTests, RejectsCallWithMissingFields)
{
    const auto result = deserializeCall("CALL|100|1|2");

    ASSERT_TRUE(std::holds_alternative<DeserializeCallError>(result));
    EXPECT_EQ(DeserializeCallError::InvalidFieldCount, std::get<DeserializeCallError>(result));
}

TEST(PersistenceTests, RejectsCallWithInvalidPrefix)
{
    const auto result = deserializeCall("USER|100|1|2|Created");

    ASSERT_TRUE(std::holds_alternative<DeserializeCallError>(result));
    EXPECT_EQ(DeserializeCallError::InvalidPrefix, std::get<DeserializeCallError>(result));
}

TEST(PersistenceTests, SavesNetworkToFile)
{
    const std::filesystem::path dir = "test_data";
    const std::filesystem::path filePath = dir / "network.txt";

    std::filesystem::remove_all(dir);
    std::filesystem::create_directories(dir);

    Network network;

    ASSERT_TRUE(network.addUser(User(UserId{1}, "Alice", "123456789")));

    ASSERT_TRUE(network.addUser(User(UserId{2}, "Bob", "987654321")));

    ASSERT_TRUE(std::holds_alternative<std::monostate>(network.createCall(CallId{100}, UserId{1}, UserId{2})));

    ASSERT_TRUE(saveNetwork(network, filePath));

    const auto content = loadText(filePath);

    ASSERT_TRUE(content.has_value());

    if (!content.has_value())
    {
        return;
    }

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

    Network network;

    const auto loadResult = loadNetwork(network, filePath);
    ASSERT_TRUE(std::holds_alternative<std::monostate>(loadResult));

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
    Network network;

    const auto loadResult = loadNetwork(network, filePath);
    ASSERT_TRUE(std::holds_alternative<LoadNetworkFailure>(loadResult));
    const auto &failure = std::get<LoadNetworkFailure>(loadResult);
    EXPECT_EQ(LoadNetworkError::InvalidCall, failure.error);
    EXPECT_EQ(4, failure.lineNumber);
    EXPECT_EQ(failure.callError, std::optional{DeserializeCallError::InvalidId});

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
    Network network;

    ASSERT_TRUE(network.addUser(User(UserId{2}, "Existing", "111111111")));

    const auto loadResult = loadNetwork(network, filePath);
    ASSERT_TRUE(std::holds_alternative<LoadNetworkFailure>(loadResult));
    EXPECT_EQ(LoadNetworkError::DuplicateUser, std::get<LoadNetworkFailure>(loadResult).error);

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
    Network network;

    const auto loadResult = loadNetwork(network, filePath);
    ASSERT_TRUE(std::holds_alternative<LoadNetworkFailure>(loadResult));
    EXPECT_EQ(LoadNetworkError::DuplicateUser, std::get<LoadNetworkFailure>(loadResult).error);

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
    Network network;

    const auto loadResult = loadNetwork(network, filePath);
    ASSERT_TRUE(std::holds_alternative<LoadNetworkFailure>(loadResult));
    EXPECT_EQ(LoadNetworkError::MissingReferencedUser, std::get<LoadNetworkFailure>(loadResult).error);

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
    Network original;

    ASSERT_TRUE(original.addUser(User(UserId{1}, "Alice", "123456789")));

    ASSERT_TRUE(original.addUser(User(UserId{2}, "Bob", "987654321")));

    ASSERT_TRUE(std::holds_alternative<std::monostate>(original.createCall(CallId{100}, UserId{1}, UserId{2})));

    const auto startResult = original.startCall(CallId{100});

    ASSERT_TRUE(std::holds_alternative<std::monostate>(startResult));

    ASSERT_TRUE(saveNetwork(original, filePath));
    Network loaded;

    const auto loadResult = loadNetwork(loaded, filePath);
    ASSERT_TRUE(std::holds_alternative<std::monostate>(loadResult));

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

TEST(PersistenceTests, DeserializeUserReturnsUser)
{
    const auto result = deserializeUser("USER|1|Alice|123456789");

    ASSERT_TRUE(std::holds_alternative<User>(result));

    const auto &user = std::get<User>(result);

    EXPECT_EQ(user.getId(), UserId{1});
    EXPECT_EQ(user.getName(), "Alice");
    EXPECT_EQ(user.getPhoneNumber(), "123456789");
}

TEST(PersistenceTests, DeserializeUserReturnsInvalidPrefix)
{
    const auto result = deserializeUser("CALL|1|Alice|123456789");

    ASSERT_TRUE(std::holds_alternative<DeserializeUserError>(result));

    EXPECT_EQ(std::get<DeserializeUserError>(result), DeserializeUserError::InvalidPrefix);
}

TEST(PersistenceTests, DeserializeUserReturnsInvalidId)
{
    const auto result = deserializeUser("USER|abc|Alice|123456789");

    ASSERT_TRUE(std::holds_alternative<DeserializeUserError>(result));

    EXPECT_EQ(std::get<DeserializeUserError>(result), DeserializeUserError::InvalidId);
}
TEST(PersistenceTests, DeserializeUserReturnsInvalidFieldCount)
{
    const auto result = deserializeUser("USER|1|Alice");

    ASSERT_TRUE(std::holds_alternative<DeserializeUserError>(result));

    EXPECT_EQ(std::get<DeserializeUserError>(result), DeserializeUserError::InvalidFieldCount);
}

TEST(PersistenceTests, LoadNetworkReturnsInvalidRecordType)
{
    const std::filesystem::path path = "test_data/invalid_type.txt";

    std::filesystem::create_directories("test_data");

    ASSERT_TRUE(saveText(path, "SOMETHING|1|2|3\n"));
    Network network;

    const auto result = loadNetwork(network, path);

    ASSERT_TRUE(std::holds_alternative<LoadNetworkFailure>(result));
    const auto &failure = std::get<LoadNetworkFailure>(result);
    EXPECT_EQ(failure.error, LoadNetworkError::InvalidRecordType);
    EXPECT_EQ(failure.lineNumber, 1);

    std::filesystem::remove_all("test_data");
}

TEST(PersistenceTests, LoadNetworkReturnsInvalidUser)
{
    const std::filesystem::path path = "test_data/invalid_user.txt";

    std::filesystem::create_directories("test_data");

    ASSERT_TRUE(saveText(path, "USER|abc|Alice|123\n"));
    Network network;

    const auto result = loadNetwork(network, path);

    ASSERT_TRUE(std::holds_alternative<LoadNetworkFailure>(result));
    const auto &failure = std::get<LoadNetworkFailure>(result);
    EXPECT_EQ(failure.error, LoadNetworkError::InvalidUser);
    EXPECT_EQ(failure.lineNumber, 1);
    EXPECT_EQ(failure.userError, std::optional{DeserializeUserError::InvalidId});

    std::filesystem::remove_all("test_data");
}

TEST(PersistenceTests, LoadNetworkPreservesUserParsingErrorAndLineNumber)
{
    const std::filesystem::path path = "test_data/invalid_user_details.txt";

    std::filesystem::create_directories("test_data");

    ASSERT_TRUE(saveText(path, "USER|1|Alice|111\n"
                               "USER|2|Bob|222\n"
                               "USER|abc|Charlie|333\n"));
    Network network;

    const auto result = loadNetwork(network, path);

    ASSERT_TRUE(std::holds_alternative<LoadNetworkFailure>(result));

    const auto &failure = std::get<LoadNetworkFailure>(result);

    EXPECT_EQ(failure.error, LoadNetworkError::InvalidUser);

    EXPECT_EQ(failure.lineNumber, 3);

    EXPECT_EQ(failure.userError, std::optional{DeserializeUserError::InvalidId});

    std::filesystem::remove_all("test_data");
}

TEST(PersistenceTests, LoadNetworkPreservesInvalidUserPrefix)
{
    const std::filesystem::path path = "test_data/invalid_user_prefix.txt";

    std::filesystem::create_directories("test_data");

    ASSERT_TRUE(saveText(path, "USER|1|Alice|111\n"
                               "CALLX|2|Bob|222\n"));
    Network network;

    const auto result = loadNetwork(network, path);

    ASSERT_TRUE(std::holds_alternative<LoadNetworkFailure>(result));

    const auto &failure = std::get<LoadNetworkFailure>(result);

    EXPECT_EQ(failure.lineNumber, 2);
    EXPECT_EQ(failure.error, LoadNetworkError::InvalidRecordType);

    EXPECT_FALSE(failure.userError.has_value());

    std::filesystem::remove_all("test_data");
}

TEST(PersistenceTests, LoadNetworkFileOpenFailureHasNoLineNumber)
{
    Network network;

    const auto result = loadNetwork(network, "does_not_exist.txt");

    ASSERT_TRUE(std::holds_alternative<LoadNetworkFailure>(result));

    const auto &failure = std::get<LoadNetworkFailure>(result);

    EXPECT_EQ(failure.error, LoadNetworkError::FileOpenFailed);

    EXPECT_EQ(failure.lineNumber, 0);
    EXPECT_FALSE(failure.userError.has_value());
}

TEST(PersistenceTests, LoadTextOrThrowReturnsContent)
{
    const std::filesystem::path dir = "test_data";
    const std::filesystem::path path = dir / "text.txt";

    std::filesystem::remove_all(dir);
    std::filesystem::create_directories(dir);

    ASSERT_TRUE(saveText(path, "Hello"));

    EXPECT_EQ(loadTextOrThrow(path), "Hello");

    std::filesystem::remove_all(dir);
}

TEST(PersistenceTests, LoadTextOrThrowThrowsWhenFileDoesNotExist)
{
    const std::filesystem::path path = "test_data/missing.txt";

    std::filesystem::remove_all("test_data");

    EXPECT_THROW(loadTextOrThrow(path), std::runtime_error);
}
