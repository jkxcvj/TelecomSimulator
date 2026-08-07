#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "TestEventSubscriber.h"

TEST(EventDispatcherTests, NotifiesSubscribedObserver)
{
    EventDispatcher dispatcher;
    auto subscriber = std::make_shared<RecordingEventSubscriber>();
    dispatcher.subscribe(subscriber);
    dispatcher.notify("User registered");

    ASSERT_EQ(subscriber->getMessages().size(), 1);
    EXPECT_EQ(subscriber->getMessages()[0], "User registered");
}

TEST(EventDispatcherTests, DoesNotKeepSubscriberAlive)
{
    EventDispatcher dispatcher;

    auto subscriber = std::make_shared<RecordingEventSubscriber>();

    std::weak_ptr<RecordingEventSubscriber> lifetimeObserver = subscriber;

    dispatcher.subscribe(subscriber);

    EXPECT_FALSE(lifetimeObserver.expired());
    EXPECT_EQ(subscriber.use_count(), 1);

    subscriber.reset();

    EXPECT_TRUE(lifetimeObserver.expired());

    EXPECT_NO_THROW(dispatcher.notify("User registered"));
}

TEST(EventDispatcherTests, SkipsExpiredSubscriberAndNotifiesActiveSubscriber)
{
    EventDispatcher dispatcher;

    auto expiredSubscriber = std::make_shared<RecordingEventSubscriber>();

    auto activeSubscriber = std::make_shared<RecordingEventSubscriber>();

    dispatcher.subscribe(expiredSubscriber);
    dispatcher.subscribe(activeSubscriber);

    expiredSubscriber.reset();

    dispatcher.notify("Call started");

    const auto &messages = activeSubscriber->getMessages();

    ASSERT_EQ(messages.size(), 1);
    EXPECT_EQ(messages[0], "Call started");
}

TEST(EventDispatcherTests, NotifiesAllActiveSubscribers)
{
    EventDispatcher dispatcher;

    auto activeSubscriber1 = std::make_shared<RecordingEventSubscriber>();

    auto activeSubscriber2 = std::make_shared<RecordingEventSubscriber>();

    dispatcher.subscribe(activeSubscriber1);
    dispatcher.subscribe(activeSubscriber2);

    dispatcher.notify("Call started");

    const auto &messages = activeSubscriber1->getMessages();

    ASSERT_EQ(messages.size(), 1);
    EXPECT_EQ(messages[0], "Call started");
    const auto &messages2 = activeSubscriber2->getMessages();

    ASSERT_EQ(messages2.size(), 1);
    EXPECT_EQ(messages2[0], "Call started");
}

TEST(EventDispatcherTests, DoesNotNotifyUnsubscribedObserver)
{
    EventDispatcher dispatcher;

    auto firstSubscriber = std::make_shared<RecordingEventSubscriber>();

    auto secondSubscriber = std::make_shared<RecordingEventSubscriber>();

    dispatcher.subscribe(firstSubscriber);
    dispatcher.subscribe(secondSubscriber);

    dispatcher.unsubscribe(firstSubscriber);

    dispatcher.notify("Call ended");

    EXPECT_TRUE(firstSubscriber->getMessages().empty());

    ASSERT_EQ(secondSubscriber->getMessages().size(), 1);
    EXPECT_EQ(secondSubscriber->getMessages()[0], "Call ended");
}

TEST(EventDispatcherTests, DoesNotSubscribeSameObserverTwice)
{
    EventDispatcher dispatcher;

    auto subscriber = std::make_shared<RecordingEventSubscriber>();

    dispatcher.subscribe(subscriber);
    dispatcher.subscribe(subscriber);

    dispatcher.notify("Call started");

    const auto &messages = subscriber->getMessages();

    ASSERT_EQ(messages.size(), 1);
    EXPECT_EQ(messages[0], "Call started");
}

TEST(EventDispatcherTests, RejectsNullSubscriber)
{
    EventDispatcher dispatcher;

    EXPECT_THROW(dispatcher.subscribe(nullptr), std::invalid_argument);
}