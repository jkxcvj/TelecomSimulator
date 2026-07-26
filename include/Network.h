#pragma once

#include <vector>
#include "User.h"
#include <cstddef>

class Network
{
public:
    bool addUser(const User &user);
    bool removeUser(int id);
    void printUsers() const;
    std::size_t getUserCount() const;

private:
    std::vector<User> mUsers;
};