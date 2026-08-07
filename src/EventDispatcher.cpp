#include "EventDispatcher.h"

#include "algorithm"

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

void EventDispatcher::notify(std::string_view message)
{
    auto iterator = mSubscribers.begin();

    while (iterator != mSubscribers.end())
    {
        if (auto subscriber = iterator->lock())
        {
            subscriber->onEvent(message);
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