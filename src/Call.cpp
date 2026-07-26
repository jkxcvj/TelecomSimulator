#include "Call.h"
#include <iostream>

Call::Call(int callId, int callerId, int receiverId) : mId(callId), mCallerId(callerId), mReceiverId(receiverId), mStatus(CallStatus::Created)
{
}

int Call::getId() const
{
    return mId;
}
int Call::getCallerId() const
{
    return mCallerId;
}
int Call::getReceiverId() const
{
    return mReceiverId;
}
CallStatus Call::getStatusId() const
{
    return mStatus;
}

bool Call::start()
{
    if (mStatus == CallStatus::Created)
    {
        mStatus = CallStatus::Active;
        return true;
    }
    else
    {
        return false;
    }
}

bool Call::end()
{
    if (mStatus == CallStatus::Active)
    {
        mStatus = CallStatus::Ended;
        return true;
    }
    else
    {
        return false;
    }
}

void Call::print() const
{
    std::cout << "Call ID: " << mId << "\n";
    std::cout << "Caller ID: " << mCallerId << "\n";
    std::cout << "Receiver ID: " << mReceiverId << "\n";
    switch (mStatus)
    {
    case CallStatus::Created:
        std::cout << "Status: Created \n";
        break;
    case CallStatus::Active:
        std::cout << "Status: Active \n";
        break;
    case CallStatus::Ended:
        std::cout << "Status: Ended \n";
        break;
    }
}