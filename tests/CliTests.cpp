#include <gtest/gtest.h>
#include <optional>
#include <sstream>

#include "CliApplication.h"
#include "CliUtils.h"
#include "PersistenceUtils.h"
#include "TestEventLogger.h"

TEST(CliTests, ReadsValidMenuOption)
{
    std::istringstream input("3");

    const auto option = readMenuOption(input);

    EXPECT_EQ(option, std::optional{MenuOption::CreateCall});
}

TEST(CliTests, RejectsInvalidNumericMenuOption)
{
    std::istringstream input("15");

    const auto option = readMenuOption(input);

    EXPECT_FALSE(option.has_value());
}

TEST(CliTests, RejectsNonNumericMenuOption)
{
    std::istringstream input("abc");

    const auto option = readMenuOption(input);

    EXPECT_FALSE(option.has_value());
}

TEST(CliTests, PrintsMenu)
{
    std::ostringstream output;

    printMenu(output);

    const std::string text = output.str();

    EXPECT_NE(text.find("1. AddUser"), std::string::npos);
    EXPECT_NE(text.find("0. Exit"), std::string::npos);
}

TEST(CliTests, AddUserFromInput)
{
    CallStatistics statistics;
    auto logger = std::make_unique<SilentEventLogger>();
    Network network(std::move(logger), statistics);

    std::istringstream input("1\n"
                             "Alice\n"
                             "123456789\n");

    std::ostringstream output;

    handleAddUser(network, input, output);

    EXPECT_EQ(network.getUserCount(), 1);

    const User *user = network.getUser(UserId{1});

    ASSERT_NE(user, nullptr);
    EXPECT_EQ(user->getName(), "Alice");
    EXPECT_EQ(user->getPhoneNumber(), "123456789");

    EXPECT_NE(output.str().find("User added"), std::string::npos);
}

TEST(CliTests, AddUserReportsDuplicate)
{
    CallStatistics statistics;
    auto logger = std::make_unique<SilentEventLogger>();
    Network network(std::move(logger), statistics);

    ASSERT_TRUE(network.addUser(User(UserId{1}, "Existing", "111")));

    std::istringstream input("1\n"
                             "Alice\n"
                             "123456789\n");

    std::ostringstream output;

    handleAddUser(network, input, output);

    EXPECT_EQ(network.getUserCount(), 1);

    EXPECT_NE(output.str().find("User already exists"), std::string::npos);
}
TEST(CliTests, RemovesUserFromInput)
{
    CallStatistics statistics;
    auto logger = std::make_unique<SilentEventLogger>();
    Network network(std::move(logger), statistics);

    ASSERT_TRUE(network.addUser(User(UserId{1}, "Alice", "111")));

    std::istringstream input("1\n");
    std::ostringstream output;

    handleRemoveUser(network, input, output);

    EXPECT_EQ(network.getUserCount(), 0);

    EXPECT_NE(output.str().find("User removed"), std::string::npos);
}

TEST(CliTests, RemoveUserReportsNotFound)
{
    CallStatistics statistics;
    auto logger = std::make_unique<SilentEventLogger>();
    Network network(std::move(logger), statistics);

    std::istringstream input("999\n");
    std::ostringstream output;

    handleRemoveUser(network, input, output);

    EXPECT_NE(output.str().find("User not found"), std::string::npos);
}

TEST(CliTests, CreatesCallFromInput)
{
    CallStatistics statistics;
    auto logger = std::make_unique<SilentEventLogger>();
    Network network(std::move(logger), statistics);

    ASSERT_TRUE(network.addUser(User(UserId{1}, "Alice", "111")));
    ASSERT_TRUE(network.addUser(User(UserId{2}, "Bob", "222")));

    std::istringstream input("100\n"
                             "1\n"
                             "2\n");

    std::ostringstream output;

    handleCreateCall(network, input, output);

    EXPECT_EQ(network.getCallCount(), 1);

    EXPECT_NE(output.str().find("Call created"), std::string::npos);
}

TEST(CliTests, CreateCallReportsMissingCaller)
{
    CallStatistics statistics;
    auto logger = std::make_unique<SilentEventLogger>();
    Network network(std::move(logger), statistics);

    ASSERT_TRUE(network.addUser(User(UserId{2}, "Bob", "222")));

    std::istringstream input("100\n"
                             "1\n"
                             "2\n");

    std::ostringstream output;

    handleCreateCall(network, input, output);

    EXPECT_NE(output.str().find("Caller not found"), std::string::npos);
}

TEST(CliTests, StartsCallFromInput)
{
    CallStatistics statistics;
    auto logger = std::make_unique<SilentEventLogger>();
    Network network(std::move(logger), statistics);

    ASSERT_TRUE(network.addUser(User(UserId{1}, "Alice", "111")));
    ASSERT_TRUE(network.addUser(User(UserId{2}, "Bob", "222")));

    const auto createResult = network.createCall(CallId{100}, UserId{1}, UserId{2});

    ASSERT_TRUE(std::holds_alternative<std::monostate>(createResult));

    std::istringstream input("100\n");
    std::ostringstream output;

    handleStartCall(network, input, output);

    EXPECT_NE(output.str().find("Call started"), std::string::npos);
}

TEST(CliTests, EndsCallFromInput)
{
    CallStatistics statistics;
    auto logger = std::make_unique<SilentEventLogger>();
    Network network(std::move(logger), statistics);

    ASSERT_TRUE(network.addUser(User(UserId{1}, "Alice", "111")));
    ASSERT_TRUE(network.addUser(User(UserId{2}, "Bob", "222")));

    const auto createResult = network.createCall(CallId{100}, UserId{1}, UserId{2});

    ASSERT_TRUE(std::holds_alternative<std::monostate>(createResult));

    const auto startResult = network.startCall(CallId{100});

    ASSERT_TRUE(std::holds_alternative<std::monostate>(startResult));

    std::istringstream input("100\n");
    std::ostringstream output;

    handleEndCall(network, input, output);

    EXPECT_NE(output.str().find("Call ended"), std::string::npos);
}

TEST(CliTests, ListsUsers)
{
    CallStatistics statistics;
    auto logger = std::make_unique<SilentEventLogger>();
    Network network(std::move(logger), statistics);

    ASSERT_TRUE(network.addUser(User(UserId{1}, "Alice", "111")));
    ASSERT_TRUE(network.addUser(User(UserId{2}, "Bob", "222")));

    std::ostringstream output;

    handleListUsers(network, output);

    const auto text = output.str();

    EXPECT_NE(text.find("Alice"), std::string::npos);
    EXPECT_NE(text.find("Bob"), std::string::npos);
    EXPECT_NE(text.find("111"), std::string::npos);
    EXPECT_NE(text.find("222"), std::string::npos);
}

TEST(CliTests, ListsCalls)
{
    CallStatistics statistics;
    auto logger = std::make_unique<SilentEventLogger>();
    Network network(std::move(logger), statistics);

    ASSERT_TRUE(network.addUser(User(UserId{1}, "Alice", "111")));
    ASSERT_TRUE(network.addUser(User(UserId{2}, "Bob", "222")));

    const auto createResult = network.createCall(CallId{100}, UserId{1}, UserId{2});

    ASSERT_TRUE(std::holds_alternative<std::monostate>(createResult));

    std::ostringstream output;

    handleListCalls(network, output);

    const auto text = output.str();

    EXPECT_NE(text.find("100"), std::string::npos);
    EXPECT_NE(text.find('1'), std::string::npos);
    EXPECT_NE(text.find('2'), std::string::npos);
    EXPECT_NE(text.find("Created"), std::string::npos);
}

TEST(CliTests, SavesNetworkFromCli)
{
    const std::filesystem::path dir = "test_data";
    const std::filesystem::path path = dir / "cli_save.txt";

    std::filesystem::remove_all(dir);
    std::filesystem::create_directories(dir);

    CallStatistics statistics;
    auto logger = std::make_unique<SilentEventLogger>();
    Network network(std::move(logger), statistics);

    ASSERT_TRUE(network.addUser(User(UserId{1}, "Alice", "111")));

    std::istringstream input(path.string() + "\n");
    std::ostringstream output;

    handleSave(network, input, output);

    EXPECT_TRUE(std::filesystem::exists(path));
    EXPECT_NE(output.str().find("Network saved"), std::string::npos);

    std::filesystem::remove_all(dir);
}

TEST(CliTests, LoadsNetworkFromCli)
{
    const std::filesystem::path dir = "test_data";
    const std::filesystem::path path = dir / "cli_load.txt";

    std::filesystem::remove_all(dir);
    std::filesystem::create_directories(dir);

    ASSERT_TRUE(saveText(path, "USER|1|Alice|111\n"));

    CallStatistics statistics;
    auto logger = std::make_unique<SilentEventLogger>();
    Network network(std::move(logger), statistics);

    std::istringstream input(path.string() + "\n");
    std::ostringstream output;

    handleLoad(network, input, output);

    EXPECT_EQ(network.getUserCount(), 1);

    EXPECT_NE(output.str().find("Network loaded"), std::string::npos);

    std::filesystem::remove_all(dir);
}

TEST(CliTests, LoadReportsFailure)
{
    CallStatistics statistics;
    auto logger = std::make_unique<SilentEventLogger>();
    Network network(std::move(logger), statistics);

    std::istringstream input("missing_file.txt\n");
    std::ostringstream output;

    handleLoad(network, input, output);

    EXPECT_NE(output.str().find("Failed to load network"), std::string::npos);
}

TEST(CliApplicationTests, ExitsWhenUserSelectsExit)
{
    CallStatistics statistics;
    auto logger = std::make_unique<SilentEventLogger>();
    Network network(std::move(logger), statistics);

    std::istringstream input("0\n");
    std::ostringstream output;

    CliApplication app(network, input, output);

    EXPECT_EQ(app.run(), 0);
}

TEST(CliApplicationTests, HandlesInvalidOptionAndContinues)
{
    CallStatistics statistics;
    auto logger = std::make_unique<SilentEventLogger>();
    Network network(std::move(logger), statistics);

    std::istringstream input("abc\n"
                             "0\n");

    std::ostringstream output;

    CliApplication app(network, input, output);

    EXPECT_EQ(app.run(), 0);

    EXPECT_NE(output.str().find("Invalid option"), std::string::npos);
}

TEST(CliTests, ReadIntReturnsValue)
{
    std::istringstream input("42\n");

    const auto result = readInt(input);

    EXPECT_EQ(result, std::optional{42});
}

TEST(CliTests, ReadIntRejectsInvalidInput)
{
    std::istringstream input("abc\n");

    const auto result = readInt(input);

    EXPECT_FALSE(result.has_value());
}

TEST(CliTests, ReadIntRecoversStreamAfterInvalidInput)
{
    std::istringstream input("abc\n"
                             "42\n");

    const auto first = readInt(input);
    const auto second = readInt(input);

    EXPECT_FALSE(first.has_value());

    EXPECT_EQ(second, std::optional{42});
}

TEST(CliApplicationTests, HandlesMultipleCommandsAndExits)
{
    CallStatistics statistics;
    auto logger = std::make_unique<SilentEventLogger>();
    Network network(std::move(logger), statistics);

    std::istringstream input("1\n" // AddUser
                             "1\n"
                             "Alice\n"
                             "111\n"

                             "1\n" // AddUser
                             "2\n"
                             "Bob\n"
                             "222\n"

                             "3\n" // CreateCall
                             "100\n"
                             "1\n"
                             "2\n"

                             "6\n" // ListUsers
                             "7\n" // ListCalls

                             "0\n" // Exit
    );

    std::ostringstream output;

    CliApplication app(network, input, output);

    EXPECT_EQ(app.run(), 0);

    EXPECT_EQ(network.getUserCount(), 2);
    EXPECT_EQ(network.getCallCount(), 1);

    const std::string text = output.str();

    EXPECT_NE(text.find("Alice"), std::string::npos);
    EXPECT_NE(text.find("Bob"), std::string::npos);
    EXPECT_NE(text.find("100"), std::string::npos);
}
