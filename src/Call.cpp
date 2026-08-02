#include "Call.h"

#include <iostream>

Call::Call(const CallParameters &parameters) : mParameters(parameters), mStatus(CallStatus::Created) {}

int Call::getId() const { return mParameters.callId; }
int Call::getCallerId() const { return mParameters.callerId; }
int Call::getReceiverId() const { return mParameters.receiverId; }
CallStatus Call::getStatusId() const { return mStatus; }

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
    std::cout << "Call ID: " << mParameters.callId << "\n";
    std::cout << "Caller ID: " << mParameters.callerId << "\n";
    std::cout << "Receiver ID: " << mParameters.receiverId << "\n";
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