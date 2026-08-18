#pragma once

#include <cstddef>
#include <memory>
#include <unordered_map>
#include <variant>
#include <vector>

#include "Call.h"
#include "CallStatistics.h"
#include "EventDispatcher.h"
#include "EventLogger.h"
#include "User.h"

enum class StartCallError
{
    CallNotFound,
    CallAlreadyStarted,
    CallAlreadyEnded,
    UserBusy
};

using StartCallResult = std::variant<std::monostate, StartCallError>;

std::string_view toString(StartCallError error);

class Network
{
  public:
    explicit Network(std::unique_ptr<EventLogger> logger, CallStatistics &statistics);
    bool addUser(const User &user);
    bool removeUser(UserId id);
    void printUsers() const;
    std::size_t getUserCount() const;
    bool createCall(CallId callId, UserId callerId, UserId receiverId);
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

  private:
    std::unordered_map<UserId, User, UserIdHash> mUsers;
    std::unordered_map<CallId, Call, CallIdHash> mCalls;
    bool userExists(UserId id) const;
    Call *findCall(CallId callId);
    const Call *findCall(CallId callId) const;
    bool isUserBusy(UserId userId) const;
    std::unique_ptr<EventLogger> mLogger;
    EventDispatcher mEventDispatcher;
    void publishEvent(std::string_view message);
    CallStatistics &mStatistics;
};
