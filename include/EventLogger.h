#pragma once

#include <string_view>

class EventLogger
{
  public:
    virtual ~EventLogger() = default;
    virtual void log(std::string_view message) = 0;
};
