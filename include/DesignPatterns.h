#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <utility>

enum class PricingType
{
    Standard,
    Discount
};

class PricingStrategy
{
  public:
    virtual ~PricingStrategy() = default;
    virtual double calculate(double minutes) const = 0;
};

class StandardPricing : public PricingStrategy
{
  public:
    double calculate(double minutes) const override { return minutes * 0.5; }
};

class DiscountPricing : public PricingStrategy
{
  public:
    double calculate(double minutes) const override { return minutes * 0.25; }
};

class CallCostCalculator
{
  public:
    explicit CallCostCalculator(std::unique_ptr<PricingStrategy> strategy) : mStrategy(std::move(strategy)) {}
    double calculate(double minutes) const { return mStrategy->calculate(minutes); }

  private:
    std::unique_ptr<PricingStrategy> mStrategy;
};

class MessageObserver
{
  public:
    virtual ~MessageObserver() = default;
    virtual void onMessage(std::string_view message) = 0;
};

class MessagePublisher
{
  public:
    void subscribe(const std::shared_ptr<MessageObserver> &observer) { observers.emplace_back(observer); }
    void publish(std::string_view message)
    {
        for (auto &observer : observers)
        {
            if (auto subscriber = observer.lock())
            {
                subscriber->onMessage(message);
            }
        }
    }

  private:
    std::vector<std::weak_ptr<MessageObserver>> observers;
};

class RecordingObserver : public MessageObserver
{
  public:
    void onMessage(std::string_view message) override { mLastMessage = message; }
    const std::string &getLastMessage() const { return mLastMessage; }

  private:
    std::string mLastMessage;
};

class PricingFactory
{
  public:
    static std::unique_ptr<PricingStrategy> create(PricingType type)
    {
        if (type == PricingType::Standard)
        {
            return std::make_unique<StandardPricing>();
        }
        else if (type == PricingType::Discount)
        {
            return std::make_unique<DiscountPricing>();
        }
        return nullptr;
    }
};

class ConnectionState
{
  public:
    virtual ~ConnectionState() = default;
    virtual std::string_view name() const = 0;
    virtual std::unique_ptr<ConnectionState> connect() const = 0;
    virtual std::unique_ptr<ConnectionState> connected() const = 0;
    virtual std::unique_ptr<ConnectionState> disconnect() const = 0;
};

template <typename State> std::unique_ptr<ConnectionState> makeConnectionState();

class DisconnectedState;
class ConnectingState;
class ConnectedState;

class DisconnectedState : public ConnectionState
{
  public:
    std::string_view name() const override { return mName; }
    std::unique_ptr<ConnectionState> connect() const override { return makeConnectionState<ConnectingState>(); }
    std::unique_ptr<ConnectionState> connected() const override { return makeConnectionState<DisconnectedState>(); }
    std::unique_ptr<ConnectionState> disconnect() const override { return makeConnectionState<DisconnectedState>(); }

  private:
    std::string mName = "Disconnected";
};
class ConnectingState : public ConnectionState
{
  public:
    std::string_view name() const override { return mName; }
    std::unique_ptr<ConnectionState> connect() const override { return makeConnectionState<ConnectingState>(); }
    std::unique_ptr<ConnectionState> connected() const override { return makeConnectionState<ConnectedState>(); }
    std::unique_ptr<ConnectionState> disconnect() const override { return makeConnectionState<DisconnectedState>(); }

  private:
    std::string mName = "Connecting";
};
class ConnectedState : public ConnectionState
{
  public:
    std::string_view name() const override { return mName; }
    std::unique_ptr<ConnectionState> connect() const override { return makeConnectionState<ConnectedState>(); }
    std::unique_ptr<ConnectionState> connected() const override { return makeConnectionState<ConnectedState>(); }
    std::unique_ptr<ConnectionState> disconnect() const override { return makeConnectionState<DisconnectedState>(); }

  private:
    std::string mName = "Connected";
};

template <typename State> std::unique_ptr<ConnectionState> makeConnectionState() { return std::make_unique<State>(); }

class Connection
{
  public:
    explicit Connection(std::unique_ptr<ConnectionState> state) : mState(std::move(state)) {}
    std::string_view stateName() const { return mState->name(); }
    void setState(std::unique_ptr<ConnectionState> state) { mState = std::move(state); }
    void connect() { mState = mState->connect(); }
    void connected() { mState = mState->connected(); }
    void disconnect() { mState = mState->disconnect(); }

  private:
    std::unique_ptr<ConnectionState> mState;
};

class MessageSender
{
  public:
    virtual ~MessageSender() = default;
    virtual void send(std::string_view message) = 0;
};

class LegacySender
{
  public:
    void transmit(const std::string &message) { mLastMessage = message; }
    const std::string &lastMessage() const { return mLastMessage; }

  private:
    std::string mLastMessage;
};

class LegacySenderAdapter : public MessageSender
{
  public:
    explicit LegacySenderAdapter(LegacySender &ls) : mlegacysender(ls) {}
    void send(std::string_view message) override
    {
        std::string str(message);
        mlegacysender.transmit(str);
    }

  private:
    LegacySender &mlegacysender;
};

class BasicMessageSender : public MessageSender
{
  public:
    void send(std::string_view message) override { mLastMessage = std::string(message); }

    const std::string &lastMessage() const { return mLastMessage; }

  private:
    std::string mLastMessage;
};
class PrefixDecorator : public MessageSender
{
  public:
    explicit PrefixDecorator(std::unique_ptr<MessageSender> ptr) : mPtr(std::move(ptr)) {}

    void send(std::string_view message) override { mPtr->send("[LOG] " + std::string(message)); }

  private:
    std::unique_ptr<MessageSender> mPtr;
};

class ApplicationCounter
{
  public:
    static ApplicationCounter &instance()
    {
        static ApplicationCounter config;
        return config;
    }

    void increment() { mValue++; }
    int value() const { return mValue; }

  private:
    ApplicationCounter() = default;

    int mValue{0};
};
