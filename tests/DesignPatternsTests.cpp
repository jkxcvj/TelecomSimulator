#include <gtest/gtest.h>
#include <memory>

#include "DesignPatterns.h"

TEST(StrategyPatternTests, CalculatesCostUsingStandardPricing)
{
    CallCostCalculator calculator{std::make_unique<StandardPricing>()};

    EXPECT_DOUBLE_EQ(calculator.calculate(10.0), 5.0);
    EXPECT_DOUBLE_EQ(calculator.calculate(4.0), 2.0);
}

TEST(StrategyPatternTests, CalculatesCostUsingDiscountPricing)
{
    CallCostCalculator calculator{std::make_unique<DiscountPricing>()};

    EXPECT_DOUBLE_EQ(calculator.calculate(10.0), 2.5);
    EXPECT_DOUBLE_EQ(calculator.calculate(4.0), 1.0);
}

TEST(StrategyPatternTests, DifferentStrategiesProduceDifferentResults)
{
    CallCostCalculator standardCalculator{std::make_unique<StandardPricing>()};
    CallCostCalculator discountCalculator{std::make_unique<DiscountPricing>()};

    const double minutes = 20.0;

    EXPECT_DOUBLE_EQ(standardCalculator.calculate(minutes), 10.0);
    EXPECT_DOUBLE_EQ(discountCalculator.calculate(minutes), 5.0);
}

TEST(ObserverPatternTests, NotifiesSingleObserver)
{
    MessagePublisher publisher;
    auto observer = std::make_shared<RecordingObserver>();

    publisher.subscribe(observer);
    publisher.publish("Hello");

    EXPECT_EQ(observer->getLastMessage(), "Hello");
}

TEST(ObserverPatternTests, NotifiesMultipleObservers)
{
    MessagePublisher publisher;

    auto observer1 = std::make_shared<RecordingObserver>();
    auto observer2 = std::make_shared<RecordingObserver>();

    publisher.subscribe(observer1);
    publisher.subscribe(observer2);

    publisher.publish("Call started");

    EXPECT_EQ(observer1->getLastMessage(), "Call started");
    EXPECT_EQ(observer2->getLastMessage(), "Call started");
}

TEST(ObserverPatternTests, ContinuesWorkingAfterObserverIsDestroyed)
{
    MessagePublisher publisher;

    auto survivingObserver = std::make_shared<RecordingObserver>();

    {
        auto temporaryObserver = std::make_shared<RecordingObserver>();

        publisher.subscribe(survivingObserver);
        publisher.subscribe(temporaryObserver);

        publisher.publish("First message");

        EXPECT_EQ(survivingObserver->getLastMessage(), "First message");
        EXPECT_EQ(temporaryObserver->getLastMessage(), "First message");
    }

    publisher.publish("Second message");

    EXPECT_EQ(survivingObserver->getLastMessage(), "Second message");
}

TEST(FactoryPatternTests, CreatesStandardPricing)
{
    auto strategy = PricingFactory::create(PricingType::Standard);

    ASSERT_NE(strategy, nullptr);
    EXPECT_DOUBLE_EQ(strategy->calculate(10.0), 5.0);
}

TEST(FactoryPatternTests, CreatesDiscountPricing)
{
    auto strategy = PricingFactory::create(PricingType::Discount);

    ASSERT_NE(strategy, nullptr);
    EXPECT_DOUBLE_EQ(strategy->calculate(10.0), 2.5);
}

TEST(FactoryPatternTests, CreatedStrategyWorksWithCalculator)
{
    auto strategy = PricingFactory::create(PricingType::Discount);

    CallCostCalculator calculator{std::move(strategy)};

    EXPECT_DOUBLE_EQ(calculator.calculate(20.0), 5.0);
}

TEST(StatePatternTests, StartsWithDisconnectedState)
{
    Connection connection{std::make_unique<DisconnectedState>()};

    EXPECT_EQ(connection.stateName(), "Disconnected");
}

TEST(StatePatternTests, ChangesStateToConnecting)
{
    Connection connection{std::make_unique<DisconnectedState>()};

    connection.setState(std::make_unique<ConnectingState>());

    EXPECT_EQ(connection.stateName(), "Connecting");
}

TEST(StatePatternTests, ChangesStateToConnected)
{
    Connection connection{std::make_unique<DisconnectedState>()};

    connection.setState(std::make_unique<ConnectingState>());
    connection.setState(std::make_unique<ConnectedState>());

    EXPECT_EQ(connection.stateName(), "Connected");
}

TEST(StatePatternTests, TransitionsFromDisconnectedToConnecting)
{
    Connection connection{std::make_unique<DisconnectedState>()};

    connection.connect();

    EXPECT_EQ(connection.stateName(), "Connecting");
}

TEST(StatePatternTests, TransitionsFromConnectingToConnected)
{
    Connection connection{std::make_unique<ConnectingState>()};

    connection.connected();

    EXPECT_EQ(connection.stateName(), "Connected");
}

TEST(StatePatternTests, TransitionsFromConnectedToDisconnected)
{
    Connection connection{std::make_unique<ConnectedState>()};

    connection.disconnect();

    EXPECT_EQ(connection.stateName(), "Disconnected");
}

TEST(StatePatternTests, HandlesFullConnectionLifecycle)
{
    Connection connection{std::make_unique<DisconnectedState>()};

    connection.connect();
    EXPECT_EQ(connection.stateName(), "Connecting");

    connection.connected();
    EXPECT_EQ(connection.stateName(), "Connected");

    connection.disconnect();
    EXPECT_EQ(connection.stateName(), "Disconnected");
}

TEST(StatePatternTests, SameActionBehavesDifferentlyDependingOnCurrentState)
{
    Connection disconnected{std::make_unique<DisconnectedState>()};
    Connection connected{std::make_unique<ConnectedState>()};

    disconnected.connect();
    connected.connect();

    EXPECT_EQ(disconnected.stateName(), "Connecting");
    EXPECT_EQ(connected.stateName(), "Connected");
}

TEST(AdapterPatternTests, AdaptsSendToLegacyTransmit)
{
    LegacySender legacySender;
    LegacySenderAdapter adapter{legacySender};

    adapter.send("Hello");

    EXPECT_EQ(legacySender.lastMessage(), "Hello");
}

TEST(AdapterPatternTests, ForwardsDifferentMessagesToLegacySender)
{
    LegacySender legacySender;
    LegacySenderAdapter adapter{legacySender};

    adapter.send("First");
    EXPECT_EQ(legacySender.lastMessage(), "First");

    adapter.send("Second");
    EXPECT_EQ(legacySender.lastMessage(), "Second");
}

TEST(DecoratorPatternTests, BasicSenderStoresMessage)
{
    auto sender = std::make_unique<BasicMessageSender>();

    sender->send("Hello");

    EXPECT_EQ(sender->lastMessage(), "Hello");
}

TEST(DecoratorPatternTests, PrefixDecoratorAddsPrefix)
{
    auto basicSender = std::make_unique<BasicMessageSender>();
    BasicMessageSender *basicPtr = basicSender.get();

    PrefixDecorator decorator{std::move(basicSender)};

    decorator.send("Hello");

    EXPECT_EQ(basicPtr->lastMessage(), "[LOG] Hello");
}

TEST(DecoratorPatternTests, DecoratorsCanBeStacked)
{
    auto basicSender = std::make_unique<BasicMessageSender>();
    BasicMessageSender *basicPtr = basicSender.get();

    auto firstDecorator = std::make_unique<PrefixDecorator>(std::move(basicSender));
    PrefixDecorator secondDecorator{std::move(firstDecorator)};

    secondDecorator.send("Hello");

    EXPECT_EQ(basicPtr->lastMessage(), "[LOG] [LOG] Hello");
}

TEST(SingletonPatternTests, ReturnsSameInstance)
{
    ApplicationCounter &counter1 = ApplicationCounter::instance();
    ApplicationCounter &counter2 = ApplicationCounter::instance();

    EXPECT_EQ(&counter1, &counter2);
}

TEST(SingletonPatternTests, SharesStateBetweenAccesses)
{
    ApplicationCounter &counter1 = ApplicationCounter::instance();
    ApplicationCounter &counter2 = ApplicationCounter::instance();

    const int initialValue = counter1.value();

    counter1.increment();

    EXPECT_EQ(counter2.value(), initialValue + 1);
}