#include <iostream>
#include <string_view>

#include "Network.h"
#include "User.h"

namespace
{
void printResult(bool success, std::string_view successMessage, std::string_view errorMessage)
{
    std::cout << (success ? "[OK] " : "[ERROR] ") << (success ? successMessage : errorMessage) << '\n';
}
} // namespace

int main()
{
    std::cout << "================================\n"
              << "      TELECOM SIMULATOR\n"
              << "================================\n\n";

    Network network;
    const User alice{1, "Alice Johnson", "+48 500 100 100"};
    const User bob{2, "Bob Smith", "+48 500 200 200"};
    const User charlie{3, "Charlie Brown", "+48 500 300 300"};

    std::cout << "--- Registering users ---\n";
    printResult(network.addUser(alice), "Added user Alice with ID 1", "Could not add user Alice");
    printResult(network.addUser(bob), "Added user Bob with ID 2", "Could not add user Bob");
    printResult(network.addUser(charlie), "Added user Charlie with ID 3", "Could not add user Charlie");
    printResult(network.addUser(alice), "Added user Alice again", "User with ID 1 already exists");

    std::cout << "\n--- Registered users ---\n";
    network.printUsers();

    std::cout << "\n--- Call scenario ---\n";
    printResult(network.createCall(101, alice.getId(), bob.getId()), "Created call 101 from Alice to Bob",
                "Could not create call 101 from Alice to Bob");
    printResult(network.startCall(101), "Started call 101 between Alice and Bob",
                "Could not start call 101 between Alice and Bob");

    printResult(network.createCall(102, alice.getId(), charlie.getId()), "Created call 102 from Alice to Charlie",
                "Could not create call 102 from Alice to Charlie");
    printResult(network.startCall(102), "Started call 102 between Alice and Charlie",
                "Could not start call 102 because Alice is already busy");

    printResult(network.endCall(101), "Ended call 101 between Alice and Bob",
                "Could not end call 101 between Alice and Bob");

    std::cout << "\n--- Final call records ---\n";
    network.printCalls();

    std::cout << "\nDemo completed. Registered users: " << network.getUserCount()
              << ", call records: " << network.getCallCount() << '\n';

    return 0;
}
