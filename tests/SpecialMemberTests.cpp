#include <gtest/gtest.h>
#include <type_traits>

#include "FileEventLogger.h"
#include "IntBuffer.h"

static_assert(!std::is_copy_constructible_v<FileEventLogger>);
static_assert(!std::is_copy_assignable_v<FileEventLogger>);
static_assert(std::is_move_constructible_v<FileEventLogger>);
static_assert(std::is_move_assignable_v<FileEventLogger>);

static_assert(std::is_copy_constructible_v<IntBuffer>);
static_assert(std::is_copy_assignable_v<IntBuffer>);
static_assert(std::is_move_constructible_v<IntBuffer>);
static_assert(std::is_move_assignable_v<IntBuffer>);

TEST(IntBufferTests, StoresValues)
{
    IntBuffer buffer(3);

    EXPECT_EQ(buffer.size(), 3);

    buffer[0] = 10;
    buffer[1] = 20;
    buffer[2] = 30;

    EXPECT_EQ(buffer[0], 10);
    EXPECT_EQ(buffer[1], 20);
    EXPECT_EQ(buffer[2], 30);
}

TEST(IntBufferTests, CopyConstructorCreatesIndependentCopy)
{
    IntBuffer original(3);

    original[0] = 10;
    original[1] = 20;
    original[2] = 30;

    IntBuffer copy(original);

    EXPECT_EQ(copy.size(), 3);
    EXPECT_EQ(copy[0], 10);
    EXPECT_EQ(copy[1], 20);
    EXPECT_EQ(copy[2], 30);

    copy[0] = 999;

    EXPECT_EQ(copy[0], 999);
    EXPECT_EQ(original[0], 10);
}

TEST(IntBufferTests, CopyAssignmentCreatesIndependentCopy)
{
    IntBuffer first(3);

    first[0] = 10;
    first[1] = 20;
    first[2] = 30;

    IntBuffer second(5);

    second = first;

    EXPECT_EQ(second.size(), 3);
    EXPECT_EQ(second[0], 10);
    EXPECT_EQ(second[1], 20);
    EXPECT_EQ(second[2], 30);

    second[0] = 999;

    EXPECT_EQ(second[0], 999);
    EXPECT_EQ(first[0], 10);
}

TEST(IntBufferTests, HandlesSelfAssignment)
{
    IntBuffer buffer(2);

    buffer[0] = 10;
    buffer[1] = 20;

    buffer = buffer;

    EXPECT_EQ(buffer.size(), 2);
    EXPECT_EQ(buffer[0], 10);
    EXPECT_EQ(buffer[1], 20);
}

TEST(IntBufferTests, MoveConstructorTransfersOwnership)
{
    IntBuffer original(3);

    original[0] = 10;
    original[1] = 20;
    original[2] = 30;

    IntBuffer moved(std::move(original));

    EXPECT_EQ(moved.size(), 3);
    EXPECT_EQ(moved[0], 10);
    EXPECT_EQ(moved[1], 20);
    EXPECT_EQ(moved[2], 30);
}

TEST(IntBufferTests, MoveAssignmentTransfersOwnership)
{
    IntBuffer original(3);

    original[0] = 10;
    original[1] = 20;
    original[2] = 30;

    IntBuffer destination(5);

    destination[0] = 999;

    destination = std::move(original);

    EXPECT_EQ(destination.size(), 3);
    EXPECT_EQ(destination[0], 10);
    EXPECT_EQ(destination[1], 20);
    EXPECT_EQ(destination[2], 30);
}