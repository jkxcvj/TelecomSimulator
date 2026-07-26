#include "Network.h"
#include <iostream>

void Network::printUsers() const
{
    if (mUsers.empty())
    {
        std::cout << "No users in the network." << std::endl;
        return;
    }
    for (const auto &user : mUsers)
    {
        std::cout << "User ID: " << user.getId() << ", Name: " << user.getName() << std::endl;
    }
}
std::size_t Network::getUserCount() const
{
    return mUsers.size();
}

bool Network::addUser(const User &user)
{
    for (const auto &userInDB : mUsers)
    {
        if (user.getId() == userInDB.getId())
        {
            return false;
        }
    }
    mUsers.push_back(user);
    return true;
}

bool Network::removeUser(int id)
{
    for (auto it = mUsers.begin(); it != mUsers.end(); it++)
    {
        if (it->getId() == id)
        {
            mUsers.erase(it);
            return true;
        }
    }
    return false;
}
