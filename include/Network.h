#pragma once

#include <vector>
#include "User.h"
#include <cstddef>
#include "Call.h"

class Network
{
public:
    bool addUser(const User &user);
    bool removeUser(int id);
    void printUsers() const;
    std::size_t getUserCount() const;
    bool createCall(int callId, int callerId, int receiverId);
    void printCalls() const;
    std::size_t getCallCount() const;
    bool startCall(int callId);
    bool endCall(int callId);

private:
    std::vector<User> mUsers;
    std::vector<Call> mCalls;
    bool userExists(int id) const;
    bool callExists(int id) const;
    Call *findCall(int callId);
    const Call *findCall(int callId) const;
    bool isUserBusy(int userId) const;
};