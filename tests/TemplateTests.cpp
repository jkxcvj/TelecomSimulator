#include <gtest/gtest.h>
#include <string_view>
#include <type_traits>

#include "GenericUtils.h"

namespace
{

template <typename T> std::string_view describeType(const T &) { return "generic"; }

std::string_view describeType(int) { return "int"; }

template <typename T> struct TypeLabel
{
    static constexpr std::string_view value = "generic";
};

template <> struct TypeLabel<int>
{
    static constexpr std::string_view value = "integer";
};

template <typename T> struct TypeCategory
{
    static constexpr std::string_view value = "generic";
};

template <typename T> struct TypeCategory<T *>
{
    static constexpr std::string_view value = "pointer";
};

template <typename T> std::string_view classifyType(const T &)
{
    if constexpr (std::is_integral_v<T>)
    {
        return "integral";
    }
    else if constexpr (std::is_floating_point_v<T>)
    {
        return "floating point";
    }
    else
    {
        return "other";
    }
}

struct NotComparable
{
    int value;
};

} // namespace

static_assert(LessThanComparable<int>);
static_assert(LessThanComparable<double>);
static_assert(!LessThanComparable<NotComparable>);

TEST(TemplateTests, CheckLargerOf)
{
    auto y = largerOf(8.5, 3.2);
    auto z = largerOf(-2, -7);
    EXPECT_DOUBLE_EQ(largerOf<double>(10, 20.5), 20.5);
    EXPECT_EQ(y, 8.5);
    EXPECT_EQ(z, -2);
}

TEST(TemplateTests, CheckBoxClass)
{
    Box<int> intBox{10};
    EXPECT_EQ(intBox.get(), 10);

    Box<std::string> stringBox{"hello"};
    EXPECT_EQ(stringBox.get(), "hello");

    stringBox.set("world");
    EXPECT_EQ(stringBox.get(), "world");
}

TEST(TemplateTests, PrefersNonTemplateOverloadWhenBothMatch)
{
    EXPECT_EQ(describeType(10), "int");
    EXPECT_EQ(describeType(10.5), "generic");
}

TEST(TemplateTests, TypeLabelTest)
{
    EXPECT_EQ(TypeLabel<double>::value, "generic");
    EXPECT_EQ(TypeLabel<std::string>::value, "generic");
    EXPECT_EQ(TypeLabel<int>::value, "integer");
}

TEST(TemplateTests, PartiallySpecializesPointerTypes)
{
    EXPECT_EQ(TypeCategory<int>::value, "generic");
    EXPECT_EQ(TypeCategory<double>::value, "generic");

    EXPECT_EQ(TypeCategory<int *>::value, "pointer");
    EXPECT_EQ(TypeCategory<double *>::value, "pointer");
}

TEST(TemplateTests, TypeTraits)
{
    EXPECT_EQ(classifyType(10), "integral");
    EXPECT_EQ(classifyType(10.5), "floating point");
    EXPECT_EQ(classifyType(std::string{"hello"}), "other");
}

TEST(TemplateTests, ConceptTests)
{
    EXPECT_EQ(doubleValue(5), 10);
    EXPECT_EQ(doubleValue(20L), 40L);
}