#pragma once

#include <string_view>

class EventSubscriber
{
  public:
    virtual ~EventSubscriber() = default;

    virtual void onEvent(std::string_view message) = 0;
};