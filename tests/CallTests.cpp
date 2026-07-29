#include <gtest/gtest.h>

#include "Call.h"

TEST(CallTests, StartChangesStatusToActive)
{
    Call createdCall(1, 1, 2);

    EXPECT_TRUE(createdCall.start());
    EXPECT_EQ(CallStatus::Active, createdCall.getStatusId());
}

TEST(CallTests, EndBeforeStartIsRejectedAndKeepsCreatedStatus)
{
    Call createdCall(1, 1, 2);

    EXPECT_FALSE(createdCall.end());
    EXPECT_EQ(CallStatus::Created, createdCall.getStatusId());
}

TEST(CallTests, CreationStoresIdentifiersAndSetsCreatedStatus)
{
    Call createdCall(1, 2, 3);

    EXPECT_EQ(1, createdCall.getId());
    EXPECT_EQ(2, createdCall.getCallerId());
    EXPECT_EQ(3, createdCall.getReceiverId());
    EXPECT_EQ(CallStatus::Created, createdCall.getStatusId());
}

TEST(CallTests, StartWhenAlreadyActiveIsRejectedAndKeepsActiveStatus)
{
    Call activeCall(1, 1, 2);
    ASSERT_TRUE(activeCall.start());

    EXPECT_FALSE(activeCall.start());
    EXPECT_EQ(CallStatus::Active, activeCall.getStatusId());
}

TEST(CallTests, EndWhenActiveSucceedsAndChangesStatusToEnded)
{
    Call activeCall(1, 1, 2);
    ASSERT_TRUE(activeCall.start());

    EXPECT_TRUE(activeCall.end());
    EXPECT_EQ(CallStatus::Ended, activeCall.getStatusId());
}

TEST(CallTests, EndWhenAlreadyEndedIsRejectedAndKeepsEndedStatus)
{
    Call endedCall(1, 1, 2);
    ASSERT_TRUE(endedCall.start());
    ASSERT_TRUE(endedCall.end());

    EXPECT_FALSE(endedCall.end());
    EXPECT_EQ(CallStatus::Ended, endedCall.getStatusId());
}

TEST(CallTests, StartAfterEndIsRejectedAndKeepsEndedStatus)
{
    Call endedCall(1, 1, 2);
    ASSERT_TRUE(endedCall.start());
    ASSERT_TRUE(endedCall.end());

    EXPECT_FALSE(endedCall.start());
    EXPECT_EQ(CallStatus::Ended, endedCall.getStatusId());
}
