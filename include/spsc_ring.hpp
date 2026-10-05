#pragma once
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdexcept>

// HW 7 (Session 7) / Project Phase 3 — single-producer/single-consumer lock-free ring buffer.
// Walk-through: labs/session07.md, Part C.
struct SPSCRing {
    explicit SPSCRing(std::size_t capacity_pow2)
        : capacity_(capacity_pow2), mask_(capacity_pow2 - 1),
          buffer_(capacity_pow2 == 0 ? nullptr : new std::uint64_t[capacity_pow2]) {
        if (capacity_ == 0 || (capacity_ & (capacity_ - 1)) != 0) {
            throw std::invalid_argument("SPSCRing capacity must be a nonzero power of two");
        }
    }

    bool push(std::uint64_t v) {
        const std::uint64_t head = head_.value.load(std::memory_order_relaxed);
        const std::uint64_t tail = tail_.value.load(std::memory_order_acquire);
        if (head - tail == capacity_) return false;
        buffer_[head & mask_] = v;
        head_.value.store(head + 1, std::memory_order_release);
        return true;
    }

    bool pop(std::uint64_t& out) {
        const std::uint64_t tail = tail_.value.load(std::memory_order_relaxed);
        const std::uint64_t head = head_.value.load(std::memory_order_acquire);
        if (tail == head) return false;
        out = buffer_[tail & mask_];
        tail_.value.store(tail + 1, std::memory_order_release);
        return true;
    }

    bool empty() const {
        return tail_.value.load(std::memory_order_acquire) ==
               head_.value.load(std::memory_order_acquire);
    }

    bool full() const {
        const std::uint64_t head = head_.value.load(std::memory_order_acquire);
        const std::uint64_t tail = tail_.value.load(std::memory_order_acquire);
        return head - tail == capacity_;
    }

private:
    struct alignas(64) PaddedIndex {
        std::atomic<std::uint64_t> value{0};
    };

    const std::uint64_t capacity_;
    const std::uint64_t mask_;
    std::unique_ptr<std::uint64_t[]> buffer_;
    PaddedIndex head_;
    PaddedIndex tail_;
};
