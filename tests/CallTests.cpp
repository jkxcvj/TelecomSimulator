#include <gtest/gtest.h>

#include "Call.h"

TEST(CallTests, StartChangesStatusToActive)
{
    Call createdCall(CallParameters{CallId{1}, UserId{1}, UserId{2}});

    EXPECT_TRUE(createdCall.start());
    EXPECT_EQ(CallStatus::Active, createdCall.getStatusId());
}

TEST(CallTests, EndBeforeStartIsRejectedAndKeepsCreatedStatus)
{
    Call createdCall(CallParameters{CallId{1}, UserId{1}, UserId{2}});

    EXPECT_FALSE(createdCall.end());
    EXPECT_EQ(CallStatus::Created, createdCall.getStatusId());
}

TEST(CallTests, CreationStoresIdentifiersAndSetsCreatedStatus)
{
    Call createdCall(CallParameters{CallId{1}, UserId{2}, UserId{3}});

    EXPECT_EQ(CallId{1}, createdCall.getId());
    EXPECT_EQ(UserId{2}, createdCall.getCallerId());
    EXPECT_EQ(UserId{3}, createdCall.getReceiverId());
    EXPECT_EQ(CallStatus::Created, createdCall.getStatusId());
}

TEST(CallTests, StartWhenAlreadyActiveIsRejectedAndKeepsActiveStatus)
{
    Call activeCall(CallParameters{CallId{1}, UserId{1}, UserId{2}});
    ASSERT_TRUE(activeCall.start());

    EXPECT_FALSE(activeCall.start());
    EXPECT_EQ(CallStatus::Active, activeCall.getStatusId());
}

TEST(CallTests, EndWhenActiveSucceedsAndChangesStatusToEnded)
{
    Call activeCall(CallParameters{CallId{1}, UserId{1}, UserId{2}});
    ASSERT_TRUE(activeCall.start());

    EXPECT_TRUE(activeCall.end());
    EXPECT_EQ(CallStatus::Ended, activeCall.getStatusId());
}

TEST(CallTests, EndWhenAlreadyEndedIsRejectedAndKeepsEndedStatus)
{
    Call endedCall(CallParameters{CallId{1}, UserId{1}, UserId{2}});
    ASSERT_TRUE(endedCall.start());
    ASSERT_TRUE(endedCall.end());

    EXPECT_FALSE(endedCall.end());
    EXPECT_EQ(CallStatus::Ended, endedCall.getStatusId());
}

TEST(CallTests, StartAfterEndIsRejectedAndKeepsEndedStatus)
{
    Call endedCall(CallParameters{CallId{1}, UserId{1}, UserId{2}});
    ASSERT_TRUE(endedCall.start());
    ASSERT_TRUE(endedCall.end());

    EXPECT_FALSE(endedCall.start());
    EXPECT_EQ(CallStatus::Ended, endedCall.getStatusId());
}
