#include "User.h"

#include <iostream>

User::User(int id, std::string name, std::string phoneNumber)
    : mId(id), mName(std::move(name)), mPhoneNumber(std::move(phoneNumber))
{
}

int User::getId() const { return mId; }

const std::string &User::getName() const { return mName; }

const std::string &User::getPhoneNumber() const { return mPhoneNumber; }

void User::print() const
{
    std::cout << "User ID: " << mId << "\n";
    std::cout << "Name: " << mName << "\n";
    std::cout << "Phone Number: " << mPhoneNumber << "\n";
}