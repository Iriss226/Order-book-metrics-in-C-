#pragma once

#include <cmath>
#include <cstdint>
#include <limits>
#include <map>
#include <string>
#include <unordered_map>

// HW 6, part 2 (Session 6) — a price-level order book + a symbol-to-id map.
// side 'B'=bid, 'S'=ask.  SymMap::get returns (uint64_t)-1 if absent.
//
// Prices use integer cent ticks, avoiding floating-point map-key comparisons.
struct Book {
    void add(uint64_t id, char side, double px, uint32_t qty) {
        if (side != 'B' && side != 'S') return;
        if (!std::isfinite(px) || px <= 0.0 ||
            px * 100.0 >= static_cast<double>(std::numeric_limits<std::int64_t>::max())) return;

        cancel(id);
        if (qty == 0) return;

        const std::int64_t tick = static_cast<std::int64_t>(std::llround(px * 100.0));
        auto& levels = side == 'B' ? bids_ : asks_;
        levels[tick] += qty;
        orders_.emplace(id, Order{side, tick, qty});

        if (side == 'B' && (!has_bid_ || tick > best_bid_tick_)) {
            best_bid_tick_ = tick;
            has_bid_ = true;
        } else if (side == 'S' && (!has_ask_ || tick < best_ask_tick_)) {
            best_ask_tick_ = tick;
            has_ask_ = true;
        }
    }

    void cancel(uint64_t id) {
        const auto order = orders_.find(id);
        if (order == orders_.end()) return;

        const Order record = order->second;
        auto& levels = record.side == 'B' ? bids_ : asks_;
        auto level = levels.find(record.tick);
        if (level != levels.end()) {
            if (level->second > record.qty) {
                level->second -= record.qty;
            } else {
                levels.erase(level);
                if (record.side == 'B' && has_bid_ && record.tick == best_bid_tick_) {
                    has_bid_ = !bids_.empty();
                    if (has_bid_) best_bid_tick_ = bids_.rbegin()->first;
                } else if (record.side == 'S' && has_ask_ && record.tick == best_ask_tick_) {
                    has_ask_ = !asks_.empty();
                    if (has_ask_) best_ask_tick_ = asks_.begin()->first;
                }
            }
        }
        orders_.erase(order);
    }

    double best_bid() const {
        return has_bid_ ? static_cast<double>(best_bid_tick_) / 100.0 : 0.0;
    }

    double best_ask() const {
        return has_ask_ ? static_cast<double>(best_ask_tick_) / 100.0 : 0.0;
    }

private:
    struct Order {
        char side;
        std::int64_t tick;
        std::uint32_t qty;
    };

    std::map<std::int64_t, std::uint64_t> bids_;
    std::map<std::int64_t, std::uint64_t> asks_;
    std::unordered_map<std::uint64_t, Order> orders_;
    std::int64_t best_bid_tick_ = 0;
    std::int64_t best_ask_tick_ = 0;
    bool has_bid_ = false;
    bool has_ask_ = false;
};

struct SymMap {
    void put(const char* sym, uint64_t id) {
        if (sym != nullptr) symbols_[sym] = id;
    }

    uint64_t get(const char* sym) const {
        if (sym == nullptr) return static_cast<uint64_t>(-1);
        const auto entry = symbols_.find(sym);
        return entry == symbols_.end() ? static_cast<uint64_t>(-1) : entry->second;
    }

private:
    std::unordered_map<std::string, uint64_t> symbols_;
};
