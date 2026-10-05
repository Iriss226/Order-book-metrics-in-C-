#pragma once
#include <atomic>
#include <cstdint>
#include <new>

// Project Phase 4 (Session 7; labs/session07.md, Part D) — POD ring living entirely in a shared-memory region (NO pointers),
// usable across processes. init() is called once by the creator before fork().
struct ShmRing {
    static constexpr uint32_t CAPACITY = 1024;   // power of two
    void init() {
        ::new (static_cast<void*>(&head)) std::atomic<uint32_t>(0);
        ::new (static_cast<void*>(&tail)) std::atomic<uint32_t>(0);
    }
    bool push(uint64_t v) {
        const uint32_t head_value = head.load(std::memory_order_relaxed);
        const uint32_t tail_value = tail.load(std::memory_order_acquire);
        if (head_value - tail_value == CAPACITY) return false;
        buf[head_value & (CAPACITY - 1)] = v;
        head.store(head_value + 1, std::memory_order_release);
        return true;
    }
    bool pop(uint64_t& out) {
        const uint32_t tail_value = tail.load(std::memory_order_relaxed);
        const uint32_t head_value = head.load(std::memory_order_acquire);
        if (tail_value == head_value) return false;
        out = buf[tail_value & (CAPACITY - 1)];
        tail.store(tail_value + 1, std::memory_order_release);
        return true;
    }

    std::atomic<uint32_t> head, tail; uint64_t buf[CAPACITY];
};
