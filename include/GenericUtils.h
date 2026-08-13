#pragma once

#include <concepts>

template <typename T>
concept LessThanComparable = requires(const T &left, const T &right) {
    { left < right } -> std::convertible_to<bool>;
};

template <LessThanComparable T> T largerOf(T left, T right) { return left < right ? right : left; }

template <typename T> class Box
{
  public:
    explicit Box(T value) : mValue(value) {};

    const T &get() const { return mValue; };
    void set(T value) { mValue = value; };

  private:
    T mValue;
};

template <std::integral T> T doubleValue(T value) { return value * 2; }

template <typename Container> typename Container::value_type getFirst(const Container &container) { return *container.begin(); }
