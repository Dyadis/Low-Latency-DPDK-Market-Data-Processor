#pragma once

#include <array>
#include <atomic>

template<typename T, size_t Size>
class LockFreeRingBuffer {
private:
    std::array<T, Size> buffer;
    std::atomic<size_t> head{0};
    std::atomic<size_t> tail{0};

    /* Calculate the next index in the circular buffer
     * This wraps around to 0 when it reaches the end
     */
    size_t next(size_t current) {
        return (current + 1) % Size;
    }

public:
    /* Attempt to push an item into the buffer
     * Returns true if successful, false if the buffer is full
     */
    bool push(const T& item) {
        size_t current_tail = tail.load(std::memory_order_relaxed);
        size_t next_tail = next(current_tail);
        if (next_tail == head.load(std::memory_order_acquire))
            return false;  // Buffer is full
        buffer[current_tail] = item;
        tail.store(next_tail, std::memory_order_release);
        return true;
    }

    /* Attempt to pop an item from the buffer
     * Returns true if successful, false if the buffer is empty
     */
    bool pop(T& item) {
        size_t current_head = head.load(std::memory_order_relaxed);
        if (current_head == tail.load(std::memory_order_acquire))
            return false;  // Buffer is empty
        item = buffer[current_head];
        head.store(next(current_head), std::memory_order_release);
        return true;
    }
};