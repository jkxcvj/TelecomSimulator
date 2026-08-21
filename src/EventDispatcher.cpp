#include "EventDispatcher.h"

#include "algorithm"

std::string_view toString(EventType eventType)
{
    switch (eventType)
    {
    case EventType::UserRegistered:
        return "User registered";
    case EventType::UserRegistrationRejected:
        return "User registration rejected";
    case EventType::CallCreated:
        return "Call created";
    case EventType::CallCreationRejected:
        return "Call creation rejected";
    case EventType::CallStarted:
        return "Call started";
    case EventType::CallStartRejected:
        return "Call start rejected";
    case EventType::CallEnded:
        return "Call ended";
    case EventType::CallEndRejected:
        return "Call end rejected";
    case EventType::CallRestored:
        return "Call restored";
    }
    return "Unknown event";
}

void EventDispatcher::subscribe(const std::shared_ptr<EventSubscriber> &subscriber)
{
    auto iterator = mSubscribers.begin();
    while (iterator != mSubscribers.end())
    {
        if (auto sub = iterator->lock())
        {
            if (sub == subscriber)
            {
                return;
            }
            ++iterator;
        }
        else
        {
            iterator = mSubscribers.erase(iterator);
        }
    }
    if (subscriber != nullptr)
    {
        mSubscribers.emplace_back(subscriber);
    }
    else
    {
        throw std::invalid_argument("Subscriber cannot be null");
    }
}

void EventDispatcher::notify(EventType event)
{
    auto iterator = mSubscribers.begin();

    while (iterator != mSubscribers.end())
    {
        if (auto subscriber = iterator->lock())
        {
            subscriber->onEvent(event);
            ++iterator;
        }
        else
        {
            iterator = mSubscribers.erase(iterator);
        }
    }
}

void EventDispatcher::unsubscribe(const std::shared_ptr<EventSubscriber> &subscriber)
{
    std::erase_if(mSubscribers,
                  [&](const std::weak_ptr<EventSubscriber> &weakSubscriber)
                  {
                      auto activeSubscriber = weakSubscriber.lock();

                      if (activeSubscriber == subscriber || !activeSubscriber)
                      {
                          return true;
                      }
                      return false;
                  });
}
