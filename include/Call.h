#pragma once

#include "Identifiers.h"

enum class CallStatus
{
    Created = 0,
    Active = 1,
    Ended = 2
};

struct CallParameters
{
    CallId callId;
    UserId callerId;
    UserId receiverId;
};
class Call
{
  public:
    explicit Call(const CallParameters &parameters);
    Call(const CallParameters &parameters, CallStatus status);
    CallId getId() const;
    UserId getCallerId() const;
    UserId getReceiverId() const;
    CallStatus getStatusId() const;
    bool start();
    bool end();
    void print() const;

  private:
    CallParameters mParameters;
    CallStatus mStatus;
};
