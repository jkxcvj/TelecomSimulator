#pragma once

#include <memory>
#include <string_view>
#include <vector>

#include "EventSubscriber.h"

class EventDispatcher
{
  public:
    void subscribe(const std::shared_ptr<EventSubscriber> &subscriber);
    void notify(std::string_view message);
    void unsubscribe(const std::shared_ptr<EventSubscriber> &subscriber);

  private:
    std::vector<std::weak_ptr<EventSubscriber>> mSubscribers;
};