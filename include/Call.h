#pragma once

enum class CallStatus
{
    Created = 0,
    Active = 1,
    Ended = 2
};
class Call
{
  public:
    Call(int callId, int callerId, int receiverId);
    int getId() const;
    int getCallerId() const;
    int getReceiverId() const;
    CallStatus getStatusId() const;
    bool start();
    bool end();
    void print() const;

  private:
    int mId;
    int mCallerId;
    int mReceiverId;
    CallStatus mStatus;
};
