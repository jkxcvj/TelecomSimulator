#include <gtest/gtest.h>
#include <optional>
#include <stdexcept>
#include <string>
#include <variant>

#include "Network.h"

namespace
{

enum class ParseError
{
    NotPositive
};

using ParseResult = std::variant<int, ParseError>;

class DestructionTracker
{
  public:
    explicit DestructionTracker(bool &destroyed) : mDestroyed(destroyed) {}

    ~DestructionTracker() { mDestroyed = true; }

  private:
    bool &mDestroyed;
};

class ConfigurationError : public std::runtime_error
{
  public:
    explicit ConfigurationError(const std::string &message) : std::runtime_error(message) {}
};

class MemberTracker
{
  public:
    explicit MemberTracker(bool &destroyed) : mDestroyed(destroyed) {}

    ~MemberTracker() { mDestroyed = true; }

  private:
    bool &mDestroyed;
};

struct DestructionState
{
    bool memberDestroyed{false};
    bool objectDestroyed{false};
};

class FailingObject
{
  public:
    explicit FailingObject(DestructionState &state) : mMember(state.memberDestroyed), mObjectDestroyed(state.objectDestroyed)
    {
        throw std::runtime_error("Constructor failed");
    }

    ~FailingObject() { mObjectDestroyed = true; }

  private:
    MemberTracker mMember;
    bool &mObjectDestroyed;
};

class SafeDestructor
{
  public:
    explicit SafeDestructor(bool &errorHandled) : mErrorHandled(errorHandled) {}

    ~SafeDestructor() noexcept
    {
        try
        {
            cleanup();
        }
        catch (...)
        {
            mErrorHandled = true;
        }
    }

  private:
    void cleanup() { throw std::runtime_error("Cleanup failed"); }

    bool &mErrorHandled;
};

void functionThatThrows(bool &destroyed)
{
    DestructionTracker obj(destroyed);

    throw std::runtime_error("Something went wrong");
}

bool isValidUserId(int id) { return id > 0; }

std::optional<int> findValue(bool exists)
{
    if (exists)
    {
        return 42;
    }

    return std::nullopt;
}

void validateConfiguration(bool configurationLoaded)
{
    if (configurationLoaded == false)
    {
        throw ConfigurationError("Configuration could not be loaded");
    }
}

std::optional<int> findUserAge(bool userExists)
{
    if (userExists)
    {
        return 25;
    }

    return std::nullopt;
}

ParseResult parsePositiveNumber(int value)
{
    if (value > 0)
    {
        return value;
    }

    return ParseError::NotPositive;
}

} // namespace

TEST(ExceptionSafetyTests, DestroysLocalObjectsDuringStackUnwinding)
{
    bool destroyed = false;

    EXPECT_THROW(functionThatThrows(destroyed), std::runtime_error);

    EXPECT_TRUE(destroyed);
}

TEST(ExceptionSafetyTests, ReturnsFalseForInvalidExpectedInput) { EXPECT_FALSE(isValidUserId(-2)); }

TEST(ExceptionSafetyTests, ReturnsEmptyOptionalWhenValueDoesNotExist) { EXPECT_EQ(findValue(false), std::nullopt); }

TEST(ExceptionSafetyTests, ThrowsWhenRequiredConfigurationCannotBeLoaded) { EXPECT_THROW(validateConfiguration(false), std::runtime_error); }

TEST(ExceptionSafetyTests, ThrowsConfigurationErrorWhenConfigurationCannotBeLoaded)
{
    EXPECT_THROW(validateConfiguration(false), ConfigurationError);
}

TEST(ExceptionSafetyTests, CatchesMostSpecificExceptionFirst)
{
    bool caughtSpecificException = false;

    try
    {
        validateConfiguration(false);
    }
    catch (const ConfigurationError &)
    {
        caughtSpecificException = true;
    }
    catch (const std::runtime_error &)
    {
        FAIL() << "Expected ConfigurationError, but caught std::runtime_error";
    }

    EXPECT_TRUE(caughtSpecificException);
}

TEST(ExceptionSafetyTests, DestroysConstructedMembersWhenConstructorThrows)
{
    DestructionState state;

    EXPECT_THROW(
        {
            FailingObject object(state);
            (void)object;
        },
        std::runtime_error);

    EXPECT_TRUE(state.memberDestroyed);
    EXPECT_FALSE(state.objectDestroyed);
}

TEST(ExceptionSafetyTests, DestructorHandlesCleanupFailureInternally)
{
    bool errorHandled = false;

    {
        SafeDestructor object(errorHandled);
    }

    EXPECT_TRUE(errorHandled);
}

TEST(ExceptionSafetyTests, OptionalRepresentsMissingValue)
{
    const auto age = findUserAge(false);

    EXPECT_FALSE(age.has_value());
}

TEST(ExceptionSafetyTests, OptionalContainsValueWhenFound)
{
    const auto age = findUserAge(true);

    EXPECT_TRUE(age.has_value());
    EXPECT_EQ(age.value_or(-1), 25);
}

TEST(ExceptionSafetyTests, ResultContainsValueOnSuccess)
{
    const auto result = parsePositiveNumber(42);

    ASSERT_TRUE(std::holds_alternative<int>(result));
    EXPECT_EQ(std::get<int>(result), 42);
}

TEST(ExceptionSafetyTests, ResultContainsErrorOnFailure)
{
    const auto result = parsePositiveNumber(-5);

    ASSERT_TRUE(std::holds_alternative<ParseError>(result));
    EXPECT_EQ(std::get<ParseError>(result), ParseError::NotPositive);
}

TEST(StartCallErrorTests, ConvertsErrorsToString)
{
    EXPECT_EQ(toString(StartCallError::CallNotFound), "Call not found");
    EXPECT_EQ(toString(StartCallError::CallAlreadyStarted), "Call already started");
    EXPECT_EQ(toString(StartCallError::CallAlreadyEnded), "Call already ended");
    EXPECT_EQ(toString(StartCallError::UserBusy), "User busy");
}