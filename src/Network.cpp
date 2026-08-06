#include "Network.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <utility>
#include <vector>

Network::Network(std::unique_ptr<EventLogger> logger) : mLogger(std::move(logger))
{
    if (mLogger == nullptr)
    {
        throw std::invalid_argument("Event logger cannot be null");
    }
}

void Network::printUsers() const
{
    if (mUsers.empty())
    {
        std::cout << "No users in the network.\n";
        return;
    }

    std::vector<UserId> userIds;
    userIds.reserve(mUsers.size());

    for (const auto &entry : mUsers)
    {
        userIds.push_back(entry.first);
    }

    std::sort(userIds.begin(), userIds.end());

    for (UserId id : userIds)
    {
        const User &user = mUsers.at(id);

        std::cout << "User ID: " << id.value << ", Name: " << user.getName() << "\n";
    }
}
std::size_t Network::getUserCount() const { return mUsers.size(); }

bool Network::addUser(const User &user)
{
    bool val = mUsers.emplace(user.getId(), user).second;
    if (val)
    {
        mLogger->log("User registred");
    }
    else
    {
        mLogger->log("User registration rejected");
    }
    return val;
}

bool Network::removeUser(UserId id) { return mUsers.erase(id) > 0; }

bool Network::userExists(UserId id) const { return mUsers.contains(id); }

std::size_t Network::getCallCount() const { return mCalls.size(); }

void Network::printCalls() const
{
    if (mCalls.empty())
    {
        std::cout << "No calls registered in the network.\n";
        return;
    }

    std::vector<CallId> callIds;
    callIds.reserve(mCalls.size());

    for (const auto &entry : mCalls)
    {
        callIds.push_back(entry.first);
    }

    std::sort(callIds.begin(), callIds.end());

    for (CallId id : callIds)
    {
        const Call &call = mCalls.at(id);
        call.print();
    }
}

bool Network::createCall(CallId callId, UserId callerId, UserId receiverId)
{
    if ((userExists(callerId) == false) || (userExists(receiverId) == false) || (callerId == receiverId))
    {
        return false;
    }
    return mCalls.try_emplace(callId, CallParameters{callId, callerId, receiverId}).second;
}

Call *Network::findCall(CallId callId)
{
    auto it = mCalls.find(callId);
    if (it != mCalls.end())
    {
        return &it->second;
    }
    return nullptr;
}
const Call *Network::findCall(CallId callId) const
{
    auto it = mCalls.find(callId);
    if (it != mCalls.end())
    {
        return &it->second;
    }
    return nullptr;
}

bool Network::isUserBusy(UserId userId) const
{
    for (const auto &[id, call] : mCalls)
    {
        if ((call.getCallerId() == userId || call.getReceiverId() == userId) &&
            call.getStatusId() == CallStatus::Active)
        {
            return true;
        }
    }
    return false;
}

bool Network::startCall(CallId callId)
{
    Call *currCall = findCall(callId);
    if (currCall == nullptr || isUserBusy(currCall->getCallerId()) || isUserBusy(currCall->getReceiverId()))
    {
        return false;
    }
    return currCall->start();
}

bool Network::endCall(CallId callId)
{
    Call *currCall = findCall(callId);
    if (currCall == nullptr)
    {
        return false;
    }
    return currCall->end();
}
