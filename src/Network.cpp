#include "Network.h"

#include <iostream>

void Network::printUsers() const
{
    if (mUsers.empty())
    {
        std::cout << "No users in the network." << "\n";
        return;
    }
    for (const auto &user : mUsers)
    {
        std::cout << "User ID: " << user.getId() << ", Name: " << user.getName() << "\n";
    }
}
std::size_t Network::getUserCount() const { return mUsers.size(); }

bool Network::addUser(const User &user)
{
    for (const auto &userInDB : mUsers)
    {
        if (user.getId() == userInDB.getId())
        {
            return false;
        }
    }
    mUsers.push_back(user);
    return true;
}

bool Network::removeUser(int id)
{
    for (auto it = mUsers.begin(); it != mUsers.end(); it++)
    {
        if (it->getId() == id)
        {
            mUsers.erase(it);
            return true;
        }
    }
    return false;
}

bool Network::userExists(int id) const
{
    for (const auto &userInDB : mUsers)
    {
        if (id == userInDB.getId())
        {
            return true;
        }
    }
    return false;
}

bool Network::callExists(int id) const
{
    for (const auto &callInDB : mCalls)
    {
        if (id == callInDB.getId())
        {
            return true;
        }
    }
    return false;
}

std::size_t Network::getCallCount() const { return mCalls.size(); }

void Network::printCalls() const
{
    if (mCalls.empty())
    {
        std::cout << "No calls registered in the network." << "\n";
        return;
    }
    for (const auto &call : mCalls)
    {
        call.print();
    }
}

bool Network::createCall(int callId, int callerId, int receiverId)
{
    if ((userExists(callerId) == false) || (userExists(receiverId) == false) || (callerId == receiverId) ||
        (callExists(callId)))
    {
        return false;
    }
    mCalls.emplace_back(callId, callerId, receiverId);
    return true;
}

Call *Network::findCall(int callId)
{
    for (auto &call : mCalls)
    {
        if (call.getId() == callId)
        {
            return &call;
        }
    }
    return nullptr;
}
const Call *Network::findCall(int callId) const
{
    for (const auto &call : mCalls)
    {
        if (call.getId() == callId)
        {
            return &call;
        }
    }
    return nullptr;
}

bool Network::isUserBusy(int userId) const
{
    for (const auto &call : mCalls)
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