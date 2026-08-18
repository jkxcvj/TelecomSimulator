#pragma once

#include <atomic>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <stop_token>
#include <string>

struct BankAccount
{
    explicit BankAccount(int initialBalance) : balance(initialBalance) {}

    int balance{0};
    std::mutex mutex;
};

void transfer(BankAccount &from, BankAccount &to, int amount)
{
    std::scoped_lock lock(from.mutex, to.mutex);
    if (from.balance < amount)
    {
        std::cout << "You dont have that kind of money\n";
    }
    else
    {
        from.balance -= amount;
        to.balance += amount;
    }
}

void incrementCounter(int &counter) { ++counter; }
void incrementManyTimes(int &counter, int iterations)
{
    for (int i = 0; i < iterations; ++i)
    {
        ++counter;
    }
}
void incrementManyTimesSafe(int &counter, std::mutex &mutex, int iterations)
{
    for (int i = 0; i < iterations; ++i)
    {
        std::lock_guard<std::mutex> lock(mutex);
        ++counter;
    }
}

void incrementAtomic(std::atomic<int> &counter, int iterations)
{
    for (int i = 0; i < iterations; ++i)
    {
        ++counter;
    }
}

struct MessageQueue
{
    std::queue<std::string> messages;
    std::mutex mutex;
    std::condition_variable condition;
};

void produce(MessageQueue &queue, const std::string &message)
{
    {
        std::lock_guard<std::mutex> lock(queue.mutex);
        queue.messages.push(message);
    }

    queue.condition.notify_one();
}
std::string consume(MessageQueue &queue)
{
    std::unique_lock<std::mutex> lock(queue.mutex);
    queue.condition.wait(lock, [&queue] { return queue.messages.empty() == false; });
    std::string message = queue.messages.front();
    queue.messages.pop();
    return message;
}

void runUntilStopped(const std::stop_token &stopToken, std::atomic<int> &counter)
{
    while (stopToken.stop_requested() == false)
    {
        ++counter;
    }
}

int calculateWithThreadByValue(int input)
{
    int result = 0;
    std::thread worker{[input, &result]() { result = input * 2; }};
    worker.join();
    return result;
}
int calculateWithThreadByReference(int input)
{
    int result = 0;
    std::thread worker{[&input, &result] { result = input * 2; }};
    worker.join();
    return result;
}

int slowDouble(int value) { return value * 2; }

int safeDivide(int left, int right)
{
    if (right == 0)
    {
        throw std::runtime_error("Nie dziel przez zero cholero");
    }
    return left / right;
}

void calculateAndSetPromise(std::promise<int> promise, int value) { promise.set_value(value * 2); }

void calculateOrFail(std::promise<int> promise, int value)
{
    try
    {
        if (value < 0)
        {
            throw std::runtime_error("Negative value");
        }

        promise.set_value(value * 2);
    }
    catch (...)
    {
        promise.set_exception(std::current_exception());
    }
}

class SafeCounter
{
  public:
    void increment()
    {
        std::lock_guard<std::mutex> lock(mMutex);
        ++mValue;
    }
    int value() const
    {
        std::lock_guard<std::mutex> lock(mMutex);
        return mValue;
    }

  private:
    int mValue{0};
    mutable std::mutex mMutex;
};