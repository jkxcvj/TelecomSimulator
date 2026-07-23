#include "User.h"
#include <iostream>

User::User(int id, std::string name, std::string phoneNumber)
    : id(id), name(std::move(name)), phoneNumber(std::move(phoneNumber))
{
}

int User::getId() const
{
    return id;
}

const std::string &User::getName() const
{
    return name;
}

const std::string &User::getPhoneNumber() const
{
    return phoneNumber;
}

void User::print() const
{
    std::cout << "User ID: " << id << "\n";
    std::cout << "Name: " << name << "\n";
    std::cout << "Phone Number: " << phoneNumber << "\n";
}