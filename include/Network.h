#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <unordered_map>
#include <variant>
#include <vector>

#include "Call.h"
#include "EventDispatcher.h"
#include "NetworkPersistence.h"
#include "User.h"

enum class StartCallError
{
    CallNotFound,
    CallAlreadyStarted,
    CallAlreadyEnded,
    UserBusy
};

enum class CreateCallError
{
    CallerNotFound,
    ReceiverNotFound,
    CallerBusy,
    ReceiverBusy,
    SameUser,
    CallAlreadyExists
};

enum class GetCallError
{
    NotFound
};

using StartCallResult = std::variant<std::monostate, StartCallError>;
using CreateCallResult = std::variant<std::monostate, CreateCallError>;
using GetCallResult = std::variant<std::reference_wrapper<const Call>, GetCallError>;

std::string_view toString(StartCallError error);

class Network
{
  public:
    Network() = default;
    Network(const Network &) = delete;
    Network &operator=(const Network &) = delete;
    Network(Network &&) = default;
    Network &operator=(Network &&) = default;
    bool addUser(const User &user);
    bool removeUser(UserId id);
    void printUsers() const;
    std::size_t getUserCount() const;
    CreateCallResult createCall(CallId callId, UserId callerId, UserId receiverId);
    void printCalls() const;
    std::size_t getCallCount() const;
    StartCallResult startCall(CallId callId);
    bool endCall(CallId callId);
    void subscribe(const std::shared_ptr<EventSubscriber> &subscriber);
    void unsubscribe(const std::shared_ptr<EventSubscriber> &subscriber);
    std::size_t getActiveCallCount() const;
    std::vector<CallId> getCallIds() const;
    std::size_t removeEndedCalls();
    std::vector<UserId> getUserIdsSortedByName() const;
    bool areAllCallsEnded() const;
    bool hasNoActiveCalls() const;
    std::vector<UserId> getUsersWithoutActiveCalls() const;
    const User *getUser(UserId id) const;
    const Call *getCall(CallId id) const;
    GetCallResult getCallResult(CallId id) const;
    std::vector<std::reference_wrapper<const User>> getUsers() const;
    std::vector<std::reference_wrapper<const Call>> getCalls() const;

  private:
    std::unordered_map<UserId, User, UserIdHash> mUsers;
    std::unordered_map<CallId, Call, CallIdHash> mCalls;
    bool userExists(UserId id) const;
    Call *findCall(CallId callId);
    const Call *findCall(CallId callId) const;
    bool isUserBusy(UserId userId) const;
    EventDispatcher mEventDispatcher;
    void publishEvent(EventType eventType);
    bool restoreCall(const Call &call);
    friend LoadNetworkResult loadNetwork(Network &network, const std::filesystem::path &path);
};
