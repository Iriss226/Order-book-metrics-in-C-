#pragma once

#include <cstdint>

namespace market {

struct TopOfBookSnapshot {
    double bid_price;
    std::uint64_t bid_size;
    double ask_price;
    std::uint64_t ask_size;
};

struct TopOfBookMetrics {
    double best_bid;
    double best_ask;
    double mid;
    double spread;
    double microprice;
    double obi;
};

TopOfBookMetrics calculate_metrics(const TopOfBookSnapshot& snapshot);

}  // namespace market
