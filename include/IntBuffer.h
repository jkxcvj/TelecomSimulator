#pragma once

#include <cstddef>
#include <vector>

class IntBufferOLD
{
  public:
    explicit IntBufferOLD(std::size_t size);
    ~IntBufferOLD();

    IntBufferOLD(const IntBufferOLD &other);
    IntBufferOLD &operator=(const IntBufferOLD &other);
    IntBufferOLD(IntBufferOLD &&other) noexcept;
    IntBufferOLD &operator=(IntBufferOLD &&other) noexcept;
    void swap(IntBufferOLD &other) noexcept;

    std::size_t size() const;
    int &operator[](std::size_t index);
    const int &operator[](std::size_t index) const;

  private:
    std::size_t mSize;
    int *mData;
};

class IntBuffer
{
  public:
    explicit IntBuffer(std::size_t size);

    std::size_t size() const;

    int &operator[](std::size_t index);
    const int &operator[](std::size_t index) const;

  private:
    std::vector<int> mData;
};