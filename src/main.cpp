#include <iostream>
#include "User.h"
#include "Network.h"
#include "Call.h"

int main()
{
    std::cout << std::boolalpha;
    Network myNetwork;
    std::cout << "Telecom Simulator\n";
    User user1(1, "John Doe", "123-456-7890");
    User user2(2, "Johnson Doeson", "123-456-6767");
    User user3(3, "Johnsini Doesini", "123-456-6969");
    std::cout << "Adding Users to myNetwork" << "\n";
    std::cout << myNetwork.addUser(user1) << "\n";
    std::cout << myNetwork.addUser(user2) << "\n";
    std::cout << myNetwork.addUser(user3) << "\n";
    user1.print();
    myNetwork.printUsers();
    std::cout << "Current User count: " << myNetwork.getUserCount() << "\n";
    // std::cout << "Removing Users to myNetwork" << "\n";
    // std::cout << myNetwork.removeUser(2) << "\n";
    // std::cout << myNetwork.removeUser(2) << "\n";
    myNetwork.printUsers();
    std::cout << "Current User count: " << myNetwork.getUserCount() << "\n";
    std::cout << "TWORZE NOWE POLACZENIE \n";
    Call firstCall(1, 1, 2);
    firstCall.print();
    std::cout << firstCall.end() << "\n";
    std::cout << firstCall.start() << "\n";
    std::cout << firstCall.start() << "\n";
    firstCall.print();
    std::cout << firstCall.end() << "\n";
    std::cout << firstCall.end() << "\n";
    std::cout << firstCall.start() << "\n";
    firstCall.print();
    std::cout << "TWORZE PRAWDZIWE POLACZENIE \n";
    std::cout << myNetwork.createCall(1, user1.getId(), user2.getId()) << "\n";
    std::cout << myNetwork.createCall(3, 4, user2.getId()) << "\n";
    std::cout << myNetwork.createCall(4, user1.getId(), 4) << "\n";
    std::cout << myNetwork.createCall(5, user1.getId(), user1.getId()) << "\n";
    std::cout << myNetwork.createCall(1, user1.getId(), user2.getId()) << "\n";
    std::cout << myNetwork.createCall(2, user1.getId(), user3.getId()) << "\n";
    myNetwork.printCalls();
    std::cout << myNetwork.getCallCount() << "\n";
    return 0;
}