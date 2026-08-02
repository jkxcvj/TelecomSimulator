#pragma once

enum class CallStatus
{
    Created = 0,
    Active = 1,
    Ended = 2
};

struct CallParameters
{
    int callId;
    int callerId;
    int receiverId;
};
class Call
{
  public:
    explicit Call(const CallParameters &parameters);
    int getId() const;
    int getCallerId() const;
    int getReceiverId() const;
    CallStatus getStatusId() const;
    bool start();
    bool end();
    void print() const;

  private:
    CallParameters mParameters;
    CallStatus mStatus;
};
