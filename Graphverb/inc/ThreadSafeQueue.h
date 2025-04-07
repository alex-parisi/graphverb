#ifndef THREAD_SAFE_QUEUE_H
#define THREAD_SAFE_QUEUE_H

#include <condition_variable>
#include <queue>

/**
 * @brief A thread-safe queue for storing vectors of data.
 * @tparam T The type of data stored in the queue.
 */
template<typename T>
class ThreadSafeQueue {
public:
    /**
     * @brief Push a vector of data into the queue.
     * @param data The vector of data to be pushed into the queue.
     */
    void push(const std::vector<T> &data) {
        std::lock_guard lock(mutex);
        queue.push(data);
    }

    /**
     * @brief Pop a vector of data from the queue.
     * @param out The vector to store the popped data.
     * @return True if data was popped, false if the queue was empty.
     */
    bool pop(std::vector<T> &out) {
        std::lock_guard lock(mutex);
        if (queue.empty())
            return false;
        out = std::move(queue.front());
        queue.pop();
        return true;
    }

private:
    /** The queue to store the data */
    std::queue<std::vector<T>> queue;

    /** Mutex for thread safety */
    std::mutex mutex;
};

#endif // THREAD_SAFE_QUEUE_H
