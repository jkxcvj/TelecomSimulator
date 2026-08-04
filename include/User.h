#pragma once

#include <string>

#include "Identifiers.h"

class User
{
  public:
    User(UserId id, std::string name, std::string phoneNumber);

    UserId getId() const;
    const std::string &getName() const;
    const std::string &getPhoneNumber() const;
    void print() const;

  private:
    UserId mId;
    std::string mName;
    std::string mPhoneNumber;
};