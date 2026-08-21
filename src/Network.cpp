#include "Network.h"

#include <algorithm>
#include <iostream>
#include <iterator>
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
        publishEvent(EventType::UserRegistered);
    }
    else
    {
        publishEvent(EventType::UserRegistrationRejected);
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

CreateCallResult Network::createCall(CallId callId, UserId callerId, UserId receiverId)
{
    if ((findCall(callId) != nullptr))
    {
        publishEvent(EventType::CallCreationRejected);
        return CreateCallError::CallAlreadyExists;
    }
    else if (!userExists(callerId))
    {
        publishEvent(EventType::CallCreationRejected);
        return CreateCallError::CallerNotFound;
    }
    else if (!userExists(receiverId))
    {
        publishEvent(EventType::CallCreationRejected);
        return CreateCallError::ReceiverNotFound;
    }
    else if (callerId == receiverId)
    {
        publishEvent(EventType::CallCreationRejected);
        return CreateCallError::SameUser;
    }
    else if (isUserBusy(callerId))
    {
        publishEvent(EventType::CallCreationRejected);
        return CreateCallError::CallerBusy;
    }
    else if (isUserBusy(receiverId))
    {
        publishEvent(EventType::CallCreationRejected);
        return CreateCallError::ReceiverBusy;
    }
    publishEvent(EventType::CallCreated);
    mCalls.try_emplace(callId, CallParameters{callId, callerId, receiverId});
    return std::monostate{};
}

bool Network::restoreCall(const Call &call)
{
    const bool restored =
        mCalls.try_emplace(call.getId(), CallParameters{call.getId(), call.getCallerId(), call.getReceiverId()}, call.getStatusId()).second;

    if (restored)
    {
        publishEvent(EventType::CallRestored);
    }

    return restored;
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
        publishEvent(EventType::CallStartRejected);
        return StartCallError::CallNotFound;
    }

    if (currCall->getStatusId() == CallStatus::Active)
    {
        publishEvent(EventType::CallStartRejected);
        return StartCallError::CallAlreadyStarted;
    }

    if (currCall->getStatusId() == CallStatus::Ended)
    {
        publishEvent(EventType::CallStartRejected);
        return StartCallError::CallAlreadyEnded;
    }

    if (isUserBusy(currCall->getCallerId()) || isUserBusy(currCall->getReceiverId()))
    {
        publishEvent(EventType::CallStartRejected);
        return StartCallError::UserBusy;
    }

    currCall->start();
    publishEvent(EventType::CallStarted);

    return std::monostate{};
}

bool Network::endCall(CallId callId)
{
    Call *currCall = findCall(callId);
    if (currCall == nullptr)
    {
        publishEvent(EventType::CallEndRejected);
        return false;
    }
    publishEvent(EventType::CallEnded);
    return currCall->end();
}

void Network::subscribe(const std::shared_ptr<EventSubscriber> &subscriber) { mEventDispatcher.subscribe(subscriber); }
void Network::unsubscribe(const std::shared_ptr<EventSubscriber> &subscriber) { mEventDispatcher.unsubscribe(subscriber); }

void Network::publishEvent(EventType eventType) { mEventDispatcher.notify(eventType); }

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

const Call *Network::getCall(CallId id) const { return findCall(id); }

const User *Network::getUser(UserId id) const
{
    const auto it = mUsers.find(id);

    if (it == mUsers.end())
    {
        return nullptr;
    }

    return &it->second;
}

GetCallResult Network::getCallResult(CallId id) const
{
    const Call *call = findCall(id);

    if (call == nullptr)
    {
        return GetCallError::NotFound;
    }

    return std::cref(*call);
}

std::vector<std::reference_wrapper<const User>> Network::getUsers() const
{
    std::vector<std::reference_wrapper<const User>> users;
    users.reserve(mUsers.size());

    for (const auto &[id, user] : mUsers)
    {
        users.push_back(std::cref(user));
    }

    return users;
}
std::vector<std::reference_wrapper<const Call>> Network::getCalls() const
{
    std::vector<std::reference_wrapper<const Call>> calls;
    calls.reserve(mCalls.size());

    for (const auto &[id, call] : mCalls)
    {
        calls.push_back(std::cref(call));
    }

    return calls;
}
