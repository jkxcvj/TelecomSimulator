#include <functional>
#include <future>
#include <gtest/gtest.h>
#include <thread>
#include <vector>

#include "CallStatistics.h"
#include "ConcurrencyExamples.h"

TEST(ConcurencyTets, FirstConcurrency)
{
    int counter = 0;
    std::thread worker{incrementCounter, std::ref(counter)};
    worker.join();
    EXPECT_EQ(counter, 1);
}

TEST(ConcurrencyTests, ThreadIsNotJoinableAfterJoin)
{
    int counter = 0;

    std::thread worker{incrementCounter, std::ref(counter)};

    EXPECT_TRUE(worker.joinable());

    worker.join();

    EXPECT_FALSE(worker.joinable());
    EXPECT_EQ(counter, 1);
}

TEST(ConcurrencyTests, JThreadJoinsAutomatically)
{
    int counter = 0;

    {
        std::jthread worker{incrementCounter, std::ref(counter)};
    }

    EXPECT_EQ(counter, 1);
}

// TEST(ConcurrencyTests, DemonstratesRaceCondition)
// {
//     int counter = 0;
//     constexpr int iterations = 100000;

//     std::thread worker1{incrementManyTimes, std::ref(counter), iterations};
//     std::thread worker2{incrementManyTimes, std::ref(counter), iterations};

//     worker1.join();
//     worker2.join();

//     EXPECT_EQ(counter, iterations * 2);
// }

TEST(ConcurrencyTests, ProtectsCounterWithMutex)
{
    int counter = 0;
    std::mutex mutex;

    constexpr int iterations = 100000;

    std::thread worker1{incrementManyTimesSafe, std::ref(counter), std::ref(mutex), iterations};

    std::thread worker2{incrementManyTimesSafe, std::ref(counter), std::ref(mutex), iterations};

    worker1.join();
    worker2.join();

    EXPECT_EQ(counter, iterations * 2);
}

TEST(ConcurrencyTests, ProtectsCounterWithAtomic)
{
    std::atomic<int> counter{0};
    constexpr int iterations = 100000;

    std::thread worker1{incrementAtomic, std::ref(counter), iterations};
    std::thread worker2{incrementAtomic, std::ref(counter), iterations};

    worker1.join();
    worker2.join();

    EXPECT_EQ(counter.load(), iterations * 2);
}

TEST(ConcurrencyTests, TransfersMoneySafelyBetweenAccounts)
{
    BankAccount accountA{100};
    BankAccount accountB{100};

    std::thread worker1{transfer, std::ref(accountA), std::ref(accountB), 50};
    std::thread worker2{transfer, std::ref(accountB), std::ref(accountA), 30};

    worker1.join();
    worker2.join();

    EXPECT_EQ(accountA.balance + accountB.balance, 200);
}

TEST(ConcurrencyTests, ConsumerWaitsForProducedMessage)
{
    MessageQueue queue;
    std::string receivedMessage;

    std::thread consumerThread{[&queue, &receivedMessage] { receivedMessage = consume(queue); }};

    std::thread producerThread{[&queue] { produce(queue, "Hello"); }};

    producerThread.join();
    consumerThread.join();

    EXPECT_EQ(receivedMessage, "Hello");
}

TEST(ConcurrencyTests, ConsumesMessagesInFIFOOrder)
{
    MessageQueue queue;

    produce(queue, "First");
    produce(queue, "Second");

    EXPECT_EQ(consume(queue), "First");
    EXPECT_EQ(consume(queue), "Second");
}

TEST(ConcurrencyTests, JThreadStopsCooperatively)
{
    std::atomic<int> counter{0};

    {
        std::jthread worker{runUntilStopped, std::ref(counter)};

        while (counter.load() == 0)
        {
            std::this_thread::yield();
        }

        worker.request_stop();
    }

    const int valueAfterStop = counter.load();

    EXPECT_GT(valueAfterStop, 0);
}

TEST(ConcurrencyTests, CalculatesUsingThreadByValueAndReference)
{
    EXPECT_EQ(calculateWithThreadByValue(10), 20);
    EXPECT_EQ(calculateWithThreadByReference(10), 20);
}

TEST(ConcurrencyTests, GetsResultFromAsyncTask)
{
    auto future = std::async(std::launch::async, slowDouble, 21);

    EXPECT_EQ(future.get(), 42);
}

TEST(ConcurrencyTests, GetsValueFromFuture)
{
    auto future = std::async(std::launch::async, safeDivide, 20, 4);

    EXPECT_EQ(future.get(), 5);
}

TEST(ConcurrencyTests, FuturePropagatesException)
{
    auto future = std::async(std::launch::async, safeDivide, 20, 0);

    EXPECT_THROW(future.get(), std::runtime_error);
}

TEST(ConcurrencyTests, GetsResultFromPromise)
{
    std::promise<int> promise;
    std::future<int> future = promise.get_future();

    std::thread worker{calculateAndSetPromise, std::move(promise), 21};

    EXPECT_EQ(future.get(), 42);

    worker.join();
}

TEST(ConcurrencyTests, PromisePropagatesException)
{
    std::promise<int> promise;
    std::future<int> future = promise.get_future();

    std::thread worker{calculateOrFail, std::move(promise), -1};

    EXPECT_THROW(future.get(), std::runtime_error);

    worker.join();
}

TEST(ConcurrencyTests, SafeCounterIsThreadSafe)
{
    SafeCounter counter;

    constexpr int threadCount = 4;
    constexpr int incrementsPerThread = 50000;

    std::vector<std::thread> workers;
    workers.reserve(threadCount);

    for (int i = 0; i < threadCount; ++i)
    {
        workers.emplace_back(
            [&counter]
            {
                for (int j = 0; j < incrementsPerThread; ++j)
                {
                    counter.increment();
                }
            });
    }

    for (auto &worker : workers)
    {
        worker.join();
    }

    EXPECT_EQ(counter.value(), threadCount * incrementsPerThread);
}

TEST(CallStatisctiscTest, CallStatisticIsThreadSafe)
{
    CallStatistics counter;

    constexpr int threadCount = 4;

    std::vector<std::thread> workers;
    workers.reserve(threadCount);

    for (int i = 0; i < threadCount; ++i)
    {
        workers.emplace_back(
            [&counter]
            {
                for (int j = 0; j < 10000; ++j)
                {
                    counter.recordCreated();
                }
                for (int k = 0; k < 5000; ++k)
                {
                    counter.recordStarted();
                }
                for (int l = 0; l < 2000; ++l)
                {
                    counter.recordEnded();
                }
            });
    }

    for (auto &worker : workers)
    {
        worker.join();
    }

    EXPECT_EQ(counter.createdCount(), 40000);
    EXPECT_EQ(counter.startedCount(), 20000);
    EXPECT_EQ(counter.endedCount(), 8000);
}