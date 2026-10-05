#pragma once
#include <cstdint>
#include <deque>

// Optional, ungraded (labs/session06.md, D5) — sliding-window event counter. count() is called with a non-decreasing clock.
//
// Edge case that costs most people a test (labs/session06.md, D5): ts_ns and
// now_ns are UNSIGNED. Early on, now_ns <= window_ns and the mathematical cutoff
// (now - window) is negative — there is no uint64_t that means that, and there is
// no safe value to clamp it to. Only compute the subtraction when it is
// meaningful; do not expire anything before then.
struct RollingCounter {
    explicit RollingCounter(uint64_t window_ns) : window_ns_(window_ns) {}

    void add(uint64_t ts_ns) { timestamps_.push_back(ts_ns); }

    uint64_t count(uint64_t now_ns) {
        if (now_ns >= window_ns_) {
            const uint64_t cutoff = now_ns - window_ns_;
            while (!timestamps_.empty() && timestamps_.front() <= cutoff) {
                timestamps_.pop_front();
            }
        }
        return timestamps_.size();
    }

private:
    uint64_t window_ns_;
    std::deque<uint64_t> timestamps_;
};
