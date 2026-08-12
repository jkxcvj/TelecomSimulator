#include "Network.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <utility>
#include <vector>

std::string_view toString(StartCallError error)
{
    switch (error)
    {
    case StartCallError::CallNotFound:
        return "Call not found";
    case StartCallError::CallAlreadyStarted:
        return "Call already started";
    case StartCallError::CallAlreadyEnded:
        return "Call already ended";
    case StartCallError::UserBusy:
        return "User busy";
    }
    return "Unknown start call error";
}

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
    std::for_each(userIds.begin(), userIds.end(),
                  [this](const auto &element)
                  {
                      const User &user = mUsers.at(element);

                      std::cout << "User ID: " << element.value << ", Name: " << user.getName() << "\n";
                  });
}
std::size_t Network::getUserCount() const { return mUsers.size(); }

bool Network::addUser(const User &user)
{
    bool val = mUsers.emplace(user.getId(), user).second;
    if (val)
    {
        publishEvent("User registered");
    }
    else
    {
        publishEvent("User registration rejected");
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
        publishEvent("Call creation rejected");
        return false;
    }
    publishEvent("Call created");
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
    return std::any_of(mCalls.begin(), mCalls.end(),
                       [userId](const std::pair<const CallId, Call> &mCalls)
                       {
                           return ((mCalls.second.getCallerId() == userId || mCalls.second.getReceiverId() == userId) &&
                                   mCalls.second.getStatusId() == CallStatus::Active);
                       });
}

StartCallResult Network::startCall(CallId callId)
{
    Call *currCall = findCall(callId);

    if (currCall == nullptr)
    {
        publishEvent("Call start rejected");
        return StartCallError::CallNotFound;
    }

    if (currCall->getStatusId() == CallStatus::Active)
    {
        publishEvent("Call start rejected");
        return StartCallError::CallAlreadyStarted;
    }

    if (currCall->getStatusId() == CallStatus::Ended)
    {
        publishEvent("Call start rejected");
        return StartCallError::CallAlreadyEnded;
    }

    if (isUserBusy(currCall->getCallerId()) || isUserBusy(currCall->getReceiverId()))
    {
        publishEvent("Call start rejected");
        return StartCallError::UserBusy;
    }

    currCall->start();
    publishEvent("Call started");

    return std::monostate{};
}

bool Network::endCall(CallId callId)
{
    Call *currCall = findCall(callId);
    if (currCall == nullptr)
    {
        publishEvent("Call end rejected");
        return false;
    }
    publishEvent("Call ended");
    return currCall->end();
}

void Network::subscribe(const std::shared_ptr<EventSubscriber> &subscriber) { mEventDispatcher.subscribe(subscriber); }
void Network::unsubscribe(const std::shared_ptr<EventSubscriber> &subscriber) { mEventDispatcher.unsubscribe(subscriber); }

void Network::publishEvent(std::string_view message)
{
    mLogger->log(message);
    mEventDispatcher.notify(message);
}

std::size_t Network::getActiveCallCount() const
{
    return std::count_if(mCalls.begin(), mCalls.end(),
                         [](const std::pair<const CallId, Call> &mCalls) { return mCalls.second.getStatusId() == CallStatus::Active; });
}

std::vector<CallId> Network::getCallIds() const
{
    std::vector<CallId> callIds;
    callIds.reserve(mCalls.size());

    std::transform(mCalls.begin(), mCalls.end(), std::back_inserter(callIds), [](const auto &entry) { return entry.first; });

    return callIds;
}

std::size_t Network::removeEndedCalls()
{
    return std::erase_if(mCalls, [](const auto &entry) { return entry.second.getStatusId() == CallStatus::Ended; });
}

std::vector<UserId> Network::getUserIdsSortedByName() const
{
    std::vector<UserId> userIds;
    userIds.reserve(mUsers.size());

    std::transform(mUsers.begin(), mUsers.end(), std::back_inserter(userIds), [](const auto &entry) { return entry.first; });
    std::sort(userIds.begin(), userIds.end(), [this](UserId left, UserId right) { return mUsers.at(left).getName() < mUsers.at(right).getName(); });
    return userIds;
}

bool Network::areAllCallsEnded() const
{
    return std::all_of(mCalls.begin(), mCalls.end(), [](const auto &entry) { return entry.second.getStatusId() == CallStatus::Ended; });
}
bool Network::hasNoActiveCalls() const
{
    return std::none_of(mCalls.begin(), mCalls.end(), [](const auto &entry) { return entry.second.getStatusId() == CallStatus::Active; });
}

std::vector<UserId> Network::getUsersWithoutActiveCalls() const
{
    std::vector<UserId> allUserIds;
    allUserIds.reserve(mUsers.size());

    std::transform(mUsers.begin(), mUsers.end(), std::back_inserter(allUserIds), [](const auto &entry) { return entry.first; });

    std::vector<UserId> availableUserIds;
    availableUserIds.reserve(allUserIds.size());

    std::copy_if(allUserIds.begin(), allUserIds.end(), std::back_inserter(availableUserIds), [this](UserId userId) { return !isUserBusy(userId); });

    std::sort(availableUserIds.begin(), availableUserIds.end());

    return availableUserIds;
}