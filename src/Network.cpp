#include "Network.h"

#include <algorithm>
#include <iostream>
#include <vector>

void Network::printUsers() const
{
    if (mUsers.empty())
    {
        std::cout << "No users in the network.\n";
        return;
    }

    std::vector<int> userIds;
    userIds.reserve(mUsers.size());

    for (const auto &entry : mUsers)
    {
        userIds.push_back(entry.first);
    }

    std::sort(userIds.begin(), userIds.end());

    for (int id : userIds)
    {
        const User &user = mUsers.at(id);

        std::cout << "User ID: " << id << ", Name: " << user.getName() << "\n";
    }
}
std::size_t Network::getUserCount() const { return mUsers.size(); }

bool Network::addUser(const User &user) { return mUsers.emplace(user.getId(), user).second; }

bool Network::removeUser(int id) { return mUsers.erase(id) > 0; }

bool Network::userExists(int id) const { return mUsers.contains(id); }

std::size_t Network::getCallCount() const { return mCalls.size(); }

void Network::printCalls() const
{
    if (mCalls.empty())
    {
        std::cout << "No calls registered in the network.\n";
        return;
    }

    std::vector<int> callIds;
    callIds.reserve(mCalls.size());

    for (const auto &entry : mCalls)
    {
        callIds.push_back(entry.first);
    }

    std::sort(callIds.begin(), callIds.end());

    for (int id : callIds)
    {
        const Call &call = mCalls.at(id);
        call.print();
    }
}

bool Network::createCall(int callId, int callerId, int receiverId)
{
    if ((userExists(callerId) == false) || (userExists(receiverId) == false) || (callerId == receiverId))
    {
        return false;
    }
    return mCalls.try_emplace(callId, CallParameters{callId, callerId, receiverId}).second;
}

Call *Network::findCall(int callId)
{
    auto it = mCalls.find(callId);
    if (it != mCalls.end())
    {
        return &it->second;
    }
    return nullptr;
}
const Call *Network::findCall(int callId) const
{
    auto it = mCalls.find(callId);
    if (it != mCalls.end())
    {
        return &it->second;
    }
    return nullptr;
}

bool Network::isUserBusy(int userId) const
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

bool Network::startCall(int callId)
{
    Call *currCall = findCall(callId);
    if (currCall == nullptr || isUserBusy(currCall->getCallerId()) || isUserBusy(currCall->getReceiverId()))
    {
        return false;
    }
    return currCall->start();
}

bool Network::endCall(int callId)
{
    Call *currCall = findCall(callId);
    if (currCall == nullptr)
    {
        return false;
    }
    return currCall->end();
}
