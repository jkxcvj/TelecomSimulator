#include <iostream>
#include "User.h"

int main()
{
    std::cout << "Telecom Simulator\n";
    User user(1, "John Doe", "123-456-7890");
    user.print();
    return 0;
}