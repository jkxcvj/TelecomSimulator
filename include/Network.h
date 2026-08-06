#pragma once

#include <cstddef>
#include <memory>
#include <unordered_map>
#include <vector>

#include "Call.h"
#include "EventDispatcher.h"
#include "EventLogger.h"
#include "User.h"

class Network
{
  public:
    explicit Network(std::unique_ptr<EventLogger> logger);
    bool addUser(const User &user);
    bool removeUser(UserId id);
    void printUsers() const;
    std::size_t getUserCount() const;
    bool createCall(CallId callId, UserId callerId, UserId receiverId);
    void printCalls() const;
    std::size_t getCallCount() const;
    bool startCall(CallId callId);
    bool endCall(CallId callId);
    void subscribe(const std::shared_ptr<EventSubscriber> &subscriber);
    void unsubscribe(const std::shared_ptr<EventSubscriber> &subscriber);

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
};
