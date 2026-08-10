#include <iostream>
#include <memory>
#include <string_view>
#include <variant>

#include "FileEventLogger.h"
#include "Network.h"

namespace
{

struct ResultMessages
{
    std::string_view success;
    std::string_view failure;
};

void printResult(bool success, ResultMessages messages)
{
    if (success)
    {
        std::cout << "[OK] " << messages.success << '\n';
    }
    else
    {
        std::cout << "[ERROR] " << messages.failure << '\n';
    }
}

void printStartCallResult(const StartCallResult &result, std::string_view successMessage)
{
    if (std::holds_alternative<std::monostate>(result))
    {
        std::cout << "[OK] " << successMessage << '\n';
        return;
    }

    const auto error = std::get<StartCallError>(result);

    std::cout << "[ERROR] Cannot start call: " << toString(error) << '\n';
}

} // namespace

int run()
{
    std::cout << "================================\n"
              << "      TELECOM SIMULATOR\n"
              << "================================\n\n";

    Network network(std::make_unique<FileEventLogger>("telecom-events.log"));

    const User alice{UserId{1}, "Alice Johnson", "+48 500 100 100"};

    const User bob{UserId{2}, "Bob Smith", "+48 500 200 200"};

    const User charlie{UserId{3}, "Charlie Brown", "+48 500 300 300"};

    std::cout << "--- Registering users ---\n";

    printResult(network.addUser(alice), {.success = "Added user Alice with ID 1", .failure = "Could not add user Alice"});

    printResult(network.addUser(bob), {.success = "Added user Bob with ID 2", .failure = "Could not add user Bob"});

    printResult(network.addUser(charlie), {.success = "Added user Charlie with ID 3", .failure = "Could not add user Charlie"});

    printResult(network.addUser(alice), {.success = "Added user Alice again", .failure = "User with ID 1 already exists"});

    std::cout << "\n--- Registered users ---\n";

    network.printUsers();

    std::cout << "\n--- Call scenario ---\n";

    printResult(network.createCall(CallId{101}, alice.getId(), bob.getId()),
                {.success = "Created call 101 from Alice to Bob", .failure = "Could not create call 101 from Alice to Bob"});

    printStartCallResult(network.startCall(CallId{101}), "Started call 101 between Alice and Bob");

    printResult(network.createCall(CallId{102}, alice.getId(), charlie.getId()),
                {.success = "Created call 102 from Alice to Charlie", .failure = "Could not create call 102 from Alice to Charlie"});

    printStartCallResult(network.startCall(CallId{102}), "Started call 102 between Alice and Charlie");

    printResult(network.endCall(CallId{101}),
                {.success = "Ended call 101 between Alice and Bob", .failure = "Could not end call 101 between Alice and Bob"});

    std::cout << "\n--- Final call records ---\n";

    network.printCalls();

    std::cout << "\nDemo completed. Registered users: " << network.getUserCount() << ", call records: " << network.getCallCount() << '\n';

    return 0;
}

int main()
{
    try
    {
        return run();
    }
    catch (const std::exception &exception)
    {
        std::cerr << "[FATAL] " << exception.what() << '\n';
        return 1;
    }
    catch (...)
    {
        std::cerr << "[FATAL] Unknown exception\n";
        return 1;
    }
}