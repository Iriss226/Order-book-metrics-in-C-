#include "top_of_book.h"

#include <array>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <stdexcept>

namespace market {

TopOfBookMetrics calculate_metrics(const TopOfBookSnapshot& snapshot) {
    if (!std::isfinite(snapshot.bid_price) || snapshot.bid_price <= 0.0 ||
        !std::isfinite(snapshot.ask_price) || snapshot.ask_price <= 0.0) {
        throw std::invalid_argument("bid and ask prices must be finite and positive");
    }

    const long double bid_size = snapshot.bid_size;
    const long double ask_size = snapshot.ask_size;
    const long double total_size = bid_size + ask_size;
    if (total_size == 0.0L) {
        throw std::invalid_argument("at least one side must have nonzero size");
    }

    const double mid = snapshot.bid_price +
                       (snapshot.ask_price - snapshot.bid_price) / 2.0;
    const long double microprice =
        (static_cast<long double>(snapshot.ask_price) * bid_size +
         static_cast<long double>(snapshot.bid_price) * ask_size) /
        total_size;
    const long double obi = (bid_size - ask_size) / total_size;

    return {
        snapshot.bid_price,
        snapshot.ask_price,
        mid,
        snapshot.ask_price - snapshot.bid_price,
        static_cast<double>(microprice),
        static_cast<double>(obi)
    };
}

}  // namespace market

int main() {
    constexpr std::array<market::TopOfBookSnapshot, 5> snapshots{{
        {100.00, 10, 100.02, 10},
        {100.00, 14, 100.02, 6},
        {100.01, 8, 100.03, 12},
        {100.02, 18, 100.03, 7},
        {100.01, 5, 100.03, 15}
    }};

    std::cout << std::fixed << std::setprecision(4)
              << "step,bid,ask,mid,spread,microprice,obi\n";
    for (std::size_t i = 0; i < snapshots.size(); ++i) {
        const market::TopOfBookMetrics metrics =
            market::calculate_metrics(snapshots[i]);
        std::cout << i + 1 << ','
                  << metrics.best_bid << ','
                  << metrics.best_ask << ','
                  << metrics.mid << ','
                  << metrics.spread << ','
                  << metrics.microprice << ','
                  << metrics.obi << '\n';
    }
}
