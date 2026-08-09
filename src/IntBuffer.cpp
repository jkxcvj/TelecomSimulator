#include "IntBuffer.h"

#include <utility>

IntBufferOLD::IntBufferOLD(std::size_t size) : mSize(size), mData(new int[size]){};
IntBufferOLD::~IntBufferOLD() { delete[] mData; };
IntBufferOLD::IntBufferOLD(const IntBufferOLD &other) : mSize(other.mSize), mData(new int[other.mSize])
{
    for (std::size_t i = 0; i < mSize; ++i)
    {
        mData[i] = other.mData[i];
    }
}

IntBufferOLD &IntBufferOLD::operator=(const IntBufferOLD &other)
{
    if (this == &other)
    {
        return *this;
    }

    IntBufferOLD temporary(other);
    swap(temporary);

    return *this;
}

IntBufferOLD::IntBufferOLD(IntBufferOLD &&other) noexcept : mSize(other.mSize), mData(other.mData)
{
    other.mSize = 0;
    other.mData = nullptr;
}
IntBufferOLD &IntBufferOLD::operator=(IntBufferOLD &&other) noexcept
{
    if (this == &other)
    {
        return *this;
    }

    delete[] mData;

    mSize = other.size();

    mData = other.mData;

    other.mSize = 0;
    other.mData = nullptr;

    return *this;
}

std::size_t IntBufferOLD::size() const { return mSize; }

int &IntBufferOLD::operator[](std::size_t index) { return mData[index]; }

const int &IntBufferOLD::operator[](std::size_t index) const { return mData[index]; }

void IntBufferOLD::swap(IntBufferOLD &other) noexcept
{
    std::swap(this->mData, other.mData);
    std::swap(this->mSize, other.mSize);
}

IntBuffer::IntBuffer(std::size_t size) : mData(size) {}

std::size_t IntBuffer::size() const { return mData.size(); }

int &IntBuffer::operator[](std::size_t index) { return mData[index]; }

const int &IntBuffer::operator[](std::size_t index) const { return mData[index]; }