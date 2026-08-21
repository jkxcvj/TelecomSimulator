#pragma once

#include <string_view>

enum class EventType
{
    UserRegistered,
    UserRegistrationRejected,
    CallCreated,
    CallCreationRejected,
    CallStarted,
    CallStartRejected,
    CallEnded,
    CallEndRejected,
    CallRestored
};

std::string_view toString(EventType eventType);

class EventSubscriber
{
  public:
    virtual ~EventSubscriber() = default;

    virtual void onEvent(EventType event) = 0;
};
