#pragma once

#include <string>

class User
{
public:
    User(int id, std::string name, std::string phoneNumber);

    int getId() const;
    const std::string &getName() const;
    const std::string &getPhoneNumber() const;
    void print() const;

private:
    int id;
    std::string name;
    std::string phoneNumber;
};