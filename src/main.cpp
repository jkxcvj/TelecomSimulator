#include <iostream>
#include "User.h"
#include "Network.h"

int main()
{
    Network myNetwork;
    std::cout << "Telecom Simulator\n";
    User user1(1, "John Doe", "123-456-7890");
    User user2(2, "Johnson Doeson", "123-456-6767");
    User user3(2, "Johnsini Doesini", "123-456-6969");
    std::cout << "Adding Users to myNetwork" << std::endl;
    std::cout << std::boolalpha << myNetwork.addUser(user1) << std::endl;
    std::cout << std::boolalpha << myNetwork.addUser(user2) << std::endl;
    std::cout << std::boolalpha << myNetwork.addUser(user3) << std::endl;
    user1.print();
    myNetwork.printUsers();
    std::cout << "Current User count: " << myNetwork.getUserCount() << std::endl;
    std::cout << "Removing Users to myNetwork" << std::endl;
    std::cout << std::boolalpha << myNetwork.removeUser(2) << std::endl;
    std::cout << std::boolalpha << myNetwork.removeUser(2) << std::endl;
    myNetwork.printUsers();
    std::cout << "Current User count: " << myNetwork.getUserCount() << std::endl;
    return 0;
}